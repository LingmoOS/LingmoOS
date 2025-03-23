// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include <gtest/gtest.h>

#include <QQmlComponent>
#include <QQmlEngine>
#include <QString>
#include <DGuiApplicationHelper>
#include <QQuickView>
#include <QTest>
#include <QQmlExpression>
#include <QQuickItem>
#include <private/qsgplaintexture_p.h>

#define TEST_OFFSCREEN_SKIP() \
if (qEnvironmentVariable("QT_QPA_PLATFORM") == "offscreen") \
    GTEST_SKIP();

#define TEST_NOTSOFTWARE_SKIP() \
if (QQuickWindow::sceneGraphBackend() != "software") \
        GTEST_SKIP_("Only test `QSGRendererInterface::Software` backend");

#define TEST_SOFTWARE_SKIP() \
if (QQuickWindow::sceneGraphBackend() == "software") \
        GTEST_SKIP_("Only test non `QSGRendererInterface::Software` backend");

#define TEST_QRHI_SKIP() \
if (TestUtil::isRunningOnRhi()) \
    GTEST_SKIP_("Only test non RHI Render, Render nodes not yet supported with QRhi");

class EnvGuard {
public:
    EnvGuard(const char *name, const QString &value)
    {
        m_name = name;
        m_originValue = qgetenv(m_name);
        qputenv(m_name, value.toUtf8());
    }
    ~EnvGuard()
    {
        qputenv(m_name, m_originValue);
    }
private:
    QByteArray m_originValue;
    const char* m_name = nullptr;
};

template<typename T = QQuickItem>
T *findItem(QQuickItem *parent, const QString &objectName = QString(), int index = -1)
{
    const QMetaObject &mo = T::staticMetaObject;
    for (int i = 0; i < parent->childItems().count(); ++i) {
        QQuickItem *item = qobject_cast<QQuickItem*>(parent->childItems().at(i));
        if (!item)
            continue;
        if (mo.cast(item) && (objectName.isEmpty() || item->objectName() == objectName)) {
            if (index != -1) {
                QQmlExpression e(qmlContext(item), item, "index");
                if (e.evaluate().toInt() == index)
                    return static_cast<T*>(item);
            } else {
                return static_cast<T*>(item);
            }
        }
        item = findItem<T>(item, objectName, index);
        if (item)
            return static_cast<T*>(item);
    }

    return nullptr;
}

static inline void updateEngineImportPath(QQmlEngine &engine)
{
    QStringList paths {
        QString::fromLocal8Bit(QML_PLUGIN_PATH),
        QString::fromLocal8Bit(CHAMELEON_PATH)
    };
    paths << engine.importPathList();
    engine.setImportPathList(paths);
}

template<class T = QObject>
class ControlHelper
{
public:
    ControlHelper()
    {
        updateEngineImportPath(engine);
    }
    ControlHelper(const QString &url)
        : ControlHelper()
    {
        load(url);
    }
    bool load(const QString &url)
    {
        clear();

        QQmlComponent component(&engine, QUrl(url), QQmlComponent::PreferSynchronous);

        return (object = create(component));
    }
    bool setData(const QByteArray &data)
    {
        clear();

        QQmlComponent component(&engine);
        component.setData(data, QUrl());

        return (object = create(component));
    }
    T *create(QQmlComponent &component)
    {
        if (!component.isReady()) {
            qWarning() << "component is not ready" << component.errorString();
            return nullptr;
        }

        auto tmp = component.create();
        if (!tmp) {
            qWarning() << "create object is wrong." << component.errorString();
            return nullptr;
        }

        return static_cast<T *>(tmp);
    }
    void requestExposed()
    {
        if (!object)
            return;

        object->show();
        QVERIFY(QTest::qWaitForWindowExposed(object));
    }
    ~ControlHelper()
    {
        clear();
    }
    void clear()
    {
        if (object)
            object->deleteLater();
    }
    QQmlEngine engine;
    T *object = nullptr;
};

template<class T = QQuickItem>
class QuickViewHelper {
public:
    QuickViewHelper()
        : view(new QQuickView)
    {
        updateEngineImportPath(*view->engine());
    }
    ~QuickViewHelper()
    {
        clear();
    }
    QuickViewHelper(const QString &url)
        : QuickViewHelper()
    {
        load(url);
        requestExposed();
    }
    bool load(const QString &url)
    {
        clear();

        view->setSource(url);

        QQuickItem *rootItem = view->rootObject();

        object = qobject_cast<T *>(rootItem);
        return object != nullptr;

    }
    void requestActivate()
    {
        view->requestActivate();
        QVERIFY(QTest::qWaitForWindowActive(view.data()));
    }
    void requestExposed()
    {
        requestExposed(view.data());
    }
    static void requestExposed(QQuickWindow *view)
    {
        view->show();
        QVERIFY(QTest::qWaitForWindowExposed(view));
    }
    void clear()
    {
        if (object)
            object->deleteLater();
    }
    QScopedPointer<QQuickView> view;
    T *object = nullptr;
};

DGUI_USE_NAMESPACE
class ThemeTypeGuard {
public:
    ThemeTypeGuard(DGuiApplicationHelper::ColorType expectedType)
    {
        oldType = DGuiApplicationHelper::instance()->themeType();
        DGuiApplicationHelper::instance()->setPaletteType(expectedType);
    }
    ~ThemeTypeGuard()
    {
        DGuiApplicationHelper::instance()->setPaletteType(oldType);
    }
private:
    DGuiApplicationHelper::ColorType oldType;
};

namespace TestUtil {
    const QColor simpleColor(0, 0, 255, 55);
    inline QSGTexture *simpleTexture(const QColor &color = simpleColor, const QSize &size = QSize(100, 100))
    {
        QImage image(size, QImage::Format_ARGB32_Premultiplied);
        image.fill(color);
        return QSGPlainTexture::fromImage(image);
    }

    inline QSGTexture *simpleTextureThreePart(const int opacity = 255, const QSize &size = QSize(100, 100))
    {
        QImage image(size, QImage::Format_ARGB32_Premultiplied);
        const QVector<QColor> partColors = {
            QColor(255, 0, 0, opacity),
            QColor(0, 255, 0, opacity),
            QColor(0, 0, 255, opacity),
        };
        int offset = 0;
        const int pHeight = image.height() / partColors.count();
        for (int i = 0; i < partColors.count(); i++) {
            int height = i < partColors.count() - 1 ? pHeight : image.height() - pHeight * i;
            QImage img(image.bits() + offset, image.width(), height, image.format());
            img.fill(partColors.at(i));
            offset += img.sizeInBytes();
        }
        return QSGPlainTexture::fromImage(image);
    }

    Q_GLOBAL_STATIC_WITH_ARGS(QImage, simpleImage, (":/icon/deepin.png"))
    inline QSGTexture *imageTexture(const QSize &size = QSize(100, 100))
    {
        if (!simpleImage.exists())
            simpleImage->convertTo(QImage::Format_ARGB32_Premultiplied);

        QImage tmp = simpleImage->scaled(size, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        return QSGPlainTexture::fromImage(tmp);
    }

    inline bool equalBlurColor(const QColor &color1, const QColor &color2)
    {
        const qreal delta = 0.1;
        if ((qAbs(color1.redF() - color2.redF()) < delta) &&
            qAbs(color1.greenF() - color2.greenF()) < delta &&
            qAbs(color1.blueF() - color2.blueF()) < delta)
            return true;

        return false;
    }

    template<class T>
    inline void registerType(const char* type)
    {
        qmlRegisterType<T>("test", 1, 0, type);
    }

    inline bool supportOpengl(const QSGRendererInterface::GraphicsApi ga)
    {
#ifndef QT_NO_OPENGL
        return (ga == QSGRendererInterface::OpenGL
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
                || ga == QSGRendererInterface::OpenGLRhi
                );
#endif
#endif
        return false;
    }

    inline bool isRunningOnRhi()
    {
        static int retval = -1;
        if (retval == -1) {
            QuickViewHelper<> helper;
            helper.requestExposed();
            retval = QSGRendererInterface::isApiRhiBased(helper.view->rendererInterface()->graphicsApi());
        }
        return static_cast<bool>(retval);
    }
}
