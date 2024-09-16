// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "driveritem.h"
#include "commontools.h"
#include "BtnLabel.h"

#include <DFontSizeManager>
#include <DApplicationHelper>
#include <DApplication>
#include <DToolTip>

#include <QHBoxLayout>

DriverCheckItem::DriverCheckItem(DWidget *parent, bool header)
    : DWidget(parent)
    , mp_cb(new DCheckBox(this))
{
    QHBoxLayout *hLayout = new QHBoxLayout(this);
    if (header) {
        hLayout->setContentsMargins(9, 7, 0, 0);
    } else {
        hLayout->setContentsMargins(1, 0, 0, 0);
    }
    hLayout->addWidget(mp_cb);
    this->setLayout(hLayout);
    this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    connect(mp_cb, &DCheckBox::stateChanged, this, &DriverCheckItem::slotStateChanged);
}

void DriverCheckItem::setChecked(bool checked, bool dis)
{
    // 设置之前先断开
    if (dis)
        disconnect(mp_cb, &DCheckBox::stateChanged, this, &DriverCheckItem::slotStateChanged);

    mp_cb->setChecked(checked);

    // 设置之后重新链接
    if (dis)
        connect(mp_cb, &DCheckBox::stateChanged, this, &DriverCheckItem::slotStateChanged);
}

bool DriverCheckItem::checked()
{
    return mp_cb->isChecked();
}

bool DriverCheckItem::isEnabled()
{
    return mp_cb->isEnabled();
}

void DriverCheckItem::setCbEnable(bool e)
{
    mp_cb->setEnabled(e);
}

void DriverCheckItem::paintEvent(QPaintEvent *event)
{
    return DWidget::paintEvent(event);
}

void DriverCheckItem::slotStateChanged(int state)
{
    if (Qt::Unchecked == state)
        emit sigChecked(false);
    else
        emit sigChecked(true);
}

DriverNameItem::DriverNameItem(DWidget *parent, DriverType dt)
    : DWidget(parent)
    , mp_Icon(new DLabel(this))
    , mp_Type(new DLabel(this))
    , mp_Name(new TipsLabel(this))
    , m_Index(-1)
{
    mp_Type->setAlignment(Qt::AlignLeft | Qt::AlignBottom);
    mp_Type->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    mp_Type->setElideMode(Qt::ElideRight);
    DFontSizeManager::instance()->bind(mp_Type, DFontSizeManager::T6);

    // 设置名称的相关参数：字体大小、颜色、对齐、布局
    mp_Name->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    mp_Name->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    mp_Name->setElideMode(Qt::ElideRight);

    mp_Icon->setPixmap(QIcon(CommonTools::getDriverPixmap(dt)).pixmap(ICON_SIZE_WIDTH, ICON_SIZE_HEIGHT));
    QString ts = DApplication::translate("QObject", CommonTools::getDriverType(dt).toStdString().data());
    mp_Type->setText(ts);

    QHBoxLayout *hLayout = new QHBoxLayout(this);
    hLayout->setContentsMargins(10, 0, 0, 0);
    hLayout->addWidget(mp_Icon);
    hLayout->addSpacing(20);

    QVBoxLayout *vLayout = new QVBoxLayout();
    vLayout->setContentsMargins(0, 0, 0, 0);
    vLayout->addSpacing(3);
    vLayout->setSpacing(0);
    vLayout->addWidget(mp_Type);
    vLayout->addWidget(mp_Name);
    hLayout->addLayout(vLayout);
    hLayout->addSpacing(15);

    this->setLayout(hLayout);
}

void DriverNameItem::setName(const QString &name)
{
    mp_Name->setText(name);
    mp_Name->setToolTip(name);
}

void DriverNameItem::setIndex(int index)
{
    m_Index = index;
}

int DriverNameItem::index()
{
    return m_Index;
}


DriverLabelItem::DriverLabelItem(DWidget *parent,  const QString &txt)
    : DWidget(parent)
    , mp_Txt(new TipsLabel(this))
{
    mp_Txt->setText(txt);
    mp_Txt->setElideMode(Qt::ElideRight);

    QVBoxLayout *vLayout = new QVBoxLayout();
    vLayout->setContentsMargins(1,0,0,0);
    vLayout->addStretch();
    vLayout->addWidget(mp_Txt);
    vLayout->addStretch();
    this->setLayout(vLayout);
    this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

DriverStatusItem::DriverStatusItem(DWidget *parent, Status s)
    : DWidget(parent)
    , mp_Spinner(new DSpinner(this))
    , mp_Icon(new DLabel(this))
    , mp_Status(new BtnLabel(this))
{
    mp_Spinner->setFixedSize(16, 16);
    mp_Spinner->start();
    mp_Status->setElideMode(Qt::ElideRight);

    QHBoxLayout *hLayout = new QHBoxLayout(this);
    this->setContentsMargins(0, 0, 0, 0);
    hLayout->setContentsMargins(0, 0, 0, 0);
    hLayout->setSpacing(4);
    hLayout->addWidget(mp_Spinner);
    hLayout->addWidget(mp_Icon);
    hLayout->addWidget(mp_Status);
    hLayout->addStretch();
    this->setLayout(hLayout);

    // 初始化图标状态和状态信息
    setStatus(s);
}

void DriverStatusItem::setStatus(Status st)
{
    showSpinner(ST_DOWNLOADING == st || ST_INSTALL == st || ST_DRIVER_BACKING_UP == st || ST_DRIVER_RESTORING == st);
    mp_Icon->setPixmap(QIcon(CommonTools::getStatusPixmap(st)).pixmap(16, 16));

    // bug132075 安装成功状态此button无法点击
    QString ts = DApplication::translate("QObject", CommonTools::getStausType(st).toStdString().data());

    if (ST_FAILED == st || ST_DRIVER_BACKUP_FAILED == st) {
        QString statusStr = QString("<a style=\"text-decoration:none\" href=\"failed\">") + ts + "</a>";
        mp_Status->setText(statusStr);
    } else {
        mp_Status->setText(ts);
    }

    m_Status = st;
}

Status DriverStatusItem::getStatus()
{
    return m_Status;
}

void DriverStatusItem::setErrorMsg(const QString &msg)
{
    mp_Status->setDesc(msg);
}

void DriverStatusItem::showSpinner(bool spin)
{
    mp_Icon->setVisible(!spin);
    mp_Spinner->setVisible(spin);
    spin ? mp_Spinner->start() : mp_Spinner->stop();
    mp_Status->setVisible(true);
}

DriverOperationItem::DriverOperationItem(DWidget *parent, Mode mode)
    : DWidget(parent)
    , mp_Btn(new DToolButton(this))
{
    QHBoxLayout *hLayout = new QHBoxLayout(this);
    hLayout->setContentsMargins(0, 0, 0, 0);
    hLayout->addWidget(mp_Btn);
    mp_Btn->setFixedSize(36, 36);
    mp_Btn->setIconSize(QSize(36, 36));
    m_mode = mode;
    setBtnIcon();
    this->setLayout(hLayout);
    // 处理信号逻辑
    connect(mp_Btn, &DIconButton::clicked, this, &DriverOperationItem::clicked);
}

void DriverOperationItem::setBtnEnable(bool enable)
{
    mp_Btn->setEnabled(enable);
}

void DriverOperationItem::setBtnIcon()
{
    switch (m_mode) {
    case Mode::INSTALL:
        mp_Btn->setIcon(QIcon::fromTheme("install"));
        mp_Btn->setToolTip(QObject::tr("Install"));
        break;
    case Mode::UPDATE:
        mp_Btn->setIcon(QIcon::fromTheme("update-btn"));
        mp_Btn->setToolTip(QObject::tr("Update"));
        break;
    case Mode::BACKUP:
        mp_Btn->setIcon(QIcon(":/icons/deepin/builtin/light/backup.svg"));
        mp_Btn->setIconSize(QSize(16, 16));
        mp_Btn->setToolTip(QObject::tr("Backup"));
        break;
    case Mode::RESTORE:
        mp_Btn->setIcon(QIcon(":/icons/deepin/builtin/light/restore.svg"));
        mp_Btn->setIconSize(QSize(16, 16));
        mp_Btn->setToolTip(QObject::tr("Restore"));
        break;
    }
}

void DriverOperationItem::enterEvent(QEvent *event)
{
    return DWidget::enterEvent(event);
}

void DriverOperationItem::leaveEvent(QEvent *event)
{
    return DWidget::leaveEvent(event);
}
