/*
 * This file is part of the KDE wacomtablet project. For copyright
 * information and license terms see the AUTHORS and COPYING files
 * in the top-level directory of this distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef ENUM_H
#define ENUM_H

#include <QList>

namespace Wacom
{

/**
 * A typesafe enumeration template class. It can be used as a comfortable replacement
 * for C++ enums and supports the following features:
 *
 *   - enumerators can be listed and iterated over
 *   - enumerator lists are sorted by a given comparator
 *   - enumerators can have a key assigned
 *   - enumerator keys can be listed and searched for
 *
 * NOTICE
 * This class uses template specialization to store a static set of all instances.
 * Thereofore a specialization of the private instances member  'instances' has
 * to be declared and instantiated before this template can be used.
 *
 * \tparam D The derived sub class type. This is the class which contains the enum instances.
 * \tparam K The key type used for these instances.
 * \tparam L A "less than" functor which can compare objects of type D*.
 * \tparam E A "equal to" functor which can compare objects of type K.
 */
template<class D, class K, class L, class E>
class Enum
{
private:
    typedef QList<const D *> Container;
    typedef typename Container::const_iterator ContainerConstIterator;

public:
    typedef typename Container::size_type size_type;

    /**
     * A constant iterator which can be used to iterator over all known Enum instances.
     */
    class const_iterator
    {
    public:
        const_iterator()
        {
        }
        const_iterator(const const_iterator &iter)
        {
            operator=(iter);
        }
        const_iterator(const ContainerConstIterator &iter)
        {
            m_iter = iter;
        }

        const D &operator*() const
        {
            return **m_iter;
        }
        const D *operator->() const
        {
            return *m_iter;
        }
        const_iterator &operator=(const const_iterator &iter)
        {
            m_iter = iter.m_iter;
            return *this;
        }
        bool operator==(const const_iterator &iter) const
        {
            return (m_iter == iter.m_iter);
        }
        bool operator!=(const const_iterator &iter) const
        {
            return (m_iter != iter.m_iter);
        }
        const_iterator &operator++()
        {
            ++m_iter;
            return *this;
        }
        const_iterator operator++(int)
        {
            const_iterator copy(*this);
            ++(*this);
            return copy;
        }
        const_iterator &operator--()
        {
            --m_iter;
            return *this;
        }
        const_iterator operator--(int)
        {
            const_iterator copy(*this);
            --(*this);
            return copy;
        }

    private:
        ContainerConstIterator m_iter;
    };

    /**
     * Equal-Compare Operator.
     */
    bool operator==(const Enum<D, K, L, E> &that) const
    {
        return (m_derived == that.m_derived);
    }

    /**
     * Not-Equal-Compare Operator.
     */
    bool operator!=(const Enum<D, K, L, E> &that) const
    {
        return (m_derived != that.m_derived);
    }

    /**
     * @return A constant iterator to the first element.
     */
    static const_iterator begin()
    {
        // force the use of const_iterator - fixes compile problem with some containers
        const Enum<D, K, L, E>::Container *container = &Enum<D, K, L, E>::instances;
        return const_iterator(container->begin());
    }

    /**
     * @return A constant iterator to the last element.
     */
    static const_iterator end()
    {
        // force the use of const_iterator - fixes compile problem with some containers
        const Enum<D, K, L, E>::Container *container = &Enum<D, K, L, E>::instances;
        return const_iterator(container->end());
    }

    /**
     * Searches for the enumeration instance by key.
     *
     * @param key The key of the instance to search.
     *
     * @return A constant pointer to the instance or nullptr if not found.
     */
    static const D *find(const K &key)
    {
        E comp;
        for (const_iterator i = begin(); i != end(); ++i) {
            if (comp(i->key(), key)) {
                return &(*i);
            }
        }
        return nullptr;
    }

    /**
     * @return A list of all valid enum instances of this specialization.
     */
    static QList<D> list()
    {
        QList<D> enums;
        for (const_iterator i = begin(); i != end(); ++i) {
            enums.push_back(*i);
        }
        return enums;
    }

    /**
     * @return The key of this enum instance.
     */
    const K &key() const
    {
        return m_key;
    }

    /**
     * Returns a list of all instances' keys of this specialization.
     *
     * @return A list of keys sorted by the less-than-functor.
     */
    static QList<K> keys()
    {
        QList<K> keys;
        for (const_iterator i = begin(); i != end(); ++i) {
            keys.push_back(i->key());
        }
        return keys;
    }

    /**
     * @return The number of enum instances of this specialization.
     */
    static size_type size()
    {
        return Enum<D, K, L, E>::instances.size();
    }

protected:
    /**
     * Initialization constructor.
     * Used to initialize class-static members. Never make this constructor
     * available to the public! Never use it for normal instances! It may
     * only be used to instantiate class-static Enum instances.
     *
     * @param derived A const pointer to the derived class of this instance.
     * @param key The key of this class-static Enum instance.
     */
    explicit Enum(const D *derived, const K &key)
        : m_key(key)
    {
        m_derived = derived;
        insert(derived);
    }

    /**
     * \return A const pointer to the derived class of this instance.
     */
    const D *derived() const
    {
        return m_derived;
    }

private:
    /**
     * Inserts an element into to the static instances container.
     * This method should only be called by the constructor!
     *
     * \param derived An instance of the derived class, never NULL.
     */
    void insert(const D *derived)
    {
        L comp;
        typename Enum<D, K, L, E>::Container::iterator i = this->instances.begin();

        for (; i != this->instances.end(); ++i) {
            if (comp(derived, *i)) {
                this->instances.insert(i, derived);
                return;
            }
        }

        this->instances.push_back(derived);
    }

    K m_key; /**< The key of this instance */
    const D *m_derived; /**< Pointer to derived class for fast comparison */

    /**
     * A static container with all the class-static Enum instances.
     * Every specialization of this template has to declare and instantiate
     * the specialization of this member variable!
     */
    static Container instances;

}; // CLASS
} // NAMESPACE
#endif // HEADER PROTECTION
