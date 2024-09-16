// Copyright (C) 2017 ~ 2017 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-only

#ifndef COMMANDLINEPARSER_H
#define COMMANDLINEPARSER_H

#include <QCommandLineParser>

class CommandLineParser
{
public:
    CommandLineParser();

    void process(const QCoreApplication &app);
    void process(const QStringList &arguments);
    void parse();

    QString source() const;
    QString target() const;
    QString logFile() const;
    QString formatLogFile() const;
    QString logBackupFile() const;

    bool isSetOverride() const;
    bool isSetDebug() const;

private:
    QCommandLineParser parser;
    QCommandLineOption o_info;
    QCommandLineOption o_dim_info;
    QCommandLineOption o_override;
    QCommandLineOption o_compress_level;
    QCommandLineOption o_buffer_size;
    QCommandLineOption o_non_ui;
    QCommandLineOption o_to_serial_url;
    QCommandLineOption o_from_serial_url;
    QCommandLineOption o_disable_check_dim;
    QCommandLineOption o_reset_checksum;
    QCommandLineOption o_log_file;
    QCommandLineOption o_format_log_file;
    QCommandLineOption o_log_backup;
    QCommandLineOption o_loop_device;
    QCommandLineOption o_debug_level;
    QCommandLineOption o_fix_boot;
    QCommandLineOption o_auto_fix_boot;
    QCommandLineOption o_write_custom_file;
    QCommandLineOption o_read_custom_file;
};

#endif // COMMANDLINEPARSER_H
