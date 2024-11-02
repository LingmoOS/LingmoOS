/*
 * liblingmosdk-qtwidgets's Library
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Zhen Sun <sunzhen1@kylinos.cn>
 *
 */

#include "kcolorcombobox.h"
#include "themeController.h"
#include <qstylepainter.h>
#include <QStyledItemDelegate>
#include <QVariant>
#include <QAbstractItemView>
#include <QCompleter>
#include <QListView>
#include <QProxyStyle>
#include <QDebug>
#include <QResizeEvent>

namespace kdk {

const static int defaultBorderRadius = 4;
const static QSize defaultPopupItemSize(20,20);
static QSize g_size;

class KComboStyle:public QProxyStyle
{
public:
    KComboStyle(){}
    ~KComboStyle(){}

QSize sizeFromContents(QStyle::ContentsType type,
                       const QStyleOption *option,
                       const QSize &contentsSize,
                       const QWidget *widget = nullptr) const override;

};

class KColorComboBoxDelegate:public QStyledItemDelegate
{
    Q_OBJECT

public:
    enum ItemRoles {
        ColorRole = Qt::UserRole + 1
    };

    KColorComboBoxDelegate(QObject *parent = nullptr,KColorComboBox* combo = nullptr);
    void paint(QPainter *painter,
               const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &index) const override;

private:
    KColorComboBox* m_combo;
};

class KColorComboBoxPrivate:public QObject,public ThemeController
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(KColorComboBox)

public:
    KColorComboBoxPrivate(KColorComboBox*parent);
    void updateList();

public Q_SLOTS:
    void  slotActivated(int);
    void  slotHighlighted(int);
    void  slotCurrentIndexChanged(int);

private:
    KColorComboBox* q_ptr;
    KColorComboBox::ComboType m_comboType;
    QList<QColor> m_colorList;
    int m_borderRadius;
    QColor m_currentColor;
    QSize m_popupItemSize;
};

KColorComboBox::KColorComboBox(QWidget *parent)
    : QComboBox(parent),
      d_ptr(new KColorComboBoxPrivate(this))
{
    Q_D(KColorComboBox);
    setItemDelegate(new KColorComboBoxDelegate(this,this));
    KComboStyle*style  = new KComboStyle();
    setStyle(style);
    view()->setFixedWidth(d->m_popupItemSize.width());
    connect(this, SIGNAL(activated(int)), d, SLOT(slotActivated(int)));
    connect(this, SIGNAL(highlighted(int)),d, SLOT(slotHighlighted(int)));
    connect(this, SIGNAL(currentIndexChanged(int)),d, SLOT(slotCurrentIndexChanged(int)));
}

void KColorComboBox::setColorList(const QList<QColor> &list)
{
    Q_D(KColorComboBox);
    d->m_colorList = list;
    d->updateList();
}

QList<QColor> KColorComboBox::colorList()
{
    Q_D(KColorComboBox);
    return d->m_colorList;
}

void KColorComboBox::addColor(const QColor &color)
{
    Q_D(KColorComboBox);
    d->m_colorList.append(color);
    addItem(QString());
    setItemData(d->m_colorList.count()-1 , d->m_colorList.back(), KColorComboBoxDelegate::ColorRole);
    update();
}

void KColorComboBox::setComboType(const KColorComboBox::ComboType &type)
{
    Q_D(KColorComboBox);
    d->m_comboType = type;
    update();
}

KColorComboBox::ComboType KColorComboBox::comboType()
{
    Q_D(KColorComboBox);
    return d->m_comboType;
}

void KColorComboBox::setPopupItemSize(const QSize &size)
{
    Q_D(KColorComboBox);

    //d->m_popupItemSize = size;
    d->m_popupItemSize = size.expandedTo(this->size());
    g_size = d->m_popupItemSize;
    view()->setFixedWidth(d->m_popupItemSize.width());
    update();
}

QSize KColorComboBox::popupItemSzie()
{
    Q_D(KColorComboBox);
    return d->m_popupItemSize;
}

void KColorComboBox::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    Q_D(KColorComboBox);
    QStylePainter painter(this);
    painter.setPen(Qt::NoPen);

    QStyleOptionComboBox opt;
    initStyleOption(&opt);

    QRect frame = this->rect();
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::transparent);
    if(d->m_currentColor.isValid())
        painter.setBrush(d->m_currentColor);
    if(d->m_comboType == KColorComboBox::Circle)
    {
        painter.drawEllipse(frame.adjusted(1, 1, -1, -1));
    }
    else
    {
        painter.drawRoundedRect(frame.adjusted(1, 1, -1, -1), d->m_borderRadius, d->m_borderRadius);
    }
}

void KColorComboBox::resizeEvent(QResizeEvent *event)
{
    setPopupItemSize(event->size());
    QComboBox::resizeEvent(event);
}

KColorComboBoxPrivate::KColorComboBoxPrivate(KColorComboBox *parent)
    :q_ptr(parent),
      m_comboType(KColorComboBox::Circle),
      m_borderRadius(defaultBorderRadius),
      m_popupItemSize(defaultPopupItemSize)
{
    setParent(parent);
}

void KColorComboBoxPrivate::updateList()
{
    Q_Q(KColorComboBox);
    while (q->count()) {
        q->removeItem(0);
    }
    for (int i = 0 ; i < m_colorList.count(); ++i) {
        q->addItem(QString());
        q->setItemData(i , m_colorList[i], KColorComboBoxDelegate::ColorRole);
    }
    q->update();
}

void KColorComboBoxPrivate::slotActivated(int index)
{
    Q_Q(KColorComboBox);

    m_currentColor = m_colorList[index];
    q->update();
    Q_EMIT q->activated(m_currentColor);
}

void KColorComboBoxPrivate::slotHighlighted(int index)
{
    Q_Q(KColorComboBox);

    auto color = m_colorList[index];
    q->update();
    Q_EMIT q->highlighted(color);
}

void KColorComboBoxPrivate::slotCurrentIndexChanged(int index)
{
    Q_Q(KColorComboBox);

    m_currentColor = m_colorList[index];
    q->update();
    Q_EMIT q->currentColorChanged(m_currentColor);
}

KColorComboBoxDelegate::KColorComboBoxDelegate(QObject *parent,KColorComboBox*combo)
    :QStyledItemDelegate(parent),
      m_combo(combo)
{
}

void KColorComboBoxDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    auto variant = index.data(ItemRoles::ColorRole);
    auto color = variant.value<QColor>();
    auto paintRect = option.rect.adjusted(5,5,-5,-5);

    switch (m_combo->comboType()) {
    case KColorComboBox::Circle:
    {
        if(color.isValid())
        {
            painter->save();
            painter->setRenderHint(QPainter::Antialiasing);
            painter->setPen(Qt::NoPen);
            painter->setBrush(color);
            painter->drawEllipse(paintRect);
            painter->restore();
        }
        if(option.state & (QStyle::State_MouseOver | QStyle::State_Selected))
        {
            painter->save();
            painter->setRenderHint(QPainter::Antialiasing);
            painter->setPen(Qt::NoPen);
            painter->setBrush(Qt::white);
            QRect subRect(paintRect.top(),paintRect.left(),paintRect.width()/2,paintRect.height()/2);
            subRect.moveCenter(paintRect.center());
            painter->drawEllipse(subRect);
            painter->restore();
        }
        break;
    }
    case KColorComboBox::RoundedRect:
    {
        if(color.isValid())
        {
            painter->save();
            painter->setRenderHint(QPainter::Antialiasing);
            if(option.state & (QStyle::State_MouseOver | QStyle::State_Selected))
            {
                QPen pen;
                pen.setWidth(2);
                pen.setBrush(Qt::white);
                painter->setPen(pen);
            }
            else
            {
                painter->setPen(Qt::NoPen);
            }
            painter->setBrush(color);
            painter->drawRoundedRect(paintRect,defaultBorderRadius,defaultBorderRadius);
            painter->restore();
        }
        break;
    }
    default:
        break;
    }
}

QSize KColorComboBoxDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    return m_combo->popupItemSzie();
}

QSize KComboStyle::sizeFromContents(QStyle::ContentsType type, const QStyleOption *option, const QSize &contentsSize, const QWidget *widget) const
{
    if(type == QStyle::CT_ComboBox)
        return g_size;

    return QProxyStyle::sizeFromContents(type,option,contentsSize,widget);
}

}
#include "kcolorcombobox.moc"
#include "moc_kcolorcombobox.cpp"
