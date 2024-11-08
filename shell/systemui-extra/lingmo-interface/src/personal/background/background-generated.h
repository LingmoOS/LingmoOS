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

#ifndef __BACKGROUND_GENERATED_H__
#define __BACKGROUND_GENERATED_H__

#include <gio/gio.h>

G_BEGIN_DECLS


/* ------------------------------------------------------------------------ */
/* Declarations for cn.lingmoos.background */

#define TYPE_BACKGROUND (background_get_type ())
#define BACKGROUND(o) (G_TYPE_CHECK_INSTANCE_CAST ((o), TYPE_BACKGROUND, Background))
#define IS_BACKGROUND(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), TYPE_BACKGROUND))
#define BACKGROUND_GET_IFACE(o) (G_TYPE_INSTANCE_GET_INTERFACE ((o), TYPE_BACKGROUND, BackgroundIface))

struct _Background;
typedef struct _Background Background;
typedef struct _BackgroundIface BackgroundIface;

struct _BackgroundIface
{
  GTypeInterface parent_iface;

  gboolean (*handle_transfer_bool_value) (
    Background *object,
    GDBusMethodInvocation *invocation,
    GVariant *arg_InArg);

  gboolean (*handle_transfer_int_value) (
    Background *object,
    GDBusMethodInvocation *invocation,
    GVariant *arg_InArg);

  gboolean (*handle_transfer_string_value) (
    Background *object,
    GDBusMethodInvocation *invocation,
    GVariant *arg_InArg);

};

GType background_get_type (void) G_GNUC_CONST;

GDBusInterfaceInfo *background_interface_info (void);
guint background_override_properties (GObjectClass *klass, guint property_id_begin);


/* D-Bus method call completion functions: */
void background_complete_transfer_bool_value (
    Background *object,
    GDBusMethodInvocation *invocation);

void background_complete_transfer_string_value (
    Background *object,
    GDBusMethodInvocation *invocation);

void background_complete_transfer_int_value (
    Background *object,
    GDBusMethodInvocation *invocation);



/* D-Bus method calls: */
void background_call_transfer_bool_value (
    Background *proxy,
    GVariant *arg_InArg,
    GCancellable *cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data);

gboolean background_call_transfer_bool_value_finish (
    Background *proxy,
    GAsyncResult *res,
    GError **error);

gboolean background_call_transfer_bool_value_sync (
    Background *proxy,
    GVariant *arg_InArg,
    GCancellable *cancellable,
    GError **error);

void background_call_transfer_string_value (
    Background *proxy,
    GVariant *arg_InArg,
    GCancellable *cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data);

gboolean background_call_transfer_string_value_finish (
    Background *proxy,
    GAsyncResult *res,
    GError **error);

gboolean background_call_transfer_string_value_sync (
    Background *proxy,
    GVariant *arg_InArg,
    GCancellable *cancellable,
    GError **error);

void background_call_transfer_int_value (
    Background *proxy,
    GVariant *arg_InArg,
    GCancellable *cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data);

gboolean background_call_transfer_int_value_finish (
    Background *proxy,
    GAsyncResult *res,
    GError **error);

gboolean background_call_transfer_int_value_sync (
    Background *proxy,
    GVariant *arg_InArg,
    GCancellable *cancellable,
    GError **error);



/* ---- */

#define TYPE_BACKGROUND_PROXY (background_proxy_get_type ())
#define BACKGROUND_PROXY(o) (G_TYPE_CHECK_INSTANCE_CAST ((o), TYPE_BACKGROUND_PROXY, BackgroundProxy))
#define BACKGROUND_PROXY_CLASS(k) (G_TYPE_CHECK_CLASS_CAST ((k), TYPE_BACKGROUND_PROXY, BackgroundProxyClass))
#define BACKGROUND_PROXY_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), TYPE_BACKGROUND_PROXY, BackgroundProxyClass))
#define IS_BACKGROUND_PROXY(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), TYPE_BACKGROUND_PROXY))
#define IS_BACKGROUND_PROXY_CLASS(k) (G_TYPE_CHECK_CLASS_TYPE ((k), TYPE_BACKGROUND_PROXY))

typedef struct _BackgroundProxy BackgroundProxy;
typedef struct _BackgroundProxyClass BackgroundProxyClass;
typedef struct _BackgroundProxyPrivate BackgroundProxyPrivate;

struct _BackgroundProxy
{
  /*< private >*/
  GDBusProxy parent_instance;
  BackgroundProxyPrivate *priv;
};

struct _BackgroundProxyClass
{
  GDBusProxyClass parent_class;
};

GType background_proxy_get_type (void) G_GNUC_CONST;

#if GLIB_CHECK_VERSION(2, 44, 0)
G_DEFINE_AUTOPTR_CLEANUP_FUNC (BackgroundProxy, g_object_unref)
#endif

void background_proxy_new (
    GDBusConnection     *connection,
    GDBusProxyFlags      flags,
    const gchar         *name,
    const gchar         *object_path,
    GCancellable        *cancellable,
    GAsyncReadyCallback  callback,
    gpointer             user_data);
Background *background_proxy_new_finish (
    GAsyncResult        *res,
    GError             **error);
Background *background_proxy_new_sync (
    GDBusConnection     *connection,
    GDBusProxyFlags      flags,
    const gchar         *name,
    const gchar         *object_path,
    GCancellable        *cancellable,
    GError             **error);

void background_proxy_new_for_bus (
    GBusType             bus_type,
    GDBusProxyFlags      flags,
    const gchar         *name,
    const gchar         *object_path,
    GCancellable        *cancellable,
    GAsyncReadyCallback  callback,
    gpointer             user_data);
Background *background_proxy_new_for_bus_finish (
    GAsyncResult        *res,
    GError             **error);
Background *background_proxy_new_for_bus_sync (
    GBusType             bus_type,
    GDBusProxyFlags      flags,
    const gchar         *name,
    const gchar         *object_path,
    GCancellable        *cancellable,
    GError             **error);


/* ---- */

#define TYPE_BACKGROUND_SKELETON (background_skeleton_get_type ())
#define BACKGROUND_SKELETON(o) (G_TYPE_CHECK_INSTANCE_CAST ((o), TYPE_BACKGROUND_SKELETON, BackgroundSkeleton))
#define BACKGROUND_SKELETON_CLASS(k) (G_TYPE_CHECK_CLASS_CAST ((k), TYPE_BACKGROUND_SKELETON, BackgroundSkeletonClass))
#define BACKGROUND_SKELETON_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), TYPE_BACKGROUND_SKELETON, BackgroundSkeletonClass))
#define IS_BACKGROUND_SKELETON(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), TYPE_BACKGROUND_SKELETON))
#define IS_BACKGROUND_SKELETON_CLASS(k) (G_TYPE_CHECK_CLASS_TYPE ((k), TYPE_BACKGROUND_SKELETON))

typedef struct _BackgroundSkeleton BackgroundSkeleton;
typedef struct _BackgroundSkeletonClass BackgroundSkeletonClass;
typedef struct _BackgroundSkeletonPrivate BackgroundSkeletonPrivate;

struct _BackgroundSkeleton
{
  /*< private >*/
  GDBusInterfaceSkeleton parent_instance;
  BackgroundSkeletonPrivate *priv;
};

struct _BackgroundSkeletonClass
{
  GDBusInterfaceSkeletonClass parent_class;
};

GType background_skeleton_get_type (void) G_GNUC_CONST;

#if GLIB_CHECK_VERSION(2, 44, 0)
G_DEFINE_AUTOPTR_CLEANUP_FUNC (BackgroundSkeleton, g_object_unref)
#endif

Background *background_skeleton_new (void);


G_END_DECLS

#endif /* __BACKGROUND_GENERATED_H__ */
