// Copyright (C) 2020 ~ 2022 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "imagemenu.h"
#include "../utils.h"
#include "../utils/borderprocessinterface.h"
#include "../utils/configsettings.h"
#include <QVBoxLayout>
#include <DRadioButton>
#include <QPixmap>
#include <DIconButton>
#include <DHiDPIHelper>
#include <QWidgetAction>
#include <QBitmap>
#include <QPaintEvent>
#include <QButtonGroup>
#include <QAbstractButton>
#include <QtSvg/QtSvg>

ImageMenu::ImageMenu(ImageBorderHelper::BorderType type, QString title, QWidget *parent) : DMenu(title, parent)
{

    m_borderType = type;
    m_actionGroup = new QButtonGroup(this);
    m_actionGroup->setExclusive(false);
    if (m_borderType == ImageBorderHelper::BorderType::Prototype) {
        ActionWidget *pc = new ActionWidget(this);
        pc->setPixmap("imageBorder/pc.svg");
        m_actionGroup->addButton(pc, 1);

        ActionWidget *notebook = new ActionWidget(this);
        notebook->setPixmap("imageBorder/notebook.svg");
        m_actionGroup->addButton(notebook, 2);

        ActionWidget *ipad = new ActionWidget(this);
        ipad->setPixmap("imageBorder/ipad.svg");
        m_actionGroup->addButton(ipad, 3);

        ActionWidget *cellphone = new ActionWidget(this);
        cellphone->setPixmap("imageBorder/cellphone.svg");
        m_actionGroup->addButton(cellphone, 4);

        QWidgetAction *pcAction = new QWidgetAction(this);
        pcAction->setDefaultWidget(pc);
        this->addAction(pcAction);

        QWidgetAction *notebookAction = new QWidgetAction(this);
        notebookAction->setDefaultWidget(notebook);
        this->addAction(notebookAction);

        QWidgetAction *ipadAction = new QWidgetAction(this);
        ipadAction->setDefaultWidget(ipad);
        this->addAction(ipadAction);

        QWidgetAction *cellphoneAction = new QWidgetAction(this);
        cellphoneAction->setDefaultWidget(cellphone);
        this->addAction(cellphoneAction);
    } else if (m_borderType == ImageBorderHelper::BorderType::External) {
        ActionWidget *externalBorder1 = new ActionWidget(this);
        externalBorder1->setPixmap("imageBorder/externalBorder1.svg");
        m_actionGroup->addButton(externalBorder1, 1);

        ActionWidget *externalBorder2 = new ActionWidget(this);
        externalBorder2->setPixmap("imageBorder/externalBorder2.svg");
        m_actionGroup->addButton(externalBorder2, 2);

        ActionWidget *externalBorder3 = new ActionWidget(this);
        externalBorder3->setPixmap("imageBorder/externalBorder3.svg");
        m_actionGroup->addButton(externalBorder3, 3);

        ActionWidget *externalBorder4 = new ActionWidget(this);
        externalBorder4->setPixmap("imageBorder/externalBorder4.svg");
        m_actionGroup->addButton(externalBorder4, 4);

        ActionWidget *externalBorder5 = new ActionWidget(this);
        externalBorder5->setPixmap("imageBorder/externalBorder5.svg");
        m_actionGroup->addButton(externalBorder5, 5);

        ActionWidget *externalBorder6 = new ActionWidget(this);
        externalBorder6->setPixmap("imageBorder/externalBorder6.svg");
        m_actionGroup->addButton(externalBorder6, 6);

        ActionWidget *externalBorder7 = new ActionWidget(this);
        externalBorder7->setPixmap("imageBorder/externalBorder7.svg");
        m_actionGroup->addButton(externalBorder7, 7);

        ActionWidget *externalBorder8 = new ActionWidget(this);
        externalBorder8->setPixmap("imageBorder/externalBorder8.svg");
        m_actionGroup->addButton(externalBorder8, 8);

        QHBoxLayout *hLayout1 = new QHBoxLayout;
        hLayout1->setSpacing(10);
        hLayout1->addWidget(externalBorder1);
        hLayout1->addWidget(externalBorder2);

        QHBoxLayout *hLayout2 = new QHBoxLayout;
        hLayout2->setSpacing(10);
        hLayout2->addWidget(externalBorder3);
        hLayout2->addWidget(externalBorder4);

        QHBoxLayout *hLayout3 = new QHBoxLayout;
        hLayout3->setSpacing(10);
        hLayout3->addWidget(externalBorder5);
        hLayout3->addWidget(externalBorder6);

        QHBoxLayout *hLayout4 = new QHBoxLayout;
        hLayout4->setSpacing(10);
        hLayout4->addWidget(externalBorder7);
        hLayout4->addWidget(externalBorder8);

        QVBoxLayout *hLayout = new QVBoxLayout(this);
        hLayout->setSpacing(10);
        hLayout->addLayout(hLayout1);
        hLayout->addLayout(hLayout2);
        hLayout->addLayout(hLayout3);
        hLayout->addLayout(hLayout4);
        this->setLayout(hLayout);
    } else if (m_borderType == ImageBorderHelper::BorderType::Projection) {
        ActionWidget *projection = new ActionWidget(this);
        projection->setPixmap("imageBorder/Projection.svg");
        m_actionGroup->addButton(projection, 1);
        QHBoxLayout *hLayout = new QHBoxLayout();
        hLayout->setContentsMargins(0, 6, 0, 1);
        hLayout->addWidget(projection);
        this->setLayout(hLayout);
    }

    connect(m_actionGroup, SIGNAL(buttonClicked(QAbstractButton *)), this, SLOT(ActionChecked(QAbstractButton *)));
}

void ImageMenu::AddAction()
{

}

int ImageMenu::getBorderTypeDetail()
{
    int typeDetail = 0;
    QAbstractButton *bu = m_actionGroup->checkedButton();
    if (bu) {
        typeDetail = m_actionGroup->id(bu);
    }
    return typeDetail;
}

void ImageMenu::setBorderTypeDetail(const int id)
{
    qDebug() << __FUNCTION__ << __LINE__ << m_borderType << id;
    ActionWidget *button = static_cast<ActionWidget *>(m_actionGroup->button(id));
    if (button) {
        button->setActionState(true);
    }
}

void ImageMenu::ActionChecked(QAbstractButton *button)
{
    foreach (QAbstractButton *bu, m_actionGroup->buttons()) {
        static_cast<ActionWidget *>(bu)->setActionState(button && button->isChecked() && button == bu);
    }
    if (button) {
        ImageBorderHelper::instance()->setActionState(m_borderType, button->isChecked());
        int border_index = ImageBorderHelper::instance()->getBorderTypeDetail();
        ConfigSettings::instance()->setValue("shot", "border_index", border_index);
    }
}

void ImageMenu::paintEvent(QPaintEvent *event)
{

    // 在绘制之前， 调整界面位置。
    QRect r = this->geometry();
    QPoint pointLocation = QPoint(r.x(), r.y());
    // 一级菜单位置
    QRect pr = static_cast<QWidget *>(this->parent())->geometry();
    if (r.x() + r.width() > ImageBorderHelper::instance()->m_screenSize.width()) {
        pointLocation.setX(pr.x() - r.width());
    }
    if (r.y() + r.height() > ImageBorderHelper::instance()->m_screenSize.height()) {
        pointLocation.setY(ImageBorderHelper::instance()->m_screenSize.height() - r.height());
    }
    this->move(pointLocation);

    QMenu::paintEvent(event);
}

ActionWidget::ActionWidget(QWidget *parent) : DIconButton(parent)
{
    this->setFlat(true);
    this->setCheckable(true);
    this->setFocusPolicy(Qt::NoFocus);
    this->setFixedSize(160, 100);
    this->setIconSize(QSize(160, 100));
}
ActionWidget::~ActionWidget()
{

}

void ActionWidget::setPixmap(const QString rePath)
{
    m_pixmap = DHiDPIHelper::loadNxPixmap(Utils::getQrcPath(rePath));
}

void ActionWidget::setActionState(const bool isChecked)
{
    this->setChecked(isChecked);
    m_isCheck = isChecked;
    update();
}

void ActionWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.drawPixmap(QPoint(0, 0), m_pixmap);
    if (m_isCheck) {
        QPixmap p = DHiDPIHelper::loadNxPixmap(Utils::getQrcPath("imageBorder/Selected-Light.svg"));
        qreal m_pixelRatio = qApp->primaryScreen()->devicePixelRatio();
        int x = static_cast<int>(this->width() - p.width() / m_pixelRatio) + 3;
        painter.drawPixmap(QPoint(x, -1), p);
    }
    painter.end();
    event->accept();
}

ImageBorderHelper *ImageBorderHelper::m_imageBorderHelper = nullptr;
ImageBorderHelper *ImageBorderHelper::instance()
{
    if (!m_imageBorderHelper) {
        m_imageBorderHelper = new ImageBorderHelper;
    }
    return m_imageBorderHelper;
}

ImageMenu *ImageBorderHelper::getBorderMenu(const BorderType type, const QString title, QWidget *parent)
{
    if (!m_allBorderMenu.contains(type)) {
        m_allBorderMenu.insert(type, new ImageMenu(type, title, parent));
    }
    return m_allBorderMenu[type];
}

void ImageBorderHelper::setActionState(const BorderType type, const bool isChecked)
{
    // 选中某边框，后清空其他类型的所有边框
    for (auto itr = m_allBorderMenu.begin(); itr != m_allBorderMenu.end(); ++itr) {
        if (itr.value()->borderType() != type) {
            itr.value()->ActionChecked(nullptr);
        }
    }

    if (type != Nothing) {
        updateBorderState(isChecked);
    }
}

int ImageBorderHelper::getBorderTypeDetail()
{
    int typeDetail = 0;
    for (auto itr = m_allBorderMenu.begin(); itr != m_allBorderMenu.end(); ++itr) {
        ImageMenu *menu = itr.value();
        typeDetail = menu->getBorderTypeDetail();
        if (typeDetail != 0) {
            typeDetail = (menu->borderType() << 8 | typeDetail);
            break;
        }
    }
    return typeDetail;
}

void ImageBorderHelper::setBorderTypeDetail(const int typeDetail)
{
    BorderType type = static_cast<BorderType>(typeDetail >> 8);
    int id = typeDetail & 0xFF;

    if (m_allBorderMenu.contains(type)) {
        m_allBorderMenu[type]->setBorderTypeDetail(id);
    }
}

QPixmap ImageBorderHelper::getPixmapAddBorder(QPixmap pix)
{
    // borderType = 257 , 外边框
    // borderType = 513 , 样机;
    // borderType = 769 , 投影;
    int borderType = getBorderTypeDetail();
    if (borderType == BorderType::Nothing) {
        return pix;
    }

    BorderType type = static_cast<BorderType>(borderType >> 8);
    if (type == BorderType::Prototype) {
        m_borderhandle = new PrototypeBorderProcess(this);
    } else if (type == BorderType::External) {
        m_borderhandle = new ExternalBorderProcess(this);
    } else if (type == BorderType::Projection) {
        m_borderhandle = new ShadowBorderProcess(this);
    }
    m_borderhandle->initBorderInfo(borderType & 0xFF);

    return m_borderhandle->getPixmapAddBorder(pix);
}

ImageBorderHelper::ImageBorderHelper(QObject *parent) : QObject(parent)
{
    m_borderhandle = nullptr;

    // 计算屏幕大小
    QList<QScreen *> screenList = qApp->screens();
    QList<QRect> screenInfo;
    QSize screenSize;

    int m_screenCount = QApplication::desktop()->screenCount();
    for (QList<QScreen *>::const_iterator it = screenList.constBegin(); it != screenList.constEnd(); ++it) {
        QRect rect = (*it)->geometry();
        screenInfo.append(rect);
    }

    screenSize.setWidth(screenInfo[0].x() + screenInfo[0].width());
    screenSize.setHeight(screenInfo[0].y() + screenInfo[0].height());

    for (int i = 1; i < m_screenCount; ++i) {
        if ((screenInfo[i].height() + screenInfo[i].y()) > screenSize.height())
            screenSize.setHeight(screenInfo[i].height() + screenInfo[i].y());

        if ((screenInfo[i].width() + screenInfo[i].x()) > screenSize.width())
            screenSize.setWidth(screenInfo[i].width() + screenInfo[i].x());
    }
    qDebug() << __FUNCTION__ << __LINE__ << screenSize;
    m_screenSize = screenSize;
}

ImageBorderHelper::~ImageBorderHelper()
{
    for (auto itr = m_allBorderMenu.begin(); itr != m_allBorderMenu.end(); ++itr) {
        delete *itr;
    }
    if (m_borderhandle != nullptr) {
        delete m_borderhandle;
    }
}
