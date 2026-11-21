#ifndef TOOLS_H
#define TOOLS_H

#include <algorithm>
#include <string>
#include <type_traits>
#include <vector>

#include "base/geometry.h"
#include "base/logging.h"

// Where the keyboard layouts (*.keytab) and the colour schemes are found; the
// installed data directory unless set.
std::string get_kb_layout_dir();
void set_kb_layout_dir(const std::string& dir);
void set_color_scheme_dir(const std::string& dir);
void add_custom_color_scheme_dir(const std::string& custom_dir);
const std::vector<std::string> get_color_schemes_dirs();

extern const Lingmo::base::LogCategory qtermwidgetLogger;

// qRound.
inline int kRound(double value)
{
    return Lingmo::base::roundToInt(value);
}

// qMin, qMax and qBound, which take mixed argument types.
template<typename T, typename U>
constexpr std::common_type_t<T, U> kMin(const T& a, const U& b)
{
    using C = std::common_type_t<T, U>;
    return C(a) < C(b) ? C(a) : C(b);
}

template<typename T, typename U>
constexpr std::common_type_t<T, U> kMax(const T& a, const U& b)
{
    using C = std::common_type_t<T, U>;
    return C(a) < C(b) ? C(b) : C(a);
}

template<typename T>
constexpr const T& kBound(const T& min, const T& value, const T& max)
{
    return std::max(min, std::min(max, value));
}

#endif
