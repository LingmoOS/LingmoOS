### %{APPNAME}

This plugin provides a simple template for a KRunner plugin using dbus.

The install script copies the KRunner config file and a D-Bus activation service file to their appropriate locations.
This way the Python script gets executed when KRunner requests matches and it does not need to be autostarted.

```bash
mkdir -p ~/.local/share/krunner/dbusplugins/
cp %{APPNAMELC}.desktop ~/.local/share/krunner/dbusplugins/
kquitapp6 krunner
python3 %{APPNAMELC}.py
```

After that you should see your runner when typing `hello` in KRunner.

More information regarding the D-Bus API can be found here:

* https://invent.kde.org/frameworks/krunner/-/blob/master/src/data/org.kde.krunner1.xml
* https://develop.kde.org/docs/features/d-bus/introduction_to_dbus/


If you feel confident about your runner you can upload it to the KDE Store https://store.kde.org/browse?cat=628&ord=latest.
