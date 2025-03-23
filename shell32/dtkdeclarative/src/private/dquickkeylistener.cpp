// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dquickkeylistener_p.h"

#include <QQuickItem>
#include <dobject_p.h>

#include <private/qkeymapper_p.h>

DQUICK_BEGIN_NAMESPACE
class DQuickKeyListenerPrivate : public DCORE_NAMESPACE::DObjectPrivate
{
    D_DECLARE_PUBLIC(DQuickKeyListener)
public:
    DQuickKeyListenerPrivate(DQuickKeyListener *qq)
        : DObjectPrivate(qq)
        , target(nullptr)
        , maxKeyCount(4)  // The default value is 4.
    {}

    int doNativeShiftKey(QKeyEvent *e, int nativeKey);
private:
    QQuickItem *target;
    QStringList keys;
    int maxKeyCount;
};


DQuickKeyListener::DQuickKeyListener(QObject *parent)
    : QObject(parent)
    , DObject(*new DQuickKeyListenerPrivate(this))
{

}

QQuickItem *DQuickKeyListener::target() const
{
    D_DC(DQuickKeyListener);
    return d->target;
}

void DQuickKeyListener::setTarget(QQuickItem *item)
{
    D_D(DQuickKeyListener);
    if (d->target == item)
        return;

    if (d->target) {
        d->target->removeEventFilter(this);
        d->keys.clear();
    }

    d->target = item;
    Q_EMIT targetChanged();
    d->target->installEventFilter(this);
}

const QStringList &DQuickKeyListener::keys() const
{
    D_DC(DQuickKeyListener);
    return d->keys;
}

void DQuickKeyListener::setKeys(const QStringList &keys)
{
    D_D(DQuickKeyListener);
    if (d->keys == keys)
        return;

    d->keys = keys;
    Q_EMIT keysChanged();
}

int DQuickKeyListener::maxKeyCount() const
{
    D_DC(DQuickKeyListener);
    return d->maxKeyCount;
}

void DQuickKeyListener::setMaxKeyCount(int count)
{
    D_D(DQuickKeyListener);
    d->maxKeyCount = count;
    Q_EMIT maxKeyCountChanged();
}

void DQuickKeyListener::clearKeys()
{
    D_D(DQuickKeyListener);
    d->keys.clear();
    Q_EMIT keysChanged();
}

bool DQuickKeyListener::eventFilter(QObject *watched, QEvent *event)
{
    D_D(DQuickKeyListener);
    if (watched != d->target)
        return false;
    if (event->type() != QEvent::KeyPress)
        return false;

    QString modifiers;
    QKeyEvent *ke = static_cast<QKeyEvent *>(event);
    int key = ke->key();
    if (ke->modifiers() & Qt::MODIFIER_MASK)
        modifiers = QKeySequence(ke->modifiers()).toString(QKeySequence::NativeText);

    QStringList keyTexts;
    static QList<int> modifierKeys = {Qt::Key_Control, Qt::Key_Shift, Qt::Key_Meta, Qt::Key_Alt};
    if (modifierKeys.contains(key)) {  // Only modifier keys.
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
        keyTexts << modifiers.split(QLatin1Char('+'), Qt::SkipEmptyParts);
#else
        keyTexts << modifiers.split(QLatin1Char('+'), QString::SkipEmptyParts);
#endif
    } else {
        key = d->doNativeShiftKey(ke, key);
        QString ks =  QKeySequence(key).toString();
        QKeySequence sequence(modifiers + ks);
        QString writing = sequence.toString();
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
        keyTexts << writing.split(QLatin1Char('+'), Qt::SkipEmptyParts);
#else
        keyTexts << writing.split(QLatin1Char('+'), QString::SkipEmptyParts);
#endif
        if (writing.contains(QLatin1String("++")))
            keyTexts << QLatin1String("+");
    }

    if (keyTexts.count() > d->maxKeyCount)
        keyTexts.mid(0, d->maxKeyCount);

    if (d->keys == keyTexts)
        return false;

    d->keys = keyTexts;
    Q_EMIT keysChanged();
    return QObject::eventFilter(watched, event);
}

int DQuickKeyListenerPrivate::doNativeShiftKey(QKeyEvent *e, int nativeKey)
{
    int key = nativeKey;
    if (!(e->modifiers() & Qt::ShiftModifier))
        return key;

    auto possibleKeys = QKeyMapper::possibleKeys(e);
    int pkTotal = possibleKeys.count();
    if (!pkTotal)
        return 0;
    bool found = false;
    for (int i = 0; i < possibleKeys.size(); ++i) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
        const auto combinedKey = possibleKeys.at(i).toCombined();
#else
        const auto combinedKey = possibleKeys.at(i);
#endif

        if (combinedKey - nativeKey == int(e->modifiers())
            || (combinedKey == nativeKey && e->modifiers() == Qt::ShiftModifier)) {
            key = combinedKey;
            found = true;
            break;
        }
    }
    // Use as fallback
    if (!found)
#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
        key = possibleKeys.first().toCombined();
#else
        key = possibleKeys.first();
#endif

    return key;
}

DQUICK_END_NAMESPACE
