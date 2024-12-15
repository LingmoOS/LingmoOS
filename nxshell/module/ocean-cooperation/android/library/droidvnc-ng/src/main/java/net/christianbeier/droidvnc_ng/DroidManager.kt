package net.christianbeier.droidvnc_ng

final class DroidManager {
    public interface ClientListener {
        fun onClientConnected(client: Long)
        fun onClientDisconnected(client: Long)
    }

    public interface InputListener {
        fun onInputPointerEvent(buttonMask: Int, x: Int, y: Int, client: Long)
        fun onInputKeyEvent(down: Int, keysym: Long, client: Long)
        fun onInputCutText(text: String?, client: Long)
    }
}