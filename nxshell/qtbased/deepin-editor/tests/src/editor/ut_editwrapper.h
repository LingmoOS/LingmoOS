// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UT_Editwrapper_H
#define UT_Editwrapper_H

#include "../../src/startmanager.h"
#include "../../src/editor/dtextedit.h"
#include "../../src/common/settings.h"
#include "../../src/widgets/window.h"
#include "../../src/editor/editwrapper.h"
#include "../../src/controls/tabbar.h"
#include "../../src/common/utils.h"
#include "../../src/encodes/detectcode.h"
#include "../stub.h"
#include "gtest/gtest.h"
#include <QObject>
#include <DFileDialog>
#include <QFileDialog>
#include <QDialog>

DWIDGET_USE_NAMESPACE

class UT_Editwrapper: public QObject, public::testing::Test
{
public:
    UT_Editwrapper();
    QByteArray FileLoadThreadRun(const QString &strFilePath, QByteArray *encode);
};

#endif // UT_Editwrapper_H
