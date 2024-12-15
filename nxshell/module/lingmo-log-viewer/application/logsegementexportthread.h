// SPDX-FileCopyrightText: 2019 - 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef LOGSEGEMENTEXPORTTHREAD_H
#define LOGSEGEMENTEXPORTTHREAD_H
#include "structdef.h"
#include "WordProcessingMerger.h"
#include "workbook.h"

#include <QRunnable>
#include <QObject>
#include <QMutex>
#include <QWaitCondition>

/**
 * @brief The LogSegementExportThread class 导出日志线程类
 */
class LogSegementExportThread : public QObject, public QRunnable
{
    Q_OBJECT
public:
    explicit LogSegementExportThread(QObject *parent = nullptr);
    ~LogSegementExportThread() override;

    /**
     * @brief The RUN_MODE enum 导出类型，用于执行时判断要执行的逻辑
     */
    enum RUN_MODE {
        UnKnown = -1,
        Txt,
        Html,
        Doc,
        Xls
    };

    // 设置导出数据
    void setParameter(const QString &fileName, const QList<QString> &jList, const QStringList& lables, LOG_FLAG flag);
    // 设置是否追加写入QFile
    void enableAppendWrite(const bool &bEnable = true) { m_bAppendWrite = bEnable; }
    // 设置进度条总值
    void setTotalProcess(const int &total) { m_nTotalProcess = total; }

    // 判断是否正在运行
    bool isProcessing();

public slots:
    // 正常停止，保存数据到文件
    void stop();
    // 强制停止线程，发生在点击进度条取消按钮，或关闭进度条对话框
    void stopImmediately();

protected:
    void run() override;

signals:
    /**
     * @brief sigProgress 导出进度信号
     * @param nCur 当前运行到的进度
     * @param nTotal 要运行的总数
     */
    void sigProgress(int nCur, int nTotal);
    /**
     * @brief sigResult 导出完成信号
     * @param isSuccess 是否成功
     */
    void sigResult(bool isSuccess);
    void sigProcessFull();
    /**
     * @brief sigError 导出失败
     * @param iError 失败信息
     */
    void sigError(QString iError);
private:
    void initDoc();
    QString getDocTemplatePath();
    void initXls();

    bool exportTxt();
    bool exportHtml();
    bool exportToDoc();
    bool exportToXls();

    void htmlEscapeCovert(QString &htmlMsg);

    void saveDoc();
    void closeXls();

private:
    //导出文件路径
    QString m_fileName = "";

    // QFile追加写入标记
    bool m_bAppendWrite = false;

    //导出日志类型
    LOG_FLAG m_flag = NONE;
    //如果导出项文本标题
    QStringList m_labels;
    // 通用json格式日志数据
    QList<QString> m_logDataList;
    //当前线程执行的逻辑种类
    RUN_MODE m_runMode = UnKnown;

    DocxFactory::WordProcessingMerger *m_pDocMerger { nullptr };
    lxw_workbook  *m_pWorkbook { nullptr };
    lxw_worksheet *m_pWorksheet { nullptr };
    qint64 m_currentXlsRow { 0 };
    
    //打开文件错误描述
    QString m_openErroStr = "export open file error";
    //强制停止导出描述信息
    QString m_forceStopStr = "force stop segement export";

    QMutex mutex;
    QWaitCondition condition;
    bool m_bStop { false };
    // 用来强制停止线程
    bool m_bForceStop { false };

    int m_nCurProcess { 0 };
    int m_nTotalProcess { 0 };
};

#endif  // LOGSEGEMENTEXPORTTHREAD_H
