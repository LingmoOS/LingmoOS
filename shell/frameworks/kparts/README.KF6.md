## KF6 Porting notes

### Plugin registration

In KPluginFactory the `registerPlugin(QWidget *parentWidget, QObject *parent, const QVariantList &args)` method was removed in favor of
`registerPlugin(QWidget *parentWidget, QObject *parent, const KPluginMetaData &data, const QVariantList &args)`.
You should adjust your constructors accordingly and pass in the `KPluginMetaData` object to the KParts-superclass you are extending.

Also, you are allowed to remove the `const QVariantList &args` constructor parameter, in case your part does not have any logic using it.

### Part::setMetaData
This method was removed in favor of passing the KPluginMetaData object directly into the constructor of the KParts baseclass.

### BrowserExtension

This class was renamed to NavigationExtension, because it is not limited to web browsing.
`ReadOnlyPart::browserExtension` was consequently renamed to `ReadOnlyPart::navigationExtension`.

### Events
The `KParts::Event` baseclass was removed in favor of events extending `QEvent` directly. The static `::test` methods stay the same, but use `QEvent::Type` internally.
In case you want to port event subclasses generate an ID using `shuf -i1000-65535 -n 1`.
