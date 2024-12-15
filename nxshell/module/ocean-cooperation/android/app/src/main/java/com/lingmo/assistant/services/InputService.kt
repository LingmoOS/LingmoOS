// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package com.lingmo.assistant.services

import android.accessibilityservice.AccessibilityService
import android.accessibilityservice.GestureDescription
import android.accessibilityservice.GestureDescription.StrokeDescription
import android.content.ClipData
import android.content.ClipboardManager
import android.graphics.Bitmap
import android.graphics.Path
import android.os.Build
import android.os.Bundle
import android.os.Handler
import android.util.DisplayMetrics
import android.util.Log
import android.view.Display
import android.view.ViewConfiguration
import android.view.WindowManager
import android.view.accessibility.AccessibilityEvent
import android.view.accessibility.AccessibilityNodeInfo
import androidx.annotation.RequiresApi
import androidx.preference.PreferenceManager
import com.lingmo.assistant.services.MainService.Companion.vncGetFramebufferHeight
import com.lingmo.assistant.services.MainService.Companion.vncGetFramebufferWidth
import com.lingmo.assistant.services.MainService.Companion.vncNewFramebuffer
import com.lingmo.assistant.services.MainService.Companion.vncUpdateFramebuffer
import net.christianbeier.droidvnc_ng.DroidManager.InputListener
import net.christianbeier.droidvnc_ng.JniDroidVnc.Companion.registerInputListener
import java.nio.ByteBuffer
import java.util.Objects
import java.util.concurrent.ConcurrentHashMap
import kotlin.math.max

class InputService : AccessibilityService() {
    /**
     * This tracks gesture completion per client.
     */
    private class GestureCallback : GestureResultCallback() {
        var mCompleted: Boolean = true // initially true so we can actually dispatch something

        @Synchronized
        override fun onCompleted(gestureDescription: GestureDescription) {
            mCompleted = true
        }

        @Synchronized
        override fun onCancelled(gestureDescription: GestureDescription) {
            mCompleted = true
        }
    }

    /**
     * Per-client input context.
     */
    private class InputContext {
        // pointer-related
        var isButtonOneDown: Boolean = false
        var path: Path = Path()
        var lastGestureStartTime: Long = 0
        var gestureCallback: GestureCallback = GestureCallback()
        var pointerView: InputPointerView? = null

        // keyboard-related
        var isKeyCtrlDown: Boolean = false
        var isKeyAltDown: Boolean = false
        var isKeyShiftDown: Boolean = false
        var isKeyDelDown: Boolean = false
        var isKeyEscDown: Boolean = false

        var displayId: Int = 0
            set(value) {
                // 在这里可以添加额外的逻辑，比如检查新值是否合法
                field = value  // 使用 field 关键字将值赋给后备字段

                // 如果 pointerView 不为空，则重新创建 pointerView
                if (pointerView != null) {
                    pointerView!!.removeView()
                    pointerView = InputPointerView(
                        instance!!,
                        value,
                        pointerView!!.red,
                        pointerView!!.green,
                        pointerView!!.blue
                    )
                    pointerView!!.addView()
                }
            }
    }

    private var mTakeScreenShots = false
    private var mTakeScreenShotDelayMs = 100

    private var mMainHandler: Handler? = null

    private val mInputContexts: MutableMap<Long, InputContext> = ConcurrentHashMap()

    /**
     * System keyboard input foci, display-specific starting on Android 10, see [Android docs](https://source.android.com/docs/core/display/multi_display/displays#focus)
     */
    private val mKeyboardFocusNodes: MutableMap<Int, AccessibilityNodeInfo?> = ConcurrentHashMap()


    override fun onAccessibilityEvent(event: AccessibilityEvent) {
        try {
            Log.d(TAG, "onAccessibilityEvent: $event")
            val displayId = if (Build.VERSION.SDK_INT >= 30) {
                event.source?.let {
                    // be display-specific
                    it.window.displayId
                } ?: Display.DEFAULT_DISPLAY // 如果源为null，则使用默认显示
            } else {
                // assume default display
                Display.DEFAULT_DISPLAY
            }

            // recycle old node if there
            val previousFocusNode = mKeyboardFocusNodes[displayId]
            if (previousFocusNode != null) {
                try {
                    previousFocusNode.recycle()
                } catch (e: Exception) {
                    Log.i(TAG, "onAccessibilityEvent: could not recycle previousFocusNode: $e")
                }
            }

            // and put new one
            event.source?.let {
                mKeyboardFocusNodes[displayId] = it
            }
        } catch (e: Exception) {
            Log.e(TAG, "onAccessibilityEvent: " + Log.getStackTraceString(e))
        }
    }

    override fun onInterrupt() {}

    public override fun onServiceConnected() {
        super.onServiceConnected()
        instance = this
        isInputEnabled = PreferenceManager.getDefaultSharedPreferences(
            this
        ).getBoolean(
            Constants.PREFS_KEY_INPUT_LAST_ENABLED, !Defaults(
                this
            ).viewOnly
        )
        scaling = PreferenceManager.getDefaultSharedPreferences(
            this
        ).getFloat(
            Constants.PREFS_KEY_SERVER_LAST_SCALING, Defaults(
                this
            ).scaling
        )
        mMainHandler = Handler(instance!!.mainLooper)

        // register the input callback by static method
        registerInputListener(mDroidInputListener)
        Log.i(TAG, "onServiceConnected")
    }

    override fun onDestroy() {
        super.onDestroy()
        instance = null
        Log.i(TAG, "onDestroy")
    }

    private val mDroidInputListener: InputListener = object : InputListener {
        override fun onInputPointerEvent(buttonMask: Int, x: Int, y: Int, client: Long) {
            var x = x
            var y = y
            if (!isInputEnabled) {
                return
            }

            try {
                val inputContext = instance!!.mInputContexts[client]
                    ?: throw IllegalStateException("Client $client was not aoceand or is already removed")

                x = (x / scaling).toInt()
                y = (y / scaling).toInt()

                /*
				draw pointer
			 */
                val pointerView = inputContext.pointerView
                if (pointerView != null) {
                    // showing pointers is enabled
                    val finalX = x
                    val finalY = y
                    pointerView.post(Runnable { pointerView.positionView(finalX, finalY) })
                }

                /*
			    left mouse button
			 */

                // down, was up
                if ((buttonMask and (1 shl 0)) != 0 && !inputContext.isButtonOneDown) {
                    inputContext.isButtonOneDown = true
                    instance!!.startGesture(inputContext, x, y)
                }

                // down, was down
                if ((buttonMask and (1 shl 0)) != 0 && inputContext.isButtonOneDown) {
                    instance!!.continueGesture(inputContext, x, y)
                }

                // up, was down
                if ((buttonMask and (1 shl 0)) == 0 && inputContext.isButtonOneDown) {
                    inputContext.isButtonOneDown = false
                    instance!!.endGesture(inputContext, x, y)
                }


                // right mouse button
                if ((buttonMask and (1 shl 2)) != 0) {
                    instance!!.longPress(inputContext, x, y)
                }

                // scroll up
                if ((buttonMask and (1 shl 3)) != 0) {
                    val displayMetrics = DisplayMetrics()
                    val wm =
                        instance!!.applicationContext.getSystemService(WINDOW_SERVICE) as WindowManager
                    wm.defaultDisplay.getRealMetrics(displayMetrics)

                    instance!!.scroll(inputContext, x, y, -displayMetrics.heightPixels / 2)
                }

                // scroll down
                if ((buttonMask and (1 shl 4)) != 0) {
                    val displayMetrics = DisplayMetrics()
                    val wm =
                        instance!!.applicationContext.getSystemService(WINDOW_SERVICE) as WindowManager
                    wm.defaultDisplay.getRealMetrics(displayMetrics)

                    instance!!.scroll(inputContext, x, y, displayMetrics.heightPixels / 2)
                }
            } catch (e: Exception) {
                // instance probably null
                Log.e(TAG, "onPointerEvent: failed: " + Log.getStackTraceString(e))
            }
        }

        override fun onInputKeyEvent(down: Int, keysym: Long, client: Long) {
            if (!isInputEnabled) {
                return
            }

            Log.d(TAG, "onKeyEvent: keysym $keysym down $down by client $client")

            try {
                val inputContext = instance!!.mInputContexts[client]
                    ?: throw IllegalStateException("Client $client was not aoceand or is already removed")

                /*
				Save states of some keys for combo handling.
			 */
                if (keysym == 0xFFE3L) inputContext.isKeyCtrlDown = down != 0

                if (keysym == 0xFFE9L || keysym == 0xFF7EL) // MacOS clients send Alt as 0xFF7E
                    inputContext.isKeyAltDown = down != 0

                if (keysym == 0xFFE1L) inputContext.isKeyShiftDown = down != 0

                if (keysym == 0xFFFFL) inputContext.isKeyDelDown = down != 0

                if (keysym == 0xFF1BL) inputContext.isKeyEscDown = down != 0

                /*
				Ctrl-Alt-Del combo.
		 	*/
                if (inputContext.isKeyCtrlDown && inputContext.isKeyAltDown && inputContext.isKeyDelDown) {
                    Log.i(TAG, "onKeyEvent: got Ctrl-Alt-Del")
                    instance!!.mMainHandler!!.post(Runnable { MediaProjectionService.togglePortraitInLandscapeWorkaround() })
                }

                /*
				Pageup(65365) || Ctrl-Shift-Esc combo.
		 	*/
                if ((keysym == 0xFF55L && down != 0) || (inputContext.isKeyCtrlDown && inputContext.isKeyShiftDown && inputContext.isKeyEscDown)) {
                    Log.i(TAG, "onKeyEvent: got Ctrl-Shift-Esc")
                    instance!!.performGlobalAction(GLOBAL_ACTION_RECENTS)
                }

                /*
				Home/Pos1
		 	*/
                if (keysym == 0xFF50L && down != 0) {
                    Log.i(TAG, "onKeyEvent: got Home/Pos1")
                    instance!!.performGlobalAction(GLOBAL_ACTION_HOME)
                }

                /*
				End
			*/
                if (keysym == 0xFF57L && down != 0) {
                    Log.i(TAG, "onKeyEvent: got End")
                    instance!!.performGlobalAction(GLOBAL_ACTION_POWER_DIALOG)
                }

                /*
				Esc
			 */
                if (keysym == 0xFF1BL && down != 0) {
                    Log.i(TAG, "onKeyEvent: got Esc")
                    instance!!.performGlobalAction(GLOBAL_ACTION_BACK)
                }

                /*
				Get current keyboard focus node for input context's display.
			 */
                val currentFocusNode = instance?.mKeyboardFocusNodes?.get(inputContext.displayId)
                requireNotNull(currentFocusNode) { "Current focus node cannot be null" }
                // refresh() is important to load the represented view's current text into the node
                currentFocusNode.refresh()

                /*
			   Left/Right
			 */
                if ((keysym == 0xff51L || keysym == 0xff53L) && down != 0) {
                    val action = Bundle()
                    action.putInt(
                        AccessibilityNodeInfo.ACTION_ARGUMENT_MOVEMENT_GRANULARITY_INT,
                        AccessibilityNodeInfo.MOVEMENT_GRANULARITY_CHARACTER
                    )
                    action.putBoolean(
                        AccessibilityNodeInfo.ACTION_ARGUMENT_EXTEND_SELECTION_BOOLEAN,
                        false
                    )
                    if (keysym == 0xff51L) Objects.requireNonNull(currentFocusNode).performAction(
                        AccessibilityNodeInfo.AccessibilityAction.ACTION_PREVIOUS_AT_MOVEMENT_GRANULARITY.id,
                        action
                    )
                    else Objects.requireNonNull(currentFocusNode).performAction(
                        AccessibilityNodeInfo.AccessibilityAction.ACTION_NEXT_AT_MOVEMENT_GRANULARITY.id,
                        action
                    )
                }

                /*
			    Backspace/Delete
			    TODO: implement deletions of text selections, right now it's only 1 char at a time
			 */
                if ((keysym == 0xff08L || keysym == 0xffffL) && down != 0) {
                    val currentFocusText = Objects.requireNonNull(currentFocusNode).text
                    val cursorPos = getCursorPos(currentFocusNode)

                    // set new text
                    val newFocusText = if (keysym == 0xff08L) {
                        // backspace
                        currentFocusText.subSequence(0, cursorPos - 1)
                            .toString() + currentFocusText.subSequence(
                            cursorPos,
                            currentFocusText.length
                        )
                    } else {
                        // delete
                        currentFocusText.subSequence(0, cursorPos)
                            .toString() + currentFocusText.subSequence(
                            cursorPos + 1,
                            currentFocusText.length
                        )
                    }
                    val action = Bundle()
                    action.putCharSequence(
                        AccessibilityNodeInfo.ACTION_ARGUMENT_SET_TEXT_CHARSEQUENCE,
                        newFocusText
                    )
                    currentFocusNode!!.performAction(
                        AccessibilityNodeInfo.AccessibilityAction.ACTION_SET_TEXT.id,
                        action
                    )

                    // ACTION_SET_TEXT moves cursor to the end, move cursor back to where it should be
                    setCursorPos(
                        currentFocusNode,
                        if (keysym == 0xff08L) cursorPos - 1 else cursorPos
                    )
                }

                /*
			   Insert
			 */
                if (keysym == 0xff63L && down != 0) {
                    val action = Bundle()
                    Objects.requireNonNull(currentFocusNode).performAction(
                        AccessibilityNodeInfo.AccessibilityAction.ACTION_PASTE.id,
                        action
                    )
                }

                /*
			    Enter, for API level 30+
			 */
                if (keysym == 0xff0dL && down != 0) {
                    if (Build.VERSION.SDK_INT >= 30) {
                        val action = Bundle()
                        Objects.requireNonNull(currentFocusNode).performAction(
                            AccessibilityNodeInfo.AccessibilityAction.ACTION_IME_ENTER.id,
                            action
                        )
                    }
                }

                /*
			    ISO-8859-1 input
			 */
                if (keysym >= 32 && keysym <= 255 && down != 0) {
                    var currentFocusText = Objects.requireNonNull(currentFocusNode).text
                    // some implementations return null for empty text, work around that
                    if (currentFocusText == null) currentFocusText = ""

                    val cursorPos = getCursorPos(currentFocusNode)

                    // set new text
                    var textBeforeCursor = ""
                    try {
                        textBeforeCursor = currentFocusText.subSequence(0, cursorPos).toString()
                    } catch (ignored: IndexOutOfBoundsException) {
                    }
                    var textAfterCursor = ""
                    try {
                        textAfterCursor =
                            currentFocusText.subSequence(cursorPos, currentFocusText.length)
                                .toString()
                    } catch (ignored: IndexOutOfBoundsException) {
                    }
                    val newFocusText = textBeforeCursor + Char(keysym.toUShort()) + textAfterCursor

                    val action = Bundle()
                    action.putCharSequence(
                        AccessibilityNodeInfo.ACTION_ARGUMENT_SET_TEXT_CHARSEQUENCE,
                        newFocusText
                    )
                    currentFocusNode!!.performAction(
                        AccessibilityNodeInfo.AccessibilityAction.ACTION_SET_TEXT.id,
                        action
                    )

                    // ACTION_SET_TEXT moves cursor to the end, move cursor back to where it should be
                    setCursorPos(currentFocusNode, if (cursorPos > 0) cursorPos + 1 else 1)
                }
            } catch (e: Exception) {
                // instance probably null
                Log.e(TAG, "onKeyEvent: failed: $e")
            }
        }

        override fun onInputCutText(text: String?, client: Long) {
            if (!isInputEnabled) {
                return
            }

            Log.d(TAG, "onCutText: text '$text' by client $client")

            try {
                instance!!.mMainHandler!!.post {
                    try {
                        (instance!!.getSystemService(CLIPBOARD_SERVICE) as ClipboardManager).setPrimaryClip(
                            ClipData.newPlainText(text, text)
                        )
                    } catch (e: Exception) {
                        // some other error on main thread
                        Log.e(TAG, "onCutText: failed: $e")
                    }
                }
            } catch (e: Exception) {
                // instance probably null
                Log.e(TAG, "onCutText: failed: $e")
            }
        }
    }

    private fun startGesture(inputContext: InputContext, x: Int, y: Int) {
        inputContext.path.reset()
        inputContext.path.moveTo(x.toFloat(), y.toFloat())
        inputContext.lastGestureStartTime = System.currentTimeMillis()
    }

    private fun continueGesture(inputContext: InputContext, x: Int, y: Int) {
        inputContext.path.lineTo(x.toFloat(), y.toFloat())
    }

    private fun endGesture(inputContext: InputContext, x: Int, y: Int) {
        inputContext.path.lineTo(x.toFloat(), y.toFloat())
        var duration = System.currentTimeMillis() - inputContext.lastGestureStartTime
        // gesture ended very very shortly after start (< 1ms). make it 1ms to get dispatched to the system
        if (duration == 0L) duration = 1
        val stroke = StrokeDescription(inputContext.path, 0, duration)
        val builder = GestureDescription.Builder()
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) {
            builder.setDisplayId(inputContext.displayId)
        }
        builder.addStroke(stroke)
        // Docs says: Any gestures currently in progress, whether from the user, this service, or another service, will be cancelled.
        // But at least on API level 32, setting different display ids with the builder allows for parallel input.
        dispatchGesture(builder.build(), null, null)
    }


    private fun longPress(inputContext: InputContext, x: Int, y: Int) {
        dispatchGesture(
            createClick(
                inputContext,
                x,
                y,
                ViewConfiguration.getTapTimeout() + ViewConfiguration.getLongPressTimeout()
            ), null, null
        )
    }

    private fun scroll(inputContext: InputContext, x: Int, y: Int, scrollAmount: Int) {
        /*
			   Ignore if another gesture is still ongoing. Especially true for scroll events:
			   These mouse button 4,5 events come per each virtual scroll wheel click, an incoming
			   event would cancel the preceding one, only actually scrolling when the user stopped
			   scrolling.
			 */
        if (!inputContext.gestureCallback.mCompleted) return

        inputContext.gestureCallback.mCompleted = false
        dispatchGesture(
            createSwipe(
                inputContext,
                x,
                y,
                x,
                y - scrollAmount,
                ViewConfiguration.getScrollDefaultDelay()
            ), inputContext.gestureCallback, null
        )
    }

    companion object {
        private const val TAG = "InputService"

        private var instance: InputService? = null

        /**
         * Scaling factor that's applied to incoming pointer events by dividing coordinates by
         * the given factor.
         */
        var scaling: Float = 0f
        var isInputEnabled: Boolean = false
        val isConnected: Boolean
            get() = instance != null

        fun addClient(client: Long, withPointer: Boolean) {
            // NB runs on a worker thread!
            try {
                val displayId = Display.DEFAULT_DISPLAY
                val inputContext = InputContext()
                inputContext.displayId = displayId
                if (withPointer) {
                    inputContext.pointerView = InputPointerView(
                        instance!!,
                        displayId,
                        0.4f * ((instance!!.mInputContexts.size + 1) % 3),
                        0.2f * ((instance!!.mInputContexts.size + 1) % 5),
                        1.0f * ((instance!!.mInputContexts.size + 1) % 2)
                    )
                    // run this on UI thread (use main handler as view is not yet aoceand)
                    instance!!.mMainHandler!!.post { inputContext.pointerView!!.addView() }
                }
                instance!!.mInputContexts[client] = inputContext
            } catch (e: Exception) {
                Log.e(TAG, "addClient: $e")
            }
        }

        fun removeClient(client: Long) {
            // NB runs on a worker thread!
            try {
                val inputContext = instance!!.mInputContexts[client]
                if (inputContext?.pointerView != null) {
                    // run this on UI thread
                    inputContext.pointerView!!.post { inputContext.pointerView!!.removeView() }
                }
                instance!!.mInputContexts.remove(client)
            } catch (e: Exception) {
                Log.e(TAG, "removeClient: $e")
            }
        }

        @RequiresApi(api = Build.VERSION_CODES.R)
        fun takeScreenShots(enable: Boolean) {
            try {
                instance!!.mTakeScreenShots = enable
                if (instance!!.mTakeScreenShots) {
                    val callback: TakeScreenshotCallback = object : TakeScreenshotCallback {
                        override fun onSuccess(screenshot: ScreenshotResult) {
                            try {
                                // create hardware bitmap from HardwareBuffer
                                var bitmap = Bitmap.wrapHardwareBuffer(
                                    screenshot.hardwareBuffer,
                                    screenshot.colorSpace
                                )
                                // create software bitmap from hardware bitmap to be able to use copyPixelsToBuffer()
                                bitmap = Objects.requireNonNull(bitmap)
                                    ?.copy(Bitmap.Config.ARGB_8888, false)
                                // apply scaling. fast NOP when scaling == 1.0
                                bitmap = Bitmap.createScaledBitmap(
                                    bitmap!!,
                                    (bitmap.getWidth() * scaling).toInt(),
                                    (bitmap.getHeight() * scaling).toInt(),
                                    true
                                ) // use filter as this makes text more readable, we're slow in this mode anyway

                                val byteBuffer =
                                    ByteBuffer.allocateDirect(Objects.requireNonNull(bitmap).byteCount)
                                bitmap.copyPixelsToBuffer(byteBuffer)

                                // if needed, setup a new VNC framebuffer that matches the new buffer's dimensions
                                if (bitmap.getWidth() != vncGetFramebufferWidth() || bitmap.getHeight() != vncGetFramebufferHeight()) vncNewFramebuffer(
                                    bitmap.getWidth(),
                                    bitmap.getHeight()
                                )

                                vncUpdateFramebuffer(byteBuffer)

                                // important, otherwise getting "A resource failed to call close." warnings from System
                                screenshot.hardwareBuffer.close()

                                // further screenshots
                                if (instance!!.mTakeScreenShots) {
                                    // try again later, using but not incrementing delay
                                    instance!!.mMainHandler!!.postDelayed(
                                        {
                                            try {
                                                instance!!.takeScreenshot(
                                                    Display.DEFAULT_DISPLAY,
                                                    instance!!.mainExecutor,
                                                    this
                                                )
                                            } catch (ignored: Exception) {
                                                // instance might be gone
                                            }
                                        },
                                        instance!!.mTakeScreenShotDelayMs.toLong()
                                    )
                                } else {
                                    Log.d(TAG, "takeScreenShots: stop")
                                }
                            } catch (e: Exception) {
                                Log.e(TAG, "takeScreenShots: onSuccess exception $e")
                            }
                        }

                        override fun onFailure(errorCode: Int) {
                            try {
                                if (errorCode == ERROR_TAKE_SCREENSHOT_INTERVAL_TIME_SHORT && instance!!.mTakeScreenShots) {
                                    // try again later, incrementing delay
                                    instance!!.mMainHandler!!.postDelayed(
                                        {
                                            try {
                                                instance!!.takeScreenshot(
                                                    Display.DEFAULT_DISPLAY,
                                                    instance!!.mainExecutor,
                                                    this
                                                )
                                            } catch (ignored: Exception) {
                                                // instance might be gone
                                            }
                                        },
                                        50.let { instance!!.mTakeScreenShotDelayMs += it; instance!!.mTakeScreenShotDelayMs }
                                            .toLong())
                                    Log.w(
                                        TAG,
                                        "takeScreenShots: onFailure with ERROR_TAKE_SCREENSHOT_INTERVAL_TIME_SHORT - upped delay to " + instance!!.mTakeScreenShotDelayMs
                                    )
                                    return
                                }
                                Log.e(TAG, "takeScreenShots: onFailure with error code $errorCode")
                                instance!!.mTakeScreenShots = false
                            } catch (e: Exception) {
                                Log.e(TAG, "takeScreenShots: onFailure exception $e")
                            }
                        }
                    }

                    // first screenshot
                    Log.d(TAG, "takeScreenShots: start")
                    instance!!.takeScreenshot(
                        Display.DEFAULT_DISPLAY,
                        instance!!.mainExecutor,
                        callback
                    )
                }
            } catch (e: Exception) {
                Log.e(TAG, "takeScreenShots: exception $e")
            }
        }

        val isTakingScreenShots: Boolean
            get() = try {
                instance!!.mTakeScreenShots
            } catch (ignored: Exception) {
                false
            }

        private fun createClick(
            inputContext: InputContext,
            x: Int,
            y: Int,
            duration: Int
        ): GestureDescription {
            val clickPath = Path()
            clickPath.moveTo(x.toFloat(), y.toFloat())
            val clickStroke = StrokeDescription(clickPath, 0, duration.toLong())
            val clickBuilder = GestureDescription.Builder()
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) {
                clickBuilder.setDisplayId(inputContext.displayId)
            }
            clickBuilder.addStroke(clickStroke)
            return clickBuilder.build()
        }

        private fun createSwipe(
            inputContext: InputContext,
            x1: Int,
            y1: Int,
            x2: Int,
            y2: Int,
            duration: Int
        ): GestureDescription {
            var x1 = x1
            var y1 = y1
            var x2 = x2
            var y2 = y2
            val swipePath = Path()

            x1 = max(x1.toDouble(), 0.0).toInt()
            y1 = max(y1.toDouble(), 0.0).toInt()
            x2 = max(x2.toDouble(), 0.0).toInt()
            y2 = max(y2.toDouble(), 0.0).toInt()

            swipePath.moveTo(x1.toFloat(), y1.toFloat())
            swipePath.lineTo(x2.toFloat(), y2.toFloat())
            val swipeStroke = StrokeDescription(swipePath, 0, duration.toLong())
            val swipeBuilder = GestureDescription.Builder()
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) {
                swipeBuilder.setDisplayId(inputContext.displayId)
            }
            swipeBuilder.addStroke(swipeStroke)
            return swipeBuilder.build()
        }

        /**
         * Returns current cursor position or -1 if no text for node.
         */
        private fun getCursorPos(node: AccessibilityNodeInfo?): Int {
            return node!!.textSelectionEnd
        }

        private fun setCursorPos(node: AccessibilityNodeInfo?, cursorPos: Int) {
            val action = Bundle()
            action.putInt(AccessibilityNodeInfo.ACTION_ARGUMENT_SELECTION_START_INT, cursorPos)
            action.putInt(AccessibilityNodeInfo.ACTION_ARGUMENT_SELECTION_END_INT, cursorPos)
            node!!.performAction(
                AccessibilityNodeInfo.AccessibilityAction.ACTION_SET_SELECTION.id,
                action
            )
        }
    }
}
