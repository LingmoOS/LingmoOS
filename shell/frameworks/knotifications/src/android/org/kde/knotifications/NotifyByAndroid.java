/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

package org.kde.knotifications;

import android.app.Activity;
import android.app.Notification;
import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.app.RemoteInput;
import android.app.PendingIntent;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.graphics.drawable.Icon;
import android.os.Build;
import android.os.Bundle;
import android.text.Html;
import android.util.Log;
import java.util.HashMap;
import java.util.HashSet;

/** Java side of the Android notification backend. */
public class NotifyByAndroid extends BroadcastReceiver
{
    private static final String TAG = "org.kde.knotifications";

    private static final String NOTIFICATION_ACTION = ".org.kde.knotifications.NOTIFICATION_ACTION";
    private static final String NOTIFICATION_DELETED = ".org.kde.knotifications.NOTIFICATION_DELETED";
    private static final String NOTIFICATION_OPENED = ".org.kde.knotifications.NOTIFICATION_OPENED";
    private static final String NOTIFICATION_REPLIED = ".org.kde.knotifications.NOTIFICATION_REPLIED";
    // the id of the notification triggering an intent
    private static final String NOTIFICATION_ID_EXTRA = "org.kde.knotifications.NOTIFICATION_ID";
    // the id of the action that was triggered for a notification
    private static final String NOTIFICATION_ACTION_ID_EXTRA = "org.kde.knotifications.NOTIFICATION_ACTION_ID";
    // the group a notification belongs too
    private static final String NOTIFICATION_GROUP_EXTRA = "org.kde.knotifications.NOTIFICATION_GROUP";
    // RemoteInput value key
    private static final String REMOTE_INPUT_KEY = "REPLY";

    // notification id offset for group summary notifications
    // we need this to stay out of the regular notification's id space (which comes from the C++ side)
    // and so we can distinguish if we received actions on regular notifications or group summaries
    private static final int NOTIFICATION_GROUP_ID_FLAG = (1 << 24);

    private android.content.Context m_ctx;
    private NotificationManager m_notificationManager;
    private int m_uniquePendingIntentId = 0;
    private HashSet<String> m_channels = new HashSet();

    private class GroupData {
        public HashSet<Integer> childIds = new HashSet();
        public int groupId;
    };
    private HashMap<String, GroupData> m_groupSummaries = new HashMap();

    public NotifyByAndroid(android.content.Context context)
    {
        Log.i(TAG, context.getPackageName());
        m_ctx = context;
        m_notificationManager = (NotificationManager)m_ctx.getSystemService(Context.NOTIFICATION_SERVICE);

        IntentFilter filter = new IntentFilter();
        filter.addAction(m_ctx.getPackageName() + NOTIFICATION_ACTION);
        filter.addAction(m_ctx.getPackageName() + NOTIFICATION_DELETED);
        filter.addAction(m_ctx.getPackageName() + NOTIFICATION_OPENED);
        filter.addAction(m_ctx.getPackageName() + NOTIFICATION_REPLIED);
        m_ctx.registerReceiver(this, filter);
    }

    public void notify(KNotification notification)
    {
        Log.i(TAG, notification.text);

        // notification channel
        if (!m_channels.contains(notification.channelId)) {
            m_channels.add(notification.channelId);

            if (Build.VERSION.SDK_INT >= 26) {
                NotificationChannel channel = new NotificationChannel(notification.channelId, notification.channelName, NotificationManager.IMPORTANCE_DEFAULT);
                channel.setDescription(notification.channelDescription);

                switch (notification.urgency) {
                    case KNotification.CriticalUrgency:
                        channel.setImportance(NotificationManager.IMPORTANCE_HIGH);
                        break;
                    case KNotification.NormalUrgency:
                        channel.setImportance(NotificationManager.IMPORTANCE_LOW);
                        break;
                    case KNotification.LowUrgency:
                        channel.setImportance(NotificationManager.IMPORTANCE_MIN);
                        break;
                    case KNotification.HighUrgency:
                    default:
                        channel.setImportance(NotificationManager.IMPORTANCE_DEFAULT);
                        break;
                }

                m_notificationManager.createNotificationChannel(channel);
            }
        }

        Notification.Builder builder;
        if (Build.VERSION.SDK_INT >= 26) {
            builder = new Notification.Builder(m_ctx, notification.channelId);
        } else {
            builder = new Notification.Builder(m_ctx);
        }

        if (Build.VERSION.SDK_INT >= 23) {
            builder.setSmallIcon((Icon)notification.icon);
        } else {
            builder.setSmallIcon(m_ctx.getApplicationInfo().icon);
        }
        builder.setContentTitle(notification.title);
        builder.setContentText(notification.text);
        // regular notifications show only a single line of content, if we have more
        // we need the "BigTextStyle" expandable notifications to make everything readable
        // in the single line case this behaves like the regular one, so no special-casing needed
        if (Build.VERSION.SDK_INT >= 24) {
            builder.setStyle(new Notification.BigTextStyle().bigText(Html.fromHtml(notification.richText, Html.FROM_HTML_MODE_COMPACT)));
        } else {
            builder.setStyle(new Notification.BigTextStyle().bigText(Html.fromHtml(notification.richText)));
        }

        // lock screen visibility
        switch (notification.visibility) {
            case "public":
                builder.setVisibility(Notification.VISIBILITY_PUBLIC);
                break;
            case "private":
                builder.setVisibility(Notification.VISIBILITY_PRIVATE);
                break;
            case "secret":
                builder.setVisibility(Notification.VISIBILITY_SECRET);
                break;
        }

        // grouping
        if (notification.group != null) {
            createGroupNotification(notification);
            builder.setGroup(notification.group);
        }

        // legacy priority handling for versions without NotificationChannel support
        if (Build.VERSION.SDK_INT < 26) {
            switch (notification.urgency) {
                case KNotification.CriticalUrgency:
                    builder.setPriority(Notification.PRIORITY_HIGH);
                    break;
                case KNotification.NormalUrgency:
                    builder.setPriority(Notification.PRIORITY_LOW);
                    break;
                case KNotification.LowUrgency:
                    builder.setPriority(Notification.PRIORITY_MIN);
                    break;
                case KNotification.HighUrgency:
                default:
                    builder.setPriority(Notification.PRIORITY_DEFAULT);
                    break;
            }
        }

        // pending intent flags backward compatibility
        int PENDING_INTENT_FLAG_IMMUTABLE = 0;
        int PENDING_INTENT_FLAG_MUTABLE = 0;
        if (Build.VERSION.SDK_INT >= 23) {
            PENDING_INTENT_FLAG_IMMUTABLE = PendingIntent.FLAG_IMMUTABLE;
        }
        if (Build.VERSION.SDK_INT >= 31) {
            PENDING_INTENT_FLAG_MUTABLE = PendingIntent.FLAG_MUTABLE;
        }

        // taping the notification shows the app
        Intent intent = new Intent(m_ctx.getPackageName() + NOTIFICATION_OPENED);
        intent.putExtra(NOTIFICATION_ID_EXTRA, notification.id);
        PendingIntent contentIntent = PendingIntent.getBroadcast(m_ctx, m_uniquePendingIntentId++, intent, PendingIntent.FLAG_UPDATE_CURRENT | PENDING_INTENT_FLAG_IMMUTABLE);
        builder.setContentIntent(contentIntent);

        // actions
        for (HashMap.Entry<String, String> entry : notification.actions.entrySet()) {
            String id = entry.getKey();
            String label = entry.getValue();

            Intent actionIntent = new Intent(m_ctx.getPackageName() + NOTIFICATION_ACTION);
            actionIntent.putExtra(NOTIFICATION_ID_EXTRA, notification.id);
            actionIntent.putExtra(NOTIFICATION_ACTION_ID_EXTRA, id);
            PendingIntent pendingIntent = PendingIntent.getBroadcast(m_ctx, m_uniquePendingIntentId++, actionIntent, PendingIntent.FLAG_UPDATE_CURRENT | PENDING_INTENT_FLAG_IMMUTABLE);
            Notification.Action action = new Notification.Action.Builder(0, label, pendingIntent).build();
            builder.addAction(action);
        }

        // inline reply actions
        if (notification.inlineReplyLabel != null) {
            Intent replyIntent = new Intent(m_ctx.getPackageName() + NOTIFICATION_REPLIED);
            replyIntent.putExtra(NOTIFICATION_ID_EXTRA, notification.id);
            PendingIntent pendingReplyIntent = PendingIntent.getBroadcast(m_ctx, m_uniquePendingIntentId++, replyIntent, PendingIntent.FLAG_UPDATE_CURRENT | PENDING_INTENT_FLAG_MUTABLE);

            RemoteInput input = new RemoteInput.Builder(REMOTE_INPUT_KEY)
                .setAllowFreeFormInput(true)
                .setLabel(notification.inlineReplyPlaceholder)
                .build();
            Notification.Action replyAction = new Notification.Action.Builder(0, notification.inlineReplyLabel, pendingReplyIntent)
                .addRemoteInput(input)
                .build();
            builder.addAction(replyAction);
        }

        // notification about user closing the notification
        Intent deleteIntent = new Intent(m_ctx.getPackageName() + NOTIFICATION_DELETED);
        deleteIntent.putExtra(NOTIFICATION_ID_EXTRA, notification.id);
        if (notification.group != null) {
            deleteIntent.putExtra(NOTIFICATION_GROUP_EXTRA, notification.group);
        }
        Log.i(TAG, deleteIntent.getExtras() + " " + notification.id);
        builder.setDeleteIntent(PendingIntent.getBroadcast(m_ctx, m_uniquePendingIntentId++, deleteIntent, PendingIntent.FLAG_UPDATE_CURRENT | PENDING_INTENT_FLAG_IMMUTABLE));

        m_notificationManager.notify(notification.id, builder.build());
    }

    public void close(int id, String group)
    {
        m_notificationManager.cancel(id);

        if (group != null && m_groupSummaries.containsKey(group)) {
            GroupData g = m_groupSummaries.get(group);
            g.childIds.remove(id);
            if (g.childIds.isEmpty()) {
                m_groupSummaries.remove(group);
                m_notificationManager.cancel(g.groupId);
            } else {
                m_groupSummaries.put(group, g);
            }
        }
    }

    @Override
    public void onReceive(Context context, Intent intent)
    {
        String action = intent.getAction();

        int id = intent.getIntExtra(NOTIFICATION_ID_EXTRA, -1);
        Log.i(TAG, action + ": " + id + " " + intent.getExtras());

        if (action.equals(m_ctx.getPackageName() + NOTIFICATION_ACTION)) {
            // user activated one of the custom actions
            String actionId = intent.getStringExtra(NOTIFICATION_ACTION_ID_EXTRA);
            notificationActionInvoked(id, actionId);
        } else if (action.equals(m_ctx.getPackageName() + NOTIFICATION_DELETED)) {
            // user (or system) dismissed the notification - this can happen both for groups and regular notifications
            String group = null;
            if (intent.hasExtra(NOTIFICATION_GROUP_EXTRA)) {
                group = intent.getStringExtra(NOTIFICATION_GROUP_EXTRA);
            }

            if ((id & NOTIFICATION_GROUP_ID_FLAG) != 0) {
                // entire group has been deleted
                m_groupSummaries.remove(group);
            } else {
                // a single regular notification, so reduce the refcount of the group if there is one
                notificationFinished(id);
                if (group != null && m_groupSummaries.containsKey(group)) {
                    // we do not need to handle the case of childIds being empty here, the system will send us a deletion intent for the group too
                    // this only matters for the logic in close().
                    GroupData g = m_groupSummaries.get(group);
                    g.childIds.remove(id);
                    m_groupSummaries.put(group, g);
                }
            }
        } else if (action.equals(m_ctx.getPackageName() + NOTIFICATION_OPENED)) {
            // user tapped the notification
            Intent newintent = new Intent(m_ctx, m_ctx.getClass());
            newintent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
            m_ctx.startActivity(newintent);
            notificationActionInvoked(id, "default");
        } else if (action.equals(m_ctx.getPackageName() + NOTIFICATION_REPLIED)) {
            Bundle remoteInput = RemoteInput.getResultsFromIntent(intent);
            if (remoteInput != null) {
                String s = (String) remoteInput.getCharSequence(REMOTE_INPUT_KEY);
                notificationInlineReply(id, s);
            }
        }
    }

    public native void notificationFinished(int notificationId);
    public native void notificationActionInvoked(int notificationId, String action);
    public native void notificationInlineReply(int notificationId, String text);

    private void createGroupNotification(KNotification notification)
    {
        if (m_groupSummaries.containsKey(notification.group)) {
            GroupData group = m_groupSummaries.get(notification.group);
            group.childIds.add(notification.id);
            m_groupSummaries.put(notification.group, group);
            return;
        }

        GroupData group = new GroupData();
        group.childIds.add(notification.id);
        group.groupId = m_uniquePendingIntentId++ + NOTIFICATION_GROUP_ID_FLAG;
        m_groupSummaries.put(notification.channelId, group);

        Notification.Builder builder;
        if (Build.VERSION.SDK_INT >= 26) {
            builder = new Notification.Builder(m_ctx, notification.channelId);
        } else {
            builder = new Notification.Builder(m_ctx);
        }

        if (Build.VERSION.SDK_INT >= 23) {
            builder.setSmallIcon((Icon)notification.icon);
        } else {
            builder.setSmallIcon(m_ctx.getApplicationInfo().icon);
        }
        builder.setContentTitle(notification.channelName);
        builder.setContentText(notification.channelDescription);
        builder.setGroup(notification.group);
        builder.setGroupSummary(true);

        // monitor for deletion (which happens when the last child notification is closed)
        int PENDING_INTENT_FLAG_IMMUTABLE = 0;
        if (Build.VERSION.SDK_INT >= 23) {
            PENDING_INTENT_FLAG_IMMUTABLE = PendingIntent.FLAG_IMMUTABLE;
        }

        Intent deleteIntent = new Intent(m_ctx.getPackageName() + NOTIFICATION_DELETED);
        deleteIntent.putExtra(NOTIFICATION_GROUP_EXTRA, notification.group);
        deleteIntent.putExtra(NOTIFICATION_ID_EXTRA, group.groupId);
        builder.setDeleteIntent(PendingIntent.getBroadcast(m_ctx, m_uniquePendingIntentId++, deleteIntent, PendingIntent.FLAG_UPDATE_CURRENT | PENDING_INTENT_FLAG_IMMUTABLE));

        // try to stay out of the normal id space for regular notifications
        m_notificationManager.notify(group.groupId, builder.build());
    }
}
