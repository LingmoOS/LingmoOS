// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef EXPORTNOTEWORKER_H
#define EXPORTNOTEWORKER_H

#include "common/datatypedef.h"
#include "vntask.h"

#include <QObject>
#include <QRunnable>

//一个记事项文本，语音导出线程
class ExportNoteWorker : public VNTask
{
    Q_OBJECT
public:
    enum ExportType {
        ExportNothing,
        ExportText,
        ExportVoice,
        ExportHtml, //导出为html
    };

    enum ExportError {
        ExportOK,
        NoteInvalid,
        PathDenied,
        PathInvalid,
        Savefailed, //保存失败
    };

    explicit ExportNoteWorker(const QString &dirPath,
                              ExportType exportType,
                              const QList<VNoteItem *> &noteList,
                              const QString &defaultName = "",
                              QObject *parent = nullptr);

signals:
    //导出完成信号
    void exportFinished(int state);
public slots:
protected:
    virtual void run() override;
    //检查路径
    ExportError checkPath();
    //导出文本
    ExportError exportText();
    //导出所有语音
    ExportError exportAllVoice();
    //导出语音
    ExportError exportOneVoice(VNoteBlock *block);
    ExportError exportOneVoice(const QString &);
    //导出为HTML
    ExportError exportAsHtml();
    //获取导出文件名，同名时创建副本
    QString getExportFileName(const QString &baseName, const QString &fileSuffix);

    ExportType m_exportType {ExportNothing};
    QString m_exportPath {""};
    //默认导出名称
    QString m_exportName {""};
    QList<VNoteItem *> m_noteList {nullptr};
};

#endif // EXPORTNOTEWORKER_H
