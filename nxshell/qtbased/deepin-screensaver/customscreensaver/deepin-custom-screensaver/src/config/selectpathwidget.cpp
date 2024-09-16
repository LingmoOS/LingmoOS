// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "selectpathwidget.h"
#include "truncatelineedit.h"

#include <DFileDialog>

#include <QHBoxLayout>
#include <QEvent>

DWIDGET_USE_NAMESPACE
DCORE_USE_NAMESPACE

SelectPathWidget::SelectPathWidget(QWidget *parent)
    : DWidget(parent)
{
    initOption();
}

void SelectPathWidget::setPath(const QString &path)
{
    m_selectLineEdit->setText(path);
}

QString SelectPathWidget::getPath()
{
    return m_selectLineEdit->getText();
}

void SelectPathWidget::initOption()
{
    QString valueText;
    // 路径转换
    if (valueText.startsWith(QStringLiteral("~")))
        valueText.replace(QStringLiteral("~"), QDir::homePath());

    m_selectTips = new DLabel(tr("Picture path"), this);
    m_selectLineEdit = new TruncateLineEdit(this);
    m_selectBtn = new DPushButton(QStringLiteral("..."), this);
    m_selectBtn->setFocusPolicy(Qt::NoFocus);

    m_selectLineEdit->setElideMode(Qt::ElideMiddle);
    m_selectLineEdit->setText(valueText);
    m_selectLineEdit->setPlaceholderText(tr("Select a path of pictures"));
    m_selectLineEdit->setClearButtonEnabled(false);
    m_selectLineEdit->lineEdit()->installEventFilter(this);

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->addWidget(m_selectTips);
    layout->addWidget(m_selectLineEdit);
    layout->addWidget(m_selectBtn);

    connect(m_selectBtn, &DPushButton::clicked, this, &SelectPathWidget::requsetSetPath);
}

bool SelectPathWidget::eventFilter(QObject *obj, QEvent *event)
{
    if (m_selectLineEdit && obj == m_selectLineEdit->lineEdit()) {
        if (event->type() == QEvent::MouseButtonPress
            || event->type() == QEvent::MouseButtonRelease
            || event->type() == QEvent::MouseButtonDblClick
            || event->type() == QEvent::MouseMove)
            return true;
    }

    return QObject::eventFilter(obj, event);
}
