/*
 * liblingmosdk-qtwidgets's Library
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Zhen Sun <sunzhen1@kylinos.cn>
 *
 */

#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QLineEdit>
#include <QWidget>

#include <QPlainTextEdit>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QListView>
#include <QPointer>
#include <QVBoxLayout>
#include <QObject>
#include "qvalidator.h"
#include "qevent.h"
#include "kinputdialog.h"

namespace  kdk
{
enum CandidateSignal {
    TextValueSelectedSignal,
    IntValueSelectedSignal,
    DoubleValueSelectedSignal,

    NumCandidateSignals
};

static const char *candidateSignal(int which)
{
    switch (CandidateSignal(which)) {
    case TextValueSelectedSignal:   return SIGNAL(textValueSelected(QString));
    case IntValueSelectedSignal:    return SIGNAL(intValueSelected(int));
    case DoubleValueSelectedSignal: return SIGNAL(doubleValueSelected(double));

    case NumCandidateSignals:
        break;
    };
    Q_UNREACHABLE();
    return nullptr;
}

static const char *signalForMember(const char *member)
{
    QByteArray normalizedMember(QMetaObject::normalizedSignature(member));

    for (int i = 0; i < NumCandidateSignals; ++i)
        if (QMetaObject::checkConnectArgs(candidateSignal(i), normalizedMember))
            return candidateSignal(i);

    return SIGNAL(accepted());
}

QT_BEGIN_NAMESPACE

class KInputDialogSpinBox : public QSpinBox
{
    Q_OBJECT

public:
    KInputDialogSpinBox(QWidget *parent)
        : QSpinBox(parent) {
        connect(lineEdit(), SIGNAL(textChanged(QString)), this, SLOT(notifyTextChanged()));
        connect(this, SIGNAL(editingFinished()), this, SLOT(notifyTextChanged()));
    }

protected:
  QSize sizeHint() const;

Q_SIGNALS:
    void textChanged(int);

private Q_SLOTS:
    void notifyTextChanged() { Q_EMIT textChanged(hasAcceptableInput()?1:0); }

private:
    void keyPressEvent(QKeyEvent *event) override {
        if ((event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) && !hasAcceptableInput()) {
#ifndef QT_NO_PROPERTIES
            setProperty("value", property("value"));
#endif
        } else {
            QSpinBox::keyPressEvent(event);
        }
        notifyTextChanged();
    }

    void mousePressEvent(QMouseEvent *event) override {
        QSpinBox::mousePressEvent(event);
        notifyTextChanged();
    }
};

class KInputDialogDoubleSpinBox : public QDoubleSpinBox
{
    Q_OBJECT

public:
    KInputDialogDoubleSpinBox(QWidget *parent = nullptr)
        : QDoubleSpinBox(parent) {
        connect(lineEdit(), SIGNAL(textChanged(QString)), this, SLOT(notifyTextChanged()));
        connect(this, SIGNAL(editingFinished()), this, SLOT(notifyTextChanged()));
    }

protected:
  QSize sizeHint() const;

Q_SIGNALS:
    void textChanged(int);

private Q_SLOTS:
    void notifyTextChanged() { Q_EMIT textChanged(hasAcceptableInput()?1:0); }

private:
    void keyPressEvent(QKeyEvent *event) override {
        if ((event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) && !hasAcceptableInput()) {
#ifndef QT_NO_PROPERTIES
            setProperty("value", property("value"));
#endif
        } else {
            QDoubleSpinBox::keyPressEvent(event);
        }
        notifyTextChanged();
    }

    void mousePressEvent(QMouseEvent *event) override {
        QDoubleSpinBox::mousePressEvent(event);
        notifyTextChanged();
    }
};

class  KInputDialogLineEdit :public QLineEdit
{
  public:
    KInputDialogLineEdit(QWidget *parent = nullptr) : QLineEdit(parent){}
protected:
    QSize sizeHint() const;
};


class KInputDialogListView : public QListView
{
public:
    KInputDialogListView(QWidget *parent = nullptr) : QListView(parent) {}
    QVariant inputMethodQuery(Qt::InputMethodQuery query) const override
    {
        if (query == Qt::ImEnabled)
            return false;
        return QListView::inputMethodQuery(query);
    }
};

class KInputDialogPrivate : public QObject
{
    Q_DECLARE_PUBLIC(KInputDialog)

public:
    KInputDialogPrivate(KInputDialog*parent);

    void ensureLayout();
    void ensureLineEdit();
    void ensurePlainTextEdit();
    void ensureComboBox();
    void ensureListView();
    void ensureIntSpinBox();
    void ensureDoubleSpinBox();
    void ensureEnabledConnection(QAbstractSpinBox *spinBox);
    void setInputWidget(QWidget *widget);
    void chooseRightTextInputWidget();
    void setComboBoxText(const QString &text);
    void setListViewText(const QString &text);
    QString listViewText() const;
    void ensureLayout() const { const_cast<KInputDialogPrivate *>(this)->ensureLayout(); }
    bool useComboBoxOrListView() const { return comboBox && comboBox->count() > 0; }
    void _q_textChanged(const QString &text);
    void _q_plainTextEditTextChanged();
    void _q_currentRowChanged(const QModelIndex &newIndex, const QModelIndex &oldIndex);

    mutable QLabel *label;
    mutable QPushButton *okButton;
    mutable QPushButton *cancelButton;
    mutable KInputDialogLineEdit *lineEdit;
    mutable QPlainTextEdit *plainTextEdit;
    mutable QSpinBox *intSpinBox;
    mutable QDoubleSpinBox *doubleSpinBox;
    mutable QComboBox *comboBox;
    mutable KInputDialogListView *listView;
    mutable QWidget *inputWidget;
    mutable QVBoxLayout *mainLayout;
    KInputDialog::InputDialogOptions opts;
    QString textValue;
    QPointer<QObject> receiverToDisconnectOnClose;
    QByteArray memberToDisconnectOnClose;

private:
    KInputDialog* q_ptr;
};

KInputDialogPrivate::KInputDialogPrivate(KInputDialog*parent)
    : q_ptr(parent),label(nullptr), okButton(nullptr),cancelButton(nullptr), lineEdit(nullptr), plainTextEdit(nullptr), intSpinBox(nullptr), doubleSpinBox(nullptr),
      comboBox(nullptr), listView(nullptr), inputWidget(nullptr), mainLayout(nullptr)
{
    Q_Q(KInputDialog);
    q->setMinimumHeight(198);
    q->setMinimumWidth(336);
    connect(q->m_gsetting,&QGSettings::changed,q,&KInputDialog::changeTheme);
}

void KInputDialogPrivate::ensureLayout()
{
    Q_Q(KInputDialog);
    if (mainLayout)
        return;
    if (!inputWidget) {
        ensureLineEdit();
        inputWidget = lineEdit;
    }

    if (!label)
        label = new QLabel(KInputDialog::tr("Enter a value:"), q);

    q->mainLayout()->setSizeConstraint(QLayout::SizeConstraint::SetFixedSize);
#ifndef QT_NO_SHORTCUT
    label->setBuddy(inputWidget);
#endif
    label->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
    label->setWordWrap(true);
    QHBoxLayout *hLayout = new QHBoxLayout();
    okButton = new QPushButton(tr("ok"),q);
    okButton->setFixedSize(96,36);
    cancelButton = new QPushButton(tr("cancel"),q);
    cancelButton->setFixedSize(96,36);
    hLayout->setContentsMargins(0,0,0,0);
    hLayout->setSpacing(0);
    hLayout->addStretch();
    hLayout->addWidget(cancelButton);
    hLayout->addSpacing(10);
    hLayout->addWidget(okButton);
    QObject::connect(okButton, SIGNAL(clicked(bool)), q, SLOT(accept()));
    QObject::connect(cancelButton, SIGNAL(clicked(bool)), q, SLOT(reject()));

    mainLayout = new QVBoxLayout();
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(24,0,24,24);
    mainLayout->addWidget(label);
    mainLayout->addSpacing(8);
    mainLayout->addWidget(inputWidget);
    mainLayout->addSpacing(32);
    //mainLayout->addStretch();
    mainLayout->addLayout(hLayout);
    q->mainWidget()->setLayout(mainLayout);
    q->changeTheme();
    ensureEnabledConnection(qobject_cast<QAbstractSpinBox *>(inputWidget));
    inputWidget->show();

}

void KInputDialogPrivate::ensureLineEdit()
{
    Q_Q(KInputDialog);
    if (!lineEdit) {
        lineEdit = new KInputDialogLineEdit(q);
        lineEdit->setFixedHeight(36);
        lineEdit->hide();
        QObject::connect(lineEdit, SIGNAL(textChanged(QString)),
                         q, SLOT(_q_textChanged(QString)));
    }
}

void KInputDialogPrivate::ensurePlainTextEdit()
{
    Q_Q(KInputDialog);
    if (!plainTextEdit) {
        plainTextEdit = new QPlainTextEdit(q);
        plainTextEdit->setLineWrapMode(QPlainTextEdit::NoWrap);
        plainTextEdit->hide();
        QObject::connect(plainTextEdit, SIGNAL(textChanged()),
                         q, SLOT(_q_plainTextEditTextChanged()));
    }
}

void KInputDialogPrivate::ensureComboBox()
{
    Q_Q(KInputDialog);
    if (!comboBox) {
        comboBox = new QComboBox(q);
        comboBox->hide();
        QObject::connect(comboBox, SIGNAL(editTextChanged(QString)),
                         q, SLOT(_q_textChanged(QString)));
        QObject::connect(comboBox, SIGNAL(currentIndexChanged(QString)),
                         q, SLOT(_q_textChanged(QString)));
    }
}

void KInputDialogPrivate::ensureListView()
{
    Q_Q(KInputDialog);
    if (!listView) {
        ensureComboBox();
        listView = new KInputDialogListView(q);
        listView->hide();
        listView->setEditTriggers(QAbstractItemView::NoEditTriggers);
        listView->setSelectionMode(QAbstractItemView::SingleSelection);
        listView->setModel(comboBox->model());
        listView->setCurrentIndex(QModelIndex());
        QObject::connect(listView->selectionModel(),
                         SIGNAL(currentRowChanged(QModelIndex,QModelIndex)),
                         q, SLOT(_q_currentRowChanged(QModelIndex,QModelIndex)));
    }
}

void KInputDialogPrivate::ensureIntSpinBox()
{
    Q_Q(KInputDialog);
    if (!intSpinBox) {
        intSpinBox = new KInputDialogSpinBox(q);
        intSpinBox->hide();
        QObject::connect(intSpinBox, SIGNAL(valueChanged(int)),
                         q, SIGNAL(intValueChanged(int)));
    }
}

void KInputDialogPrivate::ensureDoubleSpinBox()
{
    Q_Q(KInputDialog);
    if (!doubleSpinBox) {
        doubleSpinBox = new KInputDialogDoubleSpinBox(q);
        doubleSpinBox->hide();
        QObject::connect(doubleSpinBox, SIGNAL(valueChanged(double)),
                         q, SIGNAL(doubleValueChanged(double)));
    }
}

void KInputDialogPrivate::ensureEnabledConnection(QAbstractSpinBox *spinBox)
{
    if (spinBox) {
        QAbstractButton *okBtn = okButton;
        QObject::connect(spinBox, SIGNAL(textChanged(int)), okBtn, SLOT(setEnabled(bool)), Qt::UniqueConnection);
    }
}

void KInputDialogPrivate::setInputWidget(QWidget *widget)
{
    Q_Q(KInputDialog);
    Q_ASSERT(widget);
    if (inputWidget == widget)
        return;

    if (mainLayout) {
        Q_ASSERT(inputWidget);
        mainLayout->removeWidget(inputWidget);
        inputWidget->hide();
        mainLayout->insertWidget(1, widget);
        widget->show();

        QAbstractButton *okBtn = okButton;
        if (QAbstractSpinBox *spinBox = qobject_cast<QAbstractSpinBox *>(inputWidget))
            QObject::disconnect(spinBox, SIGNAL(textChanged(int)), okBtn, SLOT(setEnabled(bool)));

        QAbstractSpinBox *spinBox = qobject_cast<QAbstractSpinBox *>(widget);
        ensureEnabledConnection(spinBox);
        okButton->setEnabled(!spinBox || spinBox->hasAcceptableInput());
        q->changeTheme();
    }

    inputWidget = widget;

    if (widget == lineEdit) {
        lineEdit->setText(textValue);
    } else if (widget == plainTextEdit) {
        plainTextEdit->setPlainText(textValue);
    } else if (widget == comboBox) {
        setComboBoxText(textValue);
    } else if (widget == listView) {
        setListViewText(textValue);
        ensureLayout();
        okButton->setEnabled(listView->selectionModel()->hasSelection());
    }
}

void KInputDialogPrivate::chooseRightTextInputWidget()
{
    QWidget *widget;

    if (useComboBoxOrListView()) {
        if ((opts & KInputDialog::UseListViewForComboBoxItems) && !comboBox->isEditable()) {
            ensureListView();
            widget = listView;
        } else {
            widget = comboBox;
        }
    } else if (opts & KInputDialog::UsePlainTextEditForTextInput) {
        ensurePlainTextEdit();
        widget = plainTextEdit;
    } else {
        ensureLineEdit();
        widget = lineEdit;
    }

    setInputWidget(widget);

    if (inputWidget == comboBox) {
        _q_textChanged(comboBox->currentText());
    } else if (inputWidget == listView) {
        _q_textChanged(listViewText());
    }
}

void KInputDialogPrivate::setComboBoxText(const QString &text)
{
    int index = comboBox->findText(text);
    if (index != -1) {
        comboBox->setCurrentIndex(index);
    } else if (comboBox->isEditable()) {
        comboBox->setEditText(text);
    }
}

void KInputDialogPrivate::setListViewText(const QString &text)
{
    int row = comboBox->findText(text);
    if (row != -1) {
        QModelIndex index(comboBox->model()->index(row, 0));
        listView->selectionModel()->setCurrentIndex(index, QItemSelectionModel::Clear
                                                         | QItemSelectionModel::SelectCurrent);
    }
}

QString KInputDialogPrivate::listViewText() const
{
    if (listView->selectionModel()->hasSelection()) {
        int row = listView->selectionModel()->selectedRows().value(0).row();
        return comboBox->itemText(row);
    } else {
        return QString();
    }
}

void KInputDialogPrivate::_q_textChanged(const QString &text)
{
    Q_Q(KInputDialog);
    if (textValue != text) {
        textValue = text;
        Q_EMIT q->textValueChanged(text);
    }
}

void KInputDialogPrivate::_q_plainTextEditTextChanged()
{
    Q_Q(KInputDialog);
    QString text = plainTextEdit->toPlainText();
    if (textValue != text) {
        textValue = text;
        Q_EMIT q->textValueChanged(text);
    }
}

void KInputDialogPrivate::_q_currentRowChanged(const QModelIndex &newIndex,
                                               const QModelIndex & /* oldIndex */)
{
    _q_textChanged(comboBox->model()->data(newIndex).toString());
    okButton->setEnabled(true);
}


KInputDialog::KInputDialog(QWidget *parent)
    : KDialog(parent),d_ptr(new KInputDialogPrivate(this))
{
}


KInputDialog::~KInputDialog()
{
}


void KInputDialog::setInputMode(InputMode mode)
{
    Q_D(KInputDialog);

    QWidget *widget;

    /*
        Warning: Some functions in KInputDialog rely on implementation details
        of the code below. Look for the comments that accompany the calls to
        setInputMode() throughout this file before you change the code below.
    */

    switch (mode) {
    case IntInput:
        d->ensureIntSpinBox();
        widget = d->intSpinBox;
        break;
    case DoubleInput:
        d->ensureDoubleSpinBox();
        widget = d->doubleSpinBox;
        break;
    default:
        Q_ASSERT(mode == TextInput);
        d->chooseRightTextInputWidget();
        return;
    }

    d->setInputWidget(widget);
}

KInputDialog::InputMode KInputDialog::inputMode() const
{
    Q_D(const KInputDialog);

    if (d->inputWidget) {
        if (d->inputWidget == d->intSpinBox) {
            return IntInput;
        } else if (d->inputWidget == d->doubleSpinBox) {
            return DoubleInput;
        }
    }

    return TextInput;
}


void KInputDialog::setLabelText(const QString &text)
{
    Q_D(KInputDialog);
    if (!d->label) {
        d->label = new QLabel(text, this);
    } else {
        d->label->setText(text);
    }
}

QString KInputDialog::labelText() const
{
    Q_D(const KInputDialog);
    d->ensureLayout();
    return d->label->text();
}

void KInputDialog::setOption(InputDialogOption option, bool on)
{
    Q_D(KInputDialog);
    if (!(d->opts & option) != !on)
        setOptions(d->opts ^ option);
}


bool KInputDialog::testOption(InputDialogOption option) const
{
    Q_D(const KInputDialog);
    return (d->opts & option) != 0;
}


void KInputDialog::setOptions(InputDialogOptions options)
{
    Q_D(KInputDialog);

    InputDialogOptions changed = (options ^ d->opts);
    if (!changed)
        return;

    d->opts = options;
    d->ensureLayout();

    if (changed & NoButtons)
    {
        d->okButton->setVisible(!(options & NoButtons));
        d->cancelButton->setVisible(!(options & NoButtons));
    }
    if ((changed & UseListViewForComboBoxItems) && inputMode() == TextInput)
        d->chooseRightTextInputWidget();
    if ((changed & UsePlainTextEditForTextInput) && inputMode() == TextInput)
        d->chooseRightTextInputWidget();
}

KInputDialog::InputDialogOptions KInputDialog::options() const
{
    Q_D(const KInputDialog);
    return d->opts;
}


void KInputDialog::setTextValue(const QString &text)
{
    Q_D(KInputDialog);

    setInputMode(TextInput);
    if (d->inputWidget == d->lineEdit) {
        d->lineEdit->setText(text);
    } else if (d->inputWidget == d->plainTextEdit) {
        d->plainTextEdit->setPlainText(text);
    } else if (d->inputWidget == d->comboBox) {
        d->setComboBoxText(text);
    } else {
        d->setListViewText(text);
    }
}

QString KInputDialog::textValue() const
{
    Q_D(const KInputDialog);
    return d->textValue;
}


void KInputDialog::setTextEchoMode(QLineEdit::EchoMode mode)
{
    Q_D(KInputDialog);
    d->ensureLineEdit();
    d->lineEdit->setEchoMode(mode);
}

QLineEdit::EchoMode KInputDialog::textEchoMode() const
{
    Q_D(const KInputDialog);
    if (d->lineEdit) {
        return d->lineEdit->echoMode();
    } else {
        return QLineEdit::Normal;
    }
}


void KInputDialog::setComboBoxEditable(bool editable)
{
    Q_D(KInputDialog);
    d->ensureComboBox();
    d->comboBox->setEditable(editable);
    if (inputMode() == TextInput)
        d->chooseRightTextInputWidget();
}

bool KInputDialog::isComboBoxEditable() const
{
    Q_D(const KInputDialog);
    if (d->comboBox) {
        return d->comboBox->isEditable();
    } else {
        return false;
    }
}


void KInputDialog::setComboBoxItems(const QStringList &items)
{
    Q_D(KInputDialog);

    d->ensureComboBox();
    {
        const QSignalBlocker blocker(d->comboBox);
        d->comboBox->clear();
        d->comboBox->addItems(items);
    }

    if (inputMode() == TextInput)
        d->chooseRightTextInputWidget();
}

QStringList KInputDialog::comboBoxItems() const
{
    Q_D(const KInputDialog);
    QStringList result;
    if (d->comboBox) {
        const int count = d->comboBox->count();
        result.reserve(count);
        for (int i = 0; i < count; ++i)
            result.append(d->comboBox->itemText(i));
    }
    return result;
}


void KInputDialog::setIntValue(int value)
{
    Q_D(KInputDialog);
    setInputMode(IntInput);
    d->intSpinBox->setValue(value);
}

int KInputDialog::intValue() const
{
    Q_D(const KInputDialog);
    if (d->intSpinBox) {
        return d->intSpinBox->value();
    } else {
        return 0;
    }
}


void KInputDialog::setIntMinimum(int min)
{
    Q_D(KInputDialog);
    d->ensureIntSpinBox();
    d->intSpinBox->setMinimum(min);
}

int KInputDialog::intMinimum() const
{
    Q_D(const KInputDialog);
    if (d->intSpinBox) {
        return d->intSpinBox->minimum();
    } else {
        return 0;
    }
}


void KInputDialog::setIntMaximum(int max)
{
    Q_D(KInputDialog);
    d->ensureIntSpinBox();
    d->intSpinBox->setMaximum(max);
}

int KInputDialog::intMaximum() const
{
    Q_D(const KInputDialog);
    if (d->intSpinBox) {
        return d->intSpinBox->maximum();
    } else {
        return 99;
    }
}


void KInputDialog::setIntRange(int min, int max)
{
    Q_D(KInputDialog);
    d->ensureIntSpinBox();
    d->intSpinBox->setRange(min, max);
}


void KInputDialog::setIntStep(int step)
{
    Q_D(KInputDialog);
    d->ensureIntSpinBox();
    d->intSpinBox->setSingleStep(step);
}

int KInputDialog::intStep() const
{
    Q_D(const KInputDialog);
    if (d->intSpinBox) {
        return d->intSpinBox->singleStep();
    } else {
        return 1;
    }
}


void KInputDialog::setDoubleValue(double value)
{
    Q_D(KInputDialog);
    setInputMode(DoubleInput);
    d->doubleSpinBox->setValue(value);
}

double KInputDialog::doubleValue() const
{
    Q_D(const KInputDialog);
    if (d->doubleSpinBox) {
        return d->doubleSpinBox->value();
    } else {
        return 0.0;
    }
}


void KInputDialog::setDoubleMinimum(double min)
{
    Q_D(KInputDialog);
    d->ensureDoubleSpinBox();
    d->doubleSpinBox->setMinimum(min);
}

double KInputDialog::doubleMinimum() const
{
    Q_D(const KInputDialog);
    if (d->doubleSpinBox) {
        return d->doubleSpinBox->minimum();
    } else {
        return 0.0;
    }
}


void KInputDialog::setDoubleMaximum(double max)
{
    Q_D(KInputDialog);
    d->ensureDoubleSpinBox();
    d->doubleSpinBox->setMaximum(max);
}

double KInputDialog::doubleMaximum() const
{
    Q_D(const KInputDialog);
    if (d->doubleSpinBox) {
        return d->doubleSpinBox->maximum();
    } else {
        return 99.99;
    }
}


void KInputDialog::setDoubleRange(double min, double max)
{
    Q_D(KInputDialog);
    d->ensureDoubleSpinBox();
    d->doubleSpinBox->setRange(min, max);
}


void KInputDialog::setDoubleDecimals(int decimals)
{
    Q_D(KInputDialog);
    d->ensureDoubleSpinBox();
    d->doubleSpinBox->setDecimals(decimals);
}

int KInputDialog::doubleDecimals() const
{
    Q_D(const KInputDialog);
    if (d->doubleSpinBox) {
        return d->doubleSpinBox->decimals();
    } else {
        return 2;
    }
}


void KInputDialog::setOkButtonText(const QString &text)
{
    Q_D(const KInputDialog);
    d->ensureLayout();
    d->okButton->setText(text);
}

QString KInputDialog::okButtonText() const
{
    Q_D(const KInputDialog);
    d->ensureLayout();
    return d->okButton->text();
}

void KInputDialog::setCancelButtonText(const QString &text)
{
    Q_D(const KInputDialog);
    d->ensureLayout();
    d->cancelButton->setText(text);
}

QString KInputDialog::cancelButtonText() const
{
    Q_D(const KInputDialog);
    d->ensureLayout();
    return d->cancelButton->text();
}

void KInputDialog::open(QObject *receiver, const char *member)
{
    Q_D(KInputDialog);
    connect(this, signalForMember(member), receiver, member);
    d->receiverToDisconnectOnClose = receiver;
    d->memberToDisconnectOnClose = member;
    QDialog::open();
}


QSize KInputDialog::minimumSizeHint() const
{
    Q_D(const KInputDialog);
    d->ensureLayout();
    return QDialog::minimumSizeHint();
}

QSize KInputDialog::sizeHint() const
{
    Q_D(const KInputDialog);
    d->ensureLayout();
    return QDialog::sizeHint();
}

void KInputDialog::setVisible(bool visible)
{
    Q_D(const KInputDialog);
    if (visible) {
        d->ensureLayout();
        d->inputWidget->setFocus();
        if (d->inputWidget == d->lineEdit) {
            d->lineEdit->selectAll();
        } else if (d->inputWidget == d->plainTextEdit) {
            d->plainTextEdit->selectAll();
        } else if (d->inputWidget == d->intSpinBox) {
            d->intSpinBox->selectAll();
        } else if (d->inputWidget == d->doubleSpinBox) {
            d->doubleSpinBox->selectAll();
        }
    }
    QDialog::setVisible(visible);
}

QString KInputDialog::placeholderText() const
{
    Q_D(const KInputDialog);
    if(d->lineEdit)
        return d->lineEdit->placeholderText();
    else
        return QString();
}

void KInputDialog::setPlaceholderText(const QString &str)
{
    Q_D(const KInputDialog);
    if(d->lineEdit)
        d->lineEdit->setPlaceholderText(str);
}


void KInputDialog::done(int result)
{
    Q_D(KInputDialog);
    QDialog::done(result);
    if (result) {
        InputMode mode = inputMode();
        switch (mode) {
        case DoubleInput:
            Q_EMIT doubleValueSelected(doubleValue());
            break;
        case IntInput:
            Q_EMIT intValueSelected(intValue());
            break;
        default:
            Q_ASSERT(mode == TextInput);
            Q_EMIT textValueSelected(textValue());
        }
    }
    if (d->receiverToDisconnectOnClose) {
        disconnect(this, signalForMember(d->memberToDisconnectOnClose),
                   d->receiverToDisconnectOnClose, d->memberToDisconnectOnClose);
        d->receiverToDisconnectOnClose = nullptr;
    }
    d->memberToDisconnectOnClose.clear();
}

void KInputDialog::changeTheme()
{
    Q_D(KInputDialog);
    KDialog::changeTheme();
    if(d->okButton)
        d->okButton->setProperty("isImportant",true);
}


QString KInputDialog::getText(QWidget *parent,const QString &label,
                              QLineEdit::EchoMode mode, const QString &text, bool *ok,
                              Qt::WindowFlags flags, Qt::InputMethodHints inputMethodHints)
{
    KInputDialog* dialog = new KInputDialog(parent);
    dialog->setLabelText(label);
    dialog->setTextValue(text);
    dialog->setTextEchoMode(mode);
    dialog->setInputMethodHints(inputMethodHints);

    const int ret = dialog->exec();
    if (ok)
        *ok = !!ret;
    if (ret) {
        return dialog->textValue();
    } else {
        return QString();
    }
}



QString KInputDialog::getMultiLineText(QWidget *parent,const QString &label,
                                       const QString &text, bool *ok, Qt::WindowFlags flags,
                                       Qt::InputMethodHints inputMethodHints)
{
    KInputDialog* dialog = new KInputDialog(parent);
    dialog->setOptions(KInputDialog::UsePlainTextEditForTextInput);
    dialog->setLabelText(label);
    dialog->setTextValue(text);
    dialog->setInputMethodHints(inputMethodHints);

    const int ret = dialog->exec();
    if (ok)
        *ok = !!ret;
    if (ret) {
        return dialog->textValue();
    } else {
        return QString();
    }
}


int KInputDialog::getInt(QWidget *parent,const QString &label, int value,
                         int min, int max, int step, bool *ok, Qt::WindowFlags flags)
{
    KInputDialog* dialog = new KInputDialog(parent);
    dialog->setLabelText(label);
    dialog->setIntRange(min, max);
    dialog->setIntValue(value);
    dialog->setIntStep(step);

    const int ret = dialog->exec();
    if (ok)
        *ok = !!ret;
    if (ret) {
        return dialog->intValue();
    } else {
        return value;
    }
}

double KInputDialog::getDouble(QWidget *parent,const QString &label, double value, double minValue, double maxValue, int decimals, bool *ok, Qt::WindowFlags flags)
{
    KInputDialog* dialog = new KInputDialog(parent);
    dialog->setLabelText(label);
    dialog->setDoubleDecimals(decimals);
    dialog->setDoubleRange(minValue, maxValue);
    dialog->setDoubleValue(value);

    const int ret = dialog->exec();
    if (ok)
        *ok = !!ret;
    if (ret) {
        return dialog->doubleValue();
    } else {
        return value;
    }
}

QString KInputDialog::getItem(QWidget *parent,const QString &label,
                              const QStringList &items, int current, bool editable, bool *ok,
                              Qt::WindowFlags flags, Qt::InputMethodHints inputMethodHints)
{
    QString text(items.value(current));

    KInputDialog* dialog = new KInputDialog(parent);
    dialog->setLabelText(label);
    dialog->setComboBoxItems(items);
    dialog->setTextValue(text);
    dialog->setComboBoxEditable(editable);
    dialog->setInputMethodHints(inputMethodHints);

    const int ret = dialog->exec();
    if (ok)
        *ok = !!ret;
    if (ret) {
        return dialog->textValue();
    } else {
        return text;
    }
}

void KInputDialog::setDoubleStep(double step)
{
    Q_D(KInputDialog);
    d->ensureDoubleSpinBox();
    d->doubleSpinBox->setSingleStep(step);
}

double KInputDialog::doubleStep() const
{
    Q_D(const KInputDialog);
    if (d->doubleSpinBox)
        return d->doubleSpinBox->singleStep();
    else
        return 1.0;
}

QSize KInputDialogSpinBox::sizeHint() const
{
    QSize size;
    size.setWidth(288);
    size.setHeight(36);
    return size;
}

QSize KInputDialogLineEdit::sizeHint() const
{
    QSize size;
    size.setWidth(288);
    size.setHeight(36);
    return size;
}

QSize KInputDialogDoubleSpinBox::sizeHint() const
{
    QSize size;
    size.setWidth(288);
    size.setHeight(36);
    return size;
}

}


#include "kinputdialog.moc"
#include "moc_kinputdialog.cpp"
