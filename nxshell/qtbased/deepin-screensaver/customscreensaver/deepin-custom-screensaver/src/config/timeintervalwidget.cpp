// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "timeintervalwidget.h"
#include "src/slideshowconfig.h"

#include <QHBoxLayout>

DWIDGET_USE_NAMESPACE
DCORE_USE_NAMESPACE

#define SPECIFYLINEEDITLENGTH 65

TimeIntervalWidget::TimeIntervalWidget()
{
    initOption();
}

void TimeIntervalWidget::setInterval(int time)
{
    if (time < kMinIntervalTime || time > kMaxIntervalTime)
        time = kDefaultTime;

    m_lineEdit->setText(QString::number(time));
}

int TimeIntervalWidget::interval()
{
    auto intervalTime = m_lineEdit->text().toInt();
    if (intervalTime < kMinIntervalTime || intervalTime > kMaxIntervalTime)
        intervalTime = kDefaultTime;

    return intervalTime;
}

void TimeIntervalWidget::initOption()
{
    m_lineEdit = new DLineEdit(this);
    m_lineEdit->setClearButtonEnabled(false);
    m_lineEdit->setFixedWidth(SPECIFYLINEEDITLENGTH);
    m_lineEdit->lineEdit()->setValidator(new QIntValidator(m_lineEdit));

    m_prefixTips = new DLabel(tr("Time interval (s)"), this);
    m_suffixTips = new DLabel(QString("%1: %2-%3")
                                      .arg(tr("Range"))
                                      .arg(QString::number(kMinIntervalTime))
                                      .arg(QString::number(kMaxIntervalTime)),
                              this);

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->addWidget(m_prefixTips);
    layout->addWidget(m_lineEdit);
    layout->addWidget(m_suffixTips);
    layout->addStretch(3);

    connect(m_lineEdit, &DLineEdit::editingFinished, this, &TimeIntervalWidget::onEditingFinished);
}

void TimeIntervalWidget::onEditingFinished()
{
    auto intervalTime = m_lineEdit->text().toInt();
    if (intervalTime < kMinIntervalTime || intervalTime > kMaxIntervalTime) {
        intervalTime = kDefaultTime;
        m_lineEdit->setText(QString::number(intervalTime));
    }

    emit valueChanged(intervalTime);
    m_lineEdit->clearFocus();
}
