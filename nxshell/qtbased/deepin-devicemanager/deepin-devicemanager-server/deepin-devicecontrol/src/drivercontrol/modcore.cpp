// SPDX-FileCopyrightText: 2019 ~ 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DISABLE_DRIVER

#include "modcore.h"
#include "DDLog.h"

#include <QFile>
#include <QDir>
#include <QFileInfo>
#include <QTextStream>
#include <QProcess>
#include <QLoggingCategory>

using namespace DDLog;

const QString  BLACKLISTT_PROBE_DIR_ETC = "/etc/modprobe.d";   //黑名单配置路径
const QString  BLACKLISTT_PROBE_DIR_USR_LIB = "/usr/lib/modprobe.d";  //黑名单配置路径
const QString  LOADONBOOT_PROBE_DIR = "/etc/modules-load.d";  //开机加载配置路径
const QString  BLACKLIST_FILENAME_TEMPLETE = "blacklist-%1-drivermanager.conf"; //驱动黑名单文件命名模板
const QString  LOADONBOOT_FILENAME_TEMPLETE = "%1-drivermanager.conf";  //驱动设置开机启动配置文件

ModCore::ModCore(QObject *parent)
    : QObject(parent)
{

}

/**
 * @brief ModCore::checkModuleInUsed 获取依赖当前模块在使用的模块
 * @param modName 模块名 sample: hid or hid.ko /xx/xx/hid.ko
 * @return 返回当前依赖模块列表 like ['usbhid', 'hid_generic']
 */
QStringList ModCore::checkModuleInUsed(const QString &modName)
{
    QStringList modList;
    struct kmod_ctx *ctx = nullptr;
    const char **null_config = nullptr;

    ctx = kmod_new(nullptr, null_config);
    if (!ctx) {
        qCInfo(appLog) << "kmod_new() failed!";
    } else {
        int err = 0;
        struct kmod_module *mod = nullptr;
        err = modNew(ctx, modName, mod);

        if (err < 0) {
            qCInfo(appLog) << "modulename=" << modName << "modNew failed errcode=" << err;
        } else {
            struct kmod_list *holders = nullptr;
            int state = 0;
            state = kmod_module_get_initstate(mod);

            if (state == KMOD_MODULE_BUILTIN) {
                qCInfo(appLog) << QString("Module %1 is builtin.").arg(kmod_module_get_name(mod)) << " errcode=" << -ENOENT;
            } else if (state < 0) {
                qCInfo(appLog) << QString("Module %1 is not currently loaded").arg(kmod_module_get_name(mod)) << " errcode=" << -ENOENT;
            } else {
                holders = kmod_module_get_holders(mod);
                if (holders != nullptr) {
                    struct kmod_list *itr;
                    qCInfo(appLog) << QString("Module %1 is in use by:").arg(kmod_module_get_name(mod));
                    kmod_list_foreach(itr, holders) {
                        struct kmod_module *hm = kmod_module_get_module(itr);
                        modList.append(QString("%1").arg(kmod_module_get_name(hm)));
                        fprintf(stderr, " %s", kmod_module_get_name(hm));
                        kmod_module_unref(hm);
                    }
                    kmod_module_unref_list(holders);
                }
            }
            kmod_module_unref(mod);
        }
        kmod_unref(ctx);
    }

    return  modList;
}

/**
 * @brief ModCore::rmModForce 强制移除驱动模块 等效于rmmod -f
 * @param modName 模块名sample: hid or hid.ko
 * @return 删除结果 true:成功 false: 失败
 */
bool ModCore::rmModForce(const QString &modName, QString &errMsg)
{
    bool bsuccess = true;
    struct kmod_ctx *ctx = nullptr;
    const char **null_config = nullptr;

    ctx = kmod_new(nullptr, null_config);
    if (!ctx) {
        bsuccess = false;
        qCInfo(appLog) << __func__ << "kmod_new() failed!";
    } else {
        int err = 0;
        struct kmod_module *mod = nullptr;
        err = modNew(ctx, modName, mod);

        if (err < 0) {
            bsuccess = false;
            qCInfo(appLog) << __func__ << "modulename=" << modName << "modNew failed errcode=" << err;
            errMsg = QString("%1").arg(abs(err));
        } else {
            err = kmod_module_remove_module(mod, KMOD_REMOVE_NOWAIT);
            if (err < 0) {
                errMsg = QString("%1").arg(abs(err));
                bsuccess = false;
                qCInfo(appLog) << __func__ << QString("could not remove module %1: %1\n").arg(modName).arg(err);
            }
            kmod_module_unref(mod);
        }
        kmod_unref(ctx);
    }
    return  bsuccess;
}

/**
 * @brief ModCore::modInstall 驱动ko安装
 * @param modName 模块名sample: hid
 * @param flags 安装属性，属性值参照kmod_probe解释
 * @return 反回错误类型枚举值
 */
bool ModCore::modInstall(const QString &modName, QString &errMsg, unsigned int flags)
{
    bool success = true;
    struct kmod_ctx *ctx = nullptr;
    const char **null_config = nullptr;

    ctx = kmod_new(nullptr, null_config);
    if (!ctx) {
        success = false;
        qCInfo(appLog) << __func__ << "kmod_new() failed!";
    } else {
        int err = 0;
        struct kmod_list *modlist = nullptr;
        err = kmod_module_new_from_lookup(ctx, modName.toStdString().c_str(), &modlist);
        if (nullptr != modlist && err >= 0) {
            struct kmod_list *ltmp;
            kmod_list_foreach(ltmp, modlist) {
                struct kmod_module *mod = kmod_module_get_module(ltmp);
                err = kmod_module_probe_insert_module(mod, flags, nullptr, nullptr, nullptr, nullptr);
                if (err < 0) {
                    errMsg = QString("%1").arg(abs(err));
                    success = false;
                    qCInfo(appLog) << __func__ << QString("could not insert module %1: %1\n").arg(modName).arg(err);
                }
                kmod_module_unref(mod);
                break; //出错一次直接错误返回
            }
            kmod_module_unref_list(modlist);
        } else {
            qCInfo(appLog) << __func__ << QString("Mod %1 not found in directory %2").arg(modName).arg(kmod_get_dirname(ctx));
            success = false;
        }

        kmod_unref(ctx);
    }
    return  success;
}

/**
 * @brief ModCore::modGetPath 通过模块名获取驱动所在路径
 * @param modName 模块名sample: hid or hid.ko
 * @return 模块所在路径
 */
QString ModCore::modGetPath(const QString &modName)
{
    QString path;
    struct kmod_ctx *ctx = nullptr;
    const char **null_config = nullptr;

    ctx = kmod_new(nullptr, null_config);
    if (!ctx) {
        qCInfo(appLog) << __func__ << "kmod_new() failed!";
    } else {
        int err = 0;
        struct kmod_module *mod = nullptr;
        err = modNew(ctx, modName, mod);

        if (err < 0) {
            qCInfo(appLog) << __func__ << "modulename=" << modName << "modNew failed errcode=" << err;
        } else {
            path.append(kmod_module_get_path(mod));
            kmod_module_unref(mod);
        }
        kmod_unref(ctx);
    }
    return  path;
}

/**
 * @brief ModCore::modGetName 获取指定驱动的module name
 * @param modPath 驱动文件路径
 * @return module name
 */
QString ModCore::modGetName(const QString &modPath)
{
    QString modname;
    struct kmod_ctx *ctx = nullptr;
    const char **null_config = nullptr;

    ctx = kmod_new(nullptr, null_config);
    if (!ctx) {
        qCInfo(appLog) << __func__ << "kmod_new() failed!";
    } else {
        int err = 0;
        struct kmod_module *mod = nullptr;
        err = modNew(ctx, modPath, mod);

        if (err < 0) {
            qCInfo(appLog) << __func__ << "modulepath=" << modPath << "modNew failed errcode=" << err;
        } else {
            modname.append(kmod_module_get_name(mod));
            kmod_module_unref(mod);
        }
        kmod_unref(ctx);
    }
    return  modname;
}

/**
 * @brief ModCore::modGetInfo 获取驱动信息
 * @param modName 驱动名称/xx/xx/hid.ko or hid
 * @param infotype 信息类型枚举值
 * @return 返回驱动指定类型信息
 */
QString ModCore::modGetInfo(const QString &modName, ModCore::ModInfoType infotype)
{
    QString modinfo;
    struct kmod_ctx *ctx = nullptr;
    const char **null_config = nullptr;

    ctx = kmod_new(nullptr, null_config);
    if (!ctx) {
        qCInfo(appLog) << __func__ << "kmod_new() failed!";
        return QString();
    } else {
        int err = 0;
        struct kmod_module *mod = nullptr;
        err = modNew(ctx, modName, mod);

        if (err < 0) {
            qCInfo(appLog) << __func__ << "modName=" << modName << "modNew failed errcode=" << err;
            return QString();
        } else {
            struct kmod_list *modlist = nullptr;
            err = kmod_module_get_info(mod, &modlist);
            if (err < 0) {
                qCInfo(appLog) << __func__ << QString("could not get mod info from %1, errno=%2")
                        .arg(kmod_module_get_name(mod)).arg(err);
                return QString();
            }
            kmod_list *ltmp = nullptr;
            QString strtypeinfo = infoType2String(infotype);
            kmod_list_foreach(ltmp, modlist) {
                const char *key = kmod_module_info_get_key(ltmp);
                const char *value = kmod_module_info_get_value(ltmp);
                if (0 == strtypeinfo.compare(QString(key))) {
                    modinfo = QString(value);
                    break;//找到直接结束查找
                }
                qCInfo(appLog) << key << value;
            }

            kmod_module_info_free_list(modlist);
            kmod_module_unref(mod);
        }
        kmod_unref(ctx);
    }
    return  modinfo;
}

/**
 * @brief ModCore::modIsBuildIn  判断模块是否未内建模块
 * @param modName 模块名称 sample: /xx/xx/cfbcopyarea.ko or cfbcopyarea
 * @return true: 是 false:否
 */
bool ModCore::modIsBuildIn(const QString &modName)
{
    return  KMOD_MODULE_BUILTIN == modGetInitState(modName);
}

/**
 * @brief ModCore::modIsLoaded module is live in kernel
 * @param modName 模块名称 sample: /xx/xx/cfbcopyarea.ko or cfbcopyarea
 * @return true: 是 false:否或失败
 */
bool ModCore::modIsLoaded(const QString &modName)
{
    return  KMOD_MODULE_LIVE == modGetInitState(modName);
}

/**
 * @brief ModCore::modIsBlackListed 判断模块是否已被加入黑名单
 * @param modName sample: /xx/xx/cfbcopyarea.ko or cfbcopyarea
 * @return true: 在黑名单 false:不在黑名单或获取失败
 */
bool ModCore::modIsBlackListed(const QString &modName)
{
    QStringList confs = modGetConfsWithType(EBlackListConf);
    return  confs.contains(modGetName(modName));
}

/**
 * @brief ModCore::modNew new一个新kmod_module
 * @param ctx kmod 上下文对象
 * @param modName 模块名 sample: hid or hid.ko /xx/xx/hid.ko
 * @param mod 模块对象
 * @param bfrompath 是否为文件 true:/xx/xx/hid.ko false:hid or hid.ko
 * @return 错误码
 */
int ModCore::modNew(struct kmod_ctx *ctx, const QString &modName, kmod_module *&mod)
{
    int ret = 0;
    bool bfrompath = bFromPath(modName);
    if (bfrompath) {
        ret = kmod_module_new_from_path(ctx, modName.toStdString().c_str(), &mod);
    } else {
        ret = kmod_module_new_from_name(ctx, modName.toStdString().c_str(), &mod);
    }
    return  ret;
}

/**
 * @brief ModCore::bFromPath modName是为文件路径
 * @param modName 模块名
 * @return  true: /xx/xx/hid.ko false : hid
 */
bool ModCore::bFromPath(const QString &modName)
{
    return  QFile::exists(modName);
}

/**
 * @brief ModCore::infoType2String 模块信息类型转字符串
 * @param infotype 模块信息类型枚举值
 * @return 返回信息类型字符串
 */
QString ModCore::infoType2String(ModCore::ModInfoType infotype)
{
    QString strtype;
    switch (infotype) {
    case EAlias:
        strtype = "alias";
        break;
    case ELiense:
        strtype = "license";
        break;
    case EVersion:
        strtype = "version";
        break;
    case EAuther:
        strtype = "author";
        break;
    case EDescription:
        strtype = "description";
        break;
    case ESrcVersion:
        strtype = "srcversion";
        break;
    case EName:
        strtype = "name";
        break;
    case EVermagic:
        strtype = "vermagic";
        break;
    }
    return  strtype;
}

/**
 * @brief ModCore::modGetInitState Get the initstate of this modName, as returned by Linux Kernel, by reading /sys filesystem
 * @param modName
 * @return 模块状态枚举
 * KMOD_MODULE_BUILTIN: module is builtin;
 * KMOD_MODULE_LIVE: module is live in kernel;
 * KMOD_MODULE_COMING: module is being loaded;
 * KMOD_MODULE_GOING: module is being unloaded.
 */
int ModCore::modGetInitState(const QString &modName)
{
    int state = -1;
    struct kmod_ctx *ctx = nullptr;
    const char **null_config = nullptr;

    ctx = kmod_new(nullptr, null_config);
    if (ctx) {
        struct kmod_module *mod = nullptr;
        int err = modNew(ctx, modName, mod);

        if (0 == err) {
            state = kmod_module_get_initstate(mod);
            kmod_module_unref(mod);
        }
        kmod_unref(ctx);
    }

    return  state;
}

/**
 * @brief ModCore::rmModFromBlackListFile 从指定blacklist文件中删除对modName设置
 * @param filepath blacklist 文件路径
 * @param modName 模块名称
 */
void ModCore::rmModFromBlackListFile(const QString &filepath, const QString &modName)
{
    QFileInfo info(filepath);
    QString strpath = filepath;
    if (info.isSymLink()) {
        strpath = info.symLinkTarget();
        if (strpath.isEmpty())
            return;
    }
    QFile tmpfile(strpath);
    if (!tmpfile.exists() || !tmpfile.open(QIODevice::ReadWrite))
        return;
    QString strcontent = tmpfile.readAll();
    if (!(strcontent.contains("blacklist") && strcontent.contains(modName)))
        return;
    deleteLineOfFileWithItem(strpath, modName);
}

/**
 * @brief ModCore::rmModFromLoadonBootConf 删除自启动配置
 * @param filepath
 * @param modName
 */
void ModCore::rmModFromLoadonBootConf(const QString &filepath, const QString &modName)
{
    QFileInfo info(filepath);
    QString strpath = filepath;
    if (info.isSymLink()) {
        strpath = info.symLinkTarget();
        if (strpath.isEmpty())
            return;
    }
    QFile tmpfile(strpath);
    if (!tmpfile.exists() || !tmpfile.open(QIODevice::ReadWrite))
        return;
    QString strcontent = tmpfile.readAll();
    if (!strcontent.contains(modName))
        return;

    deleteLineOfFileWithItem(strpath, modName);
}

/**
 * @brief ModCore::deleteLineOfFileWithItem 删除文件包含item的行
 * @param filepath 文件路径
 * @param item 包含选项
 */
void ModCore::deleteLineOfFileWithItem(const QString &filepath, const QString &item)
{
    QFile tmpfile(filepath);

    //设置文件游标到起始位置
    tmpfile.seek(0);
    QTextStream stream(&tmpfile);
    QString strline;
    QString newcontent;
    QTextStream newstream(&newcontent);
    while (stream.readLineInto(&strline)) {
        if (!strline.contains(item)) {
            newstream << strline << endl;
            continue;
        }
    }
    //清除原有内容
    tmpfile.resize(0);
    stream << newcontent;
    tmpfile.close();
}

//更新现有的initramfs
void ModCore::updateInitramfs()
{
    QProcess process;
    process.start("update-initramfs -u");
    process.waitForFinished(-1);
}

void ModCore::rmModLoadedOnBoot(const QString &modName)
{
    QString conffile = QString(LOADONBOOT_FILENAME_TEMPLETE).arg(modName);
    QDir bootloaddir(LOADONBOOT_PROBE_DIR);
    QStringList bootconfs = bootloaddir.entryList(QDir::Files | QDir::NoDotAndDotDot | QDir::Readable);

    //先判断文件是否为设备管理器创建，如果是则直接删除文件
    if (bootconfs.contains(conffile)) {
        bootloaddir.remove(conffile);
        return ;
    }

    //遍历处理/etc/modules-load.d目录下文件
    foreach (QString strfile, bootconfs) {
        rmModFromLoadonBootConf(bootloaddir.absoluteFilePath(strfile), modName);
    }
}

/**
 * @brief ModCore::modGetConfsWithType 获取指定类型配置信息
 * @param conftype 配置类型
 * @return 返回配置信息列表
 */
QStringList ModCore::modGetConfsWithType(ModConfType conftype)
{
    QStringList conflist;
    struct kmod_ctx *ctx = nullptr;
    const char **null_config = nullptr;
    ctx = kmod_new(nullptr, null_config);

    if (nullptr != ctx) {
        QString confkey;
        kmod_config_iter *(*get_iter)(const struct kmod_ctx * ctx);
        switch (conftype) {
        case EBlackListConf:
            get_iter = kmod_config_get_blacklists;
            break;
        case EAliasConf:
            get_iter = kmod_config_get_aliases;
            break;
        case EOptionsConf:
            get_iter = kmod_config_get_options;
            break;
        case ESoftdepConf:
            get_iter = kmod_config_get_softdeps;
            break;
        }
        struct kmod_config_iter *iter = get_iter(ctx);
        if (nullptr != iter) {
            while (kmod_config_iter_next(iter)) {
                QString strconf(kmod_config_iter_get_key(iter));
                if (!strconf.isEmpty())
                    conflist.append(strconf);
            }
            kmod_config_iter_free_iter(iter);
        }
        kmod_unref(ctx);
    }
    return conflist;
}

/**
 * @brief ModCore::isModFile 判断文件是否未驱动文件
 * @param filePath 文件路径
 * @return true:是 false 否
 */
bool ModCore::isModFile(const QString &filePath)
{
    if (!bFromPath(filePath))
        return  false;
    bool bmodfile = false;
    struct kmod_ctx *ctx = nullptr;
    const char **null_config = nullptr;
    ctx = kmod_new(nullptr, null_config);
    if (ctx) {
        struct kmod_module *mod = nullptr;
        int err = modNew(ctx, filePath, mod);
        if (0 == err) {
            struct kmod_list *modlist = nullptr;
            err = kmod_module_get_info(mod, &modlist);
            if (err >= 0) {
                bmodfile = true;
                kmod_module_info_free_list(modlist);
            }
            kmod_module_unref(mod);
        }
        kmod_unref(ctx);
    }
    qCInfo(appLog) << "" << bmodfile;
    return bmodfile;
}

/**
 * @brief ModCore::addModBlackList 将驱动模块加入黑名单
 * @param modName 模块名称不是文件名,必须为模块名 sample bluetooth 如果是路径可通过modGetName转换获取
 * @return true:成功 false:失败
 */
bool ModCore::addModBlackList(const QString &modName)
{
    QString blacklistdir;
    if (QFileInfo::exists(BLACKLISTT_PROBE_DIR_ETC)) {
        blacklistdir = BLACKLISTT_PROBE_DIR_ETC;
    } else if (QFileInfo::exists(BLACKLISTT_PROBE_DIR_USR_LIB)) {
        blacklistdir = BLACKLISTT_PROBE_DIR_USR_LIB;
    }

    if (blacklistdir.isEmpty())
        return  false;
    //sample: /etc/modprobe.d/blacklist-bnep-drivermanager.conf
    QString basefilename = QString(BLACKLIST_FILENAME_TEMPLETE).arg(modName);
    QString filepath = QString("%1/%2").arg(blacklistdir).arg(basefilename);
    QFile blackfile(filepath);
    if (!blackfile.open(QIODevice::ReadWrite))
        return  false;
    QTextStream instream(&blackfile);
    instream << QString("blacklist %1").arg(modName) << endl;
    //屏蔽模块及所有依赖它的模块，设置后无法通过modprobe xx or insmod xx 进行安装
    instream << QString("install %1 /bin/false").arg(modName) << endl;
    //添加黑名单后需要更新现有的initramfs
    updateInitramfs();
    return  true;
}

/**
 * @brief ModCore::rmFromBlackList 移除模块黑名单
 * @param modName 模块名称非路径 sample bnep
 */
void ModCore::rmFromBlackList(const QString &modName)
{
    //移除黑名单配置，查找目录BLACKLISTT_PROBE_DIR_ETC 和BLACKLISTT_PROBE_DIR_USR_LIB
    //1.通过本应用安装可以移除文件
    //2.如果要移除其它的只能遍历其它配置文件进行删除
    QString etcblacklistfile = QString(BLACKLIST_FILENAME_TEMPLETE).arg(modName);
    QDir etcdir(BLACKLISTT_PROBE_DIR_ETC);
    QStringList etcfiles = etcdir.entryList(QDir::Files | QDir::NoDotAndDotDot | QDir::Readable);

    //先判断文件是否为设备管理器创建，如果是则直接删除文件
    if (etcfiles.contains(etcblacklistfile)) {
        etcdir.remove(etcblacklistfile);
        return ;
    }

    //遍历处理/etc/modprobe.d目录下文件
    foreach (QString strfile, etcfiles) {
        rmModFromBlackListFile(etcdir.absoluteFilePath(strfile), modName);
    }

    QString libblacklistfile = QString(BLACKLIST_FILENAME_TEMPLETE).arg(modName);
    QDir libdir(BLACKLISTT_PROBE_DIR_USR_LIB);
    QStringList libfiles = libdir.entryList();
    if (libfiles.contains(libblacklistfile)) {
        libdir.remove(libblacklistfile);
        return ;
    }
    //遍历处理/usr/lib/modprobe.d目录下文件
    foreach (QString strfile, libfiles) {
        rmModFromBlackListFile(etcdir.absoluteFilePath(strfile), modName);
    }
}

/**
 * @brief ModCore::setModLoadedOnBoot 设置启动时加载mod
 * @param modName 模块名称非路径 sample : bnep
 * @return true: 成功 false: 失败
 * 直接在加载目录下创建配置文件，不在已有文件添加
 */
bool ModCore::setModLoadedOnBoot(const QString &modName)
{
    QFileInfo fileinfo(LOADONBOOT_PROBE_DIR);
    QString strfilename = QString(LOADONBOOT_FILENAME_TEMPLETE).arg(modName);
    QString filepath = fileinfo.dir().absoluteFilePath(strfilename);
    QFile bootconf(filepath);
    if (!bootconf.open(QIODevice::ReadWrite))
        return  false;
    bootconf.write(modName.toStdString().c_str());

    return  true;
}
#endif // DISABLE_DRIVER
