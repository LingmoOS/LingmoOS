/*
 * Copyright (C) 2022, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: iaom <zhangpengfei@kylinos.cn>
 *
 */
#ifndef INDEXUPDATER_H
#define INDEXUPDATER_H

#include <QRunnable>
#include <QObject>
#include <QVector>
#include <QAtomicInt>
#include "pending-file.h"
namespace LingmoUISearch {

class IndexUpdater : public QObject, public QRunnable
{
    Q_OBJECT
public:
    explicit IndexUpdater(const QVector<PendingFile>& files,
                          QAtomicInt& indexstop,
                          QAtomicInt& contentIndexstop,
                          QAtomicInt& contentIndexOcrStop,
                          QAtomicInt& aiIndexStop);
    void run() override;

Q_SIGNALS:
    void done();
    void indexUpdated(int eventType, const QString &arg1, const QString &arg2);

private:
    void updateIndex();
    void updateContentIndex();
    void updateOcrContentIndex();
    void updateAiIndex();

    QVector<PendingFile> m_cache;
    QAtomicInt *m_contentIndexStop = nullptr;
    QAtomicInt *m_indexStop = nullptr;
    QAtomicInt *m_contentIndexOcrStop = nullptr;
    QAtomicInt *m_aiIndexStop = nullptr;
};
}
#endif // INDEXUPDATER_H
