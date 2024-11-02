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

#include "kprogressdialog.h"

namespace kdk
{
class KProgressDialogPrivate:public QObject
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(KProgressDialog)

public:
    KProgressDialogPrivate(KProgressDialog*parent);
    void adjustNormalMode();
    void infoReset();

private:
    KProgressDialog* const q_ptr;

    QProgressBar *m_pProgressBar;
    QLabel *m_pMainLabel;
    QLabel *m_pPercentLabel;
    QLabel *m_pSubContentLabel;
    QLabel *m_pProgressLabel;
    QPushButton* m_pCanelButton;
    QVBoxLayout* m_pMainLayout;

    QString m_suffix;

    bool m_autoClose;
    bool m_autoReset;
    bool m_forceHide;
};

KProgressDialog::KProgressDialog(QWidget *parent)
    :KDialog(parent),
      d_ptr(new KProgressDialogPrivate(this))
{
    Q_D(KProgressDialog);
//    setFixedSize(440,265);
    this->layout()->setSizeConstraint(QLayout::SizeConstraint::SetFixedSize);
    d->m_pMainLayout = new QVBoxLayout();
    d->m_pMainLayout->setContentsMargins(25,0,25,30);
    d->m_pMainLayout->setSpacing(0);
    d->m_pMainLabel = new QLabel(this);

    d->m_pMainLabel->setAlignment(Qt::AlignLeft);
    d->m_pProgressBar = new QProgressBar(this);
    d->m_pCanelButton = new QPushButton(this);
    d->m_pCanelButton->setText(tr("cancel"));
    d->m_pSubContentLabel = new QLabel(this);
    d->m_pPercentLabel = new QLabel(this);
    d->m_pProgressLabel = new QLabel(this);

    d->m_autoClose = true;
    d->m_autoReset = true;
    d->m_forceHide = false;

    d->adjustNormalMode();
    setShowDetail(false);
    changeTheme();
    connect(m_gsetting,&QGSettings::changed,this,&KProgressDialog::changeTheme);
    connect(this, SIGNAL(canceled()), this, SLOT(cancel()));
    connect(d->m_pCanelButton, SIGNAL(clicked()), this, SIGNAL(canceled()));
}

KProgressDialog::KProgressDialog(const QString &labelText, const QString &cancelButtonText, int minimum, int maximum, QWidget *parent)
    :KProgressDialog(parent)
{
    Q_D(KProgressDialog);
    d->m_pMainLabel->setText(labelText);
    d->m_pProgressBar->setRange(minimum,maximum);
    d->m_pCanelButton->setText(cancelButtonText);
    d->adjustNormalMode();
    setShowDetail(true);
}

KProgressDialog::~KProgressDialog()
{

}

void KProgressDialog::setLabel(QLabel *label)
{
    Q_D(KProgressDialog);
    if(!label)
        return;
    if(d->m_pMainLabel)
        delete d->m_pMainLabel;
    d->m_pMainLabel = label;
    d->m_pMainLabel->setVisible(true);
}

void KProgressDialog::setCancelButton(QPushButton *button)
{
    Q_D(KProgressDialog);
    if(!button)
        return;
    if(d->m_pCanelButton)
        delete d->m_pCanelButton;
    d->m_pCanelButton = button;
}

void KProgressDialog::setBar(QProgressBar *bar)
{
    Q_D(KProgressDialog);
    if(!bar)
        return;
    if(d->m_pProgressBar)
        delete d->m_pProgressBar;
    d->m_pProgressBar = bar;
}

void KProgressDialog::setSuffix(const QString &suffix)
{
    Q_D(KProgressDialog);
    d->m_suffix = suffix;
}

void KProgressDialog::setShowDetail(bool flag)
{
    Q_D(KProgressDialog);
    if(flag)
    {
        d->m_pSubContentLabel->setVisible(true);
        d->m_pProgressLabel->setVisible(true);
        d->m_pPercentLabel->setVisible(true);
    }
    else
    {
        d->m_pSubContentLabel->setVisible(false);
        d->m_pProgressLabel->setVisible(false);
        d->m_pPercentLabel->setVisible(false);
    }
}


int KProgressDialog::minimum() const
{
    Q_D(const KProgressDialog);
    if(d->m_pProgressBar)
        return d->m_pProgressBar->minimum();
    else
        return -1;
}

int KProgressDialog::maximum() const
{
    Q_D(const KProgressDialog);
    if(d->m_pProgressBar)
        return d->m_pProgressBar->maximum();
    else
        return -1;
}

int KProgressDialog::value() const
{
    Q_D(const KProgressDialog);
    if(d->m_pProgressBar)
        return d->m_pProgressBar->value();
    else
        return -1;
}

QString KProgressDialog::labelText() const
{
    Q_D(const KProgressDialog);
    if(d->m_pMainLabel)
        return d->m_pMainLabel->text();
    else
    {
        return QString();
    }
}

void KProgressDialog::setAutoReset(bool reset)
{
    Q_D(KProgressDialog);
    d->m_autoReset = reset;
}

bool KProgressDialog::autoReset() const
{
    Q_D(const KProgressDialog);
    return d->m_autoReset;
}

void KProgressDialog::setAutoClose(bool close)
{
    Q_D(KProgressDialog);
    d->m_autoClose = close;
}

bool KProgressDialog::autoClose() const
{
    Q_D(const KProgressDialog);
    return d->m_autoClose;
}

QProgressBar *KProgressDialog::progressBar()
{
    Q_D(KProgressDialog);
    return d->m_pProgressBar;
}

void KProgressDialog::cancel()
{
    Q_D(KProgressDialog);
    d->m_forceHide = true;
    reset();
    d->m_forceHide = false;
}

void KProgressDialog::reset()
{
    Q_D(KProgressDialog);
    if (d->m_autoClose || d->m_forceHide)
        hide();
    d->m_pProgressBar->reset();
    d->infoReset();
}

void KProgressDialog::setMaximum(int maximum)
{
    Q_D(KProgressDialog);
    if(d->m_pProgressBar)
        d->m_pProgressBar->setMaximum(maximum);
}

void KProgressDialog::setMinimum(int minimum)
{
    Q_D(KProgressDialog);
    if(d->m_pProgressBar)
        d->m_pProgressBar->setMinimum(minimum);
}

void KProgressDialog::setRange(int minimum, int maximum)
{
    Q_D(KProgressDialog);
    if(d->m_pProgressBar)
        d->m_pProgressBar->setRange(minimum,maximum);
}

void KProgressDialog::setValue(int progress)
{
    Q_D(KProgressDialog);
    if(d->m_pProgressBar)
        d->m_pProgressBar->setValue(progress);
    if(d->m_pPercentLabel)
        d->m_pPercentLabel->setText(QString::number(progress*100/maximum())+"%");
    if(d->m_pProgressLabel)
        d->m_pProgressLabel->setText(QString("%1%2/%3%4").arg(progress).arg(d->m_suffix).arg(maximum()).arg(d->m_suffix));
}

void KProgressDialog::setLabelText(const QString &text)
{
    Q_D(KProgressDialog);
    if(d->m_pMainLabel)
        d->m_pMainLabel->setText(text);
}

void KProgressDialog::setCancelButtonText(const QString &text)
{
    Q_D(KProgressDialog);
    if(d->m_pCanelButton)
        d->m_pCanelButton->setText(text);
}

void KProgressDialog::setSubContent(const QString &text)
{
    Q_D(KProgressDialog);
    if(d->m_pSubContentLabel)
    {
        d->m_pSubContentLabel->setText(text);
        setShowDetail(true);
    }

}

void KProgressDialog::changeTheme()
{
    Q_D(KProgressDialog);
    KDialog::changeTheme();
    QFont font;
    font.setPixelSize(ThemeController::systemFontSize() * 1.7);
    d->m_pMainLabel->setFont(font);
    font.setPixelSize(ThemeController::systemFontSize()  * 1.2);
    d->m_pPercentLabel->setFont(font);
    d->m_pSubContentLabel->setFont(font);
    d->m_pProgressLabel->setFont(font);
}

KProgressDialogPrivate::KProgressDialogPrivate(KProgressDialog *parent)
    :q_ptr(parent)
{
    setParent(parent);
}

void KProgressDialogPrivate::adjustNormalMode()
{
    Q_Q(KProgressDialog);

    QHBoxLayout* hLayout = new QHBoxLayout;
    hLayout->setSpacing(0);
    hLayout->addWidget(m_pSubContentLabel);
    hLayout->addStretch();
    hLayout->addWidget(m_pPercentLabel);
    hLayout->addSpacing(15);
    hLayout->addWidget(m_pProgressLabel);

    m_pMainLayout->addStretch();
    m_pMainLayout->addWidget(m_pMainLabel);
    m_pMainLayout->addSpacing(10);
    m_pMainLayout->addWidget(m_pProgressBar);
    m_pMainLayout->addLayout(hLayout);

    hLayout = new QHBoxLayout;
    hLayout->addStretch();
    hLayout->addWidget(m_pCanelButton);
    m_pMainLayout->addSpacing(30);
    m_pMainLayout->addLayout(hLayout);
    q->mainWidget()->setLayout(m_pMainLayout);
}

void KProgressDialogPrivate::infoReset()
{
    Q_Q(KProgressDialog);

    if(m_pPercentLabel)
        m_pPercentLabel->setText(+"0%");
    if(m_pProgressLabel)
        m_pProgressLabel->setText(QString("%1%2/%3%4").arg(0).arg(m_suffix).arg(q->maximum()).arg(m_suffix));
}
}
#include "kprogressdialog.moc"
#include "moc_kprogressdialog.cpp"
