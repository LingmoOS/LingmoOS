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

previous_activity=""

for ((i=0; i < ${#activities[@]}; ++i)); do
    if [ "$current_activity" = "${activities[$i]}" ]; then
        if [ "$previous_activity" != "" ]; then
            echo "Switching to $previous_activity"
            qdbus org.kde.ActivityManager /ActivityManager/Activities SetCurrentActivity $previous_activity
            exit
        else
            echo "Switching to ${activities[-1]}"
            qdbus org.kde.ActivityManager /ActivityManager/Activities SetCurrentActivity ${activities[-1]}
            exit
        fi
    else
        previous_activity="${activities[$i]}"
    fi
done

