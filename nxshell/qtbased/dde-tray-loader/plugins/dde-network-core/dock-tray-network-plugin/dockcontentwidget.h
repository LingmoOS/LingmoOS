// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DOCKCONTENTWIDGET_H
#define DOCKCONTENTWIDGET_H

#include "netmanager.h"
#include "netview.h"
#include "widget/jumpsettingbutton.h"
#include "constants.h"

#include <QDebug>
#include <QEvent>
#include <QStyleOption>
#include <QStylePainter>
#include <QToolButton>
#include <QTreeView>
#include <QVBoxLayout>
#include <QWidget>

namespace dde {
namespace network {

class DockContentWidget : public QWidget
{
    Q_OBJECT
public:
    explicit DockContentWidget(NetView *netView, NetManager *netManager, QWidget *parent = nullptr)
        : QWidget(parent)
        , m_mainLayout(new QVBoxLayout(this))
        , m_netView(netView)
        , m_minHeight(-1)
    {
        m_netView->installEventFilter(this);
        m_netView->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
        connect(m_netView, &NetView::updateSize, this, &DockContentWidget::updateSize);

        m_netSetBtn = new JumpSettingButton(this);
        m_netSetBtn->setIcon(QIcon::fromTheme("network-setting"));
        m_netSetBtn->setDescription(tr("Network settings"));
        connect(m_netSetBtn, &JumpSettingButton::clicked, netManager, &NetManager::gotoControlCenter);

        m_netCheckBtn = new JumpSettingButton(this);
        m_netCheckBtn->setIcon(QIcon::fromTheme("network-check"));
        m_netCheckBtn->setDescription(tr("Network Detection"));
        connect(m_netCheckBtn, &JumpSettingButton::clicked, netManager, &NetManager::gotoCheckNet);

        QWidget *buttonWidget = new QWidget(this);
        QVBoxLayout *buttonLayout = new QVBoxLayout;
        buttonLayout->setContentsMargins(10, 10, 10, 10);
        buttonLayout->setSpacing(10);
        buttonLayout->addWidget(m_netCheckBtn);
        buttonLayout->addWidget(m_netSetBtn);
        buttonWidget->setLayout(buttonLayout);

        m_mainLayout->setContentsMargins(0, 10, 0, 0);
        m_mainLayout->addWidget(m_netView, 0, Qt::AlignTop | Qt::AlignHCenter);
        m_mainLayout->addStretch();
        m_mainLayout->addSpacing(10);
        m_mainLayout->addWidget(buttonWidget, 0, Qt::AlignBottom);
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        setMaximumHeight(Dock::DOCK_POPUP_WIDGET_MAX_HEIGHT);
    }

    ~DockContentWidget() override { }

    void setMainLayoutMargins(const QMargins &margin) {
        m_mainLayout->setContentsMargins(margin);
    }
    void setMinHeight(int minHeight) { m_minHeight = minHeight; }
    void updateSize() {
        auto h = Dock::DOCK_POPUP_WIDGET_MAX_HEIGHT - 20 - m_mainLayout->contentsMargins().top() - (m_netCheckBtn->isVisible() ? (m_netSetBtn->height() + m_netCheckBtn->height() + 10) : m_netSetBtn->height());
        m_netView->setMaxHeight(h);
        if (m_netView->height() > h)
            m_netView->setFixedHeight(h);
        setFixedSize(m_netView->width(), qMax(m_minHeight, m_netView->height() + 20 + m_mainLayout->contentsMargins().top() + (m_netCheckBtn->isVisible() ? (m_netSetBtn->height() + m_netCheckBtn->height() + 10) : m_netSetBtn->height())));
    }
    void setNetCheckBtnVisible(bool visible) {
        m_netCheckBtn->setVisible(visible);
        updateSize();
    }

protected:
    bool eventFilter(QObject *watch, QEvent *event) override
    {
        if ((watch == m_netView && event->type() == QEvent::Resize) || event->type() == QEvent::Show) {
            updateSize();
        }
        return QWidget::eventFilter(watch, event);
    }

private:
    QVBoxLayout *m_mainLayout;
    NetView *m_netView;
    int m_minHeight;
    JumpSettingButton *m_netSetBtn;
    JumpSettingButton *m_netCheckBtn;
};

} // namespace network
} // namespace dde
#endif // DOCKCONTENTWIDGET_H
