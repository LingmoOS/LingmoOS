// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PLUGINPROCESS_H
#define PLUGINPROCESS_H

#include <QMap>
#include <QObject>

class QProcess;

namespace GrandSearch {

class PluginProcess : public QObject
{
    Q_OBJECT
public:
    explicit PluginProcess(QObject *parent = nullptr);
    ~PluginProcess();
    bool addProgram(const QString &name, const QString &path);
    void setWatched(const QString &name, bool watch);
    void clear();
protected:
    void timerEvent(QTimerEvent *event);
signals:

public slots:
    bool startProgram(const QString &name);
    void terminate(const QString &name);
private slots:
    void processStateChanged();
private:
    void removeChecklist(QProcess *process);
    void addChecklist(QProcess *process);
    void checkStability(QProcess *process);
private:
    QMap<QString, QString> m_programs;
    QMap<QString, QProcess *> m_processes;
    QMap<QString, bool> m_watch;
    QMap<QProcess*, int> m_restartCount;
    QMap<QProcess*, int> m_checklist;
};

}
#endif // PLUGINPROCESS_H
