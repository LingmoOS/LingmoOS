// Copyright (C) 2017 ~ 2017 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-only

#include "commandlineparser.h"
#include "corelib/ddiskinfo.h"
#include "corelib/dvirtualimagefileio.h"
#include "corelib/helper.h"
#include "dglobal.h"
#include "fixboot/bootdoctor.h"

#include <cstdio>

CommandLineParser::CommandLineParser()
    : o_info(QStringList() << "i" << "info")
    , o_dim_info("dim-info")
    , o_override(QStringList() << "O" << "override")
    , o_compress_level(QStringList() << "C" << "compress-level")
    , o_buffer_size(QStringList() << "B" << "buffer-size")
    , o_non_ui("tui")
    , o_to_serial_url("to-serial-url")
    , o_from_serial_url("from-serial-url")
    , o_disable_check_dim("no-check-dim")
    , o_reset_checksum("re-checksum")
    , o_log_file(QStringList() << "L" << "log-file")
    , o_format_log_file(QStringList() << "format-log-file")
    , o_log_backup(QStringList() << "log-backup")
    , o_loop_device(QStringList() << "loop-device")
    , o_debug_level(QStringList() << "d" << "debug")
    , o_fix_boot(QStringList() << "f" << "fix-boot")
    , o_auto_fix_boot(QStringList() << "auto-fix-boot")
    , o_write_custom_file(QStringList() << "write-custom-file")
    , o_read_custom_file(QStringList() << "read-custom-file")
{
    o_info.setDescription("Get the device info.");
    o_dim_info.setDescription("Get the dim file info.");
    o_override.setDescription("If the target file exists when the override.");
    o_compress_level.setDescription("Output to the dim file when the data compression level.");
    o_compress_level.setValueName("Compress Level");
    o_compress_level.setDefaultValue(QString::number(Global::compressionLevel));
    o_buffer_size.setDescription("The size of the buffer when data is transferred.");
    o_buffer_size.setValueName("Buffer Size");
    o_buffer_size.setDefaultValue(QString::number(Global::bufferSize));
    o_non_ui.setDescription("Run in TUI mode.");
    o_to_serial_url.setDescription("File path format to serial url format.");
    o_to_serial_url.setValueName("File Path");
    o_from_serial_url.setDescription("Serial url format to file path format.");
    o_from_serial_url.setValueName("Serial URL");
    o_disable_check_dim.setDescription("Ignore dim file md5 check.");
    o_reset_checksum.setDescription("Recalculate and write the checksum of the dim file.");
    o_reset_checksum.setValueName("File Path");
    o_log_file.setDescription("Log file path.");
    o_log_file.setValueName("File Path");
    o_log_file.setDefaultValue(QString("/var/log/%2.log").arg(qApp->applicationName()));
    o_format_log_file.setDescription("Log file path of the format log messages");
    o_format_log_file.setValueName("File Path");
    o_format_log_file.setDefaultValue(QString());
    o_log_backup.setDescription("Backup log file to path on application quit.");
    o_log_backup.setValueName("File Path");
    o_loop_device.setDescription("Do not block loop device.");
    o_debug_level.setDescription("Set the debug level[0|1|2].");
    o_debug_level.setValueName("Level");
    o_debug_level.setDefaultValue(QString::number(Global::debugLevel));
    o_fix_boot.setDescription("Fix the partition bootloader.");
    o_fix_boot.setValueName("Partition Device Path");
    o_auto_fix_boot.setDescription("Auto fix the partition bootloader on the clone/restore job finished.");
    o_write_custom_file.setDescription("Write custom file data into dim file. Source file format: dim://example.dim/custom");
    o_read_custom_file.setDescription("Read data from custom file. Source file format: dim://example.dim/custom");

    QDir::current().mkpath(QStandardPaths::writableLocation(QStandardPaths::CacheLocation));

    parser.addOption(o_info);
    parser.addOption(o_dim_info);
    parser.addOption(o_override);
    parser.addOption(o_buffer_size);
    parser.addOption(o_compress_level);
    parser.addOption(o_buffer_size);
    parser.addOption(o_non_ui);
    parser.addOption(o_to_serial_url);
    parser.addOption(o_from_serial_url);
    parser.addOption(o_disable_check_dim);
    parser.addOption(o_reset_checksum);
    parser.addOption(o_log_file);
    parser.addOption(o_format_log_file);
    parser.addOption(o_log_backup);
    parser.addOption(o_loop_device);
    parser.addOption(o_debug_level);
    parser.addOption(o_fix_boot);
    parser.addOption(o_auto_fix_boot);
    parser.addOption(o_write_custom_file);
    parser.addOption(o_read_custom_file);
    parser.addHelpOption();
    parser.addVersionOption();

    parser.addPositionalArgument("source", "Srouce file.", "[path/serial]");
    parser.addPositionalArgument("target", "Output file.", "[path/serial]");

    parser.setApplicationDescription(QString("e.g(path):   %1 /dev/sda ~/sda.dim\n"
                                             "             %1 /dev/sda /dev/sdb\n"
                                             "             %1 ~/sda.dim /dev/sda\n\n"
                                             "e.g(serial): %1 serial://W530B6RT ~/W530B6RT.dim\n"
                                             "             %1 serial://W530B6RT:1 serial://W530B6RT:2\n"
                                             "             %1 serial://W530B6RT.dim serial://W530B6RT:0").arg(qApp->applicationName()));
}

void CommandLineParser::process(const QCoreApplication &app)
{
    process(app.arguments());
}

void CommandLineParser::process(const QStringList &arguments)
{
    parser.process(arguments);
}

void CommandLineParser::parse()
{
    Global::isOverride = parser.isSet(o_override);
    Global::disableMD5CheckForDimFile = parser.isSet(o_disable_check_dim);
    Global::disableLoopDevice = !parser.isSet(o_loop_device);
    Global::fixBoot = parser.isSet(o_auto_fix_boot);

    if (parser.isSet(o_buffer_size)) {
        bool ok = false;

        Global::bufferSize = parser.value(o_buffer_size).toInt(&ok);

        if (!ok || Global::bufferSize <= 0) {
            parser.showHelp(EXIT_FAILURE);
        }
    }

    if (parser.isSet(o_compress_level)) {
        bool ok = false;

        Global::compressionLevel = parser.value(o_compress_level).toInt(&ok);

        if (!ok) {
            parser.showHelp(EXIT_FAILURE);
        }
    }

    if (parser.isSet(o_debug_level)) {
        bool ok = false;

        Global::debugLevel = parser.value(o_debug_level).toInt(&ok);

        if (!ok || Global::debugLevel < 0 || Global::debugLevel > 2) {
            parser.showHelp(EXIT_FAILURE);
        }
    }

    if (parser.isSet(o_info)) {
        if (parser.positionalArguments().isEmpty()) {
            parser.showHelp(EXIT_FAILURE);
        }

        const DDiskInfo &info = DDiskInfo::getInfo(parser.positionalArguments().first());

        if (!info) {
            fputs(qPrintable(QString("%1 is invalid file").arg(parser.positionalArguments().first())), stderr);
            ::exit(EXIT_FAILURE);
        }

        printf("%s\n", info.toJson().constData());
        ::exit(EXIT_SUCCESS);
    } else if (parser.isSet(o_dim_info)) {
        if (parser.positionalArguments().isEmpty()) {
            parser.showHelp(EXIT_FAILURE);
        }

        DVirtualImageFileIO io(parser.positionalArguments().first());

        if (!io.isValid()) {
            fputs(qPrintable(QString("%1 is invalid file").arg(parser.positionalArguments().first())), stderr);
            ::exit(EXIT_FAILURE);
        }

        for (const QString &file : io.fileList()) {
            printf("File Name: %s\n", qPrintable(file));
            printf("Size: %s (%lld bytes)\n", qPrintable(Helper::sizeDisplay(io.size(file))), io.size(file));
            printf("Data Start: %lld\n", io.start(file));
            printf("Data End: %lld\n\n", io.end(file));
        }

        ::exit(EXIT_SUCCESS);
    } else if (parser.isSet(o_to_serial_url)) {
        printf("%s\n", Helper::toSerialUrl(parser.value(o_to_serial_url)).toUtf8().constData());

        ::exit(EXIT_SUCCESS);
    } else if (parser.isSet(o_from_serial_url)) {
        printf("%s\n", Helper::parseSerialUrl(parser.value(o_from_serial_url)).toUtf8().constData());

        ::exit(EXIT_SUCCESS);
    } else if (parser.isSet(o_fix_boot)) {
        if (BootDoctor::fix(parser.value(o_fix_boot))) {
            printf("Finished!\n");
            ::exit(EXIT_SUCCESS);
        } else {
            fputs(qPrintable(BootDoctor::errorString()), stderr);
            ::exit(EXIT_FAILURE);
        }
    } else if (parser.isSet(o_reset_checksum)) {
        if (DVirtualImageFileIO::updateMD5sum(parser.value(o_reset_checksum))) {
            ::exit(EXIT_SUCCESS);
        } else {
            ::exit(EXIT_FAILURE);
        }
    } else if (parser.isSet(o_write_custom_file)) {
        if (source().isEmpty()) {
            fputs(qPrintable("The source file is empty!\n"), stderr);
            ::exit(EXIT_FAILURE);
        }
        bool isOK = Helper::writeCustomFile(source(), target());
        isOK ? ::exit(EXIT_SUCCESS) : ::exit(EXIT_FAILURE);
    } else if (parser.isSet(o_read_custom_file)) {
        if (source().isEmpty()) {
            fputs(qPrintable("The source file is empty!\n"), stderr);
            ::exit(EXIT_FAILURE);
        }
        bool isOK = Helper::readCustomFile(source(), target());
        isOK ? ::exit(EXIT_SUCCESS) : ::exit(EXIT_FAILURE);
    } else {
        if ((Global::isTUIMode || !parser.positionalArguments().isEmpty()) && parser.positionalArguments().count() > 2) {
            parser.showHelp(EXIT_FAILURE);
        }
    }
}

QString CommandLineParser::source() const
{
    if (parser.positionalArguments().isEmpty())
        return QString();

    return parser.positionalArguments().first();
}

QString CommandLineParser::target() const
{
    if (parser.positionalArguments().count() < 2)
        return QString();

    return parser.positionalArguments().at(1);
}

QString CommandLineParser::logFile() const
{
    return parser.value(o_log_file);
}

QString CommandLineParser::formatLogFile() const
{
    return parser.value(o_format_log_file);
}

QString CommandLineParser::logBackupFile() const
{
    return  parser.value(o_log_backup);
}

bool CommandLineParser::isSetOverride() const
{
    return parser.isSet(o_override);
}

bool CommandLineParser::isSetDebug() const
{
    return parser.isSet(o_debug_level);
}
