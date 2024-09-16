// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mainwindow.h"
#include "mainwindow_p.h"
#include "dialogs/settingdialog.h"
#include "utils/cooperationutil.h"

#include <QScreen>
#include <QUrl>
#include <QApplication>
#include <QDesktopServices>
#include <QCheckBox>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QVBoxLayout>

using namespace cooperation_core;

#ifdef __linux__
DWIDGET_USE_NAMESPACE
const char *Kicon = "dde-cooperation";
#else
const char *Kicon = ":/icons/deepin/builtin/icons/dde-cooperation_128px.svg";
#endif

MainWindowPrivate::MainWindowPrivate(MainWindow *qq)
    : q(qq)
{
}

MainWindowPrivate::~MainWindowPrivate()
{
}

void MainWindowPrivate::initConnect()
{
    connect(workspaceWidget, &WorkspaceWidget::search, q, &MainWindow::onFindDevice);
}

void MainWindowPrivate::moveCenter()
{
    QScreen *cursorScreen = nullptr;
    const QPoint &cursorPos = QCursor::pos();

    QList<QScreen *> screens = qApp->screens();
    QList<QScreen *>::const_iterator it = screens.begin();
    for (; it != screens.end(); ++it) {
        if ((*it)->geometry().contains(cursorPos)) {
            cursorScreen = *it;
            break;
        }
    }

    if (!cursorScreen)
        cursorScreen = qApp->primaryScreen();
    if (!cursorScreen)
        return;

    int x = (cursorScreen->availableGeometry().width() - q->width()) / 2;
    int y = (cursorScreen->availableGeometry().height() - q->height()) / 2;
    q->move(QPoint(x, y) + cursorScreen->geometry().topLeft());
}

void MainWindowPrivate::handleSettingMenuTriggered(int action)
{
    switch (static_cast<MenuAction>(action)) {
    case MenuAction::kSettings: {
        if (q->property("SettingDialogShown").toBool()) {
            return;
        }

        SettingDialog *dialog = new SettingDialog(q);
        dialog->show();
        dialog->setAttribute(Qt::WA_DeleteOnClose);
        q->setProperty("SettingDialogShown", true);
        QObject::connect(dialog, &SettingDialog::finished, [=] {
            q->setProperty("SettingDialogShown", false);
        });
    } break;
    case MenuAction::kDownloadWindowClient:
        QDesktopServices::openUrl(QUrl("https://www.chinauos.com/resource/assistant"));
        break;
    }
}

MainWindow::MainWindow(QWidget *parent)
    : CooperationMainWindow(parent),
      d(new MainWindowPrivate(this))
{
    d->initWindow();
    d->initTitleBar();
    d->moveCenter();
    d->initConnect();
}

MainWindow::~MainWindow()
{
}

// DeviceInfoPointer MainWindow::findDeviceInfo(const QString &ip)
// {
//     return d->workspaceWidget->findDeviceInfo(ip);
// }

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (qApp->property("onlyTransfer").toBool())
        QApplication::quit();

    showCloseDialog();
    event->ignore();
}

void MainWindow::onlineStateChanged(const QString &validIP)
{
    bool offline = validIP.isEmpty();
    if (offline) {
        d->workspaceWidget->clear();
        d->workspaceWidget->switchWidget(WorkspaceWidget::kNoNetworkWidget);
        d->workspaceWidget->setBottomIp("---");
    } else {
        d->workspaceWidget->setBottomIp(validIP);
    }
}

void MainWindow::setFirstTipVisible()
{
    d->workspaceWidget->setFirstStartTip(true);
}

void MainWindow::onLookingForDevices()
{
    _userAction = true;
    emit refreshDevices();
    d->workspaceWidget->clear();
    d->workspaceWidget->switchWidget(WorkspaceWidget::kLookignForDeviceWidget);
}

void MainWindow::onFindDevice(const QString &ip)
{
    _userAction = true;
    emit searchDevice(ip);
}

void MainWindow::onDiscoveryFinished(bool hasFound)
{
    if (!hasFound && _userAction) {
        d->workspaceWidget->switchWidget(WorkspaceWidget::kNoResultWidget);
    }

    _userAction = false;
}

void MainWindow::addDevice(const QList<DeviceInfoPointer> &infoList)
{
    d->workspaceWidget->switchWidget(WorkspaceWidget::kDeviceListWidget);
    d->workspaceWidget->addDeviceInfos(infoList);

    _userAction = false;
}

void MainWindow::removeDevice(const QString &ip)
{
    d->workspaceWidget->removeDeviceInfos(ip);
}

void MainWindow::onRegistOperations(const QVariantMap &map)
{
    d->workspaceWidget->addDeviceOperation(map);
}

#if defined(_WIN32) || defined(_WIN64)
void MainWindow::paintEvent(QPaintEvent *event)
{
    d->paintEvent(event);
}

void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    d->mouseMoveEvent(event);
}

void MainWindow::mouseReleaseEvent(QMouseEvent *event)
{
    d->mousePressEvent(event);
}

void MainWindow::mousePressEvent(QMouseEvent *event)
{
    d->mousePressEvent(event);
}
#endif

void MainWindow::showCloseDialog()
{
    QString option = CooperationUtil::closeOption();
    if (option == "Minimise") {
        minimizedAPP();
        return;
    }
    if (option == "Exit")
        QApplication::quit();

    CooperationDialog dlg;

    QVBoxLayout *layout = new QVBoxLayout();
    QCheckBox *op1 = new QCheckBox(tr("Minimise to system tray"));
    op1->setChecked(true);
    QCheckBox *op2 = new QCheckBox(tr("Exit"));

    connect(op1, &QCheckBox::stateChanged, op1, [op2](int state) {
        op2->setChecked(state != Qt::Checked);
    });
    connect(op2, &QCheckBox::stateChanged, op2, [op1](int state) {
        op1->setChecked(state != Qt::Checked);
    });

    QCheckBox *op3 = new QCheckBox(tr("No more enquiries"));

    layout->addWidget(op1);
    layout->addWidget(op2);
    layout->addWidget(op3);

#ifdef __linux__
    dlg.setIcon(QIcon::fromTheme("dde-cooperation"));
    dlg.addButton(tr("Cancel"));
    dlg.addButton(tr("Confirm"), true, DDialog::ButtonWarning);
    dlg.setTitle(tr("Please select your operation"));
    QWidget *content = new QWidget();

    content->setLayout(layout);
    dlg.addContent(content);
#else
    dlg.setWindowIcon(QIcon::fromTheme(Kicon));
    dlg.setWindowTitle(tr("Please select your operation"));
    QPushButton *okButton = new QPushButton(tr("Confirm"));
    QPushButton *cancelButton = new QPushButton(tr("Cancel"));

    QObject::connect(okButton, &QPushButton::clicked, &dlg, &QDialog::accept);
    QObject::connect(cancelButton, &QPushButton::clicked, &dlg, &QDialog::reject);

    layout->addWidget(okButton);
    layout->addWidget(cancelButton);
    dlg.setLayout(layout);
    dlg.setFixedSize(400, 200);
#endif

    int code = dlg.exec();
    if (code == QDialog::Accepted) {
        bool isExit = op2->checkState() == Qt::Checked;
        if (op3->checkState() == Qt::Checked) {
            CooperationUtil::saveOption(isExit);
        }

        if (isExit)
            QApplication::quit();
        else
            minimizedAPP();
    }
}

void MainWindow::minimizedAPP()
{
    this->hide();
    if (d->trayIcon)
        return;
    d->trayIcon = new QSystemTrayIcon(QIcon::fromTheme(Kicon), this);

    QMenu *trayMenu = new QMenu(this);
    QAction *restoreAction = trayMenu->addAction(tr("Restore"));
    QAction *quitAction = trayMenu->addAction(tr("Quit"));

    d->trayIcon->setContextMenu(trayMenu);
    d->trayIcon->show();

    QObject::connect(restoreAction, &QAction::triggered, this, &QMainWindow::show);
    QObject::connect(quitAction, &QAction::triggered, qApp, &QApplication::quit);
    QObject::connect(d->trayIcon, &QSystemTrayIcon::activated, this, [this](QSystemTrayIcon::ActivationReason reason) {
        if (reason == QSystemTrayIcon::Trigger)
            this->show();
    });
}
