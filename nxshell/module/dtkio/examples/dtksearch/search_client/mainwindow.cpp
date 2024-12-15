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
