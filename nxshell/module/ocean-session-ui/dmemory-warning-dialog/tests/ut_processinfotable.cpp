// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#define private public
#define protected public
#include "processinfotable.h"
#include "processinfomodel.h"
#include "processinfodelegate.h"
#include "buttondelegate.h"
#include "dmemorywarningdialog.h"
#undef private
#undef protected

#include <DDialog>

#include <QTest>
#include <QMouseEvent>
#include <QPainter>

#include <gtest/gtest.h>


class Ut_ProcessInfoView : public testing::Test
{

};

TEST_F(Ut_ProcessInfoView, coverageTest)
{
    ProcessInfoModel model;
    ProcessInfoView table;
    table.setModel(&model);
    table.setItemDelegate(new ProcessInfoDelegate(&table));
    table.setItemDelegateForColumn(3, new ButtonDelegate(&table));
    table.show();
    table.update();
    QMouseEvent mouseEvent(QEvent::MouseButtonRelease, QPointF(1,1), Qt::LeftButton, Qt::NoButton, Qt::NoModifier);
    table.mouseReleaseEvent(&mouseEvent);
    QTest::mouseRelease(&table, Qt::LeftButton);
    model.startRefreshData();
    QTest::qWait(500);
    int count = model.rowCount(QModelIndex());
    if (count > 0) {
        ButtonDelegate * colunmDelegate = qobject_cast<ButtonDelegate *>(table.itemDelegateForColumn(3));
        QEvent event(QEvent::MouseButtonPress);
        colunmDelegate->editorEvent(&event, &model, QStyleOptionViewItem(), model.index(0, 0, QModelIndex()));
        event = QEvent(QEvent::MouseButtonRelease);

        {
            DMemoryWarningDialog dialog;
            auto closeConfirmDialog = [ &dialog ] {
                QObjectList children = dialog.children();
                for (auto child : children) {
                    DDialog *dist = qobject_cast<DDialog*>(child);
                    if (dist) {
                        auto cancleBtn = dist->getButton(0);
                        if (cancleBtn)
                            QTest::mouseClick(cancleBtn, Qt::LeftButton, Qt::NoModifier);
                    }
                }
            };

            //防呆,一秒钟内进程未终止则强行杀死,避免卡住后续的流程
            QTimer::singleShot(1000, [] {
                qApp->exit(-1);
            });

            //点击关闭app按钮后会弹窗,延时关闭弹窗,否则程序卡住无法进行
            QTimer::singleShot(100, closeConfirmDialog);
            colunmDelegate->editorEvent(&event, &model, QStyleOptionViewItem(), model.index(0, 0, QModelIndex()));
            QTimer::singleShot(100, closeConfirmDialog);

            colunmDelegate->editorEvent(&event, &model, QStyleOptionViewItem(),  QModelIndex());
        }

        model.data(model.index(0, 0, QModelIndex()), ProcessInfoModel::TabListRole);
        model.clearPressed();
    }
}
