/*
    SPDX-FileCopyrightText: 2010 Daniel Nicoletti <dantti12@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef PRINT_QUEUE_H
#define PRINT_QUEUE_H

#include <QApplication>

class PrintQueue : public QApplication
{
    Q_OBJECT
public:
    PrintQueue(int &argc, char **argv);
    ~PrintQueue() override;

public slots:
    void showQueues(const QStringList &queues, const QString &cwd = QString());

private slots:
    void removeQueue();

private:
    void showQueue(const QString &destName);
    QHash<QString, QWidget *> m_uis;
};

#endif
