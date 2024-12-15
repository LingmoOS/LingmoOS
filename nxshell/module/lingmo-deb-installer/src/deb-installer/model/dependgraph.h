// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QString>
#include <QStringList>
#include <qapt/debfile.h>

#include <vector>

struct DependGraphNode;

class DependGraph
{
public:
    DependGraph() = default;
    ~DependGraph();

    void addNode(const QString &packagePath,
                 const QByteArray &md5,
                 const QString &packageName,
                 const QList<QApt::DependencyItem> &depends);
    std::pair<QList<QString>, QList<QByteArray>> getBestInstallQueue() const;

    void reset();
    void remove(const QByteArray &md5);

protected:
    void removeInGraph(const DependGraphNode *dependnode);

private:
    std::vector<DependGraphNode *> nodes;
};
