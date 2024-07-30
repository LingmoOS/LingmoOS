/*
 *  SPDX-FileCopyrightText: 2014-2015 Sebastian Kügler <sebas@kde.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "doctor.h"

#include <QCommandLineParser>
#include <QGuiApplication>

#include <QDebug>

/** Usage example:
 * kscreen-doctor --set output.0.disable output.1.mode.1 output.1.enable"
 *
 * Error codes:
 * 2 : general parse error
 * 3 : output id parse error
 * 4 : mode id parse error
 * 5 : position parse error
 *
 * 8 : invalid output id
 * 9 : invalid mode id
 *
 */

int main(int argc, char **argv)
{
    const QString desc = QStringLiteral(
        "kscreen-doctor allows to change the screen setup from the command-line.\n"
        "\n"
        "Setting the output configuration is done in an atomic fashion, all settings\n"
        "are applied in a single command.\n"
        "kscreen-doctor can be used to enable and disable outputs, to position screens,\n"
        "change resolution (mode setting), etc.. You should put all your options into \n"
        "a single invocation of kscreen-doctor, so they can all be applied at once.\n"
        "\n"
        "Usage examples:\n\n"
        "   Show output information:\n"
        "   $ kscreen-doctor -o\n"
        "   Output: 1 eDP-1 enabled connected Panel Modes: Modes: 1:800x600@60 [...] Geometry: 0,0 1280x800\n"
        "   Output: 70 HDMI-2 enabled connected  HDMI Modes: 1:800x600@60 [...] Geometry: 1280,0 1920x1080\n"
        "\n   Disable the hdmi output, enable the laptop panel and set it to a specific mode\n"
        "   $ kscreen-doctor output.HDMI-2.disable output.eDP-1.mode.1 output.eDP-1.enable\n"
        "\n   Position the hdmi monitor on the right of the laptop panel\n"
        "   $ kscreen-doctor output.HDMI-2.position.1280,0 output.eDP-1.position.0,0\n"
        "\n   Set resolution mode\n"
        "   $ kscreen-doctor output.HDMI-2.mode.1920x1080@60 \n"
        "\n   Set scale (note: fractional scaling is only supported on wayland)\n"
        "   $ kscreen-doctor output.HDMI-2.scale.2 \n"
        "\n   Set rotation (possible values: none, left, right, inverted)\n"
        "   $ kscreen-doctor output.HDMI-2.rotation.left \n"
        "\n   Set HDR mode (possible values: enable, disable)\n"
        "   $ kscreen-doctor output.HDMI-2.hdr.enable\n"
        "\n   Set SDR brightness (possible values: 100-1000)\n"
        "   $ kscreen-doctor output.HDMI-2.sdr-brightness.300\n"
        "\n   Set wide color gamut mode (possible values: enable, disable)\n"
        "   $ kscreen-doctor output.HDMI-2.wcg.enable\n"
        "\n   Set ICC profile path\n"
        "   $ kscreen-doctor output.HDMI-2.iccprofile.\"/path/to/profile.icc\"\n");
    /*
        "\nError codes:\n"
        "   2 : general parse error\n"
        "   3 : output id parse error\n"
        "   4 : mode id parse error\n"
        "   5 : position parse error\n"

        "   8 : invalid output id\n"
        "   9 : invalid mode id\n";
    */
    const QString syntax = QStringLiteral(
        "Specific output settings are separated by spaces, each setting is in the form of\n"
        "output.<name>.<setting>[.<value>]\n"
        "For example:\n"
        "$ kscreen-doctor output.HDMI-2.enable \\ \n"
        "                output.eDP-1.mode.4 \\ \n"
        "                output.eDP-1.position.1280,0\n"
        "Multiple settings are passed in order to have kscreen-doctor apply these settings in one go.\n");

    QGuiApplication::setDesktopSettingsAware(false);
    QGuiApplication app(argc, argv);

    KScreen::Doctor server;

    QCommandLineOption info =
        QCommandLineOption(QStringList() << QStringLiteral("i") << QStringLiteral("info"), QStringLiteral("Show runtime information: backends, logging, etc."));
    QCommandLineOption outputs = QCommandLineOption(QStringList() << QStringLiteral("o") << QStringLiteral("outputs"), QStringLiteral("Show outputs"));
    QCommandLineOption json =
        QCommandLineOption(QStringList() << QStringLiteral("j") << QStringLiteral("json"), QStringLiteral("Show configuration in JSON format"));
    QCommandLineOption dpms = QCommandLineOption(QStringList() << QStringLiteral("d") << QStringLiteral("dpms"),
                                                 QStringLiteral("Display power management (wayland only)"),
                                                 QStringLiteral("off"));
    QCommandLineOption dpmsExcluded = QCommandLineOption({QStringLiteral("dpms-excluded")},
                                                         QStringLiteral("Do not apply the dpms change to the output with said model names"),
                                                         QStringLiteral("connector"));
    QCommandLineOption log = QCommandLineOption(QStringList() << QStringLiteral("l") << QStringLiteral("log"),
                                                QStringLiteral("Write a comment to the log file"),
                                                QStringLiteral("comment"));

    QCommandLineParser parser;
    parser.setApplicationDescription(desc);
    parser.addPositionalArgument(QStringLiteral("config"), syntax, QStringLiteral("[output.<name>.<setting> output.<name>.setting [...]]"));
    parser.addHelpOption();
    parser.addOption(info);
    parser.addOption(json);
    parser.addOption(outputs);
    parser.addOption(dpms);
    parser.addOption(log);
    parser.addOption(dpmsExcluded);
    parser.process(app);

    if (!parser.positionalArguments().isEmpty()) {
        server.setOptionList(parser.positionalArguments());
    }

    server.start(&parser);
    return app.exec();
}
