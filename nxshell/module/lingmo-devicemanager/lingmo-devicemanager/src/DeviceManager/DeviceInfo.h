// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DEVICEINFO_H
#define DEVICEINFO_H

#include "document.h"
#include "xlsxdocument.h"
#include "table.h"
#include "DeviceManager.h"

#include <QString>
#include <QMap>
#include <QSet>
#include <QObject>
#include <QList>
#include <QPair>
#include <QDomDocument>
#include <QFile>

/**
 * @brief The EnableDeviceStatus enum
 * 用来标识对设备启用禁用的结果(包括成功、失败、取消三种情况)
 */
enum EnableDeviceStatus {
    EDS_Cancle,
    EDS_NoSerial,
    EDS_Faild,
    EDS_Success
};

/**
 * @brief The DeviceBaseInfo class
 * 各个摄像头描述类的基类
 */
class DeviceBaseInfo : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(DeviceBaseInfo)
public:
    explicit DeviceBaseInfo(QObject *parent = nullptr);
    virtual ~DeviceBaseInfo();

    /**
     * @brief getOtherAttribs:获取设备的其它信息
     * @return 其他信息组成的map
     */
    //const QMap<QString, QString> &getOtherAttribs()const;
    const QList<QPair<QString, QString>> &getOtherAttribs();

    /**
     * @brief getBaseAttribs::获取基本设备信息
     * @return 基本信息组成的list
     */
    const QList<QPair<QString, QString>> &getBaseAttribs();

    /**
     * @brief getTableHeader : 用于存放表格的头部
     * @return : 用于存放表格的头部
     */
    const QStringList &getTableHeader();

    /**
     * @brief getTableData ：获取表头数据
     * @return 返回表头数据
     */
    const QStringList &getTableData();

    /**
     * @brief subTitle:获取子标题
     * @return 子标题
     */
    virtual QString subTitle();

    /**
     * @brief getOverviewInfo:获取概况信息
     * @return 概况信息
     */
    virtual const QString getOverviewInfo();

    /**
     * @brief name:获取设备名称
     * @return 设备名称
     */
    virtual const QString &name() const = 0;

    /**
     * @brief vendor:获取设备制造商
     * @return 设备制造商
     */
    virtual const QString &vendor() const = 0;

    /**
     * @brief vendor:获取设备VID
     * @return 设备VID
     */

    virtual const QString &getVID() const;
    /**
     * @brief vendor:获取设备PID
     * @return 设备PID
     */
    virtual const QString &getPID() const;

    /**
     * @brief vendor:获取设备VID_PID
     * @return 设备VID_PID
     */
    virtual const QString &getVIDAndPID() const;

    /**
     * @brief vendor:获取设备Modalias
     * @return 设备Modalias
     */
    virtual const QString &getModalias() const;

    /**
     * @brief driver:获取设备驱动
     * @return 设备驱动
     */
    virtual const QString &driver() const = 0;

    /**
     * @brief setEnable 设置设备的禁用状态
     * @param enable 是否禁用
     * @return 禁用是否成功
     */
    virtual EnableDeviceStatus setEnable(bool enable);

    /**
     * @brief enable 获取设备的禁用状态
     * @return
     */
    virtual bool enable();

    /**
     * @brief availble 获取是否可用
     * @return 返回是否可用
     */
    virtual bool available();

    /**
     * @brief driverIsKernelIn 判断驱动是否属于内置驱动
     * @param driver 驱动名称
     * @return
     */
    virtual bool driverIsKernelIn(const QString &driver);

    /**
     * @brief setCanEnale : set can enable or not
     * @param can
     */
    void setCanEnale(bool can);

    /**
     * @brief canEnable : 该设备是否可以禁用
     * @return
     */
    bool canEnable();

    /**
     * @brief setEnableValue
     */
    void setEnableValue(bool e);

    /**
     * @brief canUninstall 获取该设备是否可以卸载驱动(主板等右键不需要安装、卸载action)
     * @return
     */
    bool canUninstall();

    /**
     * @brief setCanUninstall 设置设备是否可以卸载驱动(主板等右键不需要安装、卸载action)
     * @return
     */
    void setCanUninstall(bool can);

    /**
     * @brief setHardwareClass
     * @param hclass
     */
    void setHardwareClass(const QString &hclass);

    /**
     * @brief hardwareClass
     * @return
     */
    const QString &hardwareClass() const;

    /**
     * @brief uniqueID
     * @return
     */
    const QString &uniqueID() const;

    /**
     * @brief sysPath
     * @return
     */
    const QString &sysPath() const;

    /**
     * @brief getVendorOrModelId:获取Vendor 或 Model Id
     * @param sysPath 属性sysFS ID
     * @param flag true:vendor id.false:model id
     * @return
     */
    const QString getVendorOrModelId(const QString &sysPath, bool flag = true);

    /**
     * @brief get_string:读取文件内信息
     * @param path:文件绝对路径+名称
     */
    const  QString get_string(const QString  &sysPathfile);

    /**
     * @brief getDriverVersion: 获取驱动版本
     * @return
     */
    const QString getDriverVersion();

    /**
     * @brief isValid：判断属性值是否有效
     * @param value：属性值
     * @return true:属性值有效
     */
    bool isValueValid(QString &value);

    /**
     * @brief setForcedDisplay：设置强制显示
     * @param flag：是否强制显示
     */
    void setForcedDisplay(const bool &flag);

    /**
     * @brief toHtmlString:导出信息为html格式
     * @param doc xml格式文本
     */
    void toHtmlString(QDomDocument &doc);

    /**
     * @brief baseInfoToHTML:基本信息导出html
     * @param doc:xml格式文本
     * @param infoLst:信息列表
     */
    void baseInfoToHTML(QDomDocument &doc, QList<QPair<QString, QString>> &infoLst);

    /**
     * @brief subTitleToHTML:子标题导出
     * @param doc:xml格式文本
     */
    void subTitleToHTML(QDomDocument &doc);

    /**
     * @brief toDocString:导出信息为html格式
     * @param doc:doc文档
     */
    void toDocString(Docx::Document &doc);

    /**
     * @brief baseInfoToDoc:基本信息导出doc
     * @param doc:doc文档
     * @param infoLst:信息列表
     */
    void baseInfoToDoc(Docx::Document &doc, QList<QPair<QString, QString>> &infoLst);

    /**
     * @brief toXlsxString:导出信息为xlsx格式
     * @param xlsx:xlsx文档
     * @param boldFontc:字体格式
     */
    void toXlsxString(QXlsx::Document &xlsx, QXlsx::Format &boldFont);

    /**
     * @brief baseInfoToXlsx:基本信息导出xlsx
     * @param xlsx:xlsx文档
     * @param boldFont:字体格式
     * @param infoLst:信息列表
     */
    void baseInfoToXlsx(QXlsx::Document &xlsx, QXlsx::Format &boldFont, QList<QPair<QString, QString>> &infoLst);

    /**
     * @brief toTxtString:导出信息为txt格式
     * @param out:文本输出流
     */
    void toTxtString(QTextStream &out);

    /**
     * @brief baseInfoToTxt:导出信息为txt格式
     * @param out:文本输出流
     * @param infoLst:信息列表
     */
    void baseInfoToTxt(QTextStream &out, QList<QPair<QString, QString>> &infoLst);

    /**
     * @brief tableInfoToTxt:表格内容写到txt
     * @param out:txt文件流
     */
    void tableInfoToTxt(QTextStream &out);

    /**
     * @brief tableHeaderToTxt:表头信息写到txt
     * @param out:txt文件流
     */
    void tableHeaderToTxt(QTextStream &out);

    /**
     * @brief tableInfoToHtml:表格内容写到html
     * @param html:html文档
     */
    void tableInfoToHtml(QFile &html);

    /**
     * @brief tableHeaderToHtml:表头信息写到html
     * @param html:html文档
     */
    void tableHeaderToHtml(QFile &html);

    /**
     * @brief tableInfoToDoc:表格信息写到doc
     * @param tab:doc文档表格
     * @param row:表格行数，第row行
     */
    void tableInfoToDoc(Docx::Table *tab, int &row);

    /**
     * @brief tableHeaderToDoc:表头信息写到doc
     * @param tab doc文档的表格
     */
    void tableHeaderToDoc(Docx::Table *tab);

    /**
     * @brief tableInfoToXlsx:表格信息写到xlsx
     * @param xlsx xlsx文件
     */
    void tableInfoToXlsx(QXlsx::Document &xlsx);

    /**
     * @brief tableHeaderToXlsx:表头信息写到xlsx
     * @param xlsx xlsx文件
     */
    void tableHeaderToXlsx(QXlsx::Document &xlsx);

    /**
     * @brief setOtherDeviceInfo:设置其他信息信息
     * @param key:属性名称
     * @param value:属性值
     */
    void setOtherDeviceInfo(const QString &key, const QString &value);

    /**
       * @brief setInfoFromTomlOneByOne:设置从toml里面获取的信息
       * @param mapInfo:由toml获取的信息map
       * @return枚举值
       */
    TomlFixMethod setInfoFromTomlBase(const QMap<QString, QString> &mapInfo);

protected:
    /**
     * @brief:初始化过滤信息
    */
    virtual void initFilterKey() = 0;

    /**
     * @brief loadBaseDeviceInfo:加载基本信息
     */
    virtual void loadBaseDeviceInfo() = 0;
    /**
     * @brief loadOtherDeviceInfo:获取其它设备信息
     */
    virtual void loadOtherDeviceInfo() = 0;

    /**
     * @brief loadTableHeader : 过去表格的表头数据
     */
    virtual void loadTableHeader();

    /**
     * @brief loadTableData : 获取表格数据信息
     */
    virtual void loadTableData() = 0;

    /**
     * @brief addFilterKey:添加过滤信息
     * @param key:过滤信息的字符串
     */
    virtual void addFilterKey(const QString &key);

    /**
     * @brief loadOtherDeviceInfo:加载其它设备信息
     * @param mapInfo:其他设备信息map
     */
    void getOtherMapInfo(const QMap<QString, QString> &mapInfo);

    /**
     * @brief addBaseDeviceInfo:添加基本信息
     * @param key:属性名称
     * @param value:属性值
     */
    void addBaseDeviceInfo(const QString &key, const QString &value);

    /**
     * @brief addOtherDeviceInfo:添加其他信息信息
     * @param key:属性名称
     * @param value:属性值
     */
    void addOtherDeviceInfo(const QString &key, const QString &value);

    /**@brief:将属性设置到成员变量*/
    /**
     * @brief setAttribute:将属性设置到成员变量
     * @param mapInfo:设备信息map
     * @param key:属性名称
     * @param variable:设备类成员变量
     * @param overwrite:是否覆盖
     */
    void setAttribute(const QMap<QString, QString> &mapInfo, const QString &key, QString &variable, bool overwrite = true);


    /**@brief:将属性设置到成员变量*/
    /**
     * @brief setTomlAttribute:将属性设置到成员变量
     * @param mapInfo:设备信息map
     * @param key:属性名称
     * @param variable:设备类成员变量
     * @param overwrite:是否覆盖
     */
    TomlFixMethod setTomlAttribute(const QMap<QString, QString> &mapInfo, const QString &key, QString &variable, bool overwrite = true);


    /**
     * @brief mapInfoToList:QMap转换为QList
     */
    void mapInfoToList();

    /**
     * @brief setHwinfoLshwKey
     * @param mapInfo
     */
    void setHwinfoLshwKey(const QMap<QString, QString> &mapInfo);

    /**
     * @brief matchToLshw
     * @param mapInfo
     * @return
     */
    bool matchToLshw(const QMap<QString, QString> &mapInfo);

    /**
     * @brief setPhysIDMapKey
     * @param mapInfo
     * @return
     */
    void setPhysIDMapKey(const QMap<QString, QString> &mapInfo);

    /**
     * @brief PhysIDMapInfo  PID VID
     * @param mapInfo
     * @return
     */
    bool PhysIDMapInfo(const QMap<QString, QString> &mapInfo);

protected:
    QString            m_Name;         //<! 【名称】
    QString            m_Vendor;       //<! 【制造商
    QString            m_PhysID;       //<! 【物理ID】
    QString            m_VID_PID;      //<! 设备hw的唯一值 m_VID_PID = m_VID + m_PID.remove("0x")
    QString            m_VID;          //<! 设备hw的唯一值
    QString            m_PID;          //<! 设备hw的唯一值
    QString            m_Modalias;     //<! 设备hw的唯一值
    QString            m_Version;      //<! 【版本】
    QString            m_Description;  //<! 【描述】
    //----------以上为每个设备必有属性--------
    QString                        m_UniqueID;      //<! 设备的唯一值
    QString                        m_SerialID;      //<! 序列号id
    QString                        m_SysPath;       //<! 用启用的sys path
    QString                        m_HardwareClass; //<! 设备类型
    QString                        m_HwinfoToLshw;  //<! 匹配hwinfo和lshw的key
    QList<QPair<QString, QString>> m_LstBaseInfo;   //<! 基本信息
    QList<QPair<QString, QString>> m_LstOtherInfo;  //<! 其它信息
    QStringList                    m_TableHeader;   //<! 用于存放表格的表头
    QStringList                    m_TableData;     //<! 用于存放表格的内容
    QSet<QString>                  m_FilterKey;     //<! 用于避免添加重复信息
    bool                           m_Enable;        //<! 设备是否是启用状态
    bool                           m_CanEnable;     //<! 设备是否可以启用禁用
    bool                           m_CanUninstall;  //<! 是否可以卸载驱动
    bool                           m_Available;     //<! 是否可用
    bool                           m_forcedDisplay; //<! 强制显示
    int                            m_Index;         //<! 同名设备的索引
    QString                        m_PhysIDMap;         //<! 匹配 XXX和XXX的key
    QString            m_Driver;                  //<! 【驱动】

private:
    QMap<QString, QString>  m_MapOtherInfo;         //<! 其它信息
};
#endif // DEVICEINFO_H
