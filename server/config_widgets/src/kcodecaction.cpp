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
#include <QTextCodec>
#include <QVariant>

// According to http://www.iana.org/assignments/ianacharset-mib
// the default/unknown mib value is 2.
#define MIB_DEFAULT 2

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
        q->connect(tmp, qOverload<QAction *>(&KSelectAction::triggered), q, [this](QAction *action) {
            subActionTriggered(action);
        });
        tmp->setCheckable(true);
        q->addAction(tmp);
    }
    q->setCurrentItem(0);

#if KCONFIGWIDGETS_BUILD_DEPRECATED_SINCE(5, 78)
    // forward deprecated signals to undeprecated, to be backward-compatible to unported subclasses
    QObject::connect(q, qOverload<QTextCodec *>(&KCodecAction::triggered), q, &KCodecAction::codecTriggered);
    QObject::connect(q, qOverload<KEncodingProber::ProberType>(&KCodecAction::triggered), q, &KCodecAction::encodingProberTriggered);
#endif
#if KCONFIGWIDGETS_BUILD_DEPRECATED_SINCE(5, 103)
    QObject::connect(q, &KCodecAction::codecTriggered, q, [this](QTextCodec *codec) {
        Q_EMIT q->codecNameTriggered(QString::fromUtf8(codec->name()));
    });
#endif
}

#if KCONFIGWIDGETS_BUILD_DEPRECATED_SINCE(5, 103)
int KCodecAction::mibForName(const QString &codecName, bool *ok) const
{
    // FIXME logic is good but code is ugly

    bool success = false;
    int mib = MIB_DEFAULT;
    KCharsets *charsets = KCharsets::charsets();

    if (codecName == d->defaultAction->text()) {
        success = true;
    } else {
        QTextCodec *codec = charsets->codecForName(codecName, success);
        if (!success) {
            // Maybe we got a description name instead
            codec = charsets->codecForName(charsets->encodingForName(codecName), success);
        }

        if (codec) {
            mib = codec->mibEnum();
        }
    }

    if (ok) {
        *ok = success;
    }

    if (success) {
        return mib;
    }

    qCWarning(KCONFIG_WIDGETS_LOG) << "Invalid codec name: " << codecName;
    return MIB_DEFAULT;
}
#endif

#if KCONFIGWIDGETS_BUILD_DEPRECATED_SINCE(5, 103)
QTextCodec *KCodecAction::codecForMib(int mib) const
{
    if (mib == MIB_DEFAULT) {
        // FIXME offer to change the default codec
        return QTextCodec::codecForLocale();
    } else {
        return QTextCodec::codecForMib(mib);
    }
}
#endif

void KCodecAction::actionTriggered(QAction *action)
{
    // we don't want to emit any signals from top-level items
    // except for the default one
    if (action == d->defaultAction) {
#if KCONFIGWIDGETS_BUILD_DEPRECATED_SINCE(5, 78)
        Q_EMIT triggered(KEncodingProber::Universal);
#elif KCONFIGWIDGETS_BUILD_DEPRECATED_SINCE(5, 102)
        Q_EMIT encodingProberTriggered(KEncodingProber::Universal);
#endif
        Q_EMIT defaultItemTriggered();
    }
}

void KCodecActionPrivate::subActionTriggered(QAction *action)
{
    if (currentSubAction == action) {
        return;
    }
    currentSubAction = action;
    bool ok = true;
#if KCONFIGWIDGETS_BUILD_DEPRECATED_SINCE(5, 103)
    int mib = q->mibForName(action->text(), &ok);
#endif
    if (ok) {
#if KWIDGETSADDONS_BUILD_DEPRECATED_SINCE(5, 78)
        QT_WARNING_PUSH
        QT_WARNING_DISABLE_CLANG("-Wdeprecated-declarations")
        QT_WARNING_DISABLE_GCC("-Wdeprecated-declarations")
        // will also indirectly emit textTriggered, due to signal connection in KSelectAction
        Q_EMIT q->triggered(action->text());
        QT_WARNING_POP
#else
        Q_EMIT q->textTriggered(action->text());
#endif
#if KCONFIGWIDGETS_BUILD_DEPRECATED_SINCE(5, 103)
        QTextCodec *codec = q->codecForMib(mib);
#endif
#if KCONFIGWIDGETS_BUILD_DEPRECATED_SINCE(5, 78)
        // will also indirectly emit codecTriggered, due to signal connection in init()
        Q_EMIT q->triggered(codec);
#elif KCONFIGWIDGETS_BUILD_DEPRECATED_SINCE(5, 103)
        Q_EMIT q->codecTriggered(codec);
#else
        Q_EMIT q->codecNameTriggered(action->text());
#endif
    } else {
        if (!action->data().isNull()) {
#if KCONFIGWIDGETS_BUILD_DEPRECATED_SINCE(5, 102)
            const auto encodingProberType = static_cast<KEncodingProber::ProberType>(action->data().toUInt());
#endif

#if KCONFIGWIDGETS_BUILD_DEPRECATED_SINCE(5, 78)
            // will also indirectly emit encodingProberTriggered, due to signal connection in init()
            Q_EMIT q->triggered(encodingProberType);
#elif KCONFIGWIDGETS_BUILD_DEPRECATED_SINCE(5, 102)
            Q_EMIT q->encodingProberTriggered(encodingProberType);
#endif
        }
    }
}

#if KCONFIGWIDGETS_BUILD_DEPRECATED_SINCE(5, 103)
QTextCodec *KCodecAction::currentCodec() const
{
    return codecForMib(currentCodecMib());
}
#endif

#if KCONFIGWIDGETS_BUILD_DEPRECATED_SINCE(5, 103)
bool KCodecAction::setCurrentCodec(QTextCodec *codec)
{
    if (!codec) {
        return false;
    }

    for (int i = 0; i < actions().size(); ++i) {
        if (actions().at(i)->menu()) {
            for (int j = 0; j < actions().at(i)->menu()->actions().size(); ++j) {
                if (!j && !actions().at(i)->menu()->actions().at(j)->data().isNull()) {
                    continue;
                }
                if (codec == KCharsets::charsets()->codecForName(actions().at(i)->menu()->actions().at(j)->text())) {
                    d->currentSubAction = actions().at(i)->menu()->actions().at(j);
                    d->currentSubAction->trigger();
                    return true;
                }
            }
        }
    }
    return false;
}
#endif

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

#if KCONFIGWIDGETS_BUILD_DEPRECATED_SINCE(5, 103)
int KCodecAction::currentCodecMib() const
{
    return mibForName(currentCodecName());
}
#endif

#if KCONFIGWIDGETS_BUILD_DEPRECATED_SINCE(5, 103)
bool KCodecAction::setCurrentCodec(int mib)
{
    if (mib == MIB_DEFAULT) {
        return setCurrentAction(d->defaultAction);
    } else {
        return setCurrentCodec(codecForMib(mib));
    }
}
#endif

#if KCONFIGWIDGETS_BUILD_DEPRECATED_SINCE(5, 102)
KEncodingProber::ProberType KCodecAction::currentProberType() const
{
    return d->currentSubAction->data().isNull() ? KEncodingProber::None : (KEncodingProber::ProberType)d->currentSubAction->data().toUInt();
}
#endif

#if KCONFIGWIDGETS_BUILD_DEPRECATED_SINCE(5, 102)
bool KCodecAction::setCurrentProberType(KEncodingProber::ProberType scri)
{
    if (scri == KEncodingProber::Universal) {
        d->currentSubAction = d->defaultAction;
        d->currentSubAction->trigger();
        return true;
    }

    for (int i = 0; i < actions().size(); ++i) {
        if (actions().at(i)->menu()) {
            if (!actions().at(i)->menu()->actions().isEmpty() && !actions().at(i)->menu()->actions().at(0)->data().isNull()
                && actions().at(i)->menu()->actions().at(0)->data().toUInt() == (uint)scri) {
                d->currentSubAction = actions().at(i)->menu()->actions().at(0);
                d->currentSubAction->trigger();
                return true;
            }
        }
    }
    return false;
}
#endif

#include "moc_kcodecaction.cpp"
