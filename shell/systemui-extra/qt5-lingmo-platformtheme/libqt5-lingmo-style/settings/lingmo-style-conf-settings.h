/*
 * Qt5-LINGMO's Library
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: xibowen <xibowen@kylinos.cn>
 *
 */

#ifndef LINGMOSTYLECONFSETTINGS_H
#define LINGMOSTYLECONFSETTINGS_H


#include <QObject>
#include <QVariant>

typedef struct  _KSettings KSettings;

class lingmoStyleConfSettings : public QObject
{
    Q_OBJECT
public:

    enum VariantClass {
        VARIANT_CLASS_BOOLEAN       = 'b',
        VARIANT_CLASS_BYTE          = 'y',
        VARIANT_CLASS_INT16         = 'n',
        VARIANT_CLASS_UINT16        = 'q',
        VARIANT_CLASS_INT32         = 'i',
        VARIANT_CLASS_UINT32        = 'u',
        VARIANT_CLASS_INT64         = 'x',
        VARIANT_CLASS_UINT64        = 't',
        VARIANT_CLASS_HANDLE        = 'h',
        VARIANT_CLASS_DOUBLE        = 'd',
        VARIANT_CLASS_STRING        = 's',
        VARIANT_CLASS_OBJECT_PATH   = 'o',
        VARIANT_CLASS_SIGNATURE     = 'g',
        VARIANT_CLASS_VARIANT       = 'v',
        VARIANT_CLASS_MAYBE         = 'm',
        VARIANT_CLASS_ARRAY         = 'a',
        VARIANT_CLASS_TUPLE         = '(',
        VARIANT_CLASS_DICT_ENTRY    = '{'
    };


    lingmoStyleConfSettings(const QByteArray &schema_id);

    ~lingmoStyleConfSettings();

    QVariant types_to_qvariant(const char *key, const char *value) const;

    static lingmoStyleConfSettings *globalInstance();

    QVariant get(const QString &key) const;

    void set(const QString &key, const QVariant &value);

    bool trySet(const QString &key, const QVariant &value);

    QStringList keys() const;

    void reset(const QString &key);

    static bool isSettingsAvailable(const QString &schema_id);

Q_SIGNALS:
    void changed(const QString &key);

private:
    KSettings *lingmostyle_settings = NULL;
    long signal_handler_id;
};

#endif // LINGMOSTYLECONFSETTINGS_H
