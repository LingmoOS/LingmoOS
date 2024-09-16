// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <sys/types.h>
#include <stdio.h>
#include <pwd.h>
#include <unistd.h>

#include <glib.h>
#include <gio/gio.h>

#include <systemd/sd-login.h>

struct user_session_dbus {
    gchar *session_path;
    gchar *seat_path;
    gchar *username;
};

gchar *
get_user_name()
{
    gchar *username = NULL;

    struct passwd *pw = NULL;
    pw = getpwuid(getuid());
    if (pw != NULL) {
        username = g_strdup(pw->pw_name);

    } else {
        g_warning("switchtogreeter:get user name failed\n");
    }

    return username;
}

gchar *
get_seat_path(gchar *username, struct user_session_dbus *usd)
{
    GDBusProxy *display_proxy = NULL;
    GVariant *sessions_prop_var = NULL;
    const gchar **sessions = NULL;
    gsize length = 0;
    GError *error = NULL;

    usd->seat_path = NULL;
    display_proxy = g_dbus_proxy_new_for_bus_sync(G_BUS_TYPE_SYSTEM,
                    G_DBUS_PROXY_FLAGS_NONE,
                    NULL,
                    "org.freedesktop.DisplayManager",
                    "/org/freedesktop/DisplayManager",
                    "org.freedesktop.DisplayManager",
                    NULL,
                    &error);
    if (error != NULL) {
        g_warning("switchtogreeter:lightdm dbus %s\n", error->message);
        g_error_free(error);
        return NULL;
    }
    error = NULL;

    sessions_prop_var = g_dbus_proxy_get_cached_property(display_proxy, "Sessions");
    if (sessions_prop_var == NULL) {
        g_warning("switchtogreeter:get sessions list failed\n");
        g_object_unref(display_proxy);
        return NULL;
    }
    g_object_unref(display_proxy);

    sessions = g_variant_get_objv(sessions_prop_var, &length);
    if (sessions == NULL) {
        g_warning("switchtogreeter:parse sessions list failed\n");
        g_variant_unref(sessions_prop_var);
        return NULL;
    }

    for (guint i = 0; i < length; ++i) {

        GDBusProxy *session_proxy = NULL;
        GVariant *username_prop_var = NULL;
        gchar *user_name = NULL;

        session_proxy = g_dbus_proxy_new_for_bus_sync(G_BUS_TYPE_SYSTEM,
                        G_DBUS_PROXY_FLAGS_NONE,
                        NULL,
                        "org.freedesktop.DisplayManager",
                        sessions[i],
                        "org.freedesktop.DisplayManager.Session",
                        NULL,
                        &error);
        if (error != NULL) {
            g_warning("switchtogreeter:lightdm session dbus %s\n", error->message);
            g_error_free(error);
            continue;
        }
        error = NULL;

        username_prop_var = g_dbus_proxy_get_cached_property(session_proxy, "UserName");
        if (username_prop_var == NULL) {
            if (error != NULL) {
                g_warning("switchtogreeter:get username %s\n", error->message);
                g_error_free(error);
            }
            g_object_unref(session_proxy);
            continue;
        }

        user_name = g_variant_dup_string(username_prop_var, NULL);
        if (user_name == NULL) {
            g_warning("switchtogreeter:get username from session failed\n");
            g_object_unref(session_proxy);
            g_variant_unref(username_prop_var);
            continue;
        }

        if (g_strcmp0(username, user_name) == 0) {
            GVariant *seat_prop_var = NULL;
            gsize seat_path_length = 0;

            seat_prop_var = g_dbus_proxy_get_cached_property(session_proxy, "Seat");
            if (seat_prop_var != NULL) {
                usd->seat_path  = g_variant_dup_string(seat_prop_var, &seat_path_length);
                usd->session_path = g_strdup_printf("%s", sessions[i]);
                usd->username = username;
            } else {
                g_warning("switchtogreeter:get seat path failed for user %s\n", username);
            }
        } else {
            continue;
        }

        g_free(user_name);
        g_variant_unref(username_prop_var);
        g_object_unref(session_proxy);
    }

    g_variant_unref(sessions_prop_var);
    g_free(sessions);

    return usd->seat_path;
}

static uid_t name_to_uid(char const *name)
{
    if (!name) {
        return (unsigned int)-1;
    }

    struct passwd *pwbufp = getpwnam(name);

    return pwbufp ? pwbufp->pw_uid : (unsigned int)-1;
}

// ret:
// 0: change to another session
// 1: stay current session
// tips: 出现异常的时候去/run/systemd/users 和 /run/systemd/sessions 中查看文件的内容是否正确
int
switch_to_greeter(gchar *seat_path, struct user_session_dbus *usd)
{
    int ret = 1;
    char **sessions = NULL;
    char *seat = NULL;
    char *display = NULL;
    char *cur_seat = NULL;
    gchar **strlist = NULL;
    char *new_user_session = NULL;
    GDBusProxy *seat_proxy = NULL;
    GDBusProxy *login1_proxy = NULL;
    GError *error = NULL;
    char *user_display = NULL;

    if (!g_variant_is_object_path(seat_path)) {
        g_warning("switchtogreeter:invalid object path\n");
        return 1;
    }

    seat_proxy = g_dbus_proxy_new_for_bus_sync(G_BUS_TYPE_SYSTEM,
                 G_DBUS_PROXY_FLAGS_NONE,
                 NULL,
                 "org.freedesktop.DisplayManager",
                 seat_path,
                 "org.freedesktop.DisplayManager.Seat",
                 NULL,
                 &error);

    if (error != NULL) {
        g_warning("switchtogreeter:seat proxy %s\n", error->message);
        g_error_free(error);
        return 1;
    }
    error = NULL;

    printf("current seat path: %s\n", seat_path);
    strlist = g_strsplit(seat_path, "/", -1);
    while (*strlist) {
        cur_seat = *strlist;
        strlist++;
    }

    printf("current seat: %s\n", cur_seat);
    printf("usd->session_path: %s\n", usd->session_path);
    printf("usd->username: %s\n", usd->username);

    if (usd->session_path != NULL && usd->username != NULL) {
        GDBusProxy *login1_session_proxy = NULL;
        GVariant *type_prop_var = NULL;
        gchar *type = NULL;
        char objpath[256];

        sd_uid_get_sessions(name_to_uid(usd->username), 0, &sessions);
        sd_uid_get_display(name_to_uid(usd->username), &user_display);
        printf("user display: %s\n", user_display);
        printf("user sessions: %s\n", *sessions);
        while (*sessions) {
            sd_session_get_display(*sessions, &display);
            if (getenv("WAYLAND_DISPLAY") != NULL) {
                memset(objpath, 0, 256 * sizeof(char));
                sprintf(objpath, "/org/freedesktop/login1/session/_3%s", *sessions);
                login1_session_proxy = g_dbus_proxy_new_for_bus_sync(G_BUS_TYPE_SYSTEM,
                                                                     G_DBUS_PROXY_FLAGS_NONE,
                                                                     NULL,
                                                                     "org.freedesktop.login1",
                                                                     objpath,
                                                                     "org.freedesktop.login1.Session",
                                                                     NULL,
                                                                     &error);

                if (error != NULL) {
                    g_warning("switchtogreeter:login1_session_proxy %s\n", error->message);
                    g_error_free(error);
                    g_object_unref(login1_session_proxy);
                    return 1;
                }

                error = NULL;

                type_prop_var = g_dbus_proxy_get_cached_property(login1_session_proxy, "Type");
                if (type_prop_var == NULL) {
                    g_variant_unref(type_prop_var);
                    g_object_unref(login1_session_proxy);
                    return 1;
                }

                type = g_variant_dup_string(type_prop_var, NULL);

                if (NULL == display && strcmp(type, "wayland") != 0){
                    sessions++;
                    g_free(type);
                    type = NULL;
                    g_variant_unref(type_prop_var);
                    g_object_unref(login1_session_proxy);
                    continue;
                }
            } else {
                printf("X session display %s %s\n", *sessions, display);
                if (NULL == display){
                    if (*(sessions + 1) || !user_display) {
                        sessions++;
                        continue;
                    }

                    // 如果是最后一个session,此时已经异常了肯定无法切换用户, 那么使用从user文件中获取的DISPLAY数据
                    printf("Use `DISPLAY` info from user file\n");
                    display = user_display;
                }
            }

            sd_session_get_seat(*sessions, &seat);
            printf("Seat: %s, session: %s, display: %s\n", seat, *sessions, display);
            if (seat != NULL && (0 == g_ascii_strcasecmp(seat, cur_seat))) {
                new_user_session = *sessions;
                printf("switchtogreeter: find sessions path %s %s \n", new_user_session, seat);
                break;
            }
            sessions++;
        }

        if (getenv("WAYLAND_DISPLAY") != NULL) {
            if (type)
                g_free(type);
            g_variant_unref(type_prop_var);
            g_object_unref(login1_session_proxy);
        }

        if (NULL == new_user_session) {
            g_error("switchtogreeter: can not find user sessions %s \n", usd->username);
            return 1;
        }
        printf("find user session: %s\n", new_user_session);

        login1_proxy = g_dbus_proxy_new_for_bus_sync(G_BUS_TYPE_SYSTEM,
                       G_DBUS_PROXY_FLAGS_NONE,
                       NULL,
                       "org.freedesktop.login1",
                       "/org/freedesktop/login1",
                       "org.freedesktop.login1.Manager",
                       NULL,
                       &error);
        g_dbus_proxy_call_sync(login1_proxy,
                               "ActivateSession",
                               g_variant_new("(s)", new_user_session),
                               G_DBUS_CALL_FLAGS_NONE,
                               -1,
                               NULL,
                               &error);
        ret = 0;
    } else if (usd->username != NULL) {
        printf("SwitchToUser\n");
        usd->session_path = "";
        g_dbus_proxy_call_sync(seat_proxy,
                               "SwitchToUser",
                               g_variant_new("(ss)", usd->username, usd->session_path),
                               G_DBUS_CALL_FLAGS_NONE,
                               -1,
                               NULL,
                               &error);
        ret = 1;
    } else {
        g_dbus_proxy_call_sync(seat_proxy,
                               "SwitchToGreeter",
                               g_variant_new("()"),
                               G_DBUS_CALL_FLAGS_NONE,
                               -1,
                               NULL,
                               &error);
        ret = 1;
    }

    if (error != NULL) {
        g_warning("switchtogreeter:seat switchtogreeter %s\n", error->message);
        g_error_free(error);
    }
    error = NULL;

    g_object_unref(seat_proxy);
    return ret;
}

int main(int argc G_GNUC_UNUSED, char **argv G_GNUC_UNUSED)
{
    gchar *current_username = NULL;
    gchar *current_seat_path = NULL;
    gchar *new_username = NULL;
    struct user_session_dbus current_user;
    struct user_session_dbus new_user;
    int ret = 1;

    new_user.seat_path = NULL;
    new_user.session_path = NULL;

    current_username = get_user_name();
    if (current_username == NULL) {
        g_warning("switch to greeter:username is NULL\n");
        return 1;
    }

    current_user.username = current_username;
    get_seat_path(current_username, &current_user);


    if (argc == 2) {
        new_username = argv[1];
    }

    new_user.username = new_username;
    get_seat_path(new_user.username, &new_user);
    if (new_user.seat_path == NULL) {
        g_info("switch to greeter:seat path is NULL\n");
    }

    if (current_user.seat_path != NULL) {
        current_seat_path = current_user.seat_path;
    }
    if (new_user.seat_path != NULL) {
        current_seat_path = new_user.seat_path;
    }
    if (current_seat_path == NULL) {
        g_error("switch to greeter:current user seat path is NULL\n");
        g_free(current_username);
        return 1;
    }

    ret = switch_to_greeter(current_seat_path, &new_user);

    g_free(current_username);
    g_free(current_seat_path);

    return ret;
}
