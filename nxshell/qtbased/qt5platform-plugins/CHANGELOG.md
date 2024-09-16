<a name="1.2.2"></a>
## 1.2.2 (2019-05-29)


#### Features

*   support the 16bits/30bits image formats ([c50d83ef](https://github.com/linuxdeepin/qt5dxcb-plugin/commit/c50d83efa671780fd91ae73ef86ac31fb5256292))



<a name="1.2.1"></a>
## 1.2.1 (2019-05-23)


#### Features

*   add new Qt private component for Qt 5.13+ ([796ad523](https://github.com/linuxdeepin/qt5dxcb-plugin/commit/796ad5235fec4ac83b734f2876614fd8eb7b3c2b))
*   support Qt 15.12.1/15.12.2/15.12.3 ([ee3e4c8a](https://github.com/linuxdeepin/qt5dxcb-plugin/commit/ee3e4c8a965b4e0a7b920ac8bcf8af96ff47f5b2))

#### Bug Fixes

*   It is easy to trigger resize when using the touch screen to move the window ([99e43da8](https://github.com/linuxdeepin/qt5dxcb-plugin/commit/99e43da805ae29d078315dd23683d0277d0bb760))
*   the window menu cannot hide ([a077611d](https://github.com/linuxdeepin/qt5dxcb-plugin/commit/a077611df0469f498ebec29e566c68d260f10ddb))
*   the foreign platform window screen is wrong ([6eac7834](https://github.com/linuxdeepin/qt5dxcb-plugin/commit/6eac7834c93d54c9e9cbb18b155d0da1768ddf61))



<a name="1.2.0"></a>
## 1.2.0 (2019-05-09)


#### Bug Fixes

*   DForeignPlatformWindow have a wrong screen object ([49658492](https://github.com/linuxdeepin/qt5dxcb-plugin/commit/4965849204cc5fa67fb207f136222af3e4f0b2a8))
*   crash at DForeignPlatformWindow::handleConfigureNotifyEvent ([5ff0224a](https://github.com/linuxdeepin/qt5dxcb-plugin/commit/5ff0224aeafb6ec6fb76c5ad5eb3351f6de883d4))

#### Others

*   correct a typo ([83f8d46c](https://github.com/linuxdeepin/qt5dxcb-plugin/commit/83f8d46c11ac2c3a58a53ec88db08fe7d72eaf8f))



<a name="1.1.27"></a>
## 1.1.27 (2019-04-23)


#### Bug Fixes

*   window can not get focus at mapped ([42684bc9](https://github.com/linuxdeepin/qt5dxcb-plugin/commit/42684bc9dc937d0c1373d0d4d132391fa2118250))
*   do not use methods because symbols are not exported ([07eb94cd](https://github.com/linuxdeepin/qt5dxcb-plugin/commit/07eb94cd51ed05e83571c32b233ebf2e48e56700))
*   compatibility with Qt 5.12+ ([b6355ce1](https://github.com/linuxdeepin/qt5dxcb-plugin/commit/b6355ce160c2ba48df7efb81a96a4a7f6d8923e6))



<a name="1.1.26.1"></a>
## 1.1.26.1 (2019-04-22)


#### Bug Fixes

*   failed on build(Qt 5.7.x) ([b7a973db](https://github.com/linuxdeepin/qt5dxcb-plugin/commit/b7a973dbd2a6ff086114aff01c9ff33f6e01cb17))



<a name="1.1.26"></a>
## 1.1.26 (2019-04-17)


#### Bug Fixes

*   Synchronize its screen object only when the native window is called directly on setScreen ([a5749da3](https://github.com/linuxdeepin/qt5dxcb-plugin/commit/a5749da392d77c8b92cf66480b52207af3c7452f))
*   call QDeepinTheme::updateScaleLogcailDpi when screen added ([d4815e53](https://github.com/linuxdeepin/qt5dxcb-plugin/commit/d4815e5306460133bf4018040d748940e35fa014))
*   window shadow dislocation on multi-screen and the screens have different device pixel scale ratio ([d8013112](https://github.com/linuxdeepin/qt5dxcb-plugin/commit/d8013112ee4635a5e5d6177321d6fb954246d53b))



<a name="1.1.25"></a>
## 1.1.25 (2019-03-27)


#### Bug Fixes

*   the window can not move ([2011cb3f](https://github.com/linuxdeepin/qt5dxcb-plugin/commit/2011cb3fb31be1a5be540901a1af5b8c320b71e5))



<a name="1.1.24"></a>
## 1.1.24 (2019-02-26)


#### Bug Fixes

*   compatibility with Qt 5.12.1 ([1a9a05ef](https://github.com/linuxdeepin/qt5dxcb-plugin/commit/1a9a05ef7942c91dafaf24023404aef9cbd77dbe))
*   the minimized window can not active on 2D window manager ([c56c9949](https://github.com/linuxdeepin/qt5dxcb-plugin/commit/c56c9949384d51f4be13272898bd3f641c40081b))



<a name="1.1.23"></a>
## 1.1.23 (2019-01-25)


#### Bug Fixes

*   Fix window shadow drawing error if setting clip path ([c8b87b42](https://github.com/linuxdeepin/qt5dxcb-plugin/commit/c8b87b42a875eb6bbb506f0019d5ff4d4f580371))



<a name="1.1.22"></a>
## 1.1.22 (2019-01-02)


#### Bug Fixes

*   Does not reissue mouse press events when receiving an Ungrab type Enter event during mouse press ([e3569fe7](https://github.com/linuxdeepin/qt5dxcb-plugin/commit/e3569fe7a3b9b16f20fcc8cc7a793553fb87a2b4))



<a name="1.1.21"></a>
## 1.1.21 (2018-12-27)


#### Bug Fixes

*   the drag/drop is invalid if use DForeignPlatformWindow for target XWindow ([b4c0ece2](https://github.com/linuxdeepin/qt5dxcb-plugin/commit/b4c0ece2f2651f34c461aca6d478c1e41886fe1d))



<a name="1.1.2"></a>
## 1.1.20 (2018-12-10)


#### Bug Fixes

*   a typo ([c46eb749](https://github.com/linuxdeepin/qt5dxcb-plugin/commit/c46eb749f8a7118d8ce817e4f9cd63f529f52650))



<a name="1.1.15"></a>
## 1.1.15 (2018-11-30)


#### Bug Fixes

*   the Desktop application is not contains in current workspace ([e1d45711](https://github.com/linuxdeepin/qt5dxcb-plugin/commit/e1d45711fa14754cd72cff733a8a75f6d5a8efa6))
*   support Qt version>=5.9.0 for DForeignPlatformWindow ([594f73e6](https://github.com/linuxdeepin/qt5dxcb-plugin/commit/594f73e638da488efbdb4e518fb0f726b0ee09bc))

#### Features

*   add inteface: "windowManagerName" ([711e6aa1](https://github.com/linuxdeepin/qt5dxcb-plugin/commit/711e6aa1dd119fdaaf196845733df67b7d0d6b20))



<a name="1.1.14"></a>
## 1.1.14 (2018-11-19)


#### Bug Fixes

*   build failed on low version gcc ([dce47ff6](https://github.com/linuxdeepin/qt5dxcb-plugin/commit/dce47ff6aa4b34816c917cedd00e6c83048e540a))



<a name="1.1.13"></a>
## 1.1.13 (2018-11-08)

*  refactor: set default border width to 1px of can not resize window


<a name="1.1.12"></a>
## 1.1.12 (2018-10-25)


#### Bug Fixes

*   kwin not update blur ([58cda70e](https://github.com/linuxdeepin/qt5dxcb-plugin/commit/58cda70e10f1de04cb76fd98c5724ccc0f99e592))

#### Features

*   support Qt 5.11.2 ([52e74b16](https://github.com/linuxdeepin/qt5dxcb-plugin/commit/52e74b16f8fcf439f8b78807abdfb5b0d1c163ff))



<a name="1.1.11"></a>
## 1.1.11 (2018-08-06)


#### Features

*   support disable QPaintEngine features for QImage ([8e990f3a](https://github.com/linuxdeepin/qt5dxcb-plugin/commit/8e990f3a46d70a2d88a18acf7493016f5c552c5b))
*   support enable dxcb for the QWindow(the window had a platform-window) ([08c03f14](https://github.com/linuxdeepin/qt5dxcb-plugin/commit/08c03f14dec7e355304c3cb1bbc0b39d64067f09))



<a name="1.1.10"></a>
## 1.1.10 (2018-07-31)


#### Bug Fixes

*   the shadow area will shake when the window is displayed ([9b10ea7d](https://github.com/linuxdeepin/qt5dxcb-plugin/commit/9b10ea7dbeaf2699728d3ef94ae98c9baf5c13fa))

#### Features

*   add window property "_DEEPIN_DXCB_SHM_INFOS" ([e7ebe5d3](https://github.com/linuxdeepin/qt5dxcb-plugin/commit/e7ebe5d3c93e436bc81e1a739820abefa28d1a79))



<a name="1.1.9.2"></a>
## 1.1.9.2 (2018-07-19)


#### Features

*   support Qt 5.11.1 ([c73177d7](https://github.com/linuxdeepin/qt5dxcb-plugin/commit/c73177d75a3ebc91ad6a24a0ea82ad2635ba160b))
*   support Qt 5.11.0 ([f458b599](https://github.com/linuxdeepin/qt5dxcb-plugin/commit/f458b59956fc34bc5d181842d065f37d4deac903))

#### Bug Fixes

*   click on the window without focus, resulting in maximization ([cfcc6a24](https://github.com/linuxdeepin/qt5dxcb-plugin/commit/cfcc6a243832527c3c9e95408b150589fb8e5a4e))



<a name="1.1.9.1"></a>
## 1.1.9.1 (2018-06-26)




<a name="1.1.9"></a>
## 1.1.9 (2018-06-25)


#### Features

*   support XdndActionList ([b3bc3420](https://github.com/linuxdeepin/qt5dxcb-plugin/commit/b3bc3420dc81eb2066e03522d81c76660205cd50))
*   support move window by touch event ([bd104d6c](https://github.com/linuxdeepin/qt5dxcb-plugin/commit/bd104d6c51da94b33ea64c71715d5ae1e949d9cd))

#### Bug Fixes

*   crash when add scroll device ([ee6ebb4a](https://github.com/linuxdeepin/qt5dxcb-plugin/commit/ee6ebb4a416c386ec1d32460a2caa0d486d7547b))



<a name="1.1.8.8"></a>
## 1.1.8.8 (2018-06-07)


#### Bug Fixes

*   revert the fix commit of QTBUG-53993 only in BypassWindowManagerHint window ([22ea6fc9](https://github.com/linuxdeepin/qt5dxcb-plugin/commit/22ea6fc9b25e4f37df48d5b70b363cd29efe4f0f))



<a name="1.1.8.7"></a>
## 1.1.8.7 (2018-06-07)


#### Bug Fixes

*   can not get focus by mouse press on BypassWindowManagerHint window ([b7799256](https://github.com/linuxdeepin/qt5dxcb-plugin/commit/b779925639c51b9024023797c536bc7bd61805ec))
*   build failures under Qt 5.11 ([3214457d](https://github.com/linuxdeepin/qt5dxcb-plugin/commit/3214457d622b9ecbea2bcd11d8a3473aac3c0a5a))



<a name="1.1.8.6"></a>
## 1.1.8.6 (2018-05-11)


#### Bug Fixes

*   build failed on Qt 5.10.0 ([f6c86693](https://github.com/linuxdeepin/qt5dxcb-plugin/commit/f6c866935f7833b60b0701360473762158d6a11f))



<a name="1.1.8.5"></a>
## 1.1.8.5 (2018-05-04)


#### Features

*   support window group leader ([398cf704](https://github.com/linuxdeepin/qt5dxcb-plugin/commit/398cf704776d69b6e0c4b1d03827b91628cd9431))
*   support Qt 5.9.4 ([8ada8d39](https://github.com/linuxdeepin/qt5dxcb-plugin/commit/8ada8d39b33a4f66183549286132b19d9e352f84))
*   support Qt 5.10.1 ([e476353b](https://github.com/linuxdeepin/qt5dxcb-plugin/commit/e476353bbcf68eb6474d837471a4d202fa76737a))
*   add "inputEventSourceDevice" native platform function ([541fbc79](https://github.com/linuxdeepin/qt5dxcb-plugin/commit/541fbc79d93b68cb3067ffba766f16d1e6583279))
*   listen the DForeignPlatformWindow events ([09650c80](https://github.com/linuxdeepin/qt5dxcb-plugin/commit/09650c8024fa0cb626ee86d7f9f5df4cdc4fe390))

#### Bug Fixes

*   frame window radius is wrong when the window state form Qt::WindowMaximized to Qt::WindowNoState ([ea9b1215](https://github.com/linuxdeepin/qt5dxcb-plugin/commit/ea9b12150fa2c4f4f494345d9210595c9b578432))
*   crash in DPlatformWindowHelper::eventFilter if the event is DragLeave type ([5448b26f](https://github.com/linuxdeepin/qt5dxcb-plugin/commit/5448b26f118470dec442f86b892649719030b273))
*   rm .qmake.conf ([3ae6fd16](https://github.com/linuxdeepin/qt5dxcb-plugin/commit/3ae6fd160f44a59f808cf5dac4f1817a4cb59bf8))
*   can not ignore window close event ([a1d505e2](https://github.com/linuxdeepin/qt5dxcb-plugin/commit/a1d505e2954be573a322238a312f24508a5a555a))
*   the isWindowBlockedHandle function is stuck in an infinite loop ([7ce08107](https://github.com/linuxdeepin/qt5dxcb-plugin/commit/7ce081076605db4e9cf9d2be5b2ae37adce1afff))



<a name="1.1.8.4"></a>
##  (2018-03-22)


#### Bug Fixes

*   window setting Qt::WindowTransparentForInput flag is invalid ([475bc708](1.1.8.4/commit/475bc70832591da3ef9ca06ccbd328c033baefa3))
*   clear the window blur areas when the window unmap ([482d26e1](1.1.8.4/commit/482d26e1854d27c055d97c5e223f29991f8efccd))



<a name="1.1.8.3"></a>
##  (2018-03-16)


#### Bug Fixes

*   crash in DPlatformBackingStoreHelper::flush ([38d9dad0](1.1.8.3/commit/38d9dad077e127a64fbcf847a9c7cc1e31541473))



<a name="1.1.8.2"></a>
##  (2018-03-15)


#### Bug Fixes

*   dframewindow timerEvent ([5afacaec](1.1.8.2/commit/5afacaec9839c898eebf4f120c9e44a9bae098b3))
*   request active window is invalid ([ea8e7a13](1.1.8.2/commit/ea8e7a132aa603b9cf88b64d7fbc78d1c91249a3))
*   window not receive Leave event when the window is covered by other windows ([2348e1c3](1.1.8.2/commit/2348e1c3ea1138850181a84571437690f48c9a25))



<a name="1.1.8.1"></a>
##  1.1.8.1 (2018-03-08)


#### Features

*   support disable change window cursor ([0fbb6f1c](https://github.com/linuxdeepin/qt5dxcb-plugin/commit/0fbb6f1cd56d8514f512f533893f115af93cca78))

#### Bug Fixes

*   crash on handlePropertyNotifyEvent ([ebb2b1a3](https://github.com/linuxdeepin/qt5dxcb-plugin/commit/ebb2b1a32f53b2a6d888835ada88a4c98d5de537))
*   window can not maximize ([da161585](https://github.com/linuxdeepin/qt5dxcb-plugin/commit/da161585468526e8a821933d08e97b94c76d3e36))
*   no leave event when window hide ([a2bd2498](https://github.com/linuxdeepin/qt5dxcb-plugin/commit/a2bd24989a077dc058928bf58d0aefea4dc98f13))
*   the window "_d_netWmStates" property is invalid ([f48df4c1](https://github.com/linuxdeepin/qt5dxcb-plugin/commit/f48df4c139cc058cd6a8ac05d10245cdba715121))
*   window fillet anti-aliasing ([ed29cca5](https://github.com/linuxdeepin/qt5dxcb-plugin/commit/ed29cca57544f2678bbc594fd7d7203d0d9ed933))
*   blurred window sometimes does not take effect ([3aa09aa3](https://github.com/linuxdeepin/qt5dxcb-plugin/commit/3aa09aa3502e422395cc60acaa2797a15f0105c5))
*   carsh on arm platform ([92d20f72](https://github.com/linuxdeepin/qt5dxcb-plugin/commit/92d20f720331542891b04a738e42bc586064a322))
*   optimize the effect of the window fillet ([40d298a7](https://github.com/linuxdeepin/qt5dxcb-plugin/commit/40d298a7beeb7cad7e2f6bb5cadcfc79b8fdbbc2))
*   window exists property on non-kwin windows _KDE_NET_WM_BLUR_BEHIND_REGION ([4cc89fcc](https://github.com/linuxdeepin/qt5dxcb-plugin/commit/4cc89fccb8b7791bb349bdbfd0c08f744ab143e6))



<a name="1.1.8"></a>
## 1.1.8 (2018-02-09)


#### Bug Fixes

*   shadow image not repaint ([8cde9879](https://github.com/linuxdeepin/qt5dxcb-plugin/commit/8cde9879cc74944c111e23a7c99634b0ab545e15))
*   content window no clip ([359647c3](https://github.com/linuxdeepin/qt5dxcb-plugin/commit/359647c39c6220ecb6c44471b953d9bd248ce392))



<a name="1.1.7"></a>
##  1.1.7 (2018-02-07)


#### Features

*   set window radius to 0 when window maximzed ([35ef919c](https://github.com/linuxdeepin/qt5dxcb-plugin/commit/35ef919cb5dd6a50deff1e2f466bb141a4fc3424))

#### Bug Fixes

*   crash in DPlatformBackingStoreHelper::flush ([dbab1dc3](https://github.com/linuxdeepin/qt5dxcb-plugin/commit/dbab1dc31e6a3129d2b688ede676679102c099f8))
*   update copyright year ([422c0d2d](https://github.com/linuxdeepin/qt5dxcb-plugin/commit/422c0d2d53a1452ff9b80f31c166a38db95daa38))
*   shadow image size is wrong ([e537bcea](https://github.com/linuxdeepin/qt5dxcb-plugin/commit/e537bceab205b45f4f7dd5080213ab33ec0bd5bd))
*   window radius not visible ([671b8894](https://github.com/linuxdeepin/qt5dxcb-plugin/commit/671b8894667877e0b263234d2e91edecdcafab75))
*   window can not get focus on Qt 5.9+ ([3c0a44bf](https://github.com/linuxdeepin/qt5dxcb-plugin/commit/3c0a44bf9f8522ad820287d28d7b49bbeea32350))
*   QGuiApplicationPrivate::lastCursorPosition on hiDPI window ([08755140](https://github.com/linuxdeepin/qt5dxcb-plugin/commit/0875514090cf19e8129f1c6f08a42ac8c219bb3c))
*   remove debug output in DPlatformBackingStoreHelper::flush ([c68032d9](https://github.com/linuxdeepin/qt5dxcb-plugin/commit/c68032d9cfe0e4e2fc6ef508908f53eec1f2a704))
*   can not resize window by bottom/right ([e4c58d76](https://github.com/linuxdeepin/qt5dxcb-plugin/commit/e4c58d767d6e5b9fdd8805cec2449d3ddb2b5e0a))
*   can not reisze window on openbox ([218df1e2](https://github.com/linuxdeepin/qt5dxcb-plugin/commit/218df1e28708dbea9926eb0f94896d959744f7c3))
*   content window position ([2439d675](https://github.com/linuxdeepin/qt5dxcb-plugin/commit/2439d6753fdd3c6449b43c844f4191d0b60aa12b))
*   set miniumu/maximum size inoperative for window ([0ad7e599](https://github.com/linuxdeepin/qt5dxcb-plugin/commit/0ad7e59901db5d167227a6026623ffae678dab02))
*   crash at VtableHook::overrideVfptrFun ([1a8e03cf](https://github.com/linuxdeepin/qt5dxcb-plugin/commit/1a8e03cfb8574ac8df2e1f406b12568307943af1))



<a name="1.1.6"></a>
##  1.1.6 (2017-12-28)


#### Bug Fixes

*   send resize event to content window on frame window size changed ([fc88f619](https://github.com/linuxdeepin/qt5dxcb-plugin/commit/fc88f6198c3273acc49f566f95efc459b5a357ba))
*   the content window frame margins is invalid ([bb069366](https://github.com/linuxdeepin/qt5dxcb-plugin/commit/bb069366555c07d1750f376677b20b19fa09e8d3))
*   content window position is wrong at show ([2f74cd2c](https://github.com/linuxdeepin/qt5dxcb-plugin/commit/2f74cd2ce4db37009141d181f9752f3b124dee09))
*   add break for switch/case ([ca84b062](https://github.com/linuxdeepin/qt5dxcb-plugin/commit/ca84b0622ebee89d7454401d261f99f577850f36))
*   fcitx input method can not work in QTextEdit ([cbd74f2a](https://github.com/linuxdeepin/qt5dxcb-plugin/commit/cbd74f2a6a9f137d0081848a53a40bd50d350ca6))
*   window blur area is a rounded rect if set clip path ([a89e7d0c](https://github.com/linuxdeepin/qt5dxcb-plugin/commit/a89e7d0c035137bf1f85c7993eb14cb190f98ce0))
*   LICENSE ([f8013823](https://github.com/linuxdeepin/qt5dxcb-plugin/commit/f801382355dd62618fa0548c87cf6752b53673e8))
*   after moving the window release the mouse and then move the mouse will receive a mouse release event ([10a73727](https://github.com/linuxdeepin/qt5dxcb-plugin/commit/10a73727fe067bbe395d38c46890508ddd1ef559))
*   Adapt lintian ([1f862c11](https://github.com/linuxdeepin/qt5dxcb-plugin/commit/1f862c1175a2342520a3340621cb122e4478f88a))
*   also search for libqt5xcbqpa private headers in system ([e47d510f](https://github.com/linuxdeepin/qt5dxcb-plugin/commit/e47d510f9c2d371f59c33a290d27347840318f44))
*   do not paint shadow on window maximized ([b1483de1](https://github.com/linuxdeepin/qt5dxcb-plugin/commit/b1483de187d26ac360aa6b6f08469fad61b32077))

#### Features

*   eat focus event on grab/ungrab ([936da4ca](https://github.com/linuxdeepin/qt5dxcb-plugin/commit/936da4ca99900ee4256ede4ca33b8767f189e5a2))
*   support for Qt 5.10.0 ([247cabb7](https://github.com/linuxdeepin/qt5dxcb-plugin/commit/247cabb70d2f97de4e6cec062e1106c9678d97a5))



<a name=""></a>
##  1.1.5 (2017-12-07)


#### Bug Fixes

*   visable black border when maximize window ([9a25eccc](https://github.com/linuxdeepin/qt5dxcb-plugin/commit/9a25eccc5e50ccd2cf196671d898ca83ed468f19))
*   output "QPainter::begin: Paint device returned engine == 0, type: 3" ([d424dd61](https://github.com/linuxdeepin/qt5dxcb-plugin/commit/d424dd61e2adc6cb4f96315354128751a2ac60cf))



<a name=""></a>
##  1.1.4 (2017-12-06)


#### Features

*   support for Qt 5.9.3 ([32ec6503](https://github.com/linuxdeepin/qt5dxcb-plugin/commit/32ec650304f12e3107c7dd13677c0cbb0afb1023))

#### Bug Fixes

*   compiling failed in Qt5.7.0 and above ([790377a5](https://github.com/linuxdeepin/qt5dxcb-plugin/commit/790377a5ecadbcd86a0bb62bc71cf71144873984))



<a name=""></a>
##  1.1.3.1 (2017-12-06)


#### Features

*   not clean when begin paint on opaque window ([635a8252](https://github.com/linuxdeepin/qt5dxcb-plugin/commit/635a8252efe0500e2a299b0f336bab6b0362da1e))

#### Bug Fixes

*   visible lines at bottom/right on frame window ([40f53663](https://github.com/linuxdeepin/qt5dxcb-plugin/commit/40f536630fac0222f6e2ae29d0cfcd604f9e6b77))
*   content window area path on window maximized/fullscreen ([4dd1dc20](https://github.com/linuxdeepin/qt5dxcb-plugin/commit/4dd1dc204125a62137ad735bc8e387ef0150314b))
*   mouse position error on automatic adsorption ([196e34cb](https://github.com/linuxdeepin/qt5dxcb-plugin/commit/196e34cbade9e665b4b766e9c62332930cb34f54))



<a name=""></a>
##  1.1.3 (2017-11-28)


#### Features

*   add native function: popupSystemWindowMenu ([0e741b1e](https://github.com/linuxdeepin/qt5dxcb-plugin/commit/0e741b1ef87e77df108c9cccd3fb24e5a9010a88))
*   add showWindowSystemMenu function ([915c6d3f](https://github.com/linuxdeepin/qt5dxcb-plugin/commit/915c6d3f75723c4574c00a786511ea098345d0d8))
*   support for Qt 5.9.2 ([715bb1e4](https://github.com/linuxdeepin/qt5dxcb-plugin/commit/715bb1e411ebf5283a4aec5416c9b7d19d09a55d))

#### Bug Fixes

*   window vaild path is empty ([c2b1a1b5](https://github.com/linuxdeepin/qt5dxcb-plugin/commit/c2b1a1b52324f668a45213bebf1282aeec2c64e2))
*   blur area overstep the content window area ([69f5edef](https://github.com/linuxdeepin/qt5dxcb-plugin/commit/69f5edef4453780c1566bc000b63307a5417b14d))



<a name=""></a>
##  1.1.2 (2017-11-16)


#### Features

*   support set the window radius for QOpenGLWindow ([431fa688](https://github.com/linuxdeepin/qt5integration/commit/431fa688a6fad3a5057c89396342dfcc53987b04))
*   use fake platform name when set DXCB_FAKE_PLATFORM_NAME_XCB ([471216b2](https://github.com/linuxdeepin/qt5integration/commit/471216b22d3ab08129d6d6e5c9e47696947adaaa))

#### Bug Fixes

*   the drag window position when dragging from one screen to another ([3905a0a0](https://github.com/linuxdeepin/qt5integration/commit/3905a0a02e1f17092b6c324516ae130172e247c9))
*   the DFrameWindow's border path is wrong ([4882b916](https://github.com/linuxdeepin/qt5integration/commit/4882b91676593eadb8dcc98fb2a86ae82d0fe87a))
*   the content window screen is wrong ([20f7695f](https://github.com/linuxdeepin/qt5integration/commit/20f7695f59a3a07981fc9bc5dde874385e215328))
*   update screen for content window when the frame window screen changed ([f40a66ab](https://github.com/linuxdeepin/qt5integration/commit/f40a66abdaea1af1d8c00079717dc8d8c2622748))
*   can not move window on openbox wm ([af64d84d](https://github.com/linuxdeepin/qt5integration/commit/af64d84dfc7d98e2bf5b8ea5a11507e200111431))



<a name=""></a>
##  1.1.1 (2017-11-10)


#### Bug Fixes

*   crash on use the QWebEngineView ([c13bdeee](https://github.com/linuxdeepin/qt5integration/commit/c13bdeee242b0cbfebef78df925028f285913f2c))
*   mouse enter of the window to receive the mouse to release the event ([7a6989e5](https://github.com/linuxdeepin/qt5integration/commit/7a6989e5cf8a2be0cc84644c9289e647704b06c6))
*   mouse over the right border of the window to receive the mouse to leave the event ([cad2e037](https://github.com/linuxdeepin/qt5integration/commit/cad2e0379abf745e5d59fbca5902a1008edfff4f))



<a name=""></a>
##  1.1 (2017-11-06)


#### Features

*   print info on create new window ([b68d0371](https://github.com/linuxdeepin/qt5integration/commit/b68d03713feb1d77640b35891e38e05736becba9))

#### Bug Fixes

*   update the content window clip path by it size ([070e3bc4](https://github.com/linuxdeepin/qt5integration/commit/070e3bc4d9c715d1c8c4e1531077286c43658815))
*   the content window position is wrong when content margins hint changed ([9b1a28e6](https://github.com/linuxdeepin/qt5integration/commit/9b1a28e65d60e7286687943511a58485e0937bf5))
*   window focus out when mouse leave ([14c14df2](https://github.com/linuxdeepin/qt5integration/commit/14c14df2c7a92be746d5479558ae2c61765052a9))
*   the DFrameWindow's shadow and border ([e35a87a9](https://github.com/linuxdeepin/qt5integration/commit/e35a87a93872acde909383b6f5b4ca3baf63bfe5))
*   the chile window is blocked by modal window ([db20016c](https://github.com/linuxdeepin/qt5integration/commit/db20016c6baa38ea561cddb9a3676252a19d8af6))
*   warning: "W: Ignoring Provides line with non-equal DepCompareOp for package dde-qt5integration" for apt-get ([a074941e](https://github.com/linuxdeepin/qt5integration/commit/a074941e238476cf5e93fe2400e72d0901474d64))
*   disable update frame mask ([5d136679](https://github.com/linuxdeepin/qt5integration/commit/5d136679f4886523dc3bc779ac8d0347611028bc))
*   can not install ([1ac9975a](https://github.com/linuxdeepin/qt5integration/commit/1ac9975a9169b47f185ef5e33d79e3e2de2dbc50))



<a name=""></a>
##  1.0 (2017-11-02)


#### Others

*   setup .clog.toml ([d5d065d4](https://github.com/linuxdeepin/qt5integration/commit/d5d065d4099973a2d82aca0e16acd26c99621bd9))

#### Features

*   keep the window border is 1px for hiDPI ([da8040c4](https://github.com/linuxdeepin/qt5integration/commit/da8040c42df0cbde97e21693f472a288ef46a9d8))
*   QMenu hiDPI support ([a045b17e](https://github.com/linuxdeepin/qt5integration/commit/a045b17e86735779b48ea734bd18f2b2e25b4251))
* **style:**  override drawItemPixmap ([a580b9bf](https://github.com/linuxdeepin/qt5integration/commit/a580b9bf2024cc5e9564283f4880229bac1d780a))
* **theme plugin:**  read settings from config file ([aacc2995](https://github.com/linuxdeepin/qt5integration/commit/aacc299512f4006ff174c5700d471d344d7155d0))
* **theme style:**  auto update the widgets font when theme config file changed ([d478074e](https://github.com/linuxdeepin/qt5integration/commit/d478074e73d8e22e2d70080ad1430c565261d9af))

#### Bug Fixes

*   set cursor is invaild ([a2e235be](https://github.com/linuxdeepin/qt5integration/commit/a2e235bed7873d35030be380e8cd73acc0192e89))
*   menu can not hide on dde-dock ([ecae595e](https://github.com/linuxdeepin/qt5integration/commit/ecae595e29f1da24654c3128f13321c22412a7bd))
*   compatibility with Qt 5.7.1+ ([774ffb89](https://github.com/linuxdeepin/qt5integration/commit/774ffb89733232b8728b897b58db3eab6c7a6e05))
* **dxcb:**
  *  crash when screen changed ([7e1627c2](https://github.com/linuxdeepin/qt5integration/commit/7e1627c20fd92d7b4c1d7a69c55b4de5869cf6b6))
  *  the window border size is wrong ([83d6ac50](https://github.com/linuxdeepin/qt5integration/commit/83d6ac500b7e070f5c97ff220da16736229fd060))
  *  draw shadow area ([284cae9d](https://github.com/linuxdeepin/qt5integration/commit/284cae9dd90e9b9d7af92480e3a0e217a7c5f478))
  *  the DFrameWindow shadow area ([6e9c8274](https://github.com/linuxdeepin/qt5integration/commit/6e9c8274ef871fe3fa48a60a0861a96b6880dbf0))
  *  update the DFrameWindow's positionAutomatic value ([f6331ee3](https://github.com/linuxdeepin/qt5integration/commit/f6331ee32809d7e548b7d538beefc7ea5582f111))
  *  update the content window motif hints on propagateSizeHints/setVisible ([4e490995](https://github.com/linuxdeepin/qt5integration/commit/4e490995f5cfb564dbec705af114467c0ed374f0))
  *  the DFrameWindow's transientParent is self ([d5d93dba](https://github.com/linuxdeepin/qt5integration/commit/d5d93dbae8cdc21e887da17b8d710b0403f3b0aa))
* **style:**
  *  update all widgets on font changed ([0e28f244](https://github.com/linuxdeepin/qt5integration/commit/0e28f244217f57d454424cc7dfba488ee6be93e3))
  *  update all widgets on font changed ([d4a65deb](https://github.com/linuxdeepin/qt5integration/commit/d4a65deb9e21e18390447c8b26eb429843333be4))
* **theme:**
  *  "DFileSystemWatcherPrivate::addPaths: inotify_add_watch failed" ([a0322a68](https://github.com/linuxdeepin/qt5integration/commit/a0322a68447b7d918af49fbba23ebe1d5f53da25))
  *  set ini codec to "utf8" for QSettings ([3a4f7d76](https://github.com/linuxdeepin/qt5integration/commit/3a4f7d7651f6c433ef9735426d936289ac04d6c9))



