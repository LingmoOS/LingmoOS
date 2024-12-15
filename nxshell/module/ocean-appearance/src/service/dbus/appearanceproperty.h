// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef APPEARANCEPROPERTY_H
#define APPEARANCEPROPERTY_H

#include <QVariant>

void AppearancePropertiesChanged(const QString &property, const QVariant &value);

template<typename T>
class DBusProperty
{
public:
    explicit DBusProperty(QString name)
        : m_name(name) { }
    inline void init(const T &initValue)
    {
        m_data = initValue;
    }

    inline T operator=(const T &newValue)
    {
        if (m_data != newValue) {
            m_data = newValue;
            AppearancePropertiesChanged(m_name, QVariant::fromValue(m_data));
        }
        return newValue;
    }
    inline T data() const
    {
        return m_data;
    }
    inline operator T() const
    {
        return m_data;
    }
    inline const T *operator->() const
    {
        return &m_data;
    }

private:
    T m_data;
    QString m_name;
};

class AppearanceProperty
{
public:
    explicit AppearanceProperty();

public:
    DBusProperty<QString> background;
    DBusProperty<QString> cursorTheme;
    DBusProperty<double> fontSize;
    DBusProperty<QString> globalTheme;
    DBusProperty<QString> gtkTheme;
    DBusProperty<QString> iconTheme;
    DBusProperty<QString> monospaceFont;
    DBusProperty<double> opacity;
    DBusProperty<QString> qtActiveColor;
    DBusProperty<QString> standardFont;
    DBusProperty<QString> wallpaperSlideShow;
    DBusProperty<QString> wallpaperURls;
    DBusProperty<int> windowRadius;
    DBusProperty<int> dtkSizeMode;
    DBusProperty<int> qtScrollBarPolicy;
};

#endif // APPEARANCEPROPERTY_H
