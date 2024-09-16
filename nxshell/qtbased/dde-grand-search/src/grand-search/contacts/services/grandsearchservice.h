// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef GRANDSEARCHSERVICE_H
#define GRANDSEARCHSERVICE_H

#include <QObject>
#include <QScopedPointer>

#define GrandSearchViewServiceName          "com.deepin.dde.GrandSearch"
#define GrandSearchViewServicePath          "/com/deepin/dde/GrandSearch"
#define GrandSearchViewServiceInterface     "com.deepin.dde.GrandSearch"

namespace GrandSearch {

class MainWindow;
class GrandSearchServicePrivate;
class GrandSearchService : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", GrandSearchViewServiceInterface)
public:
    explicit GrandSearchService(MainWindow *mainWindow, QObject *parent = nullptr);
    ~GrandSearchService();

    Q_SCRIPTABLE bool IsVisible() const;
    Q_SCRIPTABLE void SetVisible(const bool visible);

signals:
    Q_SCRIPTABLE void VisibleChanged(const bool visible);

private:
    QScopedPointer<GrandSearchServicePrivate> d_p;
};

}

#endif // GRANDSEARCHSERVICE_H
