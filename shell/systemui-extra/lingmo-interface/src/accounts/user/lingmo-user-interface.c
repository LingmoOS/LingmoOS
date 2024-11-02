/*
 * Copyright (C) 2019 Tianjin LINGMO Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
 *
 */


#include<stdio.h>
#include<glib-2.0/gio/gio.h>
#include<glib-2.0/glib.h>
#include<stdbool.h>
#include<stdlib.h>
#include<string.h>
#include <crypt.h>
#include"lingmo-user-interface.h"

static gchar
salt_char (GRand *rand)
{
    gchar salt[] = "ABCDEFGHIJKLMNOPQRSTUVXYZ"
                   "abcdefghijklmnopqrstuvxyz"
                   "./0123456789";

    return salt[g_rand_int_range (rand, 0, G_N_ELEMENTS (salt))];
}

static gchar *
make_crypted (const char *plain)
{
    GString *salt;
    gchar *result;
    GRand *rand;
    gint i;

    rand = g_rand_new ();
    salt = g_string_sized_new (21);

    /* SHA 256 */
    g_string_append (salt, "$6$");
    for (i = 0; i < 16; i++) {
        g_string_append_c (salt, salt_char (rand));
    }
    g_string_append_c (salt, '$');

    result = g_strdup ((const gchar *)crypt(plain, salt->str));

    g_string_free (salt, TRUE);
    g_rand_free (rand);

    return result;
}

//获取用户对象路径
static gchar *getpath(char *username)
{
    GDBusProxy *account_proxy;
    GVariant *result;

    account_proxy = g_dbus_proxy_new_for_bus_sync(G_BUS_TYPE_SYSTEM, G_DBUS_PROXY_FLAGS_NONE,
                                                  NULL, "org.freedesktop.Accounts",
                                                  "/org/freedesktop/Accounts",
                                                  "org.freedesktop.Accounts", NULL, NULL);

    result=g_dbus_proxy_call_sync(account_proxy, "FindUserByName",
                      g_variant_new("(s)", username),
                      G_DBUS_CALL_FLAGS_NONE, -1, NULL, NULL);

    if(result!=NULL){
        gchar *path = (char *)g_variant_get_data(result);

        if (account_proxy){
            g_object_unref(account_proxy);
        }

        return path;
    }
    else{
        return NULL;
    }
}

//添加新用户
void lingmo_accounts_user_createuser(char *username, int accounttype, char *password, char *iconfile)
{
    GError *error = NULL;
    GError *pProxyError = NULL;
    GVariant *result;
    GDBusProxy *account_proxy;

    account_proxy = g_dbus_proxy_new_for_bus_sync(G_BUS_TYPE_SYSTEM,G_DBUS_PROXY_FLAGS_NONE,
                                                  NULL, "org.freedesktop.Accounts",
                                                  "/org/freedesktop/Accounts",
                                                  "org.freedesktop.Accounts", NULL, &error);
    if (error != NULL)
    {
        g_error("Could not connect to org.freedesktop.Accounts:%s\n",error->message);
        if (account_proxy)
            g_object_unref(account_proxy);
        return;
    }

    //设置用户名
    result=g_dbus_proxy_call_sync(account_proxy, "CreateUser",
                      g_variant_new("(ssi)", username, "", accounttype),
                      G_DBUS_CALL_FLAGS_NONE, -1, NULL, &pProxyError);

    if (result != NULL)
    {
       char *crypted = make_crypted(password);

       if(iconfile==NULL)
       iconfile = "/usr/share/pixmaps/faces/stock_person.svg";

       GDBusProxy 	*proxy;
       GError * err = NULL;

       char *path = (char *)g_variant_get_data(result);

       proxy = g_dbus_proxy_new_for_bus_sync(G_BUS_TYPE_SYSTEM, G_DBUS_PROXY_FLAGS_NONE,
                                             NULL, "org.freedesktop.Accounts",
                                             path,
                                             "org.freedesktop.Accounts.User", NULL, &err);

       //设置用户头像
       g_dbus_proxy_call_sync(proxy, "SetIconFile",
                         g_variant_new("(s)", iconfile),
                         G_DBUS_CALL_FLAGS_NONE, -1, NULL, NULL);

       //设置用户密码
       g_dbus_proxy_call_sync(proxy, "SetPassword",
                         g_variant_new("(ss)", crypted, ""),
                         G_DBUS_CALL_FLAGS_NONE, -1, NULL, NULL);

       //设置用户类型
       g_dbus_proxy_call_sync(proxy, "SetAccountType",
                         g_variant_new("(i)", accounttype),
                         G_DBUS_CALL_FLAGS_NONE, -1, NULL, NULL);

       if (proxy){
           g_object_unref(proxy);
       }

    }

    else{
        g_warning("-----------%s had already existed----------",username);
    }

    if (account_proxy){
        g_object_unref(account_proxy);
    }

}

//设置自动登录
void lingmo_accounts_user_autologin(char *username, bool login)
{
    GError * err = NULL;
    gchar *path=getpath(username);

    if(path!=NULL){
        GDBusProxy 	*proxy = g_dbus_proxy_new_for_bus_sync(G_BUS_TYPE_SYSTEM, G_DBUS_PROXY_FLAGS_NONE,
                                              NULL, "org.freedesktop.Accounts",
                                              path,
                                              "org.freedesktop.Accounts.User", NULL, &err);

        g_dbus_proxy_call_sync(proxy, "SetAutomaticLogin",
                          g_variant_new("(b)", login),
                          G_DBUS_CALL_FLAGS_NONE, -1, NULL, NULL);

        if (proxy){
            g_object_unref(proxy);
        }
    }
    else{
        g_warning("-----------%s does not existed----------",username);
    }
}

//更改用户头像
void lingmo_accounts_user_chgusericon(char *username, char *iconfile)
{
    GError * err = NULL;
    gchar *path=getpath(username);

    if(path!=NULL){
        GDBusProxy 	*proxy = g_dbus_proxy_new_for_bus_sync(G_BUS_TYPE_SYSTEM, G_DBUS_PROXY_FLAGS_NONE,
                                              NULL, "org.freedesktop.Accounts",
                                              path,
                                              "org.freedesktop.Accounts.User", NULL, &err);

        g_dbus_proxy_call_sync(proxy, "SetIconFile",
                          g_variant_new("(s)", iconfile),
                          G_DBUS_CALL_FLAGS_NONE, -1, NULL, NULL);

        if (proxy){
            g_object_unref(proxy);
        }
    }
    else{
        g_warning("-----------%s does not existed----------",username);
    }


}

//更改用户密码
void lingmo_accounts_user_chguserpasswd(char *username, char *password)
{
    GError * err = NULL;
    gchar *path=getpath(username);

    if(path!=NULL){
        GDBusProxy 	*proxy = g_dbus_proxy_new_for_bus_sync(G_BUS_TYPE_SYSTEM, G_DBUS_PROXY_FLAGS_NONE,
                                              NULL, "org.freedesktop.Accounts",
                                              path,
                                              "org.freedesktop.Accounts.User", NULL, &err);

        char *crypted = make_crypted(password);

        g_dbus_proxy_call_sync(proxy, "SetPassword",
                          g_variant_new("(ss)", crypted, ""),
                          G_DBUS_CALL_FLAGS_NONE, -1, NULL, NULL);

        if (proxy){
            g_object_unref(proxy);
        }
    }
    else{
        g_warning("-----------%s does not existed----------",username);
    }

}

//更改账户类型
void lingmo_accounts_user_chgusertype(char *username, int accounttype)
{
    GError * err = NULL;
    gchar *path=getpath(username);

    if(path!=NULL){
        GDBusProxy 	*proxy = g_dbus_proxy_new_for_bus_sync(G_BUS_TYPE_SYSTEM, G_DBUS_PROXY_FLAGS_NONE,
                                              NULL, "org.freedesktop.Accounts",
                                              path,
                                              "org.freedesktop.Accounts.User", NULL, &err);

        g_dbus_proxy_call_sync(proxy, "SetAccountType",
                          g_variant_new("(i)", accounttype),
                          G_DBUS_CALL_FLAGS_NONE, -1, NULL, NULL);

        if (proxy){
            g_object_unref(proxy);
        }
    }
    else{
        g_warning("-----------%s does not existed----------",username);
    }

}

//删除用户
void lingmo_accounts_user_deleteuser(char *username, bool removefile)
{
    GDBusProxy *account_proxy;
    GDBusProxy 	*proxy;
    GError * err = NULL;

    gchar *path = getpath(username);

    if(path!=NULL){
        proxy = g_dbus_proxy_new_for_bus_sync(G_BUS_TYPE_SYSTEM, G_DBUS_PROXY_FLAGS_NONE,
                                              NULL, "org.freedesktop.Accounts",
                                              path,
                                              "org.freedesktop.Accounts.User", NULL, &err);

        //获取用户账户的UID
        GVariant *value = g_dbus_proxy_get_cached_property(proxy, "Uid");
        gint uid = (gint)g_variant_get_uint64(value);

        g_warning("-----------%d----------",uid);

        account_proxy = g_dbus_proxy_new_for_bus_sync(G_BUS_TYPE_SYSTEM, G_DBUS_PROXY_FLAGS_NONE,
                                                      NULL, "org.freedesktop.Accounts",
                                                      "/org/freedesktop/Accounts",
                                                      "org.freedesktop.Accounts", NULL, NULL);

        //删除用户账户
        g_dbus_proxy_call_sync(account_proxy, "DeleteUser",
                               g_variant_new("(xb)", uid, removefile),
                               G_DBUS_CALL_FLAGS_NONE, -1, NULL, NULL);

        if (account_proxy){
            g_object_unref(account_proxy);
        }

        if (proxy){
            g_object_unref(proxy);
        }
    }
    else{
        g_warning("-----------%s does not existed----------",username);
    }

}
