// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QApplication>
#include <QDir>
#include <QStandardPaths>
#include <QDebug>

#if defined(CMAKE_SAFETYTEST_ARG_ON)
#include <sanitizer/asan_interface.h>
#endif

#include "common/vnotedatamanager.h"
#include "common/vnoteforlder.h"
#include "common/metadataparser.h"

#include "common/vnoteitem.h"
#include "db/vnoteitemoper.h"
#include "db/vnotefolderoper.h"

class GlobalEnvent : public testing::Environment
{
protected:
    virtual void SetUp() override;
    virtual void TearDown() override;
};

void GlobalEnvent::SetUp()
{
    VNoteDataManager *dataManager = VNoteDataManager::instance();
    dataManager->m_qspNoteFoldersMap.reset(new VNOTE_FOLDERS_MAP());
    dataManager->m_qspAllNotesMap.reset(new VNOTE_ALL_NOTES_MAP());
    QString defaultIconPathFmt(":/icons/deepin/builtin/default_folder_icons/%1.svg");

    for (int i = 0; i < 10; i++) {
        QString iconPath = defaultIconPathFmt.arg(i + 1);
        QPixmap bitmap(iconPath);
        VNoteDataManager::m_defaultIcons[IconsType::DefaultIcon].push_back(bitmap);
        VNoteDataManager::m_defaultIcons[IconsType::DefaultGrayIcon].push_back(bitmap);
    }

    for (int i = 0; i < 2; i++) {
        VNoteFolder *folder = new VNoteFolder;
        folder->name = "folderDefault";
        folder->createTime = QDateTime::currentDateTime();
        folder->modifyTime = QDateTime::currentDateTime();
        folder->deleteTime = QDateTime::currentDateTime();
        folder->id = i;
        folder->UI.icon = VNoteDataManager::instance()->getDefaultIcon(
            0, IconsType::DefaultIcon);
        folder->UI.grayIcon = VNoteDataManager::instance()->getDefaultIcon(
            0, IconsType::DefaultGrayIcon);
        dataManager->addFolder(folder);

        for (int j = 0; j < 2; j++) {
            VNoteItem *note = new VNoteItem;
            note->folderId = folder->id;
            note->noteId = i + j;
            note->noteTitle = "noteDefault";
            folder->maxNoteIdRef()++;
            note->setFolder(folder);
            for (int k = 0; k < 5; k++) {
                VNoteBlock *ptrBlock = nullptr;
                if (k % 2 == 0) {
                    ptrBlock = note->newBlock(VNoteBlock::Text);
                } else {
                    ptrBlock = note->newBlock(VNoteBlock::Voice);
                    ptrBlock->ptrVoice->voiceSize = 0;
                    ptrBlock->ptrVoice->voiceTitle = "voicetitle";
                }
                ptrBlock->blockText = "blockDefault";
                note->addBlock(ptrBlock);
            }
            MetaDataParser metaParser;
            metaParser.makeMetaData(note, note->metaDataRef());
            VNoteDataManager::instance()->addNote(note);
        }
    }
}

void GlobalEnvent::TearDown()
{
    ;
}

int main(int argc, char *argv[])
{
    qputenv("QT_QPA_PLATFORM", "offscreen");
    QApplication app(argc, argv);

    testing::InitGoogleTest(&argc, argv);
    testing::AddGlobalTestEnvironment(new GlobalEnvent);
    int ret = RUN_ALL_TESTS();

#if defined(CMAKE_SAFETYTEST_ARG_ON)
    __sanitizer_set_report_path("asan.log");
#endif

    return ret;
}
