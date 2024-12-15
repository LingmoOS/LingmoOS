// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef MIMETYPES_H
#define MIMETYPES_H

#include "commonstruct.h"
#include "dcompressornamespace.h"
DCOMPRESSOR_BEGIN_NAMESPACE
CustomMimeType determineMimeType(const QString &filename);
DCOMPRESSOR_END_NAMESPACE
#endif // MIMETYPES_H
