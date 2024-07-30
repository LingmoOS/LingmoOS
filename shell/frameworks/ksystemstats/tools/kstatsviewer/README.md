This executable subscribes to a few sensors and prints out sensor values.

It deliberately uses the native DBus API so we can test parts individually.


## Listing Available Sensors

To see all of the available sensors (unsorted), use the `--list` option.
This will print each sensor's Id and a human-readable name.

```
    kstatsviewer --list
```


## Showing Sensor Data

To subscribe to specific sensors, list the Ids of the sensors on the
command-line. The command-line flag `--details` will also display the
details (range, units, etc.) about each of the sensors. The command-line
flag `--remain` turns on the *rolling* display and will not exit, but
continue to print sensor values as they update (e.g. about once a second).

```
    kstatsviewer cpu/cpu0/usage
    kstatsviewer --details cpu/cpu0/usage
    kstatsviewer --remain cpu/cpu0/usage memory/physical/free
```

<!--
SPDX-FileCopyrightText: 2020 David Edmundson <davidedmundson@kde.org>
SPDX-License-Identifier: BSD-3-Clause
-->
