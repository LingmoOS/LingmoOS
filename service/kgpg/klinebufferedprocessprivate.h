/*
    SPDX-FileCopyrightText: 2008 Rolf Eike Beer <kde@opensource.sf-tec.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KLINEBUFFEREDPROCESSPRIVATE_H
#define KLINEBUFFEREDPROCESSPRIVATE_H

class KLineBufferedProcess;

class KLineBufferedProcessPrivate : public QObject
{
public:
    explicit KLineBufferedProcessPrivate(KLineBufferedProcess *parent);

    void _k_receivedStdout();
    void _k_receivedStderr();

    QByteArray m_stdoutBuffer;
    QByteArray m_stderrBuffer;
    int m_newlineInStdout;
    int m_newlineInStderr;
    KLineBufferedProcess * const m_parent;
    const QByteArray m_lineEnd;
};

#endif // KLINEBUFFEREDPROCESSPRIVATE_H
