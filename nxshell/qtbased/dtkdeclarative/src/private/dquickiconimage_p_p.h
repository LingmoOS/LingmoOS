// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DQUICKICONIMAGE_P_P_H
#define DQUICKICONIMAGE_P_P_H

#include <private/dquickiconimage_p.h>
#ifndef emit
#define emit Q_EMIT
#include <private/qquickimage_p_p.h>
#undef emit
#else
#include <private/qquickimage_p_p.h>
#endif

DQUICK_BEGIN_NAMESPACE

class DQuickIconImagePrivate : public QQuickImagePrivate
{
    Q_DECLARE_PUBLIC(DQuickIconImage)

public:
    void init();
    virtual void maybeUpdateUrl();
    QUrlQuery getUrlQuery();
    DQuickIconImage::Mode getIconMode() const;

    qreal calculateDevicePixelRatio() const;
    bool updateDevicePixelRatio(qreal targetDevicePixelRatio) override;

    void updateBase64Image();

    static QImage requestImageFromBase64(const QString &name, const QSize &requestedSize, qreal devicePixelRatio);

private:
    QString name;
    DQuickIconImage::State state = DQuickIconImage::State::Off;
    DQuickIconImage::Mode mode = DQuickIconImage::Mode::Invalid;
    QColor color;
    QUrl fallbackSource;

protected:
    enum IconType : qint8 {
        ThemeIconName, // 图标名称
        Base64Data, // base64编码的图标图片数据
        FileUrl // 图标文件的url地址
    };

    // 记录此图标是否应该从图标主题中获取。
    // 在freedesktop的规范中，图标可为
    // 一个本地文件，或以base64编码的图片数据。
    // 此处的兼容处理主要是为了让DQuickIconFinder
    // 能直接用于加载desktop文件的图标
    IconType iconType = ThemeIconName;
};

DQUICK_END_NAMESPACE

#endif // DQUICKICONIMAGE_P_P_H
