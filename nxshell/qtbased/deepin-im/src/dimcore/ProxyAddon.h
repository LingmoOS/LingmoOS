// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PROXYADDON_H
#define PROXYADDON_H

#include "InputMethodAddon.h"

namespace org {
namespace deepin {
namespace dim {

class InputContext;
class InputContextCursorRectChangeEvent;

class ProxyAddon : public InputMethodAddon
{
    Q_OBJECT

public:
    explicit ProxyAddon(Dim *dim, const std::string &key, const QString &iconName);
    virtual ~ProxyAddon();

    const std::vector<std::string> &activeInputMethods() const { return activeInputMethods_; }

    virtual void focusIn(uint32_t id) = 0;
    virtual void focusOut(uint32_t id) = 0;
    virtual void destroyed(uint32_t id) = 0;
    virtual void done() = 0;
    virtual void contentType(uint32_t hint, uint32_t purpose) = 0;
    virtual void cursorRectangleChangeEvent(InputContextCursorRectChangeEvent &event) = 0;
    virtual void setCurrentIM(const std::string &im) = 0;

    static bool isExecutableExisted(const QString &name);

protected:
    void updateActiveInputMethods(const std::vector<std::string> &value);

private:
    std::vector<std::string> activeInputMethods_;
};

} // namespace dim
} // namespace deepin
} // namespace org

#endif // PROXYADDON_H
