// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef INPUTCONTEXT_H
#define INPUTCONTEXT_H

#include "Events.h"
#include "InputState.h"
#include "ObjectId.h"
#include "SurroundingText.h"
#include "dimcore/ContentType.h"

#include <QObject>

#include <variant>

namespace org {
namespace deepin {
namespace dim {

class Dim;

class InputContext : public QObject, public ObjectId<InputContext>
{
    Q_OBJECT
    friend class InputState;

public:
    explicit InputContext(Dim *dim, QObject *parent = nullptr);
    ~InputContext() = default;

public:
    void destroy();
    void focusIn();
    void focusOut();
    bool keyEvent(InputContextKeyEvent &event);

    InputState &inputState();

    void updatePreedit(const QString &text, int32_t cursorBegin, int32_t cursorEnd);
    void commitString(const QString &text);
    void commit();
    void forwardKey(uint32_t keycode, bool pressed);
    ContentType &contentType();
    void updateContentType();
    SurroundingText &surroundingText();
    void updateSurroundingText();

Q_SIGNALS:
    void imSwitch(const std::pair<std::string, std::string> &imEntry);

protected:
    Dim *dim() { return dim_; }

    bool hasFocus() const;

    virtual void updatePreeditImpl(const QString &text, int32_t cursorBegin, int32_t cursorEnd) = 0;
    virtual void commitStringImpl(const QString &text) = 0;
    virtual void commitImpl() = 0;
    virtual void forwardKeyImpl(uint32_t keycode, bool pressed) = 0;

private:
    Dim *dim_;
    bool hasFocus_ = false;
    InputState inputState_;
    ContentType contentType_;
    SurroundingText surroundingText_;
};

} // namespace dim
} // namespace deepin
} // namespace org

#endif // !INPUTCONTEXT_H
