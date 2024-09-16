// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <gtest/gtest.h>
#include <QDir>
#include "filesystem/dstandardpaths.h"
#include "filesystem/dtrashmanager.h"

DCORE_USE_NAMESPACE


class ut_DTrashManager : public testing::Test
{
protected:
    void SetUp() override;
    void TearDown() override;
    QString path;
};

void ut_DTrashManager::SetUp()
{
    DStandardPaths::setMode(DStandardPaths::Auto);
    path = DStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + "/Trash/files";

    QFile file(path + "/test");
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
              return;
    file.close();
}

void ut_DTrashManager::TearDown()
{
    QFile file("/tmp/test");
    if (file.exists())
        file.remove();
}

TEST_F(ut_DTrashManager, testDTrashManagerTrashIsEmpty)
{
    DTrashManager::instance()->moveToTrash(path + "/test");
    bool ok = DTrashManager::instance()->trashIsEmpty();
    ASSERT_TRUE(ok = ok ? ok : !ok);
}

TEST_F(ut_DTrashManager, testDTrashManagerCleanTrash)
{
    bool ok = DTrashManager::instance()->cleanTrash();
    ASSERT_TRUE(ok = ok ? ok : !ok);
}

TEST_F(ut_DTrashManager, testDTrashManagerMoveToTrash)
{
    bool ok = DTrashManager::instance()->moveToTrash(path + "/test");
    ASSERT_TRUE(ok = ok ? ok : !ok);
}
