KNotifications QML API   {#qml_api}
==================

-   [Basic Notifications](#basic)
-   [Inline Reply Notification](#inline_replies)
-   [Creating Notifications Dynamically](#dynamic_notifications)
-   [Further References](#refs)

<a name="basic">

## Basic Notifications

Getting started with notifications in QML is quite similar to how this also works
in C++:
\li Create a `KNotificiation` instance.
\li Customize it to your needs via its properties (labels, icons, actions, priorities, etc).
\li Connect to its signals to react to the user interacting with the notification, by triggering its inline actions or by closing it.
\li Eventually call its `KNotification::sendEvent()` method to show or re-show the notification.

~~~
import QtQuick 2.15
import org.kde.notification 1.0

... {
    Notification {
        id: myNotification
        componentName: "your_component"
        eventId: "your_event"
        title: "Attention!"
        text: "Something important happened."
        iconName: "kde"
        flags: Notification.Persistent
        urgency: Notification.HighUrgency
        onClosed: console.log("Notification closed.")
        actions: [
            NotificationAction {
                label: "Action 1"
                onActivated: log.append("Action 1 activated.")
            },
            NotificationAction {
                label: "Action 2"
                onActivated: log.append("Action 2 activated.")
            },
            NotificationAction {
                label: "Action 3"
                onActivated: log.append("Action 3 activated.")
            }
        ]
    }

    Button {
        onClicked: myNotification.sendEvent()
    }
}
~~~

<a name="inline_replies">

## Inline Reply Notifications

Inline reply notifications are also available. There's a small difference to C++
in that the reply action doesn't need to be explicitly managed but is created on
demand behind the scenes.

~~~
import QtQuick 2.15
import org.kde.notification 1.0

... {
    Notification {
        id: myReplyNotification
        componentName: "plasma_workspace"
        eventId: "notification"
        title: "Chat message from Dr Konqui"
        text: "How are you?"
        replyAction {
            label: "Reply"
            placeholderText: "Reply to Dr Konqi..."
            submitButtonText: "Send Reply"
            submitButtonIconName: "mail-reply-all"
            onReplied: console.log(text)
        }
    }
}
~~~

<a name="dynamic_notification">

## Creating Notifications Dynamically

You might have a use-case that requires an arbitrary amount of dynamically created
notifications rather than a fixed set of reusable instances. In that case the approach
shown above might be too restrictive.

To address this it is possible to configure the auto-delete behavior of `KNotification` objects.
When used from C++ it's on by default for compatibility (but can also be switched off),
in QML it's off by default and can be switched on when needed as shown in the following example.

~~~
import QtQuick 2.15
import org.kde.notification 1.0

... {
    Component {
        id: notificationComponent
        Notification {
            componentName: "plasma_workspace"
            eventId: "notification"
            text: "Temporary notification we can create new instances of."
            autoDelete: true
        }
    }

    Button {
        property int count: 0
        onClicked: {
            var notification = notificationComponent.createObject(parent);
            notification.title = "New Notification " + count;
            n.sendEvent();
            ++count;
        }
    }
}
~~~

<a name="refs">

## Further References

There's a more complete and actually runnable pure-QML example in `examples/notificationtester.qml`
provided as part of the KNotifications source code.
