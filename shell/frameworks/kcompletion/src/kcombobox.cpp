/*
    This file is part of the KDE libraries

    SPDX-FileCopyrightText: 2000, 2001 Dawit Alemayehu <adawit@kde.org>
    SPDX-FileCopyrightText: 2000, 2001 Carsten Pfeiffer <pfeiffer@kde.org>
    SPDX-FileCopyrightText: 2000 Stefan Schimanski <1Stein@gmx.de>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "kcombobox.h"
#include "kcombobox_p.h"

#include <kcompletion_debug.h>
#include <kcompletionbox.h>

#include <QUrl>

void KComboBoxPrivate::init()
{
    Q_Q(KComboBox);
}

void KComboBoxPrivate::slotLineEditDeleted(QLineEdit *sender)
{
    Q_Q(KComboBox);
    // yes, we need those ugly casts due to the multiple inheritance
    // "sender" is guaranteed to be a KLineEdit (see the connect() to the
    // destroyed() signal
    const KCompletionBase *base = static_cast<const KCompletionBase *>(static_cast<const KLineEdit *>(sender));

    // is it our delegate, that is destroyed?
    if (base == q->delegate()) {
        q->setDelegate(nullptr);
    }
}

KComboBox::KComboBox(QWidget *parent)
    : KComboBox(*new KComboBoxPrivate(this), parent)
{
}

KComboBox::KComboBox(KComboBoxPrivate &dd, QWidget *parent)
    : QComboBox(parent)
    , d_ptr(&dd)
{
    Q_D(KComboBox);

    d->init();
}

KComboBox::KComboBox(bool rw, QWidget *parent)
    : KComboBox(*new KComboBoxPrivate(this), parent)
{
    setEditable(rw);
}

KComboBox::~KComboBox()
{
    Q_D(KComboBox);
    disconnect(d->m_klineEditConnection);
}

bool KComboBox::contains(const QString &text) const
{
    if (text.isEmpty()) {
        return false;
    }

    const int itemCount = count();
    for (int i = 0; i < itemCount; ++i) {
        if (itemText(i) == text) {
            return true;
        }
    }
    return false;
}

int KComboBox::cursorPosition() const
{
    return (isEditable()) ? lineEdit()->cursorPosition() : -1;
}

void KComboBox::setAutoCompletion(bool autocomplete)
{
    Q_D(KComboBox);
    if (d->klineEdit) {
        if (autocomplete) {
            d->klineEdit->setCompletionMode(KCompletion::CompletionAuto);
            setCompletionMode(KCompletion::CompletionAuto);
        } else {
            d->klineEdit->setCompletionMode(KCompletion::CompletionPopup);
            setCompletionMode(KCompletion::CompletionPopup);
        }
    }
}

bool KComboBox::autoCompletion() const
{
    return completionMode() == KCompletion::CompletionAuto;
}

bool KComboBox::urlDropsEnabled() const
{
    Q_D(const KComboBox);
    return d->klineEdit && d->klineEdit->urlDropsEnabled();
}

void KComboBox::setCompletedText(const QString &text, bool marked)
{
    Q_D(KComboBox);
    if (d->klineEdit) {
        d->klineEdit->setCompletedText(text, marked);
    }
}

void KComboBox::setCompletedText(const QString &text)
{
    Q_D(KComboBox);
    if (d->klineEdit) {
        d->klineEdit->setCompletedText(text);
    }
}

void KComboBox::makeCompletion(const QString &text)
{
    Q_D(KComboBox);
    if (d->klineEdit) {
        d->klineEdit->makeCompletion(text);
    }

    else { // read-only combo completion
        if (text.isNull() || !view()) {
            return;
        }

        view()->keyboardSearch(text);
    }
}

void KComboBox::rotateText(KCompletionBase::KeyBindingType type)
{
    Q_D(KComboBox);
    if (d->klineEdit) {
        d->klineEdit->rotateText(type);
    }
}

void KComboBox::setTrapReturnKey(bool trap)
{
    Q_D(KComboBox);
    d->trapReturnKey = trap;

    if (d->klineEdit) {
        d->klineEdit->setTrapReturnKey(trap);
    } else {
        qCWarning(KCOMPLETION_LOG) << "KComboBox::setTrapReturnKey not supported with a non-KLineEdit.";
    }
}

bool KComboBox::trapReturnKey() const
{
    Q_D(const KComboBox);
    return d->trapReturnKey;
}

void KComboBox::setEditUrl(const QUrl &url)
{
    QComboBox::setEditText(url.toDisplayString());
}

void KComboBox::addUrl(const QUrl &url)
{
    QComboBox::addItem(url.toDisplayString());
}

void KComboBox::addUrl(const QIcon &icon, const QUrl &url)
{
    QComboBox::addItem(icon, url.toDisplayString());
}

void KComboBox::insertUrl(int index, const QUrl &url)
{
    QComboBox::insertItem(index, url.toDisplayString());
}

void KComboBox::insertUrl(int index, const QIcon &icon, const QUrl &url)
{
    QComboBox::insertItem(index, icon, url.toDisplayString());
}

void KComboBox::changeUrl(int index, const QUrl &url)
{
    QComboBox::setItemText(index, url.toDisplayString());
}

void KComboBox::changeUrl(int index, const QIcon &icon, const QUrl &url)
{
    QComboBox::setItemIcon(index, icon);
    QComboBox::setItemText(index, url.toDisplayString());
}

void KComboBox::setCompletedItems(const QStringList &items, bool autoSuggest)
{
    Q_D(KComboBox);
    if (d->klineEdit) {
        d->klineEdit->setCompletedItems(items, autoSuggest);
    }
}

KCompletionBox *KComboBox::completionBox(bool create)
{
    Q_D(KComboBox);
    if (d->klineEdit) {
        return d->klineEdit->completionBox(create);
    }
    return nullptr;
}

QSize KComboBox::minimumSizeHint() const
{
    Q_D(const KComboBox);
    QSize size = QComboBox::minimumSizeHint();
    if (isEditable() && d->klineEdit) {
        // if it's a KLineEdit and it's editable add the clear button size
        // to the minimum size hint, otherwise looks ugly because the
        // clear button will cover the last 2/3 letters of the biggest entry
        QSize bs = d->klineEdit->clearButtonUsedSize();
        if (bs.isValid()) {
            size.rwidth() += bs.width();
            size.rheight() = qMax(size.height(), bs.height());
        }
    }
    return size;
}

void KComboBox::setLineEdit(QLineEdit *edit)
{
    Q_D(KComboBox);
    if (!isEditable() && edit && !qstrcmp(edit->metaObject()->className(), "QLineEdit")) {
        // uic generates code that creates a read-only KComboBox and then
        // calls combo->setEditable(true), which causes QComboBox to set up
        // a dumb QLineEdit instead of our nice KLineEdit.
        // As some KComboBox features rely on the KLineEdit, we reject
        // this order here.
        delete edit;
        KLineEdit *kedit = new KLineEdit(this);

        if (isEditable()) {
            kedit->setClearButtonEnabled(true);
        }

        edit = kedit;
    }

    // reuse an existing completion object, if it does not belong to the previous
    // line edit and gets destroyed with it
    QPointer<KCompletion> completion = compObj();

    QComboBox::setLineEdit(edit);
    edit->setCompleter(nullptr); // remove Qt's builtin completer (set by setLineEdit), we have our own
    d->klineEdit = qobject_cast<KLineEdit *>(edit);
    setDelegate(d->klineEdit);

    if (completion && d->klineEdit) {
        d->klineEdit->setCompletionObject(completion);
    }

    if (d->klineEdit) {
        // someone calling KComboBox::setEditable(false) destroys our
        // line edit without us noticing. And KCompletionBase::delegate would
        // be a dangling pointer then, so prevent that. Note: only do this
        // when it is a KLineEdit!
        d->m_klineEditConnection = connect(edit, &QObject::destroyed, this, [d, edit]() {
            d->slotLineEditDeleted(edit);
        });

        connect(d->klineEdit, &KLineEdit::returnKeyPressed, this, qOverload<const QString &>(&KComboBox::returnPressed));

        connect(d->klineEdit, &KLineEdit::completion, this, &KComboBox::completion);

        connect(d->klineEdit, &KLineEdit::substringCompletion, this, &KComboBox::substringCompletion);

        connect(d->klineEdit, &KLineEdit::textRotation, this, &KComboBox::textRotation);

        connect(d->klineEdit, &KLineEdit::completionModeChanged, this, &KComboBox::completionModeChanged);

        connect(d->klineEdit, &KLineEdit::aboutToShowContextMenu, [this](QMenu *menu) {
            Q_D(KComboBox);
            d->contextMenu = menu;
            Q_EMIT aboutToShowContextMenu(menu);
        });

        connect(d->klineEdit, &KLineEdit::completionBoxActivated, this, &QComboBox::textActivated);

        d->klineEdit->setTrapReturnKey(d->trapReturnKey);
    }
}

QMenu *KComboBox::contextMenu() const
{
    return d_ptr->contextMenu;
}

void KComboBox::setCurrentItem(const QString &item, bool insert, int index)
{
    int sel = -1;

    const int itemCount = count();
    for (int i = 0; i < itemCount; ++i) {
        if (itemText(i) == item) {
            sel = i;
            break;
        }
    }

    if (sel == -1 && insert) {
        if (index >= 0) {
            insertItem(index, item);
            sel = index;
        } else {
            addItem(item);
            sel = count() - 1;
        }
    }
    setCurrentIndex(sel);
}

void KComboBox::setEditable(bool editable)
{
    if (editable == isEditable()) {
        return;
    }

    if (editable) {
        // Create a KLineEdit instead of a QLineEdit
        // Compared to QComboBox::setEditable, we might be missing the SH_ComboBox_Popup code though...
        // If a style needs this, then we'll need to call QComboBox::setEditable and then setLineEdit again
        KLineEdit *edit = new KLineEdit(this);
        edit->setClearButtonEnabled(true);
        setLineEdit(edit);
    } else {
        if (d_ptr->contextMenu) {
            d_ptr->contextMenu->close();
        }
        QComboBox::setEditable(editable);
    }
}

#include "moc_kcombobox.cpp"
