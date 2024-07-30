/*
    SPDX-FileCopyrightText: 2015 Ivan Cukic <ivan.cukic(at)kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef UTILS_LAZY_VAL_H
#define UTILS_LAZY_VAL_H

namespace kamd
{
namespace utils
{
template<typename F>
class lazy_val
{
public:
    lazy_val(F f)
        : _f(std::forward<F>(f))
        , valueRetrieved(false)
    {
    }

private:
    F _f;
    mutable decltype(_f()) value;
    mutable bool valueRetrieved;

public:
    operator decltype(_f())() const
    {
        if (!valueRetrieved) {
            valueRetrieved = true;
            value = _f();
        }

        return value;
    }
};

template<typename F>
inline lazy_val<F> make_lazy_val(F &&f)
{
    return lazy_val<F>(std::forward<F>(f));
}

} // namespace utils
} // namespace kamd

#endif // UTILS_LAZY_VAL_H
