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

#ifndef __INTERFACE_GENERATED_H__
#define __INTERFACE_GENERATED_H__

#include <gio/gio.h>

G_BEGIN_DECLS


/* ------------------------------------------------------------------------ */
/* Declarations for cn.lingmoos.interface */

#define TYPE_INTERFACE (interface_get_type ())
#define INTERFACE(o) (G_TYPE_CHECK_INSTANCE_CAST ((o), TYPE_INTERFACE, Interface))
#define IS_INTERFACE(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), TYPE_INTERFACE))
#define INTERFACE_GET_IFACE(o) (G_TYPE_INSTANCE_GET_INTERFACE ((o), TYPE_INTERFACE, InterfaceIface))

struct _Interface;
typedef struct _Interface Interface;
typedef struct _InterfaceIface InterfaceIface;

struct _InterfaceIface
{
  GTypeInterface parent_iface;

  gboolean (*handle_get_bool_value) (
    Interface *object,
    GDBusMethodInvocation *invocation,
    const gchar *arg_InArg);

  gboolean (*handle_get_int_value) (
    Interface *object,
    GDBusMethodInvocation *invocation,
    const gchar *arg_InArg);

  gboolean (*handle_get_string_value) (
    Interface *object,
    GDBusMethodInvocation *invocation,
    const gchar *arg_InArg);

  gboolean (*handle_transfer_bool_value) (
    Interface *object,
    GDBusMethodInvocation *invocation,
    GVariant *arg_InArg);

  gboolean (*handle_transfer_int_value) (
    Interface *object,
    GDBusMethodInvocation *invocation,
    GVariant *arg_InArg);

  gboolean (*handle_transfer_string_value) (
    Interface *object,
    GDBusMethodInvocation *invocation,
    GVariant *arg_InArg);

};

GType interface_get_type (void) G_GNUC_CONST;

GDBusInterfaceInfo *interface_interface_info (void);
guint interface_override_properties (GObjectClass *klass, guint property_id_begin);


/* D-Bus method call completion functions: */
void interface_complete_transfer_bool_value (
    Interface *object,
    GDBusMethodInvocation *invocation,
    gboolean OutArg);

void interface_complete_get_bool_value (
    Interface *object,
    GDBusMethodInvocation *invocation,
    gboolean OutArg);

void interface_complete_transfer_int_value (
    Interface *object,
    GDBusMethodInvocation *invocation,
    gboolean OutArg);

void interface_complete_get_int_value (
    Interface *object,
    GDBusMethodInvocation *invocation,
    gint OutArg);

void interface_complete_transfer_string_value (
    Interface *object,
    GDBusMethodInvocation *invocation,
    gboolean OutArg);

void interface_complete_get_string_value (
    Interface *object,
    GDBusMethodInvocation *invocation,
    const gchar *OutArg);



/* D-Bus method calls: */
void interface_call_transfer_bool_value (
    Interface *proxy,
    GVariant *arg_InArg,
    GCancellable *cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data);

gboolean interface_call_transfer_bool_value_finish (
    Interface *proxy,
    gboolean *out_OutArg,
    GAsyncResult *res,
    GError **error);

gboolean interface_call_transfer_bool_value_sync (
    Interface *proxy,
    GVariant *arg_InArg,
    gboolean *out_OutArg,
    GCancellable *cancellable,
    GError **error);

void interface_call_get_bool_value (
    Interface *proxy,
    const gchar *arg_InArg,
    GCancellable *cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data);

gboolean interface_call_get_bool_value_finish (
    Interface *proxy,
    gboolean *out_OutArg,
    GAsyncResult *res,
    GError **error);

gboolean interface_call_get_bool_value_sync (
    Interface *proxy,
    const gchar *arg_InArg,
    gboolean *out_OutArg,
    GCancellable *cancellable,
    GError **error);

void interface_call_transfer_int_value (
    Interface *proxy,
    GVariant *arg_InArg,
    GCancellable *cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data);

gboolean interface_call_transfer_int_value_finish (
    Interface *proxy,
    gboolean *out_OutArg,
    GAsyncResult *res,
    GError **error);

gboolean interface_call_transfer_int_value_sync (
    Interface *proxy,
    GVariant *arg_InArg,
    gboolean *out_OutArg,
    GCancellable *cancellable,
    GError **error);

void interface_call_get_int_value (
    Interface *proxy,
    const gchar *arg_InArg,
    GCancellable *cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data);

gboolean interface_call_get_int_value_finish (
    Interface *proxy,
    gint *out_OutArg,
    GAsyncResult *res,
    GError **error);

gboolean interface_call_get_int_value_sync (
    Interface *proxy,
    const gchar *arg_InArg,
    gint *out_OutArg,
    GCancellable *cancellable,
    GError **error);

void interface_call_transfer_string_value (
    Interface *proxy,
    GVariant *arg_InArg,
    GCancellable *cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data);

gboolean interface_call_transfer_string_value_finish (
    Interface *proxy,
    gboolean *out_OutArg,
    GAsyncResult *res,
    GError **error);

gboolean interface_call_transfer_string_value_sync (
    Interface *proxy,
    GVariant *arg_InArg,
    gboolean *out_OutArg,
    GCancellable *cancellable,
    GError **error);

void interface_call_get_string_value (
    Interface *proxy,
    const gchar *arg_InArg,
    GCancellable *cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data);

gboolean interface_call_get_string_value_finish (
    Interface *proxy,
    gchar **out_OutArg,
    GAsyncResult *res,
    GError **error);

gboolean interface_call_get_string_value_sync (
    Interface *proxy,
    const gchar *arg_InArg,
    gchar **out_OutArg,
    GCancellable *cancellable,
    GError **error);



/* ---- */

#define TYPE_INTERFACE_PROXY (interface_proxy_get_type ())
#define INTERFACE_PROXY(o) (G_TYPE_CHECK_INSTANCE_CAST ((o), TYPE_INTERFACE_PROXY, InterfaceProxy))
#define INTERFACE_PROXY_CLASS(k) (G_TYPE_CHECK_CLASS_CAST ((k), TYPE_INTERFACE_PROXY, InterfaceProxyClass))
#define INTERFACE_PROXY_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), TYPE_INTERFACE_PROXY, InterfaceProxyClass))
#define IS_INTERFACE_PROXY(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), TYPE_INTERFACE_PROXY))
#define IS_INTERFACE_PROXY_CLASS(k) (G_TYPE_CHECK_CLASS_TYPE ((k), TYPE_INTERFACE_PROXY))

typedef struct _InterfaceProxy InterfaceProxy;
typedef struct _InterfaceProxyClass InterfaceProxyClass;
typedef struct _InterfaceProxyPrivate InterfaceProxyPrivate;

struct _InterfaceProxy
{
  /*< private >*/
  GDBusProxy parent_instance;
  InterfaceProxyPrivate *priv;
};

struct _InterfaceProxyClass
{
  GDBusProxyClass parent_class;
};

GType interface_proxy_get_type (void) G_GNUC_CONST;

#if GLIB_CHECK_VERSION(2, 44, 0)
G_DEFINE_AUTOPTR_CLEANUP_FUNC (InterfaceProxy, g_object_unref)
#endif

void interface_proxy_new (
    GDBusConnection     *connection,
    GDBusProxyFlags      flags,
    const gchar         *name,
    const gchar         *object_path,
    GCancellable        *cancellable,
    GAsyncReadyCallback  callback,
    gpointer             user_data);
Interface *interface_proxy_new_finish (
    GAsyncResult        *res,
    GError             **error);
Interface *interface_proxy_new_sync (
    GDBusConnection     *connection,
    GDBusProxyFlags      flags,
    const gchar         *name,
    const gchar         *object_path,
    GCancellable        *cancellable,
    GError             **error);

void interface_proxy_new_for_bus (
    GBusType             bus_type,
    GDBusProxyFlags      flags,
    const gchar         *name,
    const gchar         *object_path,
    GCancellable        *cancellable,
    GAsyncReadyCallback  callback,
    gpointer             user_data);
Interface *interface_proxy_new_for_bus_finish (
    GAsyncResult        *res,
    GError             **error);
Interface *interface_proxy_new_for_bus_sync (
    GBusType             bus_type,
    GDBusProxyFlags      flags,
    const gchar         *name,
    const gchar         *object_path,
    GCancellable        *cancellable,
    GError             **error);


/* ---- */

#define TYPE_INTERFACE_SKELETON (interface_skeleton_get_type ())
#define INTERFACE_SKELETON(o) (G_TYPE_CHECK_INSTANCE_CAST ((o), TYPE_INTERFACE_SKELETON, InterfaceSkeleton))
#define INTERFACE_SKELETON_CLASS(k) (G_TYPE_CHECK_CLASS_CAST ((k), TYPE_INTERFACE_SKELETON, InterfaceSkeletonClass))
#define INTERFACE_SKELETON_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), TYPE_INTERFACE_SKELETON, InterfaceSkeletonClass))
#define IS_INTERFACE_SKELETON(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), TYPE_INTERFACE_SKELETON))
#define IS_INTERFACE_SKELETON_CLASS(k) (G_TYPE_CHECK_CLASS_TYPE ((k), TYPE_INTERFACE_SKELETON))

typedef struct _InterfaceSkeleton InterfaceSkeleton;
typedef struct _InterfaceSkeletonClass InterfaceSkeletonClass;
typedef struct _InterfaceSkeletonPrivate InterfaceSkeletonPrivate;

struct _InterfaceSkeleton
{
  /*< private >*/
  GDBusInterfaceSkeleton parent_instance;
  InterfaceSkeletonPrivate *priv;
};

struct _InterfaceSkeletonClass
{
  GDBusInterfaceSkeletonClass parent_class;
};

GType interface_skeleton_get_type (void) G_GNUC_CONST;

#if GLIB_CHECK_VERSION(2, 44, 0)
G_DEFINE_AUTOPTR_CLEANUP_FUNC (InterfaceSkeleton, g_object_unref)
#endif

Interface *interface_skeleton_new (void);


G_END_DECLS

#endif /* __INTERFACE_GENERATED_H__ */
