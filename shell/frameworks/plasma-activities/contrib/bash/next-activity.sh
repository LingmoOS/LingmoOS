#! /bin/bash
#
# next-activity.sh
# SPDX-FileCopyrightText: 2016 Ivan Čukić <ivan.cukic(at)kde.org>
#
# SPDX-License-Identifier: MIT
#

current_activity=($(qdbus org.kde.ActivityManager /ActivityManager/Activities CurrentActivity))
activities=($(qdbus org.kde.ActivityManager /ActivityManager/Activities ListActivities))
found="0"

for ((i=0; i < ${#activities[@]}; ++i)); do
    if [ "$current_activity" = "${activities[$i]}" ]; then
        found="1"
    else
        if [ "$found" == "1" ]; then
            echo "Switching to ${activities[$i]}"
            qdbus org.kde.ActivityManager /ActivityManager/Activities SetCurrentActivity ${activities[$i]}
            found="0"
        fi
    fi
done

if [ "$found" == "1" ]; then
    echo "Switching to ${activities[0]}"
    qdbus org.kde.ActivityManager /ActivityManager/Activities SetCurrentActivity ${activities[0]}
fi


