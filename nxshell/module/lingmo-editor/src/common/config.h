// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CONFIG_H
#define CONFIG_H

#include <QObject>

#include <dtkcore_config.h>
#ifdef DTKCORE_CLASS_DConfigFile
#include <DConfig>
DCORE_USE_NAMESPACE
#endif

class Config : public QObject
{
    Q_OBJECT
    explicit Config(QObject *parent = nullptr);
    ~Config() override;

public:
    static Config *instance();
    bool enableImproveGB18030() const;
    bool enablePatchedIconv() const;
    QByteArray defaultEncoding() const;

private:
#ifdef DTKCORE_CLASS_DConfigFile
    DConfig *dconfig = nullptr;
#endif
    bool improveGB18030 = true;  ///< 默认提高GB18030编码识别率
    bool patchedIconv = false;   ///< 默认不再使用上层修改的iconv
    bool iocnvUse2005Standard = false;  ///< 检测当前iconv使用的GB18030编码是否为2005标准(2005标准强制使用上层补丁版本)
    QByteArray encoding;  ///< 缺省编码设置
};

#endif  // CONFIG_H
