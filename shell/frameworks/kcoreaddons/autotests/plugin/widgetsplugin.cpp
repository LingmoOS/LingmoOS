// SPDX-FileCopyrightText: 2023 Alexander Lohnau <alexander.lohnau@gmx.de>
// SPDX-License-Identifier: LGPL-2.0-or-later
#include <QWidget>
#include <kpluginfactory.h>

class MyWidget : public QWidget
{
public:
    explicit MyWidget(QWidget *parent, const KPluginMetaData &data)
        : QWidget(parent)
    {
        Q_ASSERT(!data.fileName().isEmpty());
    }
};
class MyWidgetArgs : public QWidget
{
public:
    explicit MyWidgetArgs(QWidget *parent, const KPluginMetaData &data, const QVariantList &args)
        : QWidget(parent)
    {
        Q_ASSERT(!data.fileName().isEmpty());
        setProperty("firstarg", args.first());
    }
};

class MyWidgetNoMetaData : public QWidget
{
public:
    explicit MyWidgetNoMetaData(QWidget *parent)
        : QWidget(parent)
    {
    }
};

// Ignore the duplicate registration, just make sure it compiles
K_PLUGIN_FACTORY(MyWidgetFactory, registerPlugin<MyWidgetArgs>(); registerPlugin<MyWidget>(); registerPlugin<MyWidgetNoMetaData>();)

#include "widgetsplugin.moc"
