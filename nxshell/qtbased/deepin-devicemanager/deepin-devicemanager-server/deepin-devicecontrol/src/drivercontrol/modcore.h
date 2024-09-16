// SPDX-FileCopyrightText: 2019 ~ 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DISABLE_DRIVER

#ifndef MODCORE_H
#define MODCORE_H

#include <QObject>
#include <QStringList>

#include <libkmod.h>

class ModCore: public QObject
{
    Q_OBJECT
public:
    //模块信息相关类型
    enum ModInfoType {
        EAlias = 0,
        ELiense,
        EVersion,
        EAuther,
        EDescription,
        ESrcVersion,
        EName,
        EVermagic
    };

    //conf type
    enum ModConfType {
        EBlackListConf = 0,
        EAliasConf,
        EOptionsConf,
        ESoftdepConf
    };

public:
    explicit ModCore(QObject *parent = nullptr);
    //获取依赖当前模块在使用的模块
    QStringList checkModuleInUsed(const QString &modName);
    //强制移除驱动模块 等效于rmmod -f
    bool rmModForce(const QString &modName, QString &errMsg);
    //ko安装
    bool modInstall(const QString &modName, QString &errMsg, unsigned int flags = 0);

    //通过模块名获取驱动所在路径
    QString  modGetPath(const QString &modName);
    //获取指定驱动名称
    QString modGetName(const QString &modPath);
    //获取驱动信息
    QString modGetInfo(const QString &modName, ModInfoType infotype);
    //判断模块是否为内建
    bool modIsBuildIn(const QString &modName);
    //判断模块是否已加载
    bool modIsLoaded(const QString &modName);
    //判断模块是否在黑名单中
    bool modIsBlackListed(const QString &modName);
    //获取指定项conf
    QStringList modGetConfsWithType(ModConfType conftype);
    //判断是否为驱动文件
    bool isModFile(const QString &filePath);
    //将Mod加入黑名单
    bool addModBlackList(const QString &modName);
    //将Mod移除黑名单
    void rmFromBlackList(const QString &modName);
    //设置Mod开机启动
    bool setModLoadedOnBoot(const QString &modName);
    //移除mod loaded on boot
    void rmModLoadedOnBoot(const QString &modName);


private:
    //new一个新kmod_module
    int modNew(struct kmod_ctx *ctx, const QString &modName, kmod_module *&mod);
    //判断mod是文件路径还是模块名
    bool bFromPath(const QString &modName);
    //infotype to string
    QString infoType2String(ModInfoType infotype);
    //获取kmod init state
    int modGetInitState(const QString &modName);
    //blacklist 文件删除操作
    void rmModFromBlackListFile(const QString &filepath, const QString &modName);
    //删除自启动配置
    void rmModFromLoadonBootConf(const QString &filepath, const QString &modName);
    //删除文件指定行
    void deleteLineOfFileWithItem(const QString &filepath, const QString &item);
    /**
     * @brief updateInitramfs       更新现有的initramfs
     */
    void updateInitramfs();

};

#endif // MODCORE_H
#endif // DISABLE_DRIVER
