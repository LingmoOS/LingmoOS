// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "programmeritemwidget.h"

#include <QPen>
#include <QDomElement>

ProgrammerItemWidget::ProgrammerItemWidget(QString label, const int width, const QString path, QWidget *parent)
    : QWidget(parent)
    , m_iconbtn(new DIconButton(this))
    , m_label(new QLabel(label))
    , m_iconRenderer(new QSvgRenderer(this))
{
    m_width = width;
    setMouseTracking(true);
    setFocusPolicy(Qt::NoFocus);
    m_iconbtn->setFocusPolicy(Qt::NoFocus);
    setFixedSize(QSize(width, 34));
    setAttribute(Qt::WA_TranslucentBackground, true);
    m_currentUrl = path;
    m_iconbtn->setFlat(true);
    m_iconbtn->setIconSize(QSize(34, 34)); //图片有边框，不能与ui图大小一致ui(20, 20)
    m_iconbtn->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    QWidget *markWidget = new QWidget(this);
    markWidget->setFixedSize(12, 10);
    QHBoxLayout *hlayout = new QHBoxLayout();
    hlayout->addSpacing(10);
    hlayout->addWidget(markWidget);
    hlayout->addSpacing(2);
    hlayout->addWidget(m_label);
    m_label->setFixedWidth(width - 70);
    QFont font;
    font.setWeight(2);
    font.setPixelSize(14);
    m_label->setFont(font);
    hlayout->addStretch();
    hlayout->addWidget(m_iconbtn);
    hlayout->setMargin(0);
    hlayout->setContentsMargins(0, 0, 16, 0); //图片比ui大10，此处边框比ui小5
    setLayout(hlayout);
    this->installEventFilter(this);
    setMouseTracking(true);
    m_label->installEventFilter(this);
    m_label->setMouseTracking(true);
    m_iconbtn->installEventFilter(this);
    m_iconbtn->setMouseTracking(true);
    markWidget->installEventFilter(this);
    markWidget->setMouseTracking(true);
    initMark();
    m_isshift = true;

    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged,
            this, &ProgrammerItemWidget::themetypechanged);
}

ProgrammerItemWidget::ProgrammerItemWidget(QString label, QWidget *parent)
    : QWidget(parent)
    , m_label(new QLabel(label))
{
    setFixedSize(QSize(250, 34));
    setFocusPolicy(Qt::NoFocus);
    setAttribute(Qt::WA_TranslucentBackground, true);
    QWidget *markWidget = new QWidget(this);
    markWidget->setFixedSize(12, 10);
    QHBoxLayout *hlayout = new QHBoxLayout();
    hlayout->addSpacing(10);
    hlayout->addWidget(markWidget);
    hlayout->addSpacing(2);
    hlayout->addWidget(m_label);
    QFont font;
    font.setWeight(2);
    font.setPixelSize(14);
    m_label->setFont(font);
    hlayout->setMargin(0);
    hlayout->setContentsMargins(0, 0, 21, 0);
    setLayout(hlayout);
    this->installEventFilter(this);
    setMouseTracking(true);
    m_label->installEventFilter(this);
    m_label->setMouseTracking(true);
    markWidget->installEventFilter(this);
    markWidget->setMouseTracking(true);
    initMark();
    m_isshift = false;

    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged,
            this, &ProgrammerItemWidget::themetypechanged);
}

ProgrammerItemWidget::~ProgrammerItemWidget()
{

}

void ProgrammerItemWidget::enterEvent(QEvent *event)
{
    m_ishover = true;
    update();
    QWidget::enterEvent(event);
}

void ProgrammerItemWidget::leaveEvent(QEvent *event)
{
    m_ishover = false;
    update();
    QWidget::leaveEvent(event);
}

void ProgrammerItemWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton)
        return;
//    setFocus();
    m_ispress = true;
    update();
    QWidget::mousePressEvent(event);
}

void ProgrammerItemWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton)
        return;

    if (m_ispress == true && this->rect().contains(event->pos())) {
        m_ishover = true;
    }
    m_ispress = false;

    update();
    QWidget::mouseReleaseEvent(event);
}

void ProgrammerItemWidget::paintEvent(QPaintEvent *e)
{
    Q_UNUSED(e);
    QRectF rect = this->rect();
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true); //防走样
    painter.setPen(Qt::NoPen);
    QColor press, hover, text, background;
    QColor actcolor = Dtk::Gui::DGuiApplicationHelper::instance()->applicationPalette().highlight().color().name();//活动色
    press = actcolor;
    hover = actcolor;
    text = QColor("#FFFFFF");
    if (m_themetype == 1)
        background = QColor("#F8F8F8");
    else
        background = QColor("#252525");
    if (m_ispress) {
        painter.setPen(Qt::NoPen);
        painter.setBrush(press);
        painter.drawRect(rect); //press状态下对item进行颜色填充
        DPalette pl1 = this->palette(); //按下时给label字体设置颜色
        text.setAlphaF(0.5);
        pl1.setColor(DPalette::Text, text);
        pl1.setColor(DPalette::HighlightedText, text);
        m_label->setPalette(pl1);
        m_currentUrl = m_pressUrl;
    } else if (m_ishover || m_isfocus) {
        painter.setPen(Qt::NoPen);
        painter.setBrush(press);
        painter.drawRect(rect); //press状态下对item进行颜色填充
        DPalette pl1 = this->palette(); //按下时给label字体设置颜色
        text.setAlphaF(1);
        pl1.setColor(DPalette::Text, text);
        pl1.setColor(DPalette::HighlightedText, text);
        m_label->setPalette(pl1);
        m_currentUrl = m_hoverUrl;
    } else {
        DPalette pl1 = this->palette(); //按下时给label字体设置颜色
        if (m_themetype == 1) {
            pl1.setColor(DPalette::Text, QColor("#000000"));
            pl1.setColor(DPalette::HighlightedText, QColor("#000000"));
        } else {
            pl1.setColor(DPalette::Text, QColor("#FFFFFF"));
            pl1.setColor(DPalette::HighlightedText, QColor("#FFFFFF"));
        }
        m_label->setPalette(pl1);
        m_currentUrl = m_normalUrl;
    }
    drawMark(&painter);
    if (m_isshift) {
        m_pixmap.load(m_currentUrl);
        drawIcon(&painter);
    }
}

bool ProgrammerItemWidget::eventFilter(QObject *obj, QEvent *event)
{
    //bug 101004
    if(QEvent::Show == event->type()){
        m_ishover=false;
        m_isfocus=false;
        m_ispress=false;
    }
    if (event->type() == QEvent::MouseMove) {
        if (!m_ispress) {
            m_ishover = true;
        }
        return false;
    }
    return QWidget::eventFilter(obj, event);
}

void ProgrammerItemWidget::cleanHoverState()
{
    m_ishover = false;
    update();
}

/**
 * @brief ProgrammerItemWidget::cleanFocusState
 * 取消focus状态
 */
void ProgrammerItemWidget::cleanFocusState()
{
    m_isfocus = false;
    update();
}

/**
 * @brief ProgrammerItemWidget::isMarkHide
 * @param ishide:是否隐藏勾
 * 当前已选中和点击的行显示勾，其他行隐藏
 */
void ProgrammerItemWidget::isMarkHide(bool ishide)
{
    m_isMarkHide = ishide;
}

void ProgrammerItemWidget::drawMark(QPainter *painter) const
{
    if (!m_isMarkHide) {
        QPen pen;
        QColor color;
        if (m_ispress) {
            color = QColor("#FFFFFF");
            color.setAlphaF(0.5);
        } else if (m_ishover || m_isfocus) {
            color = QColor("#FFFFFF");
        } else {
            if (m_themetype == 1) {
                color = QColor("#000000");
            } else {
                color = QColor("#FFFFFF");
            }
        }
        pen.setColor(color);
        pen.setWidth(2);
        painter->setPen(pen);
        painter->drawPolyline(m_pointVec);
    }
}

/**
 * @brief ProgrammerItemWidget::initMark
 * 初始化勾所需要的点和容器
 */
void ProgrammerItemWidget::initMark()
{
    m_markstart = QPoint(this->rect().x() + 10, this->rect().y() + 18);
    m_markmid = QPoint(this->rect().x() + 14, this->rect().y() + 22);
    m_markend = QPoint(this->rect().x() + 22, this->rect().y() + 12);
    m_pointVec << m_markstart;
    m_pointVec << m_markmid;
    m_pointVec << m_markend;
}

/**
 * @brief ProgrammerItemWidget::iconChanged
 * @param status:0-normal 1-hover 2-press
 * 根据不同鼠标事件修改icon图片
 */
void ProgrammerItemWidget::drawIcon(QPainter *painter) const
{
    painter->save();

    QFile file(m_currentUrl);
    file.open(QIODevice::ReadOnly);
    QByteArray baData = file.readAll();

    QDomDocument doc;
    doc.setContent(baData);

    file.close();

    if (m_ispress) {
        doc.documentElement().setAttribute("fill-opacity", QLatin1String("0.5"));
    }
    QRectF frameRect = m_iconbtn->rect();
    frameRect.moveTo(m_width - 50, 0);
    m_iconRenderer->load(doc.toByteArray());
    m_iconRenderer->render(painter, frameRect);
    painter->restore();
}

void ProgrammerItemWidget::themetypechanged(int type)
{
    //主题改变时切换悬浮框图片
    QString path;
    if (DGuiApplicationHelper::instance()->themeType() == 2) {
        path = QString(":/assets/images/%1/").arg("dark");
        if (m_label->text() == tr("Arithmetic shift")) {
            m_normalUrl = path + "icon_as_normal.svg";
            m_hoverUrl = path + "icon_as_hover.svg";
            m_pressUrl = path + "icon_as_hover.svg";
        } else if (m_label->text() == tr("Logical shift")) {
            m_normalUrl = path + "icon_ls_normal.svg";
            m_hoverUrl = path + "icon_ls_hover.svg";
            m_pressUrl = path + "icon_ls_hover.svg";
        } else if (m_label->text() == tr("Circular shift")) {
            m_normalUrl = path + "icon_ro_normal.svg";
            m_hoverUrl = path + "icon_ro_hover.svg";
            m_pressUrl = path + "icon_ro_hover.svg";
        } else if (m_label->text() == tr("Rotate through carry circular shift")) {
            m_normalUrl = path + "icon_rc_normal.svg";
            m_hoverUrl = path + "icon_rc_hover.svg";
            m_pressUrl = path + "icon_rc_hover.svg";
        }
    } else {
        path = QString(":/assets/images/%1/").arg("light");
        if (m_label->text() == tr("Arithmetic shift")) {
            m_normalUrl = path + "icon_as_normal.svg";
            m_hoverUrl = path + "icon_as_menuhover.svg";
            m_pressUrl = path + "icon_as_menuhover.svg";
        } else if (m_label->text() == tr("Logical shift")) {
            m_normalUrl = path + "icon_ls_normal.svg";
            m_hoverUrl = path + "icon_ls_menuhover.svg";
            m_pressUrl = path + "icon_ls_menuhover.svg";
        } else if (m_label->text() == tr("Circular shift")) {
            m_normalUrl = path + "icon_ro_normal.svg";
            m_hoverUrl = path + "icon_ro_menuhover.svg";
            m_pressUrl = path + "icon_ro_menuhover.svg";
        } else if (m_label->text() == tr("Rotate through carry circular shift")) {
            m_normalUrl = path + "icon_rc_normal.svg";
            m_hoverUrl = path + "icon_rc_menuhover.svg";
            m_pressUrl = path + "icon_rc_menuhover.svg";
        }
    }

    m_themetype = type;
    DPalette pl1 = this->palette();
    //itemwidget字体颜色设置
    if (m_themetype == 1) {
        pl1.setColor(DPalette::Text, QColor("#000000"));
    } else {
        pl1.setColor(DPalette::Text, QColor("#FFFFFF"));
    }

    m_label->setPalette(pl1);
}

void ProgrammerItemWidget::setFocus()
{
    m_isfocus = true;
    update();
}

void ProgrammerItemWidget::resetWidth(const int width)
{
    m_width = width;
    setFixedSize(QSize(width, 34));
    m_label->setFixedWidth(width - 80);
    update();
}
