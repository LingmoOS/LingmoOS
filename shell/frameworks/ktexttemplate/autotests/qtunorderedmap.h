/*
  This file is part of the KTextTemplate library

  SPDX-FileCopyrightText: 2010 Stephen Kelly <steveire@gmail.com>

  SPDX-License-Identifier: LGPL-2.1-or-later

*/

#ifndef QTUNORDEREDMAP_H
#define QTUNORDEREDMAP_H

#include <QHash>

#include <unordered_map>

template<typename Key>
struct QtHasher {
    size_t operator()(Key k) const
    {
        return qHash(k);
    }
};

template<typename Key, typename Value>
struct QtUnorderedMap : public std::unordered_map<Key, Value, QtHasher<Key>> {
};

#endif
