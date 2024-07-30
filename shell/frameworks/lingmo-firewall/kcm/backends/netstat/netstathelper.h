// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
// SPDX-FileCopyrightText: 2018 Alexis Lopes Zubeta <contact@azubieta.net>
// SPDX-FileCopyrightText: 2020 Tomaz Canabrava <tcanabrava@kde.org>

#ifndef NETSTATHELPER_H
#define NETSTATHELPER_H

#include <QLoggingCategory>
#include <QProcess>
#include <QVariantMap>

Q_DECLARE_LOGGING_CATEGORY(NetstatHelperDebug)

class QTimer;

class NetstatHelper : public QObject
{
    Q_OBJECT
public:
    using QObject::QObject;
    QString errorString() const;
    bool hasError() const;

public Q_SLOTS:
    void query();
    void stopProcess();

private Q_SLOTS:
    // called by the finished signal on the process.
    void stepExecuteFinished(int exitCode);

Q_SIGNALS:
    void queryFinished(const QList<QStringList> &query);

private:
    QList<QStringList> parseSSOutput(const QByteArray &ss);

    QString extractAndStrip(const QString &src, int index, int size);

    void resetPointers();

    QString m_errorString;
    bool m_hasError = false;
    QProcess *m_executableProcess = nullptr;
    bool m_hasTimeoutError = false;
};

#endif // NETSTATHELPER_H
