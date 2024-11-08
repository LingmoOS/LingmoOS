/*
    SPDX-FileCopyrightText: 2013 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef CONFIGVIEW_H
#define CONFIGVIEW_H

#include <QQuickView>
#include <QScopedPointer>

#include <lingmoquick/lingmoquick_export.h>
#include <qquickitem.h>

//
//  W A R N I N G
//  -------------
//
// This file is not part of the public Lingmo API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

namespace Lingmo
{
class Applet;
}

namespace LingmoQuick
{
class ConfigViewPrivate;

class ConfigModel;

class LINGMOQUICK_EXPORT ConfigView : public QQuickWindow
{
    Q_OBJECT
    Q_PROPERTY(LingmoQuick::ConfigModel *configModel READ configModel CONSTANT)
    Q_PROPERTY(QString appletGlobalShortcut READ appletGlobalShortcut WRITE setAppletGlobalShortcut NOTIFY appletGlobalShortcutChanged)

public:
    /**
     * @param applet the applet of this ConfigView
     * @param parent the QWindow in which this ConfigView is parented to
     **/
    ConfigView(Lingmo::Applet *applet, QWindow *parent = nullptr);
    ~ConfigView() override;

    QQmlEngine *engine();
    QQmlContext *rootContext();
    void setSource(const QUrl &src);
    QQuickItem *rootObject();

    virtual void init();

    Lingmo::Applet *applet();

    QString appletGlobalShortcut() const;
    void setAppletGlobalShortcut(const QString &shortcut);

    /**
     * @return the ConfigModel of the ConfigView
     **/
    LingmoQuick::ConfigModel *configModel() const;

Q_SIGNALS:
    void appletGlobalShortcutChanged();

protected:
    void hideEvent(QHideEvent *ev) override;
    void resizeEvent(QResizeEvent *re) override;

private:
    QScopedPointer<ConfigViewPrivate> const d;

    Q_PRIVATE_SLOT(d, void updateMinimumWidth())
    Q_PRIVATE_SLOT(d, void updateMinimumHeight())
    Q_PRIVATE_SLOT(d, void updateMaximumWidth())
    Q_PRIVATE_SLOT(d, void updateMaximumHeight())
    Q_PRIVATE_SLOT(d, void updateTitle())
};

}

#endif // multiple inclusion guard
