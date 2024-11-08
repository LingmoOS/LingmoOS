/*
    SPDX-FileCopyrightText: 2022 iaom <zhangpengfei@kylinos.cn>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/
#ifndef LINGMOFILEMETADATA_GLOBAL_H
#define LINGMOFILEMETADATA_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(LINGMOFILEMETADATA_LIBRARY)
#  define LINGMOFILEMETADATA_EXPORT Q_DECL_EXPORT
#else
#  define LINGMOFILEMETADATA_EXPORT Q_DECL_IMPORT
#endif

#endif // LINGMOFILEMETADATA_GLOBAL_H
