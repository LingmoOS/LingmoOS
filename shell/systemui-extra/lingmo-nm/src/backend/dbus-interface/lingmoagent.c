#include "lingmoagent.h"
#include <string.h>

#define KEYRING_UUID_TAG "connection-uuid"
#define KEYRING_SN_TAG "setting-name"
#define KEYRING_SK_TAG "setting-key"

static const SecretSchema network_manager_secret_schema = {
    "org.freedesktop.NetworkManager.Connection",
    SECRET_SCHEMA_DONT_MATCH_NAME,
    {
        { KEYRING_UUID_TAG, SECRET_SCHEMA_ATTRIBUTE_STRING },
        { KEYRING_SN_TAG, SECRET_SCHEMA_ATTRIBUTE_STRING },
        { KEYRING_SK_TAG, SECRET_SCHEMA_ATTRIBUTE_STRING },
        { NULL, 0 },
    }
};

G_DEFINE_TYPE (AppletAgent, applet_agent, NM_TYPE_SECRET_AGENT_OLD);

#define APPLET_AGENT_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), APPLET_TYPE_AGENT, AppletAgentPrivate))

typedef struct {
    GHashTable *requests;
    gboolean vpn_only;

    gboolean disposed;
} AppletAgentPrivate;

enum {
    GET_SECRETS,
    CANCEL_SECRETS,
    LAST_SIGNAL
};
static guint signals[LAST_SIGNAL] = { 0 };


/*******************************************************/

typedef struct {
    guint id;

    NMSecretAgentOld *agent;
    NMConnection *connection;
    char *path;
    char *setting_name;
    char **hints;
    guint32 flags;
    NMSecretAgentOldGetSecretsFunc get_callback;
    NMSecretAgentOldSaveSecretsFunc save_callback;
    NMSecretAgentOldDeleteSecretsFunc delete_callback;
    gpointer callback_data;

    GCancellable *cancellable;
    gint keyring_calls;
} Request;

static Request *
request_new (NMSecretAgentOld *agent,
             NMConnection *connection,
             const char *connection_path,
             const char *setting_name,
             const char **hints,
             guint32 flags,
             NMSecretAgentOldGetSecretsFunc get_callback,
             NMSecretAgentOldSaveSecretsFunc save_callback,
             NMSecretAgentOldDeleteSecretsFunc delete_callback,
             gpointer callback_data)
{
    static guint32 counter = 1;
    Request *r;

    r = g_slice_new0 (Request);
    r->id = counter++;
    r->agent = agent;
    r->connection = g_object_ref (connection);
    r->path = g_strdup (connection_path);
    r->setting_name = g_strdup (setting_name);
    if (hints)
        r->hints = g_strdupv ((gchar **) hints);
    r->flags = flags;
    r->get_callback = get_callback;
    r->save_callback = save_callback;
    r->delete_callback = delete_callback;
    r->callback_data = callback_data;
    r->cancellable = g_cancellable_new ();
    return r;
}

static void
request_free (Request *r)
{
    if (!g_cancellable_is_cancelled (r->cancellable))
        g_hash_table_remove (APPLET_AGENT_GET_PRIVATE (r->agent)->requests, GUINT_TO_POINTER (r->id));

    /* By the time the request is freed, all keyring calls should be completed */
    g_warn_if_fail (r->keyring_calls == 0);

    g_object_unref (r->connection);
    g_free (r->path);
    g_free (r->setting_name);
    g_strfreev (r->hints);
    g_object_unref (r->cancellable);
    memset (r, 0, sizeof (*r));
    g_slice_free (Request, r);
}

/*******************************************************/

static void
get_save_cb (NMSecretAgentOld *agent,
             NMConnection *connection,
             GError *error,
             gpointer user_data)
{
    /* Ignored */
}

static void
get_secrets_cb (AppletAgent *self,
                GVariant *secrets,
                GError *error,
                gpointer user_data)
{
    Request *r = user_data;

    /* 'secrets' shouldn't be valid if there was an error */
    if (error) {
        g_warn_if_fail (secrets == NULL);
        secrets = NULL;
    }

    if (!g_cancellable_is_cancelled (r->cancellable)) {
        /* Save updated secrets as long as user-interaction was allowed; otherwise
         * we'd be saving secrets we just pulled out of the keyring which is somewhat
         * redundant.
         */
        if (secrets && (r->flags != NM_SECRET_AGENT_GET_SECRETS_FLAG_NONE)) {
            NMConnection *dupl;
            GVariantIter iter;
            const char *setting_name;

            /* Copy the existing connection and update its secrets */
            dupl = nm_simple_connection_new_clone (r->connection);
            g_variant_iter_init (&iter, secrets);
            while (g_variant_iter_next (&iter, "{&s@a{sv}}", (gpointer) &setting_name, NULL))
                nm_connection_update_secrets (dupl, setting_name, secrets, NULL);

            /* And save updated secrets to the keyring */
            nm_secret_agent_old_save_secrets (NM_SECRET_AGENT_OLD (self), dupl, get_save_cb, NULL);
            g_object_unref (dupl);
        }

        r->get_callback (NM_SECRET_AGENT_OLD (r->agent), r->connection, secrets, error, r->callback_data);
    }
    request_free (r);
}

static void
ask_for_secrets (Request *r)
{
    /* Ask the applet to get some secrets for us */
    g_signal_emit (r->agent,
                   signals[GET_SECRETS],
                   0,
                   GUINT_TO_POINTER (r->id),
                   r->connection,
                   r->setting_name,
                   r->hints,
                   r->flags,
                   get_secrets_cb,
                   r);
}

static void
check_always_ask_cb (NMSetting *setting,
                     const char *key,
                     const GValue *value,
                     GParamFlags flags,
                     gpointer user_data)
{
    gboolean *always_ask = user_data;
    NMSettingSecretFlags secret_flags = NM_SETTING_SECRET_FLAG_NONE;

    if (flags & NM_SETTING_PARAM_SECRET) {
        if (nm_setting_get_secret_flags (setting, key, &secret_flags, NULL)) {
            if (secret_flags & NM_SETTING_SECRET_FLAG_NOT_SAVED)
                *always_ask = TRUE;
        }
    }
}

static gboolean
has_always_ask (NMSetting *setting)
{
    gboolean always_ask = FALSE;

    nm_setting_enumerate_values (setting, check_always_ask_cb, &always_ask);
    return always_ask;
}

static gboolean
is_connection_always_ask (NMConnection *connection)
{
    NMSettingConnection *s_con;
    const char *ctype;
    NMSetting *setting;

    /* For the given connection type, check if the secrets for that connection
     * are always-ask or not.
     */
    s_con = nm_connection_get_setting_connection (connection);
    g_assert (s_con);
    ctype = nm_setting_connection_get_connection_type (s_con);

    setting = nm_connection_get_setting_by_name (connection, ctype);
    g_return_val_if_fail (setting != NULL, FALSE);

    if (has_always_ask (setting))
        return TRUE;

    /* Try type-specific settings too; be a bit paranoid and only consider
     * secrets from settings relevant to the connection type.
     */
    if (NM_IS_SETTING_WIRELESS (setting)) {
        setting = nm_connection_get_setting (connection, NM_TYPE_SETTING_WIRELESS_SECURITY);
        if (setting && has_always_ask (setting))
            return TRUE;
        setting = nm_connection_get_setting (connection, NM_TYPE_SETTING_802_1X);
        if (setting && has_always_ask (setting))
            return TRUE;
    } else if (NM_IS_SETTING_WIRED (setting)) {
        setting = nm_connection_get_setting (connection, NM_TYPE_SETTING_PPPOE);
        if (setting && has_always_ask (setting))
            return TRUE;
        setting = nm_connection_get_setting (connection, NM_TYPE_SETTING_802_1X);
        if (setting && has_always_ask (setting))
            return TRUE;
    }

    return FALSE;
}

static void
keyring_find_secrets_cb (GObject *source,
                         GAsyncResult *result,
                         gpointer user_data)
{
    Request *r = user_data;
    GError *error = NULL;
    GError *search_error = NULL;
    const char *connection_id = NULL;
    GVariantBuilder builder_setting, builder_connection;
    GVariant *settings = NULL;
    GList *list = NULL;
    GList *iter;
    gboolean hint_found = FALSE, ask = FALSE;

    r->keyring_calls--;
    if (g_cancellable_is_cancelled (r->cancellable)) {
        /* Callback already called by NM or dispose */
        request_free (r);
        return;
    }

    list = secret_service_search_finish (NULL, result, &search_error);
    connection_id = nm_connection_get_id (r->connection);

    if (g_error_matches (search_error, G_IO_ERROR, G_IO_ERROR_CANCELLED)) {
        error = g_error_new_literal (NM_SECRET_AGENT_ERROR,
                                     NM_SECRET_AGENT_ERROR_USER_CANCELED,
                                     "The secrets request was canceled by the user");
        g_error_free (search_error);
        goto done;
    } else if (   (r->flags & NM_SECRET_AGENT_GET_SECRETS_FLAG_ALLOW_INTERACTION)
               && g_error_matches (search_error, G_DBUS_ERROR, G_DBUS_ERROR_SERVICE_UNKNOWN)) {
        /* If the connection always asks for secrets, tolerate
         * keyring service not being present. */
        g_clear_error (&search_error);
    } else if (search_error) {
        error = g_error_new (NM_SECRET_AGENT_ERROR,
                             NM_SECRET_AGENT_ERROR_FAILED,
                             "%s.%d - failed to read secrets from keyring (%s)",
                             __FILE__, __LINE__, search_error->message);
        g_error_free (search_error);
        goto done;
    }

    /* Only ask if we're allowed to, so that eg a connection editor which
     * requests secrets for its UI, for a connection which doesn't have any
     * secrets yet, doesn't trigger the applet secrets dialog.
     */
    if (   (r->flags & NM_SECRET_AGENT_GET_SECRETS_FLAG_ALLOW_INTERACTION)
        && g_list_length (list) == 0) {
        g_message ("No keyring secrets found for %s/%s; asking user.", connection_id, r->setting_name);
        ask_for_secrets (r);
        return;
    }

    g_variant_builder_init (&builder_setting, NM_VARIANT_TYPE_SETTING);

    /* Extract the secrets from the list of matching keyring items */
    for (iter = list; iter != NULL; iter = g_list_next (iter)) {
        SecretItem *item = iter->data;
        SecretValue *secret;
        const char *key_name;
        GHashTable *attributes;

        secret = secret_item_get_secret (item);
        if (secret) {
            attributes = secret_item_get_attributes (item);
            key_name = g_hash_table_lookup (attributes, KEYRING_SK_TAG);
            if (!key_name) {
                g_hash_table_unref (attributes);
                secret_value_unref (secret);
                continue;
            }

            g_variant_builder_add (&builder_setting, "{sv}", key_name,
                                   g_variant_new_string (secret_value_get (secret, NULL)));

            /* See if this property matches a given hint */
            if (r->hints && r->hints[0]) {
                if (!g_strcmp0 (r->hints[0], key_name) || !g_strcmp0 (r->hints[1], key_name))
                    hint_found = TRUE;
            }

            g_hash_table_unref (attributes);
            secret_value_unref (secret);
            break;
        }
    }

    /* If there were hints, and none of the hints were returned by the keyring,
     * get some new secrets.
     */
    if (r->flags) {
        if (r->hints && r->hints[0] && !hint_found)
            ask = TRUE;
        else if (r->flags & NM_SECRET_AGENT_GET_SECRETS_FLAG_REQUEST_NEW) {
            g_message ("New secrets for %s/%s requested; ask the user", connection_id, r->setting_name);
            ask = TRUE;
        } else if (   (r->flags & NM_SECRET_AGENT_GET_SECRETS_FLAG_ALLOW_INTERACTION)
                   && is_connection_always_ask (r->connection))
            ask = TRUE;
    }

    /* Returned secrets are a{sa{sv}}; this is the outer a{s...} hash that
     * will contain all the individual settings hashes.
     */
    g_variant_builder_init (&builder_connection, NM_VARIANT_TYPE_CONNECTION);
    g_variant_builder_add (&builder_connection, "{sa{sv}}", r->setting_name, &builder_setting);
    settings = g_variant_builder_end (&builder_connection);

done:
    g_list_free_full (list, g_object_unref);
    if (ask) {
        GVariantIter dict_iter;
        const char *setting_name;
        GVariant *setting_dict;

        /* Stuff all the found secrets into the connection for the UI to use */
        g_variant_iter_init (&dict_iter, settings);
        while (g_variant_iter_next (&dict_iter, "{s@a{sv}}", &setting_name, &setting_dict)) {
            nm_connection_update_secrets (r->connection,
                                          setting_name,
                                          setting_dict,
                                          NULL);
            g_variant_unref (setting_dict);
        }

        ask_for_secrets (r);
    } else {
        /* Otherwise send the secrets back to NetworkManager */
        r->get_callback (NM_SECRET_AGENT_OLD (r->agent), r->connection, error ? NULL : settings, error, r->callback_data);
        request_free (r);
    }

    if (settings)
        g_variant_unref (settings);
    g_clear_error (&error);
}

static void
get_secrets (NMSecretAgentOld *agent,
             NMConnection *connection,
             const char *connection_path,
             const char *setting_name,
             const char **hints,
             guint32 flags,
             NMSecretAgentOldGetSecretsFunc callback,
             gpointer callback_data)
{
    AppletAgentPrivate *priv = APPLET_AGENT_GET_PRIVATE (agent);
    Request *r;
    GError *error = NULL;
    NMSettingConnection *s_con;
    NMSetting *setting;
    const char *uuid, *ctype;
    GHashTable *attrs;

    setting = nm_connection_get_setting_by_name (connection, setting_name);
    if (!setting) {
        error = g_error_new (NM_SECRET_AGENT_ERROR,
                             NM_SECRET_AGENT_ERROR_INVALID_CONNECTION,
                             "%s.%d - Connection didn't have requested setting '%s'.",
                             __FILE__, __LINE__, setting_name);
        callback (agent, connection, NULL, error, callback_data);
        g_error_free (error);
        return;
    }

    uuid = nm_connection_get_uuid (connection);

    s_con = nm_connection_get_setting_connection (connection);
    g_assert (s_con);
    ctype = nm_setting_connection_get_connection_type (s_con);

    if (!uuid || !ctype) {
        error = g_error_new (NM_SECRET_AGENT_ERROR,
                             NM_SECRET_AGENT_ERROR_INVALID_CONNECTION,
                             "%s.%d - Connection didn't have required UUID.",
                             __FILE__, __LINE__);
        callback (agent, connection, NULL, error, callback_data);
        g_error_free (error);
        return;
    }

    /* Track the secrets request */
    r = request_new (agent, connection, connection_path, setting_name, hints, flags, callback, NULL, NULL, callback_data);
    g_hash_table_insert (priv->requests, GUINT_TO_POINTER (r->id), r);

    /* VPN passwords are handled by the VPN plugin's auth dialog */
    if (!strcmp (ctype, NM_SETTING_VPN_SETTING_NAME)) {
        ask_for_secrets (r);
        return;
    }

    /* Only handle non-VPN secrets if we're supposed to */
    if (priv->vpn_only == TRUE) {
        error = g_error_new_literal (NM_SECRET_AGENT_ERROR,
                                     NM_SECRET_AGENT_ERROR_NO_SECRETS,
                                     "Only handling VPN secrets at this time.");
        callback (agent, connection, NULL, error, callback_data);
        g_error_free (error);
        return;
    }

    /* For everything else we scrape the keyring for secrets first, and ask
     * later if required.
     */
    attrs = secret_attributes_build (&network_manager_secret_schema,
                                     KEYRING_UUID_TAG, uuid,
                                     KEYRING_SN_TAG, setting_name,
                                     NULL);

    secret_service_search (NULL, &network_manager_secret_schema, attrs,
                           SECRET_SEARCH_ALL | SECRET_SEARCH_UNLOCK | SECRET_SEARCH_LOAD_SECRETS,
                           r->cancellable, keyring_find_secrets_cb, r);

    r->keyring_calls++;
    g_hash_table_unref (attrs);
}

/*******************************************************/

static void
cancel_get_secrets (NMSecretAgentOld *agent,
                    const char *connection_path,
                    const char *setting_name)
{
    AppletAgentPrivate *priv = APPLET_AGENT_GET_PRIVATE (agent);
    GHashTableIter iter;
    Request *r;
    GError *error;

    error = g_error_new_literal (NM_SECRET_AGENT_ERROR,
                                 NM_SECRET_AGENT_ERROR_AGENT_CANCELED,
                                 "Canceled by NetworkManager");

    g_hash_table_iter_init (&iter, priv->requests);
    while (g_hash_table_iter_next (&iter, NULL, (gpointer) &r)) {
        /* Only care about GetSecrets requests here */
        if (r->get_callback == NULL)
            continue;

        /* Cancel any matching GetSecrets call */
        if (   g_strcmp0 (r->path, connection_path) == 0
            && g_strcmp0 (r->setting_name, setting_name) == 0) {
            /* cancel outstanding keyring operations */
            g_cancellable_cancel (r->cancellable);

            r->get_callback (NM_SECRET_AGENT_OLD (r->agent), r->connection, NULL, error, r->callback_data);
            g_hash_table_iter_remove (&iter);
            g_signal_emit (r->agent, signals[CANCEL_SECRETS], 0, GUINT_TO_POINTER (r->id));
        }
    }

    g_error_free (error);
}

/*******************************************************/

static void
save_request_try_complete (Request *r)
{
    /* Only call the SaveSecrets callback and free the request when all the
     * secrets have been saved to the keyring.
     */
    if (r->keyring_calls == 0) {
        if (!g_cancellable_is_cancelled (r->cancellable))
            r->save_callback (NM_SECRET_AGENT_OLD (r->agent), r->connection, NULL, r->callback_data);
        request_free (r);
    }
}

static void
save_secret_cb (GObject *source,
                GAsyncResult *result,
                gpointer user_data)
{
    secret_password_store_finish (result, NULL);
    save_request_try_complete (user_data);
}



static GHashTable *
_create_keyring_add_attr_list (NMConnection *connection,
                               const char *setting_name,
                               const char *setting_key,
                               char **out_display_name)
{
    const char *connection_id, *connection_uuid;

    g_return_val_if_fail (connection != NULL, NULL);
    g_return_val_if_fail (setting_name != NULL, NULL);
    g_return_val_if_fail (setting_key != NULL, NULL);

    connection_uuid = nm_connection_get_uuid (connection);
    g_assert (connection_uuid);
    connection_id = nm_connection_get_id (connection);
    g_assert (connection_id);

    if (out_display_name) {
        *out_display_name = g_strdup_printf ("Network secret for %s/%s/%s",
                                             connection_id,
                                             setting_name,
                                             setting_key);
    }

    return secret_attributes_build (&network_manager_secret_schema,
                                    KEYRING_UUID_TAG, connection_uuid,
                                    KEYRING_SN_TAG, setting_name,
                                    KEYRING_SK_TAG, setting_key,
                                    NULL);
}

static void
save_one_secret (Request *r,
                 NMSetting *setting,
                 const char *key,
                 const char *secret,
                 const char *display_name)
{
    GHashTable *attrs;
    char *alt_display_name = NULL;
    const char *setting_name;
    NMSettingSecretFlags secret_flags = NM_SETTING_SECRET_FLAG_NONE;

    /* Don't system-owned or always-ask secrets */
    if (!nm_setting_get_secret_flags (setting, key, &secret_flags, NULL))
        return;
    if (secret_flags != NM_SETTING_SECRET_FLAG_AGENT_OWNED)
        return;

    setting_name = nm_setting_get_name (setting);
    g_assert (setting_name);

    attrs = _create_keyring_add_attr_list (r->connection,
                                           setting_name,
                                           key,
                                           display_name ? NULL : &alt_display_name);
    g_assert (attrs);

    secret_password_storev (&network_manager_secret_schema, attrs, NULL,
                            display_name ? display_name : alt_display_name, secret,
                            r->cancellable, save_secret_cb, r);
    r->keyring_calls++;

    g_hash_table_unref (attrs);
    g_free (alt_display_name);
}

static void
vpn_secret_iter_cb (const char *key, const char *secret, gpointer user_data)
{
    Request *r = user_data;
    NMSetting *setting;
    const char *service_name, *id;
    char *display_name;

    if (secret && strlen (secret)) {
        setting = nm_connection_get_setting (r->connection, NM_TYPE_SETTING_VPN);
        g_assert (setting);
        service_name = nm_setting_vpn_get_service_type (NM_SETTING_VPN (setting));
        g_assert (service_name);
        id = nm_connection_get_id (r->connection);
        g_assert (id);

        display_name = g_strdup_printf ("VPN %s secret for %s/%s/" NM_SETTING_VPN_SETTING_NAME,
                                        key,
                                        id,
                                        service_name);
        save_one_secret (r, setting, key, secret, display_name);
        g_free (display_name);
    }
}

static void
write_one_secret_to_keyring (NMSetting *setting,
                             const char *key,
                             const GValue *value,
                             GParamFlags flags,
                             gpointer user_data)
{
    Request *r = user_data;
    GType type = G_VALUE_TYPE (value);
    const char *secret;

    /* Non-secrets obviously don't get saved in the keyring */
    if (!(flags & NM_SETTING_PARAM_SECRET))
        return;

    if (NM_IS_SETTING_VPN (setting) && (g_strcmp0 (key, NM_SETTING_VPN_SECRETS) == 0)) {
        g_return_if_fail (type == G_TYPE_HASH_TABLE);

        /* Process VPN secrets specially since it's a hash of secrets, not just one */
        nm_setting_vpn_foreach_secret (NM_SETTING_VPN (setting), vpn_secret_iter_cb, r);
    } else {
        /* FIXME: password-raw is not string */
        if (!g_strcmp0 (key, NM_SETTING_802_1X_PASSWORD_RAW))
            return;
        g_return_if_fail (type == G_TYPE_STRING);
        secret = g_value_get_string (value);
        if (secret && strlen (secret))
            save_one_secret (r, setting, key, secret, NULL);
    }
}

static void
save_delete_cb (NMSecretAgentOld *agent,
                NMConnection *connection,
                GError *error,
                gpointer user_data)
{
    Request *r = user_data;

    /* Ignore errors; now save all new secrets */
    nm_connection_for_each_setting_value (connection, write_one_secret_to_keyring, r);

    /* If no secrets actually got saved there may be nothing to do so
     * try to complete the request here.  If there were secrets to save the
     * request will get completed when those keyring calls return.
     */
    save_request_try_complete (r);
}

static void
save_secrets (NMSecretAgentOld *agent,
              NMConnection *connection,
              const char *connection_path,
              NMSecretAgentOldSaveSecretsFunc callback,
              gpointer callback_data)
{
    AppletAgentPrivate *priv = APPLET_AGENT_GET_PRIVATE (agent);
    Request *r;

    r = request_new (agent, connection, connection_path, NULL, NULL, FALSE, NULL, callback, NULL, callback_data);
    g_hash_table_insert (priv->requests, GUINT_TO_POINTER (r->id), r);

    g_message("save secret ........................");
    /* First delete any existing items in the keyring */
    nm_secret_agent_old_delete_secrets (agent, connection, save_delete_cb, r);
}

/*******************************************************/

static void
delete_find_items_cb (GObject *source,
                      GAsyncResult *result,
                      gpointer user_data)
{
    Request *r = user_data;
    GError *secret_error = NULL;
    GError *error = NULL;

    r->keyring_calls--;
    if (g_cancellable_is_cancelled (r->cancellable)) {
        /* Callback already called by NM or dispose */
        request_free (r);
        return;
    }

    secret_password_clear_finish (result, &secret_error);
    if (secret_error != NULL) {
        error = g_error_new (NM_SECRET_AGENT_ERROR,
                             NM_SECRET_AGENT_ERROR_FAILED,
                             "The request could not be completed (%s)",
                             secret_error->message);
        g_error_free (secret_error);
    }

    r->delete_callback (r->agent, r->connection, error, r->callback_data);
    request_free (r);
}

static void
delete_secrets (NMSecretAgentOld *agent,
                NMConnection *connection,
                const char *connection_path,
                NMSecretAgentOldDeleteSecretsFunc callback,
                gpointer callback_data)
{
    AppletAgentPrivate *priv = APPLET_AGENT_GET_PRIVATE (agent);
    Request *r;
    NMSettingConnection *s_con;
    const char *uuid;

    r = request_new (agent, connection, connection_path, NULL, NULL, FALSE, NULL, NULL, callback, callback_data);
    g_hash_table_insert (priv->requests, GUINT_TO_POINTER (r->id), r);

    s_con = nm_connection_get_setting_connection (connection);
    g_assert (s_con);
    uuid = nm_setting_connection_get_uuid (s_con);
    g_assert (uuid);

    secret_password_clear (&network_manager_secret_schema, r->cancellable,
                           delete_find_items_cb, r,
                           KEYRING_UUID_TAG, uuid,
                           NULL);
    r->keyring_calls++;
}

void
applet_agent_handle_vpn_only (AppletAgent *agent, gboolean vpn_only)
{
    g_return_if_fail (agent != NULL);
    g_return_if_fail (APPLET_IS_AGENT (agent));

    APPLET_AGENT_GET_PRIVATE (agent)->vpn_only = vpn_only;
}

/*******************************************************/

AppletAgent *
applet_agent_new (GError **error)
{
    AppletAgent *agent;

    agent = g_object_new (APPLET_TYPE_AGENT,
                          NM_SECRET_AGENT_OLD_IDENTIFIER, "org.freedesktop.nm-applet",
                          NM_SECRET_AGENT_OLD_CAPABILITIES, NM_SECRET_AGENT_CAPABILITY_VPN_HINTS,
                          NULL);
    if (!g_initable_init (G_INITABLE (agent), NULL, error)) {
        g_object_unref (agent);
        return NULL;
    }

    return agent;
}

static void
applet_agent_init (AppletAgent *self)
{
    AppletAgentPrivate *priv = APPLET_AGENT_GET_PRIVATE (self);

    priv->requests = g_hash_table_new (g_direct_hash, g_direct_equal);
}

static void
dispose (GObject *object)
{
    AppletAgent *self = APPLET_AGENT (object);
    AppletAgentPrivate *priv = APPLET_AGENT_GET_PRIVATE (self);

    if (!priv->disposed) {
        GHashTableIter iter;
        Request *r;

        /* Mark any outstanding requests as canceled */
        g_hash_table_iter_init (&iter, priv->requests);
        while (g_hash_table_iter_next (&iter, NULL, (gpointer) &r))
            g_cancellable_cancel (r->cancellable);

        g_hash_table_destroy (priv->requests);
        priv->disposed = TRUE;
    }

    G_OBJECT_CLASS (applet_agent_parent_class)->dispose (object);
}

static void applet_agent_class_init (AppletAgentClass *agent_class)
{
    g_message("applet_agent_class_init........................");
    GObjectClass *object_class = G_OBJECT_CLASS (agent_class);
    NMSecretAgentOldClass *parent_class = NM_SECRET_AGENT_OLD_CLASS (agent_class);

    g_type_class_add_private (agent_class, sizeof (AppletAgentPrivate));

    /* virtual methods */
    object_class->dispose = dispose;
    parent_class->get_secrets = get_secrets;
    parent_class->cancel_get_secrets = cancel_get_secrets;
    parent_class->save_secrets = save_secrets;
    parent_class->delete_secrets = delete_secrets;

    /* Signals */
    signals[GET_SECRETS] =
        g_signal_new (APPLET_AGENT_GET_SECRETS,
                      G_OBJECT_CLASS_TYPE (object_class),
                      G_SIGNAL_RUN_FIRST,
                      G_STRUCT_OFFSET (AppletAgentClass, get_secrets),
                      NULL, NULL, NULL,
                      G_TYPE_NONE, 7,
                      G_TYPE_POINTER, G_TYPE_POINTER, G_TYPE_STRING, G_TYPE_POINTER, G_TYPE_UINT, G_TYPE_POINTER, G_TYPE_POINTER);

    signals[CANCEL_SECRETS] =
        g_signal_new (APPLET_AGENT_CANCEL_SECRETS,
                      G_OBJECT_CLASS_TYPE (object_class),
                      G_SIGNAL_RUN_FIRST,
                      G_STRUCT_OFFSET (AppletAgentClass, cancel_secrets),
                      NULL, NULL, NULL,
                      G_TYPE_NONE, 1, G_TYPE_POINTER);
}

