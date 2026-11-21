// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mpvnode.h"

#include <cstring>

static char* copyString(const std::string& text)
{
    char* copy = new char[text.size() + 1];
    std::memcpy(copy, text.c_str(), text.size() + 1);
    return copy;
}

static mpv_node_list* createList(mpv_node* node, bool map, size_t count)
{
    node->format = map ? MPV_FORMAT_NODE_MAP : MPV_FORMAT_NODE_ARRAY;
    auto* list = new mpv_node_list;
    list->num = int(count);
    list->values = new mpv_node[count]();
    list->keys = map ? new char*[count]() : nullptr;
    node->u.list = list;
    return list;
}

bool MpvNode::toBool(bool fallback) const
{
    if (const bool* flag = std::get_if<bool>(&m_value))
        return *flag;
    if (const int64_t* number = std::get_if<int64_t>(&m_value))
        return *number != 0;
    return fallback;
}

int64_t MpvNode::toInt(int64_t fallback) const
{
    if (const int64_t* number = std::get_if<int64_t>(&m_value))
        return *number;
    if (const double* number = std::get_if<double>(&m_value))
        return int64_t(*number);
    if (const bool* flag = std::get_if<bool>(&m_value))
        return *flag ? 1 : 0;
    return fallback;
}

double MpvNode::toDouble(double fallback) const
{
    if (const double* number = std::get_if<double>(&m_value))
        return *number;
    if (const int64_t* number = std::get_if<int64_t>(&m_value))
        return double(*number);
    return fallback;
}

std::string MpvNode::toString() const
{
    if (const std::string* text = std::get_if<std::string>(&m_value))
        return *text;
    return {};
}

const MpvNode::List& MpvNode::list() const
{
    static const List empty;
    const List* list = std::get_if<List>(&m_value);
    return list ? *list : empty;
}

const MpvNode::Map& MpvNode::map() const
{
    static const Map empty;
    const Map* map = std::get_if<Map>(&m_value);
    return map ? *map : empty;
}

const MpvNode& MpvNode::operator[](std::string_view key) const
{
    static const MpvNode none;
    for (const auto& [name, value] : map()) {
        if (name == key)
            return value;
    }
    return none;
}

void MpvNode::fill(mpv_node* node) const
{
    if (const bool* flag = std::get_if<bool>(&m_value)) {
        node->format = MPV_FORMAT_FLAG;
        node->u.flag = *flag ? 1 : 0;
    } else if (const int64_t* number = std::get_if<int64_t>(&m_value)) {
        node->format = MPV_FORMAT_INT64;
        node->u.int64 = *number;
    } else if (const double* number = std::get_if<double>(&m_value)) {
        node->format = MPV_FORMAT_DOUBLE;
        node->u.double_ = *number;
    } else if (const std::string* text = std::get_if<std::string>(&m_value)) {
        node->format = MPV_FORMAT_STRING;
        node->u.string = copyString(*text);
    } else if (const List* items = std::get_if<List>(&m_value)) {
        mpv_node_list* list = createList(node, false, items->size());
        for (size_t i = 0; i < items->size(); ++i)
            (*items)[i].fill(&list->values[i]);
    } else if (const Map* items = std::get_if<Map>(&m_value)) {
        mpv_node_list* list = createList(node, true, items->size());
        for (size_t i = 0; i < items->size(); ++i) {
            list->keys[i] = copyString((*items)[i].first);
            (*items)[i].second.fill(&list->values[i]);
        }
    } else {
        node->format = MPV_FORMAT_NONE;
    }
}

void MpvNode::release(mpv_node* node)
{
    switch (node->format) {
    case MPV_FORMAT_STRING:
        delete[] node->u.string;
        break;
    case MPV_FORMAT_NODE_ARRAY:
    case MPV_FORMAT_NODE_MAP: {
        mpv_node_list* list = node->u.list;
        for (int i = 0; i < list->num; ++i) {
            if (list->keys)
                delete[] list->keys[i];
            release(&list->values[i]);
        }
        delete[] list->keys;
        delete[] list->values;
        delete list;
        break;
    }
    default:
        break;
    }
    node->format = MPV_FORMAT_NONE;
}

MpvNode MpvNode::from(const mpv_node* node)
{
    switch (node->format) {
    case MPV_FORMAT_STRING:
    case MPV_FORMAT_OSD_STRING:
        return std::string(node->u.string);
    case MPV_FORMAT_FLAG:
        return bool(node->u.flag);
    case MPV_FORMAT_INT64:
        return node->u.int64;
    case MPV_FORMAT_DOUBLE:
        return node->u.double_;
    case MPV_FORMAT_NODE_ARRAY: {
        List items;
        items.reserve(node->u.list->num);
        for (int i = 0; i < node->u.list->num; ++i)
            items.push_back(from(&node->u.list->values[i]));
        return items;
    }
    case MPV_FORMAT_NODE_MAP: {
        Map items;
        items.reserve(node->u.list->num);
        for (int i = 0; i < node->u.list->num; ++i)
            items.emplace_back(node->u.list->keys[i], from(&node->u.list->values[i]));
        return items;
    }
    default:
        return {};
    }
}
