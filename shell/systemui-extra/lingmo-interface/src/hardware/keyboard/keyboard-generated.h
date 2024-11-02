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

#ifndef __KEYBOARD_GENERATED_H__
#define __KEYBOARD_GENERATED_H__

#include <gio/gio.h>

G_BEGIN_DECLS


/* ------------------------------------------------------------------------ */
/* Declarations for cn.lingmoos.keyboard */

#define TYPE_KEYBOARD (keyboard_get_type ())
#define KEYBOARD(o) (G_TYPE_CHECK_INSTANCE_CAST ((o), TYPE_KEYBOARD, Keyboard))
#define IS_KEYBOARD(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), TYPE_KEYBOARD))
#define KEYBOARD_GET_IFACE(o) (G_TYPE_INSTANCE_GET_INTERFACE ((o), TYPE_KEYBOARD, KeyboardIface))

struct _Keyboard;
typedef struct _Keyboard Keyboard;
typedef struct _KeyboardIface KeyboardIface;

struct _KeyboardIface
{
  GTypeInterface parent_iface;

  gboolean (*handle_get_bool_value) (
    Keyboard *object,
    GDBusMethodInvocation *invocation,
    const gchar *arg_InArg);

  gboolean (*handle_get_int_value) (
    Keyboard *object,
    GDBusMethodInvocation *invocation,
    const gchar *arg_InArg);

  gboolean (*handle_transfer_bool_value) (
    Keyboard *object,
    GDBusMethodInvocation *invocation,
    GVariant *arg_InArg);

  gboolean (*handle_transfer_int_value) (
    Keyboard *object,
    GDBusMethodInvocation *invocation,
    GVariant *arg_InArg);

};

GType keyboard_get_type (void) G_GNUC_CONST;

GDBusInterfaceInfo *keyboard_interface_info (void);
guint keyboard_override_properties (GObjectClass *klass, guint property_id_begin);


/* D-Bus method call completion functions: */
void keyboard_complete_transfer_bool_value (
    Keyboard *object,
    GDBusMethodInvocation *invocation,
    gboolean OutArg);

void keyboard_complete_get_bool_value (
    Keyboard *object,
    GDBusMethodInvocation *invocation,
    gboolean OutArg);

void keyboard_complete_transfer_int_value (
    Keyboard *object,
    GDBusMethodInvocation *invocation,
    gboolean OutArg);

void keyboard_complete_get_int_value (
    Keyboard *object,
    GDBusMethodInvocation *invocation,
    gint OutArg);



/* D-Bus method calls: */
void keyboard_call_transfer_bool_value (
    Keyboard *proxy,
    GVariant *arg_InArg,
    GCancellable *cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data);

gboolean keyboard_call_transfer_bool_value_finish (
    Keyboard *proxy,
    gboolean *out_OutArg,
    GAsyncResult *res,
    GError **error);

gboolean keyboard_call_transfer_bool_value_sync (
    Keyboard *proxy,
    GVariant *arg_InArg,
    gboolean *out_OutArg,
    GCancellable *cancellable,
    GError **error);

void keyboard_call_get_bool_value (
    Keyboard *proxy,
    const gchar *arg_InArg,
    GCancellable *cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data);

gboolean keyboard_call_get_bool_value_finish (
    Keyboard *proxy,
    gboolean *out_OutArg,
    GAsyncResult *res,
    GError **error);

gboolean keyboard_call_get_bool_value_sync (
    Keyboard *proxy,
    const gchar *arg_InArg,
    gboolean *out_OutArg,
    GCancellable *cancellable,
    GError **error);

void keyboard_call_transfer_int_value (
    Keyboard *proxy,
    GVariant *arg_InArg,
    GCancellable *cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data);

gboolean keyboard_call_transfer_int_value_finish (
    Keyboard *proxy,
    gboolean *out_OutArg,
    GAsyncResult *res,
    GError **error);

gboolean keyboard_call_transfer_int_value_sync (
    Keyboard *proxy,
    GVariant *arg_InArg,
    gboolean *out_OutArg,
    GCancellable *cancellable,
    GError **error);

void keyboard_call_get_int_value (
    Keyboard *proxy,
    const gchar *arg_InArg,
    GCancellable *cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data);

gboolean keyboard_call_get_int_value_finish (
    Keyboard *proxy,
    gint *out_OutArg,
    GAsyncResult *res,
    GError **error);

gboolean keyboard_call_get_int_value_sync (
    Keyboard *proxy,
    const gchar *arg_InArg,
    gint *out_OutArg,
    GCancellable *cancellable,
    GError **error);



/* ---- */

#define TYPE_KEYBOARD_PROXY (keyboard_proxy_get_type ())
#define KEYBOARD_PROXY(o) (G_TYPE_CHECK_INSTANCE_CAST ((o), TYPE_KEYBOARD_PROXY, KeyboardProxy))
#define KEYBOARD_PROXY_CLASS(k) (G_TYPE_CHECK_CLASS_CAST ((k), TYPE_KEYBOARD_PROXY, KeyboardProxyClass))
#define KEYBOARD_PROXY_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), TYPE_KEYBOARD_PROXY, KeyboardProxyClass))
#define IS_KEYBOARD_PROXY(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), TYPE_KEYBOARD_PROXY))
#define IS_KEYBOARD_PROXY_CLASS(k) (G_TYPE_CHECK_CLASS_TYPE ((k), TYPE_KEYBOARD_PROXY))

typedef struct _KeyboardProxy KeyboardProxy;
typedef struct _KeyboardProxyClass KeyboardProxyClass;
typedef struct _KeyboardProxyPrivate KeyboardProxyPrivate;

struct _KeyboardProxy
{
  /*< private >*/
  GDBusProxy parent_instance;
  KeyboardProxyPrivate *priv;
};

struct _KeyboardProxyClass
{
  GDBusProxyClass parent_class;
};

GType keyboard_proxy_get_type (void) G_GNUC_CONST;

#if GLIB_CHECK_VERSION(2, 44, 0)
G_DEFINE_AUTOPTR_CLEANUP_FUNC (KeyboardProxy, g_object_unref)
#endif

void keyboard_proxy_new (
    GDBusConnection     *connection,
    GDBusProxyFlags      flags,
    const gchar         *name,
    const gchar         *object_path,
    GCancellable        *cancellable,
    GAsyncReadyCallback  callback,
    gpointer             user_data);
Keyboard *keyboard_proxy_new_finish (
    GAsyncResult        *res,
    GError             **error);
Keyboard *keyboard_proxy_new_sync (
    GDBusConnection     *connection,
    GDBusProxyFlags      flags,
    const gchar         *name,
    const gchar         *object_path,
    GCancellable        *cancellable,
    GError             **error);

void keyboard_proxy_new_for_bus (
    GBusType             bus_type,
    GDBusProxyFlags      flags,
    const gchar         *name,
    const gchar         *object_path,
    GCancellable        *cancellable,
    GAsyncReadyCallback  callback,
    gpointer             user_data);
Keyboard *keyboard_proxy_new_for_bus_finish (
    GAsyncResult        *res,
    GError             **error);
Keyboard *keyboard_proxy_new_for_bus_sync (
    GBusType             bus_type,
    GDBusProxyFlags      flags,
    const gchar         *name,
    const gchar         *object_path,
    GCancellable        *cancellable,
    GError             **error);


/* ---- */

#define TYPE_KEYBOARD_SKELETON (keyboard_skeleton_get_type ())
#define KEYBOARD_SKELETON(o) (G_TYPE_CHECK_INSTANCE_CAST ((o), TYPE_KEYBOARD_SKELETON, KeyboardSkeleton))
#define KEYBOARD_SKELETON_CLASS(k) (G_TYPE_CHECK_CLASS_CAST ((k), TYPE_KEYBOARD_SKELETON, KeyboardSkeletonClass))
#define KEYBOARD_SKELETON_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), TYPE_KEYBOARD_SKELETON, KeyboardSkeletonClass))
#define IS_KEYBOARD_SKELETON(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), TYPE_KEYBOARD_SKELETON))
#define IS_KEYBOARD_SKELETON_CLASS(k) (G_TYPE_CHECK_CLASS_TYPE ((k), TYPE_KEYBOARD_SKELETON))

typedef struct _KeyboardSkeleton KeyboardSkeleton;
typedef struct _KeyboardSkeletonClass KeyboardSkeletonClass;
typedef struct _KeyboardSkeletonPrivate KeyboardSkeletonPrivate;

struct _KeyboardSkeleton
{
  /*< private >*/
  GDBusInterfaceSkeleton parent_instance;
  KeyboardSkeletonPrivate *priv;
};

struct _KeyboardSkeletonClass
{
  GDBusInterfaceSkeletonClass parent_class;
};

GType keyboard_skeleton_get_type (void) G_GNUC_CONST;

#if GLIB_CHECK_VERSION(2, 44, 0)
G_DEFINE_AUTOPTR_CLEANUP_FUNC (KeyboardSkeleton, g_object_unref)
#endif

Keyboard *keyboard_skeleton_new (void);


G_END_DECLS

#endif /* __KEYBOARD_GENERATED_H__ */
