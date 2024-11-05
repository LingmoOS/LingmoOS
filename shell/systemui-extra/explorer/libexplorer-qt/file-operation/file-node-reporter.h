/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2020, KylinSoft Co., Ltd.
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

#ifndef FILENODEREPORTER_H
#define FILENODEREPORTER_H

#include <QObject>
#include <memory>

#include "explorer-core_global.h"

namespace Peony {

class FileNode;

/*!
 * \brief The FileNodeReporter class
 * <br>
 * This class is a signal proxy of FileNode instances.
 * Other objects can connect the signals getting the current state of filenode.
 * </br>
 */
class PEONYCORESHARED_EXPORT FileNodeReporter : public QObject
{
    Q_OBJECT
public:
    explicit FileNodeReporter(QObject *parent = nullptr);
    ~FileNodeReporter();

    void sendNodeFound(const QString &uri, const qint64 &offset) {
        Q_EMIT nodeFound(uri, offset);
    }

    void cancel() {
        m_cancelled = true;
    }
    bool isOperationCancelled() {
        return m_cancelled;
    }
    bool isUsedInCount() {
        return m_isUsedInCount;
    }
    void setUsedInCount(const bool InCount) {
        if (InCount != m_isUsedInCount) {
            m_isUsedInCount = InCount;
        }
    }

Q_SIGNALS:
    void nodeFound(const QString &uri, const qint64 &offset);
    /*!
     * \brief enumerateNodeFinished
     * \deprecated
     */
    void enumerateNodeFinished();

    /*!
     * \brief nodeOperationDone
     * \param uri
     * \param offset
     * \deprecated
     */
    void nodeOperationDone(const QString &uri, const qint64 &offset);

private:
    bool m_cancelled = false;
    bool m_isUsedInCount = false;
};

}

#endif // FILENODEREPORTER_H
