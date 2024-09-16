// SPDX-FileCopyrightText: 2015 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dmemorywarningdialog.h"
#include "processinfotable.h"
#include "processinfomodel.h"
#include "processinfodelegate.h"
#include "buttondelegate.h"

#include <QKeyEvent>
#include <QApplication>
#include <QScreen>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QDebug>

#include <ddialog.h>
#include <DTitlebar>

DWIDGET_USE_NAMESPACE

static DMemoryWarningDialog *dialog = nullptr;

bool confirm(const QPixmap &icon)
{
    const QStringList btns = QStringList() << QApplication::translate("DMemoryWarningDialog", "Cancel")
//                                           << QApplication::translate("DMemoryWarningDialog", "View")
                                           << QApplication::translate("DMemoryWarningDialog", "Release");

    DDialog terminateDialog(dialog);
    terminateDialog.setAccessibleName("TerminateDialog");
    terminateDialog.setTitle(QApplication::translate("DMemoryWarningDialog", "This application will be ended, please make sure your data has been saved!"));
    terminateDialog.setMessage(QApplication::translate("DMemoryWarningDialog", "Please save your document, text and spreadsheet"));
    terminateDialog.setIcon(icon);
    terminateDialog.addButtons(btns);

    const int r = terminateDialog.exec();
    if (r == 0)
        return false;

    if (r == 1)
        return true;

    return false;
}

DMemoryWarningDialog::DMemoryWarningDialog(QWidget *parent)
    : DAbstractDialog(parent)
    // TODO
    , m_startManagerInter(new StartManagerInter("org.deepin.dde.StartManager1", "/org/deepin/dde/StartManager1", QDBusConnection::sessionBus(), this))
    , m_infoModel(new ProcessInfoModel)
    , m_icon(new QLabel)
    , m_memNeeded(new QLabel)
    , m_cancelButton(new QPushButton)
    , m_continueButton(new QPushButton)
{
    setWindowTitle(" ");

    dialog = this;

    m_memNeeded->setWordWrap(true);
    m_memNeeded->setAccessibleName("MemNeededLabel");
    m_cancelButton->setText(tr("Cancel"));
    m_continueButton->setText(tr("Continue"));
    m_icon->setAccessibleName("DdeIconLabel");
    m_icon->setPixmap(QIcon::fromTheme("dde").pixmap(32, 32));

    ProcessInfoView *table = new ProcessInfoView;
    table->setAccessibleName("ProessInfoView");
    table->setModel(m_infoModel);
    table->setItemDelegate(new ProcessInfoDelegate(table));
    table->setItemDelegateForColumn(3, new ButtonDelegate(table));
    table->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QHBoxLayout *btnsLayout = new QHBoxLayout;
    btnsLayout->addWidget(m_cancelButton);
    btnsLayout->addWidget(m_continueButton);

    QLabel *icon = new QLabel;
    icon->setAccessibleName("MessageboxWarningIcon");
    icon->setAlignment(Qt::AlignCenter);
    icon->setPixmap(QIcon::fromTheme("messagebox_warning").pixmap(64, 64));

    QLabel *label = new QLabel;
    label->setText(tr("Insufficient system memory, please end some applications to avoid getting stuck."));
    label->setAlignment(Qt::AlignCenter);
    label->setWordWrap(true);
    QFont f = label->font();
    f.setPointSize(12);
    label->setFont(f);

    QHBoxLayout *tipsLayout = new QHBoxLayout;
    tipsLayout->addStretch();
    tipsLayout->addWidget(m_icon);
    tipsLayout->addWidget(m_memNeeded);
    tipsLayout->addStretch();

    QVBoxLayout *vLayout = new QVBoxLayout;
    vLayout->addWidget(icon);
    vLayout->addWidget(label);
    vLayout->addWidget(table);
    vLayout->addSpacing(20);
    vLayout->addLayout(tipsLayout);
    vLayout->addSpacing(20);
    vLayout->addLayout(btnsLayout);

    setLayout(vLayout);
    setFixedSize(400, 600);
    setWindowFlags(Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint | Qt::Dialog);

    connect(m_cancelButton, &QPushButton::clicked, this, &DMemoryWarningDialog::onCancelClicked);
    connect(m_continueButton, &QPushButton::clicked, this, &DMemoryWarningDialog::onContinueClicked);
    connect(m_startManagerInter, &StartManagerInter::NeededMemoryChanged, this, &DMemoryWarningDialog::onNeedMemChanged);
    onNeedMemChanged(m_startManagerInter->neededMemory());
}

DMemoryWarningDialog::~DMemoryWarningDialog()
{
    m_startManagerInter->TryAgain(false);
}

void DMemoryWarningDialog::updateAppInfo(const QString &appInfo)
{
    if (appInfo.endsWith(".desktop"))
    {
        m_icon->setPixmap(appIcon(32, appInfo));
        m_appName = genericAppName(appInfo);
        m_tipsType = LaunchApp;
    } else if (appInfo.startsWith("tabslimit-open-url")) {
        m_icon->setPixmap(QIcon::fromTheme("google-chrome").pixmap(32, 32));
        m_tipsType = OpenChromeTab;
    } else {
        m_icon->setPixmap(QIcon::fromTheme("terminal").pixmap(32, 32));
        m_tipsType = ExecuteCommand;
    }

    updateTips();
}

void DMemoryWarningDialog::keyPressEvent(QKeyEvent *e)
{
    switch (e->key())
    {
#ifdef QT_DEBUG
    case Qt::Key_Escape:    qApp->quit();   break;
#endif
    default:;
    }
}

void DMemoryWarningDialog::showEvent(QShowEvent *e)
{
    DAbstractDialog::showEvent(e);

    move(qApp->primaryScreen()->geometry().center() - rect().center());

    QTimer::singleShot(1, this, &DMemoryWarningDialog::raise);
    QTimer::singleShot(1, this, &DMemoryWarningDialog::activateWindow);

    m_infoModel->startRefreshData();
}

void DMemoryWarningDialog::hideEvent(QHideEvent *e)
{
    DAbstractDialog::hideEvent(e);

    m_infoModel->stopRefreshData();
}

void DMemoryWarningDialog::onNeedMemChanged(const qulonglong needed)
{
    m_continueButton->setEnabled(!needed);
    m_needed = needed;
    updateTips();
}

void DMemoryWarningDialog::onContinueClicked()
{
    m_startManagerInter->TryAgain(true);
    hide();
}

void DMemoryWarningDialog::onCancelClicked()
{
    m_startManagerInter->TryAgain(false);
    hide();
}

void DMemoryWarningDialog::updateTips()
{
    if (m_needed)
    {
        const int m_bytes = std::max(10, m_needed / 1024);

        switch (m_tipsType) {
        case LaunchApp:
            m_memNeeded->setText(tr("Continue to run %1, %2MB memory is required").arg(m_appName).arg(m_bytes));
            break;
        case OpenChromeTab:
            m_memNeeded->setText(tr("Continue to open browser tab, %1MB memory is required").arg(m_bytes));
            break;
        case ExecuteCommand:
            m_memNeeded->setText(tr("To continue the operation, %1MB memory is required").arg(m_bytes));
            break;
        }
    } else {
        switch (m_tipsType) {
        case LaunchApp:
            m_memNeeded->setText(tr("Your current memory is sufficient enough, continue to run %1?").arg(m_appName));
            break;
        case OpenChromeTab:
            m_memNeeded->setText(tr("Your current memory is sufficient enough, continue to open browser tab?"));
            break;
        case ExecuteCommand:
            m_memNeeded->setText(tr("Your current memory is sufficient enough, continue the operation?"));
            break;
        }
    }
}
