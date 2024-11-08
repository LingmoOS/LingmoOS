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

#ifndef __MARCOGENERAL_GENERATED_H__
#define __MARCOGENERAL_GENERATED_H__

#include <gio/gio.h>

G_BEGIN_DECLS


/* ------------------------------------------------------------------------ */
/* Declarations for cn.lingmoos.marcogeneral */

#define TYPE_MARCOGENERAL (marcogeneral_get_type ())
#define MARCOGENERAL(o) (G_TYPE_CHECK_INSTANCE_CAST ((o), TYPE_MARCOGENERAL, Marcogeneral))
#define IS_MARCOGENERAL(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), TYPE_MARCOGENERAL))
#define MARCOGENERAL_GET_IFACE(o) (G_TYPE_INSTANCE_GET_INTERFACE ((o), TYPE_MARCOGENERAL, MarcogeneralIface))

struct _Marcogeneral;
typedef struct _Marcogeneral Marcogeneral;
typedef struct _MarcogeneralIface MarcogeneralIface;

struct _MarcogeneralIface
{
  GTypeInterface parent_iface;

  gboolean (*handle_transfer_string_value) (
    Marcogeneral *object,
    GDBusMethodInvocation *invocation,
    GVariant *arg_InArg);

};

GType marcogeneral_get_type (void) G_GNUC_CONST;

GDBusInterfaceInfo *marcogeneral_interface_info (void);
guint marcogeneral_override_properties (GObjectClass *klass, guint property_id_begin);


/* D-Bus method call completion functions: */
void marcogeneral_complete_transfer_string_value (
    Marcogeneral *object,
    GDBusMethodInvocation *invocation);



/* D-Bus method calls: */
void marcogeneral_call_transfer_string_value (
    Marcogeneral *proxy,
    GVariant *arg_InArg,
    GCancellable *cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data);

gboolean marcogeneral_call_transfer_string_value_finish (
    Marcogeneral *proxy,
    GAsyncResult *res,
    GError **error);

gboolean marcogeneral_call_transfer_string_value_sync (
    Marcogeneral *proxy,
    GVariant *arg_InArg,
    GCancellable *cancellable,
    GError **error);



/* ---- */

#define TYPE_MARCOGENERAL_PROXY (marcogeneral_proxy_get_type ())
#define MARCOGENERAL_PROXY(o) (G_TYPE_CHECK_INSTANCE_CAST ((o), TYPE_MARCOGENERAL_PROXY, MarcogeneralProxy))
#define MARCOGENERAL_PROXY_CLASS(k) (G_TYPE_CHECK_CLASS_CAST ((k), TYPE_MARCOGENERAL_PROXY, MarcogeneralProxyClass))
#define MARCOGENERAL_PROXY_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), TYPE_MARCOGENERAL_PROXY, MarcogeneralProxyClass))
#define IS_MARCOGENERAL_PROXY(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), TYPE_MARCOGENERAL_PROXY))
#define IS_MARCOGENERAL_PROXY_CLASS(k) (G_TYPE_CHECK_CLASS_TYPE ((k), TYPE_MARCOGENERAL_PROXY))

typedef struct _MarcogeneralProxy MarcogeneralProxy;
typedef struct _MarcogeneralProxyClass MarcogeneralProxyClass;
typedef struct _MarcogeneralProxyPrivate MarcogeneralProxyPrivate;

struct _MarcogeneralProxy
{
  /*< private >*/
  GDBusProxy parent_instance;
  MarcogeneralProxyPrivate *priv;
};

struct _MarcogeneralProxyClass
{
  GDBusProxyClass parent_class;
};

GType marcogeneral_proxy_get_type (void) G_GNUC_CONST;

#if GLIB_CHECK_VERSION(2, 44, 0)
G_DEFINE_AUTOPTR_CLEANUP_FUNC (MarcogeneralProxy, g_object_unref)
#endif

void marcogeneral_proxy_new (
    GDBusConnection     *connection,
    GDBusProxyFlags      flags,
    const gchar         *name,
    const gchar         *object_path,
    GCancellable        *cancellable,
    GAsyncReadyCallback  callback,
    gpointer             user_data);
Marcogeneral *marcogeneral_proxy_new_finish (
    GAsyncResult        *res,
    GError             **error);
Marcogeneral *marcogeneral_proxy_new_sync (
    GDBusConnection     *connection,
    GDBusProxyFlags      flags,
    const gchar         *name,
    const gchar         *object_path,
    GCancellable        *cancellable,
    GError             **error);

void marcogeneral_proxy_new_for_bus (
    GBusType             bus_type,
    GDBusProxyFlags      flags,
    const gchar         *name,
    const gchar         *object_path,
    GCancellable        *cancellable,
    GAsyncReadyCallback  callback,
    gpointer             user_data);
Marcogeneral *marcogeneral_proxy_new_for_bus_finish (
    GAsyncResult        *res,
    GError             **error);
Marcogeneral *marcogeneral_proxy_new_for_bus_sync (
    GBusType             bus_type,
    GDBusProxyFlags      flags,
    const gchar         *name,
    const gchar         *object_path,
    GCancellable        *cancellable,
    GError             **error);


/* ---- */

#define TYPE_MARCOGENERAL_SKELETON (marcogeneral_skeleton_get_type ())
#define MARCOGENERAL_SKELETON(o) (G_TYPE_CHECK_INSTANCE_CAST ((o), TYPE_MARCOGENERAL_SKELETON, MarcogeneralSkeleton))
#define MARCOGENERAL_SKELETON_CLASS(k) (G_TYPE_CHECK_CLASS_CAST ((k), TYPE_MARCOGENERAL_SKELETON, MarcogeneralSkeletonClass))
#define MARCOGENERAL_SKELETON_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), TYPE_MARCOGENERAL_SKELETON, MarcogeneralSkeletonClass))
#define IS_MARCOGENERAL_SKELETON(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), TYPE_MARCOGENERAL_SKELETON))
#define IS_MARCOGENERAL_SKELETON_CLASS(k) (G_TYPE_CHECK_CLASS_TYPE ((k), TYPE_MARCOGENERAL_SKELETON))

typedef struct _MarcogeneralSkeleton MarcogeneralSkeleton;
typedef struct _MarcogeneralSkeletonClass MarcogeneralSkeletonClass;
typedef struct _MarcogeneralSkeletonPrivate MarcogeneralSkeletonPrivate;

struct _MarcogeneralSkeleton
{
  /*< private >*/
  GDBusInterfaceSkeleton parent_instance;
  MarcogeneralSkeletonPrivate *priv;
};

struct _MarcogeneralSkeletonClass
{
  GDBusInterfaceSkeletonClass parent_class;
};

GType marcogeneral_skeleton_get_type (void) G_GNUC_CONST;

#if GLIB_CHECK_VERSION(2, 44, 0)
G_DEFINE_AUTOPTR_CLEANUP_FUNC (MarcogeneralSkeleton, g_object_unref)
#endif

Marcogeneral *marcogeneral_skeleton_new (void);


G_END_DECLS

#endif /* __MARCOGENERAL_GENERATED_H__ */
