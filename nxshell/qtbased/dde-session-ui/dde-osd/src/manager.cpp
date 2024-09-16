// SPDX-FileCopyrightText: 2011 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "manager.h"

#include <QHBoxLayout>
#include <QGSettings>
#include <QGuiApplication>

#include "container.h"
#include "listview.h"
#include "delegate.h"
#include "model.h"

#include "audioprovider.h"
#include "brightnessprovider.h"
#include "kblayoutprovider.h"
#include "displaymodeprovider.h"
#include "osdprovider.h"

Manager::Manager(QObject *parent)
    : QObject(parent)
    , m_container(new Container)
    , m_listview(new ListView)
    , m_delegate(new Delegate)
    , m_model(new Model(this))
    , m_currentProvider(nullptr)
    , m_kbLayoutProvider(new KBLayoutProvider(this))
    , m_timer(new QTimer(this))
{
    m_timer->setSingleShot(true);

    m_listview->setItemDelegate(m_delegate);
    m_listview->setAutoFillBackground(false);
    m_listview->viewport()->setAutoFillBackground(false);
    m_listview->setModel(m_model);
    m_listview->setFrameShape(QFrame::NoFrame);
    m_listview->setAccessibleName("ListView");
    m_container->setContent(m_listview);

    connect(m_listview, &ListView::currentIndexChanged, this, [ = ](const QModelIndex & index) {
        m_currentProvider->sync(index);
    });

    m_providers << new AudioProvider(this) << new BrightnessProvider(this);
    m_providers << m_kbLayoutProvider << new DisplayModeProvider(this);
    m_providers << new OSDProvider(this);

    connect(m_timer, &QTimer::timeout, this, &Manager::doneSetting);
}

void Manager::ShowOSD(const QString &osd)
{
    qDebug() << "show osd" << osd;

    // 3D WM need long time, OSD will disappear too fast
    m_timer->setInterval(osd == "SwitchWM3D" ? 2000 : 1000);

    if (osd == "DirectSwitchLayout") {
        m_kbLayoutProvider->highlightNext();
        m_kbLayoutProvider->sync();

        KBLayoutProvider *provide = qobject_cast<KBLayoutProvider *>(m_kbLayoutProvider);
        if (provide) {
            QModelIndex currentIndex;
            currentIndex = m_listview->model()->index(provide->currentIndex(), 0);
            m_listview->setCurrentIndex(currentIndex);
            m_listview->scrollTo(currentIndex);
        }
        return;
    }

    if (osd == "SwitchLayoutDone") {
        m_timer->start();
        return;
    }

    bool repeat = false;
    for (AbstractOSDProvider *provider : m_providers) {
        if (provider->match(osd)) { // 遍历容器根据osd名称匹配provider
            repeat = (m_currentProvider == provider);
            if (m_currentProvider)
                disconnect(m_currentProvider);

            m_currentProvider = provider;
            connect(provider, &AbstractOSDProvider::dataChanged, this, &Manager::updateUI);
            break;
        }
    }

    if (!m_currentProvider)
        return;

    if (!m_currentProvider->checkConditions() && m_container->isVisible()) {
        m_container->hide();
    }

    updateUI();

    if (repeat && m_container->isVisible()) {
        KBLayoutProvider *provide = qobject_cast<KBLayoutProvider *>(m_currentProvider);
        if (provide) {
            QModelIndex currentIndex = m_listview->model()->index(provide->currentIndex(), 0);
            if (currentIndex.row() < 0)
                currentIndex = m_listview->model()->index(0, 0);

            QModelIndex targetIndex;
            if (currentIndex.row() + 1 >= m_listview->model()->rowCount(QModelIndex())) {
                targetIndex = m_listview->model()->index(0, 0);
            } else {
                targetIndex = currentIndex.sibling(currentIndex.row() + 1, 0);
            }

            m_listview->setCurrentIndex(targetIndex);
            m_listview->scrollTo(targetIndex);
            m_currentProvider->highlightNext();
        } else {
            QModelIndex currentIndex = m_listview->currentIndex();
            if (currentIndex.row() < 0)
                currentIndex = m_listview->model()->index(0, 0);

            QModelIndex targetIndex;
            if (currentIndex.row() + 1 >= m_listview->model()->rowCount(QModelIndex())) {
                targetIndex = m_listview->model()->index(0, 0);
            } else {
                targetIndex = currentIndex.sibling(currentIndex.row() + 1, 0);
            }

            m_listview->setCurrentIndex(targetIndex);
            m_listview->scrollTo(targetIndex);
            m_currentProvider->highlightNext();
        }
    } else {
        KBLayoutProvider *provide = qobject_cast<KBLayoutProvider *>(m_kbLayoutProvider);
        if (provide) {
            QModelIndex currentIndex = m_listview->currentIndex();
            if (currentIndex.row() < 0)
                currentIndex = m_listview->model()->index(0, 0).sibling(provide->currentIndex(), 0);

            m_listview->setCurrentIndex(currentIndex);
            m_listview->scrollTo(currentIndex);
        }
        m_currentProvider->highlightCurrent();
    }

    if (m_currentProvider->checkConditions()) {
        m_container->show();
        m_timer->start();
    } else {
        doneSetting();
    }
}

void Manager::updateUI()
{
    if (!m_currentProvider) return;

    // 键盘和显示列表切换，圆角为18
    if (qobject_cast<DisplayModeProvider *>(m_currentProvider) ||
            qobject_cast<KBLayoutProvider *>(m_currentProvider) ) {
        m_container->updateWindowRadius(18);
    } else {
        m_container->updateWindowRadius();
    }

    // 同一个provider中文字长度不一致也要变化窗体尺寸
    m_container->setFixedSize(m_currentProvider->contentSize());

    if (m_model->provider() != m_currentProvider) {
        m_model->setProvider(m_currentProvider);
        m_delegate->setProvider(m_currentProvider);
        m_listview->setFlow(m_currentProvider->flow());
        m_listview->setCurrentIndex(m_listview->model()->index(m_currentProvider->currentRow(), 0));
        m_container->setContentsMargins(m_currentProvider->contentMargins());
        m_container->moveToCenter();
    }
    if (!m_container->isVisible()) { // 相同模块如果osd已经显示，就不更新OSD位置，避免频繁切换在性能较弱的机器上出现闪烁
        m_model->setProvider(m_currentProvider);
        m_listview->setCurrentIndex(m_listview->model()->index(m_currentProvider->currentRow(), 0));
        m_container->setFixedSize(m_currentProvider->contentSize());
        m_container->moveToCenter();
    }
}

void Manager::doneSetting()
{
    if (qApp->queryKeyboardModifiers().testFlag(Qt::MetaModifier)) {
        return m_timer->start();
    }

    m_container->hide();
    if (m_currentProvider) {
        m_currentProvider->sync();
        m_currentProvider = nullptr;
    }
}
