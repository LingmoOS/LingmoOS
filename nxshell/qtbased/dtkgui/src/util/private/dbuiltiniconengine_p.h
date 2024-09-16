// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DBUILTINICONENGINE_H
#define DBUILTINICONENGINE_H

#include <dtkgui_global.h>

#include <QIconEngine>
#include <private/qiconloader_p.h>

DGUI_BEGIN_NAMESPACE

// 内置的主题引擎，会从Qt资源文件中查找图标
class DBuiltinIconEnginePrivate;
class Q_DECL_HIDDEN DBuiltinIconEngine : public QIconEngine
{
public:
    explicit DBuiltinIconEngine(const QString &iconName);
    ~DBuiltinIconEngine() override;
    void paint(QPainter *painter, const QRect &rect,
               QIcon::Mode mode, QIcon::State state) override;
    QSize actualSize(const QSize &size, QIcon::Mode mode,
                     QIcon::State state) override;
    QPixmap pixmap(const QSize &size, QIcon::Mode mode,
                   QIcon::State state) override;

    QString key() const override;
    QIconEngine *clone() const override;
    bool read(QDataStream &in) override;
    bool write(QDataStream &out) const override;

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QString iconName() override;
#else
    QString iconName() const override;
#endif

    static QThemeIconInfo loadIcon(const QString &iconName, uint key);

private:
    bool hasIcon() const;
    void ensureLoaded();
    void virtual_hook(int id, void *data) override;

    DBuiltinIconEngine(const DBuiltinIconEngine &other);
    QThemeIconInfo m_info;
    QString m_iconName;
    // 图标的类型(Dark/Light)
    uint m_key:2;
    // 记录是否已经初始化
    bool m_initialized:1;
    // 控制是否跟随系统级别的主题色来改变图标类型
    bool m_followSystemTheme:1;

    friend class QIconLoader;
};

DGUI_END_NAMESPACE

#endif // DBUILTINICONENGINE_H
