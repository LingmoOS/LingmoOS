// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef OFD_H
#define OFD_H

#include <string>
#include <vector>
#include "fileext/fileext.hpp"

namespace ofd {

class Ofd : public fileext::FileExtension
{
public:
    Ofd(const std::string &fileName);
    virtual ~Ofd() = default;

    int convert(bool addStyle = true, bool extractImages = false, char mergingMode = 0) override;
};

}   // namespace ofd

#endif   // OFD_H
