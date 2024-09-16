// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DTKIO_GLOBAL_H
#define DTKIO_GLOBAL_H

#define DIO_NAMESPACE Dtk::IO

#define DIO_USE_NAMESPACE using namespace DIO_NAMESPACE;

#define DIO_BEGIN_NAMESPACE \
    namespace Dtk {         \
    namespace IO {
#define DIO_END_NAMESPACE \
    }                     \
    }

#endif   // DTKIO_GLOBAL_H
