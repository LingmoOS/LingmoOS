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

#ifndef __XKBGENERAL_GENERATED_H__
#define __XKBGENERAL_GENERATED_H__

#include <gio/gio.h>

G_BEGIN_DECLS


/* ------------------------------------------------------------------------ */
/* Declarations for cn.lingmoos.xkbgeneral */

#define TYPE_XKBGENERAL (xkbgeneral_get_type ())
#define XKBGENERAL(o) (G_TYPE_CHECK_INSTANCE_CAST ((o), TYPE_XKBGENERAL, Xkbgeneral))
#define IS_XKBGENERAL(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), TYPE_XKBGENERAL))
#define XKBGENERAL_GET_IFACE(o) (G_TYPE_INSTANCE_GET_INTERFACE ((o), TYPE_XKBGENERAL, XkbgeneralIface))

struct _Xkbgeneral;
typedef struct _Xkbgeneral Xkbgeneral;
typedef struct _XkbgeneralIface XkbgeneralIface;

struct _XkbgeneralIface
{
  GTypeInterface parent_iface;

  gboolean (*handle_get_bool_value) (
    Xkbgeneral *object,
    GDBusMethodInvocation *invocation,
    const gchar *arg_InArg);

  gboolean (*handle_transfer_bool_value) (
    Xkbgeneral *object,
    GDBusMethodInvocation *invocation,
    GVariant *arg_InArg);

};

GType xkbgeneral_get_type (void) G_GNUC_CONST;

GDBusInterfaceInfo *xkbgeneral_interface_info (void);
guint xkbgeneral_override_properties (GObjectClass *klass, guint property_id_begin);


/* D-Bus method call completion functions: */
void xkbgeneral_complete_transfer_bool_value (
    Xkbgeneral *object,
    GDBusMethodInvocation *invocation,
    gboolean OutArg);

void xkbgeneral_complete_get_bool_value (
    Xkbgeneral *object,
    GDBusMethodInvocation *invocation,
    gboolean OutArg);



/* D-Bus method calls: */
void xkbgeneral_call_transfer_bool_value (
    Xkbgeneral *proxy,
    GVariant *arg_InArg,
    GCancellable *cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data);

gboolean xkbgeneral_call_transfer_bool_value_finish (
    Xkbgeneral *proxy,
    gboolean *out_OutArg,
    GAsyncResult *res,
    GError **error);

gboolean xkbgeneral_call_transfer_bool_value_sync (
    Xkbgeneral *proxy,
    GVariant *arg_InArg,
    gboolean *out_OutArg,
    GCancellable *cancellable,
    GError **error);

void xkbgeneral_call_get_bool_value (
    Xkbgeneral *proxy,
    const gchar *arg_InArg,
    GCancellable *cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data);

gboolean xkbgeneral_call_get_bool_value_finish (
    Xkbgeneral *proxy,
    gboolean *out_OutArg,
    GAsyncResult *res,
    GError **error);

gboolean xkbgeneral_call_get_bool_value_sync (
    Xkbgeneral *proxy,
    const gchar *arg_InArg,
    gboolean *out_OutArg,
    GCancellable *cancellable,
    GError **error);



/* ---- */

#define TYPE_XKBGENERAL_PROXY (xkbgeneral_proxy_get_type ())
#define XKBGENERAL_PROXY(o) (G_TYPE_CHECK_INSTANCE_CAST ((o), TYPE_XKBGENERAL_PROXY, XkbgeneralProxy))
#define XKBGENERAL_PROXY_CLASS(k) (G_TYPE_CHECK_CLASS_CAST ((k), TYPE_XKBGENERAL_PROXY, XkbgeneralProxyClass))
#define XKBGENERAL_PROXY_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), TYPE_XKBGENERAL_PROXY, XkbgeneralProxyClass))
#define IS_XKBGENERAL_PROXY(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), TYPE_XKBGENERAL_PROXY))
#define IS_XKBGENERAL_PROXY_CLASS(k) (G_TYPE_CHECK_CLASS_TYPE ((k), TYPE_XKBGENERAL_PROXY))

typedef struct _XkbgeneralProxy XkbgeneralProxy;
typedef struct _XkbgeneralProxyClass XkbgeneralProxyClass;
typedef struct _XkbgeneralProxyPrivate XkbgeneralProxyPrivate;

struct _XkbgeneralProxy
{
  /*< private >*/
  GDBusProxy parent_instance;
  XkbgeneralProxyPrivate *priv;
};

struct _XkbgeneralProxyClass
{
  GDBusProxyClass parent_class;
};

GType xkbgeneral_proxy_get_type (void) G_GNUC_CONST;

#if GLIB_CHECK_VERSION(2, 44, 0)
G_DEFINE_AUTOPTR_CLEANUP_FUNC (XkbgeneralProxy, g_object_unref)
#endif

void xkbgeneral_proxy_new (
    GDBusConnection     *connection,
    GDBusProxyFlags      flags,
    const gchar         *name,
    const gchar         *object_path,
    GCancellable        *cancellable,
    GAsyncReadyCallback  callback,
    gpointer             user_data);
Xkbgeneral *xkbgeneral_proxy_new_finish (
    GAsyncResult        *res,
    GError             **error);
Xkbgeneral *xkbgeneral_proxy_new_sync (
    GDBusConnection     *connection,
    GDBusProxyFlags      flags,
    const gchar         *name,
    const gchar         *object_path,
    GCancellable        *cancellable,
    GError             **error);

void xkbgeneral_proxy_new_for_bus (
    GBusType             bus_type,
    GDBusProxyFlags      flags,
    const gchar         *name,
    const gchar         *object_path,
    GCancellable        *cancellable,
    GAsyncReadyCallback  callback,
    gpointer             user_data);
Xkbgeneral *xkbgeneral_proxy_new_for_bus_finish (
    GAsyncResult        *res,
    GError             **error);
Xkbgeneral *xkbgeneral_proxy_new_for_bus_sync (
    GBusType             bus_type,
    GDBusProxyFlags      flags,
    const gchar         *name,
    const gchar         *object_path,
    GCancellable        *cancellable,
    GError             **error);


/* ---- */

#define TYPE_XKBGENERAL_SKELETON (xkbgeneral_skeleton_get_type ())
#define XKBGENERAL_SKELETON(o) (G_TYPE_CHECK_INSTANCE_CAST ((o), TYPE_XKBGENERAL_SKELETON, XkbgeneralSkeleton))
#define XKBGENERAL_SKELETON_CLASS(k) (G_TYPE_CHECK_CLASS_CAST ((k), TYPE_XKBGENERAL_SKELETON, XkbgeneralSkeletonClass))
#define XKBGENERAL_SKELETON_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), TYPE_XKBGENERAL_SKELETON, XkbgeneralSkeletonClass))
#define IS_XKBGENERAL_SKELETON(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), TYPE_XKBGENERAL_SKELETON))
#define IS_XKBGENERAL_SKELETON_CLASS(k) (G_TYPE_CHECK_CLASS_TYPE ((k), TYPE_XKBGENERAL_SKELETON))

typedef struct _XkbgeneralSkeleton XkbgeneralSkeleton;
typedef struct _XkbgeneralSkeletonClass XkbgeneralSkeletonClass;
typedef struct _XkbgeneralSkeletonPrivate XkbgeneralSkeletonPrivate;

struct _XkbgeneralSkeleton
{
  /*< private >*/
  GDBusInterfaceSkeleton parent_instance;
  XkbgeneralSkeletonPrivate *priv;
};

struct _XkbgeneralSkeletonClass
{
  GDBusInterfaceSkeletonClass parent_class;
};

GType xkbgeneral_skeleton_get_type (void) G_GNUC_CONST;

#if GLIB_CHECK_VERSION(2, 44, 0)
G_DEFINE_AUTOPTR_CLEANUP_FUNC (XkbgeneralSkeleton, g_object_unref)
#endif

Xkbgeneral *xkbgeneral_skeleton_new (void);


G_END_DECLS

#endif /* __XKBGENERAL_GENERATED_H__ */
