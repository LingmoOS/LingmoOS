#Contributing for dde-session-ui

#dde-session-ui layout

##Coding layout
Dde-session-ui　is developed by Qt, and use dbus interface to communication.

There are six packages:

- background

The widget background is use to draw a background label with a image, and you can choose
the image with blurring effect.

- boxframe

This widgets is a encapsulation of background, which is used to process
the situation of multi-screen.

- global_util

This packages is some global class, such as: class of global signal manager and so on.

- widgets

This packages is some common widgets, which colud be include by every sub-project.

- translations

This packages is including the translation files of all the projects.

The other packages is the sub-project.

##Others
......

##Module Design
The sub-project is design by three steps:

0. Initial the user interface;
0. initial the backend datas;
0. update the user interface by the backend data.

##List of Workaround
In some platform the lightdm-deepin-greeter enter the password very slowly,
which is caused by applying the qt style sheet(qss), so we add some conditional
compilation to workaround.
