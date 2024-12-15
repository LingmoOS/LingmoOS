#include "configselectwidget.h"
#include "appselectwidget.h"
#include "item.h"
#include "../type_defines.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QDebug>
#include <QToolButton>
#include <QStackedWidget>
#include <QCheckBox>
#include <QScrollArea>
#include <QListView>

#include <utils/optionsmanager.h>
#include <net/helper/transferhepler.h>
#include <gui/mainwindow_p.h>

ConfigSelectWidget::ConfigSelectWidget(QWidget *parent) : QFrame(parent)
{
    initUI();
}

ConfigSelectWidget::~ConfigSelectWidget() { }

void ConfigSelectWidget::initUI()
{
    setStyleSheet(".ConfigSelectWidget{background-color: white; border-radius: 8px;}");

    QVBoxLayout *mainLayout = new QVBoxLayout();
    setLayout(mainLayout);
    titileLabel = new QLabel(LocalText, this);
    QFont font;
    font.setPixelSize(24);
    font.setWeight(QFont::DemiBold);
    titileLabel->setFont(font);
    titileLabel->setAlignment(Qt::AlignTop | Qt::AlignHCenter);

    QLabel *tipLabel1 =
            new QLabel(tr("Check transfer configuration will automatically apply to UOS."), this);
    tipLabel1->setWordWrap(true);
    tipLabel1->setFixedHeight(30);
    tipLabel1->setAlignment(Qt::AlignCenter);
    font.setPointSize(10);
    font.setWeight(QFont::Thin);
    tipLabel1->setFont(font);

    ButtonLayout *buttonLayout = new ButtonLayout();
    cancelButton = buttonLayout->getButton1();
    cancelButton->setText(tr("Cancel"));
    determineButton = buttonLayout->getButton2();
    determineButton->setText(tr("Confirm"));

    connect(determineButton, &QPushButton::clicked, this, &ConfigSelectWidget::nextPage);
    connect(cancelButton, &QPushButton::clicked, this, &ConfigSelectWidget::backPage);

    initSelectBrowerBookMarkFrame();
    initSelectConfigFrame();
    mainLayout->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
    mainLayout->addSpacing(30);
    mainLayout->addWidget(titileLabel);
    mainLayout->addSpacing(3);
    mainLayout->addWidget(tipLabel1);
//    mainLayout->addSpacing(10);
    mainLayout->addWidget(selectBrowerBookMarkFrame);
    mainLayout->addSpacing(10);
    mainLayout->addWidget(selectConfigFrame);
    mainLayout->addSpacing(10);
    mainLayout->addLayout(buttonLayout);
}

void ConfigSelectWidget::initSelectBrowerBookMarkFrame()
{
    QVBoxLayout *selectframeLayout = new QVBoxLayout();
    selectframeLayout->setContentsMargins(1, 1, 1, 1);
    ItemTitlebar *titlebar = new ItemTitlebar(tr("Browser bookmarks"), tr("Recommendation"), 40,
                                              360, QRectF(10, 8, 16, 16), 3, this);
    titlebar->setFixedSize(500, 36);
    selectBrowerBookMarkFrame = new QFrame(this);
    selectBrowerBookMarkFrame->setFixedSize(500, 210);
    selectBrowerBookMarkFrame->setStyleSheet(".QFrame{"
                                             "border-radius: 8px;"
                                             " border: 2px solid rgba(0,0,0, 0.1);"
                                             " opacity: 1;"
                                             "background-color: rgba(255,255,255, 1);"
                                             "}");
    selectBrowerBookMarkFrame->setLayout(selectframeLayout);

    browserView = new SelectListView(this);

    QStandardItemModel *model = browserView->getModel();
    browserView->setItemDelegate(
            new ItemDelegate(84, 250, 376, 100, 40, QPoint(52, 6), QPoint(10, 9)));

    QMap<QString, QString> browserList = TransferHelper::instance()->getBrowserList();
    for (auto iterator = browserList.begin(); iterator != browserList.end(); iterator++) {
        QStandardItem *item = new QStandardItem();
        item->setData(iterator.key(), Qt::DisplayRole);
        item->setData(tr("Transferable"), Qt::ToolTipRole);
        item->setIcon(QIcon(iterator.value()));
        item->setCheckable(true);
        model->appendRow(item);
    }

    selectframeLayout->addWidget(titlebar);
    selectframeLayout->addWidget(browserView);
    QObject::connect(titlebar, &ItemTitlebar::selectAll, browserView,
                     &SelectListView::selectorDelAllItem);
    QObject::connect(browserView, &SelectListView::currentSelectState, titlebar,
                     &ItemTitlebar::updateSelectAllButState);
    QObject::connect(titlebar, &ItemTitlebar::sort, browserView, &SelectListView::sortListview);
}

void ConfigSelectWidget::initSelectConfigFrame()
{
    QVBoxLayout *selectframeLayout = new QVBoxLayout();
    ItemTitlebar *titlebar = new ItemTitlebar(tr("Personal Settings"), tr("Recommendation"), 40,
                                              360, QRectF(10, 8, 16, 16), 3, this);
    titlebar->setFixedSize(500, 36);

    selectConfigFrame = new QFrame(this);
    selectframeLayout->setContentsMargins(1, 1, 1, 1);
    selectConfigFrame->setFixedSize(500, 88);
    selectConfigFrame->setStyleSheet(".QFrame{"
                                     "border-radius: 8px;"
                                     "border: 2px solid rgba(0,0,0, 0.1);"
                                     "opacity: 1;"
                                     "background-color: rgba(255,255,255, 1);"
                                     "}");
    selectConfigFrame->setLayout(selectframeLayout);

    configView = new SelectListView(this);

    QStandardItemModel *model = configView->getModel();
    configView->setItemDelegate(
            new ItemDelegate(55, 250, 376, 100, 40, QPoint(52, 6), QPoint(10, 9)));

    QStandardItem *item = new QStandardItem();
    item->setData(tr("Customized Wallpaper"), Qt::DisplayRole);
    item->setData(tr("Transferable"), Qt::ToolTipRole);
    item->setCheckable(true);
    model->appendRow(item);

    selectframeLayout->addWidget(titlebar);
    selectframeLayout->addWidget(configView);
    QObject::connect(titlebar, &ItemTitlebar::selectAll, configView,
                     &SelectListView::selectorDelAllItem);
    QObject::connect(configView, &SelectListView::currentSelectState, titlebar,
                     &ItemTitlebar::updateSelectAllButState);
    QObject::connect(titlebar, &ItemTitlebar::sort, configView, &SelectListView::sortListview);
}

void ConfigSelectWidget::sendOptions()
{
    QStringList browser;
    QAbstractItemModel *model = browserView->model();
    for (int row = 0; row < model->rowCount(); ++row) {
        QModelIndex index = model->index(row, 0);
        QVariant checkboxData = model->data(index, Qt::CheckStateRole);
        Qt::CheckState checkState = static_cast<Qt::CheckState>(checkboxData.toInt());
        if (checkState == Qt::Checked) {
            browser << model->data(index, Qt::DisplayRole).toString();
        }
    }

    qInfo() << "select browser :" << browser;
    OptionsManager::instance()->addUserOption(Options::kBrowserBookmarks, browser);

    QStringList config;
    model = configView->model();
    for (int row = 0; row < model->rowCount(); ++row) {
        QModelIndex index = model->index(row, 0);
        QVariant checkboxData = model->data(index, Qt::CheckStateRole);
        Qt::CheckState checkState = static_cast<Qt::CheckState>(checkboxData.toInt());
        if (checkState == Qt::Checked) {
            config << model->data(index, Qt::DisplayRole).toString();
        }
    }

    qInfo() << "select config :" << config;
    OptionsManager::instance()->addUserOption(Options::kConfig, config);

    emit isOk(SelectItemName::CONFIG);
}

void ConfigSelectWidget::delOptions()
{
    // Clear All config Selections
    QAbstractItemModel *model = browserView->getModel();
    for (int row = 0; row < model->rowCount(); ++row) {
        QModelIndex index = model->index(row, 0);
        QVariant checkboxData = model->data(index, Qt::CheckStateRole);
        Qt::CheckState checkState = static_cast<Qt::CheckState>(checkboxData.toInt());
        if (checkState == Qt::Checked) {
            model->setData(index, Qt::Unchecked, Qt::CheckStateRole);
        }
    }

    model = configView->getModel();
    for (int row = 0; row < model->rowCount(); ++row) {
        QModelIndex index = model->index(row, 0);
        QVariant checkboxData = model->data(index, Qt::CheckStateRole);
        Qt::CheckState checkState = static_cast<Qt::CheckState>(checkboxData.toInt());
        if (checkState == Qt::Checked) {
            model->setData(index, Qt::Unchecked, Qt::CheckStateRole);
        }
    }
    OptionsManager::instance()->addUserOption(Options::kBrowserBookmarks, QStringList());
    OptionsManager::instance()->addUserOption(Options::kConfig, QStringList());

    // Deselect
    emit isOk(SelectItemName::CONFIG);
}

void ConfigSelectWidget::changeText()
{
    QString method = OptionsManager::instance()->getUserOption(Options::kTransferMethod)[0];
    if (method == TransferMethod::kLocalExport) {
        titileLabel->setText(LocalText);
    } else if (method == TransferMethod::kNetworkTransmission) {
        titileLabel->setText(InternetText);
    }
}

void ConfigSelectWidget::clear()
{
    QStandardItemModel *browsermodel = browserView->getModel();
    for (int row = 0; row < browsermodel->rowCount(); ++row) {
        QModelIndex itemIndex = browsermodel->index(row, 0);
        browsermodel->setData(itemIndex, Qt::Unchecked, Qt::CheckStateRole);
    }

    QStandardItemModel *configmodel = configView->getModel();
    for (int row = 0; row < configmodel->rowCount(); ++row) {
        QModelIndex itemIndex = configmodel->index(row, 0);
        configmodel->setData(itemIndex, Qt::Unchecked, Qt::CheckStateRole);
    }
    OptionsManager::instance()->addUserOption(Options::kBrowserBookmarks, QStringList());
    OptionsManager::instance()->addUserOption(Options::kConfig, QStringList());
}

void ConfigSelectWidget::nextPage()
{
    // send useroptions
    sendOptions();

    // nextpage
    emit TransferHelper::instance()->changeWidget(PageName::selectmainwidget);
}
void ConfigSelectWidget::backPage()
{
    // delete Options
    delOptions();

    emit TransferHelper::instance()->changeWidget(PageName::selectmainwidget);
}
