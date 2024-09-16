// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DQUICKIMAGEPROVIDER_P_H
#define DQUICKIMAGEPROVIDER_P_H

#include <QQuickImageProvider>

#include <dtkdeclarative_global.h>

DQUICK_BEGIN_NAMESPACE

class DQuickIconProvider : public QQuickImageProvider
{
public:
    DQuickIconProvider();

protected:
    QImage requestImage(const QString &id, QSize *size, const QSize &requestedSize) override;
};

class DQuickDciIconProvider : public QQuickImageProvider
{
public:
    DQuickDciIconProvider();

protected:
    QImage requestImage(const QString &id, QSize *size, const QSize &requestedSize) override;
};

class ShadowImage;
class DQuickShadowProvider : public QQuickImageProvider
{
public:
    DQuickShadowProvider();
    ~DQuickShadowProvider();

    enum Type : quint8 {
        Rectangle = 0,

        Inner = 2
    };

    struct ShadowConfig {
        quint8 type = Rectangle;
        qreal topLeftRadius {0.0};
        qreal topRightRadius {0.0};
        qreal bottomLeftRadius {0.0};
        qreal bottomRightRadius {0.0};
        qreal blurRadius {0.0};
        qreal spread {0.0};
        qreal boxSize {0.0};

        ShadowConfig() {}
        ShadowConfig(const ShadowConfig &other)
            : type(other.type)
            , topLeftRadius(other.topLeftRadius)
            , topRightRadius(other.topRightRadius)
            , bottomLeftRadius(other.bottomLeftRadius)
            , bottomRightRadius(other.bottomRightRadius)
            , blurRadius(other.blurRadius)
            , spread(other.spread)
            , boxSize(other.boxSize) {}

        inline bool isInner() const {
            return type & Inner;
        }

        inline QString toString() const {
            return QString("%1.%2.%3.%4.%5.%6.%7.%8").arg(type).
                arg(topLeftRadius).arg(topRightRadius).arg(bottomLeftRadius).arg(bottomRightRadius).
                arg(blurRadius).arg(spread).arg(boxSize);
        }

        inline ShadowConfig &operator=(const ShadowConfig &other) {
            type = other.type;
            topLeftRadius =  other.topLeftRadius;
            topRightRadius =  other.topRightRadius;
            bottomLeftRadius =  other.bottomLeftRadius;
            bottomRightRadius =  other.bottomRightRadius;
            blurRadius = other.blurRadius;
            spread = other.spread;

            return *this;
        }

        inline bool operator==(const ShadowConfig &other) const {
            return type == other.type
                    && qFuzzyCompare(topLeftRadius, other.topLeftRadius)
                    && qFuzzyCompare(topRightRadius, other.topRightRadius)
                    && qFuzzyCompare(bottomLeftRadius, other.bottomLeftRadius)
                    && qFuzzyCompare(bottomRightRadius, other.bottomRightRadius)
                    && qFuzzyCompare(blurRadius, other.blurRadius)
                    && qFuzzyCompare(spread, other.spread)
                    && qFuzzyCompare(boxSize, other.boxSize);
        }
        inline bool operator!=(const ShadowConfig &other) const {
            return !operator==(other);
        }

        friend Q_CORE_EXPORT uint qHash(const ShadowConfig &config, uint seed) noexcept;
    };

    static QUrl toUrl(qreal boxSize, qreal topLeftRadius, qreal topRightRadius, qreal bottomLeftRadius, qreal bottomRightRadius, qreal shadowBlur, QColor color,
                      qreal xOffset, qreal yOffset, qreal spread, bool hollow, bool inner);

protected:
    QImage requestImage(const QString &id, QSize *size, const QSize &requestedSize) override;

    inline static QString getShadowFilePath(const DQuickShadowProvider::ShadowConfig &config)
    {
        return ":/dtk/declarative/shadow/" + config.toString() + ".png";
    }
    ShadowImage *getRawShadow(const ShadowConfig &config);

private:
    friend class ShadowImage;
    QHash<ShadowConfig, ShadowImage*> cache;
};

inline uint qHash(const DTK_QUICK_NAMESPACE::DQuickShadowProvider::ShadowConfig &config, uint seed = 0) noexcept {
    return QT_PREPEND_NAMESPACE(qHash)(static_cast<uint>(config.type), seed) ^
            QT_PREPEND_NAMESPACE(qHash)(config.topLeftRadius, seed) ^
            QT_PREPEND_NAMESPACE(qHash)(config.topRightRadius, seed) ^
            QT_PREPEND_NAMESPACE(qHash)(config.bottomLeftRadius, seed) ^
            QT_PREPEND_NAMESPACE(qHash)(config.bottomRightRadius, seed) ^
            QT_PREPEND_NAMESPACE(qHash)(config.blurRadius, seed) ^
            QT_PREPEND_NAMESPACE(qHash)(config.spread, seed);
}

DQUICK_END_NAMESPACE

#endif // DQUICKIMAGEPROVIDER_P_H
