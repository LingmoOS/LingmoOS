// Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "iconbutton.h"

#include <QGridLayout>
#include <QDebug>
#include <QTimer>
#include <QToolTip>

const QSize HISTORY_WIDGET_CLEARBUTTONSIZE = QSize(36, 36); //历史记录区垃圾桶大小
const qreal BLURRADIUS = 12; //阴影模糊半径
const qreal ROUND_XRADIUS = 8; //按钮圆角x轴半径
const qreal ROUND_YRADIUS = 8; //按钮圆角y轴半径

IconButton::IconButton(QWidget *parent, int b, bool page)
    : TextButton("", parent)
    , m_effect(new QGraphicsDropShadowEffect(this))
    , m_iconRenderer(new QSvgRenderer(this))
{
    if (b == 1)
        setFixedSize(HISTORY_WIDGET_CLEARBUTTONSIZE);
    m_isHover = false;
    m_isPress = false;
    m_isEmptyBtn = (b == 1);
    m_page = page;
    m_effect->setOffset(0, 4); //阴影偏移
    m_effect->setBlurRadius(BLURRADIUS); //阴影模糊半径
}

IconButton::~IconButton()
{
}

/**
 * @brief 设置图片路径
 * @param normalFileName normal图片路径
 * @param hoverFileName hover图片路径
 * @param pressFileName press图片路径
 * @param mode 区分不同XML元素标签名
 */
void IconButton::setIconUrl(const QString &normalFileName, const QString &hoverFileName, const QString &pressFileName, int mode)
{
    m_normalUrl = normalFileName;
    m_hoverUrl = hoverFileName;
    m_pressUrl = pressFileName;

    m_currentUrl = m_normalUrl;
    m_buttonStatus = 0; //0-normal 1-hover 2-press
    //setIcon(QIcon(m_pixmap));
    //setIconSize(QSize(30,30)*devicePixelRatioF());
    m_mode = mode;
}

QStringList IconButton::getIconUrl() const
{
    QStringList list;
    list << m_normalUrl.right(m_normalUrl.length() - m_normalUrl.indexOf("icon"))
         << m_hoverUrl.right(m_hoverUrl.length() - m_hoverUrl.indexOf("icon"))
         << m_pressUrl.right(m_pressUrl.length() - m_pressUrl.indexOf("icon"));
    return list;
}

/**
 * @brief iconbutton物理键盘点击动画效果
 * @param msec 100ms
 */
void IconButton::animate(bool isspace, int msec)
{
    if (m_isPress == false) { //edit for bug-20492 20200416
        m_isHover = false;  //edit for bug-20508 20200414
        if (!isspace)
            setDown(true);
        m_isPress = true;
        m_currentUrl = m_pressUrl;
        m_buttonStatus = 2;
        if ((m_mode & 1) == 1)
            ++m_mode;

        QTimer::singleShot(msec, this, [ = ] {
            if (!isspace)
                setDown(false);
            m_currentUrl = m_normalUrl;
            m_buttonStatus = 0;
            if ((m_mode & 1) != 1)
                --m_mode;
            m_isPress = false;
            update();
        });
    }
}

/**
 * @brief 设置垃圾桶tooltip
 * @param ismemory true-内存垃圾桶　false-历史记录垃圾桶
 */
void IconButton::showtooltip(bool ismemory)
{
    if (ismemory == true) {
        m_cleartooltip = tr("Clear all memory");
        this->setToolTip(m_cleartooltip);
    } else {
        m_cleartooltip = tr("Clear history");
        this->setToolTip(m_cleartooltip);
    }
}

/**
 * @brief IconButton::updateWhenBtnDisable
 * 当拥有焦点时同时按下空格和鼠标后会导致问题，将其置回普通状态
 */
void IconButton::updateWhenBtnDisable()
{
    m_isPress = false;
    m_buttonStatus = 0;
    m_mode = 1;
    update();
}

/**
 * @brief IconButton::setBtnPressing
 * 用于数据长度和移位类型的按键，点开列表后，保持press的状态
 */
void IconButton::setBtnPressing(bool press)
{
    if (press) {
        m_currentUrl = m_pressUrl;
        if ((m_mode & 1) == 1)
            ++m_mode;
        m_isPress = true;
        m_buttonStatus = 2;
        m_isHover = false; //20200722删除foucus状态
        m_isPressing = true;
    } else {
        m_currentUrl = m_normalUrl;
        if ((m_mode & 1) != 1)
            --m_mode;
        m_isPress = false;
        m_buttonStatus = 0;
        m_isPressing = false;
    }
}

/**
 * @brief IconButton::setBtnHighlight
 * @param light:是否高亮
 * 设置全键盘/位键盘切换按钮高亮显示
 */
void IconButton::setBtnHighlight(bool light)
{
    m_highlight = light;
}

/**
 * @brief 点击时改变标置位
 */
void IconButton::mousePressEvent(QMouseEvent *e)
{
    m_currentUrl = m_pressUrl;
    m_buttonStatus = 2;
    if ((m_mode & 1) == 1)
        ++m_mode;
    m_isPress = true;
    m_isHover = false; //20200722删除foucus状态
    //pixmap.setDevicePixelRatio(devicePixelRatioF());
    //DPushButton::setIcon(QIcon(pixmap));

    TextButton::mousePressEvent(e);
}

/**
 * @brief 鼠标松开时改变标志位
 */
void IconButton::mouseReleaseEvent(QMouseEvent *e)
{
//    if (m_isHistorybtn)
//        clearFocus();
    m_currentUrl = m_normalUrl;
    if ((m_mode & 1) != 1)
        --m_mode;
    if (m_isPress == true && this->rect().contains(e->pos())) {
        m_currentUrl = m_hoverUrl;
        m_buttonStatus = 1;
        emit isclicked();
    } else {
        m_buttonStatus = 0;
    }
    m_isPress = false;
    if (this->rect().contains(e->pos())) {
        m_isacting = true;
        m_isHover = true;
    } else
        m_isacting = false;

    TextButton::mouseReleaseEvent(e);
}

void IconButton::enterEvent(QEvent *e)
{
    m_currentUrl = m_hoverUrl;
    m_buttonStatus = 1;
    m_isHover = true;
    //pixmap.setDevicePixelRatio(devicePixelRatioF());
    //DPushButton::setIcon(QIcon(pixmap));

    TextButton::enterEvent(e);
}

void IconButton::leaveEvent(QEvent *e)
{
    if (!m_isPressing) {
        m_currentUrl = m_normalUrl;
        m_buttonStatus = 0;
        m_isHover = false;
        m_isacting = false;
    }
    //pixmap.setDevicePixelRatio(devicePixelRatioF());
    //DPushButton::setIcon(QIcon(pixmap));

    TextButton::leaveEvent(e);
}

void IconButton::paintEvent(QPaintEvent *)
{
    QPixmap pixmap(m_currentUrl);
    m_pixmap = pixmap; //保存以取图片rect,move到该按钮中间
    QPainter painter(this);
    if (m_isEmptyBtn == false) {
        QRectF frameRect = this->rect();
        QRectF rect(frameRect.left() + 1, frameRect.top() + 1, frameRect.width() - 2, frameRect.height() - 2);
        QRectF hover(frameRect.left() + 1, frameRect.top() + 1, frameRect.width() - 2, frameRect.height() - 2);
//        if (m_isHistorybtn)
//            rect = hover = frameRect;
        painter.setRenderHint(QPainter::Antialiasing, true);
        painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
        //m_pixmap = m_pixmap.scaled(m_pixmap.size() * devicePixelRatioF());
        QColor actcolor = Dtk::Gui::DGuiApplicationHelper::instance()->applicationPalette().highlight().color().name();//活动色
        QColor pressBrush, focus, hoverFrame, base, hoverbrush;
        QColor hoverShadow, focusShadow;
        hoverShadow = actcolor;
        hoverShadow.setAlphaF(0.1);
        focusShadow = QColor(0, 0, 0);
        focusShadow.setAlphaF(0.05);
        int type = DGuiApplicationHelper::instance()->themeType();
        if (type == 1) { //浅色主题设置
            pressBrush = QColor(0, 0, 0);
            pressBrush.setAlphaF(0.1);
            focus = actcolor;
            hoverFrame = actcolor;
            hoverFrame.setAlphaF(0.2);
            if (m_page) {
                base = QColor(240, 240, 240);
                hoverbrush = QColor(240, 240, 240);
            } else {
                base = Qt::white;
                hoverbrush = Qt::white;
            }
        } else { //深色主题设置
            pressBrush = QColor(0, 0, 0);
            pressBrush.setAlphaF(0.5);
            focus = actcolor;
            hoverFrame = actcolor;
            hoverFrame.setAlphaF(0.2);
            if (m_page) {
                base = QColor(60, 60, 60);
                hoverbrush = QColor(60, 60, 60);
            } else {
                base = QColor("#303030");
                hoverbrush = QColor(60, 60, 60);
            }
        }
        if (hasFocus()) {
            if (m_isPress) {
                painter.setBrush(QBrush(pressBrush));
                QPen pen;
                pen.setColor(pressBrush);
                painter.setPen(pen);
                painter.drawRoundedRect(rect, ROUND_XRADIUS, ROUND_YRADIUS); //圆角半径单位为像素
                pen.setColor(focus);
                pen.setWidth(2);
                painter.setPen(pen);
                painter.setBrush(Qt::NoBrush);
                painter.drawRoundedRect(rect, ROUND_XRADIUS, ROUND_YRADIUS); //focus边框
            } else {
                painter.setPen(Qt::NoPen);
                painter.setBrush(QBrush(base));
                painter.drawRoundedRect(rect, ROUND_XRADIUS, ROUND_YRADIUS); //圆角半径单位为像素
                QPen pen;
//                    if (m_isacting) {
//                        painter.setPen(Qt::NoPen);
//                    } else {
                pen.setColor(focus);
                pen.setWidth(2);
                painter.setPen(pen);
//                    }
                painter.setBrush(Qt::NoBrush);
                painter.drawRoundedRect(rect, ROUND_XRADIUS, ROUND_YRADIUS); //圆角半径单位为像素
                m_effect->setColor(focusShadow);
                this->setGraphicsEffect(m_effect);
            }
        } else {
            if (m_isHover) { //hover状态设置
                QPen pen;
                pen.setColor(hoverFrame);
                pen.setWidth(1);
                painter.setPen(pen);
                painter.setBrush(QBrush(hoverbrush));
                painter.drawRoundedRect(rect, ROUND_XRADIUS, ROUND_YRADIUS); //圆角半径单位为像素
                m_effect->setColor(hoverShadow);
                this->setGraphicsEffect(m_effect);
            } else if (m_isPress) { //press状态设置
                painter.setPen(Qt::NoPen);
                painter.setBrush(QBrush(pressBrush));
                painter.drawRoundedRect(rect, ROUND_XRADIUS, ROUND_YRADIUS); //圆角半径单位为像素
            } else { //normal状态设置
                painter.setPen(Qt::NoPen);
                painter.setBrush(QBrush(base));
                painter.drawRoundedRect(rect, ROUND_XRADIUS, ROUND_YRADIUS); //圆角半径单位为像素
                m_effect->setColor(QColor(0, 0, 0, 0));
                this->setGraphicsEffect(m_effect);
            }
        }
    } else {
        QRectF frameRect = this->rect();
        QRectF rect(frameRect.left() + 1, frameRect.top() + 1, frameRect.width() - 2, frameRect.height() - 2);
        painter.setRenderHint(QPainter::Antialiasing, true);
        painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
        QColor actcolor = Dtk::Gui::DGuiApplicationHelper::instance()->applicationPalette().highlight().color().name();//活动色
        QColor focus;
        focus = actcolor;
        if (hasFocus()) {
            QPen pen;
            pen.setColor(focus);
            pen.setWidth(2);
            painter.setPen(pen);
            painter.setBrush(Qt::NoBrush);
            painter.drawRoundedRect(rect, ROUND_XRADIUS, ROUND_YRADIUS); //focus边框
        } else {
            painter.setPen(Qt::NoPen);
            painter.setBrush(Qt::NoBrush);
            painter.drawRoundedRect(rect, ROUND_XRADIUS, ROUND_YRADIUS); //圆角半径单位为像素
        }
    }

    drawCenterPixMap(painter);
}

void IconButton::keyPressEvent(QKeyEvent *e)
{
    switch (e->key()) {
    case Qt::Key_Up:
        emit focus(0);
        break;
    case Qt::Key_Down:
        emit focus(1);
        break;
    case Qt::Key_Left:
        emit focus(2);
        break;
    case Qt::Key_Right:
        emit focus(3);
        break;
    case Qt::Key_Space:
        emit space();
        break;
    case Qt::Key_Enter:
    case Qt::Key_Return:
        if (m_isEmptyBtn)
            emit space();
        break;
    default:
        DPushButton::keyPressEvent(e);
        break;
    }
}

/**
 * @brief SetAttrRecur
 * @param elem
 * @param strtagname XML元素标签名
 * @param strattr setAttribute(name)
 * @param strattrval setAttribute(value)
 * 修改加载的svg图片的颜色
 */
void IconButton::SetAttrRecur(QDomElement elem, QString strtagname, QString strattr, QString strattrval)
{
    if ((m_mode != 1 && m_mode != 3 && m_mode != 5 && m_mode != 7) || m_highlight) {
        if (elem.tagName().compare(strtagname) == 0 && elem.attribute(strattr) != "none" && elem.attribute(strattr) != "") {
            elem.setAttribute(strattr, strattrval);
            if (m_buttonStatus == 0)
                elem.setAttribute("fill-opacity", QLatin1String("0.75")); //在svg文件中添加透明度
            if (m_buttonStatus == 1)
                elem.setAttribute("fill-opacity", QLatin1String("0.65"));
            if (m_buttonStatus == 2)
                elem.setAttribute("fill-opacity", 1);
        }
        if (m_mode == 0) {
            elem.setAttribute(strattr, strattrval);
            if (m_buttonStatus == 0)
                elem.setAttribute("fill-opacity", QLatin1String("0.75"));
            if (m_buttonStatus == 1)
                elem.setAttribute("fill-opacity", QLatin1String("0.65"));
            if (m_buttonStatus == 2)
                elem.setAttribute("fill-opacity", 1);
        }
        if (m_isEmptyBtn == true || (m_mode == 4)) {
            strtagname = "path";
            if (elem.tagName().compare(strtagname) == 0 && elem.attribute(strattr) != "none" && elem.attribute(strattr) != "") {
                if (m_buttonStatus == 2) {
                    elem.setAttribute(strattr, strattrval);
                }
            }
        }
        if (m_mode == 6) {
            if (elem.tagName().compare(strtagname) == 0 && elem.attribute(strattr) != "none" && elem.attribute(strattr) != "") {
                elem.setAttribute(strattr, strattrval);
            }
        }
        if (m_mode == 8) {
            strattr = "fill";
            if (elem.attribute(strattr) != "none" && elem.attribute(strattr) != "") {
                elem.setAttribute(strattr, strattrval);
            }
            strattr = "stroke";
            if (elem.attribute(strattr) != "none" && elem.attribute(strattr) != "") {
                elem.setAttribute(strattr, strattrval);
            }
        }
        if (m_highlight) {
            strtagname = "path";
            if (elem.tagName().compare(strtagname) == 0 && elem.attribute(strattr) != "none" && elem.attribute(strattr) != "") {
                elem.setAttribute(strattr, strattrval);
            }
        }
        for (int i = 0; i < elem.childNodes().count(); i++) {
            if (!elem.childNodes().at(i).isElement()) {
                continue;
            }
            SetAttrRecur(elem.childNodes().at(i).toElement(), strtagname, strattr, strattrval);
        }
    }
}

/**
 * @brief 更改iconbutton图片为活动色，且将图片移至iconbutton中间
 */
void IconButton::drawCenterPixMap(QPainter &painter)
{
    painter.save();

    QFile file(m_currentUrl);
    file.open(QIODevice::ReadOnly);
    QByteArray baData = file.readAll();

    QDomDocument doc;
    doc.setContent(baData);

    file.close();

    //将svg文件中的fill颜色设置改为当前活动色
    SetAttrRecur(doc.documentElement(), "g", "fill", Dtk::Gui::DGuiApplicationHelper::instance()->applicationPalette().highlight().color().name());
    QRectF frameRect = this->rect();
    QRectF rect(frameRect.left(), frameRect.top(), frameRect.width(), frameRect.height());
    QRectF pixRect = m_pixmap.rect();
    pixRect.moveCenter(rect.center()); //将图片移至button中间
//    m_iconRenderer = new DSvgRenderer(doc.toByteArray(), this);
    m_iconRenderer->load(doc.toByteArray());
    m_iconRenderer->render(&painter, pixRect);
    painter.restore();
}

/*void IconButton::setIconSize(const int &size)
{
    const int scaledSize = size * devicePixelRatioF();
    const QSize iconSize(scaledSize, scaledSize);
    const QImage image = m_iconRenderer->toImage(iconSize);

    QPixmap pix;
    pix.convertFromImage(image);
    pix.setDevicePixelRatio(devicePixelRatioF());

    //m_iconWidget->setAlignment(Qt::AlignCenter);
    //m_iconWidget->setPixmap(pix);
    this->DPushButton::setIcon(QIcon(pix));
    //m_iconWidget->setFixedSize(iconSize);
}*/
