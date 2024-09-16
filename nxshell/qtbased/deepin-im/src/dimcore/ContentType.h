// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CONTENTTYPE_H
#define CONTENTTYPE_H

#include <QString>

namespace org::deepin::dim {

class ContentType
{
public:
    ContentType() = default;
    ~ContentType() = default;

    uint32_t hint() { return hint_; }

    void setHint(uint32_t hint) { hint_ = hint; }

    uint32_t purpose() { return purpose_; }

    void setPurpose(uint32_t purpose) { purpose_ = purpose; }

private:
    uint32_t hint_ = 0;
    uint32_t purpose_ = 0;
};

} // namespace org::deepin::dim

#endif // !CONTENTTYPE_H
