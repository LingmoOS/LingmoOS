/*
    SPDX-FileCopyrightText: 2003 Jason Keirstead <jason@keirstead.org>
    SPDX-FileCopyrightText: 2006 Michel Hermier <michel.hermier@gmail.com>
    SPDX-FileCopyrightText: 2007 Nick Shaforostoff <shafff@ukr.net>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kcodecaction.h"
#include "kconfigwidgets_debug.h"

#include <KCharsets>
#include <KEncodingProber>
#include <KLocalizedString>

#include <QMenu>
#include <QVariant>

class KCodecActionPrivate
{
public:
    KCodecActionPrivate(KCodecAction *parent)
        : q(parent)
    {
    }

    void init(bool);

    void subActionTriggered(QAction *);

    KCodecAction *const q;
    QAction *defaultAction = nullptr;
    QAction *currentSubAction = nullptr;
};

KCodecAction::KCodecAction(QObject *parent, bool showAutoOptions)
    : KSelectAction(parent)
    , d(new KCodecActionPrivate(this))
{
    d->init(showAutoOptions);
}

KCodecAction::KCodecAction(const QString &text, QObject *parent, bool showAutoOptions)
    : KSelectAction(text, parent)
    , d(new KCodecActionPrivate(this))
{
    d->init(showAutoOptions);
}

KCodecAction::KCodecAction(const QIcon &icon, const QString &text, QObject *parent, bool showAutoOptions)
    : KSelectAction(icon, text, parent)
    , d(new KCodecActionPrivate(this))
{
    d->init(showAutoOptions);
}

KCodecAction::~KCodecAction() = default;

void KCodecActionPrivate::init(bool showAutoOptions)
{
    q->setToolBarMode(KSelectAction::MenuMode);
    defaultAction = q->addAction(i18nc("Encodings menu", "Default"));

    const auto lstEncodings = KCharsets::charsets()->encodingsByScript();
    for (const QStringList &encodingsForScript : lstEncodings) {
        KSelectAction *tmp = new KSelectAction(encodingsForScript.at(0), q);
        if (showAutoOptions) {
            KEncodingProber::ProberType scri = KEncodingProber::proberTypeForName(encodingsForScript.at(0));
            if (scri != KEncodingProber::None) {
                tmp->addAction(i18nc("Encodings menu", "Autodetect"))->setData(QVariant((uint)scri));
                tmp->menu()->addSeparator();
            }
        }
        for (int i = 1; i < encodingsForScript.size(); ++i) {
            tmp->addAction(encodingsForScript.at(i));
        }
        q->connect(tmp, &KSelectAction::actionTriggered, q, [this](QAction *action) {
            subActionTriggered(action);
        });
        tmp->setCheckable(true);
        q->addAction(tmp);
    }
    q->setCurrentItem(0);
}

void KCodecAction::slotActionTriggered(QAction *action)
{
    // we don't want to emit any signals from top-level items
    // except for the default one
    if (action == d->defaultAction) {
        Q_EMIT defaultItemTriggered();
    }
}

void KCodecActionPrivate::subActionTriggered(QAction *action)
{
    if (currentSubAction == action) {
        return;
    }
    currentSubAction = action;
    Q_EMIT q->textTriggered(action->text());
    Q_EMIT q->codecNameTriggered(action->text().toUtf8());
}

QString KCodecAction::currentCodecName() const
{
    return d->currentSubAction->text();
}

bool KCodecAction::setCurrentCodec(const QString &codecName)
{
    if (codecName.isEmpty()) {
        return false;
    }

    for (int i = 0; i < actions().size(); ++i) {
        if (actions().at(i)->menu()) {
            for (int j = 0; j < actions().at(i)->menu()->actions().size(); ++j) {
                if (!j && !actions().at(i)->menu()->actions().at(j)->data().isNull()) {
                    continue;
                }
                if (codecName == actions().at(i)->menu()->actions().at(j)->text()) {
                    d->currentSubAction = actions().at(i)->menu()->actions().at(j);
                    d->currentSubAction->trigger();
                    return true;
                }
            }
        }
    }
    return false;
}

#include "moc_kcodecaction.cpp"
