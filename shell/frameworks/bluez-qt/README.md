# BluezQt

Qt wrapper for BlueZ 5 DBus API

## Introduction

BluezQt is a library for communication with BlueZ system and session daemons.

<ul>
  <li>BluezQt::Manager class is used to communicate with system daemon - org.bluez</li>
  <li>BluezQt::ObexManager class is used to communicate with session daemon - org.bluez.obex</li>
</ul>

All method calls are asynchronous using BluezQt::PendingCall that helps tracking
the call progress and handle errors.

