/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 1999 Reginald Stadlbauer <reggie@kde.org>
    SPDX-FileCopyrightText: 1999 Simon Hausmann <hausmann@kde.org>
    SPDX-FileCopyrightText: 2000 Nicolas Hadacek <haadcek@kde.org>
    SPDX-FileCopyrightText: 2000 Kurt Granroth <granroth@kde.org>
    SPDX-FileCopyrightText: 2000 Michael Koch <koch@kde.org>
    SPDX-FileCopyrightText: 2001 Holger Freyther <freyther@kde.org>
    SPDX-FileCopyrightText: 2002 Ellis Whitehead <ellis@kde.org>
    SPDX-FileCopyrightText: 2002 Joseph Wenninger <jowenn@kde.org>
    SPDX-FileCopyrightText: 2003 Andras Mantia <amantia@kde.org>
    SPDX-FileCopyrightText: 2005-2006 Hamish Rodda <rodda@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "kpastetextaction.h"

#include <QApplication>
#include <QClipboard>
#include <QDBusInterface>
#include <QDBusReply>
#include <QMenu>

#if KCONFIGWIDGETS_BUILD_DEPRECATED_SINCE(5, 39)

class KPasteTextActionPrivate
{
public:
    KPasteTextActionPrivate(KPasteTextAction *parent)
        : q(parent)
    {
    }

    ~KPasteTextActionPrivate()
    {
        delete m_popup;
    }

    void menuAboutToShow();
    void slotTriggered(QAction *action);

    void init();

    KPasteTextAction *const q;
    QMenu *m_popup = nullptr;
    bool m_mixedMode;
};

KPasteTextAction::KPasteTextAction(QObject *parent)
    : QAction(parent)
    , d(new KPasteTextActionPrivate(this))
{
    d->init();
}

KPasteTextAction::KPasteTextAction(const QString &text, QObject *parent)
    : QAction(parent)
    , d(new KPasteTextActionPrivate(this))
{
    d->init();
    setText(text);
}

KPasteTextAction::KPasteTextAction(const QIcon &icon, const QString &text, QObject *parent)
    : QAction(icon, text, parent)
    , d(new KPasteTextActionPrivate(this))
{
    d->init();
}

void KPasteTextActionPrivate::init()
{
    m_popup = new QMenu;
    q->connect(m_popup, &QMenu::aboutToShow, q, [this]() {
        menuAboutToShow();
    });
    q->connect(m_popup, &QMenu::triggered, q, [this](QAction *action) {
        slotTriggered(action);
    });
    m_mixedMode = true;
}

KPasteTextAction::~KPasteTextAction() = default;

void KPasteTextAction::setMixedMode(bool mode)
{
    d->m_mixedMode = mode;
}

void KPasteTextActionPrivate::menuAboutToShow()
{
    m_popup->clear();
    QStringList list;
    QDBusInterface klipper(QStringLiteral("org.kde.klipper"), QStringLiteral("/klipper"), QStringLiteral("org.kde.klipper.klipper"));
    if (klipper.isValid()) {
        QDBusReply<QStringList> reply = klipper.call(QStringLiteral("getClipboardHistoryMenu"));
        if (reply.isValid()) {
            list = reply;
        }
    }
    QString clipboardText = qApp->clipboard()->text(QClipboard::Clipboard);
    if (list.isEmpty()) {
        list << clipboardText;
    }
    bool found = false;
    const QFontMetrics fm = m_popup->fontMetrics();
    for (const QString &string : std::as_const(list)) {
        QString text = fm.elidedText(string.simplified(), Qt::ElideMiddle, fm.maxWidth() * 20);
        text.replace(QLatin1Char('&'), QLatin1String("&&"));
        QAction *action = m_popup->addAction(text);
        if (!found && string == clipboardText) {
            action->setChecked(true);
            found = true;
        }
    }
}

void KPasteTextActionPrivate::slotTriggered(QAction *action)
{
    QDBusInterface klipper(QStringLiteral("org.kde.klipper"), QStringLiteral("/klipper"), QStringLiteral("org.kde.klipper.klipper"));
    if (klipper.isValid()) {
        QDBusReply<QString> reply = klipper.call(QStringLiteral("getClipboardHistoryItem"), m_popup->actions().indexOf(action));
        if (!reply.isValid()) {
            return;
        }
        QString clipboardText = reply;
        reply = klipper.call(QStringLiteral("setClipboardContents"), clipboardText);
        // if (reply.isValid())
        //  qCDebug(KCONFIG_WIDGETS_LOG) << "Clipboard: " << qApp->clipboard()->text(QClipboard::Clipboard);
    }
}

#include "moc_kpastetextaction.cpp"

#endif // KCONFIGWIDGETS_BUILD_DEPRECATED_SINCE
