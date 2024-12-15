package com.lingmo.assistant.utils

import android.content.Context
import android.hardware.display.DisplayManager
import android.net.ConnectivityManager
import android.net.NetworkCapabilities
import android.net.wifi.WifiManager
import android.os.Build
import android.provider.Settings
import android.text.format.Formatter
import android.util.DisplayMetrics
import com.lingmo.assistant.R
import java.io.BufferedReader
import java.io.File
import java.io.FileOutputStream
import java.io.IOException
import java.io.InputStream
import java.io.InputStreamReader

object Utils {

    @JvmStatic
    fun getProp(prop: String) : String {
        val process = ProcessBuilder().command("/system/bin/getprop", prop).start()
        return BufferedReader(InputStreamReader(process.inputStream)).readLine()
    }

    @JvmStatic
    fun getDisplayMetrics(ctx: Context, displayId: Int): DisplayMetrics {
        val displayMetrics = DisplayMetrics()
        val dm = ctx.getSystemService(Context.DISPLAY_SERVICE) as DisplayManager
        dm.getDisplay(displayId).getRealMetrics(displayMetrics)
        return displayMetrics
    }

    @JvmStatic
    fun getDeviceName(ctx: Context): String? {
        // get device name
        return try {
            // This is what we had until targetSDK 33.
            Settings.Secure.getString(ctx.contentResolver, "bluetooth_name")
        } catch (ignored: SecurityException) {
            // throws on devices with API level 33, so use fallback
            if (Build.VERSION.SDK_INT > 25) {
                Settings.Global.getString(ctx.contentResolver, Settings.Global.DEVICE_NAME)
            } else {
                Build.MANUFACTURER
            }
        }
    }

    @JvmStatic
    fun Context.copyAssetsToDir(assetDir: String, outDir: String) {
        val assetManager = assets
        val outDirFile = File(outDir)

        try {
            val files = assetManager.list(assetDir) ?: return

            // Ensure the output directory exists
            if (!outDirFile.exists()) {
                outDirFile.mkdirs()
            }

            for (filename in files) {
                val assetPath = "$assetDir/$filename"
                val outFile = File(outDirFile, filename)

                // Check if this is a directory or a file
                if (assetManager.list(assetPath)?.isNotEmpty() == true) {
                    // If it's a directory, recursively copy its contents
                    copyAssetsToDir(assetPath, outFile.absolutePath)
                } else {
                    // If it's a file, copy it
                    val inStream: InputStream = assetManager.open(assetPath)
                    val outStream = FileOutputStream(outFile)

                    val buffer = ByteArray(1024)
                    var bytesRead: Int
                    while (inStream.read(buffer).also { bytesRead = it } != -1) {
                        outStream.write(buffer, 0, bytesRead)
                    }

                    inStream.close()
                    outStream.flush()
                    outStream.close()
                }
            }
        } catch (e: IOException) {
            e.printStackTrace()
        }
    }

    @JvmStatic
    fun deleteRecursively(directory: String) {
        val directory = File(directory)
        directory.deleteRecursively()
    }

    @JvmStatic
   fun getIPAddress(context: Context): String? {
        val connectivityManager = context.getSystemService(Context.CONNECTIVITY_SERVICE) as ConnectivityManager

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
            val activeNetwork = connectivityManager.activeNetwork
            val networkCapabilities = connectivityManager.getNetworkCapabilities(activeNetwork)

            return when {
                networkCapabilities?.hasTransport(NetworkCapabilities.TRANSPORT_WIFI) == true -> {
                    val wifiManager = context.applicationContext.getSystemService(Context.WIFI_SERVICE) as WifiManager
                    val ipAddress = wifiManager.connectionInfo.ipAddress
                    Formatter.formatIpAddress(ipAddress) // 格式化IP地址
                }
                networkCapabilities?.hasTransport(NetworkCapabilities.TRANSPORT_CELLULAR) == true -> {
                    null // 返回null，或根据需要添加逻辑
                }
                else -> null // 无网络连接
            }
        } else {
            return null
        }
    }
}