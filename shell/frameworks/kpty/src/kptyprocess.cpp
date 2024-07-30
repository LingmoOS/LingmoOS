/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2007 Oswald Buddenhagen <ossi@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kptyprocess.h"

#include <kptydevice.h>
#include <kuser.h>

#include <stdlib.h>
#include <unistd.h>

//////////////////
// private data //
//////////////////

class KPtyProcessPrivate
{
public:
    KPtyProcessPrivate()
    {
    }

    std::unique_ptr<KPtyDevice> pty;
    KPtyProcess::PtyChannels ptyChannels = KPtyProcess::NoChannels;
    bool addUtmp = false;
};

KPtyProcess::KPtyProcess(QObject *parent)
    : KPtyProcess(-1, parent)
{
}

KPtyProcess::KPtyProcess(int ptyMasterFd, QObject *parent)
    : KProcess(parent)
    , d_ptr(new KPtyProcessPrivate)
{
    Q_D(KPtyProcess);

    auto parentChildProcModifier = KProcess::childProcessModifier();
    setChildProcessModifier([d, parentChildProcModifier]() {
        d->pty->setCTty();
        if (d->addUtmp) {
            d->pty->login(KUser(KUser::UseRealUserID).loginName().toLocal8Bit().constData(), qgetenv("DISPLAY").constData());
        }
        if (d->ptyChannels & StdinChannel) {
            dup2(d->pty->slaveFd(), 0);
        }
        if (d->ptyChannels & StdoutChannel) {
            dup2(d->pty->slaveFd(), 1);
        }
        if (d->ptyChannels & StderrChannel) {
            dup2(d->pty->slaveFd(), 2);
        }

        if (parentChildProcModifier) {
            parentChildProcModifier();
        }
    });

    d->pty = std::make_unique<KPtyDevice>(this);

    if (ptyMasterFd == -1) {
        d->pty->open();
    } else {
        d->pty->open(ptyMasterFd);
    }

    connect(this, &QProcess::stateChanged, this, [this](QProcess::ProcessState state) {
        if (state == QProcess::NotRunning && d_ptr->addUtmp) {
            d_ptr->pty->logout();
        }
    });
}

KPtyProcess::~KPtyProcess()
{
    Q_D(KPtyProcess);

    if (state() != QProcess::NotRunning && d->addUtmp) {
        d->pty->logout();
        disconnect(this, &QProcess::stateChanged, this, nullptr);
    }
}

void KPtyProcess::setPtyChannels(PtyChannels channels)
{
    Q_D(KPtyProcess);

    d->ptyChannels = channels;
}

KPtyProcess::PtyChannels KPtyProcess::ptyChannels() const
{
    Q_D(const KPtyProcess);

    return d->ptyChannels;
}

void KPtyProcess::setUseUtmp(bool value)
{
    Q_D(KPtyProcess);

    d->addUtmp = value;
}

bool KPtyProcess::isUseUtmp() const
{
    Q_D(const KPtyProcess);

    return d->addUtmp;
}

KPtyDevice *KPtyProcess::pty() const
{
    Q_D(const KPtyProcess);

    return d->pty.get();
}

#include "moc_kptyprocess.cpp"
