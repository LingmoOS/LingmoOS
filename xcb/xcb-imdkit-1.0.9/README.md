xcb-imdkit
==============================
xcb-imdkit is an implementation of xim protocol in xcb, comparing with the
implementation of IMDkit with Xlib, and xim inside Xlib, it has less memory
foot print, better performance, and safer on malformed client.

And not to say it's all asynchronous and it works with xcb.

Though, it doesn't have complete protocol support, since some part of the protocol
is never used. Those parts include:
- XIM_EXT_SET_EVENT_MASK (no im actually use this)
- XIM_EXT_FORWARD_EVENT (it's not any better than forward event)
- XIM_AUTH (IMDkit doesn't implement this, Xlib only has stub, so still no implementation.)
- XIM_STR_CONVERSION (Synchronous protocol, but not used anywhere)
- Locale check on client side, this is actually very evil for input method.
- Only X transport is supported.

To build this project, you need:
libxcb, xcb-util, xcb-util-keysym

Commands:
 mkdir build
 cd build
 cmake -DCMAKE_INSTALL_PREFIX=/usr ..
 make
 sudo make install

xcb-imdkit is released under LGPLv2+, and it is using uthash internally,
which is released under BSD-license.
