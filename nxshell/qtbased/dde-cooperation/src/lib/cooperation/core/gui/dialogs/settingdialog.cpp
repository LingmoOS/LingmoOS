// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "settingdialog.h"
#include "settingdialog_p.h"
#include "global_defines.h"
#include "gui/utils/cooperationguihelper.h"
#include "configs/settings/configmanager.h"
#include "configs/dconfig/dconfigmanager.h"

#ifdef linux
#    include <DPalette>
#endif

#include <QPainter>
#include <QEvent>
#include <QDir>
#include <QStandardPaths>
#include <QToolTip>
#include <QScrollBar>
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QRegularExpression>
#endif

#include <utils/cooperationutil.h>
#include <base/reportlog/reportlogmanager.h>
#include <gui/widgets/deviceitem.h>

#ifdef __linux__
static const char *Kdisplay_right = "display_right";
static const char *Kdisplay_left = "display_left";
#else
const char *Kdisplay_right = ":/icons/deepin/builtin/texts/display_right_24px.svg";
const char *Kdisplay_left = ":/icons/deepin/builtin/texts/display_left_24px.svg";
#endif

static const char *KAlert = "tool_tip_name_is_valid";

using namespace cooperation_core;

SettingDialogPrivate::SettingDialogPrivate(SettingDialog *qq)
    : QObject(qq),
      q(qq)
{
    findComboBoxInfo << tr("Everyone in the same LAN")

                     << tr("Not allow");
    connectComboBoxInfo << QPair<QString, QString>(Kdisplay_right, tr("Screen right"))
                        << QPair<QString, QString>(Kdisplay_left, tr("Screen left"));

    transferComboBoxInfo << tr("Everyone in the same LAN")
                         << tr("Only those who are collaborating are allowed")
                         << tr("Not allow");

    mainLayout = new QVBoxLayout(q);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
}

SettingDialogPrivate::~SettingDialogPrivate()
{
}

void SettingDialogPrivate::initWindow()
{
    q->setFixedSize(650, 584);

    contentLayout = new QVBoxLayout;
    contentLayout->setContentsMargins(10, 0, 10, 0);
    contentLayout->setSpacing(0);

    QScrollArea *contentArea = new QScrollArea(q);
    contentArea->setFrameShape(QFrame::NoFrame);
    contentArea->setWidgetResizable(true);
    contentArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

#ifndef linux
    QString scrollBarStyle = "QScrollBar:vertical {"
                             "    background: white;"
                             "    width: 6px;"
                             "    border-radius: 3px;"
                             "    margin: 0px;"
                             "}"
                             "QScrollBar::handle:vertical {"
                             "    background: darkGrey;"
                             "    min-height: 30px;"
                             "    border-radius: 3px;"
                             "}"
                             "QScrollBar::sub-line:vertical, QScrollBar::add-line:vertical {"
                             "    height: 0px;"
                             "}";
    ;
    contentArea->verticalScrollBar()->setStyleSheet(scrollBarStyle);
#endif

    QWidget *contentWidget = new QWidget(contentArea);
    contentWidget->installEventFilter(q);
    contentWidget->setObjectName("ContentWidget");
    contentWidget->setFixedWidth(633);
    contentArea->setWidget(contentWidget);
    contentWidget->setLayout(contentLayout);

    QWidget *mainWidget = new QWidget(q);
    mainWidget->installEventFilter(q);
    mainWidget->setObjectName("MainWidget");
    QHBoxLayout *layout = new QHBoxLayout(mainWidget);
    layout->setContentsMargins(0, 10, 0, 10);
    layout->addWidget(contentArea);

    QWidget *backgroundWidget = new QWidget(q);
    backgroundWidget->setObjectName("BackgroundWidget");
    backgroundWidget->installEventFilter(q);
    QHBoxLayout *hLayout = new QHBoxLayout(backgroundWidget);
    hLayout->setContentsMargins(10, 10, 10, 10);
    hLayout->addWidget(mainWidget);

    mainLayout->addWidget(backgroundWidget);

    initFont();
    createBasicWidget();
    createDeviceShareWidget();
    createClipboardShareWidget();
    createTransferWidget();
}

void SettingDialogPrivate::createBasicWidget()
{
    CooperationLabel *basicLable = new CooperationLabel(tr("Basic Settings"), q);
    auto cm = basicLable->contentsMargins();
    cm.setLeft(10);
    basicLable->setContentsMargins(cm);
    CooperationGuiHelper::setAutoFont(basicLable, 16, QFont::DemiBold);

    findCB = new QComboBox(q);
    findCB->addItems(findComboBoxInfo);
    findCB->setFixedWidth(280);
    connect(findCB, qOverload<int>(&QComboBox::currentIndexChanged), this, &SettingDialogPrivate::onFindComboBoxValueChanged);
    SettingItem *findItem = new SettingItem(q);
    findItem->setItemInfo(tr("Discovery mode"), findCB);

    CooperationLabel *tipLabel = new CooperationLabel(tr("Other devices can discover and connect with you through the \"Cooperation\" app"), q);
    auto margins = tipLabel->contentsMargins();
    margins.setLeft(10);
    tipLabel->setContentsMargins(margins);
    tipLabel->setWordWrap(true);
    CooperationGuiHelper::setAutoFont(tipLabel, 12, tipFont.weight());

#ifdef linux
    tipLabel->setForegroundRole(DTK_GUI_NAMESPACE::DPalette::TextTips);
#else
    basicLable->setFixedHeight(36);
    setQComboxWinStyle(findCB);
    QList<QColor> colorList { QColor(0, 0, 0, static_cast<int>(255 * 0.5)),
                              QColor(192, 192, 192) };
    CooperationGuiHelper::instance()->autoUpdateTextColor(tipLabel, colorList);
#endif

    nameEdit = new CooperationLineEdit(q);
    nameEdit->installEventFilter(q);

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QRegularExpression regExp("^[a-zA-Z0-9\u4e00-\u9fa5-]+$");   // 正则表达式定义允许的字符范围
    QRegularExpressionValidator *validator = new QRegularExpressionValidator(regExp, nameEdit);
#else
    QRegExp regExp("^[a-zA-Z0-9\u4e00-\u9fa5-]+$");   // 正则表达式定义允许的字符范围
    QRegExpValidator *validator = new QRegExpValidator(regExp, nameEdit);
#endif
#ifdef linux
    nameEdit->lineEdit()->setValidator(validator);
    nameEdit->setClearButtonEnabled(false);
#else
    nameEdit->setValidator(validator);
    nameEdit->setStyleSheet("   border-radius: 8px;"
                            "   padding: 5px;"
                            "   background-color: rgba(0,0,0,0.08);");
    nameEdit->setFixedHeight(36);
    QToolButton *closeBtn = new QToolButton(nameEdit);
    closeBtn->setIcon(QIcon(":/icons/deepin/builtin/icons/tab_close_normal.svg"));
    closeBtn->setIconSize(QSize(35, 35));
    closeBtn->setStyleSheet("background-color: rgba(0,0,0,0);");
    closeBtn->setGeometry(230, -7, 50, 50);
    closeBtn->setVisible(false);
    connect(nameEdit, &QLineEdit::textChanged, this,
            [closeBtn](const QString &str) { closeBtn->setVisible(!str.isEmpty()); });
    connect(closeBtn, &QToolButton::clicked, this, [this] { nameEdit->setText(""); });
#endif
    nameEdit->setFixedWidth(280);
    connect(nameEdit, &CooperationLineEdit::editingFinished, this, &SettingDialogPrivate::onEditFinished);
    connect(nameEdit, &CooperationLineEdit::textChanged, this, &SettingDialogPrivate::onNameChanged);
    SettingItem *nameItem = new SettingItem(q);
    nameItem->setItemInfo(tr("Device name"), nameEdit);

    contentLayout->addWidget(basicLable);
    contentLayout->addSpacing(10);
    contentLayout->addWidget(findItem);
    contentLayout->addSpacing(4);
    contentLayout->addWidget(tipLabel);
    contentLayout->addSpacing(16);
    contentLayout->addWidget(nameItem);
    contentLayout->addSpacing(10);
}

void SettingDialogPrivate::createDeviceShareWidget()
{
    devShareSwitchBtn = new CooperationSwitchButton(q);
    connect(devShareSwitchBtn, &CooperationSwitchButton::clicked, this, &SettingDialogPrivate::onDeviceShareButtonClicked);

    SettingItem *deviceShareItem = new SettingItem(q);
    deviceShareItem->setItemInfo(tr("Peripheral share"), devShareSwitchBtn);

    CooperationLabel *tipLabel = new CooperationLabel(tr("Allows peripherals that have been established "
                                                         "to collaborate across devices to control this "
                                                         "device, including keyboard, mouse, trackpad, etc"),
                                                      q);
    auto margins = tipLabel->contentsMargins();
    margins.setLeft(10);
    tipLabel->setContentsMargins(margins);
    tipLabel->setWordWrap(true);

    CooperationLabel *tipLabel2 = new CooperationLabel(tr("When this device is the connected party, the mouse penetrates"
                                                          " from the screen of this device to the position of the connected party"),
                                                       q);
    margins = tipLabel2->contentsMargins();
    margins.setLeft(10);
    tipLabel2->setContentsMargins(margins);
    tipLabel2->setWordWrap(true);

    CooperationGuiHelper::setAutoFont(tipLabel, 12, tipFont.weight());
    CooperationGuiHelper::setAutoFont(tipLabel2, 12, tipFont.weight());
    connectCB = new QComboBox(q);
    connectCB->setFixedWidth(280);
    connectCB->setIconSize(QSize(24, 24));

#ifdef linux
    tipLabel->setForegroundRole(DTK_GUI_NAMESPACE::DPalette::TextTips);
    tipLabel2->setForegroundRole(DTK_GUI_NAMESPACE::DPalette::TextTips);
#else
    QList<QColor> colorList { QColor(0, 0, 0, static_cast<int>(255 * 0.5)),
                              QColor(192, 192, 192) };
    CooperationGuiHelper::instance()->autoUpdateTextColor(tipLabel, colorList);
    CooperationGuiHelper::instance()->autoUpdateTextColor(tipLabel2, colorList);
    setQComboxWinStyle(connectCB);
#endif

    for (const auto &info : connectComboBoxInfo) {
        connectCB->addItem(QIcon::fromTheme(info.first), info.second);
    }
    connect(connectCB, qOverload<int>(&QComboBox::currentIndexChanged), this, &SettingDialogPrivate::onConnectComboBoxValueChanged);
    SettingItem *connectItem = new SettingItem(q);
    connectItem->setItemInfo(tr("Connection direction"), connectCB);

    contentLayout->addWidget(deviceShareItem);
    contentLayout->addSpacing(4);
    contentLayout->addWidget(tipLabel);
    contentLayout->addSpacing(16);
    contentLayout->addWidget(connectItem);
    contentLayout->addSpacing(4);
    contentLayout->addWidget(tipLabel2);
    contentLayout->addSpacing(16);
}

void SettingDialogPrivate::createTransferWidget()
{
    transferCB = new QComboBox(q);
    transferCB->addItems(transferComboBoxInfo);
    transferCB->setFixedWidth(280);
    connect(transferCB, qOverload<int>(&QComboBox::currentIndexChanged), this, &SettingDialogPrivate::onTransferComboBoxValueChanged);
    SettingItem *transferItem = new SettingItem(q);
    transferItem->setItemInfo(tr("Allows the following users to send files to me"), transferCB);

    chooserEdit = new FileChooserEdit(q);
    chooserEdit->setFixedWidth(280);
    connect(chooserEdit, &FileChooserEdit::fileChoosed, this, &SettingDialogPrivate::onFileChoosed);

    SettingItem *fileSaveItem = new SettingItem(q);
    fileSaveItem->setItemInfo(tr("File save location"), chooserEdit);

    contentLayout->addWidget(transferItem);
    contentLayout->addSpacing(10);
    contentLayout->addWidget(fileSaveItem);
    contentLayout->addSpacing(10);
#ifndef linux
    setQComboxWinStyle(transferCB);
#endif
}

void SettingDialogPrivate::createClipboardShareWidget()
{
    clipShareSwitchBtn = new CooperationSwitchButton(q);
    connect(clipShareSwitchBtn, &CooperationSwitchButton::clicked, this, &SettingDialogPrivate::onClipboardShareButtonClicked);
    SettingItem *clipShareItem = new SettingItem(q);
    clipShareItem->setItemInfo(tr("Share clipboard"), clipShareSwitchBtn);

    CooperationLabel *tipLabel = new CooperationLabel(tr("When sharing is enabled on the"
                                                         " server (the connected device), "
                                                         "the clipboard will be shared between "
                                                         "the two computers"),
                                                      q);
    auto margins = tipLabel->contentsMargins();
    margins.setLeft(10);
    tipLabel->setContentsMargins(margins);
    tipLabel->setWordWrap(true);
    CooperationGuiHelper::setAutoFont(tipLabel, 12, tipFont.weight());
#ifdef linux
    tipLabel->setForegroundRole(DTK_GUI_NAMESPACE::DPalette::TextTips);
#else
    QList<QColor> colorList { QColor(0, 0, 0, static_cast<int>(255 * 0.5)),
                              QColor(192, 192, 192) };
    CooperationGuiHelper::instance()->autoUpdateTextColor(tipLabel, colorList);
#endif

    contentLayout->addWidget(clipShareItem);
    contentLayout->addSpacing(4);
    contentLayout->addWidget(tipLabel);
    contentLayout->addSpacing(16);
}

void SettingDialogPrivate::onFindComboBoxValueChanged(int index)
{
#ifdef linux
    DConfigManager::instance()->setValue(kDefaultCfgPath, DConfigKey::DiscoveryModeKey, index);
#else
    ConfigManager::instance()->setAppAttribute(AppSettings::GenericGroup, AppSettings::DiscoveryModeKey, index);
#endif
}

void SettingDialogPrivate::onConnectComboBoxValueChanged(int index)
{
    ConfigManager::instance()->setAppAttribute(AppSettings::GenericGroup, AppSettings::LinkDirectionKey, index);
}

void SettingDialogPrivate::onTransferComboBoxValueChanged(int index)
{
#ifdef linux
    DConfigManager::instance()->setValue(kDefaultCfgPath, DConfigKey::TransferModeKey, index);
    reportDeviceStatus(DConfigKey::TransferModeKey, index != 2);
#else
    ConfigManager::instance()->setAppAttribute(AppSettings::GenericGroup, AppSettings::TransferModeKey, index);
#endif
}

bool SettingDialogPrivate::checkNameValid()
{
    int length = nameEdit->text().length();
    if ((length < 1) || (length > 20)) {
#ifdef linux
        nameEdit->setAlert(true);
        nameEdit->showAlertMessage(tr("The device name must contain 1 to 20 characters"));
        nameEdit->setFocus();
        return false;

#else
        nameEdit->setProperty(KAlert, true);
        QToolTip::showText(nameEdit->parentWidget()->mapToGlobal(nameEdit->pos()) + QPoint(41, 22),
                           tr("The device name must contain 1 to 63 characters"));
        nameEdit->setFocus();
        return false;
    } else {
        nameEdit->setProperty(KAlert, false);
        QToolTip::hideText();
#endif
    }

    return true;
}

void SettingDialogPrivate::onEditFinished()
{
    if (checkNameValid())
        ConfigManager::instance()->setAppAttribute(AppSettings::GenericGroup, AppSettings::DeviceNameKey, nameEdit->text());
}

void SettingDialogPrivate::onNameChanged(const QString &text)
{
    Q_UNUSED(text)

#ifdef linux
    if (nameEdit->isAlert())
        nameEdit->setAlert(false);
#endif

    checkNameValid();
}

void SettingDialogPrivate::onDeviceShareButtonClicked(bool clicked)
{
    ConfigManager::instance()->setAppAttribute(AppSettings::GenericGroup, AppSettings::PeripheralShareKey, clicked);
    reportDeviceStatus(AppSettings::PeripheralShareKey, clicked);
}

void SettingDialogPrivate::onClipboardShareButtonClicked(bool clicked)
{
    ConfigManager::instance()->setAppAttribute(AppSettings::GenericGroup, AppSettings::ClipboardShareKey, clicked);
    reportDeviceStatus(AppSettings::ClipboardShareKey, clicked);
}

void SettingDialogPrivate::onFileChoosed(const QString &path)
{
    ConfigManager::instance()->setAppAttribute(AppSettings::GenericGroup, AppSettings::StoragePathKey, path);
}

void SettingDialogPrivate::initFont()
{
    groupFont = q->font();
    groupFont.setWeight(QFont::DemiBold);
    groupFont.setPixelSize(16);

    tipFont = q->font();
    tipFont.setWeight(QFont::Normal);
}

void SettingDialogPrivate::reportDeviceStatus(const QString &type, bool status)
{
#ifdef linux
    QVariantMap data;

    if (type == AppSettings::PeripheralShareKey)
        data.insert("enablePeripheralShare", status);
    else if (type == DConfigKey::TransferModeKey)
        data.insert("enableFileDelivery", status);
    else if (type == AppSettings::ClipboardShareKey)
        data.insert("enableClipboardShare", status);

    deepin_cross::ReportLogManager::instance()->commit(ReportAttribute::CooperationStatus, data);
#endif
}

SettingDialog::SettingDialog(QWidget *parent)
    : CooperationAbstractDialog(parent),
      d(new SettingDialogPrivate(this))
{
    d->initWindow();
#ifdef linux
    d->initTitleBar();
#else
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setFixedSize(650, 530);
#endif
    setModal(true);
}

SettingDialog::~SettingDialog()
{
}

bool SettingDialog::eventFilter(QObject *watched, QEvent *event)
{
    // 绘制背景
    do {
        if (event->type() != QEvent::Paint)
            break;

        QWidget *widget = qobject_cast<QWidget *>(watched);
        if (!widget)
            break;

        QPainter painter(widget);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setPen(Qt::NoPen);

        if ((watched->objectName() == "ContentWidget" || watched->objectName() == "MainWidget")) {
            QColor color(255, 255, 255);
            if (CooperationGuiHelper::isDarkTheme())
                color.setRgb(24, 24, 24);
            painter.setBrush(color);

            if (watched->objectName() == "MainWidget")
                painter.drawRoundedRect(widget->rect(), 8, 8);
            else
                painter.drawRect(widget->rect());
            return true;
        } else if (watched->objectName() == "BackgroundWidget") {
#ifdef linux
            QColor color(245, 245, 245);
#else
            QColor color(230, 230, 230);
#endif
            if (CooperationGuiHelper::isDarkTheme())
                color.setRgb(36, 36, 36);
            painter.setBrush(color);
            painter.drawRect(widget->rect());
            return true;
        }
#ifdef linux
        // DLineEdit在DAbstractDialog中使用setAlert，绘制无效，所以自绘
        else if (d->nameEdit == watched && d->nameEdit->isAlert()) {
            painter.setBrush(QColor(241, 57, 50, qRound(0.15 * 255)));
            painter.drawRoundedRect(d->nameEdit->lineEdit()->rect(), 8, 8);
            return true;
        }
#else
        // DLineEdit在DAbstractDialog中使用setAlert，绘制无效，所以自绘
        else if (d->nameEdit == watched && d->nameEdit->property(KAlert).toBool()) {
            painter.setBrush(QColor(241, 57, 50, qRound(0.15 * 255)));
            painter.drawRoundedRect(d->nameEdit->rect(), 8, 8);
            return CooperationAbstractDialog::eventFilter(watched, event);
        }
#endif
    } while (false);

    if (event->type() == QEvent::KeyRelease) {
        QWidget *widget = qobject_cast<QWidget *>(watched);
        if (widget && d->nameEdit == watched) {
            QKeyEvent *keypressEvent = static_cast<QKeyEvent *>(event);
            if (keypressEvent->key() == Qt::Key_Return || keypressEvent->key() == Qt::Key_Enter) {
                d->q->setFocus();
            }
        }
    }

    QWidget *widget = qobject_cast<QWidget *>(watched);
    if (widget && d->nameEdit == watched) {
        if (event->type() == QEvent::Paint) {
#ifdef linux
            if (d->nameEdit->hasFocus()) {
                d->nameEdit->setClearButtonEnabled(true);
            } else {
                d->nameEdit->setClearButtonEnabled(false);
            }
#else
            QToolButton *closeBtn = d->nameEdit->findChild<QToolButton *>();
            if (closeBtn != nullptr) {
                if (d->nameEdit->hasFocus()) {
                    closeBtn->setVisible(true);
                } else {
                    closeBtn->setVisible(false);
                }
            }
#endif
        }
    }

    return CooperationAbstractDialog::eventFilter(watched, event);
}

void SettingDialog::showEvent(QShowEvent *event)
{
    loadConfig();
    CooperationAbstractDialog::showEvent(event);
}

void SettingDialog::keyPressEvent(QKeyEvent *event)
{
#ifndef linux
    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
        event->ignore();
        return;
    }
#endif
    CooperationAbstractDialog::keyPressEvent(event);   // 其他键按默认处理
}

void SettingDialog::loadConfig()
{
#ifdef linux
    auto value = DConfigManager::instance()->value(kDefaultCfgPath, DConfigKey::DiscoveryModeKey, 0);
    int mode = value.toInt();
    mode = (mode < 0) ? 0 : (mode > 1) ? 1 : mode;
    d->findCB->setCurrentIndex(mode);
#else
    auto value = ConfigManager::instance()->appAttribute(AppSettings::GenericGroup, AppSettings::DiscoveryModeKey);
    if (value.isValid()) {
        d->findCB->setCurrentIndex(value.toInt());
    } else {
        // sync the default value into config
        ConfigManager::instance()->setAppAttribute(AppSettings::GenericGroup, AppSettings::DiscoveryModeKey, 0);

        d->findCB->setCurrentIndex(0);
    }
#endif

    value = ConfigManager::instance()->appAttribute(AppSettings::GenericGroup, AppSettings::DeviceNameKey);
    if (value.isValid()) {
        d->nameEdit->setText(value.toString());
    } else {
        QString defaultName = QDir(QStandardPaths::standardLocations(QStandardPaths::HomeLocation).value(0)).dirName();
        d->nameEdit->setText(defaultName);

        // sync the default name into config
        ConfigManager::instance()->setAppAttribute(AppSettings::GenericGroup, AppSettings::DeviceNameKey, defaultName);
    }

    value = ConfigManager::instance()->appAttribute(AppSettings::GenericGroup, AppSettings::PeripheralShareKey);
    if (value.isValid()) {
        d->devShareSwitchBtn->setChecked(value.toBool());
    } else {
        // sync the default value into config
        ConfigManager::instance()->setAppAttribute(AppSettings::GenericGroup, AppSettings::PeripheralShareKey, true);

        d->devShareSwitchBtn->setChecked(true);
    }

    value = ConfigManager::instance()->appAttribute(AppSettings::GenericGroup, AppSettings::LinkDirectionKey);
    d->connectCB->setCurrentIndex(value.isValid() ? value.toInt() : 0);
    if (value.isValid()) {
        d->connectCB->setCurrentIndex(value.toInt());
    } else {
        // sync the default value into config
        ConfigManager::instance()->setAppAttribute(AppSettings::GenericGroup, AppSettings::LinkDirectionKey, 0);

        d->connectCB->setCurrentIndex(0);
    }

#ifdef linux
    value = DConfigManager::instance()->value(kDefaultCfgPath, DConfigKey::TransferModeKey, 0);
    mode = value.toInt();
    mode = (mode < 0) ? 0 : (mode > 2) ? 2 : mode;
    d->transferCB->setCurrentIndex(mode);
#else
    value = ConfigManager::instance()->appAttribute(AppSettings::GenericGroup, AppSettings::TransferModeKey);
    if (value.isValid()) {
        d->transferCB->setCurrentIndex(value.toInt());
    } else {
        // sync the default value into config
        ConfigManager::instance()->setAppAttribute(AppSettings::GenericGroup, AppSettings::TransferModeKey, true);

        d->transferCB->setCurrentIndex(0);
    }
#endif

    value = ConfigManager::instance()->appAttribute(AppSettings::GenericGroup, AppSettings::StoragePathKey);
    if (value.isValid()) {
        d->chooserEdit->setText(value.toString());
    } else {
        QString defaultPath = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
        d->chooserEdit->setText(defaultPath);

        // sync the default path into config
        ConfigManager::instance()->setAppAttribute(AppSettings::GenericGroup, AppSettings::StoragePathKey, defaultPath);
    }

    value = ConfigManager::instance()->appAttribute(AppSettings::GenericGroup, AppSettings::ClipboardShareKey);
    if (value.isValid()) {
        d->clipShareSwitchBtn->setChecked(value.toBool());
    } else {
        // sync the default path into config
        ConfigManager::instance()->setAppAttribute(AppSettings::GenericGroup, AppSettings::ClipboardShareKey, true);

        d->clipShareSwitchBtn->setChecked(true);
    }
}

#ifndef linux
void SettingDialogPrivate::setQComboxWinStyle(QComboBox *combox)
{
    combox->setStyleSheet(
            "QComboBox {"
            "   border: 1px solid rgba(0,0,0,0.1);"
            "   border-radius: 8px;"
            "   padding: 5px;"
            "   background-color: rgba(0,0,0,0.08);"
            "}"
            "QComboBox::down-arrow {"
            "   width: 12px;"
            "   height: 12px;"
            "   image: url(':/icons/deepin/builtin/texts/combobox_arrow.svg');"
            "   subcontrol-origin: padding;"
            "   subcontrol-position: right center;"
            "   right: 10px;"
            "}"
            "QComboBox::drop-down {"
            "   width: 0px;"
            "   border: none;"
            "   padding-right: 0px;"
            "}");
    combox->setFixedHeight(36);
}
#endif
