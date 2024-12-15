/*
 * SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */
#include <gtest/gtest.h>
#include <QWidget>
#include <private/qguiapplication_p.h>
#include "filedialogmanagerservice.h"
#include "filedialog_interface.h"
#include "qlingmofiledialoghelper.h"
#include <QTimer>
#include <QFileDialog>

class TestQLingmoFileDialogHelper : public testing::Test
{
public:
    static void SetUpTestSuite()
    {
        qputenv("_d_fileDialogServiceName", DIALOG_SERVICE);
        manager = new FileDialogManagerService(DIALOG_SERVICE, "/com/lingmo/filemanager/filedialogmanager");
        QLingmoFileDialogHelper::initDBusFileDialogManager();
    }
    static void TearDownTestSuite()
    {
        qunsetenv("_d_fileDialogServiceName");
        delete manager;
    }

    void SetUp() override
    {
        helper = new QLingmoFileDialogHelper;
        nativedlg = new QFileDialog;
    }

    void TearDown() override
    {
        delete helper;
        delete nativedlg;
    }

    void ensureDialog()
    {
        if (filedlg)
            return;
        if (helper) {
            helper->ensureDialog();
            if (helper->filedlgInterface) {
                auto dialogPath = helper->filedlgInterface->path();
                filedlg = manager->m_dialogMap[QDBusObjectPath{dialogPath}];
            }
        }
    }

private:
    QLingmoFileDialogHelper *helper{nullptr};
    FileDialogService *filedlg{nullptr};
    QFileDialog *nativedlg{nullptr};
    static FileDialogManagerService *manager;
};

FileDialogManagerService *TestQLingmoFileDialogHelper::manager = nullptr;

TEST_F(TestQLingmoFileDialogHelper, ensureDialog)
{
    EXPECT_EQ(nullptr, filedlg);
    EXPECT_NE(nullptr, helper);
    ensureDialog();
    EXPECT_NE(nullptr, filedlg);
}

TEST_F(TestQLingmoFileDialogHelper, show)
{
    helper->setOptions(QFileDialogOptions::create());
    bool success = helper->show(Qt::Dialog, Qt::ApplicationModal, nativedlg->windowHandle());
    ASSERT_TRUE(success);
    EXPECT_TRUE(QGuiApplicationPrivate::self->modalWindowList.contains(helper->auxiliaryWindow));
    EXPECT_EQ(helper->auxiliaryWindow, qApp->modalWindow());
}

TEST_F(TestQLingmoFileDialogHelper, exec)
{
    ensureDialog();
    QTimer::singleShot(1, filedlg, &FileDialogService::accept);
    helper->exec();
    EXPECT_TRUE(filedlg->m_visible);
    // Ensure that auxiliary window is non-modal after exec
    ASSERT_NE(nullptr, helper->auxiliaryWindow);
    EXPECT_FALSE(QGuiApplicationPrivate::self->modalWindowList.contains(helper->auxiliaryWindow));
}

TEST_F(TestQLingmoFileDialogHelper, hide)
{
    ensureDialog();
    QTimer::singleShot(1, helper, &QLingmoFileDialogHelper::hide);
    helper->exec();
    EXPECT_FALSE(filedlg->m_visible);
    ASSERT_NE(nullptr, helper->auxiliaryWindow);
    EXPECT_FALSE(QGuiApplicationPrivate::self->modalWindowList.contains(helper->auxiliaryWindow));
}


TEST_F(TestQLingmoFileDialogHelper, setDirectory)
{
    ensureDialog();
    filedlg->setDirectory("/test");
    helper->setDirectory(QUrl("/dtk"));
    EXPECT_EQ("/dtk", filedlg->m_directoryUrl);
}

TEST_F(TestQLingmoFileDialogHelper, directory)
{
    ensureDialog();
    filedlg->m_directoryUrl = "/test";
    EXPECT_EQ(filedlg->directory(), "/test");
}

TEST_F(TestQLingmoFileDialogHelper, selectFile)
{
    ensureDialog();
    filedlg->m_selectedFiles.clear();
    helper->selectFile(QUrl{"/test"});
    ASSERT_EQ(1, filedlg->m_selectedFiles.size());
    EXPECT_EQ(QUrl{"/test"}, filedlg->m_selectedFiles[0]);
}

TEST_F(TestQLingmoFileDialogHelper, selectedFiles)
{
    ensureDialog();
    filedlg->m_selectedFiles = {
        QUrl{"/test1"},
        QUrl{"/test2"},
        QUrl{"/test3"}
    };;
    auto result = helper->selectedFiles();
    ASSERT_EQ(3, result.size());
    EXPECT_EQ("/test1", result[0].toString());
    EXPECT_EQ("/test2", result[1].toString());
    EXPECT_EQ("/test3", result[2].toString());
}

TEST_F(TestQLingmoFileDialogHelper, setFilter)
{
    ensureDialog();
    filedlg->m_filter = 0;
    EXPECT_EQ(0, filedlg->filter());
    // TODO Add test for setFilter. Now not sure why options are empty when helper created.
}

TEST_F(TestQLingmoFileDialogHelper, selectNameFilter)
{
    ensureDialog();
    filedlg->m_nameFilters.clear();
    helper->selectNameFilter("*.cpp");
    EXPECT_EQ(1, filedlg->m_nameFilters.size());
    EXPECT_EQ("*.cpp", filedlg->m_nameFilters[0]);
}

TEST_F(TestQLingmoFileDialogHelper, selectedNameFilters)
{
    ensureDialog();
    filedlg->m_nameFilters = QStringList{
        "*.cpp",
        "*.h"
    };
    auto result = helper->selectedNameFilter();
    EXPECT_EQ("*.cpp;*.h", result);
}
