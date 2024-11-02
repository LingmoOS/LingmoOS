#include "lingmoagentinterface.h"
#include "lingmoagent.h"
#include "lingmovpnrequest.h"

#include <gtk/gtk.h>
#include <libnma/nma-wifi-dialog.h>

GSList *        secrets_reqs;
#if 0
typedef struct _SecretsRequest SecretsRequest;
typedef void (*SecretsRequestFreeFunc) (SecretsRequest *req);

struct _SecretsRequest {
         size_t totsize;
         gpointer reqid;
         char *setting_name;
         char **hints;
         guint32 flags;
         AppletAgent *agent;
         AppletAgentSecretsCallback callback;
         gpointer callback_data;

         NMConnection *connection;

         /* Class-specific stuff */
         SecretsRequestFreeFunc free_func;
};
#endif
typedef struct {
        SecretsRequest req;
        GtkWidget *dialog;
} NMWifiInfo;

static AppletAgent * lingmoAgent = NULL;

void
applet_secrets_request_set_free_func (SecretsRequest *req,
                                      SecretsRequestFreeFunc free_func)
{
        req->free_func = free_func;
}

static void
show_ignore_focus_stealing_prevention (GtkWidget *widget)
{
     gtk_widget_realize (widget);
     gtk_widget_show (widget);
     gtk_window_present (GTK_WINDOW (widget));
}

static GVariant *
remove_unwanted_secrets (GVariant *secrets, gboolean keep_8021X)
{
    GVariant *copy, *setting_dict;
    const char *setting_name;
    GVariantBuilder conn_builder;
    GVariantIter conn_iter;

    g_variant_builder_init (&conn_builder, NM_VARIANT_TYPE_CONNECTION);
    g_variant_iter_init (&conn_iter, secrets);
    while (g_variant_iter_next (&conn_iter, "{&s@a{sv}}", &setting_name, &setting_dict)) {
        if (   !strcmp (setting_name, NM_SETTING_WIRELESS_SECURITY_SETTING_NAME)
            || (!strcmp (setting_name, NM_SETTING_802_1X_SETTING_NAME) && keep_8021X))
            g_variant_builder_add (&conn_builder, "{s@a{sv}}", setting_name, setting_dict);

        g_variant_unref (setting_dict);
    }
    copy = g_variant_builder_end (&conn_builder);
    g_variant_unref (secrets);

    return copy;
}

static void
free_wifi_info (SecretsRequest *req)
{
    NMWifiInfo *info = (NMWifiInfo *) req;

    if (info->dialog) {
        gtk_widget_hide (info->dialog);
        gtk_widget_destroy (info->dialog);
        info->dialog = NULL;
    }
}


void
applet_secrets_request_free (SecretsRequest *req)
{
    g_return_if_fail (req != NULL);

    if (req->free_func)
        req->free_func (req);

    secrets_reqs = g_slist_remove (secrets_reqs, req);

    g_object_unref (req->connection);
    g_free (req->setting_name);
    g_strfreev (req->hints);
    memset (req, 0, req->totsize);
    g_free (req);
}

void
applet_secrets_request_complete (SecretsRequest *req,
                                  GVariant *settings,
                                  GError *error)
{
    req->callback (req->agent, error ? NULL : settings, error, req->callback_data);
}

static void
get_secrets_dialog_response_cb (GtkDialog *foo,
                                gint response,
                                gpointer user_data)
{
    SecretsRequest *req = user_data;
    NMWifiInfo *info = (NMWifiInfo *) req;
    NMAWifiDialog *dialog = NMA_WIFI_DIALOG (info->dialog);
    NMConnection *connection = NULL;
    NMSettingWirelessSecurity *s_wireless_sec;
    GVariant *secrets = NULL;
    const char *key_mgmt, *auth_alg;
    gboolean keep_8021X = FALSE;
    GError *error = NULL;

    if (response != GTK_RESPONSE_OK) {
        g_set_error (&error,
                     NM_SECRET_AGENT_ERROR,
                     NM_SECRET_AGENT_ERROR_USER_CANCELED,
                     "%s.%d (%s): canceled",
                     __FILE__, __LINE__, __func__);
        goto done;
    }

    connection = nma_wifi_dialog_get_connection (dialog, NULL, NULL);
    if (!connection) {
        g_set_error (&error,
                     NM_SECRET_AGENT_ERROR,
                     NM_SECRET_AGENT_ERROR_FAILED,
                     "%s.%d (%s): couldn't get connection from Wi-Fi dialog.",
                     __FILE__, __LINE__, __func__);
        goto done;
    }

    /* Second-guess which setting NM wants secrets for. */
    s_wireless_sec = nm_connection_get_setting_wireless_security (connection);
    if (!s_wireless_sec) {
        g_set_error (&error,
                     NM_SECRET_AGENT_ERROR,
                     NM_SECRET_AGENT_ERROR_INVALID_CONNECTION,
                     "%s.%d (%s): requested setting '802-11-wireless-security'"
                     " didn't exist in the connection.",
                     __FILE__, __LINE__, __func__);
        goto done;  /* Unencrypted */
    }

    secrets = nm_connection_to_dbus (connection, NM_CONNECTION_SERIALIZE_ONLY_SECRETS);
    if (!secrets) {
        g_set_error (&error,
                     NM_SECRET_AGENT_ERROR,
                     NM_SECRET_AGENT_ERROR_FAILED,
                     "%s.%d (%s): failed to hash connection '%s'.",
                     __FILE__, __LINE__, __func__, nm_connection_get_id (connection));
        goto done;
    }
    /* If the user chose an 802.1x-based auth method, return 802.1x secrets,
     * not wireless secrets.  Can happen with Dynamic WEP, because NM doesn't
     * know the capabilities of the AP (since Dynamic WEP APs don't broadcast
     * beacons), and therefore defaults to requesting WEP secrets from the
     * wireless-security setting, not the 802.1x setting.
     */
    key_mgmt = nm_setting_wireless_security_get_key_mgmt (s_wireless_sec);
    if (!strcmp (key_mgmt, "ieee8021x") || !strcmp (key_mgmt, "wpa-eap")) {
        /* LEAP secrets aren't in the 802.1x setting */
        auth_alg = nm_setting_wireless_security_get_auth_alg (s_wireless_sec);
        if (!auth_alg || strcmp (auth_alg, "leap")) {
            NMSetting8021x *s_8021x;

            s_8021x = nm_connection_get_setting_802_1x (connection);
            if (!s_8021x) {
                g_set_error (&error,
                             NM_SECRET_AGENT_ERROR,
                             NM_SECRET_AGENT_ERROR_INVALID_CONNECTION,
                             "%s.%d (%s): requested setting '802-1x' didn't"
                             " exist in the connection.",
                             __FILE__, __LINE__, __func__);
                goto done;
            }
            keep_8021X = TRUE;
        }
    }

    /* Remove all not-relevant secrets (inner dicts) */
    secrets = remove_unwanted_secrets (secrets, keep_8021X);
    g_variant_take_ref (secrets);

done:
    applet_secrets_request_complete (req, secrets, error);
    applet_secrets_request_free (req);

    if (secrets)
        g_variant_unref (secrets);
    if (connection)
        nm_connection_clear_secrets (connection);
}

static gboolean
wifi_get_secrets (SecretsRequest *req, GError **error)
{
    NMWifiInfo *info = (NMWifiInfo *) req;

    g_return_val_if_fail (!info->dialog, FALSE);

#if GTK_CHECK_VERSION(3,90,0)
         gtk_init ();
#else
         int argc = 0;
         char ***argv = NULL;
         gtk_init (&argc, &argv);
#endif

    NMClient *nm_client = nm_client_new (NULL, NULL);
    if (!nm_client) {
        g_set_error (error,
                     NM_SECRET_AGENT_ERROR,
                     NM_SECRET_AGENT_ERROR_FAILED,
                     "%s.%d (%s): create nm client failed.",
                     __FILE__, __LINE__, __func__);
         g_warning ("create nm client failed");
         goto l_out;
    }


    info->dialog = nma_wifi_dialog_new_for_secrets (nm_client,
                                                    req->connection,
                                                    req->setting_name,
                                                    (const char *const*) req->hints);

    if (info->dialog) {
        applet_secrets_request_set_free_func (req, free_wifi_info);
        g_signal_connect (info->dialog, "response",
                          G_CALLBACK (get_secrets_dialog_response_cb),
                          info);
        show_ignore_focus_stealing_prevention (info->dialog);
    } else {
        g_set_error (error,
                     NM_SECRET_AGENT_ERROR,
                     NM_SECRET_AGENT_ERROR_FAILED,
                     "%s.%d (%s): couldn't display secrets UI",
                     __FILE__, __LINE__, __func__);
    }

    g_clear_object(&nm_client);

l_out:
    return !!info->dialog;
    //return 0;
}

static SecretsRequest *
applet_secrets_request_new (size_t totsize,
                            NMConnection *connection,
                            gpointer request_id,
                            const char *setting_name,
                            const char **hints,
                            guint32 flags,
                            AppletAgentSecretsCallback callback,
                            gpointer callback_data,
                            AppletAgent *agent)
{
    SecretsRequest *req;

    g_return_val_if_fail (totsize >= sizeof (SecretsRequest), NULL);
    g_return_val_if_fail (connection != NULL, NULL);

    req = g_malloc0 (totsize);
    req->totsize = totsize;
    req->connection = g_object_ref (connection);
    req->reqid = request_id;
    req->setting_name = g_strdup (setting_name);
    req->hints = g_strdupv ((char **) hints);
    req->flags = flags;
    req->callback = callback;
    req->callback_data = callback_data;
    req->agent = agent;
    return req;
}

static void
get_existing_secrets_cb (NMSecretAgentOld *agent,
                         NMConnection *connection,
                         GVariant *secrets,
                         GError *secrets_error,
                         gpointer user_data)
{
    SecretsRequest *req = user_data;
    //NMADeviceClass *dclass;
    GError *error = NULL;

    if (secrets)
        nm_connection_update_secrets (connection, req->setting_name, secrets, NULL);
    else
        nm_connection_clear_secrets (connection);

    /* Let the device class handle secrets */
    if (!wifi_get_secrets(req, &error)) {
        g_warning ("%s:%d - %s", __func__, __LINE__, error ? error->message : "(unknown)");
        applet_secrets_request_complete (req, NULL, error);
        applet_secrets_request_free (req);
        g_error_free (error);
    }
    /* Otherwise success; wait for the secrets callback */
}


static void
applet_agent_get_secrets_cb (AppletAgent *agent,
                             gpointer request_id,
                             NMConnection *connection,
                             const char *setting_name,
                             const char **hints,
                             guint32 flags,
                             AppletAgentSecretsCallback callback,
                             gpointer callback_data,
                             gpointer user_data)
{
    NMSettingConnection *s_con;
    GError *error = NULL;
    SecretsRequest *req = NULL;

    s_con = nm_connection_get_setting_connection (connection);
    g_return_if_fail (s_con != NULL);

    /* VPN secrets get handled a bit differently */
    if (!strcmp (nm_setting_connection_get_connection_type (s_con), NM_SETTING_VPN_SETTING_NAME)) {
        req = applet_secrets_request_new (applet_vpn_request_get_secrets_size (),
                                          connection,
                                          request_id,
                                          setting_name,
                                          hints,
                                          flags,
                                          callback,
                                          callback_data,
                                          agent);
        if (!applet_vpn_request_get_secrets (req, &error))
            goto error;

        secrets_reqs = g_slist_prepend (secrets_reqs, req);
        return;
    }


    req = applet_secrets_request_new (sizeof (NMWifiInfo),
                                      connection,
                                      request_id,
                                      setting_name,
                                      hints,
                                      flags,
                                      callback,
                                      callback_data,
                                      agent);
    secrets_reqs = g_slist_prepend (secrets_reqs, req);

    /* Get existing secrets, if any */
    nm_secret_agent_old_get_secrets (NM_SECRET_AGENT_OLD (agent),
                                     connection,
                                     setting_name,
                                     hints,
                                     NM_SECRET_AGENT_GET_SECRETS_FLAG_NONE,
                                     get_existing_secrets_cb,
                                     req);
    return;

error:
    g_warning ("%s", error->message);
    callback (agent, NULL, error, callback_data);
    g_error_free (error);

    if (req)
        applet_secrets_request_free (req);
}

static void
applet_agent_cancel_secrets_cb (AppletAgent *agent,
                                 gpointer request_id,
                                 gpointer user_data)
{
    GSList *iter, *next;

    for (iter = secrets_reqs; iter; iter = next) {
        SecretsRequest *req = iter->data;

        next = g_slist_next (iter);

        if (req->reqid == request_id) {
            /* cancel and free this password request */
            applet_secrets_request_free (req);
            break;
        }
    }
}

void agent_init()
{
    if (NULL != lingmoAgent) {
        return;
    }
    GError *error = NULL;
    lingmoAgent = applet_agent_new (&error);

    g_signal_connect (lingmoAgent, APPLET_AGENT_GET_SECRETS,
                           G_CALLBACK (applet_agent_get_secrets_cb), NULL);
    g_signal_connect (lingmoAgent, APPLET_AGENT_CANCEL_SECRETS,
                           G_CALLBACK (applet_agent_cancel_secrets_cb), NULL);


    return;
}

void agent_clear()
{
    if (NULL == lingmoAgent) {
        return;
    }
    g_clear_object (&lingmoAgent);
    lingmoAgent = NULL;
}
