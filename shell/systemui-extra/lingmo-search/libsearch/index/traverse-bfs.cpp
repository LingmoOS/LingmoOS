/*
 * Copyright (C) 2020, KylinSoft Co., Ltd.
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
 * Authors: zhangzihao <zhangzihao@kylinos.cn>
 *
 */
#include "traverse-bfs.h"
#include "file-utils.h"
using namespace LingmoUISearch;
TraverseBFS::TraverseBFS(const QStringList &path) {
    Q_ASSERT('/' == path.at(0));
    m_pathList = path;
}

void TraverseBFS::Traverse() {
    QQueue<QString> bfs;
    for(QString blockPath : m_blockList) {
        for(QString path : m_pathList) {
            if(FileUtils::isOrUnder(path, blockPath)) {
                m_pathList.removeOne(path);
            }
        }
    }
    for(QString path : m_pathList) {
        work(QFileInfo(path));
        bfs.enqueue(path);
    }

    QFileInfoList list;
    QDir dir;
    QStringList tmpList = m_blockList;
    // QDir::Hidden
    dir.setFilter(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot);
    dir.setSorting(QDir::DirsFirst);
    while(!bfs.empty()) {
        dir.setPath(bfs.dequeue());
        list = dir.entryInfoList();
        for(auto i : list) {
            bool isBlocked = false;
            for(QString path : tmpList) {
                if(i.absoluteFilePath() == path) {
                    isBlocked = true;
                    tmpList.removeOne(path);
                    break;
                }
            }
            if(isBlocked)
                continue;

            if(i.isDir() && (!(i.isSymLink()))) {
                bfs.enqueue(i.absoluteFilePath());
            }
            work(i);
        }
    }
}

void TraverseBFS::setPath(const QStringList &pathList) {
    m_pathList = pathList;
}

void TraverseBFS::setBlockPath(const QStringList &pathList)
{
    m_blockList =pathList;
}
