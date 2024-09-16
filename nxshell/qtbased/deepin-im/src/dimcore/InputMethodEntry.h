// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef INPUTMETHOD_H
#define INPUTMETHOD_H

#include <QString>

#include <memory>

class InputMethodEntryPrivate;

namespace org {
namespace deepin {
namespace dim {

class InputMethodEntry
{
public:
    InputMethodEntry(const std::string &addonKey,
                     const std::string &uniqueName,
                     const std::string &name,
                     const std::string &description,
                     const std::string &label,
                     const std::string &iconName);
    ~InputMethodEntry();

    const std::string &addonKey() const;
    const std::string &uniqueName() const;
    const std::string &name() const;
    const std::string &description() const;
    const std::string &label() const;
    const std::string &iconName() const;

private:
    std::shared_ptr<InputMethodEntryPrivate> d;
};

} // namespace dim
} // namespace deepin
} // namespace org

#endif // !INPUTMETHOD_H
