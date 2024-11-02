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

#ifndef __POWER_GENERATED_H__
#define __POWER_GENERATED_H__

#include <gio/gio.h>

G_BEGIN_DECLS


/* ------------------------------------------------------------------------ */
/* Declarations for cn.lingmoos.power */

#define TYPE_POWER (power_get_type ())
#define POWER(o) (G_TYPE_CHECK_INSTANCE_CAST ((o), TYPE_POWER, Power))
#define IS_POWER(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), TYPE_POWER))
#define POWER_GET_IFACE(o) (G_TYPE_INSTANCE_GET_INTERFACE ((o), TYPE_POWER, PowerIface))

struct _Power;
typedef struct _Power Power;
typedef struct _PowerIface PowerIface;

struct _PowerIface
{
  GTypeInterface parent_iface;

  gboolean (*handle_transfer_int_value) (
    Power *object,
    GDBusMethodInvocation *invocation,
    GVariant *arg_InArg);

};

GType power_get_type (void) G_GNUC_CONST;

GDBusInterfaceInfo *power_interface_info (void);
guint power_override_properties (GObjectClass *klass, guint property_id_begin);


/* D-Bus method call completion functions: */
void power_complete_transfer_int_value (
    Power *object,
    GDBusMethodInvocation *invocation);



/* D-Bus method calls: */
void power_call_transfer_int_value (
    Power *proxy,
    GVariant *arg_InArg,
    GCancellable *cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data);

gboolean power_call_transfer_int_value_finish (
    Power *proxy,
    GAsyncResult *res,
    GError **error);

gboolean power_call_transfer_int_value_sync (
    Power *proxy,
    GVariant *arg_InArg,
    GCancellable *cancellable,
    GError **error);



/* ---- */

#define TYPE_POWER_PROXY (power_proxy_get_type ())
#define POWER_PROXY(o) (G_TYPE_CHECK_INSTANCE_CAST ((o), TYPE_POWER_PROXY, PowerProxy))
#define POWER_PROXY_CLASS(k) (G_TYPE_CHECK_CLASS_CAST ((k), TYPE_POWER_PROXY, PowerProxyClass))
#define POWER_PROXY_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), TYPE_POWER_PROXY, PowerProxyClass))
#define IS_POWER_PROXY(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), TYPE_POWER_PROXY))
#define IS_POWER_PROXY_CLASS(k) (G_TYPE_CHECK_CLASS_TYPE ((k), TYPE_POWER_PROXY))

typedef struct _PowerProxy PowerProxy;
typedef struct _PowerProxyClass PowerProxyClass;
typedef struct _PowerProxyPrivate PowerProxyPrivate;

struct _PowerProxy
{
  /*< private >*/
  GDBusProxy parent_instance;
  PowerProxyPrivate *priv;
};

struct _PowerProxyClass
{
  GDBusProxyClass parent_class;
};

GType power_proxy_get_type (void) G_GNUC_CONST;

#if GLIB_CHECK_VERSION(2, 44, 0)
G_DEFINE_AUTOPTR_CLEANUP_FUNC (PowerProxy, g_object_unref)
#endif

void power_proxy_new (
    GDBusConnection     *connection,
    GDBusProxyFlags      flags,
    const gchar         *name,
    const gchar         *object_path,
    GCancellable        *cancellable,
    GAsyncReadyCallback  callback,
    gpointer             user_data);
Power *power_proxy_new_finish (
    GAsyncResult        *res,
    GError             **error);
Power *power_proxy_new_sync (
    GDBusConnection     *connection,
    GDBusProxyFlags      flags,
    const gchar         *name,
    const gchar         *object_path,
    GCancellable        *cancellable,
    GError             **error);

void power_proxy_new_for_bus (
    GBusType             bus_type,
    GDBusProxyFlags      flags,
    const gchar         *name,
    const gchar         *object_path,
    GCancellable        *cancellable,
    GAsyncReadyCallback  callback,
    gpointer             user_data);
Power *power_proxy_new_for_bus_finish (
    GAsyncResult        *res,
    GError             **error);
Power *power_proxy_new_for_bus_sync (
    GBusType             bus_type,
    GDBusProxyFlags      flags,
    const gchar         *name,
    const gchar         *object_path,
    GCancellable        *cancellable,
    GError             **error);


/* ---- */

#define TYPE_POWER_SKELETON (power_skeleton_get_type ())
#define POWER_SKELETON(o) (G_TYPE_CHECK_INSTANCE_CAST ((o), TYPE_POWER_SKELETON, PowerSkeleton))
#define POWER_SKELETON_CLASS(k) (G_TYPE_CHECK_CLASS_CAST ((k), TYPE_POWER_SKELETON, PowerSkeletonClass))
#define POWER_SKELETON_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), TYPE_POWER_SKELETON, PowerSkeletonClass))
#define IS_POWER_SKELETON(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), TYPE_POWER_SKELETON))
#define IS_POWER_SKELETON_CLASS(k) (G_TYPE_CHECK_CLASS_TYPE ((k), TYPE_POWER_SKELETON))

typedef struct _PowerSkeleton PowerSkeleton;
typedef struct _PowerSkeletonClass PowerSkeletonClass;
typedef struct _PowerSkeletonPrivate PowerSkeletonPrivate;

struct _PowerSkeleton
{
  /*< private >*/
  GDBusInterfaceSkeleton parent_instance;
  PowerSkeletonPrivate *priv;
};

struct _PowerSkeletonClass
{
  GDBusInterfaceSkeletonClass parent_class;
};

GType power_skeleton_get_type (void) G_GNUC_CONST;

#if GLIB_CHECK_VERSION(2, 44, 0)
G_DEFINE_AUTOPTR_CLEANUP_FUNC (PowerSkeleton, g_object_unref)
#endif

Power *power_skeleton_new (void);


G_END_DECLS

#endif /* __POWER_GENERATED_H__ */
