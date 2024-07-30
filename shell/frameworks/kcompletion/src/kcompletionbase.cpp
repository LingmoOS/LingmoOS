/*
    This file is part of the KDE libraries

    SPDX-FileCopyrightText: 2000 Dawit Alemayehu <adawit@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <kcompletionbase.h>

#include <QKeySequence>
#include <QPointer>

class KCompletionBasePrivate
{
public:
    KCompletionBasePrivate(KCompletionBase *parent)
        : q_ptr(parent)
    {
    }

    ~KCompletionBasePrivate();

    void init();

    bool autoDeleteCompletionObject;
    bool handleSignals;
    bool emitSignals;
    KCompletion::CompletionMode completionMode;
    QPointer<KCompletion> completionObject;
    KCompletionBase::KeyBindingMap keyBindingMap;
    // we may act as a proxy to another KCompletionBase object
    KCompletionBase *delegate = nullptr;
    KCompletionBase *const q_ptr;
    Q_DECLARE_PUBLIC(KCompletionBase)
};

KCompletionBasePrivate::~KCompletionBasePrivate()
{
    if (autoDeleteCompletionObject && completionObject) {
        delete completionObject;
    }
}

void KCompletionBasePrivate::init()
{
    Q_Q(KCompletionBase);
    completionMode = KCompletion::CompletionPopup;
    delegate = nullptr;
    // Initialize all key-bindings to 0 by default so that
    // the event filter will use the global settings.
    q->useGlobalKeyBindings();

    q->setAutoDeleteCompletionObject(false);
    q->setHandleSignals(true);
    q->setEmitSignals(false);
}

KCompletionBase::KCompletionBase()
    : d_ptr(new KCompletionBasePrivate(this))
{
    Q_D(KCompletionBase);
    d->init();
}

KCompletionBase::~KCompletionBase()
{
}

void KCompletionBase::setDelegate(KCompletionBase *delegate)
{
    Q_D(KCompletionBase);
    d->delegate = delegate;

    if (delegate) {
        delegate->setAutoDeleteCompletionObject(d->autoDeleteCompletionObject);
        delegate->setHandleSignals(d->handleSignals);
        delegate->setEmitSignals(d->emitSignals);
        delegate->setCompletionMode(d->completionMode);
        delegate->setKeyBindingMap(d->keyBindingMap);
    }
}

KCompletionBase *KCompletionBase::delegate() const
{
    Q_D(const KCompletionBase);
    return d->delegate;
}

KCompletion *KCompletionBase::completionObject(bool handleSignals)
{
    Q_D(KCompletionBase);
    if (d->delegate) {
        return d->delegate->completionObject(handleSignals);
    }

    if (!d->completionObject) {
        setCompletionObject(new KCompletion(), handleSignals);
        d->autoDeleteCompletionObject = true;
    }
    return d->completionObject;
}

void KCompletionBase::setCompletionObject(KCompletion *completionObject, bool handleSignals)
{
    Q_D(KCompletionBase);
    if (d->delegate) {
        d->delegate->setCompletionObject(completionObject, handleSignals);
        return;
    }

    if (d->autoDeleteCompletionObject && completionObject != d->completionObject) {
        delete d->completionObject;
    }

    d->completionObject = completionObject;

    setAutoDeleteCompletionObject(false);
    setHandleSignals(handleSignals);

    // We emit rotation and completion signals
    // if completion object is not NULL.
    setEmitSignals(!d->completionObject.isNull());
}

// BC: Inline this function and possibly rename it to setHandleEvents??? (DA)
void KCompletionBase::setHandleSignals(bool handle)
{
    Q_D(KCompletionBase);
    if (d->delegate) {
        d->delegate->setHandleSignals(handle);
    } else {
        d->handleSignals = handle;
    }
}

bool KCompletionBase::isCompletionObjectAutoDeleted() const
{
    Q_D(const KCompletionBase);
    return d->delegate ? d->delegate->isCompletionObjectAutoDeleted() : d->autoDeleteCompletionObject;
}

void KCompletionBase::setAutoDeleteCompletionObject(bool autoDelete)
{
    Q_D(KCompletionBase);
    if (d->delegate) {
        d->delegate->setAutoDeleteCompletionObject(autoDelete);
    } else {
        d->autoDeleteCompletionObject = autoDelete;
    }
}

void KCompletionBase::setEnableSignals(bool enable)
{
    Q_D(KCompletionBase);
    if (d->delegate) {
        d->delegate->setEnableSignals(enable);
    } else {
        d->emitSignals = enable;
    }
}

bool KCompletionBase::handleSignals() const
{
    Q_D(const KCompletionBase);
    return d->delegate ? d->delegate->handleSignals() : d->handleSignals;
}

bool KCompletionBase::emitSignals() const
{
    Q_D(const KCompletionBase);
    return d->delegate ? d->delegate->emitSignals() : d->emitSignals;
}

void KCompletionBase::setEmitSignals(bool emitRotationSignals)
{
    Q_D(KCompletionBase);
    if (d->delegate) {
        d->delegate->setEmitSignals(emitRotationSignals);
    } else {
        d->emitSignals = emitRotationSignals;
    }
}

void KCompletionBase::setCompletionMode(KCompletion::CompletionMode mode)
{
    Q_D(KCompletionBase);
    if (d->delegate) {
        d->delegate->setCompletionMode(mode);
        return;
    }

    d->completionMode = mode;
    // Always sync up KCompletion mode with ours as long as we
    // are performing completions.
    if (d->completionObject && d->completionMode != KCompletion::CompletionNone) {
        d->completionObject->setCompletionMode(d->completionMode);
    }
}

KCompletion::CompletionMode KCompletionBase::completionMode() const
{
    Q_D(const KCompletionBase);
    return d->delegate ? d->delegate->completionMode() : d->completionMode;
}

bool KCompletionBase::setKeyBinding(KeyBindingType item, const QList<QKeySequence> &cut)
{
    Q_D(KCompletionBase);
    if (d->delegate) {
        return d->delegate->setKeyBinding(item, cut);
    }

    if (!cut.isEmpty()) {
        for (KeyBindingMap::Iterator it = d->keyBindingMap.begin(); it != d->keyBindingMap.end(); ++it) {
            if (it.value() == cut) {
                return false;
            }
        }
    }
    d->keyBindingMap.insert(item, cut);
    return true;
}

QList<QKeySequence> KCompletionBase::keyBinding(KeyBindingType item) const
{
    Q_D(const KCompletionBase);
    return d->delegate ? d->delegate->keyBinding(item) : d->keyBindingMap[item];
}

void KCompletionBase::useGlobalKeyBindings()
{
    Q_D(KCompletionBase);
    if (d->delegate) {
        d->delegate->useGlobalKeyBindings();
        return;
    }

    d->keyBindingMap.clear();
    d->keyBindingMap.insert(TextCompletion, QList<QKeySequence>());
    d->keyBindingMap.insert(PrevCompletionMatch, QList<QKeySequence>());
    d->keyBindingMap.insert(NextCompletionMatch, QList<QKeySequence>());
    d->keyBindingMap.insert(SubstringCompletion, QList<QKeySequence>());
}

KCompletion *KCompletionBase::compObj() const
{
    Q_D(const KCompletionBase);
    return d->delegate ? d->delegate->compObj() : static_cast<KCompletion *>(d->completionObject);
}

KCompletionBase::KeyBindingMap KCompletionBase::keyBindingMap() const
{
    Q_D(const KCompletionBase);
    return d->delegate ? d->delegate->keyBindingMap() : d->keyBindingMap;
}

void KCompletionBase::setKeyBindingMap(KCompletionBase::KeyBindingMap keyBindingMap)
{
    Q_D(KCompletionBase);
    if (d->delegate) {
        d->delegate->setKeyBindingMap(keyBindingMap);
        return;
    }

    d->keyBindingMap = keyBindingMap;
}

void KCompletionBase::virtual_hook(int, void *)
{
    /*BASE::virtual_hook( id, data );*/
}
