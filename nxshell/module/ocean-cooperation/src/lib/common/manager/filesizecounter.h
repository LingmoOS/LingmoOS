// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FILESIZECOUNTER_H
#define FILESIZECOUNTER_H

#include <QThread>

class FileSizeCounter : public QThread
{
    Q_OBJECT
public:
    explicit FileSizeCounter(QObject *parent = nullptr);

    quint64 countFiles(const QString &targetIp, const QStringList paths);

signals:
    void onCountFinish(const QString targetIp, const QStringList paths, quint64 totalSize);

protected:
    void run() override;

private:
    void countFilesInDir(const QString &path);

    QStringList _paths;
    QString _targetIp;
    quint64 _totalSize {0};
};

#endif // FILESIZECOUNTER_H
