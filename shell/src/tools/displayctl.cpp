// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <cmath>
#include <cstdio>
#include <limits>
#include <map>
#include <optional>
#include <string>
#include <vector>

#include "base/command_line.h"
#include "base/dbus.h"
#include "base/event_loop.h"
#include "base/file.h"
#include "base/json.h"
#include "base/strings.h"
#include "base/timer.h"
#include "ipc/display_dbus_types.h"

namespace ipc = Lingmo::Display::ipc;
namespace base = Lingmo::base;
namespace dbus = Lingmo::base::dbus;
using base::json::Value;

namespace {

void fail(const std::string &message)
{
    std::fprintf(stderr, "%s\n", message.c_str());
}

// Members in sorted order, as the output has always listed them.
Value modeJson(const ipc::Mode &mode)
{
    return Value::object({{"current", mode.current},
                          {"height", mode.height},
                          {"name", mode.name},
                          {"preferred", mode.preferred},
                          {"refreshMilliHz", mode.refreshMilliHz},
                          {"width", mode.width}});
}

bool printState(const dbus::Reply &reply)
{
    std::string instanceId;
    std::string seatId;
    uint64_t generation = 0;
    uint64_t revision = 0;
    std::vector<ipc::PhysicalDisplay> physicalDisplays;
    std::vector<ipc::LogicalDisplay> logicalDisplays;
    std::vector<ipc::Placement> placements;
    std::string previewToken;
    uint64_t previewDeadline = 0;
    if (!reply.read(instanceId, seatId, generation, revision, physicalDisplays, logicalDisplays,
                    placements, previewToken, previewDeadline))
        return false;
    Value::Array physical;
    for (const auto &display : physicalDisplays) {
        Value::Array modes;
        for (const auto &mode : display.modes)
            modes.push_back(modeJson(mode));
        physical.push_back(Value::object({{"connector", display.connectorName},
                                          {"id", display.sessionId},
                                          {"make", display.make},
                                          {"model", display.model},
                                          {"modes", Value(std::move(modes))},
                                          {"persistentKey", display.persistentKey}}));
    }
    Value::Array logical;
    for (const auto &display : logicalDisplays) {
        logical.push_back(Value::object({{"height", display.height},
                                         {"id", display.id},
                                         {"name", display.waylandName},
                                         {"primary", display.primary},
                                         {"rotation", int(display.rotation)},
                                         {"scale120", int(display.scale120)},
                                         {"width", display.width},
                                         {"x", display.x},
                                         {"y", display.y}}));
    }
    const Value root = Value::object(
            {{"appliedRevision", base::strings::number(revision)},
             {"instanceId", instanceId},
             {"logicalDisplays", Value(std::move(logical))},
             {"physicalDisplays", Value(std::move(physical))},
             {"previewDeadlineMs", base::strings::number(previewDeadline)},
             {"previewToken", previewToken},
             {"seatId", seatId},
             {"topologyGeneration", base::strings::number(generation)}});
    std::printf("%s", root.serialize(false).c_str());
    return true;
}

// A whole number in int's range, else the fallback.
int intValue(const Value &value, int fallback = 0)
{
    if (!value.isNumber())
        return fallback;
    const double number = value.toDouble();
    if (!std::isfinite(number) || number != std::trunc(number)
        || number < double(std::numeric_limits<int>::min())
        || number > double(std::numeric_limits<int>::max()))
        return fallback;
    return int(number);
}

// A number, or a string of one as `list` prints them.
uint64_t unsignedValue(const Value &value)
{
    if (value.isString())
        return base::strings::toNumber<uint64_t>(value.toString()).value_or(0);
    return value.isNumber() && value.toDouble() > 0 ? uint64_t(std::llround(value.toDouble()))
                                                    : 0;
}

struct Configuration {
    std::string instanceId;
    uint64_t generation = 0;
    uint64_t revision = 0;
    std::vector<ipc::DisplayRequest> displays;
    std::string primary;
    std::vector<ipc::Placement> placements;
    ipc::MirrorGroups mirrorGroups;
};

std::optional<Configuration> readConfiguration(const std::string &path)
{
    const auto text = base::file::read(path);
    const auto document = text ? Value::parse(*text) : std::nullopt;
    if (!document)
        return std::nullopt;
    const Value &root = *document;
    Configuration result;
    result.instanceId = root["instanceId"].toString();
    result.generation = unsignedValue(root["topologyGeneration"]);
    result.revision = unsignedValue(root["appliedRevision"]);
    result.primary = root["primary"].toString();
    for (const auto &item : root["displays"].toArray()) {
        const Value &mode = item["mode"];
        ipc::DisplayRequest request;
        request.id = item["id"].toString();
        request.enabled = item["enabled"].toBool(true);
        request.scale120 = uint32_t(intValue(item["scale120"], 120));
        request.rotation = uint32_t(intValue(item["rotation"]));
        request.mode = {intValue(mode["width"]),
                        intValue(mode["height"]),
                        intValue(mode["refreshMilliHz"]),
                        intValue(mode["pixelClockKHz"]),
                        intValue(mode["horizontalTotal"]),
                        intValue(mode["verticalTotal"]),
                        uint32_t(intValue(mode["flags"])),
                        mode["preferred"].toBool(),
                        mode["name"].toString()};
        result.displays.push_back(request);
    }
    for (const auto &item : root["placements"].toArray()) {
        result.placements.push_back({item["display"].toString(), item["parent"].toString(),
                                     uint32_t(intValue(item["edge"])), intValue(item["offset"])});
    }
    for (const auto &value : root["mirrorGroups"].toArray()) {
        std::vector<std::string> group;
        for (const auto &id : value.toArray())
            group.push_back(id.toString());
        result.mirrorGroups.push_back(group);
    }
    return result;
}

} // namespace

int main(int argc, char **argv)
{
    base::CommandLineParser parser;
    parser.addHelpOption();
    parser.addOption({"service", "D-Bus service name", "name", "org.lingmo.DisplayConfig"});
    parser.addPositionalArgument("command", "list, test, apply, confirm, revert, reset, or power");
    parser.addPositionalArgument("arguments", "Command arguments", "[arguments...]");
    parser.process(argc, argv);
    const std::vector<std::string> args = parser.positionalArguments();
    if (args.empty())
        parser.showHelp(2);

    base::EventLoop loop;
    dbus::Connection *bus = dbus::Connection::session(&loop);
    if (!bus) {
        fail("Display configuration service is unavailable");
        return 1;
    }
    const dbus::Proxy service(bus, parser.value("service"),
                              "/org/lingmo/DisplayConfig1", "org.lingmo.DisplayConfig1");

    const std::string &command = args.front();
    std::string powerToken;
    std::map<std::string, std::pair<bool, std::string>> powerResults;
    const auto finishPower = [&loop](const std::pair<bool, std::string> &result) {
        if (!result.first)
            fail("Display power operation failed: " + result.second);
        loop.quit(result.first ? 0 : 1);
    };
    dbus::Match operations;
    base::Timer timeout(&loop);
    if (command == "power") {
        operations = service.onSignal("OperationFinished", [&](const dbus::Message &message) {
            std::string token;
            bool succeeded = false;
            bool degraded = false;
            uint32_t code = 0;
            std::string text;
            if (!message.read(token, succeeded, degraded, code, text))
                return;
            if (powerToken.empty())
                powerResults.insert_or_assign(token, std::pair(succeeded, text));
            else if (powerToken == token)
                finishPower({succeeded, text});
        });
        timeout.setSingleShot(true);
        timeout.onTimeout = [&loop] {
            fail("Timed out waiting for display power operation");
            loop.quit(1);
        };
        timeout.start(std::chrono::seconds(15));
    }

    const auto finished = [&](const dbus::Reply &reply) {
        if (reply.isError()) {
            fail(reply.error().name + " " + reply.error().message);
            loop.quit(1);
        } else if (command == "power") {
            reply.read(powerToken);
            if (powerToken.empty()) {
                fail("Display power request returned no operation token");
                loop.quit(1);
            } else if (const auto result = powerResults.find(powerToken);
                       result != powerResults.end()) {
                finishPower(result->second);
            }
        } else {
            std::string text;
            if (command == "list")
                printState(reply);
            else if (reply.read(text))
                std::printf("%s\n", text.c_str());
            loop.quit(0);
        }
    };
    dbus::PendingCall call;
    if (command == "list") {
        call = service.callAsync("GetState", finished);
    } else if ((command == "test" || command == "apply") && args.size() == 2) {
        const auto configuration = readConfiguration(args[1]);
        if (!configuration) {
            fail("Could not read the configuration JSON");
            return 2;
        }
        call = service.callAsync(command == "test" ? "TestConfiguration" : "ApplyConfiguration",
                                 finished, configuration->instanceId, configuration->generation,
                                 configuration->revision, configuration->displays,
                                 configuration->primary, configuration->placements,
                                 configuration->mirrorGroups);
    } else if (command == "confirm" && args.size() == 2) {
        call = service.callAsync("ConfirmConfiguration", finished, args[1]);
    } else if (command == "revert" && args.size() == 2) {
        call = service.callAsync("RevertConfiguration", finished, args[1]);
    } else if (command == "power" && args.size() >= 2) {
        const std::string &state = args[1];
        const uint32_t value = state == "on" ? 0 : state == "off" ? 1 : state == "suspend" ? 2 : 99;
        if (value == 99) {
            fail("Power state must be on, off, or suspend");
            return 2;
        }
        call = service.callAsync("SetDisplayPower", finished,
                                 std::vector<std::string>(args.begin() + 2, args.end()), value);
    } else {
        fail("Invalid command or arguments");
        return 2;
    }
    return loop.run();
}
