// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-only


#include "animationdialog.h"

#include <DWindowCloseButton>

#include <QVBoxLayout>
#include <QKeyEvent>

AnimationDialog::AnimationDialog(QWidget *parent) : DDialog(parent)
{
    initUi();
}

void AnimationDialog::initUi()
{
    setFixedSize(380, 160);
    setWindowFlags(windowFlags() & ~Qt::WindowCloseButtonHint);

    DFrame *m_mainFrame = new DFrame(this);
    m_mainFrame->setFrameRounded(false);
    m_mainFrame->setFrameStyle(DFrame::NoFrame);
    addContent(m_mainFrame);

    m_label = new DLabel(this);
    m_label->setAccessibleName("animationTitle");
    QVBoxLayout *mainLayout = new QVBoxLayout(m_mainFrame);
    m_spinner = new DSpinner(m_mainFrame);
    m_spinner->setFixedSize(28, 28);
    m_spinner->start();
    m_spinner->hide();

    mainLayout->addWidget(m_label, 0, Qt::AlignCenter | Qt::AlignTop);
    mainLayout->addSpacing(15);
    mainLayout->addWidget(m_spinner, 0, Qt::AlignCenter);
    mainLayout->addStretch();
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);
}

void AnimationDialog::setShowSpinner(bool isShow, const QString &title)
{
    if (isShow) {
        m_label->setText(title);
        m_spinner->show();
    } else {
        m_spinner->hide();
    }
}

void AnimationDialog::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key::Key_Escape) {
        event->ignore();
    } else {
        DDialog::keyPressEvent(event);
    }
}






