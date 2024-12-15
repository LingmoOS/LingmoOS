// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SESSION_H
#define SESSION_H

#include <QObject>
#include <QVector>

class Session : public QObject
{
    Q_OBJECT
public:
    explicit Session(QString name, QString session, QString signal, QObject *parent = nullptr);
    virtual ~Session();

    bool alive();

    QString getName();
    QString getSession();
    void addJob(int jobid);
    bool removeJob(int jobid);
    int hasJob(int jobid);
    QString signal();

signals:

public slots:

private:
    QString _name;
    QString _sessionid;
    QString _signalname;
    QVector<int> _jobs;

    bool _pingOK = false;
};

#endif // SESSION_H
