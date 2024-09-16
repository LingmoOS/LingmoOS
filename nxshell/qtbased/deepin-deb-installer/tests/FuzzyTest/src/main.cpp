// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <stdint.h>
#include <stddef.h>
#include <iostream>
using namespace std;

#include <QApt/Backend>
#include <QApt/DebFile>

using namespace QApt;

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    Q_UNUSED(size);
    Q_UNUSED(data);

//    DebFile *debfile = new DebFile(reinterpret_cast<char *>(const_cast<uint8_t *>(data)));
//    debfile->isValid();
//    delete debfile;
    return 0;
}



