/*
    SPDX-FileCopyrightText: 2012-2013 Dominik Haumann <dhaumann@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "ktexteditor/message.h"

#include <QAction>
#include <QIcon>

namespace KTextEditor
{
class MessagePrivate
{
public:
    QList<QAction *> actions;
    Message::MessageType messageType;
    Message::MessagePosition position = Message::AboveView;
    QString text;
    QIcon icon;
    bool wordWrap = false;
    int autoHideDelay = -1;
    KTextEditor::Message::AutoHideMode autoHideMode = KTextEditor::Message::AfterUserInteraction;
    int priority = 0;
    KTextEditor::View *view = nullptr;
    KTextEditor::Document *document = nullptr;
};

Message::Message(const QString &richtext, MessageType type)
    : d(new MessagePrivate())
{
    d->messageType = type;
    d->text = richtext;
}

Message::~Message()
{
    Q_EMIT closed(this);

    delete d;
}

QString Message::text() const
{
    return d->text;
}

void Message::setText(const QString &text)
{
    if (d->text != text) {
        d->text = text;
        Q_EMIT textChanged(text);
    }
}

void Message::setIcon(const QIcon &newIcon)
{
    d->icon = newIcon;
    Q_EMIT iconChanged(d->icon);
}

QIcon Message::icon() const
{
    return d->icon;
}

Message::MessageType Message::messageType() const
{
    return d->messageType;
}

void Message::addAction(QAction *action, bool closeOnTrigger)
{
    // make sure this is the parent, so all actions are deleted in the destructor
    action->setParent(this);
    d->actions.append(action);

    // call close if wanted
    if (closeOnTrigger) {
        connect(action, &QAction::triggered, this, &QObject::deleteLater);
    }
}

QList<QAction *> Message::actions() const
{
    return d->actions;
}

void Message::setAutoHide(int delay)
{
    d->autoHideDelay = delay;
}

int Message::autoHide() const
{
    return d->autoHideDelay;
}

void Message::setAutoHideMode(KTextEditor::Message::AutoHideMode mode)
{
    d->autoHideMode = mode;
}

KTextEditor::Message::AutoHideMode Message::autoHideMode() const
{
    return d->autoHideMode;
}

void Message::setWordWrap(bool wordWrap)
{
    d->wordWrap = wordWrap;
}

bool Message::wordWrap() const
{
    return d->wordWrap;
}

void Message::setPriority(int priority)
{
    d->priority = priority;
}

int Message::priority() const
{
    return d->priority;
}

void Message::setView(KTextEditor::View *view)
{
    d->view = view;
}

KTextEditor::View *Message::view() const
{
    return d->view;
}

void Message::setDocument(KTextEditor::Document *document)
{
    d->document = document;
}

KTextEditor::Document *Message::document() const
{
    return d->document;
}

void Message::setPosition(Message::MessagePosition position)
{
    d->position = position;
}

Message::MessagePosition Message::position() const
{
    return d->position;
}

} // namespace KTextEditor
