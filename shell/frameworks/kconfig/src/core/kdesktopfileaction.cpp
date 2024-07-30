/*
    SPDX-FileCopyrightText: 2023 Alexander Lohnau <alexander.lohnau@gmx.de>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#include "kdesktopfileaction.h"
#include <QSharedData>

class KDesktopFileActionPrivate : public QSharedData
{
public:
    QString m_internalActionKey;
    QString m_name;
    QString m_icon;
    QString m_exec;
    QString m_desktopFilePath;

    KDesktopFileActionPrivate(const QString &name, const QString &text, const QString &icon, const QString &exec, const QString &desktopFilePath)
        : m_internalActionKey(name)
        , m_name(text)
        , m_icon(icon)
        , m_exec(exec)
        , m_desktopFilePath(desktopFilePath)
    {
    }
    KDesktopFileActionPrivate() = default;
};

KDesktopFileAction::KDesktopFileAction(const QString &name, const QString &text, const QString &icon, const QString &exec, const QString &desktopFilePath)
    : d(new KDesktopFileActionPrivate(name, text, icon, exec, desktopFilePath))
{
}

KDesktopFileAction::KDesktopFileAction()
    : d(new KDesktopFileActionPrivate())
{
}

KDesktopFileAction::KDesktopFileAction(const KDesktopFileAction &other) = default;
KDesktopFileAction &KDesktopFileAction::operator=(const KDesktopFileAction &other) = default;
KDesktopFileAction::KDesktopFileAction(KDesktopFileAction &&other) = default;
KDesktopFileAction &KDesktopFileAction::operator=(KDesktopFileAction &&other) = default;
KDesktopFileAction::~KDesktopFileAction() = default;

QString KDesktopFileAction::actionsKey() const
{
    return d->m_internalActionKey;
}

QString KDesktopFileAction::desktopFilePath() const
{
    return d->m_desktopFilePath;
}

QString KDesktopFileAction::name() const
{
    return d->m_name;
}

QString KDesktopFileAction::icon() const
{
    return d->m_icon;
}

QString KDesktopFileAction::exec() const
{
    return d->m_exec;
}

bool KDesktopFileAction::isSeparator() const
{
    return d->m_internalActionKey == QLatin1String("_SEPARATOR_");
}
