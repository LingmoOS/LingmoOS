// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ADDON_H
#define ADDON_H

#include <QObject>

namespace org {
namespace lingmo {
namespace dim {

class Dim;

class Addon : public QObject
{
    Q_OBJECT

public:
    explicit Addon(Dim *dim, const std::string &key);
    virtual ~Addon();

    inline Dim *dim() const { return dim_; }

    inline const std::string &key() const { return key_; }

private:
    Dim *dim_;
    std::string key_;
};

#define DIM_ADDON_FACTORY(name) \
    extern "C" {                \
    Addon *create(Dim *dim)     \
    {                           \
        return new name(dim);   \
    }                           \
    }

typedef Addon *addonCreate(Dim *);

} // namespace dim
} // namespace lingmo
} // namespace org

#endif // !ADDON_H
