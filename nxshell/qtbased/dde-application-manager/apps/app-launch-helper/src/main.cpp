// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <numeric>
#include <unordered_map>
#include <vector>
#include <deque>
#include <memory>
#include <iostream>
#include <algorithm>
#include <cstdlib>
#include <map>
#include <list>
#include <thread>
#include "constant.h"
#include "types.h"
#include "variantValue.h"

namespace {

ExitCode fromString(const std::string &str)
{
    if (str == "done") {
        return ExitCode::Done;
    }

    if (str == "canceled" or str == "timeout" or str == "failed" or str == "dependency" or str == "skipped") {
        return ExitCode::SystemdError;
    }

    if (str == "internalError") {
        return ExitCode::InternalError;
    }

    if (str == "invalidInput") {
        return ExitCode::InvalidInput;
    }

    __builtin_unreachable();
}

ExitCode fromString(const char *str)
{
    if (!str) {
        return ExitCode::Waiting;
    }
    std::string tmp{str};
    return fromString(tmp);
}

[[noreturn]] void releaseRes(sd_bus_error &error, msg_ptr &msg, bus_ptr &bus, ExitCode ret)
{
    sd_bus_error_free(&error);
    sd_bus_message_unref(msg);
    sd_bus_unref(bus);

    std::exit(static_cast<int>(ret));
}

int processExecStart(msg_ptr &msg, const std::deque<std::string_view> &execArgs)
{
    int ret;

    if (ret = sd_bus_message_open_container(msg, SD_BUS_TYPE_STRUCT, "sv"); ret < 0) {
        sd_journal_perror("open struct of ExecStart failed.");
        return ret;
    }

    if (ret = sd_bus_message_append(msg, "s", "ExecStart"); ret < 0) {
        sd_journal_perror("append ExecStart failed.");
        return ret;
    }

    if (ret = sd_bus_message_open_container(msg, SD_BUS_TYPE_VARIANT, "a(sasb)"); ret < 0) {
        sd_journal_perror("open variant of execStart failed.");
        return ret;
    }

    if (ret = sd_bus_message_open_container(msg, SD_BUS_TYPE_ARRAY, "(sasb)"); ret < 0) {
        sd_journal_perror("open array of execStart failed.");
        return ret;
    }

    if (ret = sd_bus_message_open_container(msg, SD_BUS_TYPE_STRUCT, "sasb"); ret < 0) {
        sd_journal_perror("open struct of execStart failed.");
        return ret;
    }

    if (ret = sd_bus_message_append(msg, "s", execArgs[0].data()); ret < 0) {
        sd_journal_perror("append binary of execStart failed.");
        return ret;
    }

    if (ret = sd_bus_message_open_container(msg, SD_BUS_TYPE_ARRAY, "s"); ret < 0) {
        sd_journal_perror("open array of execStart variant failed.");
        return ret;
    }

    for (auto execArg : execArgs) {
        if (ret = sd_bus_message_append(msg, "s", execArg.data()); ret < 0) {
            sd_journal_perror("append args of execStart failed.");
            return ret;
        }
    }

    if (ret = sd_bus_message_close_container(msg); ret < 0) {
        sd_journal_perror("close array of execStart failed.");
        return ret;
    }

    if (ret = sd_bus_message_append(msg, "b", 0);
        ret < 0) {  // this value indicate that systemd should be considered a failure if the process exits uncleanly
        sd_journal_perror("append boolean of execStart failed.");
        return ret;
    }

    if (ret = sd_bus_message_close_container(msg); ret < 0) {
        sd_journal_perror("close struct of execStart failed.");
        return ret;
    }

    if (ret = sd_bus_message_close_container(msg); ret < 0) {
        sd_journal_perror("close array of execStart failed.");
        return ret;
    }

    if (ret = sd_bus_message_close_container(msg); ret < 0) {
        sd_journal_perror("close variant of execStart failed.");
        return ret;
    }

    if (ret = sd_bus_message_close_container(msg); ret < 0) {
        sd_journal_perror("close struct of execStart failed.");
        return ret;
    }

    return 0;
}

DBusValueType getPropType(std::string_view key)
{
    static std::unordered_map<std::string_view, DBusValueType> map{{"Environment", DBusValueType::ArrayOfString},
                                                                   {"UnsetEnvironment", DBusValueType::ArrayOfString},
                                                                   {"WorkingDirectory", DBusValueType::String},
                                                                   {"ExecSearchPath", DBusValueType::ArrayOfString}};

    if (auto it = map.find(key); it != map.cend()) {
        return it->second;
    }

    return DBusValueType::String;  // fallback to string
}

int appendPropValue(msg_ptr &msg, DBusValueType type, const std::list<std::string_view> &value)
{
    int ret;

    auto handler = creatValueHandler(msg, type);
    if (handler == nullptr) {
        sd_journal_perror("unknown type of property's variant.");
        return -1;
    }

    if (ret = handler->openVariant(); ret < 0) {
        sd_journal_perror("open property's variant value failed.");
        return ret;
    }

    for (const auto &v : value) {
        if (ret = handler->appendValue(std::string{v}); ret < 0) {
            sd_journal_perror("append property's variant value failed.");
            return ret;
        }
    }

    if (ret = handler->closeVariant(); ret < 0) {
        sd_journal_perror("close property's variant value failed.");
        return ret;
    }

    return 0;
}

int processKVPair(msg_ptr &msg, const std::map<std::string_view, std::list<std::string_view>> &props)
{
    int ret;
    if (!props.empty()) {
        for (auto [key, value] : props) {
            std::string keyStr{key};
            if (ret = sd_bus_message_open_container(msg, SD_BUS_TYPE_STRUCT, "sv"); ret < 0) {
                sd_journal_perror("open struct of properties failed.");
                return ret;
            }

            if (ret = sd_bus_message_append(msg, "s", keyStr.data()); ret < 0) {
                sd_journal_perror("append key of property failed.");
                return ret;
            }

            if (ret = appendPropValue(msg, getPropType(key), value); ret < 0) {
                sd_journal_perror("append value of property failed.");
                return ret;
            }

            if (ret = sd_bus_message_close_container(msg); ret < 0) {
                sd_journal_perror("close struct of properties failed.");
                return ret;
            }
        }
    }
    return 0;
}

std::string cmdParse(msg_ptr &msg, std::deque<std::string_view> cmdLines)
{
    std::string serviceName{"internalError"};
    std::map<std::string_view, std::list<std::string_view>> props;
    while (!cmdLines.empty()) {  // NOTE: avoid stl exception
        auto str = cmdLines.front();
        if (str.size() < 2) {
            sd_journal_print(LOG_WARNING, "invalid option %s.", str.data());
            cmdLines.pop_front();
            continue;
        }
        if (str.substr(0, 2) != "--") {
            sd_journal_print(LOG_INFO, "unknown option %s.", str.data());
            cmdLines.pop_front();
            continue;
        }

        auto kvStr = str.substr(2);
        if (!kvStr.empty()) {
            const auto *it = kvStr.cbegin();
            if (it = std::find(it, kvStr.cend(), '='); it == kvStr.cend()) {
                sd_journal_print(LOG_WARNING, "invalid k-v pair: %s", kvStr.data());
                cmdLines.pop_front();
                continue;
            }
            auto splitIndex = std::distance(kvStr.cbegin(), it);
            if (++it == kvStr.cend()) {
                sd_journal_print(LOG_WARNING, "invalid k-v pair: %s", kvStr.data());
                cmdLines.pop_front();
                continue;
            }

            auto key = kvStr.substr(0, splitIndex);
            if (key == "Type") {
                // NOTE:
                // Systemd service type must be "exec",
                // this should not be configured in command line arguments.
                cmdLines.pop_front();
                continue;
            }
            props[key].push_back(kvStr.substr(splitIndex + 1));
            cmdLines.pop_front();
            continue;
        }

        cmdLines.pop_front();  // NOTE: skip "--"
        break;
    }

    // Processing of the binary file and its parameters that am want to launch
    auto &execArgs = cmdLines;
    if (execArgs.empty()) {
        sd_journal_print(LOG_ERR, "param exec is empty.");
        serviceName = "invalidInput";
        return serviceName;
    }
    if (props.find("unitName") == props.cend()) {
        sd_journal_perror("unitName doesn't exists.");
        serviceName = "invalidInput";
        return serviceName;
    }

    int ret;
    if (ret = sd_bus_message_append(msg, "s", props["unitName"].front().data()); ret < 0) {  // unitName
        sd_journal_perror("append unitName failed.");
        return serviceName;
    }

    serviceName = props["unitName"].front();
    props.erase("unitName");

    if (ret = sd_bus_message_append(msg, "s", "replace"); ret < 0) {  // start mode
        sd_journal_perror("append startMode failed.");
        return serviceName;
    }

    // process properties: a(sv)
    if (ret = sd_bus_message_open_container(msg, SD_BUS_TYPE_ARRAY, "(sv)"); ret < 0) {
        sd_journal_perror("open array failed.");
        return serviceName;
    }

    if (ret = sd_bus_message_append(msg, "(sv)", "Type", "s", "exec"); ret < 0) {
        sd_journal_perror("append type failed.");
        return serviceName;
    }

    if (ret = sd_bus_message_append(msg, "(sv)", "ExitType", "s", "cgroup"); ret < 0) {
        sd_journal_perror("append exit type failed.");
        return serviceName;
    }

    if (ret = sd_bus_message_append(msg, "(sv)", "Slice", "s", "app.slice"); ret < 0) {
        sd_journal_perror("append application slice failed.");
        return serviceName;
    }

    if (ret = sd_bus_message_append(msg, "(sv)", "CollectMode", "s", "inactive-or-failed"); ret < 0) {
        sd_journal_perror("append application slice failed.");
        return serviceName;
    }

    if (ret = processKVPair(msg, props); ret < 0) {  // process props
        serviceName = "invalidInput";
        return serviceName;
    }

    if (ret = processExecStart(msg, execArgs); ret < 0) {
        serviceName = "invalidInput";
        return serviceName;
    }

    if (ret = sd_bus_message_close_container(msg); ret < 0) {
        sd_journal_perror("close array failed.");
        return serviceName;
    }

    // append aux, it's unused for now
    if (ret = sd_bus_message_append(msg, "a(sa(sv))", 0); ret < 0) {
        sd_journal_perror("append aux failed.");
        return serviceName;
    }

    return serviceName;
}

int jobRemovedReceiver(sd_bus_message *m, void *userdata, sd_bus_error *ret_error)
{
    int ret;
    if (ret = sd_bus_error_is_set(ret_error); ret != 0) {
        sd_journal_print(LOG_ERR, "JobRemoved error: [%s,%s]", ret_error->name, ret_error->message);
    } else {
        const char *serviceId{nullptr};
        const char *jobResult{nullptr};
        if (ret = sd_bus_message_read(m, "uoss", nullptr, nullptr, &serviceId, &jobResult); ret < 0) {
            sd_journal_perror("read from JobRemoved failed.");
        } else {
            auto *ptr = reinterpret_cast<JobRemoveResult *>(userdata);
            if (ptr->id == serviceId) {
                ptr->removedFlag = 1;
                ptr->result = fromString(jobResult);
            }
        }
    }

    if (ret_error and ret_error->_need_free) {
        sd_bus_error_free(ret_error);
    }

    return ret;
}

int process_dbus_message(sd_bus *bus)
{
    int ret;
    ret = sd_bus_process(bus, nullptr);
    if (ret < 0) {
        sd_journal_print(LOG_ERR, "event loop error.");
        return ret;
    }

    if (ret > 0) {
        return 0;
    }

    ret = sd_bus_wait(bus, std::numeric_limits<uint64_t>::max());
    if (ret < 0) {
        sd_journal_print(LOG_ERR, "sd-bus event loop error:%s", strerror(-ret));
        return ret;
    }

    return ret;
}

}  // namespace

int main(int argc, const char *argv[])
{
    sd_bus_error error{SD_BUS_ERROR_NULL};
    sd_bus_message *msg{nullptr};
    sd_bus *bus{nullptr};
    std::string serviceId;
    int ret;

    if (ret = sd_bus_open_user(&bus); ret < 0) {
        sd_journal_perror("Failed to connect to user bus.");
        releaseRes(error, msg, bus, ExitCode::InternalError);
    }

    if (ret = sd_bus_message_new_method_call(
            bus, &msg, SystemdService, SystemdObjectPath, SystemdInterfaceName, "StartTransientUnit");
        ret < 0) {
        sd_journal_perror("Failed to create D-Bus call message");
        releaseRes(error, msg, bus, ExitCode::InternalError);
    }

    std::deque<std::string_view> args;
    for (int i = 1; i < argc; ++i) {
        args.emplace_back(argv[i]);
    }

    serviceId = cmdParse(msg, std::move(args));
    if (serviceId == "internalError") {
        releaseRes(error, msg, bus, ExitCode::InternalError);
    } else if (serviceId == "invalidInput") {
        releaseRes(error, msg, bus, ExitCode::InvalidInput);
    }

    const char *path{nullptr};
    JobRemoveResult resultData{serviceId};

    if (ret = sd_bus_match_signal(
            bus, nullptr, SystemdService, SystemdObjectPath, SystemdInterfaceName, "JobRemoved", jobRemovedReceiver, &resultData);
        ret < 0) {
        sd_journal_perror("add signal matcher failed.");
        releaseRes(error, msg, bus, ExitCode::InternalError);
    }

    sd_bus_message *reply{nullptr};
    if (ret = sd_bus_call(bus, msg, 0, &error, &reply); ret < 0) {
        sd_bus_error_get_errno(&error);
        sd_journal_print(LOG_ERR, "failed to call StartTransientUnit: [%s,%s]", error.name, error.message);
        releaseRes(error, msg, bus, ExitCode::InternalError);
    } else {
        sd_journal_print(LOG_INFO, "call StartTransientUnit successfully, service ID: %s", serviceId.c_str());
    }

    if (ret = sd_bus_message_read(reply, "o", &path); ret < 0) {
        sd_journal_perror("failed to parse response message.");
        sd_bus_message_unref(reply);
        releaseRes(error, msg, bus, ExitCode::InternalError);
    }

    sd_bus_message_unref(reply);

    while (true) {  // wait for jobRemoved
        int ret = process_dbus_message(bus);
        if (ret < 0) {
            releaseRes(error, msg, bus, ExitCode::InternalError);
        }

        if (resultData.removedFlag) {
            break;
        }
    }

    releaseRes(error, msg, bus, resultData.result);
}
