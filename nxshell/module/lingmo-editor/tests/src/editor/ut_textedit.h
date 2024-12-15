// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TEST_TEXTEDIT_H
#define TEST_TEXTEDIT_H


#include "../../src/common/settings.h"
#include "../../src/controls/tabbar.h"
#include "../../src/editor/editwrapper.h"
#include "../../src/widgets/window.h"
#include "../../src/startmanager.h"
#include "../../src/editor/dtextedit.h"
#include "../../src/common/CSyntaxHighlighter.h"
#include "../../src/editor/uncommentselection.h"
#include "../../src/editor/showflodcodewidget.h"
#include <QMenu>

#include "../stub.h"
#include <QEvent>
#include "gtest/gtest.h"
#include <QObject>
#include <QClipboard>
#include <QEvent>
#include <QKeyEvent>
#include <QAbstractScrollArea>
#include <QPlainTextEdit>
#include <QRegExp>
#include <QTextDocument>
#include <QScrollBar>
#include <QUndoStack>
#include <DMenu>
#include <QList>
#include <QFile>
#include <QChar>
#include <QPaintEvent>

#include <DSettingsOption>

class test_textedit: public QObject, public::testing::Test
{
public:
    test_textedit();
    void forstub(QPoint q);
};

#endif // TEST_TEXTEDIT_H
