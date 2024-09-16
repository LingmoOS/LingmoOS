// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef INPUTSTATE_H
#define INPUTSTATE_H

#include "InputMethodEntry.h"

#include <QObject>

#include <set>

namespace org {
namespace deepin {
namespace dim {

class Dim;
class InputContext;
class InputMethodEntry;

class InputState : public QObject
{
    Q_OBJECT

public:
    explicit InputState(InputContext *ic);

    const auto &currentIMEntry() const { return currentIMKey_; }

    void switchIM();

public Q_SLOTS:
    void requestSwitchIM(const std::string &framework, const std::string &name);

private:
    std::set<std::pair<std::string, std::string>>::const_iterator findIMEntry() const;

private:
    std::pair<std::string, std::string> currentIMKey_;
    InputContext *ic_;
};

} // namespace dim
} // namespace deepin
} // namespace org

#endif // !INPUTSTATE_H
