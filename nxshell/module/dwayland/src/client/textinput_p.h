/*
    SPDX-FileCopyrightText: 2016 Martin Gräßlin <mgraesslin@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#ifndef KWAYLAND_CLIENT_TEXTINPUT_P_H
#define KWAYLAND_CLIENT_TEXTINPUT_P_H
#include "textinput.h"

#include <QObject>

struct wl_text_input;
struct wl_text_input_manager;
struct zwp_text_input_v2;

namespace KWayland
{
namespace Client
{
class EventQueue;
class TextInputUnstableV0;
class Surface;
class Seat;

class TextInputManagerUnstableV0 : public TextInputManager
{
    Q_OBJECT
public:
    /**
     * Creates a new TextInputManagerUnstableV0.
     * Note: after constructing the TextInputManagerUnstableV0 it is not yet valid and one needs
     * to call setup. In order to get a ready to use TextInputManagerUnstableV0 prefer using
     * Registry::createTextInputManagerUnstableV0.
     **/
    explicit TextInputManagerUnstableV0(QObject *parent = nullptr);
    ~TextInputManagerUnstableV0() override;

private:
    class Private;
    Private *d_func() const;
};

class TextInputManagerUnstableV2 : public TextInputManager
{
    Q_OBJECT
public:
    /**
     * Creates a new TextInputManagerUnstableV0.
     * Note: after constructing the TextInputManagerUnstableV0 it is not yet valid and one needs
     * to call setup. In order to get a ready to use TextInputManagerUnstableV0 prefer using
     * Registry::createTextInputManagerUnstableV0.
     **/
    explicit TextInputManagerUnstableV2(QObject *parent = nullptr);
    ~TextInputManagerUnstableV2() override;

private:
    class Private;
    QScopedPointer<Private> d;
};

class Q_DECL_HIDDEN TextInputManager::Private
{
public:
    Private() = default;
    virtual ~Private() = default;

    virtual void release() = 0;
    virtual void destroy() = 0;
    virtual bool isValid() = 0;
    virtual void setupV0(wl_text_input_manager *textinputmanagerunstablev0)
    {
        Q_UNUSED(textinputmanagerunstablev0)
    }
    virtual void setupV2(zwp_text_input_manager_v2 *textinputmanagerunstablev2)
    {
        Q_UNUSED(textinputmanagerunstablev2)
    }
    virtual TextInput *createTextInput(Seat *seat, QObject *parent = nullptr) = 0;
    virtual operator wl_text_input_manager *()
    {
        return nullptr;
    }
    virtual operator wl_text_input_manager *() const
    {
        return nullptr;
    }
    virtual operator zwp_text_input_manager_v2 *()
    {
        return nullptr;
    }
    virtual operator zwp_text_input_manager_v2 *() const
    {
        return nullptr;
    }

    EventQueue *queue = nullptr;
};

class Q_DECL_HIDDEN TextInput::Private
{
public:
    Private(Seat *seat);
    virtual ~Private() = default;

    virtual bool isValid() const = 0;
    virtual void enable(Surface *surface) = 0;
    virtual void disable(Surface *surface) = 0;
    virtual void showInputPanel() = 0;
    virtual void hideInputPanel() = 0;
    virtual void setCursorRectangle(const QRect &rect) = 0;
    virtual void setPreferredLanguage(const QString &lang) = 0;
    virtual void setSurroundingText(const QString &text, quint32 cursor, quint32 anchor) = 0;
    virtual void reset() = 0;
    virtual void setContentType(ContentHints hint, ContentPurpose purpose) = 0;

    EventQueue *queue = nullptr;
    Seat *seat;
    Surface *enteredSurface = nullptr;
    quint32 latestSerial = 0;
    bool inputPanelVisible = false;
    Qt::LayoutDirection textDirection = Qt::LayoutDirectionAuto;
    QByteArray language;

    struct PreEdit {
        QByteArray text;
        QByteArray commitText;
        qint32 cursor = 0;
        bool cursorSet = false;
    };
    PreEdit currentPreEdit;
    PreEdit pendingPreEdit;

    struct Commit {
        QByteArray text;
        qint32 cursor = 0;
        qint32 anchor = 0;
        DeleteSurroundingText deleteSurrounding;
    };
    Commit currentCommit;
    Commit pendingCommit;
};

class TextInputUnstableV0 : public TextInput
{
    Q_OBJECT
public:
    explicit TextInputUnstableV0(Seat *seat, QObject *parent = nullptr);
    ~TextInputUnstableV0() override;

    /**
     * Setup this TextInputUnstableV0 to manage the @p textinputunstablev0.
     * When using TextInputManagerUnstableV0::createTextInputUnstableV0 there is no need to call this
     * method.
     **/
    void setup(wl_text_input *textinputunstablev0);
    /**
     * Releases the wl_text_input interface.
     * After the interface has been released the TextInputUnstableV0 instance is no
     * longer valid and can be setup with another wl_text_input interface.
     **/
    void release();
    /**
     * Destroys the data held by this TextInputUnstableV0.
     * This method is supposed to be used when the connection to the Wayland
     * server goes away. If the connection is not valid anymore, it's not
     * possible to call release anymore as that calls into the Wayland
     * connection and the call would fail. This method cleans up the data, so
     * that the instance can be deleted or set up to a new wl_text_input interface
     * once there is a new connection available.
     *
     * It is suggested to connect this method to ConnectionThread::connectionDied:
     * @code
     * connect(connection, &ConnectionThread::connectionDied, textinputunstablev0, &TextInputUnstableV0::destroy);
     * @endcode
     *
     * @see release
     **/
    void destroy();

    operator wl_text_input *();
    operator wl_text_input *() const;

private:
    class Private;
    Private *d_func() const;
};

class TextInputUnstableV2 : public TextInput
{
    Q_OBJECT
public:
    explicit TextInputUnstableV2(Seat *seat, QObject *parent = nullptr);
    ~TextInputUnstableV2() override;

    /**
     * Setup this TextInputUnstableV2 to manage the @p textinputunstablev2.
     * When using TextInputManagerUnstableV2::createTextInputUnstableV2 there is no need to call this
     * method.
     **/
    void setup(zwp_text_input_v2 *textinputunstablev2);
    /**
     * Releases the zwp_text_input_v2 interface.
     * After the interface has been released the TextInputUnstableV2 instance is no
     * longer valid and can be setup with another zwp_text_input_v2 interface.
     **/
    void release();
    /**
     * Destroys the data held by this TextInputUnstableV2.
     * This method is supposed to be used when the connection to the Wayland
     * server goes away. If the connection is not valid anymore, it's not
     * possible to call release anymore as that calls into the Wayland
     * connection and the call would fail. This method cleans up the data, so
     * that the instance can be deleted or set up to a new zwp_text_input_v2 interface
     * once there is a new connection available.
     *
     * It is suggested to connect this method to ConnectionThread::connectionDied:
     * @code
     * connect(connection, &ConnectionThread::connectionDied, textinputunstablev2, &TextInputUnstableV2::destroy);
     * @endcode
     *
     * @see release
     **/
    void destroy();

    operator zwp_text_input_v2 *();
    operator zwp_text_input_v2 *() const;

private:
    class Private;
    Private *d_func() const;
};

}
}

#endif
