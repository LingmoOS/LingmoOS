#include "../mainwindow.h"
#include "../mainwindow_p.h"
#include "../connect/readywidget.h"
#include "../connect/choosewidget.h"
#include "../connect/connectwidget.h"
#include "../connect/promptwidget.h"
#include "../connect/startwidget.h"
#include "../connect/networkdisconnectionwidget.h"
#include "../connect/custommessagebox.h"

#include "../select/siderbarwidget.h"
#include "../select/appselectwidget.h"
#include "../select/selectmainwidget.h"
#include "../select/configselectwidget.h"
#include "../select/fileselectwidget.h"

#include "../transfer/transferringwidget.h"
#include "../transfer/errorwidget.h"

#include "../getbackup/createbackupfilewidget.h"
#include "../getbackup/zipfileprocesswidget.h"
#include "../getbackup/zipfileprocessresultwidget.h"
#include "net/helper/transferhepler.h"
#include "utils/optionsmanager.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QMenuBar>
#include <QPushButton>
#include <QToolBar>
#include <QDesktopWidget>
#include <QScreen>
#include <QToolButton>
#include <QPainter>
#include <QPainterPath>
#include <QPaintEvent>
#include <QCoreApplication>
#include <QApplication>
#include <QDebug>
#include <QThreadPool>
#include <QTimer>

#include <gui/transfer/resultdisplay.h>

using namespace data_transfer_core;

void MainWindowPrivate::initWindow()
{
    q->setWindowFlags(Qt::FramelessWindowHint);
    q->setAttribute(Qt::WA_TranslucentBackground);
    q->setFixedSize(740, 552);
    QWidget *centerWidget = new QWidget(q);
    QVBoxLayout *layout = new QVBoxLayout();
    centerWidget->setLayout(layout);
    layout->setContentsMargins(0, 0, 0, 0);

    q->setCentralWidget(centerWidget);
    windowsCentralWidgetSidebar = new QHBoxLayout();
    windowsCentralWidgetContent = new QHBoxLayout();
    windowsCentralWidget = new QHBoxLayout();

    windowsCentralWidget->addLayout(windowsCentralWidgetSidebar);
    windowsCentralWidget->addLayout(windowsCentralWidgetContent);

    q->setWindowIcon(QIcon(":/icon/icon.svg"));

    initSideBar();
    initTitleBar();
    layout->setSpacing(0);
    layout->addLayout(windowsCentralWidget);
}

void MainWindowPrivate::initWidgets()
{
    StartWidget *startwidget = new StartWidget(q);
    ChooseWidget *choosewidget = new ChooseWidget(q);
    TransferringWidget *transferringwidget = new TransferringWidget(q);
    ReadyWidget *readywidget = new ReadyWidget(q);
    PromptWidget *promptwidget = new PromptWidget(q);
    FileSelectWidget *filewselectidget =
            new FileSelectWidget(qobject_cast<SidebarWidget *>(sidebar->widget()), q);
    ResultDisplayWidget *resultwidget = new ResultDisplayWidget(q);
    ConfigSelectWidget *configselectwidget = new ConfigSelectWidget(q);
    AppSelectWidget *appselectwidget = new AppSelectWidget(q);
    ErrorWidget *errorwidget = new ErrorWidget(q);

    NetworkDisconnectionWidget *networkdisconnectionwidget = new NetworkDisconnectionWidget(q);
    ZipFileProcessWidget *zipfileprocesswidget = new ZipFileProcessWidget(q);
    ZipFileProcessResultWidget *zipfileprocessresultwidget = new ZipFileProcessResultWidget(q);
    CreateBackupFileWidget *createbackupfilewidget = new CreateBackupFileWidget(q);
    SelectMainWidget *selectmainwidget = new SelectMainWidget(q);

    stackedWidget = new QStackedWidget(q);
    stackedWidget->insertWidget(PageName::startwidget, startwidget);
    stackedWidget->insertWidget(PageName::choosewidget, choosewidget);
    stackedWidget->insertWidget(PageName::promptwidget, promptwidget);
    stackedWidget->insertWidget(PageName::readywidget, readywidget);
    stackedWidget->insertWidget(PageName::selectmainwidget, selectmainwidget);
    stackedWidget->insertWidget(PageName::transferringwidget, transferringwidget);
    stackedWidget->insertWidget(PageName::resultwidget, resultwidget);
    stackedWidget->insertWidget(PageName::filewselectidget, filewselectidget);
    stackedWidget->insertWidget(PageName::configselectwidget, configselectwidget);
    stackedWidget->insertWidget(PageName::appselectwidget, appselectwidget);
    stackedWidget->insertWidget(PageName::errorwidget, errorwidget);
    stackedWidget->insertWidget(PageName::createbackupfilewidget, createbackupfilewidget);
    stackedWidget->insertWidget(PageName::networkdisconnectwidget, networkdisconnectionwidget);
    stackedWidget->insertWidget(PageName::zipfileprocesswidget, zipfileprocesswidget);
    stackedWidget->insertWidget(PageName::zipfileprocessresultwidget, zipfileprocessresultwidget);

    stackedWidget->setCurrentIndex(PageName::startwidget);

    windowsCentralWidgetContent->setContentsMargins(8, 8, 8, 8);
    windowsCentralWidgetContent->addWidget(stackedWidget);

    QObject::connect(stackedWidget, &QStackedWidget::currentChanged, this,
                     &MainWindowPrivate::handleCurrentChanged);

    QObject::connect(TransferHelper::instance(), &TransferHelper::transferFinished, this,
                     [this] { stackedWidget->setCurrentIndex(PageName::resultwidget); });

    QObject::connect(appselectwidget, &AppSelectWidget::isOk, selectmainwidget,
                     &SelectMainWidget::changeSelectframeState);
    QObject::connect(filewselectidget, &FileSelectWidget::isOk, selectmainwidget,
                     &SelectMainWidget::changeSelectframeState);
    QObject::connect(configselectwidget, &ConfigSelectWidget::isOk, selectmainwidget,
                     &SelectMainWidget::changeSelectframeState);

    QObject::connect(selectmainwidget, &SelectMainWidget::updateBackupFileSize,
                     createbackupfilewidget, &CreateBackupFileWidget::updaeBackupFileSize);

    QObject::connect(TransferHelper::instance(), &TransferHelper::unfinishedJob, this,
                     [](const QString jsonstr) {
                         if (CustomMessageBox::SelectContinueTransfer()) {
                             TransferHelper::instance()->Retransfer(jsonstr);
                         }
                     });
    QObject::connect(selectmainwidget, &SelectMainWidget::updateBackupFileSize,
                     createbackupfilewidget, &CreateBackupFileWidget::updaeBackupFileSize);
    QObject::connect(TransferHelper::instance(), &TransferHelper::clearWidget, this,
                     &MainWindowPrivate::clearWidget);
    QObject::connect(TransferHelper::instance(), &TransferHelper::changeWidgetText, this,
                     &MainWindowPrivate::changeAllWidgtText);
    QObject::connect(TransferHelper::instance(), &TransferHelper::changeWidget,
                     [this](PageName index) { stackedWidget->setCurrentIndex(index); });

    connect(TransferHelper::instance(), &TransferHelper::onlineStateChanged,
            [this, errorwidget](bool online) {
                if (online)
                    return;
                if (OptionsManager::instance()->getUserOption(Options::kTransferMethod).isEmpty())
                    return;
                if (OptionsManager::instance()->getUserOption(Options::kTransferMethod)[0]
                    == TransferMethod::kLocalExport)
                    return;
                int index = stackedWidget->currentIndex();
                // only these need jump to networkdisconnectwidget
                if (index >= PageName::promptwidget && index <= PageName::readywidget)
                    stackedWidget->setCurrentIndex(PageName::networkdisconnectwidget);
                if (index >= PageName::selectmainwidget && index <= PageName::appselectwidget) {
                    stackedWidget->setCurrentIndex(PageName::errorwidget);
                    errorwidget->setErrorType(ErrorType::networkError);
                }
            });
    // disconect transfer
    connect(TransferHelper::instance(), &TransferHelper::disconnected, [this, errorwidget]() {
        int index = stackedWidget->currentIndex();
        if (index >= PageName::selectmainwidget && index <= PageName::transferringwidget)
            stackedWidget->setCurrentIndex(PageName::errorwidget);
        errorwidget->setErrorType(ErrorType::networkError);
    });
}
void MainWindowPrivate::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    QPainter painter(q);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setBrush(QColor(220, 220, 220));
    painter.setPen(Qt::NoPen);

    QPainterPath path;
    path.addRoundedRect(q->rect(), 20, 20);
    painter.drawPath(path);
}

void MainWindowPrivate::initSideBar()
{
    sidebar = new QDockWidget("Sidebar", q);
    q->setDockOptions(QMainWindow::AllowNestedDocks | QMainWindow::AllowTabbedDocks);
    q->addDockWidget(Qt::LeftDockWidgetArea, sidebar);
    sidebar->setTitleBarWidget(new QWidget());
    sidebar->setFixedWidth(200);

    SidebarWidget *sidebarWidget = new SidebarWidget(q);
    sidebar->setWidget(sidebarWidget);
    sidebar->setVisible(false);
    sidebar->setStyleSheet(".SidebarWidget{"
                           "background-color: white;"
                           "border-bottom-left-radius: 18px;"
                           "}");

    windowsCentralWidgetSidebar->addWidget(sidebar);
}

void MainWindowPrivate::initTitleBar()
{
    QWidget *titleBar = new QWidget(q->centralWidget());
    MoveFilter *filter = new MoveFilter(q);
    titleBar->installEventFilter(filter);
    titleBar->setFixedHeight(50);
    titleBar->setStyleSheet("background-color: white;"
                            "border-top-left-radius: 20px;"
                            "border-top-right-radius: 20px;");

    QToolButton *closeButton = new QToolButton(titleBar);
    closeButton->setFixedSize(50, 50);
    closeButton->setIcon(QIcon(":/icon/close_normal.svg"));
    closeButton->setIconSize(QSize(50, 50));

    QToolButton *minButton = new QToolButton(titleBar);
    minButton->setIcon(QIcon(":/icon/minimise_normal.svg"));
    minButton->setFixedSize(50, 50);
    minButton->setIconSize(QSize(50, 50));

    QLabel *iconLabel = new QLabel(titleBar);
    iconLabel->setPixmap(QPixmap(":/icon/icon.svg"));
    QObject::connect(closeButton, &QToolButton::clicked, q, []() { QCoreApplication::quit(); });
    QObject::connect(minButton, &QToolButton::clicked, q, &MainWindow::showMinimized);

    QHBoxLayout *titleLayout = new QHBoxLayout(titleBar);

    titleLayout->addWidget(iconLabel);
    titleLayout->addWidget(minButton, Qt::AlignHCenter);
    titleLayout->addWidget(closeButton, Qt::AlignHCenter);
    titleLayout->setContentsMargins(8, 0, 0, 0);
    titleBar->setLayout(titleLayout);

    q->centralWidget()->layout()->addWidget(titleBar);
}

void MainWindowPrivate::handleCurrentChanged(int index)
{
    if (index == PageName::filewselectidget)
        sidebar->setVisible(true);
    else
        sidebar->setVisible(false);
}

void MainWindowPrivate::clearWidget()
{
    //  OptionsManager::instance()->clear();
    SelectMainWidget *widgetMainselect =
            qobject_cast<SelectMainWidget *>(stackedWidget->widget(PageName::selectmainwidget));
    ConfigSelectWidget *widgetConfig =
            qobject_cast<ConfigSelectWidget *>(stackedWidget->widget(PageName::configselectwidget));
    AppSelectWidget *widgetApp =
            qobject_cast<AppSelectWidget *>(stackedWidget->widget(PageName::appselectwidget));
    FileSelectWidget *widgetFile =
            qobject_cast<FileSelectWidget *>(stackedWidget->widget(PageName::filewselectidget));
    CreateBackupFileWidget *widgetbackupFile = qobject_cast<CreateBackupFileWidget *>(
            stackedWidget->widget(PageName::createbackupfilewidget));
    TransferringWidget *widgetTransfer =
            qobject_cast<TransferringWidget *>(stackedWidget->widget(PageName::transferringwidget));
    ResultDisplayWidget *widgetRes =
            qobject_cast<ResultDisplayWidget *>(stackedWidget->widget(PageName::resultwidget));

    widgetFile->clear();
    widgetConfig->clear();
    widgetApp->clear();
    widgetMainselect->clear();
    widgetbackupFile->clear();
    widgetTransfer->clear();
    widgetRes->clear();
}

void MainWindowPrivate::changeAllWidgtText()
{
    SelectMainWidget *widgetMainselect =
            qobject_cast<SelectMainWidget *>(stackedWidget->widget(PageName::selectmainwidget));
    ConfigSelectWidget *widgetConfig =
            qobject_cast<ConfigSelectWidget *>(stackedWidget->widget(PageName::configselectwidget));
    AppSelectWidget *widgetApp =
            qobject_cast<AppSelectWidget *>(stackedWidget->widget(PageName::appselectwidget));
    FileSelectWidget *widgetFile =
            qobject_cast<FileSelectWidget *>(stackedWidget->widget(PageName::filewselectidget));
    widgetMainselect->changeText();
    widgetConfig->changeText();
    widgetApp->changeText();
    widgetFile->changeText();
    qobject_cast<SidebarWidget *>(sidebar->widget())->changeUI();
}

MoveFilter::MoveFilter(MainWindow *qq)
    : q(qq) {}

bool MoveFilter::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
        if (mouseEvent->buttons() == Qt::LeftButton) {
            leftButtonPressed = true;
            lastPosition = mouseEvent->globalPos() - q->pos();
        }
        return true;
    }
    if (event->type() == QEvent::MouseButtonRelease) {
        leftButtonPressed = false;
        return true;
    }
    if (event->type() == QEvent::MouseMove) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
        if ((mouseEvent->buttons() == Qt::LeftButton) && leftButtonPressed) {
            q->move(mouseEvent->globalPos() - lastPosition);
        }
        return true;
    }

    return QObject::eventFilter(obj, event);
}
