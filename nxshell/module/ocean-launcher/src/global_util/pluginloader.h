// SPDX-FileCopyrightText: 2011 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PLUGINLOADER_H
#define PLUGINLOADER_H

#include <QThread>

class PluginLoader : public QThread
{
    Q_OBJECT

public:
    explicit PluginLoader(QObject *parent = Q_NULLPTR);

signals:
    void finished() const;
    void pluginFounded(const QString &pluginFile) const;

protected:
    void run();
};

#endif // PLUGINLOADER_H
