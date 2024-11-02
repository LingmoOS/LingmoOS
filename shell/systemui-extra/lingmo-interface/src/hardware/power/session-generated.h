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

#ifndef __SESSION_GENERATED_H__
#define __SESSION_GENERATED_H__

#include <gio/gio.h>

G_BEGIN_DECLS


/* ------------------------------------------------------------------------ */
/* Declarations for cn.lingmoos.session */

#define TYPE_SESSION (session_get_type ())
#define SESSION(o) (G_TYPE_CHECK_INSTANCE_CAST ((o), TYPE_SESSION, Session))
#define IS_SESSION(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), TYPE_SESSION))
#define SESSION_GET_IFACE(o) (G_TYPE_INSTANCE_GET_INTERFACE ((o), TYPE_SESSION, SessionIface))

struct _Session;
typedef struct _Session Session;
typedef struct _SessionIface SessionIface;

struct _SessionIface
{
  GTypeInterface parent_iface;

  gboolean (*handle_transfer_int_value) (
    Session *object,
    GDBusMethodInvocation *invocation,
    GVariant *arg_InArg);

};

GType session_get_type (void) G_GNUC_CONST;

GDBusInterfaceInfo *session_interface_info (void);
guint session_override_properties (GObjectClass *klass, guint property_id_begin);


/* D-Bus method call completion functions: */
void session_complete_transfer_int_value (
    Session *object,
    GDBusMethodInvocation *invocation);



/* D-Bus method calls: */
void session_call_transfer_int_value (
    Session *proxy,
    GVariant *arg_InArg,
    GCancellable *cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data);

gboolean session_call_transfer_int_value_finish (
    Session *proxy,
    GAsyncResult *res,
    GError **error);

gboolean session_call_transfer_int_value_sync (
    Session *proxy,
    GVariant *arg_InArg,
    GCancellable *cancellable,
    GError **error);



/* ---- */

#define TYPE_SESSION_PROXY (session_proxy_get_type ())
#define SESSION_PROXY(o) (G_TYPE_CHECK_INSTANCE_CAST ((o), TYPE_SESSION_PROXY, SessionProxy))
#define SESSION_PROXY_CLASS(k) (G_TYPE_CHECK_CLASS_CAST ((k), TYPE_SESSION_PROXY, SessionProxyClass))
#define SESSION_PROXY_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), TYPE_SESSION_PROXY, SessionProxyClass))
#define IS_SESSION_PROXY(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), TYPE_SESSION_PROXY))
#define IS_SESSION_PROXY_CLASS(k) (G_TYPE_CHECK_CLASS_TYPE ((k), TYPE_SESSION_PROXY))

typedef struct _SessionProxy SessionProxy;
typedef struct _SessionProxyClass SessionProxyClass;
typedef struct _SessionProxyPrivate SessionProxyPrivate;

struct _SessionProxy
{
  /*< private >*/
  GDBusProxy parent_instance;
  SessionProxyPrivate *priv;
};

struct _SessionProxyClass
{
  GDBusProxyClass parent_class;
};

GType session_proxy_get_type (void) G_GNUC_CONST;

#if GLIB_CHECK_VERSION(2, 44, 0)
G_DEFINE_AUTOPTR_CLEANUP_FUNC (SessionProxy, g_object_unref)
#endif

void session_proxy_new (
    GDBusConnection     *connection,
    GDBusProxyFlags      flags,
    const gchar         *name,
    const gchar         *object_path,
    GCancellable        *cancellable,
    GAsyncReadyCallback  callback,
    gpointer             user_data);
Session *session_proxy_new_finish (
    GAsyncResult        *res,
    GError             **error);
Session *session_proxy_new_sync (
    GDBusConnection     *connection,
    GDBusProxyFlags      flags,
    const gchar         *name,
    const gchar         *object_path,
    GCancellable        *cancellable,
    GError             **error);

void session_proxy_new_for_bus (
    GBusType             bus_type,
    GDBusProxyFlags      flags,
    const gchar         *name,
    const gchar         *object_path,
    GCancellable        *cancellable,
    GAsyncReadyCallback  callback,
    gpointer             user_data);
Session *session_proxy_new_for_bus_finish (
    GAsyncResult        *res,
    GError             **error);
Session *session_proxy_new_for_bus_sync (
    GBusType             bus_type,
    GDBusProxyFlags      flags,
    const gchar         *name,
    const gchar         *object_path,
    GCancellable        *cancellable,
    GError             **error);


/* ---- */

#define TYPE_SESSION_SKELETON (session_skeleton_get_type ())
#define SESSION_SKELETON(o) (G_TYPE_CHECK_INSTANCE_CAST ((o), TYPE_SESSION_SKELETON, SessionSkeleton))
#define SESSION_SKELETON_CLASS(k) (G_TYPE_CHECK_CLASS_CAST ((k), TYPE_SESSION_SKELETON, SessionSkeletonClass))
#define SESSION_SKELETON_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), TYPE_SESSION_SKELETON, SessionSkeletonClass))
#define IS_SESSION_SKELETON(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), TYPE_SESSION_SKELETON))
#define IS_SESSION_SKELETON_CLASS(k) (G_TYPE_CHECK_CLASS_TYPE ((k), TYPE_SESSION_SKELETON))

typedef struct _SessionSkeleton SessionSkeleton;
typedef struct _SessionSkeletonClass SessionSkeletonClass;
typedef struct _SessionSkeletonPrivate SessionSkeletonPrivate;

struct _SessionSkeleton
{
  /*< private >*/
  GDBusInterfaceSkeleton parent_instance;
  SessionSkeletonPrivate *priv;
};

struct _SessionSkeletonClass
{
  GDBusInterfaceSkeletonClass parent_class;
};

GType session_skeleton_get_type (void) G_GNUC_CONST;

#if GLIB_CHECK_VERSION(2, 44, 0)
G_DEFINE_AUTOPTR_CLEANUP_FUNC (SessionSkeleton, g_object_unref)
#endif

Session *session_skeleton_new (void);


G_END_DECLS

#endif /* __SESSION_GENERATED_H__ */
