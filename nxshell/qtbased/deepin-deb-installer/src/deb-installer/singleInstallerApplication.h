// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SINGLEFONTAPPLICATION_H
#define SINGLEFONTAPPLICATION_H

#include <DApplication>
#include <DMainWindow>
#include <QCommandLineParser>

DWIDGET_USE_NAMESPACE

class SingleInstallerApplication : public DApplication
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.deepin.DebInstaller")
public:
    enum AppWorkChannel { NormalChannel, DdimChannel };

    explicit SingleInstallerApplication(int &argc, char **argv);
    /**
     * @brief 激活软件包安装器窗口
     *
     */
    void activateWindow();

    /**
     * @brief 解析命令行参数
     *
     * @return true
     * @return false
     */
    bool parseCmdLine();

    static AppWorkChannel mode;  // 当前运行的工作模式,用于判断二次启动的时候走哪个通道
    static std::atomic_bool BackendIsRunningInit;

public slots:

    /**
     * @brief InstallerDeb 启动软件包安装器
     *
     * @param debPathList 是否有传递的包
     * @return Q_SCRIPTABLE
     */
    Q_SCRIPTABLE void InstallerDeb(const QStringList &debPathList);
    /**
     * @brief InstallerDebPackge 安装包
     *
     * @param debPath 包路径信息
     * @return 返回安装信息
     */
    Q_SCRIPTABLE QString InstallerDebPackge(const QString &debPath);
    /**
     * @brief unInstallDebPackge 卸载包
     *
     * @param debPath 包路径信息
     * @return 返回卸载信息
     */
    Q_SCRIPTABLE QString unInstallDebPackge(const QString &debPath);
    /**
     * @brief checkInstallStatus 包安装状态
     *
     * @param debPath 包路径信息
     * @return -1: 其他错误
     *          0:当前包没有被安装
     *          1:当前已经安装过相同的版本
     *          2:当前已经安装过较早的版本
     *          3.当前已经安装过更新的版本
     */
    Q_SCRIPTABLE int checkInstallStatus(const QString &debPath);
    /**
     * @brief checkDependsStatus 包依赖信息
     *
     * @param debPath 包路径信息
     * @return -1：其他错误
     *          0：依赖满足
     *          1：依赖可用但是需要下载
     *          2：依赖不满足
     *          3: 签名验证失败
     *          4: 依赖授权失败（wine依赖）
     *          5: 架构不满足（此前架构不满足在前端验证，此后会优化到后端）
     *          6: 应用被域管限制，无法安装
     */
    Q_SCRIPTABLE int checkDependsStatus(const QString &debPath);
    /**
     * @brief checkDigitalSignature 包的数字签名
     *
     * @param debPath 包路径信息
     * @return -1: 其他错误
     *          0: 验证成功
     *          1: 解压deb文件用的临时目录不存在
     *          2: 提取deb包内容时出错
     *          3: deb包验证失败
     */
    Q_SCRIPTABLE int checkDigitalSignature(const QString &debPath);
    /**
     * @brief getPackageInfo 包信息
     *
     * @param debPath 包路径信息
     * @return (包名;包的路径;包的版本;包可用的架构;包的短描述;包的长描述)
     */
    Q_SCRIPTABLE QString getPackageInfo(const QString &debPath);

private:
    QStringList m_selectedFiles;
    QStringList m_ddimFiles;
    QScopedPointer<DMainWindow> m_qspMainWnd;  // MainWindow ptr

    bool bIsDbus = false;
};

#endif  // SINGLEFONTAPPLICATION_H
