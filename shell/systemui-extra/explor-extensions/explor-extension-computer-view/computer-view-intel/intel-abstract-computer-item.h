#ifndef INTEL_ABSTRACTCOMPUTERITEM_H
#define INTEL_ABSTRACTCOMPUTERITEM_H

/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2020, Tianjin LINGMO Information Technology Co., Ltd.
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

#include <QObject>
#include <QIcon>
#include <gio/gio.h>

namespace Intel {

class ComputerModel;

class AbstractComputerItem : public QObject
{
    Q_OBJECT
public:
    enum Type {
        Invalid,
        Personal,
        Volume,
        RemoteVolume,
        Network
    };
    Q_ENUM(Type)

    explicit AbstractComputerItem(ComputerModel *model, AbstractComputerItem *parentNode, QObject *parent = nullptr);
    ~AbstractComputerItem();

    virtual Type itemType() {return Invalid;}
    virtual const QString uri() {return nullptr;}
    virtual const QString displayName() {return nullptr;}
    virtual const QIcon icon() {return QIcon::fromTheme("text-plain");}
    virtual bool hasChildren() {return false;}
    virtual void findChildren() {}
    virtual void clearChildren() {}
    virtual void updateInfo() {}
    virtual void check() {}
    virtual bool isMount () {return false;}

    //for volumn
    virtual qint64 totalSpace() {return 0;}
    virtual qint64 usedSpace() {return 0;}

    virtual bool canUnmount() {return false;}
    virtual void unmount(GMountUnmountFlags unmountFlag) {}
    virtual void mount() {}

    virtual bool canEject() {return false;}
    virtual void eject(GMountUnmountFlags ejectFlag) {}

    //for remote volume
    virtual bool isHidden() {return false;}

    virtual QModelIndex itemIndex();

    // add by wwn
    virtual bool isExpanded() {return m_expand;}
    virtual void expand(bool expand) {m_expand = expand;}
public:
    ComputerModel *m_model;
    AbstractComputerItem *m_parentNode;
    QList<AbstractComputerItem *> m_children;

private:
    bool m_expand = false;
};

}

#endif // ABSTRACTCOMPUTERITEM_H
