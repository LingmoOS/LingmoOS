/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

package org.kde.knotifications;

import android.graphics.drawable.Icon;
import android.os.Build;

import java.lang.Object;
import java.util.ArrayList;
import java.util.HashMap;

/** Java side of KNotification.
 *  Used to convey the relevant notification data to Java.
 */
public class KNotification
{
    public int id;
    public String text;
    public String richText;
    public String title;
    public Object icon;
    public HashMap<String, String> actions = new HashMap<>();
    public String channelId;
    public String channelName;
    public String channelDescription;
    public String group;
    public int urgency;
    public String visibility;

    public String inlineReplyLabel;
    public String inlineReplyPlaceholder;

    // see knotification.h
    public static final int LowUrgency = 10;
    public static final int NormalUrgency = 50;
    public static final int HighUrgency = 70;
    public static final int CriticalUrgency = 90;

    public void setIconFromData(byte[] data, int length)
    {
        if (Build.VERSION.SDK_INT >= 23) {
            icon = Icon.createWithData(data, 0, length);
        }
    }

    public void addAction(String id, String label)
    {
        actions.put(id, label);
    }
}
