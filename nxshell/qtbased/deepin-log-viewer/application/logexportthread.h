// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef LOGEXPORTTHREAD_H
#define LOGEXPORTTHREAD_H
#include "structdef.h"

#include <QRunnable>
#include <QObject>
#include <QStandardItemModel>

/**
 * @brief The LogExportThread class 导出日志线程类
 */
class LogExportThread : public QObject, public QRunnable
{
    Q_OBJECT
public:
    explicit LogExportThread(QObject *parent = nullptr);
    ~LogExportThread();
    /**
     * @brief The RUN_MODE enum 导出类型，用于执行时判断要执行的逻辑
     */
    enum RUN_MODE {
        HtmlModel, //数据来源是model,导出格式为Html
        HtmlJOURNAL, //数据来源是系统日志数据结构体的list,导出格式为Html
        HtmlAPP, //数据来源是应用日志数据结构体的list,导出格式为Html
        HtmlDPKG, //数据来源是dpkg日志数据结构体的list,导出格式为Html
        HtmlBOOT, //数据来源是启动日志数据结构体的list,导出格式为Html
        HtmlXORG, //数据来源是xorg日志数据结构体的list,导出格式为Html
        HtmlNORMAL, //数据来源是开关机日志数据结构体的list,导出格式为Html
        HtmlKWIN, //数据来源是kwin日志数据结构体的list,导出格式为Html
        HtmlDNF,
        HtmlDMESG,
        HtmlAUDIT, //数据来源是audit日志数据结构体的list,导出格式为html
        TxtModel, //数据来源是model,导出格式为txt
        TxtJOURNAL, //数据来源是系统日志数据结构体的list,导出格式为txt
        TxtAPP, //数据来源是应用日志数据结构体的list,导出格式为txt
        TxtDPKG, //数据来源是dpkg日志数据结构体的list,导出格式为txt
        TxtBOOT, //数据来源是启动日志数据结构体的list,导出格式为txt
        TxtXORG, //数据来源是xorg日志数据结构体的list,导出格式为txt
        TxtNORMAL, //数据来源是开关机日志数据结构体的list,导出格式为txt
        TxtKWIN, //数据来源是kwin日志数据结构体的list,导出格式为txt
        TxtDNF,
        TxtDMESG,
        TxtAUDIT, //数据来源是audit日志数据结构体的list,导出格式为txt
        DocModel, //数据来源是model,导出格式为doc
        DocJOURNAL, //数据来源是系统日志数据结构体的list,导出格式为doc
        DocAPP, //数据来源是应用日志数据结构体的list,导出格式为doc
        DocDPKG, //数据来源是dpkg日志数据结构体的list,导出格式为doc
        DocBOOT, //数据来源是启动日志数据结构体的list,导出格式为doc
        DocXORG, //数据来源是xorg日志数据结构体的list,导出格式为doc
        DocNORMAL, //数据来源是开关机日志数据结构体的list,导出格式为doc
        DocKWIN, //数据来源是kwin日志数据结构体的list,导出格式为doc
        DocDNF,
        DocDMESG,
        DocAUDIT, //数据来源是audit日志数据结构体的list,导出格式为doc
        XlsModel, //数据来源是model,导出格式为xlxs
        XlsJOURNAL, //数据来源是系统日志数据结构体的list,导出格式为xlxs
        XlsAPP, //数据来源是应用日志数据结构体的list,导出格式为xlxs
        XlsDPKG, //数据来源是dpkg日志数据结构体的list,导出格式为xlxs
        XlsBOOT, //数据来源是启动日志数据结构体的list,导出格式为xlxs
        XlsXORG, //数据来源是xorg日志数据结构体的list,导出格式为xlxs
        XlsNORMAL, //数据来源是开关机日志数据结构体的list,导出格式为xlxs
        XlsKWIN, //数据来源是kwin日志数据结构体的list,导出格式为xlxs
        XlsDNF,
        XlsDMESG,
        XlsAUDIT, //数据来源是audit日志数据结构体的list,导出格式为xlxs
        ZipCoredump,
        NoneExportType = 9999 //任何行为
    };

    void enableAppendExport(bool bEnable = true);

    void exportToTxtPublic(const QString &fileName, QStandardItemModel *pModel, LOG_FLAG flag);
    void exportToTxtPublic(const QString &fileName, const QList<QString> &jList,  const QStringList &labels, LOG_FLAG flag);
    void exportToTxtPublic(const QString &fileName, const QList<LOG_MSG_JOURNAL> &jList,  const QStringList &labels, LOG_FLAG flag);
    void exportToTxtPublic(const QString &fileName, const QList<LOG_MSG_APPLICATOIN> &jList, const QStringList &labels, const QString &iAppName);
    void exportToTxtPublic(const QString &fileName, const QList<LOG_MSG_DPKG> &jList, const QStringList &labels);
    void exportToTxtPublic(const QString &fileName, const QList<LOG_MSG_BOOT> &jList, const QStringList &labels);
    void exportToTxtPublic(const QString &fileName, const QList<LOG_MSG_XORG> &jList, const QStringList &labels);
    void exportToTxtPublic(const QString &fileName, const QList<LOG_MSG_NORMAL> &jList, const QStringList &labels);
    void exportToTxtPublic(const QString &fileName, const QList<LOG_MSG_KWIN> &jList, const QStringList &labels);
    void exportToTxtPublic(const QString &fileName, const QList<LOG_MSG_DNF> &jList, const QStringList &labels);
    void exportToTxtPublic(const QString &fileName, const QList<LOG_MSG_DMESG> &jList, const QStringList &labels);
    void exportToTxtPublic(const QString &fileName, const QList<LOG_MSG_AUDIT> &jList, const QStringList &labels);

    void exportToHtmlPublic(const QString &fileName, QStandardItemModel *pModel, LOG_FLAG flag);
    void exportToHtmlPublic(const QString &fileName, const QList<QString> &jList,  const QStringList &labels, LOG_FLAG flag);
    void exportToHtmlPublic(const QString &fileName, const QList<LOG_MSG_JOURNAL> &jList,  const QStringList &labels, LOG_FLAG flag);
    void exportToHtmlPublic(const QString &fileName, const QList<LOG_MSG_APPLICATOIN> &jList, const QStringList &labels, const QString &iAppName);
    void exportToHtmlPublic(const QString &fileName, const QList<LOG_MSG_DPKG> &jList, const QStringList &labels);
    void exportToHtmlPublic(const QString &fileName, const QList<LOG_MSG_BOOT> &jList, const QStringList &labels);
    void exportToHtmlPublic(const QString &fileName, const QList<LOG_MSG_XORG> &jList, const QStringList &labels);
    void exportToHtmlPublic(const QString &fileName, const QList<LOG_MSG_NORMAL> &jList, const QStringList &labels);
    void exportToHtmlPublic(const QString &fileName, const QList<LOG_MSG_KWIN> &jList, const QStringList &labels);
    void exportToHtmlPublic(const QString &fileName, const QList<LOG_MSG_DNF> &jList, const QStringList &labels);
    void exportToHtmlPublic(const QString &fileName, const QList<LOG_MSG_DMESG> &jList, const QStringList &labels);
    void exportToHtmlPublic(const QString &fileName, const QList<LOG_MSG_AUDIT> &jList, const QStringList &labels);

    void exportToDocPublic(const QString &fileName, QStandardItemModel *pModel, LOG_FLAG flag);
    void exportToDocPublic(const QString &fileName, const QList<QString> &jList, const QStringList &labels, LOG_FLAG iFlag);
    void exportToDocPublic(const QString &fileName, const QList<LOG_MSG_JOURNAL> &jList, const QStringList &labels, LOG_FLAG iFlag);
    void exportToDocPublic(const QString &fileName, const QList<LOG_MSG_APPLICATOIN> &jList, const QStringList &labels, const QString &iAppName);
    void exportToDocPublic(const QString &fileName, const QList<LOG_MSG_DPKG> &jList, const QStringList &labels);
    void exportToDocPublic(const QString &fileName, const QList<LOG_MSG_BOOT> &jList, const QStringList &labels);
    void exportToDocPublic(const QString &fileName, const QList<LOG_MSG_XORG> &jList, const QStringList &labels);
    void exportToDocPublic(const QString &fileName, const QList<LOG_MSG_NORMAL> &jList, const QStringList &labels);
    void exportToDocPublic(const QString &fileName, const QList<LOG_MSG_KWIN> &jList, const QStringList &labels);
    void exportToDocPublic(const QString &fileName, const QList<LOG_MSG_DNF> &jList, const QStringList &labels);
    void exportToDocPublic(const QString &fileName, const QList<LOG_MSG_DMESG> &jList, const QStringList &labels);
    void exportToDocPublic(const QString &fileName, const QList<LOG_MSG_AUDIT> &jList, const QStringList &labels);

    void exportToXlsPublic(const QString &fileName, QStandardItemModel *pModel, LOG_FLAG flag);
    void exportToXlsPublic(const QString &fileName, const QList<QString> &jList, const QStringList &labels, LOG_FLAG iFlag);
    void exportToXlsPublic(const QString &fileName, const QList<LOG_MSG_JOURNAL> &jList, const QStringList &labels, LOG_FLAG iFlag);
    void exportToXlsPublic(const QString &fileName, const QList<LOG_MSG_APPLICATOIN> &jList, const QStringList &labels, const QString &iAppName);
    void exportToXlsPublic(const QString &fileName, const QList<LOG_MSG_DPKG> &jList, const QStringList &labels);
    void exportToXlsPublic(const QString &fileName, const QList<LOG_MSG_BOOT> &jList, const QStringList &labels);
    void exportToXlsPublic(const QString &fileName, const QList<LOG_MSG_XORG> &jList, const QStringList &labels);
    void exportToXlsPublic(const QString &fileName, const QList<LOG_MSG_NORMAL> &jList, const QStringList &labels);
    void exportToXlsPublic(const QString &fileName, const QList<LOG_MSG_KWIN> &jList, const QStringList &labels);
    void exportToXlsPublic(const QString &fileName, const QList<LOG_MSG_DNF> &jList, const QStringList &labels);
    void exportToXlsPublic(const QString &fileName, const QList<LOG_MSG_DMESG> &jList, const QStringList &labels);
    void exportToXlsPublic(const QString &fileName, const QList<LOG_MSG_AUDIT> &jList, const QStringList &labels);

    void exportToZipPublic(const QString &fileName, const QList<LOG_MSG_COREDUMP> &jList, const QStringList &labels);

    bool isProcessing();
public slots:
    // 停止线程
    void stopImmediately();
public:
    //是否可以运行获取逻辑，用来停止线程
    bool m_canRunning = false;
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
    bool exportToTxt(const QString &fileName, QStandardItemModel *pModel, LOG_FLAG flag);
    bool exportToTxt(const QString &fileName, const QList<QString> &jList,  const QStringList &labels, LOG_FLAG flag);
    bool exportToTxt(const QString &fileName, const QList<LOG_MSG_JOURNAL> &jList,  const QStringList &labels, LOG_FLAG flag);
    bool exportToTxt(const QString &fileName, const QList<LOG_MSG_APPLICATOIN> &jList, const QStringList &labels, const QString &iAppName);
    bool exportToTxt(const QString &fileName, const QList<LOG_MSG_DPKG> &jList, const QStringList &labels);
    bool exportToTxt(const QString &fileName, const QList<LOG_MSG_BOOT> &jList, const QStringList &labels);
    bool exportToTxt(const QString &fileName, const QList<LOG_MSG_XORG> &jList, const QStringList &labels);
    bool exportToTxt(const QString &fileName, const QList<LOG_MSG_NORMAL> &jList, const QStringList &labels);
    bool exportToTxt(const QString &fileName, const QList<LOG_MSG_KWIN> &jList, const QStringList &labels);
    bool exportToTxt(const QString &fileName, const QList<LOG_MSG_DNF> &jList, const QStringList &labels);
    bool exportToTxt(const QString &fileName, const QList<LOG_MSG_DMESG> &jList, const QStringList &labels);
    bool exportToTxt(const QString &fileName, const QList<LOG_MSG_AUDIT> &jList, const QStringList &labels);

    bool exportToDoc(const QString &fileName, const QList<QString> &jList, const QStringList &labels, LOG_FLAG iFlag);
    bool exportToDoc(const QString &fileName, const QList<LOG_MSG_JOURNAL> &jList, const QStringList &labels, LOG_FLAG iFlag);
    bool exportToDoc(const QString &fileName, const QList<LOG_MSG_APPLICATOIN> &jList, const QStringList &labels, QString &iAppName);
    bool exportToDoc(const QString &fileName, const QList<LOG_MSG_DPKG> &jList, const QStringList &labels);
    bool exportToDoc(const QString &fileName, const QList<LOG_MSG_BOOT> &jList, const QStringList &labels);
    bool exportToDoc(const QString &fileName, const QList<LOG_MSG_XORG> &jList, const QStringList &labels);
    bool exportToDoc(const QString &fileName, const QList<LOG_MSG_NORMAL> &jList, const QStringList &labels);
    bool exportToDoc(const QString &fileName, const QList<LOG_MSG_KWIN> &jList, const QStringList &labels);
    bool exportToDoc(const QString &fileName, const QList<LOG_MSG_DNF> &jList, const QStringList &labels);
    bool exportToDoc(const QString &fileName, const QList<LOG_MSG_DMESG> &jList, const QStringList &labels);
    bool exportToDoc(const QString &fileName, const QList<LOG_MSG_AUDIT> &jList, const QStringList &labels);

    bool exportToHtml(const QString &fileName, QStandardItemModel *pModel, LOG_FLAG flag);
    bool exportToHtml(const QString &fileName, const QList<QString> &jList,  const QStringList &labels, LOG_FLAG flag);
    bool exportToHtml(const QString &fileName, const QList<LOG_MSG_JOURNAL> &jList,  const QStringList &labels, LOG_FLAG flag);
    bool exportToHtml(const QString &fileName, const QList<LOG_MSG_APPLICATOIN> &jList, const QStringList &labels, QString &iAppName);
    bool exportToHtml(const QString &fileName, const QList<LOG_MSG_DPKG> &jList, const QStringList &labels);
    bool exportToHtml(const QString &fileName, const QList<LOG_MSG_BOOT> &jList, const QStringList &labels);
    bool exportToHtml(const QString &fileName, const QList<LOG_MSG_XORG> &jList, const QStringList &labels);
    bool exportToHtml(const QString &fileName, const QList<LOG_MSG_NORMAL> &jList, const QStringList &labels);
    bool exportToHtml(const QString &fileName, const QList<LOG_MSG_KWIN> &jList, const QStringList &labels);
    bool exportToHtml(const QString &fileName, const QList<LOG_MSG_DNF> &jList, const QStringList &labels);
    bool exportToHtml(const QString &fileName, const QList<LOG_MSG_DMESG> &jList, const QStringList &labels);
    bool exportToHtml(const QString &fileName, const QList<LOG_MSG_AUDIT> &jList, const QStringList &labels);

    bool exportToXls(const QString &fileName, const QList<QString> &jList, const QStringList &labels, LOG_FLAG iFlag);
    bool exportToXls(const QString &fileName, const QList<LOG_MSG_JOURNAL> &jList, const QStringList &labels, LOG_FLAG iFlag);
    bool exportToXls(const QString &fileName, const QList<LOG_MSG_APPLICATOIN> &jList, const QStringList &labels, QString &iAppName);
    bool exportToXls(const QString &fileName, const QList<LOG_MSG_DPKG> &jList, const QStringList &labels);
    bool exportToXls(const QString &fileName, const QList<LOG_MSG_BOOT> &jList, const QStringList &labels);
    bool exportToXls(const QString &fileName, const QList<LOG_MSG_XORG> &jList, const QStringList &labels);
    bool exportToXls(const QString &fileName, const QList<LOG_MSG_NORMAL> &jList, const QStringList &labels);
    bool exportToXls(const QString &fileName, const QList<LOG_MSG_KWIN> &jList, const QStringList &labels);
    bool exportToXls(const QString &fileName, const QList<LOG_MSG_DNF> &jList, const QStringList &labels);
    bool exportToXls(const QString &fileName, const QList<LOG_MSG_DMESG> &jList, const QStringList &labels);
    bool exportToXls(const QString &fileName, const QList<LOG_MSG_AUDIT> &jList, const QStringList &labels);

    bool exportToZip(const QString &fileName, const QList<LOG_MSG_COREDUMP> &jList);

    void initMap();
    QString strTranslate(const QString &iLevelStr);

    /**
     * @brief htmlEscapeCovert html转义字符转换
     * @param htmlMsg html信息
     */
    void htmlEscapeCovert(QString &htmlMsg);

private:
    //导出文件路径
    QString m_fileName = "";
    //model数据源
    QStandardItemModel *m_pModel = nullptr;
    //导出日志类型
    LOG_FLAG m_flag = NONE;
    //如果导出项文本标题
    QStringList m_labels;
    // 通用json格式日志数据
    QList<QString> m_logDataList;

    //系统日志数据源
    QList<LOG_MSG_JOURNAL> m_jList;
    //应用日志数据源
    QList<LOG_MSG_APPLICATOIN> m_appList;
    //dpkg日志数据源
    QList<LOG_MSG_DPKG> m_dpkgList;
    //启动日志数据源
    QList<LOG_MSG_BOOT> m_bootList;
    //xorg日志数据源
    QList<LOG_MSG_XORG> m_xorgList;
    //开关机日志数据源
    QList<LOG_MSG_NORMAL> m_normalList;
    //kwin日志数据源
    QList<LOG_MSG_KWIN> m_kwinList;
    QList<LOG_MSG_DNF> m_dnfList;
    QList<LOG_MSG_DMESG> m_dmesgList;
    QList<LOG_MSG_AUDIT> m_alist;
    QList<LOG_MSG_COREDUMP> m_coredumplist;
    //当前线程执行的逻辑种类
    RUN_MODE m_runMode = NoneExportType;
    //打开文件错误描述
    QString openErroStr = "export open file error";
    //停止导出描述信息
    QString stopStr = "stop export";
    //应用日志导出的应用名称
    QString m_appName = "";
    //日志等级-显示文本键值对
    QMap<QString, QString> m_levelStrMap;
    bool m_appendExport {false};
};

#endif  // LOGEXPORTTHREAD_H
