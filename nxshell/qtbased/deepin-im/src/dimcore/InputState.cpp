// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "InputState.h"

#include "Dim.h"
#include "InputContext.h"

#include <QDebug>

using namespace org::deepin::dim;

InputState::InputState(InputContext *ic)
    : ic_(ic)
    , currentIMKey_(ic->dim_->getCurrentActiveInputMethod())
{
    connect(ic_->dim_, &Dim::inputMethodEntryChanged, this, [this]() {
        auto iter = findIMEntry();
        currentIMKey_ = *iter;
    });
}

void InputState::switchIM()
{
    auto iter = findIMEntry();
    iter++;

    const auto &imList = ic_->dim_->activeInputMethodEntries();
    if (iter == imList.cend()) {
        iter = imList.cbegin();
    }

    currentIMKey_ = *iter;
    emit ic_->imSwitch(currentIMKey_);
}

void InputState::requestSwitchIM(const std::string &framework, const std::string &name)
{
    const auto &activeInputMethodEntries = ic_->dim_->activeInputMethodEntries();

    auto iter = std::find_if(activeInputMethodEntries.cbegin(),
                             activeInputMethodEntries.cend(),
                             [this, &framework, &name](const auto &pair) {
                                 return pair.first == framework && pair.second == name;
                             });
    if (iter == activeInputMethodEntries.cend()) {
        return;
    }

    currentIMKey_ = *iter;
    emit ic_->imSwitch(currentIMKey_);
}

std::set<std::pair<std::string, std::string>>::const_iterator InputState::findIMEntry() const
{
    const auto &activeInputMethodEntries = ic_->dim_->activeInputMethodEntries();

    auto iter = std::find_if(activeInputMethodEntries.cbegin(),
                             activeInputMethodEntries.cend(),
                             [this](const auto &pair) {
                                 return pair.first == currentIMKey_.first
                                     && pair.second == currentIMKey_.second;
                             });
    if (iter == activeInputMethodEntries.cend()) {
        iter = activeInputMethodEntries.cbegin();
    }

    return iter;
}
