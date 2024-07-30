/*
    This file is part of the KDE Baloo Project
    SPDX-FileCopyrightText: 2021 Stefan Brüns <stefan.bruens@rwth-aachen.de>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef COMMANDPIPE_H
#define COMMANDPIPE_H

#include <QDataStream>
#include <QObject>

class QIODevice;

namespace Baloo {
namespace Private {

/**
 * Bidirectional communication pipe
 *
 */
class ControllerPipe : public QObject
{
    Q_OBJECT

public:
    ControllerPipe(QIODevice* commandPipe, QIODevice* statusPipe);

    void processIds(const QVector<quint64>& ids);

Q_SIGNALS:
    void urlStarted(const QString& url);
    void urlFinished(const QString& url);
    void urlFailed(const QString& url);
    void batchFinished();

public Q_SLOTS:
    void processStatusData();

private:
    QDataStream m_commandStream;
    QDataStream m_statusStream;
};

class WorkerPipe : public QObject
{
    Q_OBJECT

public:
    WorkerPipe(QIODevice* commandPipe, QIODevice* statusPipe);

    void urlStarted(const QString& url);
    void urlFinished(const QString& url);
    void urlFailed(const QString& url);
    void batchFinished();

public Q_SLOTS:
    void processIdData();

Q_SIGNALS:
    void newDocumentIds(const QVector<quint64>& ids);
    void inputEnd();

private:
    QDataStream m_commandStream;
    QDataStream m_statusStream;
};

} // namespace Private
} // namespace Baloo
#endif
