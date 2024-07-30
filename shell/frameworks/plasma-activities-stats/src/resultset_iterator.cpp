/*
    SPDX-FileCopyrightText: 2015, 2016 Ivan Cukic <ivan.cukic(at)kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include <optional>

namespace KActivities
{
namespace Stats
{
using namespace Terms;

typedef ResultSet::const_iterator iterator;

// Iterator

class ResultSet_IteratorPrivate
{
public:
    ResultSet_IteratorPrivate(const ResultSet *resultSet, int currentRow = -1)
        : resultSet(resultSet)
        , currentRow(currentRow)
    {
        updateValue();
    }

    const ResultSet *resultSet;
    int currentRow;
    std::optional<ResultSet::Result> currentValue;

    inline void moveTo(int row)
    {
        if (row == currentRow)
            return;
        currentRow = row;
        updateValue();
    }

    inline void moveBy(int row)
    {
        moveTo(currentRow + row);
    }

    void updateValue()
    {
        if (!resultSet || !resultSet->d->query.seek(currentRow)) {
            currentValue.reset();

        } else {
            auto value = resultSet->d->currentResult();
            currentValue = std::move(value);
        }
    }

    friend void swap(ResultSet_IteratorPrivate &left, ResultSet_IteratorPrivate &right)
    {
        std::swap(left.resultSet, right.resultSet);
        std::swap(left.currentRow, right.currentRow);
        std::swap(left.currentValue, right.currentValue);
    }

    bool operator==(const ResultSet_IteratorPrivate &other) const
    {
        bool thisValid = currentValue.has_value();
        bool otherValid = other.currentValue.has_value();

        return
            // If one is valid, and the other is not,
            // they are not equal
            thisValid != otherValid ? false :

                                    // If both are invalid, they are equal
            !thisValid ? true
                       :

                       // Otherwise, really compare
            resultSet == other.resultSet && currentRow == other.currentRow;
    }

    bool isValid() const
    {
        return currentValue.has_value();
    }

    static bool sameSource(const ResultSet_IteratorPrivate &left, const ResultSet_IteratorPrivate &right)
    {
        return left.resultSet == right.resultSet && left.resultSet != nullptr;
    }
};

iterator::const_iterator(const ResultSet *resultSet, int currentRow)
    : d(new ResultSet_IteratorPrivate(resultSet, currentRow))
{
}

iterator::const_iterator()
    : d(new ResultSet_IteratorPrivate(nullptr, -1))
{
}

iterator::const_iterator(const const_iterator &source)
    : d(new ResultSet_IteratorPrivate(source.d->resultSet, source.d->currentRow))
{
}

bool iterator::isSourceValid() const
{
    return d->resultSet != nullptr;
}

iterator &iterator::operator=(const const_iterator &source)
{
    const_iterator temp(source);
    swap(*d, *temp.d);
    return *this;
}

iterator::~const_iterator()
{
    delete d;
}

iterator::reference iterator::operator*() const
{
    return d->currentValue.value();
}

iterator::pointer iterator::operator->() const
{
    return &d->currentValue.value();
}

// prefix
iterator &iterator::operator++()
{
    d->currentRow++;
    d->updateValue();

    return *this;
}

// postfix
iterator iterator::operator++(int)
{
    return const_iterator(d->resultSet, d->currentRow + 1);
}

// prefix
iterator &iterator::operator--()
{
    d->currentRow--;
    d->updateValue();

    return *this;
}

// postfix
iterator iterator::operator--(int)
{
    return const_iterator(d->resultSet, d->currentRow - 1);
}

iterator ResultSet::begin() const
{
    return const_iterator(this, d->database ? 0 : -1);
}

iterator ResultSet::end() const
{
    return const_iterator(this, -1);
}

iterator iterator::operator+(iterator::difference_type n) const
{
    return const_iterator(d->resultSet, d->currentRow + n);
}

iterator &iterator::operator+=(iterator::difference_type n)
{
    d->moveBy(n);
    return *this;
}

iterator iterator::operator-(iterator::difference_type n) const
{
    return const_iterator(d->resultSet, d->currentRow - n);
}

iterator &iterator::operator-=(iterator::difference_type n)
{
    d->moveBy(-n);
    return *this;
}

iterator::reference iterator::operator[](iterator::difference_type n) const
{
    return *(*this + n);
}

// bool iterator::operator==(const const_iterator &right) const
// {
//     return *d == *right.d;
// }
//
// bool iterator::operator!=(const const_iterator &right) const
// {
//     return !(*d == *right.d);
// }

bool operator==(const iterator &left, const iterator &right)
{
    return *left.d == *right.d;
}

bool operator!=(const iterator &left, const iterator &right)
{
    return !(*left.d == *right.d);
}

#define COMPARATOR_IMPL(OP)                                                                                                                                    \
    bool operator OP(const iterator &left, const iterator &right)                                                                                              \
    {                                                                                                                                                          \
        return ResultSet_IteratorPrivate::sameSource(*left.d, *right.d) ? left.d->currentRow OP right.d->currentRow : false;                                   \
    }

COMPARATOR_IMPL(<)
COMPARATOR_IMPL(>)
COMPARATOR_IMPL(<=)
COMPARATOR_IMPL(>=)

#undef COMPARATOR_IMPL

iterator::difference_type operator-(const iterator &left, const iterator &right)
{
    return ResultSet_IteratorPrivate::sameSource(*left.d, *right.d) ? left.d->currentRow - right.d->currentRow : 0;
}

} // namespace Stats
} // namespace KActivities
