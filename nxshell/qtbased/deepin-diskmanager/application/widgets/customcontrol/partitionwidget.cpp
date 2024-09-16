// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-only


#include "partitionwidget.h"
#include "common.h"
#include "passwordinputdialog.h"
#include "widgets/messagebox.h"

#include <DScrollArea>

#include <QDebug>
#include <QApplication>

PartitionWidget::PartitionWidget(QWidget *parent)
    : DDialog(parent)
{
    //    setAttribute(Qt::WA_DeleteOnClose, true);
    initUi();
    recPartitionInfo();
    initConnection();
}
PartitionWidget::~PartitionWidget()
{
}
void PartitionWidget::initUi()
{
    setModal(true);
    setFixedSize(900, 600);
    m_mainFrame = new QWidget(this);

    QVBoxLayout *mainLayout = new QVBoxLayout(m_mainFrame);
    mainLayout->setSpacing(5);

    PartitionInfo info = DMDbusHandler::instance()->getCurPartititonInfo();
    setTitle(tr("Partitioning %1").arg(info.m_path));

    DLabel *tipLabel = new DLabel(tr("Click %1 to increase the number of partitions. Click on each partition to change its name and file system.").arg("+"), m_mainFrame);
    tipLabel->setWordWrap(false);
    tipLabel->setElideMode(Qt::ElideMiddle);
    tipLabel->setAlignment(Qt::AlignCenter);

    if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::LightType) {
        DPalette tipPalette;
        QColor tipColor("#000000");
        tipColor.setAlphaF(0.7);
        tipPalette.setColor(DPalette::WindowText, tipColor);
        tipLabel->setPalette(tipPalette);
    } else if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::DarkType) {
        DPalette tipPalette;
        QColor tipColor("#FFFFFF");
        tipColor.setAlphaF(0.7);
        tipPalette.setColor(DPalette::WindowText, tipColor);
        tipLabel->setPalette(tipPalette);
    }

    m_topFrame = new DFrame(m_mainFrame);
    m_topFrame->setLineWidth(0);
    //分区页最上端的布局等
    topFrameSetting();
    m_topFrame->setFrameRounded(true);
    DPalette palette = DApplicationHelper::instance()->palette(m_topFrame);
    palette.setBrush(DPalette::Base, palette.itemBackground());
    DApplicationHelper::instance()->setPalette(m_topFrame, palette);
    m_topFrame->setAutoFillBackground(true);
    //    m_topFrame->setStyleSheet("background:red");
    m_topFrame->setFixedSize(864, 110);

    m_midFrame = new DFrame(m_mainFrame);
    //分区页中间部分的图形绘制
    midFrameSetting();
    m_midFrame->setFrameStyle(DFrame::NoFrame);
    //    m_midFrame->setStyleSheet("background:blue");
    m_midFrame->setFixedSize(864, 85);
    m_botFrame = new DFrame(m_mainFrame);
    //    m_botFrame->setStyleSheet("background:green");
    m_botFrame->setFixedSize(864, 253);
    //分区页最下端的布局等
    botFrameSetting();
    m_botFrame->setFrameStyle(DFrame::NoFrame);
    mainLayout->addWidget(tipLabel, 1);
    mainLayout->addSpacing(10);
    mainLayout->addWidget(m_topFrame, 1);
    mainLayout->addSpacing(10);
    mainLayout->addWidget(m_midFrame, 3);
    mainLayout->addWidget(m_botFrame, 10);
    mainLayout->setSpacing(0);

    setIcon(QIcon::fromTheme(appName));
    addContent(m_mainFrame);
}

void PartitionWidget::topFrameSetting()
{
    //整体水平布局
    QHBoxLayout *hLayout = new QHBoxLayout(m_topFrame);
    hLayout->setSpacing(5);
    hLayout->setContentsMargins(30, 0, 0, 0);
    DLabel *picLabel = new DLabel(m_topFrame);
    picLabel->setPixmap(Common::getIcon("disk").pixmap(85, 85));
    picLabel->setMinimumSize(85, 85);
    //垂直布局-右侧三行标签
    QVBoxLayout *vLayout = new QVBoxLayout();
    vLayout->setContentsMargins(10, 20, 0, 20);
    DLabel *deviceInfoLabel = new DLabel(tr("Disk Information"), m_topFrame);
    DFontSizeManager::instance()->bind(deviceInfoLabel, DFontSizeManager::T6, QFont::Medium);
    QPalette devicePalette;
    devicePalette.setColor(QPalette::WindowText, QColor("#414D68"));
    deviceInfoLabel->setPalette(devicePalette);

    //第一行
    QHBoxLayout *line1Layout = new QHBoxLayout();
    line1Layout->addWidget(deviceInfoLabel);
    line1Layout->addStretch();
    //第二行
    QPalette infoPalette;
    infoPalette.setColor(QPalette::WindowText, QColor("#001A2E"));

    DLabel *allMemoryLabel = new DLabel(tr("Capacity:"), m_topFrame);
    DFontSizeManager::instance()->bind(allMemoryLabel, DFontSizeManager::T8, QFont::Normal);
    allMemoryLabel->setPalette(infoPalette);

    m_allMemory = new DLabel("256GiB", m_topFrame);
    m_allMemory->setMinimumWidth(90);
    m_allMemory->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    DFontSizeManager::instance()->bind(m_allMemory, DFontSizeManager::T8, QFont::Normal);
    m_allMemory->setPalette(infoPalette);

    DLabel *selectedPartLabel = new DLabel(tr("Partition selected:"), m_topFrame);
    DFontSizeManager::instance()->bind(selectedPartLabel, DFontSizeManager::T8, QFont::Normal);
    selectedPartLabel->setPalette(infoPalette);

    m_selectedPartition = new DLabel("sda3");
    DFontSizeManager::instance()->bind(m_selectedPartition, DFontSizeManager::T8, QFont::Normal);
    m_selectedPartition->setPalette(infoPalette);

    //第三行
    m_deviceNameLabel = new DLabel(tr("Disk:"), m_topFrame);
    DFontSizeManager::instance()->bind(m_deviceNameLabel, DFontSizeManager::T8, QFont::Normal);
    m_deviceNameLabel->setPalette(infoPalette);
    QFontMetrics fm(m_deviceNameLabel->font());
    m_deviceNameLabel->setFixedWidth(fm.boundingRect(tr("Disk:")).width() + 5);

    m_deviceName = new DLabel("/dev/sda", m_topFrame);
    //    m_deviceName->setMinimumWidth(103);
    m_deviceName->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    DFontSizeManager::instance()->bind(m_deviceName, DFontSizeManager::T8, QFont::Normal);
    m_deviceName->setPalette(infoPalette);

    DLabel *deviceFormateLabel = new DLabel(tr("File system:"), m_topFrame);
    DFontSizeManager::instance()->bind(deviceFormateLabel, DFontSizeManager::T8, QFont::Normal);
    deviceFormateLabel->setPalette(infoPalette);

    m_deviceFormate = new DLabel("EXT3", m_topFrame);
    DFontSizeManager::instance()->bind(m_deviceFormate, DFontSizeManager::T8, QFont::Normal);
    m_deviceFormate->setPalette(infoPalette);

    QHBoxLayout *line2Layout = new QHBoxLayout();
    line2Layout->addWidget(allMemoryLabel);
    line2Layout->addWidget(m_allMemory);
    line2Layout->setContentsMargins(0, 0, 0, 0);

    QHBoxLayout *line3Layout = new QHBoxLayout();
    line3Layout->addWidget(selectedPartLabel);
    line3Layout->addWidget(m_selectedPartition);
    line3Layout->addStretch();
    line3Layout->setContentsMargins(0, 0, 0, 0);

    QHBoxLayout *line4Layout = new QHBoxLayout();
    line4Layout->addWidget(m_deviceNameLabel);
    line4Layout->addWidget(m_deviceName);
    line4Layout->setContentsMargins(0, 0, 0, 0);

    QHBoxLayout *line5Layout = new QHBoxLayout();
    line5Layout->addWidget(deviceFormateLabel);
    line5Layout->addWidget(m_deviceFormate);
    line5Layout->addStretch();
    line5Layout->setContentsMargins(0, 0, 0, 0);

    QVBoxLayout *line6Layout = new QVBoxLayout();
    line6Layout->addLayout(line2Layout);
    line6Layout->addLayout(line4Layout);
    line6Layout->setContentsMargins(0, 0, 0, 0);

    QVBoxLayout *line7Layout = new QVBoxLayout();
    line7Layout->addLayout(line3Layout);
    line7Layout->addLayout(line5Layout);
    line7Layout->setContentsMargins(0, 0, 0, 0);

    QHBoxLayout *line8Layout = new QHBoxLayout();
    line8Layout->addLayout(line6Layout);
    line8Layout->addSpacing(50);
    line8Layout->addLayout(line7Layout);
    line8Layout->setContentsMargins(0, 0, 0, 0);

    vLayout->addLayout(line1Layout);
    vLayout->addLayout(line8Layout);
    vLayout->setContentsMargins(0, 10, 0, 15);

    hLayout->addWidget(picLabel);
    hLayout->addLayout(vLayout, 10);
    hLayout->setContentsMargins(20, 0, 0, 0);
}

void PartitionWidget::midFrameSetting()
{
    //调用绘制的图形
    m_midFrame->setMinimumHeight(85);
    QVBoxLayout *mainLayout = new QVBoxLayout(m_midFrame);
    m_partChartWidget = new PartChartShowing(m_midFrame);
    mainLayout->addWidget(m_partChartWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
}

void PartitionWidget::botFrameSetting()
{
    QVBoxLayout *vLayout = new QVBoxLayout(m_botFrame);
    m_partWidget = new QWidget(m_botFrame);
    //分区详细页
    partInfoShowing();
    vLayout->addWidget(m_partWidget, 5);
    //按钮
    m_applyBtn = new DPushButton(tr("Confirm", "button"), m_botFrame);
    m_applyBtn->setObjectName("confirm");
    m_applyBtn->setAccessibleName("confirm");
    //取消
    m_cancleBtn = new DPushButton(tr("Cancel", "button"), m_botFrame);
    m_cancleBtn->setAccessibleName("cancel");
    //复原
    m_reveBtn = new DPushButton(tr("Revert", "button"), m_botFrame);
    m_reveBtn->setObjectName("revert");
    m_reveBtn->setAccessibleName("revert");
    m_applyBtn->setEnabled(false);
    m_applyBtn->setMinimumWidth(170);
    m_cancleBtn->setMinimumWidth(170);
    m_reveBtn->setMinimumWidth(170);
    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->addWidget(m_reveBtn, 1, Qt::AlignLeft);
    btnLayout->addWidget(m_cancleBtn);
    btnLayout->addSpacing(10);
    btnLayout->addWidget(m_applyBtn);
    btnLayout->setSpacing(0);
    btnLayout->setContentsMargins(0, 0, 0, 10);

    vLayout->addLayout(btnLayout, 1);
    vLayout->setContentsMargins(0, 0, 0, 0);
}

void PartitionWidget::partInfoShowing()
{
    auto formateList = DMDbusHandler::instance()->getAllSupportFileSystem();
    formateList.removeOne("linux-swap");
    if (!DMDbusHandler::instance()->getIsSystemDisk(DMDbusHandler::instance()->getCurPartititonInfo().m_devicePath)) {
        formateList = DMDbusHandler::instance()->getEncryptionFormate(formateList);
    }

    //整体垂直布局-三行
    QVBoxLayout *vLayout = new QVBoxLayout(m_partWidget);
    //第一行
    m_partInfoLabel = new DLabel(tr("Partition Information"), m_partWidget);
    DFontSizeManager::instance()->bind(m_partInfoLabel, DFontSizeManager::T6, QFont::Medium);
    QPalette partPalette;
    partPalette.setColor(QPalette::WindowText, QColor("#414D68"));
    m_partInfoLabel->setPalette(partPalette);

    //第二行
    QPalette infoPalette;
    infoPalette.setColor(QPalette::WindowText, QColor("#001A2E"));

    QHBoxLayout *line2Layout = new QHBoxLayout();
    line2Layout->setContentsMargins(0, 0, 0, 0);
    m_partDoLabel = new DLabel(tr("Number of partitions:"), m_partWidget);
    DFontSizeManager::instance()->bind(m_partDoLabel, DFontSizeManager::T8, QFont::Normal);
    m_partDoLabel->setPalette(infoPalette);

    DLabel *labelSpace = new DLabel(m_partWidget);
    m_addButton = new DIconButton(DStyle::SP_IncreaseElement, m_partWidget);
    m_addButton->setObjectName("addButton");
    m_addButton->setAccessibleName("addButton");
    m_remButton = new DIconButton(DStyle::SP_DecreaseElement, m_partWidget);
    m_remButton->setObjectName("removeButton");
    m_remButton->setAccessibleName("removeButton");
    m_remButton->setToolTip(tr("Delete last partition"));
    DLabel *space = new DLabel(m_partWidget);

    //按钮初始状态
    if (m_sizeInfo.size() == 0) {
        m_remButton->setEnabled(false);
    } else {
        m_remButton->setEnabled(true);
    }

    m_partNameLabel = new DLabel(tr("Name:"), m_partWidget);
    DFontSizeManager::instance()->bind(m_partNameLabel, DFontSizeManager::T8, QFont::Normal);
    m_partNameLabel->setPalette(infoPalette);

    m_partNameEdit = new DLineEdit(m_partWidget);
    m_partNameEdit->lineEdit()->setMaxLength(256);
    m_partNameEdit->setObjectName("partName");
    m_partNameEdit->setAccessibleName("partName");

    line2Layout->addWidget(m_partDoLabel, 1);
    line2Layout->addSpacing(7);
    line2Layout->addWidget(labelSpace, 5);
    line2Layout->addWidget(m_addButton, 1);
    line2Layout->addWidget(m_remButton, 1);
    line2Layout->addWidget(space, 1);
    line2Layout->addWidget(m_partNameLabel, 1);
    line2Layout->addWidget(m_partNameEdit, 7);
    //第三行
    QHBoxLayout *line3Layout = new QHBoxLayout();
    m_partFormateLabel = new DLabel(tr("File system:"), m_partWidget);
    DFontSizeManager::instance()->bind(m_partFormateLabel, DFontSizeManager::T8, QFont::Normal);
    m_partFormateLabel->setPalette(infoPalette);

    m_partFormateCombox = new DComboBox(m_partWidget);
    m_partFormateCombox->addItems(formateList);
    m_partFormateCombox->setCurrentText("ext4");
    m_partFormateCombox->setAccessibleName("File system");

    DLabel *partSizeLabel = new DLabel(tr("Size:"), m_partWidget);
    DFontSizeManager::instance()->bind(partSizeLabel, DFontSizeManager::T8, QFont::Normal);
    partSizeLabel->setPalette(infoPalette);

    m_slider = new DSlider(Qt::Horizontal);
    m_slider->setMaximum(100);
    m_slider->setValue(100);
    m_slider->setAccessibleName("slider");
    m_partSizeEdit = new DLineEdit(m_partWidget);
    m_partSizeEdit->setObjectName("partSize");
    m_partSizeEdit->setAccessibleName("partSize");
    m_partComboBox = new DComboBox(m_partWidget);
    m_partComboBox->addItem("GiB");
    m_partComboBox->addItem("MiB");
    m_partComboBox->setCurrentText("GiB");
    m_partComboBox->setAccessibleName("unit");

    line3Layout->addWidget(m_partFormateLabel, 1);
    line3Layout->addWidget(m_partFormateCombox, 7);
    line3Layout->addWidget(space, 1);
    line3Layout->addWidget(partSizeLabel);
    line3Layout->addWidget(m_slider, 2);
    line3Layout->addWidget(m_partSizeEdit, 3);
    line3Layout->addWidget(m_partComboBox, 2);

    // 第四行
    DPalette palette;
    palette.setColor(DPalette::Text, QColor("#526a7f"));

    QFont font;
    font.setWeight(QFont::Normal);
    font.setFamily("Source Han Sans");
    font.setPixelSize(12);

    m_emptyLabel = new DLabel(m_partWidget);
    m_emptyLabel->setFixedWidth(m_partFormateLabel->fontMetrics().width(tr("File system:")));
    m_encryptionInfo = new DLabel(m_partWidget);
    m_encryptionInfo->setFont(font);
    m_encryptionInfo->setPalette(palette);
    m_encryptionInfo->adjustSize();
    m_encryptionInfo->setWordWrap(true);
    m_encryptionInfo->setAccessibleName("encryptionInfo");

    m_scrollArea = new DScrollArea;
    m_scrollArea->setFrameShadow(QFrame::Plain);
    m_scrollArea->setFrameShape(QFrame::NoFrame);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff); //隐藏横向滚动条
    m_scrollArea->setWidget(m_encryptionInfo);

    // 设置滚动区域背景颜色为透明
    QPalette areaPalette;
    areaPalette.setColor(QPalette::Base, QColor(255, 255, 255, 0));
    m_scrollArea->setPalette(areaPalette);

    QHBoxLayout *line4Layout = new QHBoxLayout();
    line4Layout->addWidget(m_emptyLabel, 1);
    line4Layout->addWidget(m_scrollArea, 7);
    line4Layout->addStretch();

    vLayout->addWidget(m_partInfoLabel);
    vLayout->addLayout(line2Layout);
    vLayout->addSpacing(4);
    vLayout->addLayout(line3Layout);
    vLayout->addLayout(line4Layout);
    vLayout->addStretch();
    vLayout->setContentsMargins(0, 0, 0, 0);
    //输入框正则表达
    setRegValidator();
}

void PartitionWidget::recPartitionInfo()
{
    //获取数据
    QString diskSize;
    PartitionInfo data;
    auto it = DMDbusHandler::instance()->probDeviceInfo().find(DMDbusHandler::instance()->getCurPartititonInfo().m_devicePath);

    if (it != DMDbusHandler::instance()->probDeviceInfo().end()) {
        diskSize = QString::number(Utils::sectorToUnit(it.value().m_length, it.value().m_sectorSize, SIZE_UNIT::UNIT_GIB), 'f', 2) + "GiB";
        data = DMDbusHandler::instance()->getCurPartititonInfo();
    }

    QString devicePath = data.m_devicePath;
    QString deviceSize = diskSize;
    QString partPath = data.m_path;
    QString partFstype = Utils::fileSystemTypeToString(static_cast<FSType>(data.m_fileSystemType));
    //所选空闲分区
    int i = partPath.lastIndexOf("/");
    QString selectPartition = partPath.right(partPath.length() - i - 1);
    m_selectedPartition->setText(selectPartition);
    m_deviceName->setText(devicePath);
    m_allMemory->setText(deviceSize);
    m_deviceFormate->setText(partFstype);
    m_total = Utils::sectorToUnit(data.m_sectorEnd - data.m_sectorStart + 1, data.m_sectorSize, SIZE_UNIT::UNIT_GIB);
    m_totalSize = m_total * 1024;
    //    qDebug() << mTotal << total;
    //初始值,显示保留两位小数,真正使用保留4位小数
    m_currentEditSize = QString::number(m_totalSize, 'f', 4);
    m_partComboBox->setEnabled(true);
    setSelectUnallocatesSpace();
}

void PartitionWidget::initConnection()
{
    connect(m_slider, &DSlider::valueChanged, this, &PartitionWidget::onSliderValueChanged);
    connect(m_partSizeEdit, &DLineEdit::textChanged, this, &PartitionWidget::onSetSliderValue);
    connect(m_partNameEdit, &DLineEdit::textEdited, this, &PartitionWidget::onSetPartName);
    connect(m_partNameEdit, &DLineEdit::textChanged, this, &PartitionWidget::onTextChanged);
    connect(m_addButton, &DIconButton::clicked, this, &PartitionWidget::onAddPartition);
    connect(m_remButton, &DIconButton::clicked, this, &PartitionWidget::onRemovePartition);
    connect(m_applyBtn, &DPushButton::clicked, this, &PartitionWidget::onApplyButton);
    connect(m_reveBtn, &DPushButton::clicked, this, &PartitionWidget::onRevertButton);
    connect(m_cancleBtn, &DPushButton::clicked, this, &PartitionWidget::onCancelButton);
    //    connect(m_partComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onComboxCurTextChange(int)));
    connect(m_partComboBox, static_cast<void (DComboBox::*)(const int)>(&DComboBox::currentIndexChanged),
            this, &PartitionWidget::onComboxCurTextChange);
    connect(m_partFormateCombox, &DComboBox::currentTextChanged, this, &PartitionWidget::onComboxFormatTextChange);
    connect(m_partChartWidget, &PartChartShowing::sendMoveFlag, this, &PartitionWidget::onShowTip);
    connect(m_partChartWidget, &PartChartShowing::sendFlag, this, &PartitionWidget::onShowSelectPathInfo);
    connect(m_partChartWidget, &PartChartShowing::judgeLastPartition, this, &PartitionWidget::onJudgeLastPartition);
}

double PartitionWidget::sumValue()
{
    double sum = 0.00;

    for (int i = 0; i < m_sizeInfo.count(); i++) {
        sum = sum + m_sizeInfo.at(i);
        if (sum >= m_totalSize - 0.01)
            break;
    }

    return sum;
}

void PartitionWidget::setSelectUnallocatesSpace()
{
    m_partNameEdit->lineEdit()->setPlaceholderText(tr("Unallocated"));
    m_partSizeEdit->setText("");

    if (m_partComboBox->currentText() == "GiB") {
        m_partSizeEdit->setText(QString::number(m_total - (sumValue() / 1024), 'f', 2));
    } else {
        m_partSizeEdit->setText(QString::number(m_totalSize - sumValue(), 'f', 2));
    }
}

void PartitionWidget::setAddOrRemResult(const bool &isExceed)
{
    m_partNameEdit->setText("");
    m_partSizeEdit->setText("");

    if (m_sizeInfo.size() == 0) {
        m_flag = 1;
    } else {
        m_flag = 0;
    }

    m_partChartWidget->transFlag(m_flag, m_value);
    m_slider->setValue(100);
    m_partNameEdit->lineEdit()->setPlaceholderText(tr("Name"));
    m_partSizeEdit->lineEdit()->setPlaceholderText(tr("Size"));

    setEnable(0, isExceed);
    onSliderValueChanged(100);
}

void PartitionWidget::setRegValidator()
{
    QRegExp reg("^[0-9]+(\\.[0-9]{1,4})?$");
    QRegExpValidator *va = new QRegExpValidator(reg, this);
    m_partSizeEdit->lineEdit()->setValidator(va);

    QRegExp re("^[\u4E00-\u9FA5A-Za-z0-9_]+$");
    QRegExpValidator *va1 = new QRegExpValidator(re, this);
    m_partNameEdit->lineEdit()->setValidator(va1);
}

bool PartitionWidget::maxAmountPrimReached()
{
    bool breachMax = false;
    int primaryCount = 0;
    PartitionVec partVector = DMDbusHandler::instance()->getCurDevicePartitionArr();
    PartitionInfo info = DMDbusHandler::instance()->getCurPartititonInfo();

    for (int i = 0; i < partVector.size(); i++) {
        if (partVector[i].m_type == TYPE_PRIMARY || partVector[i].m_type == TYPE_EXTENDED)
            primaryCount++;
    }

    int maxprims = DMDbusHandler::instance()->getCurDeviceInfo().m_maxPrims;
    if (!info.m_insideExtended && primaryCount >= maxprims) {
        breachMax = true;
        //        qDebug() << QString("It is not possible to create more than %1 primary partition").arg(maxprims);
        //        qDebug() << QString("If you want more partitions you should first create an extended partition. Such a partition can contain other partitions."
        //                            "Because an extended partition is also a primary partition it might be necessary to remove a primary partition first.");
    }

    return breachMax;
}

void PartitionWidget::onShowSelectPathInfo(const int &flag, const int &num, const int &posX)
{
    m_flag = flag;
    m_number = num;

    if (m_flag == 0) {
        return;
    }

    if (m_flag == 1 || m_flag == 3) {
        setSelectUnallocatesSpace();
        m_slider->setValue(100);
        m_number = -1;
    } else if (m_flag == 2) {
        if (num < 0) {
            DPalette palette = DApplicationHelper::instance()->palette(m_partNameLabel);
            palette.setBrush(DPalette::Text, palette.placeholderText());
            m_partNameLabel->setPalette(palette);
            m_partNameEdit->setEnabled(false);
            return;
        }

        m_slider->setValue(static_cast<int>(m_sizeInfo.at(num) / m_totalSize * 100));
        m_partSizeEdit->setText("");
        m_partNameEdit->setText("");
        m_partNameEdit->lineEdit()->setPlaceholderText(m_partName.at(num));

        double clicked = m_sizeInfo.at(num);

        if (m_partComboBox->currentText() == "GiB") {
            clicked = clicked / 1024;
        }
        m_partSizeEdit->setText(QString::number(clicked, 'f', 2));
    }

    setEnable(m_flag, m_isExceed);
    onShowTip(flag, num, posX);
}

void PartitionWidget::onShowTip(const int &hover, const int &num, const int &posX)
{
    int x = frameGeometry().x();
    int y = frameGeometry().y();

    if (hover == 2) {
        if (m_partName.at(num) != " ") {
            QToolTip::showText(QPoint(x + posX + 5, y + 215), m_partName.at(num), this, QRect(QPoint(x + posX, y + 215), QSize(80, 20)), 1000);
        }
    } else if (hover == 3 || hover == 1) {
        QToolTip::showText(QPoint(x + posX + 5, y + 215), tr("Unallocated"), this, QRect(QPoint(x + posX, y + 215), QSize(80, 20)), 1000);
    }
}

void PartitionWidget::setEnable(const int &flag, const bool &isExceed)
{
    DPalette palette = DApplicationHelper::instance()->palette(m_partNameLabel);

    if (isExceed) { //还有空闲空间剩余
        if (flag == 2) { //选中新建的分区
            setControlEnable(false);
            m_partNameEdit->setEnabled(false);
        } else {
            setControlEnable(true);
        }
        //        palette.setColor(DPalette::Text, QColor(this->palette().buttonText().color()));
    } else { //无空闲空间剩余
        setControlEnable(false);
        //        if (flag != 2) {
        //            palette.setBrush(DPalette::Text, palette.placeholderText());
        //            m_partNameEdit->setEnabled(false);
        //        } else {
        //            palette.setColor(DPalette::Text, QColor(this->palette().buttonText().color()));
        m_partNameEdit->setEnabled(false);
        //        }
    }

    if (m_partNameEdit->text().toUtf8().size() <= 16) {
        m_partNameEdit->setAlert(false);
        m_partNameEdit->hideAlertMessage();
    } else {
        m_addButton->setEnabled(false);
    }

    m_partNameLabel->setPalette(palette); //各情况下,分区名称label的样式

    m_remButton->setEnabled(m_sizeInfo.size());
    m_applyBtn->setEnabled(m_sizeInfo.size());
}

void PartitionWidget::setControlEnable(const bool &isTrue)
{
    m_addButton->setEnabled(isTrue);
    m_partSizeEdit->setEnabled(isTrue);
    m_slider->setEnabled(isTrue);
    m_partComboBox->setEnabled(isTrue);
    m_partFormateCombox->setEnabled(isTrue);

    m_partNameEdit->setEnabled(true);
    setLabelColor(isTrue);
}

void PartitionWidget::setLabelColor(const bool &isOk)
{
    if (isOk) {
        DPalette framePalette = DApplicationHelper::instance()->palette(m_botFrame);
        framePalette.setColor(DPalette::Text, QColor(palette().buttonText().color()));
        m_botFrame->setPalette(framePalette);
    } else {
        DPalette palette = DApplicationHelper::instance()->palette(m_botFrame);
        palette.setBrush(DPalette::Text, palette.placeholderText());
        m_botFrame->setPalette(palette);
    }

    DPalette palette1 = DApplicationHelper::instance()->palette(m_partInfoLabel);
    palette1.setColor(DPalette::Text, QColor(palette().buttonText().color()));
    m_partInfoLabel->setPalette(palette1);

    DPalette palatte2 = DApplicationHelper::instance()->palette(m_partDoLabel);
    palatte2.setColor(DPalette::Text, QColor(palette().buttonText().color()));
    m_partDoLabel->setPalette(palatte2);
}

void PartitionWidget::onComboxCurTextChange(int index)
{
    //    qDebug() << partSizeEdit->text().toDouble();
    if (!m_partSizeEdit->text().isEmpty()) {
        double m = m_currentEditSize.toDouble();

        if (index == 1) {
            if (m_sizeInfo.size() == 0 && m_slider->value() == 100)
                m_partSizeEdit->setText(QString::number(m_totalSize, 'f', 2));
            else
                m_partSizeEdit->setText(QString::number(m, 'f', 2));
        } else if (index == 0) {
            //            double m = partSizeEdit->text().toDouble();
            if (m_sizeInfo.size() == 0 && m_slider->value() == 100)
                m_partSizeEdit->setText(QString::number(m_total, 'f', 2));
            else
                m_partSizeEdit->setText(QString::number(m / 1024, 'f', 2));
        }
        //        qDebug() << m;
    }
}

void PartitionWidget::onComboxFormatTextChange(const QString &text)
{
    if (m_partSizeEdit->isAlert()) {
        m_partSizeEdit->setAlert(false);
        m_partSizeEdit->hideAlertMessage();
    }

    QByteArray byteArray = m_partNameEdit->text().toUtf8();
    if (text.contains("fat32")) {
        if (byteArray.size() > 11) {
            m_partNameEdit->setAlert(true);
            m_partNameEdit->showAlertMessage(tr("The length exceeds the limit"), -1);
            m_addButton->setEnabled(false);
        } else {
            m_partNameEdit->setAlert(false);
            m_partNameEdit->hideAlertMessage();
            m_addButton->setEnabled(true);
        }
    } else {
        if (byteArray.size() > 16) {
            m_partNameEdit->setAlert(true);
            m_partNameEdit->showAlertMessage(tr("The length exceeds the limit"), -1);
            m_addButton->setEnabled(false);
        } else {
            m_partNameEdit->setAlert(false);
            m_partNameEdit->hideAlertMessage();
            m_addButton->setEnabled(true);
        }
    }

    m_scrollArea->setFixedWidth(m_partFormateCombox->width());
    if (text.contains("AES")) {
        QString text = tr("Use the aes-xts-plain64 standard algorithm to encrypt the disk. "
                          "If it is encrypted, you should decrypt it before mounting.");
        m_encryptionInfo->setFixedSize(m_partFormateCombox->width(),
                                       Common::getLabelAdjustHeight(m_partFormateCombox->width(), text, m_encryptionInfo->font()));
        m_encryptionInfo->setText(text);
    } else if (text.contains("SM4")) {
        QString text = tr("Use the sm4-xts-plain state cryptographic algorithm to encrypt the disk. "
                          "If it is encrypted, you should decrypt it before mounting. "
                          "Operating Systems that do not support the state cryptographic "
                          "algorithm will not be able to decrypt the disk.");
        m_encryptionInfo->setFixedSize(m_partFormateCombox->width(),
                                       Common::getLabelAdjustHeight(m_partFormateCombox->width(), text, m_encryptionInfo->font()) - 1);
        m_encryptionInfo->setText(text);
    } else {
        m_encryptionInfo->setText("");
    }
}

void PartitionWidget::onJudgeLastPartition()
{
    m_slider->setEnabled(false);
    m_partSizeEdit->setEnabled(false);
}

void PartitionWidget::onSliderValueChanged(int value)
{
    m_value = value;
    QString size;

    if (m_block == 0) {
        //选中分区大小与整个空闲分区的占比
        if (m_flag == 2) {
            if (m_partComboBox->currentText() == "MiB") {
                size = QString::number((static_cast<double>(value) / 100) * m_totalSize, 'f', 2);
            } else {
                size = QString::number((static_cast<double>(value) / 100) * m_total, 'f', 2);
            }

            m_partSizeEdit->setText(size);
        } else { //剩余空间为总大小,占比情况
            if (m_partComboBox->currentText() == "MiB") {
                size = QString::number((static_cast<double>(value) / 100) * (m_totalSize - sumValue()), 'f', 2);

                if (m_totalSize - sumValue() < 52) {
                    onJudgeLastPartition();
                }
            } else {
                size = QString::number(((static_cast<double>(value) / 100) * (m_totalSize - sumValue())) / 1024, 'f', 2);

                if (m_total - sumValue() / 1024 < 0.05) {
                    onJudgeLastPartition();
                }
            }

            m_partSizeEdit->setText(size);
        }
    }
    //    QString strSize1 = QString::number((static_cast<double>(value) / 100) * (m_totalSize - sumValue()),  'f', 2);
    m_currentEditSize = QString::number((static_cast<double>(value) / 100) * (m_totalSize - sumValue()), 'f', 4);
    qDebug() << m_currentEditSize << size << value << m_totalSize - sumValue();
    m_block = 0;
    //m_addButton->setEnabled(m_currentEditSize <= )
}

void PartitionWidget::onSetSliderValue()
{
    if (m_partSizeEdit->isAlert()) {
        m_partSizeEdit->setAlert(false);
        m_partSizeEdit->hideAlertMessage();
    }

    double value = 0;
    if (m_partSizeEdit->text().trimmed().isEmpty()) {
        value = 0;
    }

    value = m_partSizeEdit->text().toDouble();
    if (m_partComboBox->currentText() == "MiB")
        value = value / 1024;
    m_block = 1;
    m_slider->setValue(static_cast<int>((value / (m_total - (sumValue() / 1024))) * 100));
    m_currentEditSize = QString::number(value * 1024, 'f', 4);
    if (value == 0.00 || value > (m_total - sumValue() / 1024)) {
        m_addButton->setEnabled(false);
    } else {
        m_addButton->setEnabled(true);
    }
    //    qDebug() << currentEditSize;
    //    qDebug() << value * 1024 << static_cast<int>((value / total) * 100);
}

void PartitionWidget::onTextChanged(const QString &text)
{
    if (!text.isEmpty()) {
        QByteArray byteArray = text.toUtf8();
        if (m_partFormateCombox->currentText().contains("fat32")) {
            if (byteArray.size() > 11) {
                m_partNameEdit->setAlert(true);
                m_partNameEdit->showAlertMessage(tr("The length exceeds the limit"), -1);
                m_addButton->setEnabled(false);
            } else {
                m_partNameEdit->setAlert(false);
                m_partNameEdit->hideAlertMessage();
                m_addButton->setEnabled(true);
            }
        } else {
            if (byteArray.size() > 16) {
                m_partNameEdit->setAlert(true);
                m_partNameEdit->showAlertMessage(tr("The length exceeds the limit"), -1);
                m_addButton->setEnabled(false);
            } else {
                m_partNameEdit->setAlert(false);
                m_partNameEdit->hideAlertMessage();
                m_addButton->setEnabled(true);
            }
        }
    }
}

void PartitionWidget::onSetPartName()
{
    if (!m_partName.isEmpty() && m_number > -1) {
        m_partName.replace(m_number, m_partNameEdit->text());
    }
    qDebug() << m_partNameEdit->text().length();
}

void PartitionWidget::onAddPartition()
{
    DeviceInfo device = DMDbusHandler::instance()->getCurDeviceInfo();

    int partitionCount = 0;
    for (int i = 0; i < device.m_partition.size(); i++) {
        PartitionInfo info = device.m_partition.at(i);
        if (info.m_path != "unallocated") {
            partitionCount++;
        }
    }

    if (m_sizeInfo.size() >= 24 || maxAmountPrimReached() == true || (partitionCount + m_sizeInfo.size()) >= device.m_maxPrims) {
        DMessageManager::instance()->sendMessage(this, QIcon(":/icons/deepin/builtin/warning.svg"), tr("The number of new partitions exceeds the limit"));
        return;
    }

    double currentSize = 0.00;
    stPart part;

    //一次新建不超过24个分区
    if (m_partNameEdit->text().isEmpty()) {
        m_partNameEdit->setText(" ");
    }
    m_partName.append(m_partNameEdit->text());
    //输入框内的值超过剩余空闲空间,以剩余空间新建
    currentSize = m_currentEditSize.toDouble();
    if (currentSize >= QString::number((m_totalSize - sumValue()), 'f', 4).toDouble()) {
        currentSize = m_totalSize - sumValue();
        part.m_blast = true;
    }

    QString formate = m_partFormateCombox->currentText();
    if (formate.contains("AES") || formate.contains("SM4")) {
        if (currentSize <= 100) {
            m_partSizeEdit->setAlert(true);
            m_partSizeEdit->showAlertMessage(tr("To encrypt a partition, it should be larger than 100 MiB"));
            return;
        }

        QColor color = QColor(palette().buttonText().color());

        PasswordInputDialog passwordInputDialog(this);
        passwordInputDialog.setDeviceName(device.m_path);
        PartitionInfo info = DMDbusHandler::instance()->getCurPartititonInfo();
        passwordInputDialog.setTitleText(tr("Set a password to encrypt the new partition"));
        passwordInputDialog.setObjectName("passwordInputDialog");
        passwordInputDialog.setAccessibleName("passwordInputDialog");
        if (passwordInputDialog.exec() != DDialog::Accepted) {
            return;
        } else {
            part.m_password = passwordInputDialog.getPassword();
            part.m_passwordHint = passwordInputDialog.getPasswordHint();
            if (formate.contains("AES")) {
                part.m_encryption = CRYPT_CIPHER::AES_XTS_PLAIN64;
            } else if (formate.contains("SM4")) {
                part.m_encryption = CRYPT_CIPHER::SM4_XTS_PLAIN64;
            }
            part.m_isEncryption = true;

            MessageBox warningBox(this);
            warningBox.setObjectName("messageBox");
            warningBox.setAccessibleName("messageBox");
            // 为防止遗忘密码，请您自行备份密码，并妥善保存！  确 定
            warningBox.setWarings(tr("To avoid losing the password, please back up your password and keep it properly!"),
                                  tr("OK", "button"), DDialog::ButtonRecommend, "OK");
            warningBox.setGeometry(pos().x() + (width() - warningBox.width()) / 2, pos().y() + (height() - warningBox.height()) / 2,
                                   warningBox.width(), warningBox.height());
            warningBox.exec();
        }

        DPalette palette = DApplicationHelper::instance()->palette(m_botFrame);
        palette.setColor(QPalette::ButtonText, color);
        DApplicationHelper::instance()->setPalette(this, palette);

        formate = formate.trimmed().split(" ").at(0);
    } else {
        if (currentSize <= 52) {
            DMessageManager::instance()->sendMessage(this, QIcon(":/icons/deepin/builtin/warning.svg"), tr("To create a partition, you need at least 52 MB"));
            return;
        }
    }

    m_sizeInfo.append(currentSize);
    //绘制新建分区图形
    m_partChartWidget->transInfos(m_totalSize, m_sizeInfo);
    part.m_labelName = m_partNameEdit->text();
    part.m_fstype = formate;
    Byte_Value sectorSize = DMDbusHandler::instance()->getCurPartititonInfo().m_sectorSize;
    part.m_count = static_cast<Sector>(currentSize * (MEBIBYTE / sectorSize));
    //    if (partComCobox->currentText().compare("MiB") == 0) {
    //        part.count = static_cast<int>(partSizeEdit->text().toDouble() * (MEBIBYTE / sector_size)) ;
    //    } else {
    //        part.count = static_cast<int>(partSizeEdit->text().toDouble() * (GIBIBYTE / sector_size));
    //    }
    m_patrinfo.push_back(part);
    m_partChartWidget->update();

    //新增分区后样式变化
    if (sumValue() >= m_totalSize - 0.01) {
        m_isExceed = false;
    } else {
        m_isExceed = true;
    }

    m_applyBtn->setEnabled(true);
    setAddOrRemResult(m_isExceed);
}

void PartitionWidget::onRemovePartition()
{
    if (m_patrinfo.size() > 0)
        m_patrinfo.pop_back();

    m_addButton->setEnabled(true);

    if (m_sizeInfo.size() > 0) {
        m_sizeInfo.removeAt(m_sizeInfo.size() - 1);
        m_partName.removeAt(m_partName.size() - 1);
    }

    //绘制删除分区图形
    m_partChartWidget->transInfos(m_totalSize, m_sizeInfo);
    m_partChartWidget->update();
    //删除分区后样式变化
    m_isExceed = true;

    setAddOrRemResult(m_isExceed);
}

static void snapToMebibyte(DeviceInfo &device, PartitionInfo &newPart)
{
    Sector diff = 0;
    Sector secsPerMib = MEBIBYTE / newPart.m_sectorSize;

    //Align start sector
    diff = Sector(newPart.m_sectorStart % secsPerMib);
    if (diff)
        newPart.m_sectorStart += (secsPerMib - diff);

    //Align end sector
    diff = (newPart.m_sectorEnd + 1) % secsPerMib;
    if (diff)
        newPart.m_sectorEnd -= diff;

    //If this is a GPT partition table and the partition ends less than 34 sectors
    //  from the end of the device, then reserve at least a mebibyte for the
    //  backup partition table
    if ( device.m_disktype == "gpt" && ((device.m_length - newPart.m_sectorEnd) < 34))
        newPart.m_sectorEnd -= secsPerMib;
}

void PartitionWidget::onApplyButton()
{
    bool isCreate = true;
    PartitionVec partVector;
    PartitionInfo curInfo = DMDbusHandler::instance()->getCurPartititonInfo();
    DeviceInfo device = DMDbusHandler::instance()->getCurDeviceInfo();
    Sector beforend = curInfo.m_sectorStart;
    Sector deviceLength = DMDbusHandler::instance()->getCurDeviceInfoLength();

    for (int i = 0; i < m_patrinfo.size(); i++) {
        PartitionInfo newPart;
        newPart.m_sectorStart = beforend;
        if (m_patrinfo.at(i).m_blast)
            newPart.m_sectorEnd = curInfo.m_sectorEnd;
        else
            newPart.m_sectorEnd = newPart.m_sectorStart + m_patrinfo.at(i).m_count;
        qDebug() << beforend << curInfo.m_sectorStart << curInfo.m_sectorEnd;

        newPart.m_fileSystemType = Utils::stringToFileSystemType(m_patrinfo.at(i).m_fstype);
        newPart.m_fileSystemLabel = m_patrinfo.at(i).m_labelName;
        newPart.m_alignment = ALIGN_MEBIBYTE;
        newPart.m_sectorSize = curInfo.m_sectorSize;
        newPart.m_insideExtended = false;
        newPart.m_busy = false;
        newPart.m_fileSystemReadOnly = false;
        newPart.m_devicePath = curInfo.m_devicePath;
        if (device.m_disktype == "gpt") {
            newPart.m_type = TYPE_PRIMARY;
        } else {
            //非逻辑分区外没有指明创建的分区类型主分区/扩展分区，默认主分区
            if (curInfo.m_insideExtended) {
                newPart.m_type = TYPE_LOGICAL;
                newPart.m_insideExtended = true;
            } else {
                newPart.m_type = TYPE_PRIMARY;
            }
        }
        //newpart .inside_extended && newpart .type == TYPE_UNALLOCATED extend partition
        //newpart .type == TYPE_LOGICAL logical partition
        //newpart .sector_start <= (MEBIBYTE / newpart .sector_size) /* Beginning of disk device */
        if ((newPart.m_insideExtended && newPart.m_type == TYPE_UNALLOCATED)
            || newPart.m_type == TYPE_LOGICAL
            || newPart.m_sectorStart <= (MEBIBYTE / newPart.m_sectorSize)) {
            newPart.m_sectorStart += MEBIBYTE / newPart.m_sectorSize;
            newPart.m_sectorEnd += MEBIBYTE / newPart.m_sectorSize;
        }

        beforend = newPart.m_sectorEnd + 1;

        if (beforend > curInfo.m_sectorEnd && i < m_patrinfo.size() - 1) {
            isCreate = false;
            //            qDebug() << "create too partition ,no enough space";
            break;
        } else {
            if (newPart.m_sectorEnd > curInfo.m_sectorEnd)
                newPart.m_sectorEnd = curInfo.m_sectorEnd;

            bool isEndPartition = false;
            if (deviceLength == curInfo.m_sectorEnd + 1) {
                if ((newPart.m_sectorEnd == curInfo.m_sectorEnd - 1) || (newPart.m_sectorEnd == curInfo.m_sectorEnd)) {
                    newPart.m_sectorEnd = curInfo.m_sectorEnd - 33;
                    isEndPartition = true;
                }
            }

            if (!isEndPartition) {
                Sector diff = 0;
                diff = (newPart.m_sectorEnd + 1) % (MEBIBYTE / newPart.m_sectorSize);
                if (diff)
                    newPart.m_sectorEnd -= diff;
            }

            beforend = newPart.m_sectorEnd + 1;

            if (m_patrinfo.at(i).m_isEncryption) {
                newPart.m_luksFlag = LUKSFlag::IS_CRYPT_LUKS;
                newPart.m_crypt = m_patrinfo.at(i).m_encryption;
                QStringList tokenList;
                if (!m_patrinfo.at(i).m_passwordHint.isEmpty()) {
                    tokenList.append(m_patrinfo.at(i).m_passwordHint);
                }

                newPart.m_tokenList = tokenList;
                newPart.m_decryptStr = m_patrinfo.at(i).m_password;
            } else {
                newPart.m_luksFlag = LUKSFlag::NOT_CRYPT_LUKS;
                newPart.m_crypt = CRYPT_CIPHER::NOT_CRYPT;
                newPart.m_tokenList = QStringList();
                newPart.m_decryptStr = "";
            }

            snapToMebibyte(device, newPart);
            partVector.push_back(newPart);
        }
    }
    if (isCreate && partVector.size() > 0) {
        DMDbusHandler::instance()->create(partVector);
        close();
    }
}

void PartitionWidget::onRevertButton()
{
    //复原,即清空
    m_patrinfo.clear();
    m_sizeInfo.clear();
    m_partName.clear();
    m_partNameEdit->clear();
    m_partFormateCombox->setCurrentText("ext4");
    m_partComboBox->setCurrentIndex(0);
    m_partChartWidget->transInfos(m_totalSize, m_sizeInfo);
    m_isExceed = true;

    if (m_sizeInfo.size() == 0) {
        m_applyBtn->setEnabled(false);
    }

    m_partChartWidget->update();
    m_slider->setValue(100);
    onSliderValueChanged(100);
    setSelectUnallocatesSpace();
    m_partChartWidget->transFlag(1, 0);
    setEnable(1, true);
}

void PartitionWidget::onCancelButton()
{
    close();
}

bool PartitionWidget::event(QEvent *event)
{
    // 字体大小改变
    if (QEvent::ApplicationFontChange == event->type()) {
        qDebug() << event->type() << QApplication::font().pointSizeF() / 0.75;
        QFontMetrics fm(m_deviceNameLabel->font());
        m_deviceNameLabel->setFixedWidth(fm.boundingRect(tr("Disk:")).width() + 5);
        m_emptyLabel->setFixedWidth(m_partFormateLabel->fontMetrics().width(tr("File system:")));
        m_encryptionInfo->setFixedWidth(m_partFormateCombox->width());
        m_scrollArea->setFixedWidth(m_partFormateCombox->width());
        DWidget::event(event);
    }

    return DDialog::event(event);
}

void PartitionWidget::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key::Key_Escape) {
        event->ignore();
    } else {
        DDialog::keyPressEvent(event);
    }
}
