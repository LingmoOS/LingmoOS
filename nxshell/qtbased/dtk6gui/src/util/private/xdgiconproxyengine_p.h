// SPDX-FileCopyrightText: 2022-2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef XDGICONPROXYENGINE_H
#define XDGICONPROXYENGINE_H

#include <dtkgui_global.h>
#include <QHash>

#if XDG_ICON_VERSION_MAR >= 3
#include <QIconEngine>

#endif

namespace DEEPIN_XDG_THEME {
enum PaletteType {
    Text,
    Background,
    Highlight,
};
typedef QMap<PaletteType, QString> PALETTE_MAP;
};

struct ScalableEntry;
class QIconLoaderEngineEntry;
class XdgIconLoaderEngine;
DGUI_BEGIN_NAMESPACE

#if (XDG_ICON_VERSION_MAR >= 3)
class Q_DECL_HIDDEN XdgIconProxyEngine : public QIconEngine
{
public:
    XdgIconProxyEngine(XdgIconLoaderEngine *proxy);
    virtual ~XdgIconProxyEngine() override;

    static quint64 entryCacheKey(const ScalableEntry *color_entry, const QIcon::Mode mode, const QIcon::State state);

    QPixmap followColorPixmap(ScalableEntry *color_entry, const QSize &size, QIcon::Mode mode, QIcon::State state);

    QPixmap pixmapByEntry(QIconLoaderEngineEntry *entry, const QSize &size, QIcon::Mode mode, QIcon::State state);
    void paint(QPainter *painter, const QRect &rect, QIcon::Mode mode, QIcon::State state) override;
    QPixmap pixmap(const QSize &size, QIcon::Mode mode, QIcon::State state) override;
    void addPixmap(const QPixmap &pixmap, QIcon::Mode mode, QIcon::State state) override;
    void addFile(const QString &fileName, const QSize &size, QIcon::Mode mode, QIcon::State state) override;
    QString key() const override;
    QSize actualSize(const QSize &size, QIcon::Mode mode, QIcon::State state) override;
    QIconEngine *clone() const override;
    bool read(QDataStream &in) override;
    bool write(QDataStream &out) const override;
    void virtual_hook(int id, void *data) override;

private:
    XdgIconLoaderEngine *engine;
    QHash<quint64, DEEPIN_XDG_THEME::PALETTE_MAP> entryToColorScheme;
    QIcon::Mode lastMode;
};
#endif

DGUI_END_NAMESPACE

#endif // XDGICONPROXYENGINE_H
