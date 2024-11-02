#include "lingmovpnrequest.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>

#include "gsystem-local-alloc.h"
#include "nma-vpn-password-dialog.h"
#include "nm-macros-internal.h"

extern char **environ;

/*****************************************************************************/

typedef struct {
    char *name;
    char *label;
    char *value;
    gboolean is_secret;
    gboolean should_ask;
} EuiSecret;

typedef struct {
    char *uuid;
    char *id;
    char *service_type;

    guint watch_id;
    GPid pid;

    int child_stdout;
    GString *child_response;
    GIOChannel *channel;
    guint channel_eventid;
    GVariantBuilder secrets_builder;
    gboolean external_ui_mode;

    /* These are just for the external UI mode */
    EuiSecret *eui_secrets;
} RequestData;

typedef struct {
    SecretsRequest req;
    RequestData *req_data;
} VpnSecretsInfo;


gint
_nm_utils_ascii_str_to_bool (const char *str,
                              gint default_value)
{
     gsize len;
     char *s = NULL;

     if (!str)
        return default_value;

     while (str[0] && g_ascii_isspace (str[0]))
         str++;

     if (!str[0])
         return default_value;

     len = strlen (str);
     if (g_ascii_isspace (str[len - 1])) {
         s = g_strdup (str);
         g_strchomp (s);
         str = s;
     }

     if (!g_ascii_strcasecmp (str, "true") || !g_ascii_strcasecmp (str, "yes") || !g_ascii_strcasecmp (str, "on") || !g_ascii_strcasecmp (str, "1"))
         default_value = TRUE;
     else if (!g_ascii_strcasecmp (str, "false") || !g_ascii_strcasecmp (str, "no") || !g_ascii_strcasecmp (str, "off") || !g_ascii_strcasecmp (str, "0"))
         default_value = FALSE;
     if (s)
          g_free (s);
     return default_value;
}


/*****************************************************************************/

static void request_data_free (RequestData *req_data);
static void complete_request (VpnSecretsInfo *info);

/*****************************************************************************/

size_t
applet_vpn_request_get_secrets_size (void)
{
    return sizeof (VpnSecretsInfo);
}

/*****************************************************************************/

static void
external_ui_add_secrets (VpnSecretsInfo *info)
{
    RequestData *req_data = info->req_data;
    EuiSecret *secret;
    guint i;

    for (i = 0; req_data->eui_secrets[i].name; i++) {
        secret = &req_data->eui_secrets[i];
        if (   secret->is_secret
            && secret->value
            && secret->value[0]) {
            g_variant_builder_add (&req_data->secrets_builder, "{ss}",
                                   secret->name,
                                   secret->value);
        }
    }
}

static void
external_ui_dialog_response (GtkDialog *dialog, int response_id, gpointer user_data)
{
    VpnSecretsInfo *info = user_data;
    RequestData *req_data = info->req_data;
    NMAVpnPasswordDialog *vpn_dialog = NMA_VPN_PASSWORD_DIALOG (dialog);
    EuiSecret *secret;
    const char *value;
    guint i_secret, i_pw;

    for (i_secret = 0, i_pw = 0; req_data->eui_secrets[i_secret].name; i_secret++) {
        secret = &req_data->eui_secrets[i_secret];
        if (   secret->is_secret
            && secret->should_ask) {
            switch (i_pw) {
            case 0:
                value = nma_vpn_password_dialog_get_password (vpn_dialog);
                break;
            case 1:
                value = nma_vpn_password_dialog_get_password_secondary (vpn_dialog);
                break;
            case 2:
                value = nma_vpn_password_dialog_get_password_ternary (vpn_dialog);
                break;
            default:
                continue;
            }
            g_free (secret->value);
            secret->value = g_strdup (value);
            i_pw++;
        }
    }

    gtk_widget_destroy (GTK_WIDGET (dialog));
    external_ui_add_secrets (info);
    complete_request (info);
}

static gboolean
external_ui_from_child_response (VpnSecretsInfo *info, GError **error)
{
    RequestData *req_data = info->req_data;
    gs_unref_keyfile GKeyFile *keyfile = NULL;
    gs_strfreev char **groups = NULL;
    NMAVpnPasswordDialog *dialog = NULL;
    gs_free char *version = NULL;
    gs_free char *title = NULL;
    gs_free char *message = NULL;
    gsize num_groups;
    guint num_ask = 0;
    guint i_group, i_secret, i_pw;

    /* Parse response key file */
    keyfile = g_key_file_new ();

    if (!g_key_file_load_from_data (keyfile,
                                    req_data->child_response->str,
                                    req_data->child_response->len,
                                    G_KEY_FILE_NONE,
                                    error)) {
        return FALSE;
    }

    groups = g_key_file_get_groups (keyfile, &num_groups);
    if (g_strcmp0 (groups[0], "VPN Plugin UI") != 0) {
        g_set_error_literal (error,
                             NM_SECRET_AGENT_ERROR,
                             NM_SECRET_AGENT_ERROR_FAILED,
                             "Expected [VPN Plugin UI]");
        return FALSE;
    }

    version = g_key_file_get_string (keyfile, "VPN Plugin UI", "Version", error);
    if (!version)
        return FALSE;
    if (strcmp (version, "2") != 0) {
        g_set_error_literal (error,
                             NM_SECRET_AGENT_ERROR,
                             NM_SECRET_AGENT_ERROR_FAILED,
                             "Expected Version=2");
        return FALSE;
    }

    title = g_key_file_get_string (keyfile, "VPN Plugin UI", "Title", error);
    if (!title)
        return FALSE;

    message = g_key_file_get_string (keyfile, "VPN Plugin UI", "Description", error);
    if (!message)
        return FALSE;

    /* Create a secret instance for each group */
    req_data->eui_secrets = g_new0 (EuiSecret, num_groups);
    for (i_group = 1, i_secret = 0; i_group < num_groups; i_group++) {
        EuiSecret *secret = &req_data->eui_secrets[i_secret];
        const char *group = groups[i_group];
        char *label;

        label = g_key_file_get_string (keyfile, group, "Label", NULL);
        if (!label) {
            g_warning ("Skipping entry: no label\n");
            continue;
        }

        secret->name = g_strdup (group);
        secret->label = label;
        secret->value = g_key_file_get_string (keyfile, group, "Value", NULL);
        secret->is_secret = g_key_file_get_boolean (keyfile, group, "IsSecret", NULL);
        secret->should_ask = g_key_file_get_boolean (keyfile, group, "ShouldAsk", NULL);

        i_secret++;

        if (secret->is_secret && secret->should_ask)
            num_ask++;
    }

    /* If there are any secrets that must be asked to user,
     * create a dialog and display it. */
    if (num_ask > 0) {
        dialog = (NMAVpnPasswordDialog *) nma_vpn_password_dialog_new (title, message, NULL);
        g_object_ref_sink (dialog);

        nma_vpn_password_dialog_set_show_password (dialog, FALSE);
        nma_vpn_password_dialog_set_show_password_secondary (dialog, FALSE);
        nma_vpn_password_dialog_set_show_password_ternary (dialog, FALSE);

        for (i_secret = 0, i_pw = 0; req_data->eui_secrets[i_secret].name; i_secret++) {
            EuiSecret *secret = &req_data->eui_secrets[i_secret];

            if (   secret->is_secret
                && secret->should_ask) {
                switch (i_pw) {
                case 0:
                    nma_vpn_password_dialog_set_show_password (dialog, TRUE);
                    nma_vpn_password_dialog_set_password_label (dialog, secret->label);
                    if (secret->value)
                        nma_vpn_password_dialog_set_password (dialog, secret->value);
                    break;
                case 1:
                    nma_vpn_password_dialog_set_show_password_secondary (dialog, TRUE);
                    nma_vpn_password_dialog_set_password_secondary_label (dialog, secret->label);
                    if (secret->value)
                        nma_vpn_password_dialog_set_password_secondary (dialog, secret->value);
                    break;
                case 2:
                    nma_vpn_password_dialog_set_show_password_ternary (dialog, TRUE);
                    nma_vpn_password_dialog_set_password_ternary_label (dialog, secret->label);
                    if (secret->value)
                        nma_vpn_password_dialog_set_password_ternary (dialog, secret->value);
                    break;
                default:
                    g_warning ("Skipping entry: more than 3 passwords not supported\n");
                    continue;
                }
                i_pw++;
            }
        }
        g_signal_connect (dialog,
                          "response",
                          G_CALLBACK (external_ui_dialog_response),
                          info);
        gtk_widget_show (GTK_WIDGET (dialog));
        return TRUE;
    }

    /* Nothing to ask, return known secrets */
    external_ui_add_secrets (info);
    complete_request (info);
    return TRUE;
}

/*****************************************************************************/

static void
complete_request (VpnSecretsInfo *info)
{
    SecretsRequest *req = (SecretsRequest *) info;
    RequestData *req_data = info->req_data;
    GVariantBuilder settings_builder, vpn_builder;
    gs_unref_variant GVariant *settings = NULL;

    g_variant_builder_init (&settings_builder, NM_VARIANT_TYPE_CONNECTION);
    g_variant_builder_init (&vpn_builder, NM_VARIANT_TYPE_SETTING);

    g_variant_builder_add (&vpn_builder, "{sv}",
                           NM_SETTING_VPN_SECRETS,
                           g_variant_builder_end (&req_data->secrets_builder));
    g_variant_builder_add (&settings_builder, "{sa{sv}}",
                           NM_SETTING_VPN_SETTING_NAME,
                           &vpn_builder);
    settings = g_variant_builder_end (&settings_builder);

    applet_secrets_request_complete (req, settings, NULL);
    applet_secrets_request_free (req);
}

static void
process_child_response (VpnSecretsInfo *info)
{
    SecretsRequest *req = (SecretsRequest *) info;
    RequestData *req_data = info->req_data;
    gs_free_error GError *error = NULL;

    if (req_data->external_ui_mode) {
        if (!external_ui_from_child_response (info, &error)) {
            applet_secrets_request_complete (req, NULL, error);
            applet_secrets_request_free (req);
        }
    } else {
        char **lines = g_strsplit (req_data->child_response->str, "\n", -1);
        int i;

        for (i = 0; lines[i] && *(lines[i]); i += 2) {
            if (lines[i + 1] == NULL)
                break;
            g_variant_builder_add (&req_data->secrets_builder, "{ss}", lines[i], lines[i + 1]);
        }

        g_strfreev (lines);
        complete_request (info);
    }
}

static void
child_finished_cb (GPid pid, int status, gpointer user_data)
{
    VpnSecretsInfo *info = user_data;
    SecretsRequest *req = (SecretsRequest *) info;
    RequestData *req_data = info->req_data;
    gs_free_error GError *error = NULL;

    req_data->pid = 0;
    req_data->watch_id = 0;

    if (status) {
        error = g_error_new (NM_SECRET_AGENT_ERROR,
                             NM_SECRET_AGENT_ERROR_USER_CANCELED,
                             "%s.%d (%s): canceled", __FILE__, __LINE__, __func__);

        applet_secrets_request_complete (req, NULL, error);
        applet_secrets_request_free (req);
    } else if (req_data->channel_eventid == 0) {
        /* We now have both the child response and its exit status. Process it. */
        process_child_response (info);
    }
}

static gboolean
child_stdout_data_cb (GIOChannel *source, GIOCondition condition, gpointer user_data)
{
    SecretsRequest *req = user_data;
    VpnSecretsInfo *info = (VpnSecretsInfo *) req;
    RequestData *req_data = info->req_data;
    GIOStatus status;
    char buf[4096];
    size_t bytes_read;
    gs_free_error GError *error = NULL;

    status = g_io_channel_read_chars (source, buf, sizeof (buf)-1, &bytes_read, &error);
    switch (status) {
    case G_IO_STATUS_ERROR:
        req_data->channel_eventid = 0;
        applet_secrets_request_complete (req, NULL, error);
        applet_secrets_request_free (req);
        return FALSE;
    case G_IO_STATUS_EOF:
        req_data->channel_eventid = 0;
        if (req_data->pid == 0) {
            /* We now have both the childe respons and
             * its exit status. Process it. */
            process_child_response (info);
        }
        return FALSE;
    case G_IO_STATUS_NORMAL:
        g_string_append_len (req_data->child_response, buf, bytes_read);
        break;
    default:
        /* What just happened... */
        g_return_val_if_reached (FALSE);
    }

    return TRUE;
}

/*****************************************************************************/

static void
_str_append (GString *str,
             const char *tag,
             const char *val)
{
    const char *s;
    gsize i;

    nm_assert (str);
    nm_assert (tag && tag[0]);
    nm_assert (val);

    g_string_append (str, tag);
    g_string_append_c (str, '=');

    s = strchr (val, '\n');
    if (s) {
        gs_free char *val2 = g_strdup (val);

        for (i = 0; val2[i]; i++) {
            if (val2[i] == '\n')
                val2[i] = ' ';
        }
        g_string_append (str, val2);
    } else
        g_string_append (str, val);
    g_string_append_c (str, '\n');
}

static char *
connection_to_data (NMConnection *connection,
                    gsize *out_length,
                    GError **error)
{
    NMSettingVpn *s_vpn;
    GString *buf;
    const char **keys;
    guint i, len;

    g_return_val_if_fail (NM_IS_CONNECTION (connection), NULL);

    s_vpn = nm_connection_get_setting_vpn (connection);
    if (!s_vpn) {
        g_set_error_literal (error,
                             NM_SECRET_AGENT_ERROR,
                             NM_SECRET_AGENT_ERROR_FAILED,
                             ("Connection had no VPN setting"));
        return NULL;
    }

    buf = g_string_new_len (NULL, 100);

    keys = nm_setting_vpn_get_data_keys (s_vpn, &len);
    for (i = 0; i < len; i++) {
        _str_append (buf, "DATA_KEY", keys[i]);
        _str_append (buf, "DATA_VAL", nm_setting_vpn_get_data_item (s_vpn, keys[i]));
    }
    nm_clear_g_free (&keys);

    keys = nm_setting_vpn_get_secret_keys (s_vpn, &len);
    for (i = 0; i < len; i++) {
        _str_append (buf, "SECRET_KEY", keys[i]);
        _str_append (buf, "SECRET_VAL", nm_setting_vpn_get_secret (s_vpn, keys[i]));
    }
    nm_clear_g_free (&keys);

    g_string_append (buf, "DONE\n\nQUIT\n\n");
    NM_SET_OUT (out_length, buf->len);
    return g_string_free (buf, FALSE);
}

/*****************************************************************************/

static gboolean
connection_to_fd (NMConnection *connection,
                  int fd,
                  GError **error)
{
    gs_free char *data = NULL;
    gsize data_len;
    gssize w;
    int errsv;

    data = connection_to_data (connection, &data_len, error);
    if (!data)
        return FALSE;

again:
    w = write (fd, data, data_len);
    if (w < 0) {
        errsv = errno;
        if (errsv == EINTR)
            goto again;
        g_set_error (error,
                     NM_SECRET_AGENT_ERROR,
                     NM_SECRET_AGENT_ERROR_FAILED,
                     ("Failed to write connection to VPN UI: %s (%d)"), g_strerror (errsv), errsv);
        return FALSE;
    }

    if ((gsize) w != data_len) {
        g_set_error_literal (error,
                             NM_SECRET_AGENT_ERROR,
                             NM_SECRET_AGENT_ERROR_FAILED,
                             ("Failed to write connection to VPN UI: incomplete write"));
        return FALSE;
    }

    return TRUE;
}

/*****************************************************************************/

static void
vpn_child_setup (gpointer user_data)
{
    /* We are in the child process at this point */
    pid_t pid = getpid ();
    setpgid (pid, pid);
}

static gboolean
auth_dialog_spawn (const char *con_id,
                   const char *con_uuid,
                   const char *const*hints,
                   const char *auth_dialog,
                   const char *service_type,
                   gboolean supports_hints,
                   gboolean external_ui_mode,
                   guint32 flags,
                   GPid *out_pid,
                   int *out_stdin,
                   int *out_stdout,
                   GError **error)
{
    gsize hints_len;
    gsize i, j;
    gs_free const char **argv = NULL;
    gs_free const char **envp = NULL;
    gsize environ_len;

    g_return_val_if_fail (con_id, FALSE);
    g_return_val_if_fail (con_uuid, FALSE);
    g_return_val_if_fail (auth_dialog, FALSE);
    g_return_val_if_fail (service_type, FALSE);
    g_return_val_if_fail (out_pid, FALSE);
    g_return_val_if_fail (out_stdin, FALSE);
    g_return_val_if_fail (out_stdout, FALSE);

    hints_len = NM_PTRARRAY_LEN (hints);
    argv = g_new (const char *, 11 + (2 * hints_len));
    i = 0;
    argv[i++] = auth_dialog;
    argv[i++] = "-u";
    argv[i++] = con_uuid;
    argv[i++] = "-n";
    argv[i++] = con_id;
    argv[i++] = "-s";
    argv[i++] = service_type;
    if (flags & NM_SECRET_AGENT_GET_SECRETS_FLAG_ALLOW_INTERACTION)
        argv[i++] = "-i";
    if (flags & NM_SECRET_AGENT_GET_SECRETS_FLAG_REQUEST_NEW)
        argv[i++] = "-r";
    for (j = 0; supports_hints && (j < hints_len); j++) {
        argv[i++] = "-t";
        argv[i++] = hints[j];
    }
    if (external_ui_mode)
        argv[i++] = "--external-ui-mode";
    nm_assert (i <= 10 + (2 * hints_len));
    argv[i++] = NULL;

    environ_len = NM_PTRARRAY_LEN (environ);
    envp = g_new (const char *, environ_len + 1);
    memcpy (envp, environ, sizeof (const char *) * environ_len);
    for (i = 0, j = 0; i < environ_len; i++) {
        const char *e = environ[i];

        if (g_str_has_prefix (e, "G_MESSAGES_DEBUG=")) {
            /* skip this environment variable. We interact with the auth-dialog via stdout.
             * G_MESSAGES_DEBUG may enable additional debugging messages from GTK. */
            continue;
        }
        envp[j++] = e;
    }
    envp[j] = NULL;

    if (!g_spawn_async_with_pipes (NULL,
                                   (char **) argv,
                                   (char **) envp,
                                   G_SPAWN_DO_NOT_REAP_CHILD,
                                   vpn_child_setup,
                                   NULL,
                                   out_pid,
                                   out_stdin,
                                   out_stdout,
                                   NULL,
                                   error))
        return FALSE;

    return TRUE;
}

/*****************************************************************************/

static void
free_vpn_secrets_info (SecretsRequest *req)
{
    request_data_free (((VpnSecretsInfo *) req)->req_data);
}

gboolean
applet_vpn_request_get_secrets (SecretsRequest *req, GError **error)
{
    VpnSecretsInfo *info = (VpnSecretsInfo *) req;
    RequestData *req_data;
    NMSettingConnection *s_con;
    NMSettingVpn *s_vpn;
    const char *connection_type;
    const char *service_type;
    const char *auth_dialog;
    gs_unref_object NMVpnPluginInfo *plugin = NULL;
    int child_stdin;

    applet_secrets_request_set_free_func (req, free_vpn_secrets_info);

    s_con = nm_connection_get_setting_connection (req->connection);
    s_vpn = nm_connection_get_setting_vpn (req->connection);

    connection_type = nm_setting_connection_get_connection_type (s_con);
    g_return_val_if_fail (nm_streq0 (connection_type, NM_SETTING_VPN_SETTING_NAME), FALSE);

    service_type = nm_setting_vpn_get_service_type (s_vpn);
    g_return_val_if_fail (service_type, FALSE);

    plugin = nm_vpn_plugin_info_new_search_file (NULL, service_type);
    auth_dialog = plugin ? nm_vpn_plugin_info_get_auth_dialog (plugin) : NULL;
    if (!auth_dialog) {
        g_set_error (error,
                     NM_SECRET_AGENT_ERROR,
                     NM_SECRET_AGENT_ERROR_FAILED,
                     "Could not find the authentication dialog for VPN connection type '%s'",
                     service_type);
        return FALSE;
    }

    info->req_data = g_slice_new0 (RequestData);
    if (!info->req_data) {
        g_set_error_literal (error,
                             NM_SECRET_AGENT_ERROR,
                             NM_SECRET_AGENT_ERROR_FAILED,
                             "Could not create VPN secrets request object");
        return FALSE;
    }
    req_data = info->req_data;

    g_variant_builder_init (&req_data->secrets_builder, G_VARIANT_TYPE ("a{ss}"));

    req_data->external_ui_mode = _nm_utils_ascii_str_to_bool (
        nm_vpn_plugin_info_lookup_property (plugin,
                                            "GNOME",
                                            "supports-external-ui-mode"),
        FALSE);

    if (!auth_dialog_spawn (nm_setting_connection_get_id (s_con),
                            nm_setting_connection_get_uuid (s_con),
                            (const char *const*) req->hints,
                            auth_dialog,
                            service_type,
                            nm_vpn_plugin_info_supports_hints (plugin),
                            req_data->external_ui_mode,
                            req->flags,
                            &req_data->pid,
                            &child_stdin,
                            &req_data->child_stdout,
                            error))
        return FALSE;

    /* catch when child is reaped */
    req_data->watch_id = g_child_watch_add (req_data->pid, child_finished_cb, info);

    /* listen to what child has to say */
    req_data->channel = g_io_channel_unix_new (req_data->child_stdout);
    req_data->child_response = g_string_sized_new (4096);
    req_data->channel_eventid = g_io_add_watch (req_data->channel,
                                                G_IO_IN  | G_IO_ERR | G_IO_HUP | G_IO_NVAL,
                                                child_stdout_data_cb,
                                                info);

    if (!connection_to_fd (req->connection, child_stdin, error))
        return FALSE;
    close (child_stdin);

    g_io_channel_set_encoding (req_data->channel, NULL, NULL);

    /* Dump parts of the connection to the child */
    return TRUE;
}

/*****************************************************************************/

static gboolean
ensure_killed (gpointer data)
{
    pid_t pid = GPOINTER_TO_INT (data);

    if (kill (pid, 0) == 0)
        kill (pid, SIGKILL);
    /* ensure the child is reaped */
    waitpid (pid, NULL, 0);
    return FALSE;
}

static void
request_data_free (RequestData *req_data)
{
    guint i;

    if (!req_data)
        return;

    g_free (req_data->uuid);
    g_free (req_data->id);
    g_free (req_data->service_type);

    nm_clear_g_source (&req_data->watch_id);

    nm_clear_g_source (&req_data->channel_eventid);
    if (req_data->channel)
        g_io_channel_unref (req_data->channel);

    if (req_data->pid) {
        g_spawn_close_pid (req_data->pid);
        if (kill (req_data->pid, SIGTERM) == 0)
            g_timeout_add_seconds (2, ensure_killed, GINT_TO_POINTER (req_data->pid));
        else {
            kill (req_data->pid, SIGKILL);
            /* ensure the child is reaped */
            waitpid (req_data->pid, NULL, 0);
        }
    }

    if (req_data->child_response)
        g_string_free (req_data->child_response, TRUE);

    g_variant_builder_clear (&req_data->secrets_builder);

    if (req_data->eui_secrets) {
        for (i = 0; req_data->eui_secrets[i].name; i++) {
            g_free (req_data->eui_secrets[i].name);
            g_free (req_data->eui_secrets[i].label);
            g_free (req_data->eui_secrets[i].value);
        }
        g_free (req_data->eui_secrets);
    }

    g_slice_free (RequestData, req_data);
}
