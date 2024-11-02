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

#ifndef __DESKTOP_GENERATED_H__
#define __DESKTOP_GENERATED_H__

#include <gio/gio.h>

G_BEGIN_DECLS


/* ------------------------------------------------------------------------ */
/* Declarations for cn.lingmoos.desktop */

#define TYPE_DESKTOP (desktop_get_type ())
#define DESKTOP(o) (G_TYPE_CHECK_INSTANCE_CAST ((o), TYPE_DESKTOP, Desktop))
#define IS_DESKTOP(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), TYPE_DESKTOP))
#define DESKTOP_GET_IFACE(o) (G_TYPE_INSTANCE_GET_INTERFACE ((o), TYPE_DESKTOP, DesktopIface))

struct _Desktop;
typedef struct _Desktop Desktop;
typedef struct _DesktopIface DesktopIface;

struct _DesktopIface
{
  GTypeInterface parent_iface;

  gboolean (*handle_transfer_bool_value) (
    Desktop *object,
    GDBusMethodInvocation *invocation,
    GVariant *arg_InArg);

  gboolean (*handle_transfer_int_value) (
    Desktop *object,
    GDBusMethodInvocation *invocation,
    GVariant *arg_InArg);

  gboolean (*handle_transfer_string_value) (
    Desktop *object,
    GDBusMethodInvocation *invocation,
    GVariant *arg_InArg);

};

GType desktop_get_type (void) G_GNUC_CONST;

GDBusInterfaceInfo *desktop_interface_info (void);
guint desktop_override_properties (GObjectClass *klass, guint property_id_begin);


/* D-Bus method call completion functions: */
void desktop_complete_transfer_bool_value (
    Desktop *object,
    GDBusMethodInvocation *invocation);

void desktop_complete_transfer_string_value (
    Desktop *object,
    GDBusMethodInvocation *invocation);

void desktop_complete_transfer_int_value (
    Desktop *object,
    GDBusMethodInvocation *invocation);



/* D-Bus method calls: */
void desktop_call_transfer_bool_value (
    Desktop *proxy,
    GVariant *arg_InArg,
    GCancellable *cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data);

gboolean desktop_call_transfer_bool_value_finish (
    Desktop *proxy,
    GAsyncResult *res,
    GError **error);

gboolean desktop_call_transfer_bool_value_sync (
    Desktop *proxy,
    GVariant *arg_InArg,
    GCancellable *cancellable,
    GError **error);

void desktop_call_transfer_string_value (
    Desktop *proxy,
    GVariant *arg_InArg,
    GCancellable *cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data);

gboolean desktop_call_transfer_string_value_finish (
    Desktop *proxy,
    GAsyncResult *res,
    GError **error);

gboolean desktop_call_transfer_string_value_sync (
    Desktop *proxy,
    GVariant *arg_InArg,
    GCancellable *cancellable,
    GError **error);

void desktop_call_transfer_int_value (
    Desktop *proxy,
    GVariant *arg_InArg,
    GCancellable *cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data);

gboolean desktop_call_transfer_int_value_finish (
    Desktop *proxy,
    GAsyncResult *res,
    GError **error);

gboolean desktop_call_transfer_int_value_sync (
    Desktop *proxy,
    GVariant *arg_InArg,
    GCancellable *cancellable,
    GError **error);



/* ---- */

#define TYPE_DESKTOP_PROXY (desktop_proxy_get_type ())
#define DESKTOP_PROXY(o) (G_TYPE_CHECK_INSTANCE_CAST ((o), TYPE_DESKTOP_PROXY, DesktopProxy))
#define DESKTOP_PROXY_CLASS(k) (G_TYPE_CHECK_CLASS_CAST ((k), TYPE_DESKTOP_PROXY, DesktopProxyClass))
#define DESKTOP_PROXY_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), TYPE_DESKTOP_PROXY, DesktopProxyClass))
#define IS_DESKTOP_PROXY(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), TYPE_DESKTOP_PROXY))
#define IS_DESKTOP_PROXY_CLASS(k) (G_TYPE_CHECK_CLASS_TYPE ((k), TYPE_DESKTOP_PROXY))

typedef struct _DesktopProxy DesktopProxy;
typedef struct _DesktopProxyClass DesktopProxyClass;
typedef struct _DesktopProxyPrivate DesktopProxyPrivate;

struct _DesktopProxy
{
  /*< private >*/
  GDBusProxy parent_instance;
  DesktopProxyPrivate *priv;
};

struct _DesktopProxyClass
{
  GDBusProxyClass parent_class;
};

GType desktop_proxy_get_type (void) G_GNUC_CONST;

#if GLIB_CHECK_VERSION(2, 44, 0)
G_DEFINE_AUTOPTR_CLEANUP_FUNC (DesktopProxy, g_object_unref)
#endif

void desktop_proxy_new (
    GDBusConnection     *connection,
    GDBusProxyFlags      flags,
    const gchar         *name,
    const gchar         *object_path,
    GCancellable        *cancellable,
    GAsyncReadyCallback  callback,
    gpointer             user_data);
Desktop *desktop_proxy_new_finish (
    GAsyncResult        *res,
    GError             **error);
Desktop *desktop_proxy_new_sync (
    GDBusConnection     *connection,
    GDBusProxyFlags      flags,
    const gchar         *name,
    const gchar         *object_path,
    GCancellable        *cancellable,
    GError             **error);

void desktop_proxy_new_for_bus (
    GBusType             bus_type,
    GDBusProxyFlags      flags,
    const gchar         *name,
    const gchar         *object_path,
    GCancellable        *cancellable,
    GAsyncReadyCallback  callback,
    gpointer             user_data);
Desktop *desktop_proxy_new_for_bus_finish (
    GAsyncResult        *res,
    GError             **error);
Desktop *desktop_proxy_new_for_bus_sync (
    GBusType             bus_type,
    GDBusProxyFlags      flags,
    const gchar         *name,
    const gchar         *object_path,
    GCancellable        *cancellable,
    GError             **error);


/* ---- */

#define TYPE_DESKTOP_SKELETON (desktop_skeleton_get_type ())
#define DESKTOP_SKELETON(o) (G_TYPE_CHECK_INSTANCE_CAST ((o), TYPE_DESKTOP_SKELETON, DesktopSkeleton))
#define DESKTOP_SKELETON_CLASS(k) (G_TYPE_CHECK_CLASS_CAST ((k), TYPE_DESKTOP_SKELETON, DesktopSkeletonClass))
#define DESKTOP_SKELETON_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), TYPE_DESKTOP_SKELETON, DesktopSkeletonClass))
#define IS_DESKTOP_SKELETON(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), TYPE_DESKTOP_SKELETON))
#define IS_DESKTOP_SKELETON_CLASS(k) (G_TYPE_CHECK_CLASS_TYPE ((k), TYPE_DESKTOP_SKELETON))

typedef struct _DesktopSkeleton DesktopSkeleton;
typedef struct _DesktopSkeletonClass DesktopSkeletonClass;
typedef struct _DesktopSkeletonPrivate DesktopSkeletonPrivate;

struct _DesktopSkeleton
{
  /*< private >*/
  GDBusInterfaceSkeleton parent_instance;
  DesktopSkeletonPrivate *priv;
};

struct _DesktopSkeletonClass
{
  GDBusInterfaceSkeletonClass parent_class;
};

GType desktop_skeleton_get_type (void) G_GNUC_CONST;

#if GLIB_CHECK_VERSION(2, 44, 0)
G_DEFINE_AUTOPTR_CLEANUP_FUNC (DesktopSkeleton, g_object_unref)
#endif

Desktop *desktop_skeleton_new (void);


G_END_DECLS

#endif /* __DESKTOP_GENERATED_H__ */
