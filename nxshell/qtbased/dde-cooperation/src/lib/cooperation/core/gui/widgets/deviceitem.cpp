// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "deviceitem.h"
#include "buttonboxwidget.h"
#include "gui/utils/cooperationguihelper.h"

#ifdef linux
#    include <DPalette>
#endif
#ifdef DTKWIDGET_CLASS_DSizeMode
#    include <DSizeMode>
DWIDGET_USE_NAMESPACE
#endif

#include <QIcon>
#include <QVBoxLayout>
#include <QPainter>
#include <QBitmap>

using namespace cooperation_core;

#ifdef __linux__
static constexpr char Kcomputer_connected[] = "computer_connected";
static constexpr char Kcomputer_can_connect[] = "computer_can_connect";
static constexpr char Kcomputer_off_line[] = "computer_off_line";
#else
const char *Kcomputer_connected = ":/icons/deepin/builtin/icons/computer_connected_52px.svg";
const char *Kcomputer_can_connect = ":/icons/deepin/builtin/icons/computer_can_connect_52px.svg";
const char *Kcomputer_off_line = ":/icons/deepin/builtin/icons/computer_off_line_52px.svg";
#endif

StateLabel::StateLabel(QWidget *parent)
    : CooperationLabel(parent)
{
}

void StateLabel::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);

    QColor brushColor;
    QColor textColor;
    switch (st) {
    case DeviceInfo::Connected:
        brushColor.setRgb(241, 255, 243);
        textColor.setRgb(51, 202, 78);
        if (CooperationGuiHelper::isDarkTheme()) {
            brushColor.setRgb(63, 70, 64, static_cast<int>(255 * 0.2));
            textColor.setRgb(67, 159, 83);
        }
        break;
    case DeviceInfo::Connectable:
        brushColor.setRgb(56, 127, 247, 22);
        textColor.setRgb(0, 130, 250);
        if (CooperationGuiHelper::isDarkTheme()) {
            brushColor.setRgb(26, 84, 182, static_cast<int>(255 * 0.2));
            textColor.setRgb(0, 105, 202);
        }
        break;
    case DeviceInfo::Offline:
    default:
        brushColor.setRgb(0, 0, 0, 25);
        textColor.setRgb(0, 0, 0, 128);
        if (CooperationGuiHelper::isDarkTheme()) {
            brushColor.setRgb(255, 255, 255, static_cast<int>(255 * 0.05));
            textColor.setRgb(255, 255, 255, static_cast<int>(255 * 0.4));
        }
        break;
    }

    painter.setBrush(brushColor);
    painter.drawRoundedRect(rect(), 8, 8);

    painter.setPen(textColor);
    painter.drawText(rect(), Qt::AlignCenter, text());
}

DeviceItem::DeviceItem(QWidget *parent)
    : BackgroundWidget(parent)
{
    initUI();
    initConnect();
}

DeviceItem::~DeviceItem()
{
}

void DeviceItem::setDeviceInfo(const DeviceInfoPointer info)
{
    devInfo = info;
    setDeviceName(info->deviceName());
    setDeviceStatus(info->connectStatus());
    ipLabel->setText(info->ipAddress());

    update();
    updateOperations();
}

DeviceInfoPointer DeviceItem::deviceInfo() const
{
    return devInfo;
}

void DeviceItem::initUI()
{
    setFixedSize(480, qApp->property("onlyTransfer").toBool() ? 72 : 90);
    setBackground(8, NoType, TopAndBottom);

    iconLabel = new CooperationLabel(this);
    nameLabel = new CooperationLabel(this);
    nameLabel->installEventFilter(this);
    CooperationGuiHelper::setAutoFont(nameLabel, 14, QFont::Medium);

    ipLabel = new CooperationLabel(this);
    CooperationGuiHelper::setAutoFont(ipLabel, 12, QFont::Medium);
#ifdef linux
    ipLabel->setForegroundRole(DTK_GUI_NAMESPACE::DPalette::TextTips);
#endif

    stateLabel = new StateLabel();
    stateLabel->setContentsMargins(8, 2, 8, 2);
    CooperationGuiHelper::setAutoFont(stateLabel, 11, QFont::Medium);

    QVBoxLayout *vLayout = new QVBoxLayout;
    vLayout->setSpacing(2);
    vLayout->setContentsMargins(0, 10, 0, 10);
    vLayout->addWidget(nameLabel);
    vLayout->addWidget(ipLabel);

    QHBoxLayout *hLayout = new QHBoxLayout;
    hLayout->setContentsMargins(0, 0, 0, 0);
    if (!qApp->property("onlyTransfer").toBool())
        hLayout->addWidget(stateLabel);
    hLayout->addSpacerItem(new QSpacerItem(1, 1, QSizePolicy::Expanding));
    vLayout->addLayout(hLayout);

    btnBoxWidget = new ButtonBoxWidget(this);

    QHBoxLayout *mainLayout = new QHBoxLayout;
    mainLayout->setContentsMargins(10, 0, 10, 0);
    mainLayout->addWidget(iconLabel, 0, Qt::AlignLeft | Qt::AlignVCenter);
    mainLayout->addLayout(vLayout, 0);
    mainLayout->addWidget(btnBoxWidget, 0, Qt::AlignRight);
    setLayout(mainLayout);
}

void DeviceItem::initConnect()
{
    connect(btnBoxWidget, &ButtonBoxWidget::buttonClicked, this, &DeviceItem::onButtonClicked);
}

void DeviceItem::setDeviceName(const QString &name)
{
    QFontMetrics fm(nameLabel->font());
    auto showName = fm.elidedText(name, Qt::ElideMiddle, 385);

    nameLabel->setText(showName);
    if (showName != name)
        nameLabel->setToolTip(name);
}

void DeviceItem::setDeviceStatus(DeviceInfo::ConnectStatus status)
{
    stateLabel->setState(status);
    switch (status) {
    case DeviceInfo::Connected: {
        QIcon icon = QIcon::fromTheme(Kcomputer_connected);
        iconLabel->setPixmap(icon.pixmap(52, 52));
        stateLabel->setText(tr("connected"));
    } break;
    case DeviceInfo::Connectable: {
        QIcon icon = QIcon::fromTheme(Kcomputer_can_connect);
        iconLabel->setPixmap(icon.pixmap(52, 52));
        stateLabel->setText(tr("connectable"));
    } break;
    case DeviceInfo::Offline:
    default: {
        QIcon icon = QIcon::fromTheme(Kcomputer_off_line);
        iconLabel->setPixmap(icon.pixmap(52, 52));
        stateLabel->setText(tr("offline"));
    } break;
    }
}

void DeviceItem::setOperations(const QList<Operation> &operations)
{
    auto tmpOperaList = operations;
    tmpOperaList << indexOperaMap.values();

    std::sort(tmpOperaList.begin(), tmpOperaList.end(),
            [](const Operation &op1, const Operation &op2) {
                  if (op1.location < op2.location)
                      return true;

                  return false;
            });

    for (auto op : tmpOperaList) {
        int index = btnBoxWidget->addButton(QIcon::fromTheme(op.icon), op.description,
                                            static_cast<ButtonBoxWidget::ButtonStyle>(op.style));
        indexOperaMap.insert(index, op);
    }
}

void DeviceItem::updateOperations()
{
    auto iter = indexOperaMap.begin();
    for (; iter != indexOperaMap.end(); ++iter) {
        if (!iter.value().visibleCb)
            continue;

        bool visible = iter.value().visibleCb(iter.value().id, devInfo);
        btnBoxWidget->setButtonVisible(iter.key(), visible);

        if (!iter.value().clickableCb)
            continue;

        bool clickable = iter.value().clickableCb(iter.value().id, devInfo);
        btnBoxWidget->setButtonClickable(iter.key(), clickable);
    }
}

void DeviceItem::onButtonClicked(int index)
{
    if (!indexOperaMap.contains(index))
        return;

    if (indexOperaMap[index].clickedCb)
        indexOperaMap[index].clickedCb(indexOperaMap[index].id, devInfo);

    updateOperations();
}

void DeviceItem::enterEvent(EnterEvent *event)
{
    updateOperations();
    btnBoxWidget->setVisible(true);
    BackgroundWidget::enterEvent(event);
}

void DeviceItem::leaveEvent(QEvent *event)
{
    btnBoxWidget->setVisible(false);
    BackgroundWidget::leaveEvent(event);
}

void DeviceItem::showEvent(QShowEvent *event)
{
    if (hasFocus()) {
        updateOperations();
    } else {
        btnBoxWidget->setVisible(false);
    }

    BackgroundWidget::showEvent(event);
}

bool DeviceItem::eventFilter(QObject *watched, QEvent *event)
{
    // 设备名的蒙版效果，采用字体渐变色实现
    if (watched == nameLabel && event->type() == QEvent::Paint && btnBoxWidget->isVisible()) {
        QPainter painter(nameLabel);
        QLinearGradient lg(nameLabel->rect().topLeft(), nameLabel->rect().bottomRight());
        lg.setColorAt(0.8, nameLabel->palette().windowText().color());
        lg.setColorAt(1, nameLabel->palette().window().color());
        painter.setPen(QPen(lg, nameLabel->font().weight()));

        painter.drawText(nameLabel->rect(), static_cast<int>(nameLabel->alignment()), nameLabel->text());
        return true;
    }

    return BackgroundWidget::eventFilter(watched, event);
}
