#ifndef __SCREEN_DEFINE_H__
#define __SCREEN_DEFINE_H__

#define LINGMO_SETTINGS_DAEMON_COLOR_SCHEMA   "org.lingmo.SettingsDaemon.plugins.color"
#define LINGMO_SETTINGS_DAEMON_COLOR_EYS_CARE "eye-care"
#define LINGMO_SETTINGS_DAEMON_COLOR_NIGHT_MODE   "night-light-enabled"
#define MY_NAME  "lingmo-framework-screen"
#define LINGMO_POWER_MANAGER_DBUS_NAME         "org.lingmo.powermanagement"
#define LINGMO_POWER_MANAGER_DBUS_PATH         "/"
#define LINGMO_POWER_MANAGER_DBUS_INTERFACE    "org.lingmo.powermanagement.interface"

struct OutputGammaInfo{
    QString outputName;
    uint    gamma;
    uint    brignthess;
    uint    temperature;
};

typedef QList<OutputGammaInfo> OutputGammaInfoList;

Q_DECLARE_METATYPE(OutputGammaInfo)
Q_DECLARE_METATYPE(OutputGammaInfoList)

inline QDBusArgument &operator << (QDBusArgument &argument, const OutputGammaInfo &outputInfo)
{
    argument.beginStructure();
    argument << outputInfo.outputName;
    argument << outputInfo.brignthess;
    argument << outputInfo.gamma;
    argument << outputInfo.temperature;
    argument.endStructure();

    return argument;
}

inline const QDBusArgument &operator >> (const QDBusArgument &argument, OutputGammaInfo &outputInfo) {
    argument.beginStructure();
    argument >> outputInfo.outputName;
    argument >> outputInfo.brignthess;
    argument >> outputInfo.gamma;
    argument >> outputInfo.temperature;
    argument.endStructure();
    return argument;
}

#endif
