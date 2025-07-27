/*
    SPDX-FileCopyrightText: 2008 Rolf Eike Beer <kde@opensource.sf-tec.de>
    SPDX-FileCopyrightText: 2016 Andrius Štikonas <andrius@stikonas.eu>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "klinebufferedprocess.h"

KLineBufferedProcessPrivate::KLineBufferedProcessPrivate(KLineBufferedProcess *parent)
 : QObject(parent),
   m_newlineInStdout(-1),
   m_newlineInStderr(-1),
   m_parent(parent),
#ifdef Q_OS_WIN 	//krazy:exclude=cpp
   m_lineEnd("\r\n")
#else
   m_lineEnd("\n")
#endif
{
}

void KLineBufferedProcessPrivate::_k_receivedStdout()
{
    QByteArray ndata = m_parent->readAllStandardOutput();
    int oldBufferSize = m_stdoutBuffer.size();
    m_stdoutBuffer.append(ndata);

    if (m_newlineInStdout < 0) {
        m_newlineInStdout = ndata.indexOf(m_lineEnd);
        if (m_newlineInStdout >= 0) {
            m_newlineInStdout += oldBufferSize;
            Q_EMIT m_parent->lineReadyStandardOutput();
        }
    }
}

void KLineBufferedProcessPrivate::_k_receivedStderr()
{
    QByteArray ndata = m_parent->readAllStandardError();
    int oldBufferSize = m_stderrBuffer.size();
    m_stderrBuffer.append(ndata);

   if (m_newlineInStderr < 0) {
        m_newlineInStderr = ndata.indexOf(m_lineEnd);
        if (m_newlineInStderr >= 0) {
            m_newlineInStderr += oldBufferSize;
            Q_EMIT m_parent->lineReadyStandardError();
        }
    }
}
