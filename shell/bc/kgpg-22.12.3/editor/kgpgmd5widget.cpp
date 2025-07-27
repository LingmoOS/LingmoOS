/*

    SPDX-License-Identifier: GPL-2.0-or-later

*/

#include "kgpgmd5widget.h"

#include <QApplication>
#include <QCryptographicHash>
#include <QFile>
#include <QClipboard>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>

#include <KMessageBox>
#include <KLocalizedString>
#include <KLed>
#include <KConfigGroup>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QVBoxLayout>


Md5Widget::Md5Widget(QWidget *parent, const QUrl &url)
         : QDialog(parent)
{
    setWindowTitle(i18n("MD5 Checksum"));
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Close|QDialogButtonBox::Apply);
    QWidget *mainWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    setLayout(mainLayout);
    mainLayout->addWidget(mainWidget);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &Md5Widget::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &Md5Widget::reject);
    buttonBox->button(QDialogButtonBox::Close)->setDefault(true);
    buttonBox->button(QDialogButtonBox::Apply)->setText(i18n("Compare MD5 with Clipboard"));

    QFile f(url.path());
    QCryptographicHash checkfile(QCryptographicHash::Md5);

    if (f.open(QIODevice::ReadOnly)) {
	checkfile.addData(&f);
	f.close();
    }

    m_md5sum = QLatin1String( checkfile.result() );

    QWidget *page = new QWidget(this);

    QLabel *firstlabel = new QLabel(page);
    firstlabel->setText(i18n("MD5 sum for <b>%1</b> is:", url.fileName()));

    QLineEdit *md5lineedit = new QLineEdit(m_md5sum, page);
    md5lineedit->setReadOnly(true);

    m_led = new KLed(QColor(80, 80, 80), KLed::Off, KLed::Sunken, KLed::Circular, page);
    QSizePolicy policy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    policy.setVerticalStretch(0);
    policy.setHorizontalStretch(0);
    policy.setHeightForWidth(m_led->sizePolicy().hasHeightForWidth());
    m_led->setSizePolicy(policy);

    m_label = new QLabel(page);
    m_label->setText(i18n("<b>Unknown status</b>"));

    QHBoxLayout *ledlayout = new QHBoxLayout();
    ledlayout->addWidget(m_led);
    ledlayout->addWidget(m_label);

    QVBoxLayout *dialoglayout = new QVBoxLayout(page);
    dialoglayout->addWidget(firstlabel);
    dialoglayout->addWidget(md5lineedit);
    dialoglayout->addLayout(ledlayout);
    dialoglayout->addStretch();

    mainLayout->addWidget(page);
    mainLayout->addWidget(buttonBox);

    connect(buttonBox->button(QDialogButtonBox::Apply), &QPushButton::clicked, this, &Md5Widget::slotApply);
}

void Md5Widget::slotApply()
{
    QString text = qApp->clipboard()->text().remove(QLatin1Char( ' ' ));
    if (!text.isEmpty())
    {
        if (text.length() != m_md5sum.length())
            KMessageBox::error(this, i18n("Clipboard content is not a MD5 sum."));
        else
        if (text == m_md5sum)
        {
            m_label->setText(i18n("<b>Correct checksum</b>, file is ok."));
            m_led->setColor(QColor(Qt::green));
            m_led->on();
        }
        else
        {
            m_label->setText(i18n("<b>Wrong checksum, <em>file corrupted</em></b>"));
            m_led->setColor(QColor(Qt::red));
            m_led->on();
        }
    }
}
