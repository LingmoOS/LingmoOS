/*

    This file is part of the KDE project, module kfile.
    SPDX-FileCopyrightText: 2000 Geert Jansen <jansen@kde.org>
    SPDX-FileCopyrightText: 2000 Kurt Granroth <granroth@kde.org>
    SPDX-FileCopyrightText: 1997 Christoph Neerfeld <chris@kde.org>
    SPDX-FileCopyrightText: 2002 Carsten Pfeiffer <pfeiffer@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "kiconbutton.h"

#include <QFileInfo>

#include <KLocalizedString>

#include "kicondialog.h"

class KIconButtonPrivate
{
public:
    KIconButtonPrivate(KIconButton *qq, KIconLoader *loader);
    ~KIconButtonPrivate();

    KIconDialog *dialog();

    // slots
    void _k_slotChangeIcon();
    void _k_newIconName(const QString &);

    KIconButton *q;

    int iconSize;
    int buttonIconSize;
    bool m_bStrictIconSize;

    bool mbUser;
    KIconLoader::Group mGroup;
    KIconLoader::Context mContext;

    QString mIcon;
    KIconDialog *mpDialog;
    KIconLoader *mpLoader;
};

/*
 * KIconButton: A "choose icon" pushbutton.
 */

KIconButton::KIconButton(QWidget *parent)
    : QPushButton(parent)
    , d(new KIconButtonPrivate(this, KIconLoader::global()))
{
    QPushButton::setIconSize(QSize(48, 48));
}

KIconButtonPrivate::KIconButtonPrivate(KIconButton *qq, KIconLoader *loader)
    : q(qq)
{
    m_bStrictIconSize = false;
    iconSize = 0; // let KIconLoader choose the default
    buttonIconSize = -1; // When buttonIconSize is -1, iconSize will be used for the button

    mGroup = KIconLoader::Desktop;
    mContext = KIconLoader::Application;
    mbUser = false;

    mpLoader = loader;
    mpDialog = nullptr;
    QObject::connect(q, &KIconButton::clicked, q, [this]() {
        _k_slotChangeIcon();
    });

    q->setToolTip(i18nc("@info:tooltip", "Select Icon…"));
}

KIconButtonPrivate::~KIconButtonPrivate()
{
    delete mpDialog;
}

KIconDialog *KIconButtonPrivate::dialog()
{
    if (!mpDialog) {
        mpDialog = new KIconDialog(q);
        QObject::connect(mpDialog, &KIconDialog::newIconName, q, [this](const QString &iconName) {
            _k_newIconName(iconName);
        });
    }

    return mpDialog;
}

KIconButton::~KIconButton() = default;

void KIconButton::setStrictIconSize(bool b)
{
    d->m_bStrictIconSize = b;
}

bool KIconButton::strictIconSize() const
{
    return d->m_bStrictIconSize;
}

void KIconButton::setIconSize(int size)
{
    if (d->buttonIconSize == -1) {
        QPushButton::setIconSize(QSize(size, size));
    }

    d->iconSize = size;
}

int KIconButton::iconSize() const
{
    return d->iconSize;
}

void KIconButton::setButtonIconSize(int size)
{
    QPushButton::setIconSize(QSize(size, size));
    d->buttonIconSize = size;
}

int KIconButton::buttonIconSize() const
{
    return QPushButton::iconSize().height();
}

void KIconButton::setIconType(KIconLoader::Group group, KIconLoader::Context context, bool user)
{
    d->mGroup = group;
    d->mContext = context;
    d->mbUser = user;
}

void KIconButton::setIcon(const QString &icon)
{
    d->mIcon = icon;
    setIcon(QIcon::fromTheme(d->mIcon));

    if (d->mbUser) {
        d->dialog()->setCustomLocation(QFileInfo(d->mpLoader->iconPath(d->mIcon, d->mGroup, true)).absolutePath());
    }
}

void KIconButton::setIcon(const QIcon &icon)
{
    QPushButton::setIcon(icon);
}

void KIconButton::resetIcon()
{
    d->mIcon.clear();
    setIcon(QIcon());
}

const QString &KIconButton::icon() const
{
    return d->mIcon;
}

void KIconButtonPrivate::_k_slotChangeIcon()
{
    dialog()->setup(mGroup, mContext, m_bStrictIconSize, iconSize, mbUser);
    dialog()->setSelectedIcon(mIcon);
    dialog()->showDialog();
}

void KIconButtonPrivate::_k_newIconName(const QString &name)
{
    if (name.isEmpty()) {
        return;
    }

    q->setIcon(QIcon::fromTheme(name));
    mIcon = name;

    if (mbUser) {
        mpDialog->setCustomLocation(QFileInfo(mpLoader->iconPath(mIcon, mGroup, true)).absolutePath());
    }

    Q_EMIT q->iconChanged(name);
}

#include "moc_kiconbutton.cpp"
