/*
    SPDX-FileCopyrightText: 2012 Dominik Haumann <dhaumann@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KATE_MESSAGE_WIDGET_H
#define KATE_MESSAGE_WIDGET_H

#include <QHash>
#include <QPointer>
#include <QWidget>

#include <ktexteditor_export.h>

namespace KTextEditor
{
class Message;
class ViewPrivate;
}

class KMessageWidget;
class KateAnimation;

/**
 * This class implements a message widget based on KMessageWidget.
 * It is used to show messages through the KTextEditior::MessageInterface.
 */
class KateMessageWidget : public QWidget
{
    Q_OBJECT

public:
    /**
     * The position of the KateMessageWidget
     *
     * @see KMessageWidget::Position
     */
    enum class Position {
        Inline, ///< Inline message.
        Header, ///< Message positioned at the top of the view.
        Footer, ///< Message positioned at the bottom of the view.
    };

    /**
     * Constructor. By default, the widget is hidden.
     */
    explicit KateMessageWidget(QWidget *parent, bool applyFadeEffect = false);

    /**
     * Post a new incoming message. Show either directly, or queue
     */
    void postMessage(KTextEditor::Message *message, QList<std::shared_ptr<QAction>> actions);

    // for unit test
    KTEXTEDITOR_EXPORT QString text() const;

    void setPosition(Position position);

protected Q_SLOTS:
    /**
     * Show the next message in the queue.
     */
    void showNextMessage();

    /**
     * Helper that enables word wrap to avoid breaking the layout
     */
    void setWordWrap(KTextEditor::Message *message);

    /**
     * catch when a message is deleted, then show next one, if applicable.
     */
    void messageDestroyed(KTextEditor::Message *message);

    // ViewPrivate calls startAutoHideTimer()
    friend class KTextEditor::ViewPrivate;
    /**
     * Start autoHide timer if requested
     */
    void startAutoHideTimer();
    /**
     * User hovers on a link in the message widget.
     */
    void linkHovered(const QString &link);

private:
    // sorted list of pending messages
    QList<KTextEditor::Message *> m_messageQueue;
    // pointer to current Message
    QPointer<KTextEditor::Message> m_currentMessage;
    // shared pointers to QActions as guard
    QHash<KTextEditor::Message *, QList<std::shared_ptr<QAction>>> m_messageHash;
    // the message widget, showing the actual contents
    KMessageWidget *m_messageWidget;
    // the show / hide effect controller
    KateAnimation *m_animation;

private: // some state variables
    // autoHide only once user interaction took place
    QTimer *m_autoHideTimer;
    // flag: save message's autohide time
    int m_autoHideTime;
};

#endif
