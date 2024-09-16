// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DUDFBURNENGINE_H
#define DUDFBURNENGINE_H

#include <QObject>
#include <QLibrary>

#include "dtkburn_global.h"
#include "dtkburntypes.h"

DBURN_BEGIN_NAMESPACE

class DUDFBurnEngine : public QObject
{
    Q_OBJECT
public:
    explicit DUDFBurnEngine(QObject *parent = nullptr);
    ~DUDFBurnEngine() override;

    bool doBurn(const QString &dev, const QPair<QString, QString> &files, const QString &volId);
    QStringList lastErrorMessage() const;

Q_SIGNALS:
    void jobStatusChanged(JobStatus status, int progress);

private:
    void initialize();
    bool canSafeUse() const;
    QStringList readErrorsFromLog() const;

private:
    QLibrary m_lib;
    bool m_libLoaded { false };
    bool m_funcsLoaded { true };
    QStringList m_message;
};

DBURN_END_NAMESPACE

#endif   // DUDFBURNENGINE_H
