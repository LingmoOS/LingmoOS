// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef GRAPHICSDRIVERINTERFACE_H_
#define GRAPHICSDRIVERINTERFACE_H_

#include <QObject>
#include <QStringList>
#include "graphicsdeviceinfo.h"
#include "resolutions.h"
#include <QProcess>
#include <QDebug>
#include <QDir>
#include <functional>

inline const QString scriptAbsolutePath(const QString &scriptName)
{
#ifdef QT_DEBUG
    return QDir::currentPath() + "/server/scripts/" + scriptName;
#else
    return "/usr/lib/deepin-graphics-driver-manager/" + scriptName;
#endif
}

// #ifdef QT_DEBUG
#define QPROCESS_DUMP(Process) \
    connect(Process, &QProcess::readyReadStandardOutput, Process, [=] { qDebug().noquote() << proc->readAllStandardOutput(); }); \
    connect(Process, &QProcess::readyReadStandardError, Process, [=] { qWarning().noquote() << proc->readAllStandardError(); });
// #else
// #define QPROCESS_DUMP(Process) Q_UNUSED(Process)
// #endif

#define QPROCESS_DELETE_SELF(Process) \
    connect(Process, static_cast<void (QProcess::*)(int)>(&QProcess::finished), Process, &QProcess::deleteLater);

#define EXECUTE_SCRIPT(Process, Script) \
    Process->start("/bin/bash", QStringList() << "-x" << scriptAbsolutePath(Script));

#define EXECUTE_SCRIPT_ROOT(Process, Script) \
    Process->start(QStringList() << "/bin/bash" << "-x" << scriptAbsolutePath(Script));


class GraphicsDriverInterface : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.deepin.daemon.GraphicsDriver")
public:
    explicit GraphicsDriverInterface(QObject* parent  = nullptr);
    virtual ~GraphicsDriverInterface();
public slots:
    /*************************************************
    Function:       GetDevice
    Description:    获取系统上所有显卡设备信息
    Input:          无
    Output:         无
    Return:         显卡设备信息列表(json格式)
    Others:         无
    *************************************************/
    QString GetDevice();


    /*************************************************
    Function:       GetResolutionTitle
    Description:    获取所有可选方案信息
    Input:          无
    Output:         无
    Return:         可选方案信息列表(json 格式)
    Others:         无
    *************************************************/
    QString GetResolutionTitle();

    /*************************************************
    Function:       PrepareInstall
    Description:    完成切换前下载安装依赖包等准备工作
    Input:          name:方案名称; language:用户语言类型
    Output:         无
    Return:         空
    Others:         无
    *************************************************/
    void PrepareInstall(QString name, QString language);

    /*************************************************
    Function:       CancelInstall
    Description:    实现取消切换动作
    Input:          无
    Output:         无
    Return:         空
    Others:         无
    *************************************************/
    void CancelInstall();

    /*************************************************
    Function:       TestSuccess
    Description:    设置测试结果
    Input:          无
    Output:         无
    Return:         空
    Others:         无
    *************************************************/
    void TestSuccess();

    /*************************************************
    Function:       IsTestSuccess
    Description:    查询测试结果
    Input:          无
    Output:         无
    Return:         空
    Others:         无
    *************************************************/
    bool IsTestSuccess();
    
    /*************************************************
    Function:       RealInstall
    Description:    非overlay环境上切换驱动
    Input:          无
    Output:         无
    Return:         方案名称
    Others:         无
    *************************************************/
    void RealInstall();

    /*************************************************
    Function:       TestInstall
    Description:    overlay环境上切换驱动
    Input:          无
    Output:         无
    Return:         方案名称
    Others:         无
    *************************************************/
    void TestInstall();

    /*************************************************
    Function:       GetOldDriverName
    Description:    获取老方案名称
    Input:          无
    Output:         无
    Return:         方案名称
    Others:         无
    *************************************************/
    QString GetOldDriverName();

    /*************************************************
    Function:       GetNewDriverName
    Description:    获取新方案名称
    Input:          无
    Output:         无
    Return:         空
    Others:         无
    *************************************************/
    QString GetNewDriverName();

    /*************************************************
    Function:       GetCurrDriverName
    Description:    获取当前方案名称
    Input:          无
    Output:         无
    Return:         空
    Others:         无
    *************************************************/
    QString GetCurrDriverName();


private:
    QString GetDriverName(QString type);
    QString GetKernelVersion();
    QString GetCurrPackageVersion(QString pkg_name);
    QString GetDepoPackageVersion(QString pkg_name);
    void Install(QString script);
    bool command(const QString &cmd, const QStringList &args, QString &output);
    bool isInOverlayRoot();

Q_SIGNALS:
    void ReportProgress(QString ratio);
    void Cancel();
private:
    GraphicsDeviceInfo m_devInfo;
    Resolutions m_resolutions;
};

#endif
