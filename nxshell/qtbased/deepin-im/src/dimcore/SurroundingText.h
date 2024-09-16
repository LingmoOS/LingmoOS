// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SURROUDINGTEXT_H
#define SURROUDINGTEXT_H

#include <QString>

namespace org::deepin::dim {

class SurroundingText
{
public:
    SurroundingText() = default;
    ~SurroundingText() = default;

    const QString &text() { return text_; }

    void setText(const QString &text) { text_ = text; }

    uint32_t cursor() { return cursor_; }

    void setCursor(uint32_t cursor) { cursor_ = cursor; }

    uint32_t anchor() { return anchor_; }

    void setAnchor(uint32_t anchor) { anchor_ = anchor; }

private:
    QString text_;
    uint32_t cursor_ = 0;
    uint32_t anchor_ = 0;
};

} // namespace org::deepin::dim

#endif // !SURROUDINGTEXT_H
