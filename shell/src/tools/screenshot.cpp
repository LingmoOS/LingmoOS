// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <memory>
#include <string>
#include <vector>
#include <wayland-client.h>

#include "base/command_line.h"
#include "base/dbus.h"
#include "base/event_loop.h"
#include "base/file.h"

namespace {

namespace base = Lingmo::base;
namespace dbus = Lingmo::base::dbus;

struct Output {
    wl_output *proxy = nullptr;
    std::string name;
};

const wl_output_listener outputListener = {
        [](void *, wl_output *, int32_t, int32_t, int32_t, int32_t, int32_t, const char *, const char *,
           int32_t) {},
        [](void *, wl_output *, uint32_t, int32_t, int32_t, int32_t) {},
        [](void *, wl_output *) {},
        [](void *, wl_output *, int32_t) {},
        [](void *data, wl_output *, const char *name) {
            static_cast<Output *>(data)->name = name;
        },
        [](void *, wl_output *, const char *) {}};

const wl_registry_listener registryListener = {
        [](void *data, wl_registry *registry, uint32_t name, const char *interface, uint32_t version) {
            if (strcmp(interface, wl_output_interface.name) != 0)
                return;
            auto output = std::make_unique<Output>();
            output->proxy = static_cast<wl_output *>(
                    wl_registry_bind(registry, name, &wl_output_interface, std::min(version, 4u)));
            wl_output_add_listener(output->proxy, &outputListener, output.get());
            static_cast<std::vector<std::unique_ptr<Output>> *>(data)->push_back(std::move(output));
        },
        [](void *, wl_registry *, uint32_t) {}};

// The output names, from the wl_output globals every client sees.
bool listOutputs()
{
    wl_display *display = wl_display_connect(nullptr);
    if (!display)
        return false;
    std::vector<std::unique_ptr<Output>> outputs;
    wl_registry *registry = wl_display_get_registry(display);
    wl_registry_add_listener(registry, &registryListener, &outputs);
    const bool ok = wl_display_roundtrip(display) >= 0 && wl_display_roundtrip(display) >= 0;
    for (const auto &output : outputs) {
        printf("%s\n", output->name.c_str());
        wl_output_destroy(output->proxy);
    }
    wl_registry_destroy(registry);
    wl_display_disconnect(display);
    return ok;
}

} // namespace

// The picture itself is the shell's to take: the compositor lets no program
// read the screen merely for running this one.
int main(int argc, char **argv)
{
    base::CommandLineParser parser;
    parser.setApplicationDescription("Save a picture of an output to PNG, taken by the shell.");
    parser.addHelpOption();
    parser.addOption({"list", "List output names."});
    parser.addOption({"output", "Output to capture (required with multiple outputs).", "name"});
    parser.addOption({"cursor", "Include the pointer cursor."});
    parser.addPositionalArgument("file", "Destination PNG file.", "[file]");
    parser.process(argc, argv);
    const auto fail = [](const std::string &message) {
        fprintf(stderr, "%s\n", message.c_str());
        return 1;
    };
    if (parser.isSet("list"))
        return listOutputs() ? 0 : fail("Cannot connect to Wayland display");
    if (parser.positionalArguments().size() != 1)
        return fail("Specify one destination PNG file");

    base::EventLoop loop;
    std::unique_ptr<dbus::Connection> bus = dbus::Connection::open(dbus::Connection::Bus::Session, &loop);
    if (!bus)
        return fail("Cannot connect to the session bus");
    dbus::Proxy shell(bus.get(), "org.lingmo.Screenshot", "/Screenshot", "org.lingmo.Screenshot");
    shell.setTimeout(std::chrono::seconds(30));
    const dbus::Reply reply = shell.call("captureOutput", parser.value("output"), parser.isSet("cursor"));
    if (reply.isError())
        return fail(reply.error().message.empty() ? reply.error().name : reply.error().message);
    const std::optional<std::vector<uint8_t>> png = reply.value<std::vector<uint8_t>>();
    if (!png || png->empty())
        return fail("The shell sent no picture");
    if (!base::file::writeAtomically(parser.positionalArguments().front(),
                                     std::string(png->begin(), png->end())))
        return fail("Cannot save PNG");
    return 0;
}
