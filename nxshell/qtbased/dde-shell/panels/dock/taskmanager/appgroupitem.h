// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dsglobal.h"
#include "abstractitem.h"
#include "desktopfileabstractparser.h"

#include <QList>

namespace dock {
/// for AppGroup docked on dock
class AppGroupItem : public AbstractItem
{
    Q_OBJECT
    // indetifier
    Q_PROPERTY(QString id READ id)
    Q_PROPERTY(ItemType itemType READ itemType)

    Q_PROPERTY(QString name READ name NOTIFY nameChanged)
    Q_PROPERTY(QString menus READ menus NOTIFY menusChanged)
    Q_PROPERTY(QString icon READ icon NOTIFY iconChanged FINAL)
    Q_PROPERTY(bool isActive READ isActive NOTIFY activeChanged)

    Q_PROPERTY(bool isDocked READ isDocked WRITE setDocked NOTIFY dockedChanged)

public:
    ~AppGroupItem();

    virtual QString id() override;
    virtual ItemType itemType() override;

    virtual QString icon() override;
    virtual QString name() override;
    virtual QString menus() override;

    virtual bool isActive() override;
    virtual void active() override;

    virtual bool isDocked() override;
    virtual void setDocked(bool docked) override;

    virtual void handleClick(const QString& clickItem) override;

protected:
    AppGroupItem(QString id, QObject *parent = nullptr);

Q_SIGNALS:
    void nameChanged();
    void iconChanged();
    void menusChanged();

    void activeChanged();
    void dockedChanged();

private:
    QList<QSharedPointer<DesktopfileAbstractParser>> m_desktopfiles;
};
}
