# PowerDevil

PowerDevil is the internal name of the KDE power management service for Plasma.
It is responsible for some (but not all) interactions with hardware functionality. The service will:

* Suspend or shut down sessions under certain conditions such as user inactivity, closing the laptop lid or pressing the power button.
* Adjust the brightness level of displays and keyboards, or turn display backlights off/on altogether.
* Change settings according to the current power state (plugged in, battery, low battery), which can be customized in System Settings.
* Monitor the current battery charge, and set charge thresholds for battery-powered devices that support it.
* Keep track of system state - e.g. suspend/idle/etc. inhibitors, activities, screen locking - to adjust power management behaviors accordingly.
* Communicate with underlying services such as [UPower](https://gitlab.freedesktop.org/upower/upower), [power-profiles-daemon](https://gitlab.freedesktop.org/upower/power-profiles-daemon), [ddcutil](https://github.com/rockowitz/ddcutil), and/or [systemd](https://systemd.io/) to implement some of the above.
* Provide a D-Bus interface for other Plasma components such as the Power Management applet or the Brightness and Color applet.


## Related components

Directly interacting with this service, but not located in this repository:

* The [Power Management applet](https://invent.kde.org/plasma/plasma-workspace/-/tree/master/applets/batterymonitor) in plasma-workspace.
* The [Brightness and Color applet](https://invent.kde.org/plasma/plasma-workspace/-/tree/master/applets/brightness), also part of plasma-workspace.
* Even [KRunner commands for PowerDevil actions](https://invent.kde.org/plasma/plasma-workspace/-/tree/master/runners/powerdevil) are part of plasma-workspace for some reason.
* In a Wayland session, [KWin](https://invent.kde.org/plasma/kwin) takes over certain functionality related to user activity tracking and turning displays on/off which in X11 is managed by PowerDevil directly.
* Plasma Mobile has [its own UI for power management settings](https://invent.kde.org/plasma/plasma-mobile/-/tree/master/kcms/powermanagement).

PowerDevil does not handle hardware interactions such as display configuration, turning wireless interfaces off/on ("airplane mode"), mounting/unmounting disks, or sound/audio configuration. Other Plasma components are responsible for these features.


## Troubleshooting & reporting bugs

PowerDevil will be automatically started when logging into a Plasma session. When logging into different Plasma sessions with different users at the same time, each session will run its own instance of the service.

Similar to other KDE software, PowerDevil will only log warnings and errors by default. When troubleshooting or filing a bug report, it's a good idea to increase the verbosity of system log messages first.

If your system is based on systemd, like most Linux distributions nowadays, then the various Plasma components including PowerDevil are started as systemd service units after logging in. You can adjust the way that the service is run, by executing (in a terminal):

```
systemctl --user edit plasma-powerdevil.service
```

This will open an editor. Define an additional environment variable in the designated space to show all log messages in the PowerDevil log category, like this:

```
### Editing /home/$USER/.config/systemd/user/plasma-powerdevil.service.d/override.conf
### Anything between here and the comment below will become the contents of the drop-in file

[Service]
Environment="QT_LOGGING_RULES=org.kde.powerdevil=true"

### Edits below this comment will be discarded
### (...)
```

PowerDevil can be restarted inside an active Plasma session by executing:

```
systemctl --user restart plasma-powerdevil.service
```

After restarting the service, expanded logs will start appearing in your system log, which can be viewed with e.g. `journalctl --boot`. PowerDevil logs show up under the `org_kde_powerdevil` name.

Stopping the service is possible without closing or crashing the session, but a few expected desktop features will be absent until you restart it.

If the service crashes, `coredumpctl` should be able to retrieve stack traces that are useful for developers.

PowerDevil is split into two main components on the KDE bug tracker:

* [Powerdevil](https://bugs.kde.org/buglist.cgi?bug_status=UNCONFIRMED&bug_status=CONFIRMED&bug_status=ASSIGNED&bug_status=REOPENED&bug_status=NEEDSINFO&component=general&list_id=2670705&order=changeddate%20DESC%2Cbug_status%2Cpriority%2Cassigned_to%2Cbug_id&product=Powerdevil&query_format=advanced), the background service ([report bug](https://bugs.kde.org/enter_bug.cgi?product=Powerdevil) after checking for duplicates)
* The [kcm_powerdevil component in System Settings](https://bugs.kde.org/buglist.cgi?bug_status=UNCONFIRMED&bug_status=CONFIRMED&bug_status=ASSIGNED&bug_status=REOPENED&bug_status=NEEDSINFO&component=kcm_powerdevil&list_id=2670714&order=changeddate%20DESC%2Cbug_status%2Cpriority%2Cassigned_to%2Cbug_id&product=systemsettings&query_format=advanced), referring to the Power Management settings UI ([report bug](https://bugs.kde.org/enter_bug.cgi?product=systemsettings&component=kcm_powerdevil) after checking for duplicates)

Remember to include versions of relevant system software, and (if appropriate) system logs. The easier you make it for developers to reproduce bugs or find out why & where the code breaks, the more likely it is to get those issues fixed. Thanks for your help in making life better for other users and developers.


### Troubleshooting DDC/CI monitor brightness controls

PowerDevil uses [(lib)ddcutil](https://github.com/rockowitz/ddcutil) to adjust brightness of external monitors. Log messages of ddcutil are included in the `org_kde_powerdevil` log output.

DDC/CI experiences can differ greatly among different monitor manufacturers and ddcutil is making the best of a complicated situation. Sometimes users can nonetheless run into issues with DDC/CI monitor support. If this happens and the power management service does not start correctly, Plasma as a whole will have trouble starting and functioning as intended.

Instead of disabling the PowerDevil service altogether, you can disable merely its use of ddcutil by defining `POWERDEVIL_NO_DDCUTIL=1` as environment variable to your service configuration. Refer to the instructions for `QT_LOGGING_RULES` above for how to go about this. Aside from missing DDC/CI brightness controls, this will leave you with a working system until a fix can be found and deployed to your system.

Advanced users who want to dig into ddcutil can specify a [`ddcutil/ddcutilrc` file in their config directory](http://www.ddcutil.com/config_file/), to change selected ddcutil options ([like these](http://www.ddcutil.com/performance_options/)) and/or enable extremely detailed [ddcutil tracing output](http://www.ddcutil.com/debug_options/). Chances are you'll need technical background knowledge and/or support from the ddcutil maintainer to get any useful fixes out of this. So let's just say that PowerDevil supports ddcutil config files, will apply the `[global]` and `[libddcutil]` config sections, and leave it at that.


## Building & debugging

Set up a KDE development environment [as described in the developer documentation](https://develop.kde.org/docs/getting-started/building/), in particular the parts about [setting up kdesrc-build](https://develop.kde.org/docs/getting-started/building/kdesrc-build-setup/) and [building KDE software including Plasma](https://develop.kde.org/docs/getting-started/building/kdesrc-build-compile/). PowerDevil is part of the `workspace` meta-target for `kdesrc-build` and will be built by following the standard instructions.

Restart using systemd (or your init system of choice) as mentioned earlier. Use system log messages and attach gdb to the running session for further debugging.

PowerDevil ships with KAuth helpers, which have to be configured on a system-wide basis. If you need to change these helpers, consult MRs [!1715](https://invent.kde.org/plasma/plasma-workspace/-/merge_requests/1715) and/or [!3705](https://invent.kde.org/plasma/plasma-workspace/-/merge_requests/3705) in plasma-workspace for possible ways to enable their development builds on your system.
