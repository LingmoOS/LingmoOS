// SPDX-FileCopyrightText: 2024 - 2027 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef DCCAPP_H
#define DCCAPP_H
#include "oceanuiobject.h"

#include <QObject>

QT_BEGIN_NAMESPACE
class QWindow;
class QAbstractItemModel;
class QSortFilterProxyModel;
QT_END_NAMESPACE

namespace oceanuiV25 {
class OceanUIApp : public QObject
{
    Q_OBJECT
public:
    enum UosEdition {
        UosEditionUnknown,
        UosProfessional,
        UosHome,
        UosCommunity,
        UosMilitary,
        UosEnterprise,
        UosEnterpriseC,
        UosEuler,
        UosMilitaryS, // for Server
        UosDeviceEdition,
        UosEducation,

        UosEditionCount // must at last
    };
    Q_ENUM(UosEdition)

    static OceanUIApp *instance();

    Q_PROPERTY(int width READ width)
    Q_PROPERTY(int height READ height)
    Q_PROPERTY(OceanUIObject * root READ root NOTIFY rootChanged)
    Q_PROPERTY(OceanUIObject * activeObject READ activeObject NOTIFY activeObjectChanged)

    virtual int width() const;
    virtual int height() const;
    virtual OceanUIObject *root() const;
    virtual OceanUIObject *activeObject() const;

public Q_SLOTS:
    virtual OceanUIObject *object(const QString &name);
    virtual void addObject(OceanUIObject *obj);
    virtual void removeObject(OceanUIObject *obj);
    virtual void removeObject(const QString &name);
    virtual void showPage(const QString &url);
    virtual void showPage(OceanUIObject *obj, const QString &cmd);
    virtual QWindow *mainWindow() const;
    virtual QAbstractItemModel *navModel() const;
    virtual QSortFilterProxyModel *searchModel() const;

Q_SIGNALS:
    void rootChanged(OceanUIObject *root);
    void activeObjectChanged(OceanUIObject *activeObject);
    void activeItemChanged(QQuickItem *item);

protected:
    explicit OceanUIApp(QObject *parent = nullptr);
    ~OceanUIApp() override;
};
} // namespace oceanuiV25

#endif // DCCAPP_H
