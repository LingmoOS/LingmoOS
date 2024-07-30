# Print Manager

Print Manager is a small, relatively self-contained set of components integrated with Plasma System Settings for managing CUPS printer configurations.


## Components

* Set of legacy stand-alone apps:
  * `kde-add-printer` (for adding printers and printer groups)
  * `configure-printer` (for configuring current printers and printer groups)
  * `kde-print-queue` (for managing printer job queues)

* System tray plasmoid that provides quick access to printers and print job status

* KDed module for new device notification

* KCM module 

* KCups Library that provides an interface to the CUPS api as well as a few models that are used by the legacy apps, the KCM and the plasmoid


## External Dependencies

The legacy apps, the KDed module and the KCM make use of the interfaces provided by the [OpenPrinting system-config-printer project.](https://github.com/OpenPrinting/system-config-printer)  These interfaces provide the following features used by print-manager:

* New device notifications
* Device discovery and grouping
* Recommended driver discovery

While not required, it is recommended to install this component for print-manager to provide the best results.

Another useful external package (not required) is [IPP-USB](https://github.com/OpenPrinting/ipp-usb).  IPP-USB is a mechanism that allows USB connected printers to be discovered and configured with IPP Driverless tools.  USB printers that support IPP but for whatever reason, do not support a network interface can usually be configured using IPP-USB.

Of note, if a printer is not discovered on the network or with a USB connection and it does not support IPP, then the printer will have to be configured manually with print-manager.


## General

For Plasma6, the KCM will provide the primary means to add/configure printers and printer groups.  In addition, some of the basic CUPS print server options can be configured.  There are some limitations to KCM add-printer features that mirror the legacy apps:

* Limited support for Windows printers using samba
* Limited support for auto print queue identification for older connection schemes (LPD/SOCKET)

While the legacy `kde-add-printer` app is available, the KCM is the primary means to add printers and printer groups and provides no direct access to the add-printer app.

The KCM also depends on the following features that are present in the legacy `configure-printer` app:

* Media settings (paper size, mode, type, quality, etc)
* Banners, Policies and Allowed Users
  
These features are available via the `configure-printer` app, accessed from the KCM and the plasmoid as a configure action.  In a future release, these features will be integrated with the KCM.

The legacy `kde-print-queue` app is provided as the primary means to manage configured printer job queues.  It remains directly accessible from both the KCM and the plasmoid.


## Building

The easiest way to make changes and test print-manager during development is to [build it with kdesrc-build](https://community.kde.org/Get_Involved/development/Build_software_with_kdesrc-build).

When building print-manager manually, keep in mind that the Qt5 and Qt6 versions will be built by default. To control which versions are built, use the `BUILD_QT5` and `BUILD_QT6` CMake variables.


## Contributing

Like other projects in the KDE ecosystem, contributions are welcome from all. This repository is managed in [KDE Invent](https://invent.kde.org/plasma/print-manager), our GitLab instance.

* Want to contribute code? See the [GitLab wiki page](https://community.kde.org/Infrastructure/GitLab) for a tutorial on how to send a merge request.
* Reporting a bug? Please submit it on the [KDE Bugtracking System](https://bugs.kde.org/enter_bug.cgi?format=guided&product=print-manager). Please do not use the Issues
tab to report bugs.
* Is there a part of print-manager that's not translated? See the [Getting Involved in Translation wiki page](https://community.kde.org/Get_Involved/translation) to see how
you can help translate!

If you get stuck or need help with anything at all, head over to the [KDE New Contributors room](https://go.kde.org/matrix/#/#kde-welcome:kde.org) on Matrix. For questions about print-manager, please ask in the [KDE Development room](https://go.kde.org/matrix/#/#kde-devel:kde.org). See [Matrix](https://community.kde.org/Matrix) for more details.
