// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dmessagemanager_p.h"

#include <QQuickItem>
#include <QQmlEngine>
#include <QQmlContext>
#include <QQuickWindow>

DQUICK_BEGIN_NAMESPACE

FloatingMessageContainer::FloatingMessageContainer(QObject *parent)
    : QObject(parent)
{
}

QQuickItem *FloatingMessageContainer::panel() const
{
    return m_panel;
}

QVariant FloatingMessageContainer::message() const
{
    return m_message;
}

void FloatingMessageContainer::setMessage(const QVariant &message)
{
    if (m_message == message)
        return;

    m_message = message;
    Q_EMIT messageChanged();
}

void FloatingMessageContainer::setPanel(QQuickItem *panel)
{
    m_panel = panel;
}

int FloatingMessageContainer::duration() const
{
    return m_duration;
}

void FloatingMessageContainer::setDuration(int duration)
{
    if (m_duration == duration)
        return;

    m_duration = duration;
    Q_EMIT durationChanged();
}

void FloatingMessageContainer::close()
{
    if (auto manager = qobject_cast<MessageManager *>(parent())) {
        manager->close(this);
    }
}

QString FloatingMessageContainer::messageId() const
{
    return m_msgId;
}

void FloatingMessageContainer::setMessageId(const QString &msgId)
{
    m_msgId = msgId;
}


MessageManager::MessageManager(QQuickWindow *parent)
    : QObject(parent)
    , m_delegate(nullptr)
    , m_layout(nullptr)
{
}

QQuickWindow *MessageManager::window() const
{
    return qobject_cast<QQuickWindow*>(parent());
}

bool MessageManager::sendMessage(const QString &content, const QString &iconName, int duration, const QString &msgId)
{
    ensureDelegate();

    QVariantMap message {{"content", content}, {"iconName", iconName}};

    return sendMessage(m_delegate, message, duration, msgId);
}

bool MessageManager::sendMessage(QQmlComponent *delegate, const QVariant &message, int duration, const QString &msgId)
{
    if (FloatingMessageContainer *container = replaceMessage(msgId)) {
        container->setMessage(message);
        return true;
    }

    if (duration >= 0 && reachedUpperLimit())
        return false;

    auto container = beginCreateMessage(delegate);

    container->setMessageId(msgId);
    container->setMessage(message);
    container->setDuration(duration);

    endCreateMessage(delegate, container);
    return true;
}

void MessageManager::close(FloatingMessageContainer *message)
{
    message->panel()->setParentItem(nullptr);
    message->deleteLater();

    // centralized removal to avoid dangling pointer in m_timers.
    for (auto iter = m_timers.begin(); iter != m_timers.end();) {
        if (iter->second == message) {
            iter = m_timers.erase(iter);
            break;
        } else {
            iter++;
        }
    }
}

void MessageManager::close(const QString &msgId)
{
    if (msgId.isEmpty() || !m_layout)
        return;

    for (auto item : messages(msgId)) {
        item->close();
    }
}

void MessageManager::setCapacity(int capacity)
{
    m_capacity = capacity;
}

int MessageManager::count() const
{
    return m_layout ? m_layout->childItems().count() : 0;
}

QList<FloatingMessageContainer *> MessageManager::messages(const QString &msgId) const
{
    QList<FloatingMessageContainer *> result;
    for (auto item : children()) {
        auto container = qobject_cast<FloatingMessageContainer *>(item);
        if (!container)
            continue;

        if (msgId == container->messageId())
            result.push_back(container);
    }
    return result;
}

void MessageManager::setLayout(QQuickItem *layout)
{
    if (m_layout == layout)
        return;

    if (m_layout) {
        disconnect(m_layout, &QQuickItem::childrenChanged, this, &MessageManager::countChanged);
        m_layout->deleteLater();
    }

    m_layout = layout;
    QQmlEngine::setObjectOwnership(m_layout, QQmlEngine::CppOwnership);
    m_layout->setParent(this);
    connect(m_layout, &QQuickItem::childrenChanged, this, &MessageManager::countChanged);
    if (window()) {
        m_layout->setParentItem(window()->contentItem());
    }
}

void MessageManager::ensureLayout()
{
    if (!m_layout) {
        QQmlComponent columnCom(qmlEngine(parent()));
        columnCom.setData("import QtQuick 2.11\n"
                          "Column {\n"
                          "  anchors {\n"
                          "      bottom: parent.bottom\n"
                          "      bottomMargin: 10;\n"
                          "      horizontalCenter: parent.horizontalCenter\n"
                          "  }\n"
                          "}\n", QUrl());
        auto layout = columnCom.beginCreate(qmlContext(parent()));
        setLayout(qobject_cast<QQuickItem *>(layout));
        columnCom.completeCreate();
    }
}

void MessageManager::ensureDelegate()
{
    if (!m_delegate) {
        QQmlComponent *com = new QQmlComponent(qmlEngine(parent()));
        com->setData("import org.deepin.dtk 1.0 as D\n"
                     "D.FloatingMessage {}\n",
                     QUrl());
        setDelegate(com);
    }
}

bool MessageManager::reachedUpperLimit() const
{
    return m_timers.count() >= m_capacity;
}

FloatingMessageContainer *MessageManager::replaceMessage(const QString &msgId)
{
    if (msgId.isEmpty())
        return nullptr;

    for (int i = 0; i < m_timers.size(); ++i) {
        auto &item = m_timers[i];
        if (item.second && item.second->messageId() == msgId) {
            // restart timer for the quickitem.
            killTimer(item.first);
            auto message = item.second;
            int duration = message->duration();
            int timerId = startTimer(duration);
            item.first = timerId;

            stackBeforeMessage(message->panel());

            return message;
        }
    }

    return nullptr;
}

void MessageManager::stackBeforeMessage(QQuickItem *message)
{
    // `message` should be top in layout.
    const auto &childItems = m_layout->childItems();
    if (childItems.count() > 1 && childItems.front() != message) {
        message->stackBefore(childItems.first());
    }
}

FloatingMessageContainer *MessageManager::beginCreateMessage(QQmlComponent *component)
{
    // create new quickitem.
    ensureLayout();

    auto context = component->creationContext();
    if (!context) {
        context = qmlContext(parent());
    }
    auto message = qobject_cast<FloatingMessageContainer *>(component->beginCreate(context));
    Q_ASSERT_X(message, __FUNCTION__, qPrintable(component->errorString()));
    message->panel()->setParentItem(m_layout);
    /* Need to recode the `MessageManager` for closing.
     `MessageManager` maybe can't be find by window because of window changing. */
    message->setParent(this);
    return message;
}

void MessageManager::endCreateMessage(QQmlComponent *component, FloatingMessageContainer *container)
{
    component->completeCreate();

    int duration = container->duration();
    if (duration > 0) {
        int timerId = startTimer(duration);
        m_timers.append(qMakePair(timerId, container));
    }

    stackBeforeMessage(container->panel());
}

void MessageManager::setDelegate(QQmlComponent *delegate)
{
    if (m_delegate == delegate)
        return;

    if (m_delegate)
        m_delegate->deleteLater();

    m_delegate = delegate;
    QQmlEngine::setObjectOwnership(m_delegate, QQmlEngine::CppOwnership);
    m_delegate->setParent(this);
}

int MessageManager::capacity() const
{
    return m_capacity;
}

void MessageManager::timerEvent(QTimerEvent *e)
{
    for (auto item : m_timers) {
        if (item.first != e->timerId())
            continue;

        killTimer(e->timerId());
        e->accept();
        if (auto container = item.second) {
            close(container);
        }
        break;
    }
    QObject::timerEvent(e);
}

QQuickItem *MessageManager::layout() const
{
    return m_layout;
}

QQmlComponent *MessageManager::delegate() const
{
    return m_delegate;
}

MessageManager *MessageManager::qmlAttachedProperties(QObject *object)
{
    if (QQuickWindow *window = qobject_cast<QQuickWindow *>(object))
        return new MessageManager(window);

    return nullptr;
}

DQUICK_END_NAMESPACE

#include "moc_dmessagemanager_p.cpp"
