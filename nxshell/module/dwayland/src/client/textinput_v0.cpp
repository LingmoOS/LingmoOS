/*
    SPDX-FileCopyrightText: 2016 Martin Gräßlin <mgraesslin@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#include "event_queue.h"
#include "seat.h"
#include "surface.h"
#include "textinput_p.h"
#include "wayland_pointer_p.h"

#include <wayland-text-input-v0-client-protocol.h>

namespace KWayland
{
namespace Client
{
class TextInputUnstableV0::Private : public TextInput::Private
{
public:
    Private(TextInputUnstableV0 *q, Seat *seat);

    void setup(wl_text_input *textinputmanagerunstablev0);

    bool isValid() const override;
    void enable(Surface *surface) override;
    void disable(Surface *surface) override;
    void showInputPanel() override;
    void hideInputPanel() override;
    void setCursorRectangle(const QRect &rect) override;
    void setPreferredLanguage(const QString &lang) override;
    void setSurroundingText(const QString &text, quint32 cursor, quint32 anchor) override;
    void reset() override;
    void setContentType(ContentHints hint, ContentPurpose purpose) override;

    WaylandPointer<wl_text_input, wl_text_input_destroy> textinputunstablev0;

private:
    static void enterCallaback(void *data, wl_text_input *wl_text_input, wl_surface *surface);
    static void leaveCallback(void *data, wl_text_input *wl_text_input);
    static void modifiersMapCallback(void *data, wl_text_input *wl_text_input, wl_array *map);
    static void inputPanelStateCallback(void *data, wl_text_input *wl_text_input, uint32_t state);
    static void preeditStringCallback(void *data, wl_text_input *wl_text_input, uint32_t serial, const char *text, const char *commit);
    static void preeditStylingCallback(void *data, wl_text_input *wl_text_input, uint32_t index, uint32_t length, uint32_t style);
    static void preeditCursorCallback(void *data, wl_text_input *wl_text_input, int32_t index);
    static void commitStringCallback(void *data, wl_text_input *wl_text_input, uint32_t serial, const char *text);
    static void cursorPositionCallback(void *data, wl_text_input *wl_text_input, int32_t index, int32_t anchor);
    static void deleteSurroundingTextCallback(void *data, wl_text_input *wl_text_input, int32_t index, uint32_t length);
    static void keysymCallback(void *data, wl_text_input *wl_text_input, uint32_t serial, uint32_t time, uint32_t sym, uint32_t state, uint32_t modifiers);
    static void languageCallback(void *data, wl_text_input *wl_text_input, uint32_t serial, const char *language);
    static void textDirectionCallback(void *data, wl_text_input *wl_text_input, uint32_t serial, uint32_t direction);

    TextInputUnstableV0 *q;

    static const wl_text_input_listener s_listener;
};

const wl_text_input_listener TextInputUnstableV0::Private::s_listener = {enterCallaback,
                                                                         leaveCallback,
                                                                         modifiersMapCallback,
                                                                         inputPanelStateCallback,
                                                                         preeditStringCallback,
                                                                         preeditStylingCallback,
                                                                         preeditCursorCallback,
                                                                         commitStringCallback,
                                                                         cursorPositionCallback,
                                                                         deleteSurroundingTextCallback,
                                                                         keysymCallback,
                                                                         languageCallback,
                                                                         textDirectionCallback};

void TextInputUnstableV0::Private::enterCallaback(void *data, wl_text_input *wl_text_input, wl_surface *surface)
{
    auto t = reinterpret_cast<TextInputUnstableV0::Private *>(data);
    Q_ASSERT(t->textinputunstablev0 == wl_text_input);
    t->enteredSurface = Surface::get(surface);
    Q_EMIT t->q->entered();
}

void TextInputUnstableV0::Private::leaveCallback(void *data, wl_text_input *wl_text_input)
{
    auto t = reinterpret_cast<TextInputUnstableV0::Private *>(data);
    Q_ASSERT(t->textinputunstablev0 == wl_text_input);
    t->enteredSurface = nullptr;
    Q_EMIT t->q->left();
}

void TextInputUnstableV0::Private::modifiersMapCallback(void *data, wl_text_input *wl_text_input, wl_array *map)
{
    Q_UNUSED(map)
    auto t = reinterpret_cast<TextInputUnstableV0::Private *>(data);
    Q_ASSERT(t->textinputunstablev0 == wl_text_input);
    // TODO: implement
}

void TextInputUnstableV0::Private::inputPanelStateCallback(void *data, wl_text_input *wl_text_input, uint32_t state)
{
    auto t = reinterpret_cast<TextInputUnstableV0::Private *>(data);
    Q_ASSERT(t->textinputunstablev0 == wl_text_input);
    if (t->inputPanelVisible != state) {
        t->inputPanelVisible = state;
        Q_EMIT t->q->inputPanelStateChanged();
    }
}

void TextInputUnstableV0::Private::preeditStringCallback(void *data, wl_text_input *wl_text_input, uint32_t serial, const char *text, const char *commit)
{
    Q_UNUSED(serial)
    auto t = reinterpret_cast<TextInputUnstableV0::Private *>(data);
    Q_ASSERT(t->textinputunstablev0 == wl_text_input);
    t->pendingPreEdit.commitText = QByteArray(commit);
    t->pendingPreEdit.text = QByteArray(text);
    if (!t->pendingPreEdit.cursorSet) {
        t->pendingPreEdit.cursor = t->pendingPreEdit.text.length();
    }
    t->currentPreEdit = t->pendingPreEdit;
    t->pendingPreEdit = TextInput::Private::PreEdit();
    Q_EMIT t->q->composingTextChanged();
}

void TextInputUnstableV0::Private::preeditStylingCallback(void *data, wl_text_input *wl_text_input, uint32_t index, uint32_t length, uint32_t style)
{
    Q_UNUSED(index)
    Q_UNUSED(length)
    Q_UNUSED(style)
    // TODO: implement
    auto t = reinterpret_cast<TextInputUnstableV0::Private *>(data);
    Q_ASSERT(t->textinputunstablev0 == wl_text_input);
}

void TextInputUnstableV0::Private::preeditCursorCallback(void *data, wl_text_input *wl_text_input, int32_t index)
{
    auto t = reinterpret_cast<TextInputUnstableV0::Private *>(data);
    Q_ASSERT(t->textinputunstablev0 == wl_text_input);
    t->pendingPreEdit.cursor = index;
    t->pendingPreEdit.cursorSet = true;
}

void TextInputUnstableV0::Private::commitStringCallback(void *data, wl_text_input *wl_text_input, uint32_t serial, const char *text)
{
    Q_UNUSED(serial)
    auto t = reinterpret_cast<TextInputUnstableV0::Private *>(data);
    Q_ASSERT(t->textinputunstablev0 == wl_text_input);
    t->pendingCommit.text = QByteArray(text);
    t->currentCommit = t->pendingCommit;
    // TODO: what are the proper values it should be set to?
    t->pendingCommit = TextInput::Private::Commit();
    t->pendingCommit.deleteSurrounding.beforeLength = 0;
    t->pendingCommit.deleteSurrounding.afterLength = 0;
    Q_EMIT t->q->committed();
}

void TextInputUnstableV0::Private::cursorPositionCallback(void *data, wl_text_input *wl_text_input, int32_t index, int32_t anchor)
{
    auto t = reinterpret_cast<TextInputUnstableV0::Private *>(data);
    Q_ASSERT(t->textinputunstablev0 == wl_text_input);
    t->pendingCommit.cursor = index;
    t->pendingCommit.anchor = anchor;
}

void TextInputUnstableV0::Private::deleteSurroundingTextCallback(void *data, wl_text_input *wl_text_input, int32_t index, uint32_t length)
{
    auto t = reinterpret_cast<TextInputUnstableV0::Private *>(data);
    Q_ASSERT(t->textinputunstablev0 == wl_text_input);
    t->pendingCommit.deleteSurrounding.beforeLength = qAbs(index);
    t->pendingCommit.deleteSurrounding.afterLength = length - t->pendingCommit.deleteSurrounding.beforeLength;
}

void TextInputUnstableV0::Private::keysymCallback(void *data,
                                                  wl_text_input *wl_text_input,
                                                  uint32_t serial,
                                                  uint32_t time,
                                                  uint32_t sym,
                                                  uint32_t wlState,
                                                  uint32_t modifiers)
{
    Q_UNUSED(serial)
    // TODO: add support for modifiers
    Q_UNUSED(modifiers)
    auto t = reinterpret_cast<TextInputUnstableV0::Private *>(data);
    Q_ASSERT(t->textinputunstablev0 == wl_text_input);
    TextInput::KeyState state;
    switch (wlState) {
    case WL_KEYBOARD_KEY_STATE_RELEASED:
        state = TextInput::KeyState::Released;
        break;
    case WL_KEYBOARD_KEY_STATE_PRESSED:
        state = TextInput::KeyState::Pressed;
        break;
    default:
        // invalid
        return;
    }
    Q_EMIT t->q->keyEvent(sym, state, Qt::KeyboardModifiers(), time);
}

void TextInputUnstableV0::Private::languageCallback(void *data, wl_text_input *wl_text_input, uint32_t serial, const char *language)
{
    Q_UNUSED(serial)
    auto t = reinterpret_cast<TextInputUnstableV0::Private *>(data);
    Q_ASSERT(t->textinputunstablev0 == wl_text_input);
    if (qstrcmp(t->language, language) != 0) {
        t->language = QByteArray(language);
        Q_EMIT t->q->languageChanged();
    }
}

void TextInputUnstableV0::Private::textDirectionCallback(void *data, wl_text_input *wl_text_input, uint32_t serial, uint32_t wlDirection)
{
    Q_UNUSED(serial)
    auto t = reinterpret_cast<TextInputUnstableV0::Private *>(data);
    Q_ASSERT(t->textinputunstablev0 == wl_text_input);
    Qt::LayoutDirection direction;
    switch (wlDirection) {
    case WL_TEXT_INPUT_TEXT_DIRECTION_LTR:
        direction = Qt::LeftToRight;
        break;
    case WL_TEXT_INPUT_TEXT_DIRECTION_RTL:
        direction = Qt::RightToLeft;
        break;
    case WL_TEXT_INPUT_TEXT_DIRECTION_AUTO:
        direction = Qt::LayoutDirectionAuto;
        break;
    default:
        // invalid
        return;
    }
    if (direction != t->textDirection) {
        t->textDirection = direction;
        Q_EMIT t->q->textDirectionChanged();
    }
}

TextInputUnstableV0::Private::Private(TextInputUnstableV0 *q, Seat *seat)
    : TextInput::Private(seat)
    , q(q)
{
}

void TextInputUnstableV0::Private::setup(wl_text_input *ti)
{
    Q_ASSERT(ti);
    Q_ASSERT(!textinputunstablev0);
    textinputunstablev0.setup(ti);
    wl_text_input_add_listener(ti, &s_listener, this);
}

bool TextInputUnstableV0::Private::isValid() const
{
    return textinputunstablev0.isValid();
}

void TextInputUnstableV0::Private::enable(Surface *surface)
{
    wl_text_input_activate(textinputunstablev0, *seat, *surface);
}

void TextInputUnstableV0::Private::disable(Surface *surface)
{
    Q_UNUSED(surface)
    wl_text_input_deactivate(textinputunstablev0, *seat);
}

void TextInputUnstableV0::Private::showInputPanel()
{
    wl_text_input_show_input_panel(textinputunstablev0);
}

void TextInputUnstableV0::Private::hideInputPanel()
{
    wl_text_input_hide_input_panel(textinputunstablev0);
}

void TextInputUnstableV0::Private::setCursorRectangle(const QRect &rect)
{
    wl_text_input_set_cursor_rectangle(textinputunstablev0, rect.x(), rect.y(), rect.width(), rect.height());
}

void TextInputUnstableV0::Private::setPreferredLanguage(const QString &lang)
{
    wl_text_input_set_preferred_language(textinputunstablev0, lang.toUtf8().constData());
}

void TextInputUnstableV0::Private::setSurroundingText(const QString &text, quint32 cursor, quint32 anchor)
{
    const QStringView strView(text);

    wl_text_input_set_surrounding_text(textinputunstablev0,
                                       text.toUtf8().constData(),
                                       strView.left(cursor).toUtf8().length(),
                                       strView.left(anchor).toUtf8().length());
}

void TextInputUnstableV0::Private::reset()
{
    wl_text_input_reset(textinputunstablev0);
}

void TextInputUnstableV0::Private::setContentType(ContentHints hints, ContentPurpose purpose)
{
    uint32_t wlHints = 0;
    uint32_t wlPurpose = 0;
    if (hints.testFlag(ContentHint::AutoCompletion)) {
        wlHints |= WL_TEXT_INPUT_CONTENT_HINT_AUTO_COMPLETION;
    }
    if (hints.testFlag(ContentHint::AutoCorrection)) {
        wlHints |= WL_TEXT_INPUT_CONTENT_HINT_AUTO_CORRECTION;
    }
    if (hints.testFlag(ContentHint::AutoCapitalization)) {
        wlHints |= WL_TEXT_INPUT_CONTENT_HINT_AUTO_CAPITALIZATION;
    }
    if (hints.testFlag(ContentHint::LowerCase)) {
        wlHints |= WL_TEXT_INPUT_CONTENT_HINT_LOWERCASE;
    }
    if (hints.testFlag(ContentHint::UpperCase)) {
        wlHints |= WL_TEXT_INPUT_CONTENT_HINT_UPPERCASE;
    }
    if (hints.testFlag(ContentHint::TitleCase)) {
        wlHints |= WL_TEXT_INPUT_CONTENT_HINT_TITLECASE;
    }
    if (hints.testFlag(ContentHint::HiddenText)) {
        wlHints |= WL_TEXT_INPUT_CONTENT_HINT_HIDDEN_TEXT;
    }
    if (hints.testFlag(ContentHint::SensitiveData)) {
        wlHints |= WL_TEXT_INPUT_CONTENT_HINT_SENSITIVE_DATA;
    }
    if (hints.testFlag(ContentHint::Latin)) {
        wlHints |= WL_TEXT_INPUT_CONTENT_HINT_LATIN;
    }
    if (hints.testFlag(ContentHint::MultiLine)) {
        wlHints |= WL_TEXT_INPUT_CONTENT_HINT_MULTILINE;
    }
    switch (purpose) {
    case ContentPurpose::Normal:
        wlPurpose = WL_TEXT_INPUT_CONTENT_PURPOSE_NORMAL;
        break;
    case ContentPurpose::Alpha:
        wlPurpose = WL_TEXT_INPUT_CONTENT_PURPOSE_ALPHA;
        break;
    case ContentPurpose::Digits:
        wlPurpose = WL_TEXT_INPUT_CONTENT_PURPOSE_DIGITS;
        break;
    case ContentPurpose::Number:
        wlPurpose = WL_TEXT_INPUT_CONTENT_PURPOSE_NUMBER;
        break;
    case ContentPurpose::Phone:
        wlPurpose = WL_TEXT_INPUT_CONTENT_PURPOSE_PHONE;
        break;
    case ContentPurpose::Url:
        wlPurpose = WL_TEXT_INPUT_CONTENT_PURPOSE_URL;
        break;
    case ContentPurpose::Email:
        wlPurpose = WL_TEXT_INPUT_CONTENT_PURPOSE_EMAIL;
        break;
    case ContentPurpose::Name:
        wlPurpose = WL_TEXT_INPUT_CONTENT_PURPOSE_NAME;
        break;
    case ContentPurpose::Password:
        wlPurpose = WL_TEXT_INPUT_CONTENT_PURPOSE_PASSWORD;
        break;
    case ContentPurpose::Date:
        wlPurpose = WL_TEXT_INPUT_CONTENT_PURPOSE_DATE;
        break;
    case ContentPurpose::Time:
        wlPurpose = WL_TEXT_INPUT_CONTENT_PURPOSE_TIME;
        break;
    case ContentPurpose::DateTime:
        wlPurpose = WL_TEXT_INPUT_CONTENT_PURPOSE_DATETIME;
        break;
    case ContentPurpose::Terminal:
        wlPurpose = WL_TEXT_INPUT_CONTENT_PURPOSE_TERMINAL;
        break;
    }
    wl_text_input_set_content_type(textinputunstablev0, wlHints, wlPurpose);
}

TextInputUnstableV0::TextInputUnstableV0(Seat *seat, QObject *parent)
    : TextInput(new Private(this, seat), parent)
{
}

TextInputUnstableV0::~TextInputUnstableV0()
{
    release();
}

TextInputUnstableV0::Private *TextInputUnstableV0::d_func() const
{
    return reinterpret_cast<Private *>(d.data());
}

void TextInputUnstableV0::setup(wl_text_input *textinputunstablev0)
{
    Q_D();
    d->setup(textinputunstablev0);
}

void TextInputUnstableV0::release()
{
    Q_D();
    d->textinputunstablev0.release();
}

void TextInputUnstableV0::destroy()
{
    Q_D();
    d->textinputunstablev0.destroy();
}

TextInputUnstableV0::operator wl_text_input *()
{
    Q_D();
    return d->textinputunstablev0;
}

TextInputUnstableV0::operator wl_text_input *() const
{
    Q_D();
    return d->textinputunstablev0;
}

class TextInputManagerUnstableV0::Private : public TextInputManager::Private
{
public:
    Private() = default;

    void release() override;
    void destroy() override;
    bool isValid() override;
    void setupV0(wl_text_input_manager *ti) override;
    TextInput *createTextInput(Seat *seat, QObject *parent = nullptr) override;
    using TextInputManager::Private::operator zwp_text_input_manager_v2 *; // overriding only one overload results in a compiler warning. This tells GCC we're
                                                                           // doing it deliberately
    operator wl_text_input_manager *() override
    {
        return textinputmanagerunstablev0;
    }
    operator wl_text_input_manager *() const override
    {
        return textinputmanagerunstablev0;
    }

    WaylandPointer<wl_text_input_manager, wl_text_input_manager_destroy> textinputmanagerunstablev0;
};

void TextInputManagerUnstableV0::Private::release()
{
    textinputmanagerunstablev0.release();
}

void TextInputManagerUnstableV0::Private::destroy()
{
    textinputmanagerunstablev0.destroy();
}

bool TextInputManagerUnstableV0::Private::isValid()
{
    return textinputmanagerunstablev0.isValid();
}

TextInputManagerUnstableV0::TextInputManagerUnstableV0(QObject *parent)
    : TextInputManager(new Private, parent)
{
}

TextInputManagerUnstableV0::Private *TextInputManagerUnstableV0::d_func() const
{
    return reinterpret_cast<Private *>(d.data());
}

TextInputManagerUnstableV0::~TextInputManagerUnstableV0()
{
    release();
}

void TextInputManagerUnstableV0::Private::setupV0(wl_text_input_manager *ti)
{
    Q_ASSERT(ti);
    Q_ASSERT(!textinputmanagerunstablev0);
    textinputmanagerunstablev0.setup(ti);
}

TextInput *TextInputManagerUnstableV0::Private::createTextInput(Seat *seat, QObject *parent)
{
    Q_ASSERT(isValid());
    TextInputUnstableV0 *t = new TextInputUnstableV0(seat, parent);
    auto w = wl_text_input_manager_create_text_input(textinputmanagerunstablev0);
    if (queue) {
        queue->addProxy(w);
    }
    t->setup(w);
    return t;
}

}
}
