/* -*- Mode: C++; indent-tabs-mode: nil; tab-width: 4 -*-
 * -*- coding: utf-8 -*-
 *
 * Copyright (C) 2020 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef QCONFTYPE_H
#define QCONFTYPE_H

#include <glib.h>
#include <QVariant>

QVariant::Type qconf_types_convert (const GVariantType* gtype);
GVariant* qconf_types_collect (const GVariantType* gtype, const void* argument);
GVariant* qconf_types_collect_from_variant(const GVariantType* gtype, const QVariant& v);
QVariant qconf_types_to_qvariant (GVariant* value);
void qconf_types_unpack (GVariant* value, void* argument);

QString qtify_name(const char *name);
gchar * unqtify_name(const QString &name);

#endif // QCONFTYPE_H
