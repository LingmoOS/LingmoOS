<a name="2.0.14"></a>
## 2.0.14 (2019-05-23)

#### Bug Fixes

* **DSettings:**  crash when calling getOption() if option doesn't exist ([90ac734b](https://github.com/linuxdeepin/dtkcore/commit/90ac734b872203ea698808a7197aa7a9c7e2b5bd))



<a name="2.0.12"></a>
## 2.0.12 (2019-04-18)




<a name="2.0.11"></a>
## 2.0.11 (2019-04-17)


#### Bug Fixes

*   Cross-builds incorrectly, built packages contain paths from build architecture ([8d32577a](https://github.com/linuxdeepin/dtkcore/commit/8d32577a89e54b5c9c834caae83d98e50f59df77))
*   https://github.com/linuxdeepin/dtkcore/issues/10 ([3f99873a](https://github.com/linuxdeepin/dtkcore/commit/3f99873a786f6830688ecd0d8d2e2bf8dfb63ce0))



<a name="2.0.10"></a>
## 2.0.10 (2019-03-27)


#### Bug Fixes

*   crash at application ([d852a218](https://github.com/linuxdeepin/dtkcore/commit/d852a21811f9f86e04274fc9f732d7c7a210ef3f))

#### Features

*   add DNotifySender ([89bbcd7c](https://github.com/linuxdeepin/dtkcore/commit/89bbcd7c3821985bb2bca51247394fd4a65b25bf))



<a name="2.0.9.17"></a>
## 2.0.9.17 (2019-02-26)




<a name="2.0.9.16"></a>
## 2.0.9.16 (2019-02-26)


#### Bug Fixes

*   deepin-os-release support cpu model and other info query ([cbeb47c9](https://github.com/linuxdeepin/dtkcore/commit/cbeb47c97e31d2b5dd3c198c60ee74332fecb293))



<a name="2.0.9.15"></a>
## 2.0.9.15 (2019-01-25)


#### Bug Fixes

*   failed build the deepin-os-release on Qt 5.7.1 ([8bae8654](https://github.com/linuxdeepin/dtkcore/commit/8bae8654bdb20a7f773130d22b9db139460ba575))
*   use main project c/cxx/ld flags on build deepin-os-release ([86dbd507](https://github.com/linuxdeepin/dtkcore/commit/86dbd507c1b3b101c1816f091782430ec1ce20ce))



<a name="2.0.9.14"></a>
## 2.0.9.14 (2019-01-02)




<a name="2.0.9.13"></a>
## 2.0.9.13 (2018-12-28)




<a name="2.0.9.12"></a>
## 2.0.9.12 (2018-12-24)


#### Bug Fixes

* **DPathBuf:**  missing default constructor ([74374cb4](https://github.com/linuxdeepin/dtkcore/commit/74374cb4cf0245ab1fe73f62fe0d13566f945db3))

#### Features

*   support connan build ([ba2d213f](https://github.com/linuxdeepin/dtkcore/commit/ba2d213fd6c7e36e118288305e5892c339250623))



<a name="2.0.9.11"></a>
## 2.0.9.11 (2018-12-14)




<a name="2.0.9.10"></a>
## 2.0.9.10 (2018-12-05)


#### Bug Fixes

*   include unistd.h instead of sys/unistd.h ([39c50a13](https://github.com/linuxdeepin/dtkcore/commit/39c50a1398c34123e3806a3060a4c64e7f45ed68))
*   url encoding ([4a6b7b61](https://github.com/linuxdeepin/dtkcore/commit/4a6b7b61bb3ad9ab417eda69249b5e9aced0aa97))



<a name="2.0.9.9"></a>
## 2.0.9.9 (2018-11-19)


#### Features

*   add DRecentManager class. ([a2defafd](https://github.com/linuxdeepin/dtkcore/commit/a2defafdcf57078461221c665e322287a43d24a8))

#### Bug Fixes

*   compatibility with Qt 5.6 ([0ec7f3ce](https://github.com/linuxdeepin/dtkcore/commit/0ec7f3ce389b323ecb2b103801c1cd1d55f100fa))
* **drecentmanager:**
  *  xbel file does not exist. ([c57ffe71](https://github.com/linuxdeepin/dtkcore/commit/c57ffe714f26b1a8a8859e2ffbeeed3d75ee11a1))
  *  uniform url format. ([413a8988](https://github.com/linuxdeepin/dtkcore/commit/413a8988116708ab8bcf9efbb9bc8f52e048efa5))
  *  url encoded. ([e234a8cc](https://github.com/linuxdeepin/dtkcore/commit/e234a8cc5ad9d2c14a16950838115c4f2f27c605))
* **recent:**  chinese doc ([fb256461](https://github.com/linuxdeepin/dtkcore/commit/fb256461d1bdb0862b1a3a129978fc3932a6bcab))



<a name="2.0.9.8"></a>
## 2.0.9.8 (2018-11-09)


#### Bug Fixes

*   can't get correct disk size in some case ([20a12b62](https://github.com/linuxdeepin/dtkcore/commit/20a12b622ea7b01f0616c15a8af85e31fc2d36cb))



<a name="2.0.9.5"></a>
## 2.0.9.5 (2018-10-26)


#### Features

*   update version number for expermimental ([02b5d5c1](https://github.com/linuxdeepin/dtkcore/commit/02b5d5c1e01a05f57651b774b02cae31ef9a549f))



<a name="2.0.9"></a>
## 2.0.9 (2018-07-20)


#### Bug Fixes

*   remove qt symbols ([57ec78ba](https://github.com/linuxdeepin/dtkcore/commit/57ec78ba685a53692b0260d3d558d8b0915fc3e4))
*   non array type value is wrong on parse josn file ([9f138664](https://github.com/linuxdeepin/dtkcore/commit/9f13866439d8d650893434594da023e7d331d866))



<a name="2.0.8.1"></a>
### 2.0.8.1 (2018-05-14)


#### Bug Fixes

*   update symbols ([f6c53cc4](https://github.com/linuxdeepin/dtkcore/commit/f6c53cc493c1bcf55dca54dbf500e2e484af73c9))
*   add LIBDTKCORESHARED_EXPORT for windows ([6fb1096f](https://github.com/linuxdeepin/dtkcore/commit/6fb1096f6d0784937cf84f0e4ae1f5f7587085e5))
* **changelog:**  update email format ([cb09a0ca](https://github.com/linuxdeepin/dtkcore/commit/cb09a0cadcf2fa0ba271b1d98d3b96a993eb892b))



<a name="2.0.8"></a>
## 2.0.8 (2018-05-02)


#### Features

*   add symbols ([048de455](https://github.com/linuxdeepin/dtkcore/commit/048de4551bdd770aca5e9c12798362f913061654))



<a name="2.0.7"></a>
## 2.0.7 (2018-03-01)


#### Bug Fixes

*   cmake link depends ([cdfcff9e](https://github.com/linuxdeepin/dtkcore/commit/cdfcff9e2f3e92bc6dbb45644d2714d6c4dbdda0))
*   better static lib support ([99886406](https://github.com/linuxdeepin/dtkcore/commit/99886406a0cae849fad23286fdf64bb399e37da0))
*   read settings value failed ([cf1c7698](https://github.com/linuxdeepin/dtkcore/commit/cf1c769893773794dff5a67c235c5d1f3234541a))
*   set default should not use ([146529f6](https://github.com/linuxdeepin/dtkcore/commit/146529f6887e798606f2bf763ab8a760969bff26))
*   fix dtk-settings install path ([1893cff3](https://github.com/linuxdeepin/dtkcore/commit/1893cff301dacb546a246a4f824dab68eac51351))
*   develop package no install the "version.pri" file ([5667b562](https://github.com/linuxdeepin/dtkcore/commit/5667b562630565fca5abed690f3d3478dd3c7603))
*   awk script failed ([524a3fa6](https://github.com/linuxdeepin/dtkcore/commit/524a3fa6021ee54db416503520aea65ef0e2c3a0))
*   set default build version for debian changelog ([ec6e2a83](https://github.com/linuxdeepin/dtkcore/commit/ec6e2a8376c7aca7162b4fbb782b998c9a6ab630))
*   set its value only if VERSION is empty ([1836000c](https://github.com/linuxdeepin/dtkcore/commit/1836000c49eb149a6495322c4cbb1474d5d48204))

#### Features

*   add hide support for group ([e7e4fb66](https://github.com/linuxdeepin/dtkcore/commit/e7e4fb669276fbce61c6378e74ae82573e7c0313))
*   add get option interface ([d8682485](https://github.com/linuxdeepin/dtkcore/commit/d8682485a6737da83fb28f22335f1da1afb8956c))
*   add group interface for DSettingsGroup ([c876180f](https://github.com/linuxdeepin/dtkcore/commit/c876180f535e3027dce63628f31379ef874367ed))
*   support generate cmake with qt function ([524b0559](https://github.com/linuxdeepin/dtkcore/commit/524b055929b7be84375a45f9d10cbc3a0ecac6de))
*   config pkg config with dtk_module ([137b9138](https://github.com/linuxdeepin/dtkcore/commit/137b91388d9b9db24c8136dd4e2c6e690a5712c5))
*   support qt module ([17ca0de9](https://github.com/linuxdeepin/dtkcore/commit/17ca0de9156a320cea32208dcff2f8cdf7d6a237))
*   add the "version.pri" file ([07aab9fd](https://github.com/linuxdeepin/dtkcore/commit/07aab9fd6478c83c7bae1062f64b4bd20b21869c))
*   remove build version from install path ([3bf0bfb5](https://github.com/linuxdeepin/dtkcore/commit/3bf0bfb5f49c3e83d4c36cc33f219150bf3731d8))
*   make version parser easier ([6d3b4ead](https://github.com/linuxdeepin/dtkcore/commit/6d3b4ead7080158d1d8977bf7cf99ae842e574ec))
*   set verion when build ([9083dbd3](https://github.com/linuxdeepin/dtkcore/commit/9083dbd3e29bf9d06b1032901ba13848fa964f4c))
*   add .qmake.conf file ([2890f643](https://github.com/linuxdeepin/dtkcore/commit/2890f643a57c3532ab623410f7c6c6dbfdd6788d))
*   add DtkCore and dtkcore_config.h headers ([308a0cc4](https://github.com/linuxdeepin/dtkcore/commit/308a0cc41101499c04308b4ef3bb2fff4ab8d783))
* **DSettings:**  support set default value ([5fe9bfd0](https://github.com/linuxdeepin/dtkcore/commit/5fe9bfd0a5e20cef7393639712302825b803db29))



<a name="2.0.6"></a>
## 2.0.6 (2018-01-15)




<a name="2.0.5.3"></a>
## 2.0.5.3 (2017-12-27)


#### Bug Fixes

*   Adapt lintian ([27df15df](https://github.com/linuxdeepin/dtkcore/commit/27df15df32788002491a24f06f098a5f849a4988))
*   break forever loop for syncing backend data ([f70e500e](https://github.com/linuxdeepin/dtkcore/commit/f70e500ec2fd5c751e40833bdc4df586614bcff2))

#### Features

* **util:**  add dpinyin ([128d7d67](https://github.com/linuxdeepin/dtkcore/commit/128d7d678e921bc580dd732b14a454973397899c))



<a name="2.0.5.2"></a>
## 2.0.5.2 (2017-11-28)


#### Bug Fixes

*   make macosx build success ([af04bbe1](https://github.com/linuxdeepin/dtkcore/commit/af04bbe193a4b4251908f830d927ebdc8f4459e7))
*   windows build failed ([66c4c812](https://github.com/linuxdeepin/dtkcore/commit/66c4c812eb29634710642f4e9d6b3d69cc692cb2))

#### Features

*   add macro D_DECL_DEPRECATED ([89e49868](https://github.com/linuxdeepin/dtkcore/commit/89e49868f113ef01c03bcf5b6846eec95c428382))



<a name="2.0.5"></a>
## 2.0.5 (2017-11-06)


#### Bug Fixes

*   build failed on used dbasefilewatcher.h project ([34fbe4b3](34fbe4b3))
*   add miss libgsettings-qt-dev ([f61c1b54](f61c1b54))
*   not select python version ([7e7e8832](7e7e8832))

#### Features

*   support gsettingsbackend, remove dsettings-key ([26a29800](26a29800))
*   create gsettingsbackend ([b94b97b1](b94b97b1))
