// SPDX-FileCopyrightText: 2023 - 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "cooperationdialog.h"

#include <QHBoxLayout>
#include <QCloseEvent>
#include <QMovie>

#include "gui/utils/cooperationguihelper.h"

using namespace cooperation_core;

ConfirmWidget::ConfirmWidget(QWidget *parent)
    : QWidget(parent)
{
    init();
}

void ConfirmWidget::setDeviceName(const QString &name)
{
    static QString msg(tr("\"%1\" send some files to you"));
    msgLabel->setText(msg.arg(name));
}

void ConfirmWidget::init()
{
    msgLabel = new CooperationLabel(this);
    msgLabel->setWordWrap(true);
    rejectBtn = new QPushButton(tr("Reject", "button"), this);
    acceptBtn = new QPushButton(tr("Accept", "button"), this);

    connect(rejectBtn, &QPushButton::clicked, this, &ConfirmWidget::rejected);
    connect(acceptBtn, &QPushButton::clicked, this, &ConfirmWidget::accepted);

    QHBoxLayout *btnLayout = new QHBoxLayout;
    btnLayout->addWidget(rejectBtn);
    btnLayout->addWidget(acceptBtn);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(msgLabel, 1, Qt::AlignCenter);
    mainLayout->addLayout(btnLayout, 1);
}

WaitConfirmWidget::WaitConfirmWidget(QWidget *parent)
    : QWidget(parent)
{
    init();
}

void WaitConfirmWidget::startMovie()
{
#ifdef linux
    spinner->start();
#else
    spinner->movie()->start();
#endif
}

void WaitConfirmWidget::init()
{
    QVBoxLayout *vLayout = new QVBoxLayout(this);

    spinner = new CooperationSpinner(this);
    spinner->setFixedSize(48, 48);
    spinner->setAttribute(Qt::WA_TransparentForMouseEvents);
    spinner->setFocusPolicy(Qt::NoFocus);
#ifndef linux
    QLabel *titleLabel = new QLabel(tr("File Transfer"), this);
    QFont font;
    font.setWeight(QFont::DemiBold);
    font.setPixelSize(18);
    titleLabel->setFont(font);
    titleLabel->setAlignment(Qt::AlignHCenter);

    vLayout->addSpacing(30);
    vLayout->addWidget(titleLabel, 0, Qt::AlignHCenter);
    vLayout->addSpacing(50);

    QMovie *movie = new QMovie(":/icons/lingmo/builtin/icons/spinner.gif");
    spinner->setMovie(movie);
    movie->setSpeed(180);
#endif

    CooperationLabel *label = new CooperationLabel(tr("Wait for confirmation..."), this);
    label->setAlignment(Qt::AlignHCenter);
    vLayout->addSpacing(20);
    vLayout->addWidget(spinner, 0, Qt::AlignHCenter);
    vLayout->addSpacing(15);
    vLayout->addWidget(label, 0, Qt::AlignHCenter);
    vLayout->addStretch();
}

ProgressWidget::ProgressWidget(QWidget *parent)
    : QWidget(parent)
{
    init();
}

void ProgressWidget::setTitle(const QString &title)
{
    titleLabel->setText(title);
}

void ProgressWidget::setProgress(int value, const QString &msg)
{
    progressBar->setValue(value);
    QString remainTimeMsg(tr("Remaining time %1 | %2%").arg(msg, QString::number(value)));
    msgLabel->setText(remainTimeMsg);
}

void ProgressWidget::init()
{
    titleLabel = new CooperationLabel(this);
    msgLabel = new CooperationLabel(this);

    CooperationGuiHelper::setAutoFont(msgLabel, 12, QFont::Normal);
    // QColor color(0, 0, 0, 180);
    // CooperationGuiHelper::setFontColor(msgLabel, color);

    progressBar = new QProgressBar(this);
    progressBar->setRange(0, 100);
    progressBar->setTextVisible(false);
    progressBar->setFixedHeight(8);

    cancelBtn = new QPushButton(tr("Cancel", "button"), this);
    connect(cancelBtn, &QPushButton::clicked, this, &ProgressWidget::canceled);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    mainLayout->addWidget(titleLabel, 1, Qt::AlignHCenter);
    mainLayout->addWidget(progressBar, 1);
    mainLayout->addWidget(msgLabel, 1, Qt::AlignHCenter);
    mainLayout->addWidget(cancelBtn, 1);
}

ResultWidget::ResultWidget(QWidget *parent)
    : QWidget(parent)
{
    init();
}

void ResultWidget::setResult(bool success, const QString &msg, bool view)
{
    res = success;
    if (success) {
        QIcon icon(":/icons/lingmo/builtin/icons/transfer_success_128px.svg");
        iconLabel->setPixmap(icon.pixmap(48, 48));
#ifndef __linux__
        viewBtn->setVisible(view);
#endif
    } else {
        QIcon icon(":/icons/lingmo/builtin/icons/transfer_fail_128px.svg");
        iconLabel->setPixmap(icon.pixmap(48, 48));
        viewBtn->setVisible(false);
    }

    msgLabel->setText(msg);
}

void ResultWidget::init()
{
    iconLabel = new CooperationLabel(this);
    msgLabel = new CooperationLabel(this);
    msgLabel->setWordWrap(true);
    msgLabel->setAlignment(Qt::AlignHCenter);

    okBtn = new QPushButton(tr("Ok", "button"), this);
    connect(okBtn, &QPushButton::clicked, this, &ResultWidget::completed);

    viewBtn = new QPushButton(tr("View", "button"), this);
    viewBtn->setVisible(false);
    connect(viewBtn, &QPushButton::clicked, this, &ResultWidget::viewed);

    QHBoxLayout *btnLayout = new QHBoxLayout;
    btnLayout->addWidget(okBtn);
    btnLayout->addWidget(viewBtn);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 5, 0, 0);
    mainLayout->addWidget(iconLabel, 0, Qt::AlignHCenter);
    mainLayout->addWidget(msgLabel);
    mainLayout->addLayout(btnLayout);
}

bool ResultWidget::getResult() const
{
    return res;
}

CooperationTransDialog::CooperationTransDialog(QWidget *parent)
    : CooperationDialog(parent)
{
    init();
}

void CooperationTransDialog::showConfirmDialog(const QString &name)
{
    confirmWidget->setDeviceName(name);
    mainLayout->setCurrentWidget(confirmWidget);
    show();
}

void CooperationTransDialog::showWaitConfirmDialog()
{
    waitconfirmWidget->startMovie();
    mainLayout->setCurrentWidget(waitconfirmWidget);
    show();
}

void CooperationTransDialog::showResultDialog(bool success, const QString &msg, bool view)
{
    mainLayout->setCurrentWidget(resultWidget);
    resultWidget->setResult(success, msg, view);
    show();
}

void CooperationTransDialog::showProgressDialog(const QString &title)
{
    if (mainLayout->currentWidget() == progressWidget)
        return;

    progressWidget->setTitle(title);
    mainLayout->setCurrentWidget(progressWidget);
    show();
}

void CooperationTransDialog::updateProgress(int value, const QString &msg)
{
    progressWidget->setProgress(value, msg);
}

void CooperationTransDialog::closeEvent(QCloseEvent *e)
{
    if (!isVisible())
        e->accept();

    if (mainLayout->currentWidget() == confirmWidget) {
        Q_EMIT rejected();
    } else if (mainLayout->currentWidget() == progressWidget) {
        Q_EMIT cancel();
    } else if (mainLayout->currentWidget() == waitconfirmWidget) {
        Q_EMIT cancelApply();
    } else if (mainLayout->currentWidget() == resultWidget) {
        if (qApp->property("onlyTransfer").toBool() && resultWidget->getResult())
            qApp->exit();
    }
}

void CooperationTransDialog::init()
{
#ifdef linux
    setIcon(QIcon::fromTheme("ocean-cooperation"));
    setTitle(tr("File Transfer"));
#else
    setWindowTitle(tr("File transfer"));
    setWindowIcon(QIcon(":/icons/lingmo/builtin/icons/ocean-cooperation_128px.svg"));
#endif
    // it has to set fixed size, which may crash on treeland and workaround UI not smooth.
    setFixedSize(380, 220);

    confirmWidget = new ConfirmWidget(this);
    connect(confirmWidget, &ConfirmWidget::rejected, this, &CooperationTransDialog::rejected);
    connect(confirmWidget, &ConfirmWidget::rejected, this, &CooperationTransDialog::close);
    connect(confirmWidget, &ConfirmWidget::accepted, this, &CooperationTransDialog::accepted);

    progressWidget = new ProgressWidget(this);
    connect(progressWidget, &ProgressWidget::canceled, this, &CooperationTransDialog::cancel);

    waitconfirmWidget = new WaitConfirmWidget(this);
    connect(waitconfirmWidget, &WaitConfirmWidget::canceled, this, &CooperationTransDialog::cancel);

    resultWidget = new ResultWidget(this);
    connect(resultWidget, &ResultWidget::viewed, this, &CooperationTransDialog::viewed);
    connect(resultWidget, &ResultWidget::completed, this, &CooperationTransDialog::close);

    mainLayout = new QStackedLayout;
    mainLayout->addWidget(confirmWidget);
    mainLayout->addWidget(waitconfirmWidget);
    mainLayout->addWidget(progressWidget);
    mainLayout->addWidget(resultWidget);

#ifdef linux
    QWidget *contentWidget = new QWidget(this);
    contentWidget->setLayout(mainLayout);
    addContent(contentWidget);
    setContentsMargins(0, 0, 0, 0);
#else
    setLayout(mainLayout);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
#endif
}
