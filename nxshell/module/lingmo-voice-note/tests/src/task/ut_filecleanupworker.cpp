// Copyright (C) 2019 ~ 2019 UnionTech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_filecleanupworker.h"

#include <QStandardPaths>

UT_FileCleanupWorker::UT_FileCleanupWorker()
{
}

void UT_FileCleanupWorker::SetUp()
{
    qspAllNotesMap = new VNOTE_ALL_NOTES_MAP();
    voiceItem = new VNOTE_ITEMS_MAP();
    qspAllNotesMap->notes.insert(0, voiceItem);
    VNoteItem *note = new VNoteItem();
    note->htmlCode = "<div> <p> </div>";
    voiceItem->folderNotes.insert(0, note);
    voiceItem->autoRelease = true;
    VNoteItem *note2 = new VNoteItem();
    voiceItem->folderNotes.insert(1, note2);
    qspAllNotesMap->autoRelease = true;
}

void UT_FileCleanupWorker::TearDown()
{
    delete qspAllNotesMap;
}

TEST_F(UT_FileCleanupWorker, UT_FileCleanupWorker_run_001)
{
    FileCleanupWorker *work = new FileCleanupWorker(nullptr);
    work->run();

    delete work;
}

TEST_F(UT_FileCleanupWorker, UT_FileCleanupWorker_run_002)
{
    FileCleanupWorker *work = new FileCleanupWorker(qspAllNotesMap);
    work->run();
    delete work;
}

TEST_F(UT_FileCleanupWorker, UT_FileCleanupWorker_cleanVoice_001)
{
    FileCleanupWorker *work = new FileCleanupWorker(qspAllNotesMap);
    work->cleanVoice();
    delete work;
}

TEST_F(UT_FileCleanupWorker, UT_FileCleanupWorker_cleanPicture_001)
{
    FileCleanupWorker *work = new FileCleanupWorker(qspAllNotesMap);
    work->cleanPicture();
    delete work;
}

TEST_F(UT_FileCleanupWorker, UT_FileCleanupWorker_fillVoiceSet_001)
{
    FileCleanupWorker *work = new FileCleanupWorker(qspAllNotesMap);
    QString dirPath = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
    QDir(dirPath).mkdir("/voicenote");
    work->fillVoiceSet();

    QDir(dirPath).remove("/voicenote");
    work->fillVoiceSet();
    delete work;
}

TEST_F(UT_FileCleanupWorker, UT_FileCleanupWorker_fillPictureSet_001)
{
    FileCleanupWorker *work = new FileCleanupWorker(qspAllNotesMap);
    QString dirPath = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
    QDir(dirPath).mkdir("/images");
    work->fillPictureSet();

    QDir(dirPath).remove("/images");
    work->fillPictureSet();
    delete work;
}

TEST_F(UT_FileCleanupWorker, UT_FileCleanupWorker_scanAllNotes_001)
{
    FileCleanupWorker *work = new FileCleanupWorker(qspAllNotesMap);
    work->scanAllNotes();

    delete work;
}

TEST_F(UT_FileCleanupWorker, UT_FileCleanupWorker_removeVoicePathBySet_001)
{
    FileCleanupWorker *work = new FileCleanupWorker(qspAllNotesMap);
    work->removeVoicePathBySet("");
    work->removeVoicePathBySet("/test");
    delete work;
}

TEST_F(UT_FileCleanupWorker, UT_FileCleanupWorker_removePicturePathBySet_001)
{
    FileCleanupWorker *work = new FileCleanupWorker(qspAllNotesMap);
    work->removePicturePathBySet("");
    work->removePicturePathBySet("/test");
    delete work;
}

TEST_F(UT_FileCleanupWorker, UT_FileCleanupWorker_scanVoiceByHtml_001)
{
    FileCleanupWorker *work = new FileCleanupWorker(qspAllNotesMap);
    work->scanVoiceByHtml("");

    work->scanVoiceByHtml("test");
    delete work;
}

TEST_F(UT_FileCleanupWorker, UT_FileCleanupWorker_scanVoiceByHtml_002)
{
    QString metadata = "<div jsonkey=\"/test/test/voicenote/sad23.mp3\"> </div>";

    FileCleanupWorker *work = new FileCleanupWorker(qspAllNotesMap);
    work->scanVoiceByHtml(metadata);
    delete work;
}

TEST_F(UT_FileCleanupWorker, UT_FileCleanupWorker_scanPictureByHtml_001)
{
    FileCleanupWorker *work = new FileCleanupWorker(qspAllNotesMap);
    work->scanPictureByHtml("");

    work->scanPictureByHtml("test");
    delete work;
}

TEST_F(UT_FileCleanupWorker, UT_FileCleanupWorker_scanPictureByHtml_002)
{
    QString metadata = "<div <img src=\"/test/test/images/test.jpg\"> </div>";

    FileCleanupWorker *work = new FileCleanupWorker(qspAllNotesMap);
    work->scanPictureByHtml(metadata);
    delete work;
}

TEST_F(UT_FileCleanupWorker, UT_FileCleanupWorker_scanVoiceByBlocks_001)
{
    VNOTE_DATAS note;
    VNVoiceBlock *voice = new VNVoiceBlock();
    note.voiceBlocks.push_back(voice);
    FileCleanupWorker *work = new FileCleanupWorker(qspAllNotesMap);
    work->scanVoiceByBlocks(note);
    delete voice;
    delete work;
}
