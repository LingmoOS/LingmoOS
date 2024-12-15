// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QVariant>

#include <algorithm>
#include <gtest/gtest.h>

#include <QSignalSpy>

#include "rolecombinemodel.h"
#include "combinemodela.h"
#include "combinemodelb.h"

TEST(RoleCombineModel, RoleNamesTest) {
    TestModelA modelA;
    TestModelB modelB;
    RoleCombineModel model(&modelA, &modelB, 1, [](QVariant data, QAbstractItemModel* model) -> QModelIndex{ return QModelIndex();});
    QHash<int, QByteArray> res(modelA.roleNames());
    auto keys = modelA.roleNames().keys();
    auto maxKey = *(std::max_element(keys.begin(), keys.end())) + 1;
    for (auto c : modelB.roleNames()) {
        res.insert(maxKey++, c);
    }

    EXPECT_EQ(model.roleNames(), res);
}

TEST(RoleCombineModel, RowCountTest) {
    TestModelA modelA;
    TestModelB modelB;
    modelA.addData(new DataA(0, &modelA));
    modelA.addData(new DataA(1, &modelA));

    modelB.addData(new DataB(0, &modelB));
    modelB.addData(new DataB(1, &modelB));
    modelB.addData(new DataB(2, &modelB));
    RoleCombineModel model(&modelA, &modelB, TestModelA::idRole, [](QVariant data, QAbstractItemModel* model) -> QModelIndex 
        {
            return QModelIndex();
        }
    );

    // modelA is major dataModel
    EXPECT_EQ(model.rowCount(), modelA.rowCount());
    EXPECT_NE(model.rowCount(), modelB.rowCount());
}

TEST(RoleCombineModel, dataTest) {
    TestModelA modelA;
    TestModelB modelB;
    RoleCombineModel model(&modelA, &modelB, TestModelA::idRole, [](QVariant data, QAbstractItemModel* model) -> QModelIndex 
        {
            auto c = model->match(model->index(0, 0), TestModelB::idRole, data);
            if (c.size() > 0) {
                return c.first();
            }

            return QModelIndex();
        }
    );

    QSignalSpy spyA(&modelA, &QAbstractItemModel::dataChanged);
    QSignalSpy spyB(&modelB, &QAbstractItemModel::dataChanged);
    QSignalSpy spy(&model, &QAbstractItemModel::dataChanged);

    modelA.addData(new DataA(0, "dataA0", &modelA));
    modelA.addData(new DataA(1, "dataA1", &modelA));
    modelA.addData(new DataA(3, "dataA3", &modelA));

    modelB.addData(new DataB(0, "dataB0", &modelB));
    modelB.addData(new DataB(1, "dataB1", &modelB));
    modelB.addData(new DataB(2, "dataB2",&modelB));

    auto roleNames = model.roleNames();
    auto roleNamesA = modelA.roleNames();
    auto roleNamesB = modelB.roleNames();

    QHash<QByteArray, int> names2Role;

    for (auto roleName : roleNames.keys()) {
        names2Role.insert(roleNames.value(roleName), roleName);
    }

    // below data are same
    EXPECT_EQ(model.index(0, 0).data(names2Role.value(roleNamesA.value(TestModelA::dataRole))), modelA.index(0, 0).data(TestModelA::dataRole));
    EXPECT_EQ(model.index(1, 0).data(names2Role.value(roleNamesA.value(TestModelA::dataRole))), modelA.index(1, 0).data(TestModelA::dataRole));

    EXPECT_EQ(model.index(0, 0).data(names2Role.value(roleNamesB.value(TestModelB::dataRole))), modelB.index(0, 0).data(TestModelB::dataRole));
    EXPECT_EQ(model.index(1, 0).data(names2Role.value(roleNamesB.value(TestModelB::dataRole))), modelB.index(1, 0).data(TestModelB::dataRole));

    // // because lack of dataB id is 3, so will return a empty QString
    EXPECT_EQ(model.index(2, 0).data(names2Role.value(roleNamesB.value(TestModelB::dataRole))).toString(), QString());

    // below data are difference, because combine func can find id 2 for DataA;
    EXPECT_NE(model.index(2, 0).data(names2Role.value(roleNamesB.value(TestModelB::dataRole))), modelA.index(1, 0).data(TestModelB::dataRole));

    modelB.setData(modelB.index(1), "dataB22");
    EXPECT_EQ(model.index(1, 0).data(names2Role.value(roleNamesB.value(TestModelB::dataRole))), modelB.index(1, 0).data(TestModelB::dataRole));
}
