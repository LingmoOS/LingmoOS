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
 * Authors: SiSheng He <hesisheng@kylinos.cn>
 *
 */

#include "ksecurityquestiondialog.h"
#include <QObject>
#define LABELWIDTH 92
#define BUTTONWIDTH 96
#define DIALOGWIDTH 424
#define DIALOGHEIGHT 476

namespace kdk
{

class KSecurityQuestionDialogPrivate:public QObject
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(KSecurityQuestionDialog)
public:
    KSecurityQuestionDialogPrivate(KSecurityQuestionDialog *parent);

private:
    KSecurityQuestionDialog *q_ptr;
    QVBoxLayout *m_pMainVLayout = nullptr;
    QLabel *m_pTitleLabel = nullptr;
    KPushButton *m_pCancelKBtn = nullptr;
    KPushButton *m_pConfirmKBtn = nullptr;
    QList<QComboBox *> m_questionComboxList;
    QList<QLineEdit *> m_questionLineeditList;
    QList<QLineEdit *> m_answerLineeditList;
    QList<QLabel *> m_questionLabelList;
    QList<QLabel *> m_answerLabelList;
    QList<QLabel *> m_tipsLabelList;
    QStringList m_questionlist;
    bool m_isVerify = false;

public Q_SLOTS:
    void setQustionCombox();
    void refreshConfirmBtn();
};

KSecurityQuestionDialog::KSecurityQuestionDialog(QWidget *parent)
    :KDialog(parent),
     d_ptr(new KSecurityQuestionDialogPrivate(this))
{
    Q_D(KSecurityQuestionDialog);
    setObjectName("KSecurityQuestionDialog");
}

KSecurityQuestionDialog::~KSecurityQuestionDialog()
{
}

KPushButton *KSecurityQuestionDialog::cancelButton()
{
    Q_D(KSecurityQuestionDialog);
    return d->m_pCancelKBtn;
}

KPushButton *KSecurityQuestionDialog::confirmButton()
{
    Q_D(KSecurityQuestionDialog);
    return d->m_pConfirmKBtn;
}

void KSecurityQuestionDialog::setTitleText(const QString &text)
{
    Q_D(KSecurityQuestionDialog);
    d->m_pTitleLabel->setText(text);
}

void KSecurityQuestionDialog::addSecurityQuestionItem(const int count, bool mutex)
{
    Q_D(KSecurityQuestionDialog);
    for (int i = 1; i <= count; i++) {
        QLabel *questionLabel = new QLabel(tr("Security question%1").arg(i));
        questionLabel->setFixedWidth(LABELWIDTH);
        QHBoxLayout *questionHlayout = new QHBoxLayout();
        questionHlayout->setContentsMargins(0, 0, 0, 0);
        questionHlayout->addWidget(questionLabel);
        questionHlayout->addSpacing(16);

        QLabel *answerLabel =  new QLabel(tr("Answer"));
        answerLabel->setFixedWidth(LABELWIDTH);
        QLineEdit *answerLineEdit = new QLineEdit();
        answerLineEdit->setPlaceholderText(tr("Required"));
        QHBoxLayout *answerHlayout = new QHBoxLayout();
        answerHlayout->setContentsMargins(0, 0, 0, 0);
        answerHlayout->addWidget(answerLabel);
        answerHlayout->addSpacing(16);
        answerHlayout->addWidget(answerLineEdit);

        d->m_answerLineeditList.append(answerLineEdit);
        d->m_questionLabelList.append(questionLabel);
        d->m_answerLabelList.append(answerLabel);

        QVBoxLayout *itemVLayout = new QVBoxLayout;
        itemVLayout->setContentsMargins(0, 0, 0, 0);
        itemVLayout->setSpacing(0);
        itemVLayout->addLayout(questionHlayout);
        itemVLayout->addSpacing(8);
        itemVLayout->addLayout(answerHlayout);

        if (d->m_isVerify) {
            QLineEdit *questionLineedit = new QLineEdit();
            d->m_questionLineeditList.append(questionLineedit);
            questionLineedit->setEnabled(false);
            QPalette palette = questionLineedit->palette();
            QColor color = palette.color(QPalette::Active, QPalette::Text);
            palette.setColor(QPalette::Disabled, QPalette::Text, color);
            questionLineedit->setPalette(palette);
            questionHlayout->addWidget(questionLineedit);
            QLabel *tiplabel = new QLabel();
            d->m_tipsLabelList.append(tiplabel);
            QPalette pe;
            pe.setColor(QPalette::WindowText,Qt::red);
            tiplabel->setPalette(pe);
            QLabel *placeholderLabel =  new QLabel();
            placeholderLabel->setFixedWidth(LABELWIDTH);
            QHBoxLayout *tipHLayout = new QHBoxLayout();
            tipHLayout->setContentsMargins(0, 0, 0, 0);
            tipHLayout->addWidget(placeholderLabel);
            tipHLayout->addSpacing(16);
            tipHLayout->addWidget(tiplabel);

            itemVLayout->addLayout(tipHLayout);

        } else {
            QComboBox *questionCombox = new QComboBox();
            d->m_questionComboxList.append(questionCombox);
            questionHlayout->addWidget(questionCombox);
            itemVLayout->addStretch();

            if (mutex) {
                connect(questionCombox, &QComboBox::currentTextChanged, d, &KSecurityQuestionDialogPrivate::setQustionCombox);
            }
        }

        d->m_pMainVLayout->insertLayout(d->m_pMainVLayout->count() - 1, itemVLayout);

        connect(answerLineEdit, &QLineEdit::textChanged, d, &KSecurityQuestionDialogPrivate::refreshConfirmBtn);
    }
}

void KSecurityQuestionDialog::addSecurityQuestionItem(const int count, bool mutex, bool isVerify)
{
    Q_D(KSecurityQuestionDialog);
    d->m_isVerify = isVerify;
    addSecurityQuestionItem(count, mutex);
}

void KSecurityQuestionDialog::initQustionCombox(const QStringList &questionlist)
{
    Q_D(KSecurityQuestionDialog);
    d->m_questionlist = questionlist;
    for (int i = 0; i < d->m_questionComboxList.count(); i++) {
        QComboBox *question = d->m_questionComboxList.at(i);
        question->blockSignals(true);
        question->addItems(questionlist);
        question->blockSignals(false);
        question->setCurrentIndex(i);
    }

    for (int i = 0; i < d->m_questionLineeditList.count(); i++) {
        QLineEdit *question = d->m_questionLineeditList.at(i);
        if (questionlist.count() > i) {
            question->setText(questionlist.at(i));
        }
    }
}

QLabel *KSecurityQuestionDialog::tipsLabel(const int index)
{
    Q_D(KSecurityQuestionDialog);
    // questionIndex 最小为0 count最小为1，
    if (index >= 0 && d->m_tipsLabelList.count() > index) {
        return d->m_tipsLabelList.at(index);
    }

    return nullptr;
}

QLabel *KSecurityQuestionDialog::questionLabel(const int questionIndex)
{
    Q_D(KSecurityQuestionDialog);
    // questionIndex 最小为0 count最小为1，
    if (questionIndex >= 0 && d->m_questionLabelList.count() > questionIndex) {
        return d->m_questionLabelList.at(questionIndex);
    }

    return nullptr;
}

QLabel *KSecurityQuestionDialog::answerLabel(const int answerIndex)
{
    Q_D(KSecurityQuestionDialog);
    if (answerIndex >= 0 && d->m_answerLabelList.count() > answerIndex) {
        return d->m_answerLabelList.at(answerIndex);
    }

    return nullptr;
}

QComboBox *KSecurityQuestionDialog::questionCombox(const int questionIndex)
{
    Q_D(KSecurityQuestionDialog);
    if (questionIndex >= 0 && d->m_questionComboxList.count() > questionIndex) {
        return d->m_questionComboxList.at(questionIndex);
    }

    return nullptr;
}

QLineEdit *KSecurityQuestionDialog::answerLineedit(const int answerIndex)
{
    Q_D(KSecurityQuestionDialog);
    if (answerIndex >= 0 && d->m_answerLineeditList.count() > answerIndex) {
        return d->m_answerLineeditList.at(answerIndex);
    }

    return nullptr;
}

void KSecurityQuestionDialog::keyPressEvent(QKeyEvent * event)
{
    Q_D(KSecurityQuestionDialog);
    switch (event->key())
    {
    case Qt::Key_Enter:
        if (d->m_pConfirmKBtn->isEnabled())
            d->m_pConfirmKBtn->clicked();
        break;
    case Qt::Key_Return:
        if (d->m_pConfirmKBtn->isEnabled())
            d->m_pConfirmKBtn->clicked();
        break;
    default:
        KDialog::keyPressEvent(event);
    }
}

KSecurityQuestionDialogPrivate::KSecurityQuestionDialogPrivate(KSecurityQuestionDialog *parent)
    :q_ptr(parent)
{
    Q_Q(KSecurityQuestionDialog);
    setParent(parent);
    q->setWindowIcon("lingmo-control-center");
    q->setWindowTitle(tr("Security Question"));

    m_pTitleLabel = new QLabel(tr("Setting security questions to reset password when forget it, "
                                  "please remember answer."));
    m_pTitleLabel->setWordWrap(true);
    QHBoxLayout *titleHLayout = new QHBoxLayout();
    titleHLayout->setContentsMargins(0, 0, 0, 0);
    titleHLayout->addWidget(m_pTitleLabel);
    m_pCancelKBtn = new KPushButton();
    m_pCancelKBtn->setFocusPolicy(Qt::FocusPolicy::NoFocus);
    m_pCancelKBtn->setFixedWidth(BUTTONWIDTH);
    m_pCancelKBtn->setText(tr("Cancel"));
    m_pConfirmKBtn = new KPushButton();
    m_pConfirmKBtn->setFixedWidth(BUTTONWIDTH);
    m_pConfirmKBtn->setText(tr("Save"));
    m_pConfirmKBtn->setEnabled(false);
    m_pConfirmKBtn->setFocusPolicy(Qt::FocusPolicy::NoFocus);
    q->closeButton()->setFocusPolicy(Qt::FocusPolicy::NoFocus);

    QHBoxLayout *hLayout = new QHBoxLayout();
    hLayout->setContentsMargins(0, 0, 0, 0);
    hLayout->setSpacing(16);
    hLayout->addStretch();
    hLayout->addWidget(m_pCancelKBtn);
    hLayout->addWidget(m_pConfirmKBtn);

    m_pMainVLayout = new QVBoxLayout();
    m_pMainVLayout->setContentsMargins(21, 16, 27, 24);
    m_pMainVLayout->setSpacing(0);
    m_pMainVLayout->addLayout(titleHLayout);
    m_pMainVLayout->addSpacing(24);
    m_pMainVLayout->addLayout(hLayout);

    q->mainWidget()->setLayout(m_pMainVLayout);

    q->resize(DIALOGWIDTH, DIALOGHEIGHT);

    QObject::connect(m_pCancelKBtn, SIGNAL(clicked(bool)), q, SLOT(reject()));

    QObject::connect(m_pConfirmKBtn, &KPushButton::clicked, this, [=](){
        foreach (QLineEdit *tempLineedit, m_answerLineeditList) {
            if (tempLineedit != nullptr) {
                tempLineedit->clearFocus();
            }
        }
    });
}

// 安全问题下拉框问题不可重复，已选择问题下拉框中置灰
void KSecurityQuestionDialogPrivate::setQustionCombox()
{
    Q_Q(KSecurityQuestionDialog);
    QStringList allComboxCurrentText;
    for (int i = 0; i < m_questionComboxList.count(); i++) {
        QComboBox *tempCombox = m_questionComboxList.at(i);
        // 获取所有combox当前选择的text
        allComboxCurrentText.append(tempCombox->currentText());
    }

    for (int j = 0; j < m_questionComboxList.count(); j++) {
        QComboBox *tempCombox = m_questionComboxList.at(j);
        QString currentText =  tempCombox->currentText();
        if (m_questionlist.contains(currentText)) {
            tempCombox->blockSignals(true);
            tempCombox->clear();
            tempCombox->addItems(m_questionlist);
            tempCombox->setCurrentText(currentText);
            tempCombox->blockSignals(false);
            // 将在当前combox的其他combox的当前项置灰
            foreach (QString text, allComboxCurrentText) {
                if (text != currentText) {
                    QVariant v(0);
                    tempCombox->setItemData(tempCombox->findText(text), v, Qt::UserRole - 1);
                }
            }
        }
    }
}

void KSecurityQuestionDialogPrivate::refreshConfirmBtn()
{
    foreach (QLabel *tempLabel, m_tipsLabelList) {
        if (!tempLabel->text().isEmpty()) {
            tempLabel->clear();
        }
    }

    foreach (QLineEdit *tempLineedit, m_answerLineeditList) {
        if (tempLineedit->text().isEmpty()) {
            m_pConfirmKBtn->setEnabled(false);
            m_pConfirmKBtn->setProperty("isImportant", false);
            return;
        }
    }

    m_pConfirmKBtn->setEnabled(true);
    m_pConfirmKBtn->setProperty("isImportant", true);
}

}

#include "ksecurityquestiondialog.moc"
#include "moc_ksecurityquestiondialog.cpp"
