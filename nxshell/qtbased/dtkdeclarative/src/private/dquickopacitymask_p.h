// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DQUICKOPACITYMASK_P_H
#define DQUICKOPACITYMASK_P_H

#include <dtkdeclarative_global.h>

#include <QQuickItem>

DQUICK_BEGIN_NAMESPACE

class DQuickOpacityMaskPrivate;
class DQuickOpacityMask : public QQuickItem
{
    Q_OBJECT
    Q_PROPERTY(bool invert READ invert WRITE setInvert NOTIFY invertChanged)
    Q_PROPERTY(QQuickItem *source READ source WRITE setSource NOTIFY sourceChanged)
    Q_PROPERTY(QQuickItem *maskSource READ maskSource WRITE setMaskSource NOTIFY maskSourceChanged)
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QML_NAMED_ELEMENT(SoftwareOpacityMask)
#endif

public:
    explicit DQuickOpacityMask(QQuickItem *parent = nullptr);

    bool invert();
    void setInvert(bool invert);

    QQuickItem *source() const;
    void setSource(QQuickItem *item);

    QQuickItem *maskSource() const;
    void setMaskSource(QQuickItem *item);

Q_SIGNALS:
    void invertChanged();
    void sourceChanged();
    void maskSourceChanged();

protected:
    QSGNode *updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *) override;

protected:
    DQuickOpacityMask(DQuickOpacityMaskPrivate &dd, QQuickItem *parent = nullptr);

private:
    Q_DISABLE_COPY(DQuickOpacityMask)
    Q_DECLARE_PRIVATE(DQuickOpacityMask)
};

DQUICK_END_NAMESPACE

#endif // DQUICKOPACITYMASK_P_H
