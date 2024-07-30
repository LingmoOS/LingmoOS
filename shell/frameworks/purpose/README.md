# Purpose

Offers available actions for a specific purpose

## Introduction

This framework offers the possibility to create integrate services and actions
on any application without having to implement them specifically. Purpose will
offer them mechanisms to list the different alternatives to execute given the
requested action type and will facilitate components so that all the plugins
can receive all the information they need.

## Usage

There's 2 main ways of using Purpose: from C++ and QML/QtQuick.

To import it from QML, import

    import org.kde.purpose

It offers different ways of integrating the actions in an application. For full
control on the procedure, we can use:
* *AlternativesModel* for listing the different possibilities
* *JobView* for displaying a job's status, including configuration

Furthermore, there's the *AlternativesView* component that will integrate all the
process defined below for convenience.

If you want to import in the C++ application, you can import it using CMake by
calling:

    find_package(KF6Purpose)

Or its QMake counterpart. Then you'll have available the Purpose library if it
needs to be done specifically and PurposeWidgets for convenience.

To integrate on the UI, QtQuick will still be used, as the configuration files
provided by the plugins are written in QML. The recommended way to integrate
on a QtWidgets interface is by using the *Purpose::Menu* class that will allow
us to place the integration wherever pleases us. This class will offer us
a pointer to the used *Purpose::AlternativesModel* so that we can specify what kind of
services we're interested in.

## Plugins

### The plugin configuration

There will be 2 files specifying the behavior of a plugin:
* The `*PluginType.json` files.
* The plugin metadata JSON file.

The plugin type will be identified by the file name. It will specify:
* `X-Purpose-InboundArguments` defines the arguments the application must provide.
* `X-Purpose-OutboundArguments` defines the arguments the plugin must provide by
the end of the execution.

In the plugin metadata we will define:
* `X-Purpose-PluginTypes` defines the purposes tackled by the plugin
* `X-Purpose-Constraints` defines the conditions that make plugin is useful, given the
inboundArguments. These are the currently supported constraints
    * mimeType: for example `mimeType:video/*`. Useful to specify which kind of files
    the plugin is interested in.
    * exec: for example `exec:kate`. Can be used to show the plugin only if an executable is
    present in the `$PATH` directories.
    * application: for example `application:org.kde.okular.desktop`. Checks
    that the specified file is present in the `$XDG_DATA_DIRS/applications`
    directories of the system.
    * dbus: for example `dbus:org.kde.kdeconnect`. Will only offer the plugin if
    a certain dbus service is running on the system.
    * `[]`: for example `['exec:bash', 'exec:zsh']`. an array of constraints
    can be used to restrict to either of the conditions instead of all of them.
* `X-Purpose-Configuration` provides a list of extra arguments that the plugin will need.
Ideally everything should be in the plugin type but sometimes we can only wish. This allows
the opportunity to the application to let the user add the missing data.

### Plugin types
The application says what it wants to do, Purpose finds the correct plugins. This
is how we balance decoupling of implementation but keep on top of what the framework
is supposed to do.

An example of such files is the `ExportPluginType.json`:
```json
{
    "KPlugin": {
        "Icon": "edit-paste",
        "Name": "Upload…"
    },
    "X-Purpose-InboundArguments": [ "urls", "mimeType" ],
    "X-Purpose-OutboundArguments": [ "url" ]
}
```

As previously discussed, here we can define the generic tasks that the different
plugins will implement on top, having the inbound arguments as a given and the
outbound as a requirement.

Examples of such plugin types are (hypothetically, not all implemented yet):
* Share: where you can get the different services to share
* GetImage that would list your scanner, camera and also some web services.
* AddContact that would let you add a contact on your address book or
in whichever plugin is offered.

### Plugin creation

There's two approaches to plugin implementation: native plugins and separate
processes.

#### Native
To implement a Qt-based plugin, it will be required to implement a
`Purpose::PluginBase` class, that acts as a factory for its `Purpose::Job`
instances.

These will be the jobs in charge of performing the action the plugin is meant to
do.

Furthermore, a `pluginname_config.qml` will be provided for extra Configuration,
if required.

These plugins can be optionally be executed in-process.

#### Separate Process
Sometimes executing some actions through Qt code can require some extra work.
For those cases, it's possible to implement the plugin in a separate process.
It will require some extra work when it comes to implementing the feedback
process with the main process but it allows to run plugins in any imaginable
technologies.

The file structure for these plugins is the one of defined by [KPackage](https://api.kde.org/frameworks-api/frameworks5-apidocs/kpackage/html/index.html)
which allows to package and distributethe plugins in an archive.

To that end, we will need to provide:
* A `manifest.json` file, that will define the plugin description, capabilities
and requirements.
* A `code/main*` file that will be executed when the plugin action needs happen.
* A `config/config.qml` file that will be in charge of requesting the necessary
information to the user.

### Disallowing plugins
It is possible to globally disable certain plugins through configuration file called
`purposerc` in /etc/xdg (applies to all users) or in ~/.config (applies to current
user).

The disabled plugins are specified as a comma-separated list in the `disabled` key
in the `plugins` group.

```
[plugins]
# Disable KDE Connect and Imgur sharing plugins
disabled=kdeconnectplugin,imgurplugin
```
