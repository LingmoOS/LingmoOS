// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// 项目自身文件
#include "DeviceFactory.h"
#include "X86Generator.h"
#include "MipsGenerator.h"
#include "ArmGenerator.h"
#include "KLUGenerator.h"
#include "PanguGenerator.h"
#include "PanguVGenerator.h"
#include "HWGenerator.h"
#include "KLVGenerator.h"
#include "commonfunction.h"
#include "PanguxGenerator.h"

// Qt库文件
#include <QProcess>
#include <QFile>

DeviceFactory::DeviceFactory()
{

}

DeviceGenerator *DeviceFactory::getDeviceGenerator()
{
    // 获取系统架构
    QString arch = Common::getArch();

    // 根据架构创建设备信息生成器
    DeviceGenerator *generator = nullptr;
    if (arch == "x86_64")
        generator = new X86Generator();
    else if (arch == "mips64")
        generator = new MipsGenerator();
    else if (arch == "aarch64") {
        QString type = Common::boardVendorType();
        if (!type.isEmpty()) {
            if (type == "KLVV")
                generator = new KLVGenerator();
            else if (type == "PGUV" || type == "PGUW")
                generator = new PanguVGenerator();
            else if (type == "KLVU")
                generator = new KLUGenerator();
            else if (type == "PGUX")
                generator = new PanguXGenerator();
            else
                generator = new HWGenerator();
        } else
            generator = new ArmGenerator();
    } else
        generator = new X86Generator();

    return generator;
}
