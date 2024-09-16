<!--
SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.

SPDX-License-Identifier: GPL-3.0-or-later
-->

<a name=""></a>
##  1.5.0 (2019-04-09)




<a name="1.4.8.1"></a>
## 1.4.8.1 (2019-01-25)




<a name="1.4.8"></a>
##  1.4.8 (2018-11-23)


#### Features

* **desktop:**  add deepin vendor. ([d48d0ad3](https://github.com/linuxdeepin/deepin-system-monitor/commit/d48d0ad34c9ed869b36e6df1fb23a448008a19c5))



<a name="1.4.7"></a>
## 1.4.7 (2018-11-09)


#### Bug Fixes

*   can not find some icons. ([9b5f6e32](https://github.com/linuxdeepin/deepin-system-monitor/commit/9b5f6e32dc76d4cc95e155efab5c7108ba8c5502))
*   remove tooltip delay hiding. ([2491c04d](https://github.com/linuxdeepin/deepin-system-monitor/commit/2491c04dd90c351149ca47a9b2de929850aebfe8))



<a name="1.4.6"></a>
### 1.4.6 (2018-08-12)


#### Bug Fixes

*   remove click focus. ([c55009ed](https://github.com/linuxdeepin/deepin-system-monitor/commit/c55009ede66ba61c45236ad8885788d10e68bdca))
*   no display decimals. ([8d229df7](https://github.com/linuxdeepin/deepin-system-monitor/commit/8d229df7585564350c1a00af4b856728a36d2d4f))



<a name="1.4.5"></a>
### 1.4.5 (2018-07-20)




# Change Log
Deepin system monitor: a more user-friendly system monitor.

## [Unreleased]

## [1.4.4] - 2018-05-14
- HiDPI: fixed attribute dialog's icon is not clear.
- HiDPI: fixed listview icon is not render smooth.
- Add tray prefix in title if process is tray process.
- Adjust network grid padding, make it in visible center of left panel.
- Fixed arithmetic overflow error when calcuate disk read/write io.
- Adjust curve width.

## [1.4.3] - 2018-03-14
- Update manual.
- Fixed display error cause by mismatch type: double -> unsigned long.
- Add detach function to avoid segfault when exit application.

## [1.4.2] - 2018-03-06
- Change code version.

## [1.4.1] - 2018-03-06
- Init object double/float value, avoid use un-init value to draw wrong result. Create function 'filterInvalidNumber' to filter Inf/NaN numnber.

## [1.4] - 2018-03-01
- Update translations.
- Follow DTK's API
- Move end process dialog at center of window.
- Add compact mode, provide multi-cpu graphics.
- Add disk monitor graphics.
- Fixed getcmdline crash error cause by cmdline.length() == 0
- Fix searchbar focus problem.
- Use default icon instead, if not found "Icon=" line in desktop file.

## [1.3.7] - 2017-11-28
- Split flatpak cmdline's fullpath to get real filename to find location of flatpak application.

## [1.3.6.1] - 2017-11-22
- Update translation

## [1.3.6] - 2017-11-06
- Make interactive kill support FlatPak application window
- Make first tab display FlatPak application process
- Click 'location' menu item to find FlatPak application location
- Fix the bug that which command that can't found location of dde-file-manager and nautils
- Fix cursor not hide after interactive kill
- Find desktop file from process name if found environ variable 'GIO_LAUNCHED_DESKTOP_FILE' failed from tray process
- Find icon from FlatPak application's local directory
- Rasie window to top of stack window list after call deepin-system-monitor command again
- Change code with DTK's newest setFrame API
- Synchronous translation


