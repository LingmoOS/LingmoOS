/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2001 Carsten Pfeiffer <pfeiffer@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KSORTABLELIST_H
#define KSORTABLELIST_H

#include <kcompletion_export.h>

#include <QList>
#include <QPair>
#include <algorithm>

/**
 * \class KSortableItem ksortablelist.h <KSortableItem>
 *
 * KSortableItem is a QPair that provides several operators
 * for sorting.
 * @see KSortableList
 */
template<typename T, typename Key = int>
class KSortableItem : public QPair<Key, T>
{
public:
    /**
     * Creates a new KSortableItem with the given values.
     * @param i the first value (the key)
     * @param t the second value (the item)
     */
    KSortableItem(Key i, const T &t)
        : QPair<Key, T>(i, t)
    {
    }
    /**
     * Creates a new KSortableItem that copies another one.
     * @param rhs the other item to copy
     */
    KSortableItem(const KSortableItem<T, Key> &rhs)
        : QPair<Key, T>(rhs.first, rhs.second)
    {
    }

    /**
     * Creates a new KSortableItem with uninitialized values.
     */
    KSortableItem()
    {
    }

    /**
     * Assignment operator, just copies the item.
     */
    KSortableItem<T, Key> &operator=(const KSortableItem<T, Key> &i)
    {
        this->first = i.first;
        this->second = i.second;
        return *this;
    }

    // operators for sorting
    /**
     * Compares the two items. This implementation only compares
     * the first value.
     */
    bool operator>(const KSortableItem<T, Key> &i2) const
    {
        return (i2.first < this->first);
    }
    /**
     * Compares the two items. This implementation only compares
     * the first value.
     */
    bool operator<(const KSortableItem<T, Key> &i2) const
    {
        return (this->first < i2.first);
    }
    /**
     * Compares the two items. This implementation only compares
     * the first value.
     */
    bool operator>=(const KSortableItem<T, Key> &i2) const
    {
        return (this->first >= i2.first);
    }
    /**
     * Compares the two items. This implementation only compares
     * the first value.
     */
    bool operator<=(const KSortableItem<T, Key> &i2) const
    {
        return !(i2.first < this->first);
    }
    /**
     * Compares the two items. This implementation only compares
     * the first value.
     */
    bool operator==(const KSortableItem<T, Key> &i2) const
    {
        return (this->first == i2.first);
    }
    /**
     * Compares the two items. This implementation only compares
     * the first value.
     */
    bool operator!=(const KSortableItem<T, Key> &i2) const
    {
        return (this->first != i2.first);
    }

    /**
     * @return the second value (the item)
     */
    T &value()
    {
        return this->second;
    }

    /**
     * @return the second value (the item)
     */
    const T &value() const
    {
        return this->second;
    }

    /**
     * @return the first value.
     */
    Key key() const
    {
        return this->first;
    }
};

/**
 * \class KSortableList ksortablelist.h <KSortableList>
 *
 * KSortableList is a QList which associates a key with each item in the list.
 * This key is used for sorting when calling sort().
 *
 * This allows to temporarily calculate a key and use it for sorting, without having
 * to store that key in the items, or calculate that key many times for the same item
 * during sorting if that calculation is expensive.
 */
template<typename T, typename Key = int>
class KSortableList : public QList<KSortableItem<T, Key>>
{
public:
    /**
     * Insert a KSortableItem with the given values.
     * @param i the first value
     * @param t the second value
     */
    void insert(Key i, const T &t)
    {
        QList<KSortableItem<T, Key>>::append(KSortableItem<T, Key>(i, t));
    }
    // add more as you please...

    /**
     * Returns the first value of the KSortableItem at the given position.
     * @return the first value of the KSortableItem
     */
    T &operator[](Key i)
    {
        return QList<KSortableItem<T, Key>>::operator[](i).value();
    }

    /**
     * Returns the first value of the KSortableItem at the given position.
     * @return the first value of the KSortableItem
     */
    const T &operator[](Key i) const
    {
        return QList<KSortableItem<T, Key>>::operator[](i).value();
    }

    /**
     * Sorts the KSortableItems.
     */
    void sort()
    {
        std::sort(this->begin(), this->end());
    }
};

#endif // KSORTABLELIST_H
