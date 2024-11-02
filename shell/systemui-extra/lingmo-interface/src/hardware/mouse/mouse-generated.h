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

#ifndef __MOUSE_GENERATED_H__
#define __MOUSE_GENERATED_H__

#include <gio/gio.h>

G_BEGIN_DECLS


/* ------------------------------------------------------------------------ */
/* Declarations for cn.lingmoos.mouse */

#define TYPE_MOUSE (mouse_get_type ())
#define MOUSE(o) (G_TYPE_CHECK_INSTANCE_CAST ((o), TYPE_MOUSE, Mouse))
#define IS_MOUSE(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), TYPE_MOUSE))
#define MOUSE_GET_IFACE(o) (G_TYPE_INSTANCE_GET_INTERFACE ((o), TYPE_MOUSE, MouseIface))

struct _Mouse;
typedef struct _Mouse Mouse;
typedef struct _MouseIface MouseIface;

struct _MouseIface
{
  GTypeInterface parent_iface;

  gboolean (*handle_get_bool_value) (
    Mouse *object,
    GDBusMethodInvocation *invocation,
    const gchar *arg_InArg);

  gboolean (*handle_get_double_value) (
    Mouse *object,
    GDBusMethodInvocation *invocation,
    const gchar *arg_InArg);

  gboolean (*handle_get_int_value) (
    Mouse *object,
    GDBusMethodInvocation *invocation,
    const gchar *arg_InArg);

  gboolean (*handle_get_string_value) (
    Mouse *object,
    GDBusMethodInvocation *invocation,
    const gchar *arg_InArg);

  gboolean (*handle_transfer_bool_value) (
    Mouse *object,
    GDBusMethodInvocation *invocation,
    GVariant *arg_InArg);

  gboolean (*handle_transfer_double_value) (
    Mouse *object,
    GDBusMethodInvocation *invocation,
    GVariant *arg_InArg);

  gboolean (*handle_transfer_int_value) (
    Mouse *object,
    GDBusMethodInvocation *invocation,
    GVariant *arg_InArg);

  gboolean (*handle_transfer_string_value) (
    Mouse *object,
    GDBusMethodInvocation *invocation,
    GVariant *arg_InArg);

};

GType mouse_get_type (void) G_GNUC_CONST;

GDBusInterfaceInfo *mouse_interface_info (void);
guint mouse_override_properties (GObjectClass *klass, guint property_id_begin);


/* D-Bus method call completion functions: */
void mouse_complete_transfer_bool_value (
    Mouse *object,
    GDBusMethodInvocation *invocation,
    gboolean OutArg);

void mouse_complete_get_bool_value (
    Mouse *object,
    GDBusMethodInvocation *invocation,
    gboolean OutArg);

void mouse_complete_transfer_string_value (
    Mouse *object,
    GDBusMethodInvocation *invocation,
    gboolean OutArg);

void mouse_complete_get_string_value (
    Mouse *object,
    GDBusMethodInvocation *invocation,
    const gchar *OutArg);

void mouse_complete_transfer_int_value (
    Mouse *object,
    GDBusMethodInvocation *invocation,
    gboolean OutArg);

void mouse_complete_get_int_value (
    Mouse *object,
    GDBusMethodInvocation *invocation,
    gint OutArg);

void mouse_complete_transfer_double_value (
    Mouse *object,
    GDBusMethodInvocation *invocation,
    gboolean OutArg);

void mouse_complete_get_double_value (
    Mouse *object,
    GDBusMethodInvocation *invocation,
    gdouble OutArg);



/* D-Bus method calls: */
void mouse_call_transfer_bool_value (
    Mouse *proxy,
    GVariant *arg_InArg,
    GCancellable *cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data);

gboolean mouse_call_transfer_bool_value_finish (
    Mouse *proxy,
    gboolean *out_OutArg,
    GAsyncResult *res,
    GError **error);

gboolean mouse_call_transfer_bool_value_sync (
    Mouse *proxy,
    GVariant *arg_InArg,
    gboolean *out_OutArg,
    GCancellable *cancellable,
    GError **error);

void mouse_call_get_bool_value (
    Mouse *proxy,
    const gchar *arg_InArg,
    GCancellable *cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data);

gboolean mouse_call_get_bool_value_finish (
    Mouse *proxy,
    gboolean *out_OutArg,
    GAsyncResult *res,
    GError **error);

gboolean mouse_call_get_bool_value_sync (
    Mouse *proxy,
    const gchar *arg_InArg,
    gboolean *out_OutArg,
    GCancellable *cancellable,
    GError **error);

void mouse_call_transfer_string_value (
    Mouse *proxy,
    GVariant *arg_InArg,
    GCancellable *cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data);

gboolean mouse_call_transfer_string_value_finish (
    Mouse *proxy,
    gboolean *out_OutArg,
    GAsyncResult *res,
    GError **error);

gboolean mouse_call_transfer_string_value_sync (
    Mouse *proxy,
    GVariant *arg_InArg,
    gboolean *out_OutArg,
    GCancellable *cancellable,
    GError **error);

void mouse_call_get_string_value (
    Mouse *proxy,
    const gchar *arg_InArg,
    GCancellable *cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data);

gboolean mouse_call_get_string_value_finish (
    Mouse *proxy,
    gchar **out_OutArg,
    GAsyncResult *res,
    GError **error);

gboolean mouse_call_get_string_value_sync (
    Mouse *proxy,
    const gchar *arg_InArg,
    gchar **out_OutArg,
    GCancellable *cancellable,
    GError **error);

void mouse_call_transfer_int_value (
    Mouse *proxy,
    GVariant *arg_InArg,
    GCancellable *cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data);

gboolean mouse_call_transfer_int_value_finish (
    Mouse *proxy,
    gboolean *out_OutArg,
    GAsyncResult *res,
    GError **error);

gboolean mouse_call_transfer_int_value_sync (
    Mouse *proxy,
    GVariant *arg_InArg,
    gboolean *out_OutArg,
    GCancellable *cancellable,
    GError **error);

void mouse_call_get_int_value (
    Mouse *proxy,
    const gchar *arg_InArg,
    GCancellable *cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data);

gboolean mouse_call_get_int_value_finish (
    Mouse *proxy,
    gint *out_OutArg,
    GAsyncResult *res,
    GError **error);

gboolean mouse_call_get_int_value_sync (
    Mouse *proxy,
    const gchar *arg_InArg,
    gint *out_OutArg,
    GCancellable *cancellable,
    GError **error);

void mouse_call_transfer_double_value (
    Mouse *proxy,
    GVariant *arg_InArg,
    GCancellable *cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data);

gboolean mouse_call_transfer_double_value_finish (
    Mouse *proxy,
    gboolean *out_OutArg,
    GAsyncResult *res,
    GError **error);

gboolean mouse_call_transfer_double_value_sync (
    Mouse *proxy,
    GVariant *arg_InArg,
    gboolean *out_OutArg,
    GCancellable *cancellable,
    GError **error);

void mouse_call_get_double_value (
    Mouse *proxy,
    const gchar *arg_InArg,
    GCancellable *cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data);

gboolean mouse_call_get_double_value_finish (
    Mouse *proxy,
    gdouble *out_OutArg,
    GAsyncResult *res,
    GError **error);

gboolean mouse_call_get_double_value_sync (
    Mouse *proxy,
    const gchar *arg_InArg,
    gdouble *out_OutArg,
    GCancellable *cancellable,
    GError **error);



/* ---- */

#define TYPE_MOUSE_PROXY (mouse_proxy_get_type ())
#define MOUSE_PROXY(o) (G_TYPE_CHECK_INSTANCE_CAST ((o), TYPE_MOUSE_PROXY, MouseProxy))
#define MOUSE_PROXY_CLASS(k) (G_TYPE_CHECK_CLASS_CAST ((k), TYPE_MOUSE_PROXY, MouseProxyClass))
#define MOUSE_PROXY_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), TYPE_MOUSE_PROXY, MouseProxyClass))
#define IS_MOUSE_PROXY(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), TYPE_MOUSE_PROXY))
#define IS_MOUSE_PROXY_CLASS(k) (G_TYPE_CHECK_CLASS_TYPE ((k), TYPE_MOUSE_PROXY))

typedef struct _MouseProxy MouseProxy;
typedef struct _MouseProxyClass MouseProxyClass;
typedef struct _MouseProxyPrivate MouseProxyPrivate;

struct _MouseProxy
{
  /*< private >*/
  GDBusProxy parent_instance;
  MouseProxyPrivate *priv;
};

struct _MouseProxyClass
{
  GDBusProxyClass parent_class;
};

GType mouse_proxy_get_type (void) G_GNUC_CONST;

#if GLIB_CHECK_VERSION(2, 44, 0)
G_DEFINE_AUTOPTR_CLEANUP_FUNC (MouseProxy, g_object_unref)
#endif

void mouse_proxy_new (
    GDBusConnection     *connection,
    GDBusProxyFlags      flags,
    const gchar         *name,
    const gchar         *object_path,
    GCancellable        *cancellable,
    GAsyncReadyCallback  callback,
    gpointer             user_data);
Mouse *mouse_proxy_new_finish (
    GAsyncResult        *res,
    GError             **error);
Mouse *mouse_proxy_new_sync (
    GDBusConnection     *connection,
    GDBusProxyFlags      flags,
    const gchar         *name,
    const gchar         *object_path,
    GCancellable        *cancellable,
    GError             **error);

void mouse_proxy_new_for_bus (
    GBusType             bus_type,
    GDBusProxyFlags      flags,
    const gchar         *name,
    const gchar         *object_path,
    GCancellable        *cancellable,
    GAsyncReadyCallback  callback,
    gpointer             user_data);
Mouse *mouse_proxy_new_for_bus_finish (
    GAsyncResult        *res,
    GError             **error);
Mouse *mouse_proxy_new_for_bus_sync (
    GBusType             bus_type,
    GDBusProxyFlags      flags,
    const gchar         *name,
    const gchar         *object_path,
    GCancellable        *cancellable,
    GError             **error);


/* ---- */

#define TYPE_MOUSE_SKELETON (mouse_skeleton_get_type ())
#define MOUSE_SKELETON(o) (G_TYPE_CHECK_INSTANCE_CAST ((o), TYPE_MOUSE_SKELETON, MouseSkeleton))
#define MOUSE_SKELETON_CLASS(k) (G_TYPE_CHECK_CLASS_CAST ((k), TYPE_MOUSE_SKELETON, MouseSkeletonClass))
#define MOUSE_SKELETON_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), TYPE_MOUSE_SKELETON, MouseSkeletonClass))
#define IS_MOUSE_SKELETON(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), TYPE_MOUSE_SKELETON))
#define IS_MOUSE_SKELETON_CLASS(k) (G_TYPE_CHECK_CLASS_TYPE ((k), TYPE_MOUSE_SKELETON))

typedef struct _MouseSkeleton MouseSkeleton;
typedef struct _MouseSkeletonClass MouseSkeletonClass;
typedef struct _MouseSkeletonPrivate MouseSkeletonPrivate;

struct _MouseSkeleton
{
  /*< private >*/
  GDBusInterfaceSkeleton parent_instance;
  MouseSkeletonPrivate *priv;
};

struct _MouseSkeletonClass
{
  GDBusInterfaceSkeletonClass parent_class;
};

GType mouse_skeleton_get_type (void) G_GNUC_CONST;

#if GLIB_CHECK_VERSION(2, 44, 0)
G_DEFINE_AUTOPTR_CLEANUP_FUNC (MouseSkeleton, g_object_unref)
#endif

Mouse *mouse_skeleton_new (void);


G_END_DECLS

#endif /* __MOUSE_GENERATED_H__ */
