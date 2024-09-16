// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <stdbool.h>
#include <stdio.h>
#include <gio/gio.h>
#define SECRET_WITH_UNSTABLE
#include <libsecret/secret.h>

#define KEYRING_LOGIN "login"
#define PASSWORD_SECRET_VALUE_CONTENT_TYPE "text/plain"
#define COLLECTION_INTERFACE "org.freedesktop.Secret.Collection"

bool is_default_keyring_exists(SecretService *service) {
    GError *err = NULL;
    SecretCollection *collection = secret_collection_for_alias_sync(service,
                                                                    SECRET_COLLECTION_DEFAULT,
                                                                    SECRET_COLLECTION_NONE,
                                                                    NULL,
                                                                    &err);
    if (err != NULL) {
        printf("failed to get default secret collection: %s\n", err->message);
        g_error_free(err);
        return false;
    }
    if (collection == NULL) {
        printf("default secret collection not exists\n");
        return false;
    }

    g_object_unref(collection);
    return true;
}

// CheckLogin check whether the login keyring exists.
// If not, create it and set as default.
int check_login() {
    int res = 0;

    GError *err = NULL;
    SecretService *service = NULL;
    SecretCollection *collection = NULL;
    SecretValue *password = NULL;
    GDBusConnection *bus = NULL;
    GVariant *ret = NULL;

    do {
        service = secret_service_get_sync(SECRET_SERVICE_OPEN_SESSION, NULL, &err);
        if (service == NULL) {
            printf("failed to get secret service: %s\n", err->message);
            res = 1;
            break;
        }

        if (is_default_keyring_exists(service)) {
            break;
        }

        password = secret_value_new("", 0, PASSWORD_SECRET_VALUE_CONTENT_TYPE);

        bus = g_bus_get_sync(G_BUS_TYPE_SESSION, NULL, &err);
        if (bus == NULL) {
            printf("failed to get session bus: %s\n", err->message);
            res = 1;
            break;
        }

        // create new collection without prompt
        GVariant *label = g_variant_new_dict_entry(
                    g_variant_new_string(COLLECTION_INTERFACE ".Label"),
                    g_variant_new_variant(g_variant_new_string(KEYRING_LOGIN)));
        GVariant *attributes = g_variant_new_array(G_VARIANT_TYPE("{sv}"), &label, 1);

        ret = g_dbus_connection_call_sync(
                    bus,
                    "org.gnome.keyring",
                    "/org/freedesktop/secrets",
                    "org.gnome.keyring.InternalUnsupportedGuiltRiddenInterface",
                    "CreateWithMasterPassword",
                    g_variant_new("(@a{sv}@(oayays))",
                                  attributes,
                                  secret_service_encode_dbus_secret(service, password)),
                    NULL,
                    G_DBUS_CALL_FLAGS_NONE,
                    G_MAXINT,
                    NULL,
                    &err);
        if (err != NULL) {
            printf("failed to create keyring: %s\n", err->message);
            res = 1;
            break;
        }
    } while (false);

    if (err != NULL) g_error_free(err);
    if (service != NULL) g_object_unref(service);
    if (collection != NULL) g_object_unref(collection);
    if (password != NULL) secret_value_unref(password);
    if (bus != NULL) g_object_unref(bus);
    if (ret != NULL) g_variant_unref(ret);

    return res;
}

/**
 * @brief 当无法从 PAM 拿到用户密码时（免密码登录、自动登录）， pam_gnome_keyring.so 不会自动创建「登录」钥匙环。
 * 若无默认钥匙环，则自动创建一个无密码的钥匙环。
 */
int main(int argc, char *argv[])
{
    printf("keyring checker init\n");
    check_login();
    printf("keyring checker init finished\n");
    return 0;
}
