/*
    SPDX-FileCopyrightText: 2016 Martin Gräßlin <mgraesslin@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#include "textinput_p.h"

namespace KWayland
{
namespace Client
{
TextInput::Private::Private(Seat *seat)
    : seat(seat)
{
    currentCommit.deleteSurrounding.afterLength = 0;
    currentCommit.deleteSurrounding.beforeLength = 0;
    pendingCommit.deleteSurrounding.afterLength = 0;
    pendingCommit.deleteSurrounding.beforeLength = 0;
}

TextInput::TextInput(Private *p, QObject *parent)
    : QObject(parent)
    , d(p)
{
}

TextInput::~TextInput() = default;

void TextInput::setEventQueue(EventQueue *queue)
{
    d->queue = queue;
}

EventQueue *TextInput::eventQueue() const
{
    return d->queue;
}

bool TextInput::isValid() const
{
    return d->isValid();
}

Surface *TextInput::enteredSurface() const
{
    return d->enteredSurface;
}

bool TextInput::isInputPanelVisible() const
{
    return d->inputPanelVisible;
}

void TextInput::enable(Surface *surface)
{
    d->enable(surface);
}

void TextInput::disable(Surface *surface)
{
    d->disable(surface);
}

void TextInput::showInputPanel()
{
    d->showInputPanel();
}

void TextInput::hideInputPanel()
{
    d->hideInputPanel();
}

void TextInput::reset()
{
    d->reset();
}

void TextInput::setSurroundingText(const QString &text, quint32 cursor, quint32 anchor)
{
    d->setSurroundingText(text, cursor, anchor);
}

void TextInput::setContentType(ContentHints hint, ContentPurpose purpose)
{
    d->setContentType(hint, purpose);
}

void TextInput::setCursorRectangle(const QRect &rect)
{
    d->setCursorRectangle(rect);
}

void TextInput::setPreferredLanguage(const QString &language)
{
    d->setPreferredLanguage(language);
}

Qt::LayoutDirection TextInput::textDirection() const
{
    return d->textDirection;
}

QByteArray TextInput::language() const
{
    return d->language;
}

qint32 TextInput::composingTextCursorPosition() const
{
    return d->currentPreEdit.cursor;
}

QByteArray TextInput::composingText() const
{
    return d->currentPreEdit.text;
}

QByteArray TextInput::composingFallbackText() const
{
    return d->currentPreEdit.commitText;
}

qint32 TextInput::anchorPosition() const
{
    return d->currentCommit.anchor;
}

qint32 TextInput::cursorPosition() const
{
    return d->currentCommit.cursor;
}

TextInput::DeleteSurroundingText TextInput::deleteSurroundingText() const
{
    return d->currentCommit.deleteSurrounding;
}

QByteArray TextInput::commitText() const
{
    return d->currentCommit.text;
}

TextInputManager::TextInputManager(Private *p, QObject *parent)
    : QObject(parent)
    , d(p)
{
}

TextInputManager::~TextInputManager() = default;

void TextInputManager::setup(wl_text_input_manager *textinputmanagerunstablev0)
{
    d->setupV0(textinputmanagerunstablev0);
}

void TextInputManager::setup(zwp_text_input_manager_v2 *textinputmanagerunstablev2)
{
    d->setupV2(textinputmanagerunstablev2);
}

void TextInputManager::release()
{
    d->release();
}

void TextInputManager::destroy()
{
    d->destroy();
}

void TextInputManager::setEventQueue(EventQueue *queue)
{
    d->queue = queue;
}

EventQueue *TextInputManager::eventQueue()
{
    return d->queue;
}

TextInputManager::operator wl_text_input_manager *()
{
    return *(d.data());
}

TextInputManager::operator wl_text_input_manager *() const
{
    return *(d.data());
}

TextInputManager::operator zwp_text_input_manager_v2 *()
{
    return *(d.data());
}

TextInputManager::operator zwp_text_input_manager_v2 *() const
{
    return *(d.data());
}

bool TextInputManager::isValid() const
{
    return d->isValid();
}

TextInput *TextInputManager::createTextInput(Seat *seat, QObject *parent)
{
    return d->createTextInput(seat, parent);
}

}
}

#include "moc_textinput.cpp"
#include "moc_textinput_p.cpp"
