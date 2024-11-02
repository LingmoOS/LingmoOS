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
#ifndef LINGMO_VPN_REQUEST_H
#define LINGMO_VPN_REQUEST_H

#include <gtk/gtk.h>
#include "lingmoagent.h"

typedef struct _SecretsRequest SecretsRequest;
typedef void (*SecretsRequestFreeFunc) (SecretsRequest *req);
struct _SecretsRequest {
         size_t totsize;
         gpointer reqid;
         char *setting_name;
         char **hints;
         guint32 flags;
         AppletAgent *agent;
         AppletAgentSecretsCallback callback;
         gpointer callback_data;

         NMConnection *connection;

         /* Class-specific stuff */
         SecretsRequestFreeFunc free_func;
};


size_t applet_vpn_request_get_secrets_size(void);

gboolean applet_vpn_request_get_secrets(SecretsRequest *req, GError **error);

#endif  /* APPLET_VPN_REQUEST_H */
