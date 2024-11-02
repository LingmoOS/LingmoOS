/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2021, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */

#ifndef CUSTOMERRORHANDLER_H
#define CUSTOMERRORHANDLER_H

#include <QObject>

#include "explor-core_global.h"

namespace Peony {

class PEONYCORESHARED_EXPORT CustomErrorHandler : public QObject
{
    Q_OBJECT
public:
    explicit CustomErrorHandler(QObject *parent = nullptr);

    /*!
     * \brief errorCodeSupportHandling
     * \return a list of error code the handler supported.
     */
    virtual QList<int> errorCodeSupportHandling();
    virtual void handleCustomError(const QString &uri, int errorCode);

Q_SIGNALS:
    /*!
     * \brief finished
     * tell the enumerator error handling finished, and can continue
     * do enumeration.
     */
    void finished();

    /*!
     * \brief canceled
     * tell the enumerator error handling cancelled, enumerator will go back
     * to previous directory.
     */
    void cancelled();

    /*!
     * \brief failed
     * \param message
     * similar to cancelled(), but providing a error message for show error message.
     */
    void failed(const QString &message);
};

}

#endif // CUSTOMERRORHANDLER_H
