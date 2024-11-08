# explorer-extensions

## Getting Start
- Make sure all build dependencies have been installed in your machine. You can reference debian/control for building depends.

* Create *build* directory and change to it, then build with cmake. The follow
  commands will auto build extensions as library.

``` sh
#cmake ..
qmake ..
make
```
- Put the generated libraries into &&[QT_INSTALL_LIBS]/explorer-extensions.
  
- Test the extension with explorer.

## Current extensions

- **terminal**, open terminal with right click menu.
- **shared**, a share properties page in properties window.
- ~~**admin**, open a directory or a file as root with menu.~~
- **parchives**, compress/uncompress files with menu.
- **computer view**, a special view for displaying computer:///.
- **send-to-device**, provides a directory menu action for sending selected files to mounted device.
- **bluetooth**, provides a directory menu action for sneding selected files by bluetooth, depending on lingmo-bluetooth.
- **set-wallparper**, privides a directory menu action for set desktop background.

## binary compatibility
Peony-Extensions strongly relies on Peony library, that means the binary files of those plugins might not compat with Peony when the library updated and some API changed. Sometimes the old plugins will not be loaded, or will make Peony crashed due to call an unmatch method. To avoid the binary compatibility problems, we add a version checkment in latest version both in this project and Peony. The plugins' version infomation are loaded from [common.json](common.json), this must match to the Peony VERSION marco.

When explorer extensions doesn't work or can't be build correctly, please consider using latest Peony library built locally, and rebuild this project again.
## Translations
Unlike Peony, Peony Extensions translations job is relatively primitive and crude. The main idea is every plugin translates itself, and compile translations resources with code (qrc).

When you are going to translate explorer extensions (I created), here are the mainly steps.

- use lupdate to generate .ts file for your language, the naming rules reference existing files.
- use lrelease to publish .ts file to .qm file.
- modify the project's .qrc file, add your .qm file into recources to be compiled.
