@page dtksearch dtksearch
@brief dtk 搜索业务模块

# 项目介绍

dtksearch 是为 deepin/UOS 系统上对搜索业务进行接口封装，为应用层开发者提供了对文件和文件内容的搜索功能，其目的是在于方便第三方开发者轻松且快速的调用接口进行开发。

@ref group_dtksearch "接口文档"


## 项目结构

对外暴露出 `dsearch.h`、`dtksearch_global.h`、`dtksearchtypes.h` 这 3 个头文件。

# 如何使用项目

如果要使用此项目，可以阅读相关文档再参照以下示例进行调用。

## 使用示例

`settingdialog.h`

```c++
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef SETTINGDIALOG_H
#define SETTINGDIALOG_H

#include <DDialog>
#include <DLineEdit>
#include <DComboBox>
#include <DSpinBox>

DWIDGET_USE_NAMESPACE
class SettingDialog : public DDialog
{
    Q_OBJECT
public:
    explicit SettingDialog(QWidget *parent = nullptr);

    QVariantMap searchConfig();

private:
    void initUI();

private:
    DLineEdit *pathEdit { nullptr };
    DLineEdit *indexPathEdit { nullptr };
    DComboBox *modeCheckBox { nullptr };
    DSpinBox *maxResultBox { nullptr };
};

#endif   // SETTINGDIALOG_H

```

`settingdialog.cpp`

```c++
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "settingdialog.h"

#include <DLabel>

#include <QGridLayout>
#include <QStandardPaths>
#include <QDir>

SettingDialog::SettingDialog(QWidget *parent)
    :DDialog (parent)
{
    initUI();
}

QVariantMap SettingDialog::searchConfig()
{
    QVariantMap map;
    map.insert("path", pathEdit->text());
    map.insert("index", indexPathEdit->text());
    map.insert("mode", modeCheckBox->currentText());
    map.insert("count", maxResultBox->value());

    return map;
}

void SettingDialog::initUI()
{
    QWidget *widget = new QWidget(this);
    QGridLayout *layout = new QGridLayout;
    widget->setLayout(layout);

    DLabel *modeLabel = new DLabel(tr("Search mode:"), this);
    modeCheckBox = new DComboBox(this);
    modeCheckBox->addItems({tr("File name"), tr("Pinyin"), tr("Full-Text"), tr("All")});

    DLabel *pathLabel = new DLabel(tr("Search path:"), this);
    pathEdit = new DLineEdit(this);
    pathEdit->setText(QStandardPaths::writableLocation(QStandardPaths::DesktopLocation));

    DLabel *indexPathLabel = new DLabel(tr("Index path:"), this);
    indexPathEdit = new DLineEdit(this);
    indexPathEdit->setText(QDir::currentPath() + "/index");

    DLabel *maxResultLabel = new DLabel(tr("Max result count:"), this);
    maxResultBox = new DSpinBox(this);
    maxResultBox->setMaximum(INT32_MAX);
    maxResultBox->setMinimum(1);
    maxResultBox->setValue(100);

    layout->addWidget(modeLabel, 0, 0);
    layout->addWidget(modeCheckBox, 0, 1);
    layout->addWidget(pathLabel, 1, 0);
    layout->addWidget(pathEdit, 1, 1);
    layout->addWidget(indexPathLabel, 2, 0);
    layout->addWidget(indexPathEdit, 2, 1);
    layout->addWidget(maxResultLabel, 3, 0);
    layout->addWidget(maxResultBox, 3, 1);

    addButton(tr("Save"), true, DDialog::ButtonRecommend);
    addContent(widget);
}
```

`mainwindow.h`

```c++
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "settingdialog.h"

#include <DSearch>

#include <DMainWindow>
#include <DPushButton>
#include <DLineEdit>
#include <DListView>
#include <DSearchEdit>

#include <QStandardItemModel>

DSEARCH_USE_NAMESPACE
DWIDGET_USE_NAMESPACE

class MainWindow : public DMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);

public Q_SLOTS:
    void onSearchBtnClicked();
    void onActionTriggered(QAction *action);
    void onMatched();
    void onCompleted();
    void onStoped();

private:
    void initUI();
    void initMenu();
    void initConnect();

private:
    DSearch *searchObj { nullptr };

    DPushButton *searchBtn { nullptr };
    DSearchEdit *searchEdit { nullptr };
    DListView *resultView { nullptr };
    SettingDialog *settingDialog { nullptr };
    QStandardItemModel *model { nullptr };
};

#endif   // MAINWINDOW_H
```

`mainwindow.cpp`

```c++
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "mainwindow.h"

#include <dtksearchtypes.h>

#include <DLabel>
#include <DTitlebar>

#include <QMenu>
#include <QVBoxLayout>
#include <QMap>

MainWindow::MainWindow(QWidget *parent)
    : DMainWindow(parent)
    , searchObj(new DSearch(this))
{
    initUI();
    initMenu();
    initConnect();
}

void MainWindow::onSearchBtnClicked()
{

    static QMap<QString, SearchFlag> modeMap { { tr("File name"), SearchFlag::FileName },
                                               { tr("Pinyin"), SearchFlag::PinYin },
                                               { tr("Full-Text"), SearchFlag::FullText },
                                               { tr("All"), SearchFlag::AllEntries } };

    if (!searchBtn->property("isStop").toBool()) {
        model->clear();

        const auto &map = settingDialog->searchConfig();
        const auto &path = map["path"].toString();
        const auto &index = map["index"].toString();
        const auto &maxCount = map["count"].toUInt();
        const auto &mode = map["mode"].toString();

        searchObj->setIndexPath(index);
        searchObj->setSearchFlags(modeMap[mode]);
        searchObj->setMaxResultCount(maxCount);

        if (searchObj->search(path, searchEdit->text())) {
            searchBtn->setText(tr("Stop"));
            searchBtn->setProperty("isStop", true);
        }
    } else {
        searchObj->stop();
    }
}

void MainWindow::onActionTriggered(QAction *action)
{
    if (action->data().isValid() && action->data().toString() == "SettingDialog") {
        settingDialog->show();
        settingDialog->raise();
    }
}

void MainWindow::onMatched()
{
    const auto &results = searchObj->matchedResults();
    for (const auto &r : results)
        model->appendRow(new QStandardItem(r));
}

void MainWindow::onCompleted()
{
    searchBtn->setText(tr("Search"));
    searchBtn->setProperty("isStop", false);
}

void MainWindow::onStoped()
{
    searchBtn->setText(tr("Search"));
    searchBtn->setProperty("isStop", false);
}

void MainWindow::initUI()
{
    setMinimumSize(650, 600);
    settingDialog = new SettingDialog(this);
    resultView = new DListView(this);
    model = new QStandardItemModel(resultView);
    resultView->setModel(model);
    resultView->setGridSize(QSize(10, 30));
    resultView->setItemSize(QSize(10, 40));

    DLabel *label = new DLabel(tr("Keyword:"), this);
    searchEdit = new DSearchEdit(this);
    searchBtn = new DPushButton(tr("Search"), this);
    searchBtn->setProperty("isStop", false);

    QHBoxLayout *hlayout = new QHBoxLayout;
    hlayout->addWidget(label);
    hlayout->addWidget(searchEdit);
    hlayout->addWidget(searchBtn);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(hlayout);
    mainLayout->addWidget(resultView);

    QWidget *mainWidget = new QWidget(this);
    mainWidget->setLayout(mainLayout);
    this->setCentralWidget(mainWidget);
}

void MainWindow::initMenu()
{
    QMenu *menu = new QMenu(this);
    QAction *act = new QAction(tr("Setting"), this);
    act->setData("SettingDialog");
    menu->addAction(act);
    titlebar()->setMenu(menu);
}

void MainWindow::initConnect()
{
    connect(searchBtn, &DPushButton::clicked, this, &MainWindow::onSearchBtnClicked);
    connect(searchEdit, &DLineEdit::returnPressed, this, &MainWindow::onSearchBtnClicked);

    connect(titlebar()->menu(), &QMenu::triggered, this, &MainWindow::onActionTriggered);
    connect(searchObj, &DSearch::matched, this, &MainWindow::onMatched);
    connect(searchObj, &DSearch::completed, this, &MainWindow::onCompleted);
    connect(searchObj, &DSearch::stoped, this, &MainWindow::onStoped);
}
```

`main.cpp`

```c++
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "mainwindow.h"

#include <DApplication>

#include <memory>

DWIDGET_USE_NAMESPACE
int main(int argc, char *argv[])
{
    std::unique_ptr<DApplication> a(DApplication::globalApplication(argc, argv));

    MainWindow w;
    w.show();

    return a->exec();
}
```

@defgroup dtksearch
@brief 搜索业务模块
@details 示例文档:
@subpage dtksearch

@anchor group_dtksearch