/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2022, KylinSoft Co., Ltd.
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

#ifndef EMBLEMPROVIDER_H
#define EMBLEMPROVIDER_H

#include <QObject>
#include <QMutex>
#include "explorer-core_global.h"

class QTimer;

namespace Peony {

class PEONYCORESHARED_EXPORT EmblemProvider : public QObject
{
    Q_OBJECT
public:
    explicit EmblemProvider(QObject *parent = nullptr);

    virtual const QString emblemKey();

    /*!
     * \brief getFileEmblemIcons
     * \param uri, the file which proposed to get emblem file.
     * \return override this function to return a emblem icon list.
     */
    virtual QStringList getFileEmblemIcons(const QString &uri);

Q_SIGNALS:
    /*!
     * \brief requestUpdateFile
     * \param uri, the file which want to update dynamiclly
     */
    void requestUpdateFile(const QString &uri);

    /*!
     * \brief requestUpdateAllFiles
     * usually used in repaint the view. use it becaful.
     */
    void requestUpdateAllFiles();
    void visibleChanged(bool visible);
};

class PEONYCORESHARED_EXPORT EmblemProviderManager : public QObject
{
    Q_OBJECT
public:
    static EmblemProviderManager *getInstance();

    void registerProvider(EmblemProvider *provider);
    void unregisterProvider(EmblemProvider *provider);

    QStringList getAllEmblemsForUri(const QString &uri);
    QStringList getEmblemsByNameForUri(const QString &uri, const QString &name);

Q_SIGNALS:
    void requestUpdateFile(const QString &uri);
    void requestUpdateAllFiles();
    void visibleChanged(const QString &name, bool visible);

    void queueQueryFinished(); //private signal

public Q_SLOTS:
    void querySync(const QString &uri);
    void queryAsync(const QString &uri);
    void cancelQuery(const QString &uri);

private:
    explicit EmblemProviderManager(QObject *parent = nullptr);

private Q_SLOTS:
    void queryInternal();

private:
    QList<EmblemProvider *> m_providers;
    QStringList m_queryQueue;
    QTimer *m_timer = nullptr;
    bool m_busy = false;

    QMutex m_mutex;
};

}

#endif // EMBLEMPROVIDER_H
