/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: hxf <hewenfei@kylinos.cn>
 */

#ifndef ERNIE_BOT_ERNIE_BOT_H
#define ERNIE_BOT_ERNIE_BOT_H

#include <lingmo-menu/menu-extension-plugin.h>
#include <lingmo-menu/widget-extension.h>

class ErnieBotPrivate;

class ErnieBotPlugin : public LingmoUIMenu::MenuExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID LINGMO_MENU_EXTENSION_I_FACE_IID FILE "metadata.json")
    Q_INTERFACES(LingmoUIMenu::MenuExtensionPlugin)
public:
    ~ErnieBotPlugin() override;
    QString id() override;
    LingmoUIMenu::WidgetExtension *createWidgetExtension() override;
    LingmoUIMenu::ContextMenuExtension *createContextMenuExtension() override;
};

class ErnieBot : public LingmoUIMenu::WidgetExtension
{
    Q_OBJECT
public:
    explicit ErnieBot(QObject *parent = nullptr);

    int index() const override;
    LingmoUIMenu::MetadataMap metadata() const override;
    QVariantMap data() override;
    void receive(const QVariantMap &data) override;

private:
    ErnieBotPrivate *d {nullptr};
};


#endif //ERNIE_BOT_ERNIE_BOT_H
