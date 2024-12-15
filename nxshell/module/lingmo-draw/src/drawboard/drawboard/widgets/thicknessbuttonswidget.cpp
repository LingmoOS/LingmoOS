// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "thicknessbuttonswidget.h"

#include<QLabel>
#include<QPushButton>
#include<QList>
#include<QPicture>
#include<QPainter>
#include<QtMath>
#include<QHBoxLayout>
#include<QButtonGroup>

const QSize CIRCLE_BORDER_SIZE = QSize(37, 37);
class ThicknessButtonsWidget::ThicknessButtonsWidget_private
{
public:
    explicit ThicknessButtonsWidget_private(ThicknessButtonsWidget *qq) : q(qq)
    {
        m_label = new QLabel(q);
        m_circleBtnGroup = new QButtonGroup(q);
    }

    circleIconPushButton *createButton(int width)
    {
        circleIconPushButton *btn = new circleIconPushButton(width, q);
        m_circleBtnGroup->addButton(btn);
        return btn;
    }

    void addLaybel()
    {
        m_label->resize(CIRCLE_BORDER_SIZE);
        q->layout()->addWidget(m_label);
    }

    void setText(QString t)
    {
        m_label->setText(t);
    }

    ThicknessButtonsWidget *q;
    QLabel  *m_label = nullptr;
    QButtonGroup    *m_circleBtnGroup = nullptr;
};

ThicknessButtonsWidget::ThicknessButtonsWidget(QWidget *parent) : QWidget(parent)
    , ThicknessButtonsWidget_d(new ThicknessButtonsWidget_private(this))
{
    InitLayout();
    ThicknessButtonsWidget_d->addLaybel();
    setText(QObject::tr(""));
}

void ThicknessButtonsWidget::setText(const QString &str)
{
    ThicknessButtonsWidget_d->setText(str);
}

void ThicknessButtonsWidget::add(int lineWidth)
{
    auto btn = ThicknessButtonsWidget_d->createButton(lineWidth);
    connect(btn, &circleIconPushButton::clicked, this, [ = ] {
        emit valueChanged(qobject_cast<circleIconPushButton *>(sender())->penWidth(), EChanged);
    });
    layout()->addWidget(btn);
}

//void ThicknessButtonsWidget::addDefaultButtons()
//{
//    removeAllButtons();
//    for (int i = 1; i <= 7; ++i) {
//        add(i);
//    }
//}

void ThicknessButtonsWidget::removeAllButtons()
{
    auto l = qobject_cast<QHBoxLayout *>(layout());
    if (nullptr == l) {
        return;
    }

    while (l->count() > 1) {
        circleIconPushButton *btn = dynamic_cast<circleIconPushButton *>(l->takeAt(l->count() - 1));
        if (nullptr != btn) {
            btn->deleteLater();
        }
    }
}

void ThicknessButtonsWidget::addButtons(int *arr, int count)
{
    for (int i = 0; i < count; ++i) {
        add(arr[i]);
    }
}

QSize ThicknessButtonsWidget::sizeHint() const
{
    return QSize(334, QWidget::sizeHint().height());
}

QSize ThicknessButtonsWidget::minimumSizeHint() const
{
    return QSize(334, QWidget::minimumSizeHint().height());
}

void ThicknessButtonsWidget::setTextVisible(bool bVisible)
{
    if (!bVisible)
        ThicknessButtonsWidget_d->m_label->hide();
    else
        ThicknessButtonsWidget_d->m_label->show();
}

void ThicknessButtonsWidget::InitLayout()
{
    if (nullptr != layout()) {
        delete layout();
    }

    QHBoxLayout *hLayout = new QHBoxLayout(this);
    hLayout->setContentsMargins(0, 0, 0, 0);
    hLayout->setSpacing(4);
    setLayout(hLayout);
}

void ThicknessButtonsWidget::setValue(int value)
{
    //没有当前值的时候，不选中
    ThicknessButtonsWidget_d->m_circleBtnGroup->setExclusive(false);
    for (int i = 1; i < qobject_cast<QHBoxLayout *>(layout())->count(); ++i) {
        circleIconPushButton *btn = dynamic_cast<circleIconPushButton *>(layout()->itemAt(i)->widget());
        if (nullptr == btn) {
            continue;
        }

        btn->setChecked(false);

        if (btn->penWidth() == value) {
            btn->setChecked(true);
        }
    }
    ThicknessButtonsWidget_d->m_circleBtnGroup->setExclusive(true);
}

circleIconPushButton::circleIconPushButton(int penWidth, QWidget *parent): QPushButton(parent)
{
    setFocusPolicy(Qt::NoFocus);
    m_penWidth = penWidth;
    setCheckable(true);

    connect(this, &circleIconPushButton::clicked, this, [ = ]() {
        this->setChecked(true);
    });
}

int circleIconPushButton::penWidth()
{
    return m_penWidth;
}

QSize circleIconPushButton::sizeHint() const
{
    return CIRCLE_BORDER_SIZE;
}

void circleIconPushButton::paintEvent(QPaintEvent *e)
{
    QPushButton::paintEvent(e);

    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing);
    painter.setBrush(QBrush(Qt::black));
    QPen pen(Qt::black);
    if (isChecked()) {
        pen.setColor(Qt::white);
    }
    pen.setJoinStyle(Qt::RoundJoin);
    pen.setCapStyle(Qt::RoundCap);
    pen.setWidth(m_penWidth);
    painter.setPen(pen);
    painter.drawPoint(rect().center());

}
