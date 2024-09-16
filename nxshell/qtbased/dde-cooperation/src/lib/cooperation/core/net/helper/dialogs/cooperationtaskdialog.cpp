// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "cooperationtaskdialog.h"
#include "common/commonutils.h"

#include <QVBoxLayout>

using namespace cooperation_core;
using namespace deepin_cross;

CooperationTaskDialog::CooperationTaskDialog(QWidget *parent)
    : CooperationDialog(parent)
{
    init();
}

void CooperationTaskDialog::switchWaitPage(const QString &title)
{
    setTaskTitle(title);
    switchLayout->setCurrentIndex(0);
}

void CooperationTaskDialog::switchFailPage(const QString &title, const QString &msg, bool retry)
{
    setTaskTitle(title);

    failMsgLabel->setText(msg);
    retryBtn->setVisible(retry);
    switchLayout->setCurrentIndex(1);
}

void CooperationTaskDialog::switchConfirmPage(const QString &title, const QString &msg)
{
    setTaskTitle(title);
    confirmMsgLabel->setText(msg);
    switchLayout->setCurrentIndex(2);
}

void CooperationTaskDialog::switchInfomationPage(const QString &title, const QString &msg)
{
    setTaskTitle(title);
    infoLabel->setText(msg);
    switchLayout->setCurrentIndex(3);
}

void CooperationTaskDialog::init()
{
#ifdef linux
    setIcon(QIcon::fromTheme("dde-cooperation"));
#else
    setWindowIcon(QIcon(":/icons/deepin/builtin/icons/dde-cooperation_128px.svg"));
#endif
    setFixedWidth(380);

    switchLayout = new QStackedLayout;
    switchLayout->addWidget(createWaitPage());
    switchLayout->addWidget(createFailPage());
    switchLayout->addWidget(createConfirmPage());
    switchLayout->addWidget(createInfomationPage());

#ifdef linux
    QWidget *contentWidget = new QWidget(this);
    contentWidget->setLayout(switchLayout);
    addContent(contentWidget);
    setContentsMargins(0, 0, 0, 0);
#else
    setLayout(switchLayout);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
#endif
}

void CooperationTaskDialog::setTaskTitle(const QString &title)
{
#ifdef linux
    setTitle(title);
#else
    setWindowTitle(title);
#endif
}

QWidget *CooperationTaskDialog::createWaitPage()
{
    QWidget *widget = new QWidget(this);
    QVBoxLayout *vlayout = new QVBoxLayout(widget);
    vlayout->setContentsMargins(0, 0, 0, 0);

    CooperationSpinner *spinner = new CooperationSpinner(this);
    spinner->setFixedSize(36, 36);
    spinner->setAttribute(Qt::WA_TransparentForMouseEvents);
    spinner->setFocusPolicy(Qt::NoFocus);
#ifdef linux
    spinner->start();
#endif

    QPushButton *celBtn = new QPushButton(tr("Cancel", "button"), this);
    connect(celBtn, &QPushButton::clicked, this, &CooperationTaskDialog::waitCanceled);

    vlayout->addWidget(spinner, 0, Qt::AlignHCenter);
    vlayout->addWidget(celBtn, 0, Qt::AlignBottom);

    return widget;
}

QWidget *CooperationTaskDialog::createFailPage()
{
    QWidget *widget = new QWidget(this);
    QVBoxLayout *vlayout = new QVBoxLayout(widget);
    vlayout->setContentsMargins(0, 0, 0, 0);

    failMsgLabel = new CooperationLabel(this);
    failMsgLabel->setAlignment(Qt::AlignHCenter);
    failMsgLabel->setWordWrap(true);

    QPushButton *cancelBtn = new QPushButton(tr("Cancel", "button"), this);
    connect(cancelBtn, &QPushButton::clicked, this, &CooperationTaskDialog::close);

    retryBtn = new CooperationSuggestButton(tr("Retry", "button"), this);
    connect(retryBtn, &QPushButton::clicked, this, &CooperationTaskDialog::retryConnected);

    QHBoxLayout *hlayout = new QHBoxLayout;
    hlayout->addWidget(cancelBtn, 0, Qt::AlignBottom);
    hlayout->addWidget(retryBtn, 0, Qt::AlignBottom);

    vlayout->addWidget(failMsgLabel);
    vlayout->addLayout(hlayout);
    return widget;
}

QWidget *CooperationTaskDialog::createConfirmPage()
{
    QWidget *widget = new QWidget(this);
    QVBoxLayout *vlayout = new QVBoxLayout(widget);
    vlayout->setContentsMargins(0, 0, 0, 0);

    confirmMsgLabel = new CooperationLabel(this);
    confirmMsgLabel->setAlignment(Qt::AlignHCenter);
    confirmMsgLabel->setWordWrap(true);

    QPushButton *rejectBtn = new QPushButton(tr("Reject", "button"), this);
    connect(rejectBtn, &QPushButton::clicked, this, &CooperationTaskDialog::rejectRequest);
    connect(rejectBtn, &QPushButton::clicked, this, &CooperationTaskDialog::close);
    QPushButton *acceptBtn = new CooperationSuggestButton(tr("Accept", "button"), this);
    connect(acceptBtn, &QPushButton::clicked, this, &CooperationTaskDialog::acceptRequest);

    QHBoxLayout *hlayout = new QHBoxLayout;
    hlayout->addWidget(rejectBtn, 0, Qt::AlignBottom);
    hlayout->addWidget(acceptBtn, 0, Qt::AlignBottom);

    vlayout->addWidget(confirmMsgLabel);
    vlayout->addLayout(hlayout);
    return widget;
}

QWidget *CooperationTaskDialog::createInfomationPage()
{
    QWidget *widget = new QWidget(this);
    QVBoxLayout *vlayout = new QVBoxLayout(widget);
    vlayout->setContentsMargins(0, 0, 0, 0);

    infoLabel = new CooperationLabel(this);
    infoLabel->setAlignment(Qt::AlignHCenter);
    infoLabel->setWordWrap(true);

    QPushButton *closeBtn = new QPushButton(tr("Close", "button"), this);
    connect(closeBtn, &QPushButton::clicked, this, &CooperationTaskDialog::close);

    vlayout->addWidget(infoLabel);
    vlayout->addWidget(closeBtn, 0, Qt::AlignBottom);
    return widget;
}
