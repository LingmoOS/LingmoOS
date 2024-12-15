#include "selectmainwidget.h"
#include "../type_defines.h"
#include "userselectfilesize.h"

#include <QToolButton>
#include <QHBoxLayout>
#include <QStackedWidget>
#include <QDebug>
#include <QLabel>
#include <QPainterPath>
#include <gui/connect/choosewidget.h>
#include <gui/mainwindow_p.h>
#include <net/helper/transferhepler.h>
#include <utils/optionsmanager.h>

SelectMainWidget::SelectMainWidget(QWidget *parent) : QFrame(parent)
{
    initUi();
}

SelectMainWidget::~SelectMainWidget() { }

void SelectMainWidget::changeSelectframeState(const SelectItemName &name)
{
    if (name == SelectItemName::APP) {
        QStringList list = OptionsManager::instance()->getUserOption(Options::kApp);
        if (list.isEmpty()) {
            appItem->isOk = false;
        } else {
            appItem->isOk = true;
        }
        appItem->updateSelectSize(QString::number(list.size()));
    } else if (name == SelectItemName::FILES) {
        if (OptionsManager::instance()->getUserOption(Options::kFile).isEmpty()) {
            fileItem->isOk = false;
        } else {
            fileItem->isOk = true;
        }
    } else if (name == SelectItemName::CONFIG) {
        QStringList ConfigList = OptionsManager::instance()->getUserOption(Options::kConfig);
        QStringList BrowserList =
                OptionsManager::instance()->getUserOption(Options::kBrowserBookmarks);
        if (ConfigList.isEmpty() && BrowserList.isEmpty()) {
            configItem->isOk = false;
        } else {
            configItem->isOk = true;
        }
        configItem->updateSelectSize(QString::number(ConfigList.size() + BrowserList.size()));
    }
}

void SelectMainWidget::changeText()
{
    QString method = OptionsManager::instance()->getUserOption(Options::kTransferMethod)[0];
    if (method == TransferMethod::kLocalExport) {
        titileLabel->setText(LocalText);
        nextButton->setText(BtnLocalText);
        LocalIndelabel->setVisible(true);
        InternetIndelabel->setVisible(false);
    } else if (method == TransferMethod::kNetworkTransmission) {
        titileLabel->setText(InternetText);
        nextButton->setText(BtnInternetText);
        LocalIndelabel->setVisible(false);
        InternetIndelabel->setVisible(true);
    }
}

void SelectMainWidget::clear()
{
    changeSelectframeState(SelectItemName::FILES);
    changeSelectframeState(SelectItemName::CONFIG);
    changeSelectframeState(SelectItemName::APP);
}

void SelectMainWidget::initUi()
{
    setStyleSheet("background-color: white; border-radius: 10px;");

    QVBoxLayout *mainLayout = new QVBoxLayout();
    setLayout(mainLayout);

    titileLabel = new QLabel(LocalText, this);
    titileLabel->setFixedHeight(50);
    QFont font;
    font.setPixelSize(24);
    font.setWeight(QFont::DemiBold);
    titileLabel->setFont(font);
    titileLabel->setAlignment(Qt::AlignTop | Qt::AlignHCenter);

    fileItem = new SelectItem(tr("File"), QIcon(":/icon/picture-file@2x.png"),
                              SelectItemName::FILES, this);
    QObject::connect(UserSelectFileSize::instance(), &UserSelectFileSize::updateUserFileSelectSize,
                     fileItem, &SelectItem::updateSelectSize);

    appItem = new SelectItem(tr("App"), QIcon(":/icon/picture-app@2x.png"), SelectItemName::APP,
                             this);
    configItem = new SelectItem(tr("Config"), QIcon(":/icon/picture-configuration@2x.png"),
                                SelectItemName::CONFIG, this);

    QHBoxLayout *modeLayout = new QHBoxLayout();

    modeLayout->addWidget(fileItem, Qt::AlignTop);
    modeLayout->addSpacing(0);
    modeLayout->addWidget(appItem, Qt::AlignTop);
    modeLayout->addSpacing(0);
    modeLayout->addWidget(configItem, Qt::AlignTop);
    modeLayout->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
    modeLayout->setSpacing(10);

    ButtonLayout *buttonLayout = new ButtonLayout();
    QPushButton *backButton = buttonLayout->getButton1();
    backButton->setText(tr("Back"));
    nextButton = buttonLayout->getButton2();
    nextButton->setText(BtnLocalText);

    connect(backButton, &QPushButton::clicked, this, &SelectMainWidget::backPage);
    connect(nextButton, &QToolButton::clicked, this, &SelectMainWidget::nextPage);

    LocalIndelabel = new IndexLabel(1, this);
    LocalIndelabel->setAlignment(Qt::AlignCenter);
    InternetIndelabel = new IndexLabel(2, this);
    InternetIndelabel->setAlignment(Qt::AlignCenter);

    QHBoxLayout *indexLayout = new QHBoxLayout();
    indexLayout->addWidget(LocalIndelabel, Qt::AlignCenter);
    indexLayout->addWidget(InternetIndelabel, Qt::AlignCenter);

   // mainLayout->setSpacing(0);
    mainLayout->addSpacing(40);
    mainLayout->addWidget(titileLabel);
    mainLayout->addSpacing(45);
    mainLayout->addLayout(modeLayout);
    mainLayout->addSpacing(160);
    mainLayout->addLayout(buttonLayout);
    mainLayout->addSpacing(4);
    mainLayout->addLayout(indexLayout);
}
void SelectMainWidget::nextPage()
{
    // If the selected file is being calculated ,return
    if (!UserSelectFileSize::instance()->done()) {
        return;
    }

    QStringList sizelist;
    sizelist.push_back(QString::number(
            static_cast<qint64>(UserSelectFileSize::instance()->getAllSelectSize())));
    OptionsManager::instance()->addUserOption(Options::KSelectFileSize, sizelist);
    LOG << "user select file size:"
        << OptionsManager::instance()->getUserOption(Options::KSelectFileSize)[0].toStdString();

    PageName next;
    QString method = OptionsManager::instance()->getUserOption(Options::kTransferMethod)[0];
    if (method == TransferMethod::kLocalExport) {
        next = PageName::createbackupfilewidget;
        emit updateBackupFileSize();
    } else if (method == TransferMethod::kNetworkTransmission) {
        next = PageName::transferringwidget;
        // transfer
        TransferHelper::instance()->startTransfer();
    }

    // ui
    emit TransferHelper::instance()->changeWidget(next);
}

void SelectMainWidget::backPage()
{
    PageName back;
    QString method = OptionsManager::instance()->getUserOption(Options::kTransferMethod)[0];
    if (method == TransferMethod::kLocalExport) {
        back = PageName::choosewidget;
    } else if (method == TransferMethod::kNetworkTransmission) {
        back = PageName::readywidget;
    }
    TransferHelper::instance()->disconnectRemote();
    emit TransferHelper::instance()->changeWidget(back);
}

void SelectMainWidget::selectPage()
{
    SelectItem *selectitem = qobject_cast<SelectItem *>(QObject::sender());
    QStackedWidget *stackedWidget = qobject_cast<QStackedWidget *>(this->parent());
    int pageNum = -1;
    if (selectitem->name == SelectItemName::FILES) {
        pageNum = PageName::filewselectidget;
    } else if (selectitem->name == SelectItemName::APP) {
        pageNum = PageName::appselectwidget;
    } else if (selectitem->name == SelectItemName::CONFIG) {
        pageNum = PageName::configselectwidget;
    }
    if (pageNum == -1) {
        WLOG << "Jump to next page failed, qobject_cast<QStackedWidget *>(this->parent()) = "
                "nullptr";
    } else {
        stackedWidget->setCurrentIndex(pageNum);
    }
}

SelectItem::SelectItem(QString text, QIcon icon, SelectItemName itemName, QWidget *parent)
    : QFrame(parent), name(itemName)
{
    setStyleSheet(" background-color: rgba(0, 0, 0,0.08); border-radius: 8px;");
    setFixedSize(160, 185);

    QLabel *selectNname = new QLabel(text, this);
    selectNname->setAlignment(Qt::AlignCenter);
    selectNname->setStyleSheet("background-color: rgba(0, 0, 0, 0);");

    QLabel *iconLabel = new QLabel(this);
    iconLabel->setPixmap(icon.pixmap(100, 80));
    iconLabel->setAlignment(Qt::AlignCenter);
    iconLabel->setStyleSheet("background-color: rgba(0, 0, 0, 0);");

    sizeLabel = new QLabel(this);
    sizeLabel->setText(tr("Selected:0"));
    sizeLabel->setAlignment(Qt::AlignCenter);
    sizeLabel->setStyleSheet("background-color: rgba(0, 0, 0, 0);");

    QVBoxLayout *mainLayout = new QVBoxLayout();
    setLayout(mainLayout);
    mainLayout->setSpacing(0);
    mainLayout->addWidget(selectNname);
    mainLayout->addWidget(iconLabel);
    mainLayout->addWidget(sizeLabel);

    initEditFrame();

    QObject::connect(this, &SelectItem::changePage, qobject_cast<SelectMainWidget *>(parent),
                     &SelectMainWidget::selectPage);
}
SelectItem::~SelectItem() { }

void SelectItem::updateSelectSize(QString num)
{
    if (name == SelectItemName::APP) {
        sizeLabel->setText(QString(tr("Selected:%1")).arg(num));
    } else if (name == SelectItemName::FILES) {
        sizeLabel->setText(QString(tr("Selected:%1")).arg(num));
    } else if (name == SelectItemName::CONFIG) {
        sizeLabel->setText(QString(tr("Selected:%1")).arg(num));
    } else {
        DLOG << "selectItemName is error!";
    }
}
void SelectItem::mousePressEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    emit changePage();
}

void SelectItem::enterEvent(EnterEvent *event)
{
    Q_UNUSED(event)
    editFrame->show();
}

void SelectItem::leaveEvent(QEvent *event)
{
    Q_UNUSED(event)
    editFrame->hide();
}

void SelectItem::paintEvent(QPaintEvent *event)
{
    QFrame::paintEvent(event);

    if (isOk) {
        this->setStyleSheet(".SelectItem{background-color:rgba(0,129,255,0.1);}");
        QPainter painter(this);

        painter.setRenderHint(QPainter::Antialiasing);

        QPainterPath path;
        path.moveTo(QPoint(this->width() - 16, 0));
        path.arcTo(this->width() - 16, 0, 16, 16, 0, 90);
        path.lineTo(QPoint(this->width() - 30, 0));
        path.lineTo(QPoint(this->width(), 34));
        path.lineTo(QPoint(this->width(), 8));
        painter.fillPath(path, QColor(0, 129, 255));

        QPixmap iconPixmap(":/icon/checked.svg");
        QSize scaledSize(13, 11);
        QPixmap scaledPixmap = iconPixmap.scaled(scaledSize);
        QRect iconRect(this->width() - 15, 5, scaledSize.width(), scaledSize.height());
        painter.drawPixmap(iconRect, scaledPixmap);
    } else {
        this->setStyleSheet(".SelectItem{background-color: rgba(0, 0, 0,0.08);}");
    }
}
void SelectItem::initEditFrame()
{
    editFrame = new QFrame(this);
    editFrame->setStyleSheet("background-color: rgba(0, 0, 0, 0.3);");
    editFrame->setGeometry(0, 0, width(), height());
    editFrame->hide();

    QLabel *iconLabel = new QLabel(editFrame);
    iconLabel->setPixmap(QIcon(":/icon/edit.svg").pixmap(24, 24));
    iconLabel->setAlignment(Qt::AlignBottom | Qt::AlignHCenter);
    iconLabel->setStyleSheet("background-color: rgba(0, 0, 0, 0);");

    QLabel *textLabel = new QLabel(editFrame);
    textLabel->setText(tr("Edit"));
    textLabel->setStyleSheet("background-color: rgba(0, 0, 0, 0);"
                             "font-family: \"SourceHanSansSC-Medium\";"
                             "color:white;"
                             "font-size: 14px;"
                             "font-weight: 500;"
                             "font-style: normal;"
                             "text-align: center;");
    textLabel->setAlignment(Qt::AlignTop | Qt::AlignHCenter);

    QVBoxLayout *editLayout = new QVBoxLayout(editFrame);

    editFrame->setLayout(editLayout);
    editLayout->addWidget(iconLabel);
    editLayout->addWidget(textLabel);
    editLayout->setSpacing(10);
}

void SelectItem::changeState(const bool &ok)
{
    isOk = ok;
    update();
}
