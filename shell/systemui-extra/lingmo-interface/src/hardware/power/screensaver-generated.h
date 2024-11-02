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

#ifndef __SCREENSAVER_GENERATED_H__
#define __SCREENSAVER_GENERATED_H__

#include <gio/gio.h>

G_BEGIN_DECLS


/* ------------------------------------------------------------------------ */
/* Declarations for cn.lingmoos.screensaver */

#define TYPE_SCREENSAVER (screensaver_get_type ())
#define SCREENSAVER(o) (G_TYPE_CHECK_INSTANCE_CAST ((o), TYPE_SCREENSAVER, Screensaver))
#define IS_SCREENSAVER(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), TYPE_SCREENSAVER))
#define SCREENSAVER_GET_IFACE(o) (G_TYPE_INSTANCE_GET_INTERFACE ((o), TYPE_SCREENSAVER, ScreensaverIface))

struct _Screensaver;
typedef struct _Screensaver Screensaver;
typedef struct _ScreensaverIface ScreensaverIface;

struct _ScreensaverIface
{
  GTypeInterface parent_iface;

  gboolean (*handle_transfer_bool_value) (
    Screensaver *object,
    GDBusMethodInvocation *invocation,
    GVariant *arg_InArg);

};

GType screensaver_get_type (void) G_GNUC_CONST;

GDBusInterfaceInfo *screensaver_interface_info (void);
guint screensaver_override_properties (GObjectClass *klass, guint property_id_begin);


/* D-Bus method call completion functions: */
void screensaver_complete_transfer_bool_value (
    Screensaver *object,
    GDBusMethodInvocation *invocation);



/* D-Bus method calls: */
void screensaver_call_transfer_bool_value (
    Screensaver *proxy,
    GVariant *arg_InArg,
    GCancellable *cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data);

gboolean screensaver_call_transfer_bool_value_finish (
    Screensaver *proxy,
    GAsyncResult *res,
    GError **error);

gboolean screensaver_call_transfer_bool_value_sync (
    Screensaver *proxy,
    GVariant *arg_InArg,
    GCancellable *cancellable,
    GError **error);



/* ---- */

#define TYPE_SCREENSAVER_PROXY (screensaver_proxy_get_type ())
#define SCREENSAVER_PROXY(o) (G_TYPE_CHECK_INSTANCE_CAST ((o), TYPE_SCREENSAVER_PROXY, ScreensaverProxy))
#define SCREENSAVER_PROXY_CLASS(k) (G_TYPE_CHECK_CLASS_CAST ((k), TYPE_SCREENSAVER_PROXY, ScreensaverProxyClass))
#define SCREENSAVER_PROXY_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), TYPE_SCREENSAVER_PROXY, ScreensaverProxyClass))
#define IS_SCREENSAVER_PROXY(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), TYPE_SCREENSAVER_PROXY))
#define IS_SCREENSAVER_PROXY_CLASS(k) (G_TYPE_CHECK_CLASS_TYPE ((k), TYPE_SCREENSAVER_PROXY))

typedef struct _ScreensaverProxy ScreensaverProxy;
typedef struct _ScreensaverProxyClass ScreensaverProxyClass;
typedef struct _ScreensaverProxyPrivate ScreensaverProxyPrivate;

struct _ScreensaverProxy
{
  /*< private >*/
  GDBusProxy parent_instance;
  ScreensaverProxyPrivate *priv;
};

struct _ScreensaverProxyClass
{
  GDBusProxyClass parent_class;
};

GType screensaver_proxy_get_type (void) G_GNUC_CONST;

#if GLIB_CHECK_VERSION(2, 44, 0)
G_DEFINE_AUTOPTR_CLEANUP_FUNC (ScreensaverProxy, g_object_unref)
#endif

void screensaver_proxy_new (
    GDBusConnection     *connection,
    GDBusProxyFlags      flags,
    const gchar         *name,
    const gchar         *object_path,
    GCancellable        *cancellable,
    GAsyncReadyCallback  callback,
    gpointer             user_data);
Screensaver *screensaver_proxy_new_finish (
    GAsyncResult        *res,
    GError             **error);
Screensaver *screensaver_proxy_new_sync (
    GDBusConnection     *connection,
    GDBusProxyFlags      flags,
    const gchar         *name,
    const gchar         *object_path,
    GCancellable        *cancellable,
    GError             **error);

void screensaver_proxy_new_for_bus (
    GBusType             bus_type,
    GDBusProxyFlags      flags,
    const gchar         *name,
    const gchar         *object_path,
    GCancellable        *cancellable,
    GAsyncReadyCallback  callback,
    gpointer             user_data);
Screensaver *screensaver_proxy_new_for_bus_finish (
    GAsyncResult        *res,
    GError             **error);
Screensaver *screensaver_proxy_new_for_bus_sync (
    GBusType             bus_type,
    GDBusProxyFlags      flags,
    const gchar         *name,
    const gchar         *object_path,
    GCancellable        *cancellable,
    GError             **error);


/* ---- */

#define TYPE_SCREENSAVER_SKELETON (screensaver_skeleton_get_type ())
#define SCREENSAVER_SKELETON(o) (G_TYPE_CHECK_INSTANCE_CAST ((o), TYPE_SCREENSAVER_SKELETON, ScreensaverSkeleton))
#define SCREENSAVER_SKELETON_CLASS(k) (G_TYPE_CHECK_CLASS_CAST ((k), TYPE_SCREENSAVER_SKELETON, ScreensaverSkeletonClass))
#define SCREENSAVER_SKELETON_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), TYPE_SCREENSAVER_SKELETON, ScreensaverSkeletonClass))
#define IS_SCREENSAVER_SKELETON(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), TYPE_SCREENSAVER_SKELETON))
#define IS_SCREENSAVER_SKELETON_CLASS(k) (G_TYPE_CHECK_CLASS_TYPE ((k), TYPE_SCREENSAVER_SKELETON))

typedef struct _ScreensaverSkeleton ScreensaverSkeleton;
typedef struct _ScreensaverSkeletonClass ScreensaverSkeletonClass;
typedef struct _ScreensaverSkeletonPrivate ScreensaverSkeletonPrivate;

struct _ScreensaverSkeleton
{
  /*< private >*/
  GDBusInterfaceSkeleton parent_instance;
  ScreensaverSkeletonPrivate *priv;
};

struct _ScreensaverSkeletonClass
{
  GDBusInterfaceSkeletonClass parent_class;
};

GType screensaver_skeleton_get_type (void) G_GNUC_CONST;

#if GLIB_CHECK_VERSION(2, 44, 0)
G_DEFINE_AUTOPTR_CLEANUP_FUNC (ScreensaverSkeleton, g_object_unref)
#endif

Screensaver *screensaver_skeleton_new (void);


G_END_DECLS

#endif /* __SCREENSAVER_GENERATED_H__ */
