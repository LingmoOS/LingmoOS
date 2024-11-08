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

#ifndef __TOUCHPAD_GENERATED_H__
#define __TOUCHPAD_GENERATED_H__

#include <gio/gio.h>

G_BEGIN_DECLS


/* ------------------------------------------------------------------------ */
/* Declarations for cn.lingmoos.touchpad */

#define TYPE_TOUCHPAD (touchpad_get_type ())
#define TOUCHPAD(o) (G_TYPE_CHECK_INSTANCE_CAST ((o), TYPE_TOUCHPAD, Touchpad))
#define IS_TOUCHPAD(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), TYPE_TOUCHPAD))
#define TOUCHPAD_GET_IFACE(o) (G_TYPE_INSTANCE_GET_INTERFACE ((o), TYPE_TOUCHPAD, TouchpadIface))

struct _Touchpad;
typedef struct _Touchpad Touchpad;
typedef struct _TouchpadIface TouchpadIface;

struct _TouchpadIface
{
  GTypeInterface parent_iface;

  gboolean (*handle_transfer_bool_value) (
    Touchpad *object,
    GDBusMethodInvocation *invocation,
    GVariant *arg_InArg);

  gboolean (*handle_transfer_int_value) (
    Touchpad *object,
    GDBusMethodInvocation *invocation,
    GVariant *arg_InArg);

};

GType touchpad_get_type (void) G_GNUC_CONST;

GDBusInterfaceInfo *touchpad_interface_info (void);
guint touchpad_override_properties (GObjectClass *klass, guint property_id_begin);


/* D-Bus method call completion functions: */
void touchpad_complete_transfer_bool_value (
    Touchpad *object,
    GDBusMethodInvocation *invocation);

void touchpad_complete_transfer_int_value (
    Touchpad *object,
    GDBusMethodInvocation *invocation);



/* D-Bus method calls: */
void touchpad_call_transfer_bool_value (
    Touchpad *proxy,
    GVariant *arg_InArg,
    GCancellable *cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data);

gboolean touchpad_call_transfer_bool_value_finish (
    Touchpad *proxy,
    GAsyncResult *res,
    GError **error);

gboolean touchpad_call_transfer_bool_value_sync (
    Touchpad *proxy,
    GVariant *arg_InArg,
    GCancellable *cancellable,
    GError **error);

void touchpad_call_transfer_int_value (
    Touchpad *proxy,
    GVariant *arg_InArg,
    GCancellable *cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data);

gboolean touchpad_call_transfer_int_value_finish (
    Touchpad *proxy,
    GAsyncResult *res,
    GError **error);

gboolean touchpad_call_transfer_int_value_sync (
    Touchpad *proxy,
    GVariant *arg_InArg,
    GCancellable *cancellable,
    GError **error);



/* ---- */

#define TYPE_TOUCHPAD_PROXY (touchpad_proxy_get_type ())
#define TOUCHPAD_PROXY(o) (G_TYPE_CHECK_INSTANCE_CAST ((o), TYPE_TOUCHPAD_PROXY, TouchpadProxy))
#define TOUCHPAD_PROXY_CLASS(k) (G_TYPE_CHECK_CLASS_CAST ((k), TYPE_TOUCHPAD_PROXY, TouchpadProxyClass))
#define TOUCHPAD_PROXY_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), TYPE_TOUCHPAD_PROXY, TouchpadProxyClass))
#define IS_TOUCHPAD_PROXY(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), TYPE_TOUCHPAD_PROXY))
#define IS_TOUCHPAD_PROXY_CLASS(k) (G_TYPE_CHECK_CLASS_TYPE ((k), TYPE_TOUCHPAD_PROXY))

typedef struct _TouchpadProxy TouchpadProxy;
typedef struct _TouchpadProxyClass TouchpadProxyClass;
typedef struct _TouchpadProxyPrivate TouchpadProxyPrivate;

struct _TouchpadProxy
{
  /*< private >*/
  GDBusProxy parent_instance;
  TouchpadProxyPrivate *priv;
};

struct _TouchpadProxyClass
{
  GDBusProxyClass parent_class;
};

GType touchpad_proxy_get_type (void) G_GNUC_CONST;

#if GLIB_CHECK_VERSION(2, 44, 0)
G_DEFINE_AUTOPTR_CLEANUP_FUNC (TouchpadProxy, g_object_unref)
#endif

void touchpad_proxy_new (
    GDBusConnection     *connection,
    GDBusProxyFlags      flags,
    const gchar         *name,
    const gchar         *object_path,
    GCancellable        *cancellable,
    GAsyncReadyCallback  callback,
    gpointer             user_data);
Touchpad *touchpad_proxy_new_finish (
    GAsyncResult        *res,
    GError             **error);
Touchpad *touchpad_proxy_new_sync (
    GDBusConnection     *connection,
    GDBusProxyFlags      flags,
    const gchar         *name,
    const gchar         *object_path,
    GCancellable        *cancellable,
    GError             **error);

void touchpad_proxy_new_for_bus (
    GBusType             bus_type,
    GDBusProxyFlags      flags,
    const gchar         *name,
    const gchar         *object_path,
    GCancellable        *cancellable,
    GAsyncReadyCallback  callback,
    gpointer             user_data);
Touchpad *touchpad_proxy_new_for_bus_finish (
    GAsyncResult        *res,
    GError             **error);
Touchpad *touchpad_proxy_new_for_bus_sync (
    GBusType             bus_type,
    GDBusProxyFlags      flags,
    const gchar         *name,
    const gchar         *object_path,
    GCancellable        *cancellable,
    GError             **error);


/* ---- */

#define TYPE_TOUCHPAD_SKELETON (touchpad_skeleton_get_type ())
#define TOUCHPAD_SKELETON(o) (G_TYPE_CHECK_INSTANCE_CAST ((o), TYPE_TOUCHPAD_SKELETON, TouchpadSkeleton))
#define TOUCHPAD_SKELETON_CLASS(k) (G_TYPE_CHECK_CLASS_CAST ((k), TYPE_TOUCHPAD_SKELETON, TouchpadSkeletonClass))
#define TOUCHPAD_SKELETON_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), TYPE_TOUCHPAD_SKELETON, TouchpadSkeletonClass))
#define IS_TOUCHPAD_SKELETON(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), TYPE_TOUCHPAD_SKELETON))
#define IS_TOUCHPAD_SKELETON_CLASS(k) (G_TYPE_CHECK_CLASS_TYPE ((k), TYPE_TOUCHPAD_SKELETON))

typedef struct _TouchpadSkeleton TouchpadSkeleton;
typedef struct _TouchpadSkeletonClass TouchpadSkeletonClass;
typedef struct _TouchpadSkeletonPrivate TouchpadSkeletonPrivate;

struct _TouchpadSkeleton
{
  /*< private >*/
  GDBusInterfaceSkeleton parent_instance;
  TouchpadSkeletonPrivate *priv;
};

struct _TouchpadSkeletonClass
{
  GDBusInterfaceSkeletonClass parent_class;
};

GType touchpad_skeleton_get_type (void) G_GNUC_CONST;

#if GLIB_CHECK_VERSION(2, 44, 0)
G_DEFINE_AUTOPTR_CLEANUP_FUNC (TouchpadSkeleton, g_object_unref)
#endif

Touchpad *touchpad_skeleton_new (void);


G_END_DECLS

#endif /* __TOUCHPAD_GENERATED_H__ */
