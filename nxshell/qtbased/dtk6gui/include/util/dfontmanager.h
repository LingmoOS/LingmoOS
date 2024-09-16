// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DFONTSIZEMANAGER_H
#define DFONTSIZEMANAGER_H
#include <DObject>

#include <dtkgui_global.h>

#include <QFont>
#include <QObject>

DGUI_BEGIN_NAMESPACE

class DFontManagerPrivate;
class DFontManager : public QObject
    , public DTK_CORE_NAMESPACE::DObject
{
    Q_OBJECT

    Q_PROPERTY(QFont t1 READ t1 NOTIFY fontChanged)
    Q_PROPERTY(QFont t2 READ t2 NOTIFY fontChanged)
    Q_PROPERTY(QFont t3 READ t3 NOTIFY fontChanged)
    Q_PROPERTY(QFont t4 READ t4 NOTIFY fontChanged)
    Q_PROPERTY(QFont t5 READ t5 NOTIFY fontChanged)
    Q_PROPERTY(QFont t6 READ t6 NOTIFY fontChanged)
    Q_PROPERTY(QFont t7 READ t7 NOTIFY fontChanged)
    Q_PROPERTY(QFont t8 READ t8 NOTIFY fontChanged)
    Q_PROPERTY(QFont t9 READ t9 NOTIFY fontChanged)
    Q_PROPERTY(QFont t10 READ t10 NOTIFY fontChanged)

    Q_PROPERTY(QFont baseFont READ baseFont WRITE setBaseFont RESET resetBaseFont NOTIFY fontChanged)

public:
    enum SizeType {
        T1,
        T2,
        T3,
        T4,
        T5,
        T6,
        T7,
        T8,
        T9,
        T10,
        NSizeTypes
    };
    Q_ENUM(SizeType)

    DFontManager(QObject *parent = nullptr);
    ~DFontManager() override;

    Q_INVOKABLE int fontPixelSize(SizeType type) const;
    Q_INVOKABLE void setFontPixelSize(SizeType type, int size);

    Q_INVOKABLE static int fontPixelSize(const QFont &font);

    Q_INVOKABLE static QFont get(int pixelSize, const QFont &base);
    inline const QFont get(SizeType type, const QFont &base) const
    {
        return get(fontPixelSize(type), base);
    }
    inline const QFont get(SizeType type) const
    {
        return get(type, baseFont());
    }

    QFont baseFont() const;
    void setBaseFont(const QFont &font);
    void resetBaseFont();

    inline const QFont t1() const
    {
        return get(T1);
    }
    inline const QFont t2() const
    {
        return get(T2);
    }
    inline const QFont t3() const
    {
        return get(T3);
    }
    inline const QFont t4() const
    {
        return get(T4);
    }
    inline const QFont t5() const
    {
        return get(T5);
    }
    inline const QFont t6() const
    {
        return get(T6);
    }
    inline const QFont t7() const
    {
        return get(T7);
    }
    inline const QFont t8() const
    {
        return get(T8);
    }
    inline const QFont t9() const
    {
        return get(T9);
    }
    inline const QFont t10() const
    {
        return get(T10);
    }

Q_SIGNALS:
    void fontChanged();

private:
    D_DECLARE_PRIVATE(DFontManager)
};

DGUI_END_NAMESPACE

#endif // DFONTSIZEMANAGER_H
