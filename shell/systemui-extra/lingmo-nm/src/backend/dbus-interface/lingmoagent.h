/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2022 Tianjin LINGMO Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#ifndef __LINGMONMANGENT_H__
#define __LINGMONMANGENT_H__

#ifdef __cplusplus
extern "C"{
#endif

#include <libnm/NetworkManager.h>
#include <libnm/nm-secret-agent-old.h>
#include <gio/ginitable.h>
#include <gio/gdbusintrospection.h>
#include <libsecret/secret.h>

#define APPLET_TYPE_AGENT            (applet_agent_get_type ())
#define APPLET_AGENT(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), APPLET_TYPE_AGENT, AppletAgent))
#define APPLET_AGENT_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), APPLET_TYPE_AGENT, AppletAgentClass))
#define APPLET_IS_AGENT(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), APPLET_TYPE_AGENT))
#define APPLET_IS_AGENT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), APPLET_TYPE_AGENT))
#define APPLET_AGENT_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), APPLET_TYPE_AGENT, AppletAgentClass))

#define APPLET_AGENT_GET_SECRETS "get-secrets"
#define APPLET_AGENT_CANCEL_SECRETS "cancel-secrets"

typedef struct {
    NMSecretAgentOld parent;
} AppletAgent;

typedef void (*AppletAgentSecretsCallback) (AppletAgent *self,
                                            GVariant *secrets,
                                            GError *error,
                                            gpointer user_data);

typedef struct {
    NMSecretAgentOldClass parent_class;

    void (*get_secrets)        (AppletAgent *self,
                                void *request_id,
                                NMConnection *connection,
                                const char *setting_name,
                                const char **hints,
                                guint32 flags,
                                AppletAgentSecretsCallback callback,
                                gpointer callback_data);

    void (*cancel_secrets)     (AppletAgent *self,
                                void *request_id);
} AppletAgentClass;


GType applet_agent_get_type (void) G_GNUC_CONST;

AppletAgent *applet_agent_new (GError **error);

void applet_agent_handle_vpn_only (AppletAgent *agent, gboolean vpn_only);

#ifdef __cplusplus
}
#endif

#endif
