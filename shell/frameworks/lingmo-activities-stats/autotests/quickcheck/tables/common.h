/*
    SPDX-FileCopyrightText: 2015 Ivan Cukic <ivan.cukic(at)kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef QUICKCHECK_DATABASE_COMMON_H
#define QUICKCHECK_DATABASE_COMMON_H

struct PrimaryKeyOrdering {
    template<typename T>
    bool operator()(const T &left, const T &right) const
    {
        return left.primaryKey() < right.primaryKey();
    }
};

#define TABLE(Table) std::set<Table::Item, PrimaryKeyOrdering>

#define DECL_COLUMN(ColumnType, ColumnName)                                                                                                                    \
    inline Column<Item, ColumnType> ColumnName()                                                                                                               \
    {                                                                                                                                                          \
        return Column<Item, ColumnType>(&Item::ColumnName);                                                                                                    \
    }

template<typename Type, typename ColumnType>
class Column
{
    typedef Column<Type, ColumnType> ThisType;

public:
    Column(const ColumnType Type::*memberptr)
        : memberptr(memberptr)
    {
    }

    //_ Column comparator functor
    template<typename Comp1, typename Comp2>
    class CompositeComparator
    {
    public:
        CompositeComparator(Comp1 comp1, Comp2 comp2)
            : comp1(comp1)
            , comp2(comp2)
        {
        }

        inline bool operator()(const Type &left, const Type &right) const
        {
            return comp1(left, right) ? true : comp1(right, left) ? false : comp2(left, right);
        }

    private:
        const Comp1 comp1;
        const Comp2 comp2;
    };

    class Comparator
    {
    public:
        Comparator(const ColumnType Type::*memberptr, bool invert)
            : memberptr(memberptr)
            , invert(invert)
        {
        }

        inline bool operator()(const Type &left, const Type &right) const
        {
            return (invert) ? right.*memberptr < left.*memberptr //
                            : left.*memberptr < right.*memberptr;
        }

        template<typename Comp2>
        CompositeComparator<Comparator, Comp2> operator|(const Comp2 &comp2)
        {
            return CompositeComparator<Comparator, Comp2>(*this, comp2);
        }

    private:
        const ColumnType Type::*memberptr;
        const bool invert;
    };
    //^

    inline Comparator asc() const
    {
        return Comparator(memberptr, false);
    }

    inline Comparator desc() const
    {
        return Comparator(memberptr, true);
    }

    //_ Column filtering functor
    enum ComparisonOperation {
        Less,
        LessOrEqual,
        Equal,
        GreaterOrEqual,
        Greater,
    };

    template<typename T>
    class Filterer
    {
    public:
        Filterer(const ColumnType Type::*memberptr, ComparisonOperation comparison, const T &value)
            : memberptr(memberptr)
            , comparison(comparison)
            , value(value)
        {
        }

        inline bool operator()(const Type &item) const
        {
            return comparison == Less          ? item.*memberptr < value
                : comparison == LessOrEqual    ? item.*memberptr <= value
                : comparison == Equal          ? item.*memberptr == value
                : comparison == GreaterOrEqual ? item.*memberptr >= value
                : comparison == Greater        ? item.*memberptr > value
                                               : false;
        }

    private:
        const ColumnType Type::*memberptr;
        const ComparisonOperation comparison;
        const T value;
    };

#define IMPLEMENT_COMPARISON_OPERATOR(OPERATOR, NAME)                                                                                                          \
    template<typename T>                                                                                                                                       \
    inline Filterer<T> operator OPERATOR(const T &value) const                                                                                                 \
    {                                                                                                                                                          \
        return Filterer<T>(memberptr, NAME, value);                                                                                                            \
    }
    //^

    IMPLEMENT_COMPARISON_OPERATOR(<, Less)
    IMPLEMENT_COMPARISON_OPERATOR(<=, LessOrEqual)
    IMPLEMENT_COMPARISON_OPERATOR(==, Equal)
    IMPLEMENT_COMPARISON_OPERATOR(>=, GreaterOrEqual)
    IMPLEMENT_COMPARISON_OPERATOR(>, Greater)

#undef IMPLEMENT_COMPARISON_OPERATOR

    // Column stuff

private:
    const ColumnType Type::*memberptr;
};

template<typename Range, typename ColumnMemberPointer, typename MergeFunction>
inline auto groupBy(const Range &range, const ColumnMemberPointer &memberptr, const MergeFunction &merge) -> std::vector<typename Range::value_type>
{
    std::vector<typename Range::value_type> result;

    for (const auto &item : range) {
        if (result.size() == 0 || result.back().*memberptr != item.*memberptr) {
            result.push_back(item);
        } else {
            merge(result.back(), item);
        }
    }

    return result;
}

#endif // QUICKCHECK_DATABASE_COMMON_H

// vim: set foldmethod=marker:
