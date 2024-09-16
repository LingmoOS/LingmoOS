<a name="0.3.11"></a>
## 0.3.11 (2019-04-17)


#### Bug Fixes

*   ScreenScaleFactors cannot be applied to all screens when only one value is specified ([24a73d6b](https://github.com/linuxdeepin/qt5integration/commit/24a73d6b2243602781dd7c56071efadb5387142c))
*   font pixel changed when connect a screen ([fcca6b5e](https://github.com/linuxdeepin/qt5integration/commit/fcca6b5e64863741a48600bdc22b18d2c3eac3d6))
*   enable real-time scale for window only on enable screenScaleFactors ([520f00eb](https://github.com/linuxdeepin/qt5integration/commit/520f00eb47e57883838040372fa4bc377cf8be78))

#### Features

*   support auto update the window geometry by new device pixel ratio ([6902fc8a](https://github.com/linuxdeepin/qt5integration/commit/6902fc8ad8923f1579b115561f4aa0a361ac0f64))



<a name="0.3.10"></a>
## 0.3.10 (2019-04-04)




<a name="0.3.9"></a>
## 0.3.9 (2019-04-03)


#### Bug Fixes

*   build failed on Qt 5.7.x ([9159a9fa](https://github.com/linuxdeepin/qt5integration/commit/9159a9fadf70c0190c698769b7f0d576bf380567))

#### Features

*   screen scale support takes effect in real time ([1e9c855b](https://github.com/linuxdeepin/qt5integration/commit/1e9c855b7409fdaa04eb938ddab42b5a38d51790))



<a name="0.3.8"></a>
## 0.3.8 (2019-03-27)




<a name="0.3.7.2"></a>
## 0.3.7.2 (2019-01-10)


#### Bug Fixes

*   QComboBox show icon ([9ac0b0bc](https://github.com/linuxdeepin/qt5integration/commit/9ac0b0bc54add64fa249cf78d1960a270e8f5af1))
*   adjust QComboBox height ([4778931f](https://github.com/linuxdeepin/qt5integration/commit/4778931f7605a04ff5ec3c6166d89bb9166c9b4e))

#### Features

*   add property "touchFlickBeginMoveDelay" for DThemeSettings ([2ab1fab0](https://github.com/linuxdeepin/qt5integration/commit/2ab1fab0b65f6d4f41d3f2c06a61816346fda153))



<a name="0.3.7.1"></a>
## 0.3.7.1 (2018-12-14)


#### Bug Fixes

*   QPushButton icon color follow theme ([e1f6d379](https://github.com/linuxdeepin/qt5integration/commit/e1f6d379ebf05a96689ddcfa01e02126df42c540))



<a name="0.3.7"></a>
## 0.3.7 (2018-11-30)


#### Bug Fixes

*   can not read image file on the second time ([668540cf](https://github.com/linuxdeepin/qt5integration/commit/668540cfc655ae9e9953b90b70411f8f6f7a732a))
*   set the QMenu Qt::WA_TranslucentBackground does not take effect ([6814e028](https://github.com/linuxdeepin/qt5integration/commit/6814e028a90e3b82e4db91e8b62ee76cc86a5489))
* **Menu:**  check icon when select state ([8fe47513](https://github.com/linuxdeepin/qt5integration/commit/8fe475130f7d7aacef438b258acfaeae0560c598))



<a name="0.3.6"></a>
## 0.3.6 (2018-10-25)


#### Features

* **menu:**  add check label ([f7d4530b](https://github.com/linuxdeepin/qt5integration/commit/f7d4530bb396e79a45e6b62e0cdb0b1c14b431e9))



<a name="0.3.5"></a>
## 0.3.5 (2018-08-28)


#### Bug Fixes

* **iconengine:**  use the original one if cast failed ([50286e1d](https://github.com/linuxdeepin/qt5integration/commit/50286e1d2d4a31c86bc556f3cc8433ac2e2803bb))



<a name="0.3.4"></a>
## 0.3.4 (2018-08-13)


#### Bug Fixes

*   build failed on libqt5xdg2 ([da972cac](https://github.com/linuxdeepin/qt5integration/commit/da972cac239240ae4f2ed08e505c75a2bd7aaae9))
*   return a null pixmap in QIcon::pixmap ([8d923ea9](https://github.com/linuxdeepin/qt5integration/commit/8d923ea94f49eee3ad6cda201a04b1125031ebe7))



<a name="0.3.3"></a>
## 0.3.3 (2018-08-06)


#### Features

*   anyway try to enable dxcb for QMenu and QTipLabel ([689e440b](https://github.com/linuxdeepin/qt5integration/commit/689e440bf160a0ed094f03eca9d307e1d1d0f63c))

#### Bug Fixes

*   the dde-desktop UI is blocked ([c75ffad8](https://github.com/linuxdeepin/qt5integration/commit/c75ffad83ff353f10840dac2e38a2daa3b8e3187))



<a name="0.3.2"></a>
## 0.3.2 (2018-07-31)


#### Bug Fixes

*   not update the xdg system icon theme when the icon theme is changed ([cc738c11](https://github.com/linuxdeepin/qt5integration/commit/cc738c114853ed9a9be46f2ebe711e2db98a41e1))
*   remove gtk2 depend ([6b2d1f7d](https://github.com/linuxdeepin/qt5integration/commit/6b2d1f7dfae7d17591f3f1d98a67da933505b9f6))

#### Features

*   support custom the color scheme of get icon from theme ([0daec72b](https://github.com/linuxdeepin/qt5integration/commit/0daec72b6467a29bcbc126b9bfefe5a7e22c1123))



<a name="0.3.1"></a>
## 0.3.1 (2018-07-19)


#### Bug Fixes

*   can not build if the "Qt5Xdg" package version is not 2.0.0 ([444f3a47](https://github.com/linuxdeepin/qt5integration/commit/444f3a47cec873010c254c726249b476dd29c8fc))

#### Features

* **DIconEngine:**  bypass some path which contains '/' ([0c6a7496](https://github.com/linuxdeepin/qt5integration/commit/0c6a7496ec64348fde628866c5d42b493aa7cdd7))



<a name="0.3.0"></a>
### 0.3.0 (2018-05-24)


#### Features

* **style:**  use QPalette active text brush on DTabBar tab widget ([d8ee34e6](https://github.com/linuxdeepin/qt5integration/commit/d8ee34e6efeda4faa3a953849d2b5e5a6a183985))

#### Bug Fixes

*   crash on paint QTabBar ([8d74156e](https://github.com/linuxdeepin/qt5integration/commit/8d74156e4fc07eecc86bf992a9a2a05370f2756d))
*   the line edit widget border color on focus state ([4718470d](https://github.com/linuxdeepin/qt5integration/commit/4718470d0b31b28d67b71c1a77d05a0443fcaa21))
* **QLineEdit:**  not refresh close button after theme changed ([7aca7279](https://github.com/linuxdeepin/qt5integration/commit/7aca72794235f9345c916102f5ed9a2666c3e61f))
* **combox:**  don't render text when edit state ([2f558376](https://github.com/linuxdeepin/qt5integration/commit/2f558376d7788cadf27e280eed01c174435d1262))
* **style:**  DTabBar close button cannot visible ([399ad32d](https://github.com/linuxdeepin/qt5integration/commit/399ad32d4facfc73574d4a56551309f3f7c28e6b))



<a name="0.2.10"></a>
### 0.2.10 (2018-03-14)


#### Bug Fixes

*   the line edit widget border color on focus state ([4718470d](https://github.com/linuxdeepin/qt5integration/commit/4718470d0b31b28d67b71c1a77d05a0443fcaa21))



<a name=""></a>
##  0.2.9 (2018-03-08)


#### Bug Fixes

*   set the progress bar widget radius to 4 ([3cc29bbf](https://github.com/linuxdeepin/qt5integration/commit/3cc29bbf49d93d48ec3027a35721f4ccfa6af1cb))
*   progress bar border color ([7da33a5d](https://github.com/linuxdeepin/qt5integration/commit/7da33a5d5882debae5448a24fc01200b4a390e56))
*   button border color in pressed state ([578948d4](https://github.com/linuxdeepin/qt5integration/commit/578948d4d1a22521f8c39d2f89914761af4a0c20))
*   Style::fullBrush filled icon is not the right size ([fb785a3f](https://github.com/linuxdeepin/qt5integration/commit/fb785a3fa6fabc82117846eeca64a3ee479077f8))
*   drawProgressBarContentsControl crash if widget is nullptr ([1494cff6](https://github.com/linuxdeepin/qt5integration/commit/1494cff6247968025c3c0750f30cb4f186acafcd))
*   crash at drawPushButtonLabel ([795e9fb6](https://github.com/linuxdeepin/qt5integration/commit/795e9fb65f7fb88fc414fb33b230396db7e4ffb4))
*   crash at drawStandardIcon/isTabBarToolButton ([45a305fd](https://github.com/linuxdeepin/qt5integration/commit/45a305fd5919525b8dc6cc9dc0234f242cf00874))
*   tab of QTabBar close button visible position is wrong on after tabs move ([6049831b](https://github.com/linuxdeepin/qt5integration/commit/6049831bc14a0a39aa15d3189b1ab35155e20d0f))
*   not visible close button on tab when QTabBar style is QStyleSheetStyle ([0ca51e06](https://github.com/linuxdeepin/qt5integration/commit/0ca51e0651972ebbf26d14cbbeb0528cf2e2df79))
* **dstyle:**  button border corner offset ([2c74e56e](https://github.com/linuxdeepin/qt5integration/commit/2c74e56e9d75adab5b4a8853043af596abdf2710))
* **style:**
  *  set the scroll bar padding to 2px ([8cf9a91a](https://github.com/linuxdeepin/qt5integration/commit/8cf9a91ad77c90684e2da9412810b37dbcc6be64))
  *  scroll bar border color on dark style ([0b3472b0](https://github.com/linuxdeepin/qt5integration/commit/0b3472b048cc20803b8c468c60662de477d3a2c5))

#### Features

*   support add custom widgets to deepin file chooser dialog ([63caf8ff](https://github.com/linuxdeepin/qt5integration/commit/63caf8ff553c3951ff91af12792f3c40be1eba61))
*   add split line for DTabBar add button ([31730a0b](https://github.com/linuxdeepin/qt5integration/commit/31730a0b2285504cbb3d803655ad55adae545187))
*   add dark style for QTabBar/DTabBar ([7c811b44](https://github.com/linuxdeepin/qt5integration/commit/7c811b440b6fafa1233261690b03abc81645011d))
*   draw QTabBar widget ([9da78b60](https://github.com/linuxdeepin/qt5integration/commit/9da78b6088596da68e800a048a47d6264642ee01))
* **dstyle:**
  *  hide menu icon with DApplication and QLineEdit ([291c7292](https://github.com/linuxdeepin/qt5integration/commit/291c7292ff6f49349e3591835962bb1c715e8972))
  *  support SuggestButtonColor ([a644e697](https://github.com/linuxdeepin/qt5integration/commit/a644e697d47573ee8d5dc907c9d21411d925fd99))
* **style:**
  *  draw alert border on DLineEdit ([810ebeda](https://github.com/linuxdeepin/qt5integration/commit/810ebedad0c50610aee6edd8507858931bfc954d))
  *  the tab bar tab text below the close button increases the transparency gradient ([83bdc592](https://github.com/linuxdeepin/qt5integration/commit/83bdc59210366ac53a41f7397dbc32c22d364d65))
  *  lineedit left padding to 3px ([e56ae93c](https://github.com/linuxdeepin/qt5integration/commit/e56ae93c32f3020d528e7cd780d544c63fea3ab5))



<a name=""></a>
##  0.2.8.3 (2017-12-06)


#### Bug Fixes

* **style:**  visable bottom lines on QMenu when hover item ([c8eb56b7](https://github.com/linuxdeepin/qt5integration/commit/c8eb56b7cadb8f7a3d95207bf6ff7a7048a89706))



<a name=""></a>
##  0.2.8.2 (2017-12-06)


#### Features

* **theme:**  set window flags for native dialog ([3ec67bf0](https://github.com/linuxdeepin/qt5integration/commit/3ec67bf01fead41dbac5a4b096622f7ca50d685c))

#### Bug Fixes

*   is dxcb platform if the "_d_isDxcb" property is true of QCoreApplication ([714b5f78](https://github.com/linuxdeepin/qt5integration/commit/714b5f78b98f7e63757db15ab84a8eb653c36b31))
* **style:**
  *  visable left/top lines on draw rubber band control ([9c06e770](https://github.com/linuxdeepin/qt5integration/commit/9c06e7709b99958e6ebb98714fbbfa2a61d7a0a5))
  *  bottom/right visible lines on draw rubber band rect for QListView ([1177381a](https://github.com/linuxdeepin/qt5integration/commit/1177381ae5914188ca4236e993475367071d66bd))
  *  slider icons ([dae8caa6](https://github.com/linuxdeepin/qt5integration/commit/dae8caa6ec73b95b3781ebe6ecd6da0b359a189f))



<a name=""></a>
##  0.2.8.1 (2017-11-02)


#### Bug Fixes

*   remove libqt5dxcb-dev for "Conflicts" and "Provides" ([4d38dbca](https://github.com/linuxdeepin/qt5integration/commit/4d38dbcac71375f96469cfdda2b4bad0124c7b16))



<a name=""></a>
##  0.2.8 (2017-11-02)


#### Features

*   keep the window border is 1px for hiDPI ([da8040c4](https://github.com/linuxdeepin/qt5integration/commit/da8040c42df0cbde97e21693f472a288ef46a9d8))
* **theme plugin:**  read settings from config file ([aacc2995](https://github.com/linuxdeepin/qt5integration/commit/aacc299512f4006ff174c5700d471d344d7155d0))
* **theme style:**  auto update the widgets font when theme config file changed ([d478074e](https://github.com/linuxdeepin/qt5integration/commit/d478074e73d8e22e2d70080ad1430c565261d9af))

#### Bug Fixes

*   add build-depend: libegl1-mesa-dev ([2d265267](https://github.com/linuxdeepin/qt5integration/commit/2d265267ad1f6f113a74c931a67fe938b85a5af0))
*   build failed(add build-depend: libmtdev-dev) ([babdc356](https://github.com/linuxdeepin/qt5integration/commit/babdc3561ab22d85ab83e4b6e385a4daa1387516))
*   set cursor is invaild ([a2e235be](https://github.com/linuxdeepin/qt5integration/commit/a2e235bed7873d35030be380e8cd73acc0192e89))
* **dxcb:**
  *  crash when screen changed ([7e1627c2](https://github.com/linuxdeepin/qt5integration/commit/7e1627c20fd92d7b4c1d7a69c55b4de5869cf6b6))
  *  the window border size is wrong ([83d6ac50](https://github.com/linuxdeepin/qt5integration/commit/83d6ac500b7e070f5c97ff220da16736229fd060))
  *  draw shadow area ([284cae9d](https://github.com/linuxdeepin/qt5integration/commit/284cae9dd90e9b9d7af92480e3a0e217a7c5f478))
  *  the DFrameWindow shadow area ([6e9c8274](https://github.com/linuxdeepin/qt5integration/commit/6e9c8274ef871fe3fa48a60a0861a96b6880dbf0))
* **style:**
  *  update all widgets on font changed ([0e28f244](https://github.com/linuxdeepin/qt5integration/commit/0e28f244217f57d454424cc7dfba488ee6be93e3))
  *  update all widgets on font changed ([d4a65deb](https://github.com/linuxdeepin/qt5integration/commit/d4a65deb9e21e18390447c8b26eb429843333be4))
* **theme:**
  *  "DFileSystemWatcherPrivate::addPaths: inotify_add_watch failed" ([a0322a68](https://github.com/linuxdeepin/qt5integration/commit/a0322a68447b7d918af49fbba23ebe1d5f53da25))
  *  set ini codec to "utf8" for QSettings ([3a4f7d76](https://github.com/linuxdeepin/qt5integration/commit/3a4f7d7651f6c433ef9735426d936289ac04d6c9))



