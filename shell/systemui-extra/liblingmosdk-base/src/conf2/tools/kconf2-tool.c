/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: tianshaoshuai <tianshaoshuai@kylinos.cn>
 *
 */

#include "../api/libkysettings.h"
#include "../api/ksettingsschema.h"

#include <stdio.h>

void on_key_changed(KSettings *ksettigns, const char *key, void *user_data)
{
    char *value = kdk_conf2_get_value(ksettigns, key);
    printf("%s changed to %s\n", key, value);
    free(value);
}

void on_reload(KSettings *ksettigns, void *user_data)
{
    printf("view reload\n");
}

/*句柄结构体内部元素*/
typedef struct _KSettingsPrivate
{
    GMainContext *main_context;
    KSettingsSchema *schema;
    GDBusConnection *conn;
    guint sub_id[2];
} KSettingsPrivate;

struct _KSettings
{
    GObject parent_instance;
    KSettingsPrivate *priv;
};

static void print_help()
{
    printf("Usage:\n"
           "    kconf2 COMMAND [ARGS...]\n"
           "COMMAND:\n"
           "    help                  Show this information\n"
           "    list-keys             List keys in a schema\n"
           "    list-children         List children of a schema\n"
           "    list-recursively      List keys and values, recursively\n"
           "    range                 Queries the range of a key\n"
           "    describe              Queries the description of a key\n"
           "    get                   Get the value of a key\n"
           "    set                   Set the value of a key\n"
           "    reset                 Reset the value of a key\n"
           //    "   reset-recursively     Reset all values in a given schema\n"
           "    writable              Check if a key is writable\n"
           "    monitor               Watch for changes\n"
           //    "    monitor-reload        Watch for realod\n"
           "    reload                Reload configuration file, retain user configuration"
           "    save                  Save User config in target folder"
           "\n"
           "Use kconf2 help \"COMMAND\" to get detailed help.\n");
}

static void print_command(char *command)
{
    if (0 == strcmp(command, "help"))
    {
        printf("Usage:\n"
               "    kconf2 help COMMAND\n"
               "\n"
               "Print help\n"
               "\n"
               "Args\n"
               "    COMMAND     Commands to be explained\n");
    }
    else if (0 == strcmp(command, "list-keys"))
    {
        printf("Usage:\n"
               "    kconf2 list-keys SCHEMA[:VERSION]\n"
               "\n"
               "Lists the keys in the schema\n"
               "\n"
               "Args\n"
               "    SCHEMA      The name of the schema\n"
               "    VERSION     Specify the version of the schema\n");
    }
    else if (0 == strcmp(command, "list-children"))
    {
        printf("Usage:\n"
               "    kconf2 list-children SCHEMA[:VERSION]\n"
               "\n"
               "Lists the child objects in the schema\n"
               "\n"
               "Args\n"
               "    SCHEMA      The name of the schema\n"
               "    VERSION     Specify the version of the schema\n");
    }
    else if (0 == strcmp(command, "list-recursively"))
    {
        printf("Usage:\n"
               "    kconf2 list-recursively SCHEMA[:VERSION]\n"
               "\n"
               "Keys and values are listed recursively\n"
               "IF no SCHEMA is given, list all keys\n"
               "\n"
               "Args\n"
               "    SCHEMA      The name of the schema\n"
               "    VERSION     Specify the version of the schema\n");
    }
    else if (0 == strcmp(command, "range"))
    {
        printf("Usage:\n"
               "    kconf2 range SCHEMA[:VERSION] KEY\n"
               "\n"
               "Query the range of valid values for the key\n"
               "\n"
               "Args\n"
               "    SCHEMA      The name of the schema\n"
               "    VERSION     Specify the version of the schema\n"
               "    KEY         Key in the schema\n");
    }
    else if (0 == strcmp(command, "describe"))
    {
        printf("Usage:\n"
               "    kconf2 describe SCHEMA[:VERSION] KEY\n"
               "\n"
               "Query the description of the key\n"
               "\n"
               "Args\n"
               "    SCHEMA      The name of the schema\n"
               "    VERSION     Specify the version of the schema\n"
               "    KEY         Key in the schema\n");
    }
    else if (0 == strcmp(command, "get"))
    {
        printf("Usage:\n"
               "    kconf2 get SCHEMA[:VERSION] KEY\n"
               "\n"
               "Query the value of the key\n"
               "\n"
               "Args\n"
               "    SCHEMA      The name of the schema\n"
               "    VERSION     Specify the version of the schema\n"
               "    KEY         Key in the schema\n");
    }
    else if (0 == strcmp(command, "set"))
    {
        printf("Usage:\n"
               "    kconf2 reset SCHEMA[:VERSION] KEY VALUE\n"
               "\n"
               "Set the value of the key\n"
               "\n"
               "Args\n"
               "    SCHEMA      The name of the schema\n"
               "    VERSION     Specify the version of the schema\n"
               "    KEY         Key in the schema\n"
               "    VALUE       The value to set\n");
    }
    else if (0 == strcmp(command, "reset"))
    {
        printf("Usage:\n"
               "    kconf2 reset SCHEMA[:VERSION] KEY\n"
               "\n"
               "Reset the value of the key\n"
               "\n"
               "Args\n"
               "    SCHEMA      The name of the schema\n"
               "    VERSION     Specify the version of the schema\n"
               "    KEY         Key in the schema\n");
    }
    // else if (0 == strcmp(command, "reset-recursively"))
    // {
    //     printf("Usage:\n"
    //            "    kconf2 reset-recursively SCHEMA[:VERSION]\n"
    //            "\n"
    //            "Reset all keys in the schema to default\n"
    //            "\n"
    //            "Args\n"
    //            "    SCHEMA      The name of the schema\n"
    //            "    VERSION     Specify the version of the schema\n");
    // }
    else if (0 == strcmp(command, "writable"))
    {
        printf("Usage:\n"
               "    kconf2 writable SCHEMA[:VERSION] KEY\n"
               "\n"
               "Reset the value of the key\n"
               "\n"
               "Args\n"
               "    SCHEMA      The name of the schema\n"
               "    VERSION     Specify the version of the schema\n"
               "    KEY         Key in the schema\n");
    }
    else if (0 == strcmp(command, "monitor"))
    {
        printf("Usage:\n"
               "    kconf2 monitor SCHEMA[:VERSION] [KEY]\n"
               "\n"
               "Monitor changes to the KEY.\n"
               "If no KEY is specified, all keys in the schema are monitored.\n"
               "Use ^C to stop monitoring.\n"
               "\n"
               "Args\n"
               "    SCHEMA      The name of the schema\n"
               "    VERSION     Specify the version of the schema\n"
               "    KEY         Key in the schema\n");
    }
    // else if (0 == strcmp(command, "monitor-reload"))
    // {
    //     printf("Usage:\n"
    //            "    kconf2 monitor-reload\n"
    //            "\n"
    //            "Monitor reload to the view.\n"
    //            "Use ^C to stop monitoring.\n"
    //            "\n"
    //            "Args\n"
    //            "    SCHEMA      The name of the schema\n");
    // }
    else if (0 == strcmp(command, "list-schemas"))
    {
        printf("Usage:\n"
               "    kconf2 list-scemas APP [VERSION]\n"
               "\n"
               "If you don't specify an app and version, list all installed schemas.\n"
               "otherwise list schemas items that specify the app and version.\n"
               "\n"
               "Args\n"
               "    APP      The name of the application\n"
               "    VERSION  Specify the version of the schemas\n");
    }
    else if (0 == strcmp(command, "save"))
    {
        printf("Usage:\n"
               "    kconf2 save PATH\n"
               "\n"
               "Store the user configuration as a configuration file in the destination folder\n"
               "\n"
               "Args\n"
               "    PATH      The destination folder for the city defense profile\n");
    }
    else
        print_help();
}

int main(int argc, char *argv[])
{
    if (argc < 2 || 0 == strcmp(argv[1], "help"))
    {
        if (2 >= argc)
            print_help();
        else
            print_command(argv[2]);
        exit(0);
    }

    char *p = NULL;
    char *id = NULL;
    char *version = NULL;

    if (argc > 2)
    {
        KSettings *ks = kdk_conf2_new(argv[2], NULL);
        if (NULL == ks)
        {
            p = argv[2];
            while (*p != ':' && *p != '\0')
                p++;
            version = *p ? p + 1 : NULL;
            if (':' == *p)
                *p = '\0';
        }
        id = argv[2];
    }

    if (0 == strcmp(argv[1], "list-schemas"))
    {
        // if (argc < 3)
        // {
        //     print_command("list-schemas");
        //     exit(0);
        // }
        gboolean limit_app = FALSE, limit_version = FALSE;
        if (argc > 2)
            limit_app = TRUE;
        if (argc > 3)
            limit_version = TRUE;

        if (NULL == schemas_table)
            kdk_conf2_schema_update_schemas_table();
        if (NULL == schemas_table)
        {
            printf("error\n");
            exit(0);
        }

        AppData *app = NULL;
        GHashTable *app_hash = NULL;
        GHashTableIter iter;
        g_hash_table_iter_init(&iter, schemas_table);
        while (g_hash_table_iter_next(&iter, (gpointer *)&app, (gpointer *)&app_hash))
        {
            if (limit_app && 0 != strcmp(argv[2], app->name))
                continue;
            printf("%s:\n", app->name);

            VersionData *version = NULL;
            GHashTableIter ver_iter;
            GHashTable *version_hash;
            g_hash_table_iter_init(&ver_iter, app_hash);
            while (g_hash_table_iter_next(&ver_iter, (gpointer *)&version, (gpointer *)&version_hash))
            {
                if (limit_version && 0 != strcmp(argv[3], version->name))
                    continue;
                printf("\t%s:\n", version->name);
                GList *schemas = g_hash_table_get_keys(version_hash);
                for (GList *tmp = schemas; tmp; tmp = tmp->next)
                {
                    printf("\t\t%s\n", (char *)tmp->data);
                }
                g_list_free(schemas);
            }
        }
    }
    else if (0 == strcmp(argv[1], "list-children"))
    {
        if (argc < 3)
        {
            print_command("list-children");
            exit(0);
        }
        KSettings *ksettings = kdk_conf2_new(id, version);
        if (NULL == ksettings)
        {
            printf("Wrong schema id\n");
            exit(0);
        }

        char **children = kdk_conf2_list_children(ksettings);
        if (NULL == children)
        {
            printf("No child in schema %s or get children failed\n", argv[2]);
            exit(0);
        }
        for (int i = 0; children[i]; i++)
        {
            printf("%s.%s\n", argv[2], children[i]);
        }
    }
    else if (0 == strcmp(argv[1], "list-keys"))
    {
        if (argc < 3)
        {
            print_command("list-keys");
            exit(0);
        }

        KSettings *ksettings = kdk_conf2_new(id, version);
        if (NULL == ksettings)
        {
            printf("Wrong schema id\n");
            exit(0);
        }

        char **keys = kdk_conf2_list_keys(ksettings);
        if (NULL == keys)
        {
            printf("No key in id %s or get keys failed\n", argv[2]);
            exit(0);
        }
        for (int i = 0; keys[i]; i++)
        {
            printf("%s\n", keys[i]);
        }
    }
    else if (0 == strcmp(argv[1], "list-recursively"))
    {
        if (argc < 3)
        {
            print_command("list-recursively");
            exit(0);
        }
        KSettings *ksettings = kdk_conf2_new(id, version);
        if (NULL == ksettings)
        {
            printf("Wrong schema id\n");
            exit(0);
        }

        char **keys = kdk_conf2_list_keys(ksettings);
        if (NULL == keys)
        {
            printf("No key in id %s or get keys failed\n", argv[2]);
            return 0;
        }
        for (int i = 0; keys[i]; i++)
        {
            char *value = kdk_conf2_get_value(ksettings, keys[i]);
            printf("%s:\t%s\n", keys[i], value);
        }
    }
    else if (0 == strcmp(argv[1], "get"))
    {
        if (argc < 4)
        {
            print_command("get");
            exit(0);
        }

        KSettings *ksettings = kdk_conf2_new(id, version);
        if (NULL == ksettings)
        {
            printf("Wrong schema id\n");
            exit(0);
        }

        char *value = kdk_conf2_get_value(ksettings, argv[argc - 1]);
        if (NULL == value)
        {
            printf("Wrong key\n");
            exit(0);
        }
        printf("%s\n", value);
    }
    else if (0 == strcmp(argv[1], "set"))
    {
        if (argc < 5)
        {
            print_command("set");
            exit(0);
        }

        KSettings *ksettings = kdk_conf2_new(id, version);
        if (NULL == ksettings)
        {
            printf("Wrong schema id\n");
            exit(0);
        }

        int success = kdk_conf2_set_value(ksettings, argv[argc - 2], argv[argc - 1]);
        if (FALSE == success)
            printf("Set value failed\n");
    }
    else if (0 == strcmp(argv[1], "range"))
    {
        if (argc < 4)
        {
            print_command("range");
            exit(0);
        }

        KSettings *ksettings = kdk_conf2_new(id, version);
        if (NULL == ksettings)
        {
            printf("Wrong schema id\n");
            exit(0);
        }

        GHashTable *key = kdk_conf2_schema_get_key(ksettings->priv->schema, argv[argc - 1]);
        if (NULL == key)
        {
            printf("Wrong key\n");
            exit(0);
        }

        char *type = kdk_conf2_schema_key_value_type(key);
        char *range = kdk_conf2_schema_key_get_range(key);
        if (range)
        {
            if (0 == strcmp(type, "enum"))
            {
                const gchar *endptr;
                GError *error;
                GVariant *variant = g_variant_parse(G_VARIANT_TYPE("a{si}"), range, NULL, &endptr, &error);
                if (NULL == variant)
                {
                    printf("parser enum error %s\n", error->message);
                    exit(0);
                }

                printf("enum:\n");

                char *nick = NULL;
                gint32 value = 0;
                GVariantIter *iter = NULL;
                g_variant_get(variant, "a{si}", &iter);
                while (g_variant_iter_loop(iter, "{si}", &nick, &value))
                {
                    printf("  '%s'\n", nick);
                }
                g_variant_iter_free(iter);
                g_variant_unref(variant);
            }
            else
            {
                p = range;
                while (*p != ',' && *p != '\0')
                    p++;
                *p = '\0';
                printf("range %s %s %s\n", type, range, p + 1);
            }
        }
        else
            printf("type %s\n", type);
    }
    else if (0 == strcmp(argv[1], "reset"))
    {
        if (argc < 4)
        {
            print_command("reset");
            exit(0);
        }
        KSettings *ksettings = kdk_conf2_new(id, version);
        if (NULL == ksettings)
        {
            printf("Wrong schema id\n");
            exit(0);
        }
        if (NULL == kdk_conf2_schema_get_key(ksettings->priv->schema, argv[argc - 1]))
        {
            printf("Wrong key\n");
            exit(0);
        }

        kdk_conf2_reset(ksettings, argv[argc - 1]);
    }
    else if (0 == strcmp(argv[1], "monitor"))
    {
        if (argc < 3)
        {
            print_command("monitor");
            exit(0);
        }

        KSettings *ksettings = kdk_conf2_new(id, version);
        if (NULL == ksettings)
        {
            printf("Wrong schema id\n");
            exit(0);
        }

        guint id = 0;
        if (4 == argc)
        {
            char *signal_name = g_strdup_printf("changed::%s", argv[3]);
            id = kdk_conf2_connect_signal(ksettings, signal_name, (KCallBack)on_key_changed, NULL);
        }
        else
            id = kdk_conf2_connect_signal(ksettings, "changed", (KCallBack)on_key_changed, NULL);

        if (id <= 0)
        {
            printf("Connect signal failed\n");
            exit(0);
        }

        for (;;)
            g_main_context_iteration(NULL, TRUE);
    }
    else if (0 == strcmp(argv[1], "monitor-reload"))
    {
        if (argc < 2)
        {
            print_command("monitor-reload");
            exit(0);
        }

        KSettings *ksettings = kdk_conf2_new("test.notice", NULL);
        if (NULL == ksettings)
        {
            printf("Wrong schema id\n");
            exit(0);
        }

        guint id = 0;
        id = kdk_conf2_connect_signal(ksettings, "reload", (KCallBack)on_reload, NULL);
        if (id <= 0)
        {
            printf("Connect signal failed\n");
            exit(0);
        }

        for (;;)
            g_main_context_iteration(NULL, TRUE);
    }
    else if (0 == strcmp(argv[1], "writable"))
    {
        if (argc < 4)
        {
            print_command("writable");
            exit(0);
        }

        KSettings *ksettings = kdk_conf2_new(id, version);
        if (NULL == ksettings)
        {
            printf("Wrong schema id\n");
            exit(0);
        }

        printf("%s\n", kdk_conf2_is_writable(ksettings, argv[argc - 1]) ? "TRUE" : "FALSE");
    }
    else if (0 == strcmp(argv[1], "update") || 0 == strcmp(argv[1], "reload"))
    {
        kdk_conf2_reload();
    }
    else if (0 == strcmp(argv[1], "describe"))
    {
        if (argc < 4)
        {
            print_command("describe");
            exit(0);
        }

        KSettings *ksettings = kdk_conf2_new(id, version);
        if (NULL == ksettings)
        {
            printf("Wrong schema id\n");
            exit(0);
        }

        printf("%s\n", kdk_conf2_get_descrition(ksettings, argv[argc - 1]));
    }
    else if (0 == strcmp(argv[1], "save"))
    {
        if (argc < 3)
        {
            print_command("save");
            exit(0);
        }
        int success = kdk_conf2_save_user_configure(argv[2]);
        if (0 == success)
            printf("save configure failed\n");
    }
    else
    {
        print_help();
    }
    return 0;
}
