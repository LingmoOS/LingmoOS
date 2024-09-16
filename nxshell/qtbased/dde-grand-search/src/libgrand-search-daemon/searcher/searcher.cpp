// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "searcher.h"

using namespace GrandSearch;

Searcher::Searcher(QObject *parent) : QObject(parent)
{

}

bool Searcher::activate()
{
    return false;
}
