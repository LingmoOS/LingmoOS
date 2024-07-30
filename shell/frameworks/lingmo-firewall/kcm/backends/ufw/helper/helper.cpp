// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: 2011 Craig Drummond <craig.p.drummond@gmail.com>
// SPDX-FileCopyrightText: 2018 Alexis Lopes Zubeta <contact@azubieta.net>
// SPDX-FileCopyrightText: 2020 Tomaz Canabrava <tcanabrava@kde.org>
/*
 * UFW KControl Module
 */

#include "helper.h"
#include <QByteArray>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QProcess>
#include <QProcessEnvironment>
#include <QString>
#include <QStringList>
#include <QStandardPaths>
#include <sys/stat.h>

#include <KAuth/HelperSupport>
#include <KLocalizedString>

#include "ufw_helper_config.h"

namespace
{
constexpr int FILE_PERMS = 0644;
constexpr int DIR_PERMS = 0755;
const QString KCM_UFW_DIR = QStringLiteral("/etc/kcm_ufw");
const QString PROFILE_EXTENSION = QStringLiteral(".ufw");

void setPermissions(const QString &f, int perms)
{
    // Clear any umask before setting file perms
    mode_t oldMask(umask(0000));
    ::chmod(QFile::encodeName(f).constData(), perms);
    // Reset umask
    ::umask(oldMask);
}

void checkFolder()
{
    QDir d(KCM_UFW_DIR);

    if (!d.exists()) {
        d.mkpath(KCM_UFW_DIR);
        setPermissions(d.absolutePath(), DIR_PERMS);
    }
}

} // namespace

namespace UFW
{
ActionReply Helper::query(const QVariantMap &args)
{
    ActionReply reply =
        args[QStringLiteral("defaults")].toBool() ? run({"--status", "--defaults", "--list", "--modules"}, "query") : run({"--status", "--list"}, "query");

    if (args[QStringLiteral("profiles")].toBool()) {
        QDir dir(KCM_UFW_DIR);
        const QStringList profiles = dir.entryList({"*" + PROFILE_EXTENSION});
        QMap<QString, QVariant> data;
        for (const QString &profile : profiles) {
            QFile f(dir.canonicalPath() + QChar('/') + profile);
            if (f.open(QIODevice::ReadOnly)) {
                data.insert(profile, f.readAll());
            }
        }
        reply.addData(QStringLiteral("profiles"), data);
    }

    return reply;
}

QStringList getLogFromSystemd(const QString &lastLine)
{
    QString program = QStringLiteral("journalctl");
    QStringList arguments{"-xb", "-n", "100", "-g", "UFW"};

    QProcess myProcess;
    myProcess.start(program, arguments);
    myProcess.waitForFinished();

    auto resultString = QString(myProcess.readAllStandardOutput());
    const auto resultList = resultString.split(QStringLiteral("\n"));

    // Example Line from Systemd:
    // Dec 06 17:42:45 tomatoland kernel: [UFW BLOCK] IN=wlan0 OUT= MAC= SRC=192.168.50.181 DST=224.0.0.252 LEN=56 TOS=0x00
    //     PREC=0x00 TTL=255 ID=52151 PROTO=UDP SPT=5355 DPT=5355 LEN=36
    // We need to remove everything up to the space after ']'.

    QStringList result;
    for (const QString &line : resultList) {
        if (!lastLine.isEmpty() && line == lastLine) {
            result.clear();
            continue;
        }
        result.append(line);
    }
    return result;
}

ActionReply Helper::queryapps(const QVariantMap &args)
{
    Q_UNUSED(args);
    QProcess ufw;
    ActionReply reply;

    const QString ufwexe = QStandardPaths::findExecutable("ufw", {"/usr/sbin", "/usr/bin", "/sbin", "/bin"});

    if (ufwexe.isEmpty()) {
        qDebug() << "Executable not found: ufw";
        return reply;
    }

    ufw.start(ufwexe, {"app", "list"}, QIODevice::ReadOnly);
    if (ufw.waitForStarted()) {
        ufw.waitForFinished();
    }

    auto result = QString::fromLocal8Bit(ufw.readAllStandardOutput()).split(QLatin1Char('\n'), Qt::SkipEmptyParts);

    // The first line of the array is "Available Applications:", remove that.
    if (result.count()) {
        result.removeAt(0);
    }

    for (auto &value : result) {
        value = value.trimmed();
    }

    reply.setData({{"response", result}});
    return reply;
}

ActionReply Helper::viewlog(const QVariantMap &args)
{
    ActionReply reply;
    QString lastLine = args["lastLine"].toString();

    QStringList result = getLogFromSystemd(lastLine);
    reply.addData(QStringLiteral("lines"), result);
    return reply;
}

ActionReply Helper::modify(const QVariantMap &args)
{
    QString cmd = args[QStringLiteral("cmd")].toString();

    return QStringLiteral("setStatus") == cmd    ? setStatus(args, cmd)
        : QStringLiteral("addRules") == cmd      ? addRules(args, cmd)
        : QStringLiteral("removeRule") == cmd    ? removeRule(args, cmd)
        : QStringLiteral("moveRule") == cmd      ? moveRule(args, cmd)
        : QStringLiteral("editRule") == cmd      ? editRule(args, cmd)
        : QStringLiteral("reset") == cmd         ? reset(cmd)
        : QStringLiteral("setDefaults") == cmd   ? setDefaults(args, cmd)
        : QStringLiteral("setModules") == cmd    ? setModules(args, cmd)
        : QStringLiteral("setProfile") == cmd    ? setProfile(args, cmd)
        : QStringLiteral("saveProfile") == cmd   ? saveProfile(args, cmd)
        : QStringLiteral("deleteProfile") == cmd ? deleteProfile(args, cmd)
                                                 : ActionReply::HelperErrorReply(STATUS_INVALID_CMD);
}

ActionReply Helper::setStatus(const QVariantMap &args, const QString &cmd)
{
    const QString enabled = args[QStringLiteral("status")].toBool() ? "true" : "false";

    return run({"--setEnabled=" + enabled}, {"--status"}, cmd);
}

ActionReply Helper::setDefaults(const QVariantMap &args, const QString &cmd)
{
    QStringList pquery({"--defaults"});
    if (args[QStringLiteral("ipv6")].toBool()) {
        pquery.append(QStringLiteral("--list"));
    }

    const QString defaults = args[QStringLiteral("xml")].toString();

    return run({"--setDefaults=" + defaults}, pquery, cmd);
}

ActionReply Helper::setModules(const QVariantMap &args, const QString &cmd)
{
    return run({"--setModules=" + args["xml"].toString()}, {"--modules"}, cmd);
}

ActionReply Helper::setProfile(const QVariantMap &args, const QString &cmd)
{
    QStringList cmdArgs;

    if (args.contains(QStringLiteral("ruleCount"))) {
        unsigned int count = args[QStringLiteral("ruleCount")].toUInt();

        cmdArgs.append(QStringLiteral("--clearRules"));
        for (unsigned int i = 0; i < count; ++i) {
            const QString argument = args["rule" + QString::number(i)].toString();
            cmdArgs.append("--add=" + argument);
        }
    }

    if (args.contains(QStringLiteral("defaults"))) {
        cmdArgs << "--setDefaults=" + args[QStringLiteral("defaults")].toString();
    }
    if (args.contains(QStringLiteral("modules"))) {
        cmdArgs << "--setModules=" + args[QStringLiteral("modules")].toString();
    }

    if (cmdArgs.isEmpty()) {
        auto action = ActionReply::HelperErrorReply(STATUS_INVALID_ARGUMENTS);
        action.setErrorDescription(i18n("Invalid arguments passed to the profile"));
        return action;
    }

    checkFolder();
    return run(cmdArgs, {"--status", "--defaults", "--list", "--modules"}, cmd);
}

ActionReply Helper::saveProfile(const QVariantMap &args, const QString &cmd)
{
    QString name(args[QStringLiteral("name")].toString()), xml(args["xml"].toString());
    ActionReply reply;
    auto prepareData = [&] {
        reply.addData(QStringLiteral("cmd"), cmd);
        reply.addData(QStringLiteral("name"), name);
        reply.addData("profiles", QDir(KCM_UFW_DIR).entryList({"*" + PROFILE_EXTENSION}));
    };

    if (name.isEmpty() || xml.isEmpty()) {
        reply = ActionReply::HelperErrorReply(STATUS_INVALID_ARGUMENTS);
        prepareData();
        return reply;
    }

    checkFolder();

    QFile f(QString(KCM_UFW_DIR) + "/" + name + PROFILE_EXTENSION);

    if (!f.open(QIODevice::WriteOnly)) {
        reply = ActionReply::HelperErrorReply(STATUS_OPERATION_FAILED);
        reply.setErrorDescription(i18n("Error saving the profile."));
        prepareData();
        return reply;
    }

    QTextStream(&f) << xml;
    f.close();
    setPermissions(f.fileName(), FILE_PERMS);
    prepareData();
    return reply;
}

ActionReply Helper::deleteProfile(const QVariantMap &args, const QString &cmd)
{
    QString name(args[QStringLiteral("name")].toString());
    ActionReply reply;
    auto prepareData = [&] {
        reply.addData(QStringLiteral("cmd"), cmd);
        reply.addData(QStringLiteral("name"), name);
        reply.addData(QStringLiteral("profiles"), QDir(KCM_UFW_DIR).entryList({"*" + PROFILE_EXTENSION}));
    };

    if (name.isEmpty()) {
        reply = ActionReply::HelperErrorReply(STATUS_INVALID_ARGUMENTS);
        reply.setErrorDescription(i18n("Invalid arguments passed to delete profile"));
        prepareData();
        return reply;
    }

    if (!QFile::remove(QString(KCM_UFW_DIR) + "/" + name + PROFILE_EXTENSION)) {
        reply = ActionReply::HelperErrorReply(STATUS_OPERATION_FAILED);
        reply.setErrorDescription(i18n("Could not remove the profile from disk."));
        prepareData();
        return reply;
    }

    prepareData();
    return reply;
}

ActionReply Helper::addRules(const QVariantMap &args, const QString &cmd)
{
    int count = args[QStringLiteral("count")].toInt();

    if (count <= 0) {
        ActionReply reply = ActionReply::HelperErrorReply(STATUS_INVALID_ARGUMENTS);
        reply.setErrorDescription(i18n("Invalid argument passed to add Rules"));
        return reply;
    }
    QStringList cmdArgs;

    for (int i = 0; i < count; ++i) {
        cmdArgs << "--add=" + args["xml" + QString::number(i)].toString();
    }
    qDebug() << "Cmd args passed to ufw:" << cmdArgs;

    checkFolder();
    return run(cmdArgs, {"--list"}, cmd);
}

ActionReply Helper::removeRule(const QVariantMap &args, const QString &cmd)
{
    checkFolder();
    return run({"--remove=" + args["index"].toString()}, {"--list"}, cmd);
}

ActionReply Helper::moveRule(const QVariantMap &args, const QString &cmd)
{
    checkFolder();
    const QString from = QString::number(args[QStringLiteral("from")].toUInt());
    const QString to = QString::number(args[QStringLiteral("to")].toUInt());

    return run({"--move=" + from + ':' + to}, {"--list"}, cmd);
}

ActionReply Helper::editRule(const QVariantMap &args, const QString &cmd)
{
    checkFolder();

    qDebug() << args[QStringLiteral("xml")].toString();

    return run({"--update=" + args["xml"].toString()}, {"--list"}, cmd);
}

ActionReply Helper::reset(const QString &cmd)
{
    return run({"--reset"}, {"--status", "--defaults", "--list", "--modules"}, cmd);
}

ActionReply Helper::run(const QStringList &args, const QStringList &second, const QString &cmd)
{
    ActionReply reply = run(args, cmd);
    if (reply.errorCode() == EXIT_SUCCESS) {
        reply = run(second, cmd);
    }
    return reply;
}

ActionReply Helper::run(const QStringList &args, const QString &cmd)
{
    QProcess ufw;
    ActionReply reply;
    ufw.start(UFW_PLUGIN_HELPER_PATH, args, QIODevice::ReadOnly);
    if (ufw.waitForStarted()) {
        ufw.waitForFinished();
    }

    int exitCode(ufw.exitCode());

    if (exitCode != EXIT_SUCCESS) {
        QString errorString = ufw.readAllStandardError().simplified();

        const QString errorPrefix = QStringLiteral("ERROR: ");
        if (errorString.startsWith(errorPrefix)) {
            errorString = errorString.mid(errorPrefix.length());
        }

        reply = ActionReply::HelperErrorReply(exitCode);
        reply.setErrorDescription(i18n("An error occurred in command '%1': %2", cmd, errorString));
        reply.addData(QStringLiteral("cmd"), cmd);
        return reply;
    }

    // reply.addData(QStringLiteral("response"), ufw.readAllStandardOutput());
    QString output = ufw.readAllStandardOutput();
    qDebug() << "Command" << UFW_PLUGIN_HELPER_PATH << args << output;

    reply.addData("response", output);
    reply.addData(QStringLiteral("cmd"), cmd);
    return reply;
}

}

KAUTH_HELPER_MAIN("org.kde.ufw", UFW::Helper)
