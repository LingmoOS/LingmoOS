// SPDX-FileCopyrightText: 2022 lpxxn <mi_duo@live.com>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DOCX_GLOBAL_H
#define DOCX_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(DOCX_LIBRARY)
#  define DOCX_EXPORT Q_DECL_EXPORT
#else
#  define DOCX_EXPORT Q_DECL_IMPORT
#endif

/*!
 * \defgroup Docx 操作微软docx文件的模块.
 *
 *
 */

#endif // DOCX_GLOBAL_H
