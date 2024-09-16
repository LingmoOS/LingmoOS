// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <QCoreApplication>
#include <QCommandLineParser>
#include <QMetaProperty>
#include <QDebug>

#include <DOpticalDiscManager>
#include <DOpticalDiscInfo>
#include <DOpticalDiscOperator>

DBURN_USE_NAMESPACE

// --dev '/dev/sr0' -p
void print(const QString &device)
{
    qInfo() << __PRETTY_FUNCTION__;
    auto expected { DOpticalDiscManager::createOpticalDiscInfo(device) };
    if (!expected) {
        qWarning() << expected.error();
        return;
    }

    DOpticalDiscInfo *info { expected.value() };
    if (!info)
        return;

    auto meta { info->metaObject() };
    int count { meta->propertyCount() };
    qInfo() << "===============Begin visit info===============";
    for (int i = 1; i != count; ++i) {
        const QMetaProperty &property { meta->property(i) };
        qInfo() << "-> Property: " << property.name() << info->property(property.name());
    }
    qInfo() << "===============End visit info===============";

    delete info;
}
// --dev '/dev/sr0' --erase
void erase(const QString &device)
{
    qInfo() << __PRETTY_FUNCTION__;

    auto expected { DOpticalDiscManager::createOpticalDiscOperator(device) };
    if (!expected) {
        qWarning() << expected.error();
        return;
    }

    DOpticalDiscOperator *oprator { expected.value() };
    if (!oprator)
        return;

    QObject::connect(oprator, &DOpticalDiscOperator::jobStatusChanged,
                     [](JobStatus status, int progress, QString speed, QStringList message) {
                         qDebug() << "erase progress: " << int(status) << progress << speed << message;
                     });

    oprator->erase();
    delete oprator;
}

// --dev '/dev/sr0' --burn '<directory>'
void burn(const QString &device, const QString &stagePath, const QString &fs)
{
    // TODO(zhangs): suport filesystem
    Q_UNUSED(fs)

    qInfo() << __PRETTY_FUNCTION__;
    auto expected { DOpticalDiscManager::createOpticalDiscOperator(device) };
    if (!expected) {
        qWarning() << expected.error();
        return;
    }

    DOpticalDiscOperator *oprator { expected.value() };
    if (!oprator)
        return;

    QObject::connect(oprator, &DOpticalDiscOperator::jobStatusChanged,
                     [](JobStatus status, int progress, QString speed, QStringList message) {
                         qDebug() << "burn progress: " << int(status) << progress << speed << message;
                     });

    BurnOptions opts { BurnOption::KeepAppendable };
    opts |= BurnOption::JolietSupported;
    oprator->burn(stagePath, opts);

    delete oprator;
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    // 创建命令行解析对象
    QCommandLineParser parser;
    // 定义帮助信息
    parser.setApplicationDescription(QString("%1 helper").arg(QCoreApplication::applicationName()));
    // 定义参数， addOption() 方法
    parser.addOption(QCommandLineOption({ "dev", "device" }, "Set disc device(like '/dev/sr0').", "device"));
    // 'iso9660', 'joliet', 'rockridge' and 'udf' supported
    parser.addOption(QCommandLineOption({ "fs", "filesystem" },
                                        "Set filesystem for burn files (iso9660', 'joliet', 'rockridge' and 'udf' supported).",
                                        "filesystem name", "joliet"));
    parser.addOption(QCommandLineOption({ "p", "print" }, "Print disc info."));
    parser.addOption(QCommandLineOption("erase", "Erase a disc."));
    parser.addOption(QCommandLineOption("check", "Check media."));
    parser.addOption(QCommandLineOption("dump", "Dump a iso file from disc to local <directory>.", "directory"));
    parser.addOption(QCommandLineOption("write", "Write a image file to disc.", "image directory"));
    parser.addOption(QCommandLineOption("burn", "Burn files from local <directory> to disc.", "directory"));

    parser.addHelpOption();
    // 解析参数，parse() 方法
    parser.process(app);

    // 获取参数值
    const QString &device { parser.value("dev") };

    if (device.isEmpty()) {
        qWarning() << "Cannot find 'device'";
        exit(-1);
    }

    // 执行光盘相关操作（必须卸载掉光盘）
    {
        // 打印光盘信息
        if (parser.isSet("p")) {
            print(device);
            exit(0);
        }

        // 擦除光盘
        if (parser.isSet("erase")) {
            erase(device);
            ::exit(0);
        }

        // 刻录文件到光盘
        if (parser.isSet("burn")) {
            const QString &fs { parser.value("fs") };
            const QString &path { parser.value("burn") };
            burn(device, path, fs);
            ::exit(0);
        }

        qWarning() << "No operation was performed";
    }

    return app.exec();
}
