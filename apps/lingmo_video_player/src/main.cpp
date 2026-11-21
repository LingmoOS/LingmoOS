// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "application.h"
#include "urls.h"
#include "views/mainwindow.h"
#include "views/widgets.h"

#include "base/command_line.h"
#include "base/dbus.h"
#include "base/i18n.h"
#include "base/paths.h"
#include "ui/application.h"

#include <chrono>
#include <filesystem>

using namespace Lingmo;
using namespace std::chrono_literals;

int main(int argc, char** argv)
{
    base::CommandLineParser parser;
    parser.setApplicationDescription("Lingmo Video Player");
    parser.setApplicationVersion(VIDEO_PLAYER_VERSION);
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("files", "Files or URLs to play.", "[files...]");
    parser.process(argc, argv);

    ui::Application app(argc, argv);
    app.setApplicationId("lingmo-video-player");
    i18n::setDomain("lingmo-video-player");
    views::registerGlyphs();

    std::error_code error;
    const std::string directory = std::filesystem::current_path(error).string();
    std::vector<std::string> arguments;
    for (const std::string& argument : parser.positionalArguments())
        arguments.push_back(urls::fromUserInput(argument, directory, true));

    // The object goes up before the name, so a second launch that finds the
    // name always finds someone to hand its files to.
    Application application(base::paths::lingmoConfig("video-player"));
    base::dbus::Connection* bus = app.sessionBus();
    if (bus) {
        application.publish(bus);
        if (!bus->requestName(Application::kService)) {
            base::dbus::Proxy running(bus, Application::kService, Application::kPath, Application::kService);
            running.setTimeout(3s);
            // A player that does not answer gets a window of our own instead.
            if (!running.call("open", arguments, app.takeActivationToken()).isError())
                return 0;
        }
    }

    for (const std::string& url : arguments) {
        std::error_code fileError;
        if (urls::isLocalFile(url) && std::filesystem::is_regular_file(urls::toLocalFile(url), fileError)
                && !Application::isSubtitle(url)) {
            application.setLaunchVideoSize(Player::probeVideoSize(url, 1000ms));
            break;
        }
    }

    MainWindow window(application);
    window.show();
    if (!arguments.empty())
        application.open(arguments);
    return app.run();
}
