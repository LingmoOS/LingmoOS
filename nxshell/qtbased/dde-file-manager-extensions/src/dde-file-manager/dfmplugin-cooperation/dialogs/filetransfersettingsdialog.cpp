// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "filetransfersettingsdialog.h"
#include "configs/settings/configmanager.h"
#include "configs/dconfig/dconfigmanager.h"
#include "reportlog/reportlogmanager.h"
#include <DStyle>
#include <DGuiApplicationHelper>

#include <QLabel>
#include <QPainter>
#include <QFileDialog>
#include <QStandardPaths>
#include <QPainterPath>

using namespace dfmplugin_cooperation;
DWIDGET_USE_NAMESPACE

FileChooserEdit::FileChooserEdit(QWidget *parent)
    : QWidget(parent)
{
    initUI();
}

void FileChooserEdit::setText(const QString &text)
{
    QFontMetrics fontMetrices(pathLabel->font());
    QString showName = fontMetrices.elidedText(text, Qt::ElideRight, pathLabel->width() - 16);
    if (showName != text)
        pathLabel->setToolTip(text);

    pathLabel->setText(showName);
}

void FileChooserEdit::initUI()
{
    pathLabel = new QLabel(this);
    pathLabel->setContentsMargins(8, 8, 8, 8);
    pathLabel->setText(QStandardPaths::writableLocation(QStandardPaths::DownloadLocation));

    fileChooserBtn = new DSuggestButton(this);
    fileChooserBtn->setIcon(DStyleHelper(style()).standardIcon(DStyle::SP_SelectElement, nullptr));
    fileChooserBtn->setFixedSize(36, 36);
    connect(fileChooserBtn, &DSuggestButton::clicked, this, &FileChooserEdit::onButtonClicked);

    QHBoxLayout *mainLayout = new QHBoxLayout;
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(10);
    setLayout(mainLayout);

    mainLayout->addWidget(pathLabel);
    mainLayout->addWidget(fileChooserBtn);
}

void FileChooserEdit::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);

    QColor color(0, 0, 0, static_cast<int>(255 * 0.08));
    if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::DarkType) {
        color = QColor("#444444");
    }

    painter.setBrush(color);
    painter.drawRoundedRect(pathLabel->rect(), 8, 8);

    QWidget::paintEvent(event);
}

void FileChooserEdit::onButtonClicked()
{
    auto dirPath = QFileDialog::getExistingDirectory(this);
    if (dirPath.isEmpty())
        return;

    setText(dirPath);
    emit fileChoosed(dirPath);
}

BackgroundWidget::BackgroundWidget(QWidget *parent)
    : QFrame(parent)
{
}

void BackgroundWidget::setRoundRole(BackgroundWidget::RoundRole role)
{
    this->role = role;
}

void BackgroundWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    const int radius = 8;
    QRect paintRect = this->rect();
    QPainterPath path;

    switch (role) {
    case Top:
        path.moveTo(paintRect.bottomRight());
        path.lineTo(paintRect.topRight() + QPoint(0, radius));
        path.arcTo(QRect(QPoint(paintRect.topRight() - QPoint(radius * 2, 0)),
                         QSize(radius * 2, radius * 2)),
                   0, 90);
        path.lineTo(paintRect.topLeft() + QPoint(radius, 0));
        path.arcTo(QRect(QPoint(paintRect.topLeft()), QSize(radius * 2, radius * 2)), 90, 90);
        path.lineTo(paintRect.bottomLeft());
        path.lineTo(paintRect.bottomRight());
        break;
    case Bottom:
        path.moveTo(paintRect.bottomRight() - QPoint(0, radius));
        path.lineTo(paintRect.topRight());
        path.lineTo(paintRect.topLeft());
        path.lineTo(paintRect.bottomLeft() - QPoint(0, radius));
        path.arcTo(QRect(QPoint(paintRect.bottomLeft() - QPoint(0, radius * 2)),
                         QSize(radius * 2, radius * 2)),
                   180, 90);
        path.lineTo(paintRect.bottomLeft() + QPoint(radius, 0));
        path.arcTo(QRect(QPoint(paintRect.bottomRight() - QPoint(radius * 2, radius * 2)),
                         QSize(radius * 2, radius * 2)),
                   270, 90);
        break;
    default:
        break;
    }

    QColor color = DGuiApplicationHelper::instance()->applicationPalette().frameBorder().color();
    if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::DarkType) {
        color = QColor("#323232");
        color.setAlpha(230);
    }

    painter.fillPath(path, color);
    QFrame::paintEvent(event);
}

FileTransferSettingsDialog::FileTransferSettingsDialog(QWidget *parent)
    : DDialog(parent)
{
    initUI();
    initConnect();
}

void FileTransferSettingsDialog::initUI()
{
    setIcon(QIcon::fromTheme("dde-file-manager"));
    setTitle(tr("File transfer settings"));
    setFixedWidth(400);
    setContentsMargins(0, 0, 0, 0);

    QWidget *contentWidget = new QWidget(this);
    mainLayout = new QVBoxLayout;
    mainLayout->setContentsMargins(0, 10, 0, 10);
    mainLayout->setSpacing(1);
    contentWidget->setLayout(mainLayout);
    addContent(contentWidget);

    fileChooserEdit = new FileChooserEdit(this);

    comBox = new DComboBox(this);
    QStringList items { tr("Everyone in the same LAN"),
                        tr("Only those who are collaborating are allowed"),
                        tr("Not allow") };
    comBox->addItems(items);
    comBox->setFocusPolicy(Qt::NoFocus);

    addItem(tr("Allows the following users to send files to me"), comBox, 0);
    addItem(tr("File save location"), fileChooserEdit, 1);
}

void FileTransferSettingsDialog::initConnect()
{
    connect(comBox, qOverload<int>(&DComboBox::currentIndexChanged), this, &FileTransferSettingsDialog::onComBoxValueChanged);
    connect(fileChooserEdit, &FileChooserEdit::fileChoosed, this, &FileTransferSettingsDialog::onFileChoosered);
}

void FileTransferSettingsDialog::loadConfig()
{
#ifdef linux
    auto value = DConfigManager::instance()->value(kDefaultCfgPath, "cooperation.transfer.mode", 0);
    int mode = value.toInt();
    mode = (mode < 0) ? 0 : (mode > 2) ? 2 : mode;
    comBox->setCurrentIndex(mode);
#else
    auto value = ConfigManager::instance()->appAttribute("GenericAttribute", "TransferMode");
    comBox->setCurrentIndex(value.isValid() ? value.toInt() : 0);
#endif

    value = ConfigManager::instance()->appAttribute("GenericAttribute", "StoragePath");
    fileChooserEdit->setText(value.isValid() ? value.toString() : QStandardPaths::writableLocation(QStandardPaths::DownloadLocation));
}

void FileTransferSettingsDialog::addItem(const QString &text, QWidget *widget, int indexPos)
{
    BackgroundWidget *bgWidget = new BackgroundWidget(this);
    switch (indexPos) {
    case 0:
        bgWidget->setRoundRole(BackgroundWidget::Top);
        break;
    case 1:
        bgWidget->setRoundRole(BackgroundWidget::Bottom);
        break;
    default:
        break;
    }

    QVBoxLayout *vLayout = new QVBoxLayout;
    vLayout->setContentsMargins(10, 10, 10, 10);
    vLayout->setSpacing(10);
    bgWidget->setLayout(vLayout);

    QLabel *label = new QLabel(text, this);
    vLayout->addWidget(label);
    vLayout->addWidget(widget);

    mainLayout->addWidget(bgWidget);
}

void FileTransferSettingsDialog::onFileChoosered(const QString &fileName)
{
    ConfigManager::instance()->setAppAttribute("GenericAttribute", "StoragePath", fileName);
}

void FileTransferSettingsDialog::onComBoxValueChanged(int index)
{
#ifdef linux
    DConfigManager::instance()->setValue(kDefaultCfgPath, "cooperation.transfer.mode", index);
    bool status = index == 2 ? false : true;
    QVariantMap data;
    data.insert("enableFileDelivery", status);
    deepin_cross::ReportLogManager::instance()->commit("CooperationStatus", data);
#else
    ConfigManager::instance()->setAppAttribute("GenericAttribute", "TransferMode", index);
#endif
}

void FileTransferSettingsDialog::showEvent(QShowEvent *e)
{
    loadConfig();
    DDialog::showEvent(e);
}
