/*
 * SPDX-FileCopyrightText: 2017~2017 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef _KCM_FCITX_ISO639_H_
#define _KCM_FCITX_ISO639_H_

#include <QMap>
#include <QString>
#include <fcitx-utils/i18n.h>

namespace fcitx {
namespace kcm {

class Iso639 {
public:
    Iso639();

    QString query(const QString &code) const {
        auto value = iso639_2data_.value(code);
        if (!value.isEmpty()) {
            return translateDomain("iso_639-2", value.toUtf8().constData());
        }
        value = iso639_3data_.value(code);
        if (!value.isEmpty()) {
            return translateDomain("iso_639-3", value.toUtf8().constData());
        }
        value = iso639_5data_.value(code);
        if (!value.isEmpty()) {
            return translateDomain("iso_639-5", value.toUtf8().constData());
        }
        return value;
    }

private:
    QMap<QString, QString> iso639_2data_;
    QMap<QString, QString> iso639_3data_;
    QMap<QString, QString> iso639_5data_;
};

} // namespace kcm
} // namespace fcitx

#endif // _KCM_FCITX_ISO639_H_
