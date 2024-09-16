// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FILESTATISTICSTHREAD_H
#define FILESTATISTICSTHREAD_H

#include <QThread>

class QTimer;

namespace GrandSearch {

class FileStatisticsThread : public QThread
{
    Q_OBJECT
public:
    enum State {
        StoppedState,
        RunningState
    };

    explicit FileStatisticsThread(QObject *parent = nullptr);
    ~FileStatisticsThread() Q_DECL_OVERRIDE;

public slots:
    void start(const QString &sourceFile);
    void stop();
signals:
    void dataNotify(qint64 size);
protected:
    void run() Q_DECL_OVERRIDE;
private:
    void setState(State state);
    bool stateCheck();
    void processFile(const QString &file, QQueue<QString> &dirQueue);
private:
    QTimer *m_notifyDataTimer = nullptr;
    QAtomicInteger<qint64> m_totalSize = 0;
    QString m_sourceFile;
    State m_state = StoppedState;
};

}

#endif // FILESTATISTICSTHREAD_H
