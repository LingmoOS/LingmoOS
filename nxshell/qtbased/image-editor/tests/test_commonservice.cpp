// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "gtestview.h"
#include "service/commonservice.h"

TEST_F(gtestview, cp2Image)
{
    for (int i = 0; i < 200; i++) {
        QFile::copy(":/jpg.jpg", QApplication::applicationDirPath() + "/test/jpg" + QString::number(i) + ".jpg");
        QFile(QApplication::applicationDirPath() + "/test/jpg" + QString::number(i) + ".jpg").setPermissions(\
                                                                                                             QFile::WriteUser | QFile::ReadUser | QFile::WriteOther | \
                                                                                                             QFile::ReadOther | QFile::ReadGroup | QFile::WriteGroup);
        EXPECT_EQ(true, QFileInfo(QApplication::applicationDirPath() + "/test/jpg" + QString::number(i) + ".jpg").isFile());
    }

}

TEST_F(gtestview, commonservice_reName)
{
    QString oldName = QApplication::applicationDirPath() + "/test/jpg.jpg";
    QString newName = QApplication::applicationDirPath() + "/test/jpgxxx.jpg";

    LibCommonService::instance()->reName(oldName, newName);
//    EXPECT_EQ(true, bRet);
}

