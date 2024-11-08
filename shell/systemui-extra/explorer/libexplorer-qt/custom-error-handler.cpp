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

#include "custom-error-handler.h"
#include <QMessageBox>

using namespace Peony;

CustomErrorHandler::CustomErrorHandler(QObject *parent) : QObject(parent)
{

}

QList<int> CustomErrorHandler::errorCodeSupportHandling()
{
    return QList<int>();
}

void CustomErrorHandler::handleCustomError(const QString &uri, int errorCode)
{
    Q_UNUSED(uri)
    Q_UNUSED(errorCode)

    QMessageBox::StandardButton button = QMessageBox::question(0, 0, tr("Is Error Handled?"),
                                                               QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
    if (button == QMessageBox::Yes) {
        Q_EMIT finished();
    } else {
        Q_EMIT failed(tr("Error not be handled correctly"));
    }
}
