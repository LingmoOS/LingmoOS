// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DEALDEPENDTHREAD_H
#define DEALDEPENDTHREAD_H

#include <QObject>
#include <QThread>
#include <QProcess>

class DealDependThread : public QThread
{
    Q_OBJECT
public:
    DealDependThread(QObject *parent = nullptr);
    virtual ~DealDependThread();

    /**
     * @brief setDependsList 添加安装列表
     * @param dependList 依赖列表
     * @param index 需要安装的依赖下标
     */
    void setDependsList(const QStringList &dependList, int index);

    /**
     * @brief setBrokenDepend 设置依赖不满足的依赖名称
     * @param dependName    break的依赖名
     */
    void setBrokenDepend(const QString &dependName);

    /**
     * @brief run 线程的运行函数
     */
    void run();

signals:
    /**
     * @brief signalDependResult 依赖下载的过程处理信号
     */
    void signalDependResult(int, int, QString);

    /**
     * @brief enableCloseButton 依赖下载时设置关闭按钮是否可用的信号
     */
    void signalEnableCloseButton(bool);

public slots:
    /**
     * @brief onFinished 依赖下载完成的处理槽函数
     */
    void slotInstallFinished(int);

    /**
     * @brief on_readoutput 依赖下载过程中的输出函数
     */
    void slotReadOutput();

private:
    // 执行下载的进程指针
    QProcess *proc = nullptr;

    // 出现问题依赖的下标
    int m_index = -1;

    // 需要安装的依赖列表
    QStringList m_dependsList = { nullptr };

    // 依赖安装状态是否错误的标识
    bool bDependsStatusErr = false;
    // 分级管控验签失败的标识
    bool bVerifyStatusErr = false;

    // 下载失败的依赖的名称
    QString m_brokenDepend = "";
};

#endif  // DEALDEPENDTHREAD_H
