// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "InputMethodEntry.h"

class InputMethodEntryPrivate
{
public:
    std::string addonKey;
    std::string uniqueName;
    std::string name;
    std::string description;
    std::string label;
    std::string iconName;
};

using namespace org::deepin::dim;

InputMethodEntry::InputMethodEntry(const std::string &addonKey,
                                   const std::string &uniqueName,
                                   const std::string &name,
                                   const std::string &description,
                                   const std::string &label,
                                   const std::string &iconName)
    : d(new InputMethodEntryPrivate{ addonKey, uniqueName, name, description, label, iconName })
{
}

InputMethodEntry::~InputMethodEntry() { }

const std::string &InputMethodEntry::addonKey() const
{
    return d->addonKey;
}

const std::string &InputMethodEntry::uniqueName() const
{
    return d->uniqueName;
}

const std::string &InputMethodEntry::name() const
{
    return d->name;
}

const std::string &InputMethodEntry::description() const
{
    return d->description;
}

const std::string &InputMethodEntry::label() const
{
    return d->label;
}

const std::string &InputMethodEntry::iconName() const
{
    return d->iconName;
}
