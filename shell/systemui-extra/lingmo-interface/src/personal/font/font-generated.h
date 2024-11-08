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

#ifndef __FONT_GENERATED_H__
#define __FONT_GENERATED_H__

#include <gio/gio.h>

G_BEGIN_DECLS


/* ------------------------------------------------------------------------ */
/* Declarations for cn.lingmoos.font */

#define TYPE_FONT (font_get_type ())
#define FONT(o) (G_TYPE_CHECK_INSTANCE_CAST ((o), TYPE_FONT, Font))
#define IS_FONT(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), TYPE_FONT))
#define FONT_GET_IFACE(o) (G_TYPE_INSTANCE_GET_INTERFACE ((o), TYPE_FONT, FontIface))

struct _Font;
typedef struct _Font Font;
typedef struct _FontIface FontIface;

struct _FontIface
{
  GTypeInterface parent_iface;

  gboolean (*handle_transfer_double_value) (
    Font *object,
    GDBusMethodInvocation *invocation,
    GVariant *arg_InArg);

  gboolean (*handle_transfer_int_value) (
    Font *object,
    GDBusMethodInvocation *invocation,
    GVariant *arg_InArg);

};

GType font_get_type (void) G_GNUC_CONST;

GDBusInterfaceInfo *font_interface_info (void);
guint font_override_properties (GObjectClass *klass, guint property_id_begin);


/* D-Bus method call completion functions: */
void font_complete_transfer_int_value (
    Font *object,
    GDBusMethodInvocation *invocation);

void font_complete_transfer_double_value (
    Font *object,
    GDBusMethodInvocation *invocation);



/* D-Bus method calls: */
void font_call_transfer_int_value (
    Font *proxy,
    GVariant *arg_InArg,
    GCancellable *cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data);

gboolean font_call_transfer_int_value_finish (
    Font *proxy,
    GAsyncResult *res,
    GError **error);

gboolean font_call_transfer_int_value_sync (
    Font *proxy,
    GVariant *arg_InArg,
    GCancellable *cancellable,
    GError **error);

void font_call_transfer_double_value (
    Font *proxy,
    GVariant *arg_InArg,
    GCancellable *cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data);

gboolean font_call_transfer_double_value_finish (
    Font *proxy,
    GAsyncResult *res,
    GError **error);

gboolean font_call_transfer_double_value_sync (
    Font *proxy,
    GVariant *arg_InArg,
    GCancellable *cancellable,
    GError **error);



/* ---- */

#define TYPE_FONT_PROXY (font_proxy_get_type ())
#define FONT_PROXY(o) (G_TYPE_CHECK_INSTANCE_CAST ((o), TYPE_FONT_PROXY, FontProxy))
#define FONT_PROXY_CLASS(k) (G_TYPE_CHECK_CLASS_CAST ((k), TYPE_FONT_PROXY, FontProxyClass))
#define FONT_PROXY_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), TYPE_FONT_PROXY, FontProxyClass))
#define IS_FONT_PROXY(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), TYPE_FONT_PROXY))
#define IS_FONT_PROXY_CLASS(k) (G_TYPE_CHECK_CLASS_TYPE ((k), TYPE_FONT_PROXY))

typedef struct _FontProxy FontProxy;
typedef struct _FontProxyClass FontProxyClass;
typedef struct _FontProxyPrivate FontProxyPrivate;

struct _FontProxy
{
  /*< private >*/
  GDBusProxy parent_instance;
  FontProxyPrivate *priv;
};

struct _FontProxyClass
{
  GDBusProxyClass parent_class;
};

GType font_proxy_get_type (void) G_GNUC_CONST;

#if GLIB_CHECK_VERSION(2, 44, 0)
G_DEFINE_AUTOPTR_CLEANUP_FUNC (FontProxy, g_object_unref)
#endif

void font_proxy_new (
    GDBusConnection     *connection,
    GDBusProxyFlags      flags,
    const gchar         *name,
    const gchar         *object_path,
    GCancellable        *cancellable,
    GAsyncReadyCallback  callback,
    gpointer             user_data);
Font *font_proxy_new_finish (
    GAsyncResult        *res,
    GError             **error);
Font *font_proxy_new_sync (
    GDBusConnection     *connection,
    GDBusProxyFlags      flags,
    const gchar         *name,
    const gchar         *object_path,
    GCancellable        *cancellable,
    GError             **error);

void font_proxy_new_for_bus (
    GBusType             bus_type,
    GDBusProxyFlags      flags,
    const gchar         *name,
    const gchar         *object_path,
    GCancellable        *cancellable,
    GAsyncReadyCallback  callback,
    gpointer             user_data);
Font *font_proxy_new_for_bus_finish (
    GAsyncResult        *res,
    GError             **error);
Font *font_proxy_new_for_bus_sync (
    GBusType             bus_type,
    GDBusProxyFlags      flags,
    const gchar         *name,
    const gchar         *object_path,
    GCancellable        *cancellable,
    GError             **error);


/* ---- */

#define TYPE_FONT_SKELETON (font_skeleton_get_type ())
#define FONT_SKELETON(o) (G_TYPE_CHECK_INSTANCE_CAST ((o), TYPE_FONT_SKELETON, FontSkeleton))
#define FONT_SKELETON_CLASS(k) (G_TYPE_CHECK_CLASS_CAST ((k), TYPE_FONT_SKELETON, FontSkeletonClass))
#define FONT_SKELETON_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), TYPE_FONT_SKELETON, FontSkeletonClass))
#define IS_FONT_SKELETON(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), TYPE_FONT_SKELETON))
#define IS_FONT_SKELETON_CLASS(k) (G_TYPE_CHECK_CLASS_TYPE ((k), TYPE_FONT_SKELETON))

typedef struct _FontSkeleton FontSkeleton;
typedef struct _FontSkeletonClass FontSkeletonClass;
typedef struct _FontSkeletonPrivate FontSkeletonPrivate;

struct _FontSkeleton
{
  /*< private >*/
  GDBusInterfaceSkeleton parent_instance;
  FontSkeletonPrivate *priv;
};

struct _FontSkeletonClass
{
  GDBusInterfaceSkeletonClass parent_class;
};

GType font_skeleton_get_type (void) G_GNUC_CONST;

#if GLIB_CHECK_VERSION(2, 44, 0)
G_DEFINE_AUTOPTR_CLEANUP_FUNC (FontSkeleton, g_object_unref)
#endif

Font *font_skeleton_new (void);


G_END_DECLS

#endif /* __FONT_GENERATED_H__ */
