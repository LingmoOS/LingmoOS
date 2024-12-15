// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "variantValue.h"
#include "constant.h"
#include <sstream>

std::unique_ptr<VariantValue> creatValueHandler(msg_ptr &msg, DBusValueType type)
{
    switch (type) {
    case DBusValueType::String:
        return std::make_unique<StringValue>(msg);
    case DBusValueType::ArrayOfString:
        return std::make_unique<ASValue>(msg);
    default:
        return nullptr;
    }
}

int StringValue::openVariant() noexcept
{
    return sd_bus_message_open_container(msgRef(), SD_BUS_TYPE_VARIANT, "s");
}

int StringValue::closeVariant() noexcept
{
    return sd_bus_message_close_container(msgRef());
}

int StringValue::appendValue(std::string &&value) noexcept
{
    return sd_bus_message_append(msgRef(), "s", value.data());
}

int ASValue::openVariant() noexcept
{
    if (int ret = sd_bus_message_open_container(msgRef(), SD_BUS_TYPE_VARIANT, "as"); ret < 0)
        return ret;

    return sd_bus_message_open_container(msgRef(), SD_BUS_TYPE_ARRAY, "s");
}

int ASValue::closeVariant() noexcept
{
    if (int ret = sd_bus_message_close_container(msgRef()); ret < 0)
        return ret;

    return sd_bus_message_close_container(msgRef());
}

int ASValue::appendValue(std::string &&value) noexcept
{
    return sd_bus_message_append(msgRef(), "s", value.data());
}
