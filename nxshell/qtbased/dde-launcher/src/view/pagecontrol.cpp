// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "pagecontrol.h"
#include "util.h"
#include "appsmanager.h"

#include <QBoxLayout>

PageControl::PageControl(QWidget *parent)
    : QWidget(parent)
    , m_pageCount(0)
    , m_pixChecked(renderSVG(":/widgets/images/checked_rounded_rect.svg", QSize(20, 12)))
{
    QHBoxLayout *layout = new QHBoxLayout();
    layout->setMargin(0);
    layout->setSpacing(0);
    setLayout(layout);

    createButtons();
}

void PageControl::setPageCount(int count)
{
    if (count > m_buttonList.size())
        return;

    for (int i = m_pageCount ; i < count ; i++)
        addButton(m_buttonList[i]);

    for (int i = count; i < m_pageCount ; i++) {
        DIconButton *pageButton = qobject_cast<DIconButton *>(layout()->takeAt(0)->widget());
        int index = m_buttonList.indexOf(pageButton);
        if (index != -1) {
            m_buttonList[index]->setVisible(false);
            m_buttonList[index]->disconnect(this);
        }
    }

    m_pageCount = count;

    setCurrent(0);
}

void PageControl::setCurrent(int pageIndex)
{
    if (pageIndex < layout()->count()) {
        DIconButton *pageButton = qobject_cast<DIconButton *>(layout()->itemAt(pageIndex)->widget());
        if (!pageButton)
            return;

        pageButton->setChecked(true);
        update();
    }
}

void PageControl::updateIconSize(double scaleX, double scaleY)
{
    double scale = (qAbs(1 - scaleX) < qAbs(1 - scaleY)) ? scaleX : scaleY;
    for (int i = 0; i < m_pageCount ; i++) {
        DIconButton *pageButton = qobject_cast<DIconButton *>(layout()->itemAt(i)->widget());
        if (!pageButton)
            return;

        pageButton->setIconSize(QSize(PAGE_ICON_SIZE * scale, PAGE_ICON_SIZE * scale));
        pageButton->setFixedSize(QSize(PAGE_BUTTON_SIZE * scale, PAGE_BUTTON_SIZE * scale));
    }

    setFixedHeight(PAGE_BUTTON_SIZE * scale);
}

/** 提前创建分页控件的按钮
 * @brief PageControl::createButtons
 */
void PageControl::createButtons()
{
#define SINGLE_PAGE_MINIMUM_ITEM 12.0
    // 获取当前最大可能的页数
    int totalPage = qCeil(AppsManager::instance()->appsInfoListSize(AppsListModel::WindowedAll) / SINGLE_PAGE_MINIMUM_ITEM);

    for (int i = 0; i < totalPage; i++) {
        DIconButton *pageButton = new DIconButton(this);
        pageButton->setVisible(false);
        m_buttonList.append(pageButton);
    }
}

void PageControl::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

    qreal ratio = qApp->devicePixelRatio();
    for (DIconButton *button : m_buttonList) {
        if (!button->isVisible())
            continue;

        if (button->isChecked()) {
            QRect rect = QRect(button->rect().topLeft(), QSize(20, 12) / ratio);
            rect.moveCenter(button->geometry().center());
            painter.drawPixmap(rect, m_pixChecked);
        } else {
            QRect rect = QRect(button->rect().topLeft(), QSize(10, 10) / ratio);
            rect.moveCenter(button->geometry().center());
            QColor penColor = Qt::black;
            penColor.setAlphaF(0.1);
            QPen pen(penColor, 1);

            QColor brushColor = Qt::white;
            brushColor.setAlphaF(0.2);
            painter.setBrush(brushColor);
            painter.setPen(pen);
            painter.drawEllipse(rect);
        }
    }

    QWidget::paintEvent(event);
}

void PageControl::addButton(DIconButton *pageButton)
{
    pageButton->setAccessibleName("thisPageButton");
    pageButton->setIconSize(QSize(PAGE_ICON_SIZE, PAGE_ICON_SIZE));
    pageButton->setFixedSize(QSize(PAGE_BUTTON_SIZE, PAGE_BUTTON_SIZE));
    pageButton->setBackgroundRole(DPalette::Button);
    pageButton->setAutoExclusive(true);
    pageButton->setFocusPolicy(Qt::NoFocus);
    pageButton->setCheckable(true);
    pageButton->setFlat(true);
    pageButton->setVisible(true);
    layout()->addWidget(pageButton);

    connect(pageButton, &DIconButton::toggled, this, &PageControl::pageBtnClicked);
}

void PageControl::pageBtnClicked(bool checked)
{
    DIconButton *pageButton = qobject_cast<DIconButton *>(sender());
    if (!pageButton)
        return;

    pageButton->setAccessibleName("addPageBtn");

    if (checked)
        emit onPageChanged(layout()->indexOf(pageButton));
}
