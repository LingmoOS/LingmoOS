# Cooperation|dde-cooperation|

### Overview

Cooperation is a software that helps users to collaborate between multiple devices, it enables file drop, keystroke sharing, clipboard sharing between windows and UOS devices.

For UOS, you can get the app by searching for "Cross-end Collaboration" in the **App Store**. For Windows, you can go to[https://www.chinauos.com/resource/assistant](https://www.chinauos.com/resource/assistant).

### Connecting devices

**1. Selection of connected equipment through the equipment list**

When the "Cooperation" application is turned on, the device will automatically scan other devices under the same *** network segment***, and the scanned devices will be displayed in the device list of the application, as shown in the following figure：

![listofdevices](./fig/listofdevices.png)

Find the target collaborative device (which can be recognized by computer name and IP), click Connect![dde-cooperation](../common/connect.svg)after completing the connection of the two devices.。

**2. Connection to equipment by searching for equipment IP**

In the search box, search for the IP of the target collaborative device and click Connect ![dde-cooperation](../common/connect.svg)and then complete the connection of the two devices.

###  File delivery

**1. Delivery through the collaborative equipment list -- file delivery button **

When the two devices are in a cooperative state, in the device list interface, click![dde-cooperation](../common/send.svg), in the file selection window, select the file that needs to be sent, and click **OK**, that is to start the file delivery.

**2. Delivery via file-right-click menu**

Right click on the file you want to send, select **Send to - File Drop**, in the device list, select the target device you need to drop, that is to start the file drop.

```
PS：If you are casting to the Windows side, you need to keep the Cooperation app on the Windows side turned on.
```

### Keyboard and mouse sharing

When two devices A and B are in a cooperative state, and the keyboard and mouse sharing switch in the setting item of computer B is on, the mouse and keyboard of computer A can be used to directly control computer B. The mouse and keyboard of computer A can directly control computer B according to the screen splicing method in the setting item. The mouse can penetrate between the displays of the two computers according to the screen splicing method in the setting item.

### Clipboard sharing

When two devices, A and B, are in a collaborative state, and the "Clipboard Sharing" switch in the settings of the device that initiated the sharing is on, the two computers will share a clipboard, so that you can realize the function of copying in Device A and pasting in Device B.

### Cooperation settings

You can access the Cross-end Collaboration settings screen by clicking on the title bar![dde-cooperation](../common/menu.svg)and selecting **Settings** to enter the settings interface of cross-end collaboration.

1. Discoverable

Used to control whether the current device can be automatically searched by other devices under the same network segment, you can choose all people, not allowed.

2. Equipment nicknames

It is easy for other devices to discover you by this nickname, which defaults to the computer name of this device.

3. Peripheral sharing

When turned on, allows the keyboard, mouse, and touchpad of another device in the collaboration to control this device.

4. Direction of connection

Used to control the positional relationship with the screen splices of the collaborative device.

5. The following users are permitted to submit documents to me

Used to control the permissions of other users on the same LAN to drop files to me, with options to select everyone, those who are collaborating, and disallow.

6. Location of documents

Set the storage path for the received files, which is by default in the folder named after the nickname of the other party's device in the current user's download directory. You can also set to select another directory as the default storage path.

7. Clipboard sharing

Controls whether the clipboard is shared with the collaborating devices. The initiator of the collaboration acts as the server, and as long as the initiator turns on this feature, both parties will share the clipboard.

### Frequently Asked Questions

Q: Why can't I find my other device in the device list?

A: Unable to find the device has the following conditions, please check them one by one:

1. Devices need to have the Cross-Collaboration application enabled in order to be discovered;
2. The devices are not in the same LAN, or in the same LAN but not in the same network segment. In this case, please make sure both devices are under the same LAN first, and then search the IP of the target device in the search box to discover the device;
3. Check Settings - Discoverable - "Allow everyone on the same LAN".

Q: How can I tell if my two devices are under the same network segment?

A: Usually, you can check the first three digits of the IP address, if they are all the same, that is, under the same network segment.

Q: Is data transfer secure in cross-end collaboration or file delivery?

A: The data is transmitted using SSL/TLS1.3 encryption after protobuf serialization to ensure the security of the data during transmission.















