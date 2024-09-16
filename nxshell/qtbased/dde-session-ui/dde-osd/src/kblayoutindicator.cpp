// SPDX-FileCopyrightText: 2011 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "kblayoutindicator.h"

#include <DDBusSender>

#include <QDBusConnection>
#include <QPainter>
#include <QApplication>
#include <QMouseEvent>

DataWrapper::DataWrapper(KeyboardInterface *kinter, QObject *parent) :
    QObject(parent),
    m_keyboardInter(kinter)
{
    m_keyboardInter->setSync(false, false);

    auto keyboardIsValid = [=] (bool isvalid) {
        if (isvalid) {
            initLayoutListAll();
            setLayoutList(m_keyboardInter->userLayoutList());
            setCurrentLayout(m_keyboardInter->currentLayout());
        }
    };

    connect(m_keyboardInter, &KeyboardInterface::UserLayoutListChanged,
            this, &DataWrapper::setLayoutList);
    connect(m_keyboardInter, &KeyboardInterface::CurrentLayoutChanged,
            this, &DataWrapper::setCurrentLayout);
    connect(m_keyboardInter, &KeyboardInterface::serviceValidChanged,
            this, keyboardIsValid);

    keyboardIsValid(m_keyboardInter->isValid());
}

QString DataWrapper::currentLayout() const
{
    return m_currentLayout;
}

void DataWrapper::setCurrentLayout(const QString &currentLayout)
{
    qDebug() << "current layout changed to: " << currentLayout;

    if (currentLayout == m_currentLayout)
        return;

    m_currentLayout = currentLayout;
    emit currentLayoutChanged(m_currentLayout);
}

QStringList DataWrapper::layoutList() const
{
    return m_layoutList;
}

void DataWrapper::setLayoutList(const QStringList &layoutList)
{
    qDebug() << "user layouts changed to: " << layoutList;

    if (layoutList == m_layoutList)
        return;

    m_layoutList = layoutList;
    emit layoutListChanged(layoutList);
}

void DataWrapper::initLayoutListAll()
{
    QDBusPendingCall call = m_keyboardInter->LayoutList();
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(call, this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [this, call] {
        if (call.isError()) {
            qWarning() << "failed to get all keyboard list: " << call.error().message();
        } else {
            QDBusReply<KeyboardLayoutList> reply = call.reply();
            m_layoutListAll = reply.value();
        }
    });
}

QString DataWrapper::localizedNameOfLayout(const QString &layout) const
{
    return m_layoutListAll.value(layout);
}

QStringList DataWrapper::userLayoutList() const
{
    return m_keyboardInter->userLayoutList();
}

KBLayoutIndicator::KBLayoutIndicator(QWidget *parent)
    : QSystemTrayIcon(parent),
      m_keyboardInter(new KeyboardInterface("org.deepin.dde.InputDevices1",
                                            "/org/deepin/dde/InputDevice1/Keyboard",
                                            QDBusConnection::sessionBus(),
                                            this)),
      m_data(new DataWrapper(m_keyboardInter, this)),
      m_menu(new QMenu),
      m_addLayoutAction(nullptr)
{
    updateIcon();
    updateMenu();

    connect(m_data, &DataWrapper::layoutListChanged,
            this, &KBLayoutIndicator::handleDataChanged);
    connect(m_data, &DataWrapper::currentLayoutChanged,
            this, &KBLayoutIndicator::handleDataChanged);

    connect(m_menu, &QMenu::triggered,
            this, &KBLayoutIndicator::handleActionTriggered);

    connect(this, &KBLayoutIndicator::activated,
            this, &KBLayoutIndicator::handleActivated);
}

void KBLayoutIndicator::updateMenu()
{
    if (m_menu->actions().length())
        m_menu->clear();

    setContextMenu(m_menu);

    for (const QString &layout : m_data->layoutList()) {
        const QString layoutName = duplicateCheck(layout);
        const QString layoutLocalizedName = m_data->localizedNameOfLayout(layout);
        const QString text = QString("%1 (%2)").arg(layoutLocalizedName).arg(layoutName);

        QAction *action = new QAction(text, m_menu);
        action->setObjectName(layout);
        action->setCheckable(true);
        action->setChecked(layout == m_data->currentLayout());
        m_menu->addAction(action);
    }

    m_menu->addSeparator();

    m_addLayoutAction = new QAction(tr("Add keyboard layout"), m_menu);

    m_menu->addAction(m_addLayoutAction);
}

void KBLayoutIndicator::updateIcon()
{
    const qreal dpr = qApp->devicePixelRatio();
    const QString layout = duplicateCheck(m_data->currentLayout());

    QPixmap pixNormal = generateIconPixmap(layout);
    QPixmap pixHiDPI = generateIconPixmap(layout, dpr);

    QIcon icon;
    icon.addPixmap(pixNormal);
    icon.addPixmap(pixHiDPI);

    setIcon(icon);
}

QString KBLayoutIndicator::duplicateCheck(const QString &kb)
{
    QStringList list;
    const QString kbFirst = kb.split(";").first();
    for (const QString &data : m_data->userLayoutList()) {
        if (data.split(";").first() == kbFirst) {
            list << data;
        }
    }

    const QString kblayout = kb.split(";").first().mid(0, 2);

    return kblayout + (list.count() > 1 ? QString::number(list.indexOf(kb) + 1) : "");
}

QPixmap KBLayoutIndicator::generateIconPixmap(const QString &layout, qreal dpr)
{
    const int iconSize = 16;

    QPixmap pix(100, 100);
    pix.fill(Qt::transparent);

    QPainter pa(&pix);
    pa.setRenderHints(pa.renderHints() | QPainter::TextAntialiasing);
    pa.setPen(Qt::white);

    QFont font = pa.font();
    font.setPointSizeF(70);
    font.setWeight(QFont::Medium);
    pa.setFont(font);

    QTextOption op;
    op.setAlignment(Qt::AlignCenter);

    QRect r( pix.rect() );
    r.setTop(r.top() - 14);
    pa.drawText(r, layout, op);

    QPixmap ret = pix.scaled(QSize(static_cast<int>(iconSize * dpr), static_cast<int>(iconSize * dpr)),
                             Qt::KeepAspectRatio, Qt::SmoothTransformation);
    ret.setDevicePixelRatio(dpr);

    return ret;
}

void KBLayoutIndicator::switchToNextLayout()
{
    const QString layout = m_data->currentLayout();
    const QStringList layouts = m_data->layoutList();

    const int currentIdx = layouts.indexOf(layout);
    const QString targetLayout = layouts.at((currentIdx + 1) % layouts.length());

    m_keyboardInter->setCurrentLayout(targetLayout);
}

void KBLayoutIndicator::handleDataChanged()
{
    if (m_data->layoutList().length() < 2)
        return setVisible(false);

    updateIcon();
    updateMenu();
    show();
}

void KBLayoutIndicator::handleActionTriggered(QAction *action)
{
    if (action == m_addLayoutAction) {
        DDBusSender()
                .service("org.deepin.dde.ControlCenter1")
                .interface("org.deepin.dde.ControlCenter1")
                .path("/org/deepin/dde/ControlCenter1")
                .method(QString("ShowModule"))
                .arg(QString("keyboard"))
                .call();
    }

    const QString layout = action->objectName();
    if (m_data->layoutList().contains(layout)) {
        m_keyboardInter->setCurrentLayout(layout);
    }
}

void KBLayoutIndicator::handleActivated(QSystemTrayIcon::ActivationReason reason)
{
    if (reason == QSystemTrayIcon::Trigger) {
        switchToNextLayout();
    }
}

