// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DICONPROXYENGINE_H
#define DICONPROXYENGINE_H

#include <dtkgui_global.h>

#include <QIconEngine>

#include "dicontheme.h"

DGUI_BEGIN_NAMESPACE

class Q_DECL_HIDDEN DIconProxyEngine : public QIconEngine
{
public:
    explicit DIconProxyEngine(const QString &iconName, DIconTheme::Options m_option);
    virtual ~DIconProxyEngine() override;
    void paint(QPainter *painter, const QRect &rect, QIcon::Mode mode, QIcon::State state) override;
    QSize actualSize(const QSize &size, QIcon::Mode mode, QIcon::State state) override;
    QPixmap pixmap(const QSize &size, QIcon::Mode mode, QIcon::State state) override;

    QString key() const override;
    QIconEngine *clone() const override;
    bool read(QDataStream &in) override;
    bool write(QDataStream &out) const override;

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QString iconName() override;
#else
    QString iconName() const override;
#endif

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QList<QSize> availableSizes(QIcon::Mode mode = QIcon::Normal, QIcon::State state = QIcon::Off) override;
#else
    QList<QSize> availableSizes(QIcon::Mode mode = QIcon::Normal, QIcon::State state = QIcon::Off) const override;
#endif

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    bool isNull() override;
#endif
    inline QString themeName() const { return m_iconThemeName; }

    QString proxyKey();
private:
    void virtual_hook(int id, void *data) override;

    void ensureEngine();
    DIconProxyEngine(const DIconProxyEngine &other);
    QString m_iconName;
    QString m_iconThemeName;
    QIconEngine *m_iconEngine = nullptr;
    DIconTheme::Options m_option;
};

DGUI_END_NAMESPACE

#endif // DICONPROXYENGINE_H
