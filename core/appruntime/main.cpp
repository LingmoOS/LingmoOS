// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "appruntime.h"

#include "base/dbus.h"
#include "base/event_loop.h"
#include "base/logging.h"

#include <systemd/sd-bus.h>
#include <cstring>

#include <QString>
#include <QStringList>

namespace dbus = Lingmo::Base::DBus;

static constexpr char kService[] = "org.lingmo.AppRuntime";
static constexpr char kPath[]    = "/AppRuntime";

// launchApplication and launchCommand also come without an activation token.
// A method table holds one signature per name, so these shorter forms are
// answered here, before the table sees them.
static int handleShortForms(sd_bus_message *message, void *userdata, sd_bus_error *)
{
    if (!sd_bus_message_is_method_call(message, kService, nullptr))
        return 0;

    auto *runtime = static_cast<AppRuntime *>(userdata);
    const dbus::Message call(message);
    const char *member = sd_bus_message_get_member(message);
    uint32_t pid = 0;

    if (std::strcmp(member, "launchApplication") == 0 && call.signature() == "sas") {
        std::string appId;
        std::vector<std::string> arguments;
        if (!call.read(appId, arguments))
            return 0;
        pid = runtime->launchApplication(QString::fromStdString(appId), toQStringList(arguments));
    } else if (std::strcmp(member, "launchCommand") == 0 && call.signature() == "sass") {
        std::string appId;
        std::vector<std::string> command;
        std::string workingDirectory;
        if (!call.read(appId, command, workingDirectory))
            return 0;
        pid = runtime->launchCommand(QString::fromStdString(appId), toQStringList(command),
                                     QString::fromStdString(workingDirectory));
    } else {
        return 0;
    }

    sd_bus_reply_method_return(message, "u", pid);
    return 1;
}

int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    Lingmo::Base::EventLoop loop;
    static const Lingmo::Base::LogCategory log("lingmo.appruntime");

    dbus::Connection *bus = dbus::Connection::session(&loop);
    if (!bus) {
        LM_WARNING(log) << "Cannot connect to the session bus";
        return 1;
    }

    AppRuntime runtime;

    dbus::Object object(bus, kPath, kService);
    object.addMethod(
        "launchApplication",
        [&runtime](const std::string &appId, const std::vector<std::string> &arguments,
                   const std::string &activationToken) {
            return runtime.launchApplication(QString::fromStdString(appId), toQStringList(arguments),
                                             QString::fromStdString(activationToken));
        },
        {{"appId", "arguments", "activationToken"}, {"pid"}});
    object.addMethod(
        "launchCommand",
        [&runtime](const std::string &appId, const std::vector<std::string> &command,
                   const std::string &workingDirectory, const std::string &activationToken) {
            return runtime.launchCommand(QString::fromStdString(appId), toQStringList(command),
                                         QString::fromStdString(workingDirectory),
                                         QString::fromStdString(activationToken));
        },
        {{"appId", "command", "workingDirectory", "activationToken"}, {"pid"}});
    object.addMethod(
        "quitApplication",
        [&runtime](const std::string &appId) {
            return runtime.quitApplication(QString::fromStdString(appId));
        },
        {{"appId"}, {"result"}});
    object.addMethod("quitAll", [&runtime] { return runtime.quitAll(); }, {{}, {"result"}});
    object.addMethod(
        "quitByPid",
        [&runtime](uint32_t pid) { return runtime.quitByPid(pid); },
        {{"pid"}, {"result"}});
    object.addMethod(
        "forceQuitByPid",
        [&runtime](uint32_t pid) { return runtime.forceQuitByPid(pid); },
        {{"pid"}, {"result"}});
    object.addMethod(
        "isRunning",
        [&runtime](const std::string &appId) {
            return runtime.isRunning(QString::fromStdString(appId));
        },
        {{"appId"}, {"result"}});
    object.addMethod(
        "runningApplications",
        [&runtime] { return toStdStrings(runtime.runningApplications()); },
        {{}, {"appIds"}});
    object.addMethod(
        "pidsForApplication",
        [&runtime](const std::string &appId) {
            return toStdVector(runtime.pidsForApplication(QString::fromStdString(appId)));
        },
        {{"appId"}, {"pids"}});
    object.addSignal("applicationLaunched", "su", {"appId", "pid"});
    object.addSignal("applicationQuit",     "su", {"appId", "pid"});
    object.publish();

    sd_bus_slot *shortForms = nullptr;
    sd_bus_add_object(bus->bus(), &shortForms, kPath, handleShortForms, &runtime);
    const dbus::Slot shortFormsSlot(shortForms);

    const auto launched = runtime.applicationLaunched.connect(
        [&object](const QString &appId, uint32_t pid) {
            object.emitSignal("applicationLaunched", appId.toStdString(), pid);
        });
    const auto quit = runtime.applicationQuit.connect(
        [&object](const QString &appId, uint32_t pid) {
            object.emitSignal("applicationQuit", appId.toStdString(), pid);
        });

    if (!bus->requestName(kService)) {
        LM_WARNING(log) << "Another application runtime is already running";
        return 1;
    }

    return loop.run();
}
