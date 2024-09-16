// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_exportnoteworker.h"
#include "task/exportnoteworker.h"
#include "common/vnotedatamanager.h"
#include "common/vnoteitem.h"

#include <QApplication>
#include <QList>

void UT_ExportNoteWorker::SetUp()
{
    VNOTE_ALL_NOTES_MAP *notes = VNoteDataManager::instance()->getAllNotesInFolder();
    if (notes && !notes->notes.isEmpty()) {
        VNOTE_ITEMS_MAP *tmp = notes->notes.first();
        if (tmp && !tmp->folderNotes.isEmpty()) {
            m_noteList.push_back(tmp->folderNotes.first());
            for (auto it : tmp->folderNotes.first()->datas.dataConstRef()) {
                if (it->getType() == VNoteBlock::Voice) {
                    m_block = it;
                    break;
                }
            }
        }
    }
    note = new VNoteItem();
    note->htmlCode = "<div> <p> <p> </div>";
    m_noteList.push_back(note);
}

void UT_ExportNoteWorker::TearDown()
{
    delete note;
}

UT_ExportNoteWorker::UT_ExportNoteWorker()
{
}

TEST_F(UT_ExportNoteWorker, UT_ExportNoteWorker_exportText_001)
{
    ExportNoteWorker work(QCoreApplication::applicationDirPath(), ExportNoteWorker::ExportText, m_noteList);
    work.run();
    EXPECT_EQ(work.m_exportType, ExportNoteWorker::ExportText);
}

TEST_F(UT_ExportNoteWorker, UT_ExportNoteWorker_exportAllVoice_001)
{
    ExportNoteWorker work(QCoreApplication::applicationDirPath(), ExportNoteWorker::ExportVoice, m_noteList);
    work.run();
    EXPECT_EQ(work.m_exportType, ExportNoteWorker::ExportVoice);
}

TEST_F(UT_ExportNoteWorker, UT_ExportNoteWorker_exportOneVoice_002)
{
    QString metadata = "{\"createTime\":\"2021-09-16 17:19:22.065\",\"state\":false,\"text\":\"\",\"title\":\"20210916 17.19.22\",\"transSize\":\"00:01\",\"type\":2,"
                       "\"voicePath\":\"/home/uos/.local/share/deepin/deepin-voice-note/voicenote/0020210916171920.mp3\",\"voiceSize\":1420}";
    ExportNoteWorker work(QCoreApplication::applicationDirPath(), ExportNoteWorker::ExportVoice, m_noteList);
    EXPECT_EQ(ExportNoteWorker::Savefailed, work.exportOneVoice(metadata)) << "has data";

    metadata = "{}";
    EXPECT_EQ(ExportNoteWorker::NoteInvalid, work.exportOneVoice(metadata)) << "not have data";
}

TEST_F(UT_ExportNoteWorker, UT_ExportNoteWorker_checkPath_001)
{
    ExportNoteWorker work(QCoreApplication::applicationDirPath(), ExportNoteWorker::ExportVoice, m_noteList);
    EXPECT_EQ(work.checkPath(), ExportNoteWorker::ExportOK);
}

TEST_F(UT_ExportNoteWorker, UT_ExportNoteWorker_exportAsHtml_001)
{
    ExportNoteWorker work("test", ExportNoteWorker::ExportHtml, m_noteList);
    work.run();
    EXPECT_EQ(ExportNoteWorker::ExportOK, work.exportAsHtml());
    EXPECT_EQ(work.m_exportType, ExportNoteWorker::ExportHtml);
}

TEST_F(UT_ExportNoteWorker, UT_ExportNoteWorker_exportAsHtml_002)
{
    ExportNoteWorker work("/test", ExportNoteWorker::ExportHtml, m_noteList);
    work.run();
    EXPECT_EQ(ExportNoteWorker::Savefailed, work.exportAsHtml());
    EXPECT_EQ(work.m_exportType, ExportNoteWorker::ExportHtml);
}

TEST_F(UT_ExportNoteWorker, UT_ExportNoteWorker_getExportFileName_001)
{
    ExportNoteWorker work("/test", ExportNoteWorker::ExportHtml, m_noteList);
    EXPECT_EQ("123.txt", work.getExportFileName("123", ".txt"));
}
