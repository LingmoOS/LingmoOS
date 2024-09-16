// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "types.h"
#include <memory>

class VariantValue
{
public:
    explicit VariantValue(msg_ptr &msg)
        : m_msg(msg)
    {
    }
    virtual ~VariantValue() = default;
    VariantValue(const VariantValue &) = delete;
    VariantValue(VariantValue &&) = delete;
    VariantValue &operator=(const VariantValue &) = delete;
    VariantValue &operator=(VariantValue &&) = delete;

    virtual int openVariant() noexcept = 0;
    virtual int closeVariant() noexcept = 0;
    virtual int appendValue(std::string &&value) noexcept = 0;

    msg_ptr &msgRef() noexcept { return m_msg; }

private:
    msg_ptr &m_msg;
};

class StringValue : public VariantValue
{
    using VariantValue::VariantValue;

public:
    int openVariant() noexcept override;
    int closeVariant() noexcept override;
    int appendValue(std::string &&value) noexcept override;
};

class ASValue : public VariantValue
{
    using VariantValue::VariantValue;

public:
    int openVariant() noexcept override;
    int closeVariant() noexcept override;
    int appendValue(std::string &&value) noexcept override;
};

std::unique_ptr<VariantValue> creatValueHandler(msg_ptr &msg, DBusValueType type);
