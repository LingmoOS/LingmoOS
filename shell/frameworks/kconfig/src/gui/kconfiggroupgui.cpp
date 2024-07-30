/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2007 Thiago Macieira <thiago@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kconfiggui_export.h"
#include <kconfiggroup.h>

#include <QColor>
#include <QDebug>
#include <QFont>

#include <kconfiggroup_p.h>

/**
 * Try to read a GUI type from config group @p cg at key @p key.
 * @p input is the default value and also indicates the type to be read.
 * @p output is to be set with the value that has been read.
 *
 * @returns true if something was handled (even if output was set to clear or default)
 *          or false if nothing was handled (e.g., Core type)
 */
static bool readEntryGui(const QByteArray &data, const char *key, const QVariant &input, QVariant &output)
{
    const auto errString = [&]() {
        return QStringLiteral("\"%1\" - conversion from \"%3\" to %2 failed")
            .arg(QLatin1String(key), QLatin1String(input.typeName()), QLatin1String(data.constData()));
    };

    // set in case of failure
    output = input;

    switch (static_cast<QMetaType::Type>(input.userType())) {
    case QMetaType::QColor: {
        if (data.isEmpty() || data == "invalid") {
            output = QColor(); // return what was stored
            return true;
        } else if (data.at(0) == '#') {
            QColor col = QColor::fromString(QUtf8StringView(data.constData(), data.length()));
            if (!col.isValid()) {
                qCritical() << qPrintable(errString());
            }
            output = col;
            return true;
        } else if (!data.contains(',')) {
            QColor col = QColor::fromString(QUtf8StringView(data.constData(), data.length()));
            if (!col.isValid()) {
                qCritical() << qPrintable(errString());
            }
            output = col;
            return true;
        } else {
            const QList<QByteArray> list = data.split(',');
            const int count = list.count();

            if (count != 3 && count != 4) {
                qCritical() //
                    << qPrintable(errString()) //
                    << qPrintable(QStringLiteral(" (wrong format: expected '%1' items, read '%2')").arg(QStringLiteral("3' or '4")).arg(count));
                return true; // return default
            }

            int temp[4];
            // bounds check components
            for (int i = 0; i < count; i++) {
                bool ok;
                const int j = temp[i] = list.at(i).toInt(&ok);
                if (!ok) { // failed to convert to int
                    qCritical() << qPrintable(errString()) << " (integer conversion failed)";
                    return true; // return default
                }
                if (j < 0 || j > 255) {
                    static const char *const components[] = {"red", "green", "blue", "alpha"};
                    qCritical() << qPrintable(errString())
                                << qPrintable(QStringLiteral(" (bounds error: %1 component %2)")
                                                  .arg(QLatin1String(components[i]), //
                                                       j < 0 ? QStringLiteral("< 0") : QStringLiteral("> 255")));
                    return true; // return default
                }
            }
            QColor aColor(temp[0], temp[1], temp[2]);
            if (count == 4) {
                aColor.setAlpha(temp[3]);
            }

            if (aColor.isValid()) {
                output = aColor;
            } else {
                qCritical() << qPrintable(errString());
            }
            return true;
        }
    }

    case QMetaType::QFont: {
        QVariant tmp = QString::fromUtf8(data.constData(), data.length());
        if (tmp.canConvert<QFont>()) {
            output = tmp;
        } else {
            qCritical() << qPrintable(errString());
        }
        return true;
    }
    case QMetaType::QPixmap:
    case QMetaType::QImage:
    case QMetaType::QBrush:
    case QMetaType::QPalette:
    case QMetaType::QIcon:
    case QMetaType::QRegion:
    case QMetaType::QBitmap:
    case QMetaType::QCursor:
    case QMetaType::QSizePolicy:
    case QMetaType::QPen:
        // we may want to handle these in the future

    default:
        break;
    }

    return false; // not handled
}

/**
 * Try to write a GUI type @p prop to config group @p cg at key @p key.
 *
 * @returns true if something was handled (even if an empty value was written)
 *          or false if nothing was handled (e.g., Core type)
 */
static bool writeEntryGui(KConfigGroup *cg, const char *key, const QVariant &prop, KConfigGroup::WriteConfigFlags pFlags)
{
    switch (static_cast<QMetaType::Type>(prop.userType())) {
    case QMetaType::QColor: {
        const QColor rColor = prop.value<QColor>();

        if (!rColor.isValid()) {
            cg->writeEntry(key, "invalid", pFlags);
            return true;
        }

        QList<int> list;
        list.insert(0, rColor.red());
        list.insert(1, rColor.green());
        list.insert(2, rColor.blue());
        if (rColor.alpha() != 255) {
            list.insert(3, rColor.alpha());
        }

        cg->writeEntry(key, list, pFlags);
        return true;
    }
    case QMetaType::QFont: {
        QFont f = prop.value<QFont>();
        // If the styleName property is set for a QFont, using setBold(true) would
        // lead to Qt using an "emboldended"/synthetic font style instead of using
        // the bold style provided by the font itself; the latter looks much better
        // than the former. For more details see:
        // https://bugreports.qt.io/browse/QTBUG-63792
        // https://bugs.kde.org/show_bug.cgi?id=378523
        /* clang-format off */
        if (f.weight() == QFont::Normal
            && (f.styleName() == QLatin1String("Regular")
                || f.styleName() == QLatin1String("Normal")
                || f.styleName() == QLatin1String("Book")
                || f.styleName() == QLatin1String("Roman"))) { /* clang-format on */
            f.setStyleName(QString());
        }
        cg->writeEntry(key, f.toString().toUtf8(), pFlags);
        return true;
    }
    case QMetaType::QPixmap:
    case QMetaType::QImage:
    case QMetaType::QBrush:
    case QMetaType::QPalette:
    case QMetaType::QIcon:
    case QMetaType::QRegion:
    case QMetaType::QBitmap:
    case QMetaType::QCursor:
    case QMetaType::QSizePolicy:
    case QMetaType::QPen:
        // we may want to handle one of these in the future
        break;

    default:
        break;
    }

    return false;
}

// Not static, because for static builds we use it in the kconfigguistaticinitializer.cpp file
// Exported as we need this to force linking in consumers that read GUI types from KConfig, but
// which are otherwise not using this library (and thus linking with --as-needed or MSVC would
// break things)
KCONFIGGUI_EXPORT int initKConfigGroupGui()
{
    _kde_internal_KConfigGroupGui.readEntryGui = readEntryGui;
    _kde_internal_KConfigGroupGui.writeEntryGui = writeEntryGui;
    return 42; // because 42 is nicer than 1 or 0
}

#ifdef Q_CONSTRUCTOR_FUNCTION
Q_CONSTRUCTOR_FUNCTION(initKConfigGroupGui)
#else
static int dummyKConfigGroupGui = initKConfigGroupGui();
#endif
