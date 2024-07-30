/*
    SPDX-FileCopyrightText: 2015 Chusslove Illich <caslav.ilic@gmx.net>

    SPDX-License-Identifier: MIT
*/

#include "test_translation_kde.h"
#include <QGuiApplication>

int main(int argc, char **argv)
{
    QGuiApplication app(argc, argv);
    Q_UNUSED(app);
    TestNameSpace::TestTranslationKde t = TestNameSpace::TestTranslationKde();
    return 0;
}
