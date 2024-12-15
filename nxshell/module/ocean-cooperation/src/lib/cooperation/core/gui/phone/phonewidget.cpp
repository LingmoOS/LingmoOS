// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "phonewidget.h"
#include "gui/widgets/cooperationstatewidget.h"
#include "gui/widgets/devicelistwidget.h"
#include "common/commonutils.h"

#include <QMouseEvent>
#include <QTimer>
#include <QToolButton>
#include <QToolTip>
#include <QFile>
#include <QStackedLayout>
#include <QDesktopServices>

#include <qrencode.h>
#include <gui/utils/cooperationguihelper.h>

using namespace cooperation_core;

PhoneWidget::PhoneWidget(QWidget *parent)
    : QWidget(parent)
{
    initUI();
}

void PhoneWidget::initUI()
{
    stackedLayout = new QStackedLayout;

    nnWidget = new NoNetworkWidget(this);
    dlWidget = new DeviceListWidget(this);
    qrcodeWidget = new QRCodeWidget(this);
    dlWidget->setContentsMargins(10, 0, 10, 0);

    stackedLayout->addWidget(qrcodeWidget);
    stackedLayout->addWidget(dlWidget);
    stackedLayout->addWidget(nnWidget);
    stackedLayout->setCurrentIndex(0);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setSpacing(0);
    mainLayout->addSpacing(10);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addLayout(stackedLayout);
    setLayout(mainLayout);
}

void PhoneWidget::setDeviceInfo(const DeviceInfoPointer info)
{
    switchWidget(PageName::kDeviceListWidget);
    dlWidget->clear();
    dlWidget->appendItem(info);
}

void PhoneWidget::addOperation(const QVariantMap &map)
{
    dlWidget->addItemOperation(map);
}

void PhoneWidget::onSetQRcodeInfo(const QString &info)
{
    qrcodeWidget->setQRcodeInfo(info);
}

void PhoneWidget::switchWidget(PageName page)
{
    if (stackedLayout->currentIndex() == page || page == kUnknownPage)
        return;

    stackedLayout->setCurrentIndex(page);
}

QRCodeWidget::QRCodeWidget(QWidget *parent)
    : QWidget(parent)
{
    initUI();
}

void QRCodeWidget::initUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout();

    QLabel *title = new QLabel(tr("Scan code connection"), this);
    title->setAlignment(Qt::AlignCenter);
    QString lightStyle = "font-weight: bold; font-size: 20px; color:rgba(0, 0, 0, 0.85);";
    QString darkStyle = "font-weight: bold; font-size: 20px; color:rgba(255, 255, 255, 0.85);";
    CooperationGuiHelper::initThemeTypeConnect(title, lightStyle, darkStyle);

    QLabel *instruction = new QLabel(tr("Please use the cross end collaboration app to scan the code"), this);
    instruction->setAlignment(Qt::AlignCenter);
    lightStyle = "font-weight: 400; font-size: 14px; color:rgba(0, 0, 0, 0.7);";
    darkStyle = "font-weight: 400; font-size: 14px; color:rgba(255, 255, 255, 0.7);";
    CooperationGuiHelper::initThemeTypeConnect(instruction, lightStyle, darkStyle);

    QLabel *instruction2 = new QLabel(tr("Mobile phones and devices need to be connected to the same local area network"), this);
    instruction2->setAlignment(Qt::AlignCenter);
    lightStyle = "font-weight: 400; font-size: 12px; color:rgba(0, 0, 0, 0.6);";
    darkStyle = "font-weight: 400; font-size: 12px; color:rgba(255, 255, 255, 0.6);";
    CooperationGuiHelper::initThemeTypeConnect(instruction2, lightStyle, darkStyle);

    QFrame *qrFrame = new QFrame(this);
    lightStyle = "background-color: rgba(0, 0, 0, 0.05); border-radius: 18px;";
    darkStyle = "background-color: rgba(255, 255, 255, 0.1); border-radius: 18px;";
    CooperationGuiHelper::initThemeTypeConnect(qrFrame, lightStyle, darkStyle);

    qrFrame->setLayout(new QVBoxLayout);
    qrFrame->setFixedSize(200, 200);

    qrCode = new QLabel(qrFrame);
    QPixmap qrImage = generateQRCode("", 7);
    qrCode->setPixmap(qrImage);
    qrCode->setAlignment(Qt::AlignCenter);
    qrCode->setStyleSheet("background-color : white;border-radius: 10px;");
    qrCode->setFixedSize(185, 185);

    auto qricon = new QLabel(qrCode);
    QIcon icon(":/icons/lingmo/builtin/icons/uos_assistant@3x.png");
    qricon->setPixmap(icon.pixmap(50, 50));
    qricon->setGeometry(67, 67, 50, 50);

    qrFrame->layout()->setAlignment(Qt::AlignCenter);
    qrFrame->layout()->addWidget(qrCode);

    QHBoxLayout *hLayout = new QHBoxLayout();
    hLayout->addWidget(qrFrame);
    hLayout->setAlignment(Qt::AlignCenter);

    QString hypertext = tr("Click to download UOS assistant APP");
    QString hyperlink = "https://www.chinauos.com/resource/assistant";

    QString websiteLinkTemplate =
        "<br/><a href='%1' style='text-decoration: none; color: #0081FF;word-wrap: break-word;'>%2</a>";
    QString content1 = websiteLinkTemplate.arg(hyperlink, hypertext);

    CooperationLabel *linkLable1 = new CooperationLabel(this);
    linkLable1->setWordWrap(false);
    linkLable1->setAlignment(Qt::AlignCenter);
    linkLable1->setText(content1);
    connect(linkLable1, &QLabel::linkActivated, this, [](const QString &link) {
        QDesktopServices::openUrl(QUrl(link));
    });

    mainLayout->setSpacing(5);
    mainLayout->addSpacing(80);
    mainLayout->addWidget(title);
    mainLayout->addSpacing(2);
    mainLayout->addWidget(instruction);
    mainLayout->addWidget(instruction2);
    mainLayout->addSpacing(20);
    mainLayout->addLayout(hLayout);
    mainLayout->addSpacing(120);
    mainLayout->addWidget(linkLable1);
    mainLayout->addSpacing(120);
    setLayout(mainLayout);
}

void QRCodeWidget::setQRcodeInfo(const QString &info)
{
    QPixmap qrImage = generateQRCode(info, 7);
    qrCode->setPixmap(qrImage);
}

QPixmap QRCodeWidget::generateQRCode(const QString &text, int scale)
{
    // 创建二维码对象，scale 参数控制二维码的大小
    QRcode *qrcode = QRcode_encodeString(text.toStdString().c_str(), 0, QR_ECLEVEL_H, QR_MODE_8, scale);

    if (!qrcode) {
        return QPixmap();   // 处理编码失败情况
    }

    // 计算二维码的实际大小，加入边框
    int size = (qrcode->width) * scale;   // 加上边框
    QImage image(size, size, QImage::Format_RGB32);
    image.fill(Qt::white);   // 填充白色背景

    // 在二维码中间绘制一个 80x80 的空白区域
    int iconSize = 80;   // 空白区域的尺寸
    int iconX = (size - iconSize) / 2;   // 图标X坐标
    int iconY = (size - iconSize) / 2;   // 图标Y坐标
    QRect iconRect(iconX, iconY, iconSize, iconSize);

    // 绘制白色矩形，留空区域
    QPainter painter(&image);
    painter.fillRect(iconRect, Qt::white);

    // 将二维码数据绘制到 QImage
    for (int i = 0; i < qrcode->width; i++) {
        for (int j = 0; j < qrcode->width; j++) {
            if (qrcode->data[j * qrcode->width + i] & 0x1) {
                // 设定黑色像素区域
                for (int x = 0; x < scale; ++x) {
                    for (int y = 0; y < scale; ++y) {
                        // 判断当前绘制的黑色像素是否在空白区域内
                        if (!(iconX <= i * scale + x && i * scale + x < iconX + iconSize && iconY <= j * scale + y && j * scale + y < iconY + iconSize)) {
                            int pixelX = i * scale + x + 1;
                            int pixelY = j * scale + y + 1;
                            // 确保坐标在图像范围内
                            if (pixelX >= 0 && pixelX < image.width() && pixelY >= 0 && pixelY < image.height())
                                image.setPixel(pixelX, pixelY, qRgb(0, 0, 0));   // 设置为黑色像素
                        }
                    }
                }
            }
        }
    }

    // 删除二维码对象
    QRcode_free(qrcode);

    // 转换为 QPixmap 并返回
    return QPixmap::fromImage(image).scaled(170, 170, Qt::KeepAspectRatio);
}
