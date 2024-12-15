// Copyright (C) 2020 ~ 2022 Uniontech Software Co., Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ddropdown.h"

#include <DIconButton>
#include <DLabel>
#include <DMenu>

#include <QDebug>
#include <QHBoxLayout>
#include <QMouseEvent>

class DDropdownPrivate : public QObject
{
public:
    explicit DDropdownPrivate(DDropdown *parent);
    ~DDropdownPrivate() override;

private:
    DMenu *menu = nullptr;
    DLabel *text = nullptr;
    DIconButton *dropdown = nullptr;
    QString status;
    DDropdown *q_ptr;
    Q_DECLARE_PUBLIC(DDropdown)
};

DDropdown::DDropdown(QWidget *parent)
    : DWidget(parent)
    , d_ptr(new DDropdownPrivate(this))
{
    Q_D(DDropdown);
    setObjectName("DDropdown");
    auto layout = new QHBoxLayout(this);
    d->menu = new DMenu(this);
    m_pAllAction = d->menu->addAction(m_strAllText);
    m_pInstalledAction = d->menu->addAction(m_strInstallText);
    m_pUnInstalledAction = d->menu->addAction(m_strUnInstallText);
    d->text = new DLabel(m_strAllText);
    d->text->installEventFilter(this);
    auto font = d->text->font();
    font.setFamily("SourceHanSansSC");
    font.setWeight(QFont::Normal);
    font.setPixelSize(14);
    d->text->setFont(font);
    d->text->setObjectName("DDropdownText");
    d->text->setForegroundRole(DPalette::TextTitle);
    d->dropdown = new DIconButton(DStyle::SP_ArrowDown, this);
    d->dropdown->setFocusPolicy(Qt::NoFocus);
    d->dropdown->setIconSize(QSize(9, 9));
    d->dropdown->setObjectName("DDropdownIcon");
    d->dropdown->setFixedSize(9, 9);
    d->dropdown->setWindowModality(Qt::WindowModal);
    DPalette pl = d->dropdown->palette();
    pl.setColor(DPalette::Light, QColor(Qt::transparent));
    pl.setColor(DPalette::Dark, QColor(Qt::transparent));
    QColor sbcolor("#000000");
    sbcolor.setAlphaF(0);
    pl.setColor(DPalette::Shadow, sbcolor);
    d->dropdown->setPalette(pl);

    layout->addStretch();
    layout->addStretch();
    layout->addWidget(d->text, 0, Qt::AlignCenter);
    layout->setSpacing(10);
    layout->addWidget(d->dropdown, 0, Qt::AlignCenter);
    layout->setContentsMargins(0, 0, 0, 0);

    connect(d->menu, &DMenu::triggered, this, [=](QAction *action) {
        if (m_pAllAction == action) {
            Q_EMIT triggeredAll();
        } else if (m_pInstalledAction == action) {
            Q_EMIT triggeredInstalled();
        } else {
            Q_EMIT triggeredUnInstalled();
        }

        d->text->setText(action->text());
    });

    connect(this, &DDropdown::requestContextMenu, this, [=]() {
        auto center = this->mapToGlobal(this->rect().topLeft());
        center.setY(center.y() + this->height());
        d->menu->move(center);
        d->menu->exec();
    });

    connect(d->dropdown, &DIconButton::clicked, this, [=](bool checked) {
        Q_UNUSED(checked)
        Q_EMIT requestContextMenu();
    });
}

DDropdown::~DDropdown() { }

QString DDropdown::status() const
{
    Q_D(const DDropdown);
    return d->status;
}

QList<QAction *> DDropdown::actions() const
{
    Q_D(const DDropdown);
    return d->menu->actions();
}

bool DDropdown::eventFilter(QObject *watched, QEvent *event)
{
    Q_D(DDropdown);

    if (d->text == watched) {
        if (event->type() == QEvent::MouseButtonPress) {
            QMouseEvent *pEv = static_cast<QMouseEvent *>(event);

            if (pEv->button() == Qt::LeftButton) {
                Q_EMIT requestContextMenu();
            }
        }
    }

    return DWidget::eventFilter(watched, event);
}

void DDropdown::setText(const QString &text)
{
    Q_D(DDropdown);
    d->text->setText(text);
}

void DDropdown::setCurrentAction(QAction *action)
{
    Q_D(DDropdown);
    if (action) {
        for (auto action : d->menu->actions()) {
            action->setChecked(false);
        }
        d->text->setText(action->text());
        action->setChecked(true);
    } else {
        for (auto action : d->menu->actions()) {
            action->setChecked(false);
        }
    }
}

void DDropdown::setCurrentAction(int index)
{
    Q_D(DDropdown);
    if (!d->menu->actions().isEmpty()) {
        setCurrentAction(d->menu->actions().at(index));
    }
}

QAction *DDropdown::addAction(const QString &item, const QVariant &var)
{
    Q_D(DDropdown);
    auto action = d->menu->addAction(item);
    /*d->menu->addSeparator();*/
    auto font = action->font();
    font.setFamily("SourceHanSansSC");
    font.setWeight(QFont::Medium);
    action->setData(var);
    action->setCheckable(true);
    return action;
}

void DDropdown::setStatus(QString status)
{
    Q_D(DDropdown);
    d->status = status;
}

void DDropdown::enterEvent(QEvent *event)
{
    setStatus("hover");
    DWidget::enterEvent(event);

    update();
}

void DDropdown::leaveEvent(QEvent *event)
{
    setStatus("");
    DWidget::leaveEvent(event);

    update();
}

void DDropdown::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        Q_EMIT requestContextMenu();
    }
    DWidget::mouseReleaseEvent(event);
}

DDropdownPrivate::DDropdownPrivate(DDropdown *parent)
    : QObject(parent)
    , q_ptr(parent)
{
}

DDropdownPrivate::~DDropdownPrivate()
{
    if (menu) {
        menu->deleteLater();
        menu = nullptr;
    }

    if (text) {
        text->deleteLater();
        text = nullptr;
    }

    if (dropdown) {
        dropdown->deleteLater();
        dropdown = nullptr;
    }
}
