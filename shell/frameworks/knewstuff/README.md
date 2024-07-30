# KNewStuff

Framework for downloading and sharing additional application data

## Introduction

The KNewStuff library implements collaborative data sharing for
applications. It uses libattica to support the Open Collaboration Services
specification.


## Usage

There are three parts to KNewStuff:

* *KNewStuffCore* - The core functionality, which takes care of the actual work
  (downloading data and interacting with the remote services). Importantly, this
  library has no dependencies past Tier 1, and so while the entire framework is
  to be considered Tier 3, the KNewStuffCore library can be considered Tier 2
  for integration purposes.
* *KNewStuff* - A Qt Widget based UI library, designed for ease of implementation of
  various UI patterns found through KDE applications (such as the Get New Stuff buttons,
  as well as generic download and upload dialogues)
* *KNewStuffQuick* - A set of Qt Quick based components designed to provide similar
  pattern support as KNewStuff, except for Qt Quick based applications, and specifically
  in Kirigami based applications.

If you are using CMake, you need to find the modules, which can be done by doing one of
the following in your CMakeLists.txt:

Either use the more modern (and compact) component based method (only actually add the
component you need, since both NewStuff and NewStuffQuick depend on NewStuffCore):

    find_package(KF6 COMPONENTS NewStuffCore NewStuff NewStuffQuick)

Or use the old-fashioned syntax

    find_package(KF6NewStuffCore CONFIG) # for the KNewStuffCore library only
    find_package(KF6NewStuff CONFIG) # for the KNewStuff UI library, will pull in KNewStuffCore for you
    find_package(KF6NewStuffQuick CONFIG) # for the KNewStuffQuick UI library, will pull in KNewStuffCore for you

Also remember to link to the library you are using (either KF6::NewStuff or
KF6::NewStuffCore), and for the Qt Quick NewStuffQuick module, add the following
to the QML files where you wish to use the components:

    import org.kde.newstuff 1.0

Finally, because KNewStuffQuick is not a link time requirement, it would be good form
to mark it as a runtime requirement (and describing why you need them), which is done
by adding the following in your CMakeLists.txt sometime after the find statement:

    set_package_properties(KF6NewStuffQuick PROPERTIES
        DESCRIPTION "Qt Quick components used for interacting with remote data services"
        URL "https://api.kde.org/frameworks/knewstuff/html/index.html"
        PURPOSE "Required to Get Hot New Stuff for my application"
        TYPE RUNTIME)

When installing your knsrc configuration file, you should install it into the location
where KNewStuffCore expects it to be found. Do this by using the CMake variable
`KDE_INSTALL_KNSRCDIR` as provided by the KNewStuffCore module, since 5.57.0.
To support older versions you can use the CMake variable `KDE_INSTALL_CONFDIR` from
[Extra-CMake-Modules' KDEInstallDirs](https://api.kde.org/ecm/kde-module/KDEInstallDirs.html).
You can also handle this yourself, which means you will need to feed `Engine::init()`
the full path to the knsrc file.

## Which module should you use?

When building applications designed to fit in with other classic, widget based
applications, the application authors should use KNS3::DownloadDialog for
downloading application content. For uploading KNS3::UploadDialog is used.

When building Qt Quick (and in particular Kirigami) based applications, you can
use the NewStuffList item from the org.kde.newstuff import to achieve a similar
functionality to KNS3::DownloadDialog. You can also use the ItemsModel directly,
if this is not featureful enough. Uploading is currently not exposed in KNewStuffQuick.

If neither of these options are powerful enough for your needs, you can access
the functionality directly through the classes in the KNSCore namespace.

If you are looking for some tutorials, Techbase has a couple of those here:
[Get Hot New Stuff tutorials](https://techbase.kde.org/Development/Tutorials#Get_Hot_New_Stuff).

## Configuration Files

The Engine (and consequently all the various abstracted options like Page and Dialog) is configured by a KNewStuff
Resource and Configuration, or .knsrc file for short, containing the details of how the engine should be set up.

Your application should install a file into the systemwide configuration location defined by the KNSCore CMake module,
which provides the variable `KDE_INSTALL_KNSRCDIR`, which will put the file into something equivalent to
`/usr/share/knsrcfiles/appname.knsrc` (that is, the systemwide data directory, the subdirectory knsrcfiles, and then
appname, which can technically be anything but which should commonly be your application's binary name for consistency
purposes).

As an example, the file might look like this for wallpapers found on the KDE Store:

    [KNewStuff3]
    Name=Wallpapers
    ProvidersUrl=https://autoconfig.kde.org/ocs/providers.xml
    Categories=KDE Wallpaper 1920x1200,KDE Wallpaper 1600x1200
    XdgTargetDir=wallpapers
    Uncompress=archive
    ContentWarning=Static

### Name

This sets a human-readable name that KNewStuff can use to tell the user what type of content this knsrc file
provides access to. It will usually be considerably different in nature to the appname of the file itself.
For example, the book store in [Peruse](https://peruse.kde.org) (an electronic reading app by KDE) has the knsrc
filename `peruse.knsrc` for ease of packaging and distribution, whereas this entry in that file is `Name=Books`.

If there is no name set for the file, the UI might end up looking slightly weird (as this results in exposing
the knsrc filename to the end user, which likely will not be what you are intending).

### ProvidersUrl

To use a local providers file instead of a remote one, you can set `UseLocalProvidersFile=true` instead of
ProvidersUrl. An example of where this would be useful is if you wish to use an
[OPDS feed](https://wiki.mobileread.com/wiki/OPDS). These are likely to not have a providers file, and if you
lack a server on which to host one of your own, being able to simply put it beside the knsrc file becomes useful.
This will make the engine attempt to load a file located next to your knsrc file, with the same name as that
file, with .providers appended. For example, if your configuration file is called `appname.knsrc`, then your
providers file should be named `appname.providers`.

### Categories

The comma-separated list in the entry Categories defines which subsections of the service should be used to fetch content,
and is further shown in a dropdown list on NewStuff.Page and in KNS3::Dialog. They exist in a three-layer format, where the
usual human-readable ID in the knsrc file corresponds to a UID in the service, which has a number of pieces of metadata
(such as translated human-readable titles). See also KNSCore::Engine::categoriesMetadata().

To see which categories are available on a particular Attica provider, you can fetch the list by performing a GET call to the
content/categories object (by, for example, opening it in a browser). For example, for an OCS service with the server address
ocs.server.org, you would fetch `https://ocs.server.org/v1/content/categories`, which will give you an XML file, with
all categories available on that service, and their metadata.

### XdgTargetDir

This defines the location where the downloaded items will be put. This is the name of a directory in your $XDG_DATA_HOME
directory, such as `~/.local/share/wallpapers`, which is what you would get by setting `XdgTargetDir=wallpapers` as in the
example above.

This is what `QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1Char('/') + name` will return.

The two following options are deprecated and should not be used in new code (and are only listed here to allow you
to understand older knsrc files should you come across them):

* `StandardResource`: Not available in KF6, use XdgTargetDir instead.
* `TargetDir`: Since KF6, this is equivalent to XdgTargetDir.

#### InstallPath

Alternatively to using an XDG data directory, you can bypass this and just have a named location in the user's home directory.
To do this, set `InstallPath` instead of `XdgTargetDir`. For example, setting `InstallPath=Applications` will put downloaded
items into `~/Applications`.

### Uncompress

Uncompress can be one of: `always`, `never`, `archive`, `surdir`, or `kpackage`:

* `always`: Assume all downloaded files are archives that need to be extracted
* `never`: Never try to extract the file
* `archive`: If the file is an archive, uncompress it, otherwise just pass it on
* `subdir`: Logic as archive, but decompress into a subdirectory named after the payload filename
* `kpackage`: Require that the downloaded file is a kpackage, and use the KPackage framework for handling installation and removal (since 5.70).
  See also the note on [KPackage Support below](#kpackage-support) below.

### ContentWarning

Depending on store categories, downloaded content might be of more intrinsically innocuous nature such as Images or simple text files—or potentially more security-sensitive, for instance containing executable JavaScript code.
For this reason, the `ContentWarning` key can be set to one of two values appropriate for the content type:
* `Static`: for static contents such as images (this is the default value).
* `Executable`: for content that can include executable scripts such as JavaScript or QML, or textual metadata like .desktop files that can specify executables which will then be run by the application.

### Entry and Download Item Tag Filter

Use the `TagFilter` and `DownloadTagFilter` options to set filters based on the abstract tags which can be present
on both entries and the download items they contain. To see the full documentation on this, read further in
KNSCore::Engine::setTagFilter(const QStringList &filter), but the following is an example of its use in a knsrc file:

    TagFilter=ghns_excluded!=1,data##mimetype==application/cbr+zip,data##mimetype==application/cbr+rar

This will honor the ghns_exclusion tag (the default value if you do not set one, and generally you should include
this entry). It then further filters out anything that does not include a comic book archive in either ZIP or RAR
format in one or more of the download items. Notice in particular that there are two `data##mimetype` entries. Use this
for when a tag may have multiple values.

### Automatic Dead Entry Removal

If you set `RemoveDeadEntries=true`, entries whose installed files have all been deleted without going through
KNewStuff will be removed from the cache. The removal will happen only after all listed files have been removed, which
means that if, for example, an entry was installed from archive, which was decompressed to yield multiple installed files,
if even one of those files remains, the entry will remain marked as installed.

**Note** In KDE Frameworks 6, this option will default to `true`, and you should consider switching it on at this point,
and rework your code to support this functionality, or explicitly set it to `false` now, if you need this to retain its
current functionality.

### Adoption Command

Set the `AdoptionCommand` option to add a supplementary action to the places where entries are displayed which allows the
user to "adopt" the entry (that is, use it in some way, which will depend on the specific type of entry). In NewStuff.Page,
the delegates all get a new button shown on them, and on detail pages, a new action will be shown in either the toolbar
on desktops, or in the context drawer on mobile.

#### Use Label

Usually, once installed, an item will have some canonical way of being used (for example, using a wallpaper will set it
as the wallpaper on the primary desktop, using a color scheme will switch the environment to that scheme), but in some
cases the word "Use" usually used as the label for the action which activates the Adoption Command does not quite fit.
To change this label, you can add an entry like the following to your knsrc file: `UseLabel=Read`, which in this case
will show the word "Read" for that action instead of "Use".

### Upload Assistance

While KNewStuff does not currently handle content uploading, the UI will attempt to guide users in how to upload new entries
on whatever provider their content is currently coming from. If you as an application developer want to explicitly not
suggest that users should do this, add an `UploadEnabled=false` entry to your configuration file.

Not adding this entry to your configuration will cause KNewStuff to add an entry to NewStuff.Page's contextual actions
with the label "Upload...", which pushes a NewStuff.UploadPage to the stack. At this current time, this page simply
gives instructions on how uploading can be done (with specific attention paid to identifying the KDE Store and giving
more specific instructions for that).

### Installation Control

The `InstallationCommand` and `UninstallCommand` entries can be used to handle items once they have been put into their
installation location. For example, you might need to register an item with some service, and unregister it before removing
it, and using these two entries will allow that to happen.

You can also handle the output from the installation process, by returning a non-zero value on exiting, and writing
to standard output. If the return value is non-zero, KNewStuff will report this to the user through various ways,
primarily through the error displayer in NewStuff.Page (and associated components), and the KNS3::DownloadDialog.

An example of this is how Plymouth graphical boot themes are handled, by running the `kplymouththemeinstaller` tool with the
appropriate flags set. You can see the file here: [https://invent.kde.org/plasma/plymouth-kcm/-/blob/master/src/plymouth.knsrc]

### <a id="kpackage-support" />KPackage Support

To make use of the KPackage option described above, in addition to the Uncompress setting above, you should also specify
the type of archive expected by KPackage. While it is possible to deduce this from the package metadata in many situations,
it is not a requirement of the format that this information exists, and we need to have a fallback in the case it is not
available there. As such, you will want to add a `KPackageType` entry to your knsrc file. The following example shows how this
is done for Plasma themes:

    [KNewStuff3]
    Categories=Plasma Theme
    StandardResource=tmp
    TagFilter=ghns_excluded!=1,plasma##version==5
    DownloadTagFilter=plasma##version==5
    Uncompress=kpackage
    KPackageStructure=Plasma/Theme

Using KPackage support will automatically enable the removal of dead entries option. You can override this if you
want to, by explicitly adding `RemoveDeadEntries=false` to your knsrc file, though this would likely result in your
knewstuff cache to end up out of sync at some point.
