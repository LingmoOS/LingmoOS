# How to contribute

## language
### [zh_CN](CONTRIBUTING_zh_CN.md)

## As an user

LINGMO style effects almost whole qt's applications in LINGMO Desktop Environment. Such as button's rounded radius and color, tab page's switch animations, menu's transparency and blur effects, etc.

We hope users point out the inconsistencies and lackes of details. That will help us make targeted improvements.

However, lingmo-style can not take over all works of application's displaying. One application might have it own style, even in LINGMO. And there also might be some adjustment for applications which using lingmo-style. It is hard to distinguish wether it is fully lingmo-styled application for user.

Anyway, if you found out a problem about visual senses or bugs. You can emit a issue to us. Whatever here or at projects which have problems.

## As a developer

If you are familiar with Qt5 QPA platformtheme and QStyle, you will know the structure of this project quickly. This project is similar to some other platformtheme project, such as qt5-gtk2-platformtheme, qt5ct, kde, etc. By the platformtheme and style plugins of this project, we build our own desktop environment theme, that means not only our own applications, but also other qt's applications will seemed as LINGMO style in our DE, too.

Platformtheme will completely resove the problem that developers have to cost much time for tuning style to attach LINGMO's desgin. We hope developers spend more their time in business logic and functions, and also can ensure the styled unity even in other platform, such as KDE.

To archive that goal, it's hard to rely on me alone. Developers also need to understand how to develop an application in a standard, and how to comply with standards and expand the application. By doing that you will find its significance in long term.

 For now, Although I have put almost all my energy into the development of this project, It still progresses very slowly. There are many troubles that want volunteers' help to resolve.

### Platform Theme Level
As other platform themes have done, we also want to provide all the features about a platform.

- Platform Dialogs, such as filechooser dialogs.
- Platform Configs, such as font.
- Platform Extensions, such as global menu.

Gtk's applcations theme is also a problem, we have to consider providing a way to uniform the applications' styles between Qt and Gtk.

### Styles Level
A nice looking style is always complex, such as breeze and oxygen. For making our own style, we have to spend a lot of time for UI designing and details maintaining.

As a developer, we need change the desgin to codes. There could be roughly divided into 2 parts.

- Tuning of control's painting, such as button, menu's round radius. System or control's palette.
- Combing animations with controls, such as tab switching with sliding, button hovering with fading.

You first need to understand how QStyle and style plugins work, how they draw a control and let control animated.

### Animations Frameworks Level
Animation Frameworks is usually based on Qt's Animations Frameworks. It aims to:

- Make widgets and view items animated, and let the state changing more smoothly and cool.

Note that even though Animations Frameworks is relatively independent  with platform. But it will have great influence on the theme or style.

## Active communication

If you were intersted to contribute, I would also be pleasure to help you understand this project for my best. Starting with an issue or e-mail is good.

Yue Lan, <lanyue@kylinos.cn>