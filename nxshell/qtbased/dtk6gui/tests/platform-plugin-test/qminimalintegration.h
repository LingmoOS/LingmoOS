// SPDX-FileCopyrightText: 2023 Uniontech Software Technology Co.,Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef QPLATFORMINTEGRATION_MINIMAL_H
#define QPLATFORMINTEGRATION_MINIMAL_H

#include <qpa/qplatformintegration.h>
#include <qpa/qplatformscreen.h>

#include <qscopedpointer.h>

class MinimalScreen : public QPlatformScreen
{
public:
    MinimalScreen()
        : mDepth(32), mFormat(QImage::Format_ARGB32_Premultiplied) {}

    QRect geometry() const override { return mGeometry; }
    int depth() const override { return mDepth; }
    QImage::Format format() const override { return mFormat; }

public:
    QRect mGeometry;
    int mDepth;
    QImage::Format mFormat;
    QSize mPhysicalSize;
};

class MinimalIntegration : public QPlatformIntegration
{
public:
    explicit MinimalIntegration(const QStringList &parameters);
    ~MinimalIntegration() override;

    QPlatformWindow *createPlatformWindow(QWindow *window) const override;
    QPlatformBackingStore *createPlatformBackingStore(QWindow *window) const override;
    QAbstractEventDispatcher *createEventDispatcher() const override;

    QPlatformServices *services() const override;
    QPlatformNativeInterface *nativeInterface() const override;
    QStringList themeNames() const override;

    static MinimalIntegration *instance();

private:
    mutable QScopedPointer<QPlatformNativeInterface> m_nativeInterface;
    mutable QScopedPointer<QPlatformServices> m_services;
    MinimalScreen *m_primaryScreen;
};

#endif
