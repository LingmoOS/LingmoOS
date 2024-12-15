// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package com.lingmo.assistant.services

import android.annotation.SuppressLint
import android.app.Notification
import android.app.NotificationChannel
import android.app.NotificationManager
import android.app.PendingIntent
import android.app.Service
import android.content.Intent
import android.content.pm.ServiceInfo
import android.net.ConnectivityManager
import android.net.ConnectivityManager.NetworkCallback
import android.net.Network
import android.os.Build
import android.os.Handler
import android.os.IBinder
import android.os.Looper
import android.os.PowerManager
import android.os.PowerManager.WakeLock
import android.os.SystemClock
import android.util.Log
import android.view.Display
import androidx.core.app.NotificationCompat
import androidx.preference.PreferenceManager
import com.lingmo.assistant.R
import com.lingmo.assistant.ui.activity.HomeActivity
import com.lingmo.assistant.utils.Utils
import com.lingmo.assistant.utils.Utils.getDeviceName
import com.lingmo.assistant.utils.Utils.getDisplayMetrics
import net.christianbeier.droidvnc_ng.DroidManager.ClientListener
import net.christianbeier.droidvnc_ng.JniDroidVnc
import net.christianbeier.droidvnc_ng.JniDroidVnc.Companion.registerClientListener
import java.io.File
import java.net.NetworkInterface
import java.net.SocketException
import java.nio.ByteBuffer
import java.util.concurrent.ConcurrentHashMap
import kotlin.math.min


//import androidx.core.content.IntentSanitizer;
class MainService : Service() {
    private var mResultCode = 0
    private var mResultData: Intent? = null
    private var mWakeLock: WakeLock? = null
    private var mNotification: Notification? = null

    private var mNumberOfClients = 0

    private var mDroidJni: JniDroidVnc? = null

    private class OutboundClientReconnectData {
        var intent: Intent? = null
        var client: Long = 0
        var reconnectTriesLeft: Int = 0
        var backoff: Int = 0

        companion object {
            const val BACKOFF_INIT: Int = 2
            const val BACKOFF_LIMIT: Int = 64
        }
    }

    /// This maps the Intent's request id to an OutboundClientReconnectData entry
    private val mOutboundClientsToReconnect =
        ConcurrentHashMap<String, OutboundClientReconnectData>()
    private val mOutboundClientReconnectHandler = Handler(Looper.getMainLooper())
    private val mNetworkChangeListener: NetworkCallback = object : NetworkCallback() {
        override fun onAvailable(network: Network) {
            // fires when wifi lost and mobile data selected as well, but that won't hurt...
            Log.d(TAG, "DefaultNetworkCallback: now available: $network")
            /*
                A new default network came up: try to reconnect disconnected outbound clients immediately
             */
            mOutboundClientsToReconnect
                .entries
                .stream()
                .filter { entry: Map.Entry<String, OutboundClientReconnectData> -> entry.value.client == 0L } // is disconnected
                .forEach { entry: Map.Entry<String, OutboundClientReconnectData> ->
                    // if the client is set to reconnect, it definitely has tries left on disconnect
                    // (otherwise it wouldn't be in the list), so fire up reconnect action
                    Log.d(
                        TAG,
                        "DefaultNetworkCallback: resetting backoff and reconnecting outbound connection w/ request id " + entry.key
                    )

                    // remove other callbacks as we don't want 2 runnables for this request on the handler queue at the same time!
                    mOutboundClientReconnectHandler.removeCallbacksAndMessages(entry.key)
                    // reset backoff for this connection
                    entry.value.backoff = OutboundClientReconnectData.BACKOFF_INIT
                    entry.value.reconnectTriesLeft =
                        entry.value.intent!!.getIntExtra(EXTRA_RECONNECT_TRIES, 0)
                    // NB that onAvailable() runs on internal ConnectivityService thread, so still use mOutboundClientReconnectHandler here
                    mOutboundClientReconnectHandler.postAtTime(
                        {
                            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
                                startForegroundService(entry.value.intent)
                            } else {
                                startService(entry.value.intent)
                            }
                        },
                        SystemClock.uptimeMillis() + entry.value.backoff * 1000L
                    )
                }
        }
    }

    private var mIsStopping = false

    // service is stopping on OUR initiative, NOT by stopService()
    private var mIsStoppingByUs = false

    private var mDefaults: Defaults? = null

    private val mDroidClientListener: ClientListener = object : ClientListener {
        override fun onClientConnected(client: Long) {
            Log.d(TAG, "onClientConnected: client $client")

            try {
                instance!!.mWakeLock!!.acquire()
                instance!!.mNumberOfClients++
                instance!!.updateNotification()
                InputService.addClient(
                    client, PreferenceManager.getDefaultSharedPreferences(
                        instance!!
                    ).getBoolean(
                        PREFS_KEY_SERVER_LAST_SHOW_POINTERS, Defaults(
                            instance!!
                        ).showPointers
                    )
                )
                if (!MediaProjectionService.isMediaProjectionEnabled() && InputService.isTakingScreenShots) {
                    Log.d(TAG, "onClientConnected: in fallback screen capture mode, asking for upgrade")
                    val mediaProjectionRequestIntent = Intent(
                        instance,
                        MediaProjectionRequestActivity::class.java
                    )
                    mediaProjectionRequestIntent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK)
                    mediaProjectionRequestIntent.putExtra(
                        MediaProjectionRequestActivity.EXTRA_UPGRADING_FROM_FALLBACK_SCREEN_CAPTURE,
                        true
                    )
                    instance!!.startActivity(mediaProjectionRequestIntent)
                }
            } catch (e: Exception) {
                // instance probably null
                Log.e(TAG, "onClientConnected: error: $e")
            }
        }

        override fun onClientDisconnected(client: Long) {
            Log.d(TAG, "onClientDisconnected: client $client")

            try {
                instance!!.mWakeLock!!.release()
                instance!!.mNumberOfClients--
                if (!instance!!.mIsStopping) {
                    // don't show notifications when clients are disconnected on orderly server shutdown
                    instance!!.updateNotification()
                }
                InputService.removeClient(client)

                // check if the gone client was part of a reconnect entry
                instance!!.mOutboundClientsToReconnect
                    .entries
                    .stream()
                    .filter { entry: Map.Entry<String, OutboundClientReconnectData> -> entry.value.client == client }
                    .forEach { entry: Map.Entry<String, OutboundClientReconnectData> ->
                        // unset entry's client as it's now disconnected
                        entry.value.client = 0
                        // if the connections is set to reconnect, it definitely has tries left on disconnect
                        // (otherwise it wouldn't be in the list), so fire up reconnect action
                        Log.d(
                            TAG,
                            "onClientDisconnected: outbound connection " + entry.key + " set to reconnect, reconnecting with delay of " + entry.value.backoff + " seconds"
                        )
                        instance!!.mOutboundClientReconnectHandler.postAtTime(
                            {
                                try {
                                    if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
                                        instance!!.startForegroundService(entry.value.intent)
                                    } else {
                                        instance!!.startService(entry.value.intent)
                                    }
                                } catch (ignored: NullPointerException) {
                                    // onClientDisconnected() is triggered by vncStopServer() from onDestroy(),
                                    // but the actual call might happen well after instance is set to null in onDestroy()
                                }
                            },
                            entry.key,
                            SystemClock.uptimeMillis() + entry.value.backoff * 1000L
                        )
                    }
            } catch (e: Exception) {
                // instance probably null
                Log.e(TAG, "onClientDisconnected: error: $e")
            }
        }
    }

    override fun onBind(intent: Intent): IBinder? {
        return null
    }

    override fun onCreate() {
        Log.d(TAG, "onCreate")

        instance = this

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            /*
                Create notification channel
             */
            val serviceChannel = NotificationChannel(
                packageName,
                "Foreground Service Channel",
                NotificationManager.IMPORTANCE_DEFAULT
            )
            val manager = getSystemService(
                NotificationManager::class.java
            )
            manager.createNotificationChannel(serviceChannel)

            /*
                startForeground() w/ notification
             */
            if (Build.VERSION.SDK_INT >= 29) {
                startForeground(
                    NOTIFICATION_ID,
                    getNotification(null, true),
                    ServiceInfo.FOREGROUND_SERVICE_TYPE_CONNECTED_DEVICE
                )
            } else {
                startForeground(NOTIFICATION_ID, getNotification(null, true))
            }
        }

        /*
            Get a wake lock
         */
        mWakeLock = (instance!!.getSystemService(POWER_SERVICE) as PowerManager).newWakeLock(
            (PowerManager.SCREEN_DIM_WAKE_LOCK or PowerManager.ACQUIRE_CAUSES_WAKEUP or PowerManager.ON_AFTER_RELEASE),
            TAG + ":clientsConnected"
        )

        /*
            Register a listener for network-up events
         */
        (getSystemService(CONNECTIVITY_SERVICE) as ConnectivityManager).registerDefaultNetworkCallback(
            mNetworkChangeListener
        )

        /*
            Load defaults
         */
        mDefaults = Defaults(this)

        mDroidJni = JniDroidVnc()
        registerClientListener(mDroidClientListener)
    }


    override fun onDestroy() {
        Log.d(TAG, "onDestroy")

        mIsStopping = true

        if (!mIsStoppingByUs && mDroidJni!!.vncIsActive()) {
            // stopService() from OS or other component
            Log.d(TAG, "onDestroy: sending ACTION_STOP")
            sendBroadcastToOthersAndUs(Intent(ACTION_STOP))
        }

        if (Build.VERSION.SDK_INT < Build.VERSION_CODES.O) {
            // API levels < 26 don't have the mandatory foreground notification and need manual notification dismissal
            getSystemService(NotificationManager::class.java).cancelAll()
        }

        // unregister network change listener
        try {
            (getSystemService(CONNECTIVITY_SERVICE) as ConnectivityManager).unregisterNetworkCallback(
                mNetworkChangeListener
            )
        } catch (ignored: Exception) {
            // was not registered
        }

        // remove all pending client reconnects
        mOutboundClientReconnectHandler.removeCallbacksAndMessages(null)

        stopScreenCapture()
        mDroidJni!!.vncStopServer()
        instance = null
    }


    override fun onStartCommand(intent: Intent, flags: Int, startId: Int): Int {
        val accessKey = intent.getStringExtra(EXTRA_ACCESS_KEY)
        if (accessKey == null || accessKey.isEmpty()
            || accessKey != PreferenceManager.getDefaultSharedPreferences(this)
                .getString(Constants.PREFS_KEY_SETTINGS_ACCESS_KEY, mDefaults!!.accessKey)
        ) {
            Log.e(TAG, "Access key missing or incorrect")
            if (!mDroidJni!!.vncIsActive()) {
                stopSelfByUs()
            }
            return START_NOT_STICKY
        }

        if (ACTION_HANDLE_MEDIA_PROJECTION_RESULT == intent.action) {
            Log.d(TAG, "onStartCommand: handle media projection result")
            // Step 4 (optional): coming back from capturing permission check, now starting capturing machinery
            mResultCode = intent.getIntExtra(EXTRA_MEDIA_PROJECTION_RESULT_CODE, 0)
            mResultData = intent.getParcelableExtra(EXTRA_MEDIA_PROJECTION_RESULT_DATA)

            if (intent.getBooleanExtra(
                    EXTRA_MEDIA_PROJECTION_UPGRADING_FROM_FALLBACK_SCREEN_CAPTURE,
                    false
                )
            ) {
                // just restart screen capture
                stopScreenCapture()
                startScreenCapture()
            } else {
                val displayMetrics = getDisplayMetrics(this, Display.DEFAULT_DISPLAY)
                val port = PreferenceManager.getDefaultSharedPreferences(this).getInt(
                    PREFS_KEY_SERVER_LAST_PORT, mDefaults!!.port
                )
                // get device name
                val name = getDeviceName(this)

                val status = mDroidJni!!.vncStartServer(
                    displayMetrics.widthPixels,
                    displayMetrics.heightPixels,
                    port,
                    name!!,
                    PreferenceManager.getDefaultSharedPreferences(this).getString(
                        PREFS_KEY_SERVER_LAST_PASSWORD, mDefaults!!.password
                    )!!,
                    filesDir.absolutePath + File.separator + "novnc"
                )
                val answer = Intent(ACTION_START)
                answer.putExtra(
                    EXTRA_REQUEST_ID, PreferenceManager.getDefaultSharedPreferences(
                        this
                    ).getString(PREFS_KEY_SERVER_LAST_START_REQUEST_ID, null)
                )
                answer.putExtra(EXTRA_REQUEST_SUCCESS, status)
                sendBroadcastToOthersAndUs(answer)

                if (status) {
                    startScreenCapture()
                    updateNotification()
                    // if we got here, we want to restart if we were killed
                    return START_REDELIVER_INTENT
                } else {
                    stopSelfByUs()
                    return START_NOT_STICKY
                }
            }
        }

        if (ACTION_HANDLE_WRITE_STORAGE_RESULT == intent.action || ACTION_HANDLE_NOTIFICATION_RESULT == intent.action) {
            if (ACTION_HANDLE_WRITE_STORAGE_RESULT == intent.action) {
                Log.d(TAG, "onStartCommand: handle write storage result")
                // Step 3 on Android < 13: coming back from write storage permission check, start capturing
                // or ask for capturing permission first (then going in step 4)
            }
            if (ACTION_HANDLE_NOTIFICATION_RESULT == intent.action) {
                Log.d(TAG, "onStartCommand: handle notification result")
                // Step 3 on Android >= 13: coming back from notification permission check, start capturing
                // or ask for capturing permission first (then going in step 4)
            }

            if (mResultCode != 0 && mResultData != null
                || (Build.VERSION.SDK_INT >= 30 && PreferenceManager.getDefaultSharedPreferences(
                    this
                ).getBoolean(PREFS_KEY_SERVER_LAST_FALLBACK_SCREEN_CAPTURE, false))
            ) {
                val displayMetrics = getDisplayMetrics(this, Display.DEFAULT_DISPLAY)
                val port = PreferenceManager.getDefaultSharedPreferences(this).getInt(
                    PREFS_KEY_SERVER_LAST_PORT, mDefaults!!.port
                )
                val name = getDeviceName(this)
                val status = mDroidJni!!.vncStartServer(
                    displayMetrics.widthPixels,
                    displayMetrics.heightPixels,
                    port,
                    name!!,
                    PreferenceManager.getDefaultSharedPreferences(this).getString(
                        PREFS_KEY_SERVER_LAST_PASSWORD, mDefaults!!.password
                    )!!,
                    filesDir.absolutePath + File.separator + "novnc"
                )

                val answer = Intent(ACTION_START)
                answer.putExtra(
                    EXTRA_REQUEST_ID, PreferenceManager.getDefaultSharedPreferences(
                        this
                    ).getString(PREFS_KEY_SERVER_LAST_START_REQUEST_ID, null)
                )
                answer.putExtra(EXTRA_REQUEST_SUCCESS, status)
                sendBroadcastToOthersAndUs(answer)

                if (status) {
                    startScreenCapture()
                    updateNotification()
                    // if we got here, we want to restart if we were killed
                    return START_REDELIVER_INTENT
                } else {
                    stopSelfByUs()
                    return START_NOT_STICKY
                }
            } else {
                Log.i(TAG, "Requesting confirmation")
                // This initiates a prompt dialog for the user to confirm screen projection.
                val mediaProjectionRequestIntent =
                    Intent(this, MediaProjectionRequestActivity::class.java)
                mediaProjectionRequestIntent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK)
                startActivity(mediaProjectionRequestIntent)
                // if screen capturing was not started, we don't want a restart if we were killed
                // especially, we don't want the permission asking to replay.
                return START_NOT_STICKY
            }
        }

        if (ACTION_HANDLE_INPUT_RESULT == intent.action) {
            Log.d(TAG, "onStartCommand: handle input result")
            // Step 2: coming back from input permission check, now setup InputService and ask for write storage permission or notification permission
            InputService.isInputEnabled = intent.getBooleanExtra(EXTRA_INPUT_RESULT, false)
            if (Build.VERSION.SDK_INT < 33) {
                val writeStorageRequestIntent =
                    Intent(this, WriteStorageRequestActivity::class.java)
                writeStorageRequestIntent.putExtra(
                    EXTRA_FILE_TRANSFER,
                    PreferenceManager.getDefaultSharedPreferences(this).getBoolean(
                        PREFS_KEY_SERVER_LAST_FILE_TRANSFER, mDefaults!!.fileTransfer
                    )
                )
                writeStorageRequestIntent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK)
                startActivity(writeStorageRequestIntent)
            } else {
                val notificationRequestIntent =
                    Intent(this, NotificationRequestActivity::class.java)
                notificationRequestIntent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK)
                startActivity(notificationRequestIntent)
            }
            // if screen capturing was not started, we don't want a restart if we were killed
            // especially, we don't want the permission asking to replay.
            return START_NOT_STICKY
        }

        if (ACTION_START == intent.action) {
            Log.d(TAG, "onStartCommand: start")

            if (mDroidJni!!.vncIsActive()) {
                val answer = Intent(ACTION_START)
                answer.putExtra(EXTRA_REQUEST_ID, intent.getStringExtra(EXTRA_REQUEST_ID))
                answer.putExtra(EXTRA_REQUEST_SUCCESS, false)
                sendBroadcastToOthersAndUs(answer)
                return START_NOT_STICKY
            }

            // Step 0: persist given arguments to be able to recover from possible crash later
            val prefs = PreferenceManager.getDefaultSharedPreferences(this)
            val ed = prefs.edit()
            ed.putInt(
                PREFS_KEY_SERVER_LAST_PORT, intent.getIntExtra(
                    EXTRA_PORT, prefs.getInt(
                        Constants.PREFS_KEY_SETTINGS_PORT, mDefaults!!.port
                    )
                )
            )
            ed.putString(
                PREFS_KEY_SERVER_LAST_PASSWORD,
                if (intent.getStringExtra(EXTRA_PASSWORD) != null) intent.getStringExtra(
                    EXTRA_PASSWORD
                ) else prefs.getString(Constants.PREFS_KEY_SETTINGS_PASSWORD, mDefaults!!.password)
            )
            ed.putBoolean(
                PREFS_KEY_SERVER_LAST_FILE_TRANSFER, intent.getBooleanExtra(
                    EXTRA_FILE_TRANSFER,
                    prefs.getBoolean(
                        Constants.PREFS_KEY_SETTINGS_FILE_TRANSFER,
                        mDefaults!!.fileTransfer
                    )
                )
            )
            ed.putBoolean(
                Constants.PREFS_KEY_INPUT_LAST_ENABLED, !intent.getBooleanExtra(
                    EXTRA_VIEW_ONLY,
                    prefs.getBoolean(Constants.PREFS_KEY_SETTINGS_VIEW_ONLY, mDefaults!!.viewOnly)
                )
            )
            ed.putFloat(
                Constants.PREFS_KEY_SERVER_LAST_SCALING, intent.getFloatExtra(
                    EXTRA_SCALING, prefs.getFloat(
                        Constants.PREFS_KEY_SETTINGS_SCALING, mDefaults!!.scaling
                    )
                )
            )
            ed.putString(
                PREFS_KEY_SERVER_LAST_START_REQUEST_ID, intent.getStringExtra(
                    EXTRA_REQUEST_ID
                )
            )
            // showing pointers depends on view-only being false
            ed.putBoolean(
                PREFS_KEY_SERVER_LAST_SHOW_POINTERS,
                !intent.getBooleanExtra(
                    EXTRA_VIEW_ONLY,
                    prefs.getBoolean(Constants.PREFS_KEY_SETTINGS_VIEW_ONLY, mDefaults!!.viewOnly)
                )
                        && intent.getBooleanExtra(
                    EXTRA_SHOW_POINTERS,
                    prefs.getBoolean(
                        Constants.PREFS_KEY_SETTINGS_SHOW_POINTERS,
                        mDefaults!!.showPointers
                    )
                )
            )
            // using fallback screen capture depends on view-only being false
            ed.putBoolean(
                PREFS_KEY_SERVER_LAST_FALLBACK_SCREEN_CAPTURE,
                !intent.getBooleanExtra(
                    EXTRA_VIEW_ONLY,
                    prefs.getBoolean(Constants.PREFS_KEY_SETTINGS_VIEW_ONLY, mDefaults!!.viewOnly)
                )
                        && intent.getBooleanExtra(EXTRA_FALLBACK_SCREEN_CAPTURE, false)
            )
            ed.apply()
            // also set new value for InputService
            InputService.scaling = PreferenceManager.getDefaultSharedPreferences(
                this
            ).getFloat(
                Constants.PREFS_KEY_SERVER_LAST_SCALING, Defaults(
                    this
                ).scaling
            )

            // Step 1: check input/start-on-boot permission
            val inputRequestIntent = Intent(this, InputRequestActivity::class.java)
            inputRequestIntent.putExtra(
                EXTRA_VIEW_ONLY,
                intent.getBooleanExtra(EXTRA_VIEW_ONLY, mDefaults!!.viewOnly)
            )
            inputRequestIntent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK)
            startActivity(inputRequestIntent)
            // if screen capturing was not started, we don't want a restart if we were killed
            // especially, we don't want the permission asking to replay.
            return START_NOT_STICKY
        }

        if (ACTION_STOP == intent.action) {
            Log.d(TAG, "onStartCommand: stop")
            stopSelfByUs()
            val answer = Intent(ACTION_STOP)
            answer.putExtra(EXTRA_REQUEST_ID, intent.getStringExtra(EXTRA_REQUEST_ID))
            answer.putExtra(EXTRA_REQUEST_SUCCESS, mDroidJni!!.vncIsActive())
            sendBroadcastToOthersAndUs(answer)
            return START_NOT_STICKY
        }

        if (ACTION_CONNECT_REVERSE == intent.action) {
            Log.d(
                TAG, "onStartCommand: connect reverse, id " + intent.getStringExtra(
                    EXTRA_REQUEST_ID
                )
            )
            if (mDroidJni!!.vncIsActive()) {
                // run on worker thread
                Thread {
                    var client: Long = 0
                    try {
                        client = instance!!.mDroidJni!!.vncConnectReverse(
                            intent.getStringExtra(
                                EXTRA_HOST
                            )!!, intent.getIntExtra(EXTRA_PORT, mDefaults!!.portReverse)
                        )
                    } catch (ignored: NullPointerException) {
                    }
                    val answer = Intent(ACTION_CONNECT_REVERSE)
                    answer.putExtra(EXTRA_REQUEST_ID, intent.getStringExtra(EXTRA_REQUEST_ID))
                    answer.putExtra(EXTRA_REQUEST_SUCCESS, client != 0L)
                    sendBroadcastToOthersAndUs(answer)
                    // check if set to reconnect and handle accordingly
                    handleClientReconnect(intent, client, "reverse")
                }.start()
            } else {
                stopSelfByUs()
            }

            return START_NOT_STICKY
        }

        if (ACTION_CONNECT_REPEATER == intent.action) {
            Log.d(
                TAG, "onStartCommand: connect repeater, id " + intent.getStringExtra(
                    EXTRA_REQUEST_ID
                )
            )

            if (mDroidJni!!.vncIsActive()) {
                // run on worker thread
                Thread {
                    var client: Long = 0
                    try {
                        client = instance!!.mDroidJni!!.vncConnectRepeater(
                            intent.getStringExtra(EXTRA_HOST)!!,
                            intent.getIntExtra(EXTRA_PORT, mDefaults!!.portRepeater),
                            intent.getStringExtra(EXTRA_REPEATER_ID)!!
                        )
                    } catch (ignored: NullPointerException) {
                    }
                    val answer = Intent(ACTION_CONNECT_REPEATER)
                    answer.putExtra(EXTRA_REQUEST_ID, intent.getStringExtra(EXTRA_REQUEST_ID))
                    answer.putExtra(EXTRA_REQUEST_SUCCESS, client != 0L)
                    sendBroadcastToOthersAndUs(answer)
                    // check if set to reconnect and handle accordingly
                    handleClientReconnect(intent, client, "repeater")
                }.start()
            } else {
                stopSelfByUs()
            }

            return START_NOT_STICKY
        }

        // no known action was given, stop the _service_ again if the _server_ is not active
        if (!mDroidJni!!.vncIsActive()) {
            stopSelfByUs()
        }

        return START_NOT_STICKY
    }

    private fun handleClientReconnect(reconnectIntent: Intent, client: Long, logTag: String) {
        //        final Intent intent = new IntentSanitizer.Builder()
//                .allowComponent(new ComponentName(this, getClass()))
//                .allowAction(ACTION_CONNECT_REPEATER)
//                .allowAction(ACTION_CONNECT_REVERSE)
//                .allowExtra(EXTRA_REQUEST_ID, String.class)
//                .allowExtra(EXTRA_RECONNECT_TRIES, Integer.class)
//                .allowExtra(EXTRA_ACCESS_KEY, String.class)
//                .allowExtra(EXTRA_HOST, String.class)
//                .allowExtra(EXTRA_PORT, Integer.class)
//                .allowExtra(EXTRA_REPEATER_ID, String.class)
//                .build()
//                .sanitizeByFiltering(reconnectIntent);

        val intent = Intent()

        val requestId = intent.getStringExtra(EXTRA_REQUEST_ID)
        if (intent.getIntExtra(EXTRA_RECONNECT_TRIES, 0) > 0 && requestId != null) {
            if (client != 0L) {
                Log.d(
                    TAG, "handleClientReconnect: "
                            + logTag
                            + ": request id "
                            + intent.getStringExtra(EXTRA_REQUEST_ID)
                            + " successfully (re)connected"
                )
                // connection successful, save Intent and client for later and set backoff and tries-left to init values
                val data = OutboundClientReconnectData()
                data.intent = intent
                data.client = client
                data.backoff = OutboundClientReconnectData.BACKOFF_INIT
                data.reconnectTriesLeft = intent.getIntExtra(EXTRA_RECONNECT_TRIES, 0)
                mOutboundClientsToReconnect[requestId] = data
            } else {
                // connection fail, check if entry in reconnect list
                val reconnectData: OutboundClientReconnectData? =
                    mOutboundClientsToReconnect[requestId]

                // also, get exact key reference (needed for cancellation at the Handler, as this does a "==" comparison, not .equals())
                val key: String? = mOutboundClientsToReconnect.entries
                    .firstOrNull { entry -> entry.value == reconnectData }
                    ?.key


                if (reconnectData != null && key != null) {
                    // if we come here, there was already 1 reconnect from the client disconnect handler.
                    // thus unset client, decrease reconnect tries, increase backoff
                    reconnectData.client = 0
                    reconnectData.reconnectTriesLeft--
                    reconnectData.backoff = min(
                        (reconnectData.backoff * 2).toDouble(),
                        OutboundClientReconnectData.BACKOFF_LIMIT.toDouble()
                    )
                        .toInt()
                    // then check if reconnect tries left
                    if (reconnectData.reconnectTriesLeft > 0) {
                        // yes, fire up another reconnect action
                        Log.d(
                            TAG, "handleClientReconnect: "
                                    + logTag
                                    + ": request id "
                                    + intent.getStringExtra(EXTRA_REQUEST_ID)
                                    + " reconnect failed, has "
                                    + reconnectData.reconnectTriesLeft
                                    + " reconnect tries left, reconnecting with delay of "
                                    + reconnectData.backoff
                                    + " seconds"
                        )
                        mOutboundClientReconnectHandler.postAtTime(
                            {
                                if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
                                    startForegroundService(intent)
                                } else {
                                    startService(intent)
                                }
                            },
                            key,  // important to use exact key reference here, see above!
                            SystemClock.uptimeMillis() + reconnectData.backoff * 1000L
                        )
                    } else {
                        // no, delete entry
                        Log.d(
                            TAG, "handleClientReconnect: "
                                    + logTag
                                    + ": request id "
                                    + intent.getStringExtra(EXTRA_REQUEST_ID)
                                    + "reconnect failed, exceeded reconnect tries, removing from reconnect list"
                        )
                        mOutboundClientsToReconnect.remove(requestId)
                    }
                }
            }
        }
    }

    private fun startScreenCapture() {
        if (mResultCode != 0 && mResultData != null) {
            Log.d(TAG, "startScreenCapture: using MediaProjection backend")
            val intent = Intent(this, MediaProjectionService::class.java)
            intent.putExtra(EXTRA_MEDIA_PROJECTION_RESULT_CODE, mResultCode)
            intent.putExtra(EXTRA_MEDIA_PROJECTION_RESULT_DATA, mResultData)

            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
                startForegroundService(intent)
            } else {
                startService(intent)
            }
        } else {
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) {
                Log.d(TAG, "startScreenCapture: trying takeScreenShot backend")
                InputService.takeScreenShots(true)
            } else {
                Log.w(TAG, "startScreenCapture: no backend available")
            }
        }
    }

    private fun stopScreenCapture() {
        // stop all backends unconditionally
        stopService(Intent(this, MediaProjectionService::class.java))
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) {
            InputService.takeScreenShots(false)
        }
    }

    /**
     * Wrapper around stopSelf() that indicates that the stop was on our initiative.
     */
    private fun stopSelfByUs() {
        mIsStoppingByUs = true
        stopSelf()
    }

    private fun sendBroadcastToOthersAndUs(intent: Intent) {
        // Send implicit Intent to others (and us on API level < 34).
        sendBroadcast(intent)
        // Send explicit Intent to us, i.e. MainActivity which has a not-exported broadcast receiver
        // and needs an explicit Intent from Android 14 on as per https://developer.android.com/about/versions/14/behavior-changes-14#runtime-receivers-exported
        // This will not be delivered to others as per https://developer.android.com/develop/background-work/background-tasks/broadcasts#security-and-best-practices
        if (Build.VERSION.SDK_INT >= 34) {
            intent.setPackage(packageName)
            sendBroadcast(intent)
        }
    }

    @SuppressLint("WrongConstant")
    private fun getNotification(text: String?, isSilent: Boolean): Notification {
        val notificationIntent = Intent(this, HomeActivity::class.java)

        val pendingIntent = PendingIntent.getActivity(
            this, 0,
            notificationIntent, PendingIntent.FLAG_IMMUTABLE
        )

        val builder = NotificationCompat.Builder(this, packageName)
            .setSmallIcon(R.drawable.ic_notification)
            .setContentTitle(getString(R.string.app_name))
            .setContentText(text)
            .setSilent(isSilent)
            .setOngoing(true)
            .setContentIntent(pendingIntent)
        if (Build.VERSION.SDK_INT >= 31) {
            builder.setForegroundServiceBehavior(NotificationCompat.FOREGROUND_SERVICE_IMMEDIATE)
        }

        mNotification = builder.build()
        return mNotification!!
    }

    private fun updateNotification() {
        val port = PreferenceManager.getDefaultSharedPreferences(this).getInt(
            PREFS_KEY_SERVER_LAST_PORT, mDefaults!!.port
        )
        if (port < 0) {
            (getSystemService(NOTIFICATION_SERVICE) as NotificationManager)
                .notify(
                    NOTIFICATION_ID,
                    getNotification(
                        getString(R.string.main_service_notification_not_listening),
                        false
                    )
                )
        } else {
            (getSystemService(NOTIFICATION_SERVICE) as NotificationManager)
                .notify(
                    NOTIFICATION_ID,
                    getNotification(
                        resources.getQuantityString(
                            R.plurals.main_service_notification_listening,
                            mNumberOfClients,
                            port,
                            mNumberOfClients
                        ),
                        false
                    )
                )
        }
    }

    companion object {
        private const val TAG = "MainService"
        const val NOTIFICATION_ID: Int = 11
        const val ACTION_START: String = "com.lingmo.assistant.ACTION_START"
        const val ACTION_STOP: String = "com.lingmo.assistant.ACTION_STOP"
        const val ACTION_CONNECT_REVERSE: String = "com.lingmo.assistant.ACTION_CONNECT_REVERSE"
        const val ACTION_CONNECT_REPEATER: String = "com.lingmo.assistant.ACTION_CONNECT_REPEATER"
        const val EXTRA_REQUEST_ID: String = "com.lingmo.assistant.EXTRA_REQUEST_ID"
        const val EXTRA_REQUEST_SUCCESS: String = "com.lingmo.assistant.EXTRA_REQUEST_SUCCESS"
        const val EXTRA_HOST: String = "com.lingmo.assistant.EXTRA_HOST"
        const val EXTRA_PORT: String = "com.lingmo.assistant.EXTRA_PORT"
        const val EXTRA_REPEATER_ID: String = "com.lingmo.assistant.EXTRA_REPEATER_ID"
        const val EXTRA_RECONNECT_TRIES: String = "com.lingmo.assistant.EXTRA_RECONNECT_TRIES"
        const val EXTRA_ACCESS_KEY: String = "com.lingmo.assistant.EXTRA_ACCESS_KEY"
        const val EXTRA_PASSWORD: String = "com.lingmo.assistant.EXTRA_PASSWORD"
        const val EXTRA_VIEW_ONLY: String = "com.lingmo.assistant.EXTRA_VIEW_ONLY"
        const val EXTRA_SHOW_POINTERS: String = "com.lingmo.assistant.EXTRA_SHOW_POINTERS"
        const val EXTRA_SCALING: String = "com.lingmo.assistant.EXTRA_SCALING"

        /**
         * Only used on Android 12 and earlier.
         */
        const val EXTRA_FILE_TRANSFER: String = "com.lingmo.assistant.EXTRA_FILE_TRANSFER"

        /**
         * Only used on Android 10 and later.
         */
        const val EXTRA_FALLBACK_SCREEN_CAPTURE: String =
            "com.lingmo.assistant.EXTRA_FALLBACK_SCREEN_CAPTURE"

        const val ACTION_HANDLE_MEDIA_PROJECTION_RESULT: String =
            "action_handle_media_projection_result"
        const val EXTRA_MEDIA_PROJECTION_RESULT_DATA: String = "result_data_media_projection"
        const val EXTRA_MEDIA_PROJECTION_RESULT_CODE: String = "result_code_media_projection"
        const val EXTRA_MEDIA_PROJECTION_UPGRADING_FROM_FALLBACK_SCREEN_CAPTURE: String =
            "upgrading_from_fallback_screen_capture"

        const val ACTION_HANDLE_INPUT_RESULT: String = "action_handle_a11y_result"
        const val EXTRA_INPUT_RESULT: String = "result_a11y"

        const val ACTION_HANDLE_WRITE_STORAGE_RESULT: String = "action_handle_write_storage_result"
        const val EXTRA_WRITE_STORAGE_RESULT: String = "result_write_storage"

        const val ACTION_HANDLE_NOTIFICATION_RESULT: String = "action_handle_notification_result"

        private const val PREFS_KEY_SERVER_LAST_PORT = "server_last_port"
        private const val PREFS_KEY_SERVER_LAST_PASSWORD = "server_last_password"
        private const val PREFS_KEY_SERVER_LAST_FILE_TRANSFER = "server_last_file_transfer"
        private const val PREFS_KEY_SERVER_LAST_SHOW_POINTERS = "server_last_show_pointers"
        private const val PREFS_KEY_SERVER_LAST_FALLBACK_SCREEN_CAPTURE =
            "server_last_fallback_screen_capture"
        private const val PREFS_KEY_SERVER_LAST_START_REQUEST_ID = "server_last_start_request_id"

        private var instance: MainService? = null

        fun isServerActive(): Boolean {
            return try {
                instance!!.mDroidJni!!.vncIsActive()
            } catch (ignored: java.lang.Exception) {
                false
            }
        }

        /**
         * Get non-loopback IPv4 addresses.
         * @return A list of strings, each containing one IPv4 address.
         */
        fun getIPv4s(): ArrayList<String> {
            val hosts = ArrayList<String>()

            // if running on Chrome OS, this prop is set and contains the device's IPv4 address,
            // see https://chromeos.dev/en/games/optimizing-games-networking
            val prop: String = Utils.getProp("arc.net.ipv4.host_address")
            if (!prop.isEmpty()) {
                hosts.add(prop)
                return hosts
            }

            // not running on Chrome OS
            try {
                // thanks go to https://stackoverflow.com/a/20103869/361413
                val nis = NetworkInterface.getNetworkInterfaces()
                var ni: NetworkInterface
                while (nis.hasMoreElements()) {
                    ni = nis.nextElement()
                    if (!ni.isLoopback /*not loopback*/ && ni.isUp /*it works now*/) {
                        for (ia in ni.interfaceAddresses) {
                            //filter for ipv4/ipv6
                            if (ia.address.address.size == 4) {
                                //4 for ipv4, 16 for ipv6
                                hosts.add(ia.address.toString().replace("/".toRegex(), ""))
                            }
                        }
                    }
                }
            } catch (e: SocketException) {
                //unused
            }

            return hosts
        }

        fun getPort(): Int {
            try {
                val prefs = PreferenceManager.getDefaultSharedPreferences(
                    instance!!
                )
                return prefs.getInt(
                    PREFS_KEY_SERVER_LAST_PORT, Defaults(
                        instance!!
                    ).port
                )
            } catch (e: java.lang.Exception) {
                return -2
            }
        }

        @JvmStatic
        val currentNotification: Notification?
            get() = try {
                instance!!.mNotification
            } catch (ignored: Exception) {
                null
            }

        @JvmStatic
        fun vncGetFramebufferWidth(): Int {
            try {
                return instance!!.mDroidJni!!.vncGetFramebufferWidth()
            } catch (e: Exception) {
                throw RuntimeException(e)
            }
        }

        @JvmStatic
        fun vncGetFramebufferHeight(): Int {
            try {
                return instance!!.mDroidJni!!.vncGetFramebufferHeight()
            } catch (e: Exception) {
                throw RuntimeException(e)
            }
        }

        @JvmStatic
        fun vncNewFramebuffer(w: Int, h: Int): Boolean {
            try {
                return instance!!.mDroidJni!!.vncNewFramebuffer(w, h)
            } catch (e: Exception) {
                throw RuntimeException(e)
            }
        }

        @JvmStatic
        fun vncUpdateFramebuffer(buffer: ByteBuffer?): Boolean {
            try {
                return instance!!.mDroidJni!!.vncUpdateFramebuffer(buffer)
            } catch (e: Exception) {
                throw RuntimeException(e)
            }
        }
    }
}
