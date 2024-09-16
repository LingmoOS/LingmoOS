// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DQUICKBEHINDWINDOWBLUR_P_H
#define DQUICKBEHINDWINDOWBLUR_P_H

#include <DPlatformHandle>

#include <dtkdeclarative_global.h>

#include <QQuickItem>

DQUICK_BEGIN_NAMESPACE

class DQuickWindowAttached;
class DQuickBehindWindowBlurPrivate;
class DQuickBehindWindowBlur : public QQuickItem
{
    Q_OBJECT
    Q_PROPERTY(qreal cornerRadius READ cornerRadius WRITE setCornerRadius NOTIFY cornerRadiusChanged)
    Q_PROPERTY(QColor blendColor READ blendColor WRITE setBlendColor NOTIFY blendColorChanged)
    Q_PROPERTY(bool valid READ valid NOTIFY validChanged)
    Q_PROPERTY(bool blurEnabled READ blurEnabled WRITE setBlurEnabled NOTIFY blurEnabledChanged)
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QML_NAMED_ELEMENT(BehindWindowBlur)
#endif

public:
    explicit DQuickBehindWindowBlur(QQuickItem *parent = nullptr);
    ~DQuickBehindWindowBlur() override;

    qreal cornerRadius() const;
    void setCornerRadius(qreal newRadius);

    const QColor &blendColor() const;
    void setBlendColor(const QColor &newBlendColor);

    bool valid() const;

    bool blurEnabled() const;
    void setBlurEnabled(bool newBlurEnabled);

Q_SIGNALS:
    void cornerRadiusChanged();
    void blendColorChanged();
    void validChanged();
    void blurEnabledChanged();

protected:
    QSGNode *updatePaintNode(QSGNode *, UpdatePaintNodeData *) override;
    void itemChange(ItemChange change, const ItemChangeData &value);
    void componentComplete() override;

private:
    void setWindowAttached(DQuickWindowAttached *wa);

private:
    Q_DISABLE_COPY(DQuickBehindWindowBlur)
    Q_DECLARE_PRIVATE(DQuickBehindWindowBlur)
    Q_PRIVATE_SLOT(d_func(), void _q_onHasBlurWindowChanged())
    Q_PRIVATE_SLOT(d_func(), void _q_updateBlurArea())
    friend class DQuickWindowAttachedPrivate;
    friend class DSGBlendNode;
};

DQUICK_END_NAMESPACE

#endif // DQUICKBEHINDWINDOWBLUR_P_H
