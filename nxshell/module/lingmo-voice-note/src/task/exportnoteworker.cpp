// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "exportnoteworker.h"
#include "globaldef.h"
#include "common/vnoteitem.h"
#include "common/metadataparser.h"
#include "common/setting.h"
#include "common/utils.h"

#include <DLog>

#include <QDir>
#include <QDateTime>
#include <QFile>
#include <QFileInfo>

/**
 * @brief ExportNoteWorker::ExportNoteWorker
 * @param dirPath 导出目录
 * @param exportType 导出类型
 * @param note 绑定记事项数据
 * @param defaultName 默认名称
 * @param parent
 */
ExportNoteWorker::ExportNoteWorker(const QString &dirPath, ExportType exportType,
                                   const QList<VNoteItem *> &noteList, const QString &defaultName, QObject *parent)
    : VNTask(parent)
    , m_exportType(exportType)
    , m_exportPath(dirPath)
    , m_exportName(defaultName)
    //笔记列表
    , m_noteList(noteList)
{
}

/**
 * @brief ExportNoteWorker::run
 * 执行导出操作
 */
void ExportNoteWorker::run()
{
    ExportError error = checkPath();

    if (ExportOK == error) {
        if (ExportText == m_exportType) {
            error = exportText();
            setting::instance()->setOption(VNOTE_EXPORT_TEXT_PATH_KEY, m_exportPath);
        } else if (ExportVoice == m_exportType) {
            error = exportAllVoice();
            setting::instance()->setOption(VNOTE_EXPORT_VOICE_PATH_KEY, m_exportPath);
        } else if (ExportHtml == m_exportType) {
            error = exportAsHtml();
            setting::instance()->setOption(VNOTE_EXPORT_TEXT_PATH_KEY, m_exportPath);
        }
    } else {
        qCritical() << "Export note error: m_exportType=" << m_exportType;
    }

    emit exportFinished(error);
}

/**
 * @brief ExportNoteWorker::checkPath
 * @return 错误码
 */
ExportNoteWorker::ExportError ExportNoteWorker::checkPath()
{
    ExportError error = ExportOK;

    QFileInfo exportDir(m_exportPath);

    if (!m_exportPath.isEmpty()) {
        if (!exportDir.exists()) {
            if (!QDir().mkpath(m_exportPath)) {
                error = PathDenied;
            }
        } else if (!exportDir.isWritable()) {
            error = PathDenied;
        }
    } else {
        error = PathInvalid;
    }

    return error;
}

/**
 * @brief ExportNoteWorker::exportText
 * @return 错误码
 * 导出文本
 */
ExportNoteWorker::ExportError ExportNoteWorker::exportText()
{
    ExportError error = ExportOK;
    //存在note
    if (m_noteList.size()) {
        for (auto noteData : m_noteList) {
            if (!noteData->haveText()) {
                continue;
            }
            QString filePath = "";
            //没有指定保存名称，则设置默认名称为笔记标题
            if (m_exportName.isEmpty()) {
                QString baseFileName = m_exportPath + "/" + Utils::filteredFileName(noteData->noteTitle, "note");
                QString fileSuffix = ".txt";
                filePath = getExportFileName(baseFileName, fileSuffix);
            } else {
                filePath = m_exportPath + "/" + m_exportName;
            }
            QFile out(filePath);
            if (!out.open(QIODevice::WriteOnly | QIODevice::Text)) {
                return Savefailed; //保存失败
            }
            //富文本数据需要转换为纯文本
            if (!noteData->htmlCode.isEmpty()) {
                QTextDocument doc;
                doc.setHtml(noteData->htmlCode);
                out.write(doc.toPlainText().toUtf8());
            } else {
                for (auto it : noteData->datas.datas) {
                    if (VNoteBlock::Text == it->getType()) {
                        out.write(it->blockText.toUtf8());
                        out.write("\n");
                    }
                }
            }
            out.close();
        }
    } else {
        error = NoteInvalid;
    }
    return error;
}

/**
 * @brief ExportNoteWorker::exportAllVoice
 * @return 错误码
 * 导出语音
 */
ExportNoteWorker::ExportError ExportNoteWorker::exportAllVoice()
{
    ExportError error = ExportOK;
    //存在note
    if (m_noteList.size()) {
        for (auto noteData : m_noteList) {
            if (!noteData->haveVoice()) {
                continue;
            }
            if (noteData->htmlCode.isEmpty()) {
                for (auto it : noteData->datas.datas) {
                    error = exportOneVoice(it);
                    //某一个保存失败则后续的不再进行保存操作
                    if (Savefailed == error) {
                        return error;
                    }
                }
            } else {
                //富文本笔记
                for (auto it : noteData->getVoiceJsons()) {
                    error = exportOneVoice(it);
                    //某一个保存失败则后续的不再进行保存操作
                    if (Savefailed == error) {
                        return error;
                    }
                }
            }
        }
    } else {
        error = NoteInvalid;
    }

    return error;
}

/**
 * @brief ExportNoteWorker::exportOneVoice
 * 导出语音
 * @param json json格式的语音数据
 * @return 错误码
 */
ExportNoteWorker::ExportError ExportNoteWorker::exportOneVoice(const QString &json)
{
    VNVoiceBlock voiceBlock;
    MetaDataParser dataParser;
    //解析json数据
    if (dataParser.parse(json, &voiceBlock)) {
        return exportOneVoice(&voiceBlock);
    }
    return NoteInvalid;
}

/**
 * @brief ExportNoteWorker::exportOneVoice
 * @param noteblock
 * @return 错误码
 */
ExportNoteWorker::ExportError ExportNoteWorker::exportOneVoice(VNoteBlock *noteblock)
{
    ExportError error = ExportOK;

    if (noteblock && noteblock->blockType == VNoteBlock::Voice) {
        QString baseFileName = m_exportPath + "/" + noteblock->ptrVoice->voiceTitle;
        QString fileSuffix = ".mp3";
        QString dstFileName = getExportFileName(baseFileName, fileSuffix);
        if (!QFile::copy(noteblock->ptrVoice->voicePath, dstFileName)) {
            error = Savefailed; //保存失败
        }
    } else {
        error = NoteInvalid;
    }

    return error;
}

/**
 * @brief ExportNoteWorker::exportAsHtml
 * 将笔记导出为Html
 * @return 错误码
 */
ExportNoteWorker::ExportError ExportNoteWorker::exportAsHtml()
{
    ExportError error = ExportOK;
    for (auto note : m_noteList) {
        if (!note->haveText()) {
            continue;
        }
        QString filePath = "";
        //没有指定保存名称，则设置默认名称为笔记标题
        if (m_exportName.isEmpty()) {
            QString baseFileName = m_exportPath + "/" + Utils::filteredFileName(note->noteTitle, "note");
            QString fileSuffix = ".html";
            filePath = getExportFileName(baseFileName, fileSuffix);
        } else {
            filePath = m_exportPath + "/" + m_exportName;
        }
        QFile out(filePath);
        if (!out.open(QIODevice::WriteOnly | QIODevice::Text)) {
            return Savefailed; //保存失败
        }
        if (!out.write(note->getFullHtml().toUtf8())) {
            return Savefailed; //保存失败
        }
    }
    return error;
}

QString ExportNoteWorker::getExportFileName(const QString &baseName, const QString &fileSuffix)
{
    QString filePath = baseName + fileSuffix;
    int count = 1;
    while (1) {
        QFile file(filePath);
        if (!file.exists()) {
            break;
        }
        filePath = baseName + "(" + QString::number(count++) + ")" + fileSuffix;
    }
    return filePath;
}
