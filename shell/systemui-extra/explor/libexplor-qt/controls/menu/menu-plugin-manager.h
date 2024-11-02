/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2020, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */

#ifndef MENUPLUGINMANAGER_H
#define MENUPLUGINMANAGER_H

#include <QObject>
#include "explor-core_global.h"
#include "menu-plugin-iface.h"
#include <QListWidgetItem>
#include <QPushButton>
#include <QMenu>
#include <QMainWindow>

#include "color-pushbutton.h"

namespace Peony {

class PEONYCORESHARED_EXPORT MenuPluginManager : public QObject
{
    Q_OBJECT
public:
    bool registerPlugin(MenuPluginInterface *plugin);
    bool unregisterPlugin(MenuPluginInterface *plugin);

    static MenuPluginManager *getInstance();
    const QStringList getPluginIds();
    MenuPluginInterface *getPlugin(const QString &pluginId);

    void close();

private:
    QHash<QString, MenuPluginInterface*> m_hash;

    explicit MenuPluginManager(QObject *parent = nullptr);
    ~MenuPluginManager();
};

class CreateLinkInternalPlugin : public QObject, public MenuPluginInterface
{
    Q_OBJECT
public:
    explicit CreateLinkInternalPlugin(QObject *parent);

    PluginInterface::PluginType pluginType() override {
        return PluginInterface::MenuPlugin;
    }
    const QString name() override {
        return tr("Peony-Qt Create Link Extension");
    }
    const QString description() override {
        return tr("Create Link Menu Extension.");
    }
    const QIcon icon() override {
        return QIcon::fromTheme("emblem-link-symbolic");
    }
    void setEnable(bool enable) override {
        m_enable = enable;
    }
    bool isEnable() override {
        return m_enable;
    }

    QString testPlugin() override {
        return "test create link";
    }
    QList<QAction *> menuActions(Types types, const QString &uri, const QStringList &selectionUris) override;

private:
    bool m_enable = true;
};

class FileLabelWidget;
class FileLabelInternalMenuPlugin : public QObject, public MenuPluginInterface
{
    Q_OBJECT
    friend class FileLabelWidget;
public:
    explicit FileLabelInternalMenuPlugin(QObject *parent);

    PluginInterface::PluginType pluginType() override {
        return PluginInterface::MenuPlugin;
    }
    const QString name() override {
        return tr("Peony File Labels Menu Extension");
    }
    const QString description() override {
        return tr("Tag a File with Menu.");
    }
    const QIcon icon() override {
        return QIcon::fromTheme("emblem-link-symbolic");
    }
    void setEnable(bool enable) override {
        m_enable = enable;
    }
    bool isEnable() override {
        return m_enable;
    }

    QString testPlugin() override {
        return "test create file label";
    }
    QList<QAction *> menuActions(Types types, const QString &uri, const QStringList &selectionUris) override;

private:
    bool m_enable = true;
};

class FileLabelWidget : public QWidget
{
public:
    friend class FileLabelInternalMenuPlugin;
    Q_OBJECT

Q_SIGNALS:
    void changeText(const QString &text);

public Q_SLOTS:
    void clickItem(int index);

protected:
    void paintEvent(QPaintEvent *e);

private:
    explicit FileLabelWidget(const QStringList &selectionUris);
    QStringList  *m_selectionUris;
    QButtonGroup *m_colorgroup;
    ColorPushButton *m_colorbutton;
    QList<int> m_ids;
};

class CreateSharedFileLinkMenuPlugin : public QObject, public MenuPluginInterface
{
    Q_OBJECT
public:
    explicit CreateSharedFileLinkMenuPlugin(QObject *parent);

    PluginInterface::PluginType pluginType() override {
        return PluginInterface::MenuPlugin;
    }
    const QString name() override {
        return tr("Peony-Qt Share File menu Extension");
    }
    const QString description() override {
        return tr("Tag  with Menu.");
    }
    const QIcon icon() override {
        return QIcon::fromTheme("emblem-link-symbolic");
    }
    void setEnable(bool enable) override {
        m_enable = enable;
    }
    bool isEnable() override {
        return m_enable;
    }

    QString testPlugin() override {
        return "test create share file link label";
    }
    QList<QAction *> menuActions(Types types, const QString &uri, const QStringList &selectionUris) override;

private:
    bool m_enable = true;
};

}

#endif // MENUPLUGINMANAGER_H
