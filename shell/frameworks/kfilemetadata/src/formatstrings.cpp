/*
    SPDX-FileCopyrightText: 2018 Alexander Stippich <a.stippich@gmx.net>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "formatstrings_p.h"

#include <math.h>
#include <QDateTime>
#include <KLocalizedString>
#include <KFormat>

using namespace KFileMetaData;

/*
 * Calculates and returns the number of digits after the
 * comma to always show three significant digits for use
 * with KFormat::formatValue.
 */
int threeSignificantDigits(int value)
{
    if (value == 0) {
        return 0;
    }
    int before_decimal_point = static_cast<int>(log10(value > 0 ? value : -value)) % 3;
    return 2 - before_decimal_point;
}

QString FormatStrings::toStringFunction(const QVariant& value)
{
    return value.toString();
}

QString FormatStrings::formatDouble(const QVariant& value)
{
    return QLocale().toString(value.toDouble(),'g',3);
}

QString FormatStrings::formatDate(const QVariant& value)
{
    KFormat form;
    QDateTime dt;
    if (value.userType() == QMetaType::QDateTime) {
        dt = value.toDateTime();
    } else {
        dt = QDateTime::fromString(value.toString(), Qt::ISODate);
    }
    if (dt.isValid()) {
        return form.formatRelativeDateTime(dt, QLocale::LongFormat);
    }
    return QString();
}

QString FormatStrings::formatDuration(const QVariant& value)
{
    KFormat form;
    return form.formatDuration(value.toInt() * 1000);
}

QString FormatStrings::formatBitRate(const QVariant& value)
{
    KFormat form;
    return i18nc("@label bitrate (per second)", "%1/s", form.formatValue(value.toInt(),
         KFormat::Unit::Bit, threeSignificantDigits(value.toInt()), KFormat::UnitPrefix::AutoAdjust, KFormat::MetricBinaryDialect));
}

QString FormatStrings::formatSampleRate(const QVariant& value)
{
    KFormat form;
    return form.formatValue(value.toInt(), KFormat::Unit::Hertz, threeSignificantDigits(value.toInt()), KFormat::UnitPrefix::AutoAdjust, KFormat::MetricBinaryDialect);
}

QString FormatStrings::formatOrientationValue(const QVariant& value)
{
    QString string;
    switch (value.toInt()) {
    case 1: string = i18nc("Description of image orientation", "Unchanged"); break;
    case 2: string = i18nc("Description of image orientation", "Horizontally flipped"); break;
    case 3: string = i18nc("Description of image orientation", "180° rotated"); break;
    case 4: string = i18nc("Description of image orientation", "Vertically flipped"); break;
    case 5: string = i18nc("Description of image orientation", "Transposed"); break;
    case 6: string = i18nc("Description of image orientation, counter clock-wise rotated", "90° rotated CCW "); break;
    case 7: string = i18nc("Description of image orientation", "Transversed"); break;
    case 8: string = i18nc("Description of image orientation, counter clock-wise rotated", "270° rotated CCW"); break;
    default:
        break;
    }
    return string;
}

QString FormatStrings::formatPhotoFlashValue(const QVariant& value)
{
    // copied from exiv2 tags_int.cpp
    const QMap<int, QString> flashTranslation = {
        { 0x00, i18nc("Description of photo flash", "No flash")                                                      },
        { 0x01, i18nc("Description of photo flash", "Fired")                                                         },
        { 0x05, i18nc("Description of photo flash", "Fired, return light not detected")                              },
        { 0x07, i18nc("Description of photo flash", "Fired, return light detected")                                  },
        { 0x08, i18nc("Description of photo flash", "Yes, did not fire")                                             },
        { 0x09, i18nc("Description of photo flash", "Yes, compulsory")                                               },
        { 0x0d, i18nc("Description of photo flash", "Yes, compulsory, return light not detected")                    },
        { 0x0f, i18nc("Description of photo flash", "Yes, compulsory, return light detected")                        },
        { 0x10, i18nc("Description of photo flash", "No, compulsory")                                                },
        { 0x14, i18nc("Description of photo flash", "No, did not fire, return light not detected")                   },
        { 0x18, i18nc("Description of photo flash", "No, auto")                                                      },
        { 0x19, i18nc("Description of photo flash", "Yes, auto")                                                     },
        { 0x1d, i18nc("Description of photo flash", "Yes, auto, return light not detected")                          },
        { 0x1f, i18nc("Description of photo flash", "Yes, auto, return light detected")                              },
        { 0x20, i18nc("Description of photo flash", "No flash function")                                             },
        { 0x30, i18nc("Description of photo flash", "No, no flash function")                                         },
        { 0x41, i18nc("Description of photo flash", "Yes, red-eye reduction")                                        },
        { 0x45, i18nc("Description of photo flash", "Yes, red-eye reduction, return light not detected")             },
        { 0x47, i18nc("Description of photo flash", "Yes, red-eye reduction, return light detected")                 },
        { 0x49, i18nc("Description of photo flash", "Yes, compulsory, red-eye reduction")                            },
        { 0x4d, i18nc("Description of photo flash", "Yes, compulsory, red-eye reduction, return light not detected") },
        { 0x4f, i18nc("Description of photo flash", "Yes, compulsory, red-eye reduction, return light detected")     },
        { 0x50, i18nc("Description of photo flash", "No, red-eye reduction")                                         },
        { 0x58, i18nc("Description of photo flash", "No, auto, red-eye reduction")                                   },
        { 0x59, i18nc("Description of photo flash", "Yes, auto, red-eye reduction")                                  },
        { 0x5d, i18nc("Description of photo flash", "Yes, auto, red-eye reduction, return light not detected")       },
        { 0x5f, i18nc("Description of photo flash", "Yes, auto, red-eye reduction, return light detected")           }
    };
    if (flashTranslation.contains(value.toInt())) {
        return flashTranslation.value(value.toInt());
    } else {
        return i18n("Unknown");
    }

}

QString FormatStrings::formatAsDegree(const QVariant& value)
{
    return i18nc("Symbol of degree, no space", "%1°", QLocale().toString(value.toDouble()));
}

QString FormatStrings::formatAsMeter(const QVariant& value)
{
    KFormat form;
    return form.formatValue(value.toDouble(), KFormat::Unit::Meter, 1, KFormat::UnitPrefix::AutoAdjust, KFormat::MetricBinaryDialect);
}

QString FormatStrings::formatAsMilliMeter(const QVariant& value)
{
    return i18nc("Focal length given in mm", "%1 mm", QLocale().toString(value.toDouble(), 'g', 3));
}

QString FormatStrings::formatAsFrameRate(const QVariant& value)
{
    return i18nc("Symbol of frames per second, with space", "%1 fps", QLocale().toString(round(value.toDouble() * 100) / 100));
}

QString FormatStrings::formatPhotoTime(const QVariant& value)
{
    auto val = value.toDouble();
    if (val < 0.3 && !qFuzzyIsNull(val)) {
        auto reciprocal = 1.0/val;
        auto roundedReciprocal = round(reciprocal);
        if (abs(reciprocal - roundedReciprocal) < 1e-3) {
            return i18nc("Time period given in seconds as rational number, denominator is given", "1/%1 s", roundedReciprocal);
        }
    }
    return i18nc("Time period given in seconds", "%1 s", QLocale().toString(value.toDouble(), 'g', 3));
}

QString FormatStrings::formatPhotoExposureBias(const QVariant& value)
{
    QLocale locale;
    auto val = value.toDouble();
    /*
     * Exposure values are mostly in steps of one half or third.
     * Try to construct a rational number from it.
     * Output as double when it is not possible.
     */
    auto sixthParts = val * 6;
    int roundedSixthParts = static_cast<int>(round(sixthParts));
    int fractional = roundedSixthParts % 6;
    if (fractional == 0 || abs(sixthParts - roundedSixthParts) > 1e-3) {
        return i18nc("Exposure bias/compensation in exposure value (EV)", "%1 EV", locale.toString(val, 'g', 3));
    }
    int integral = roundedSixthParts / 6;
    int nominator = fractional;
    int denominator = 6;
    if (nominator % 2 == 0) {
        nominator = nominator / 2;
        denominator = denominator / 2;
    } else if (nominator % 3 == 0) {
        nominator = nominator / 3;
        denominator = denominator / 3;
    }
    if (integral != 0) {
        return i18nc("Exposure compensation given as integral with fraction, in exposure value (EV)",
                     "%1 %2/%3 EV", locale.toString(integral), locale.toString(abs(nominator)), locale.toString(denominator));
    }
    return i18nc("Exposure compensation given as rational, in exposure value (EV)",
                 "%1/%2 EV",  locale.toString(nominator), locale.toString(denominator));
}

QString FormatStrings::formatAspectRatio(const QVariant& value)
{
    return i18nc("Aspect ratio, normalized to one", "%1:1", QLocale().toString(round(value.toDouble() * 100) / 100));
}

QString FormatStrings::formatAsFNumber(const QVariant& value)
{
    return i18nc("F number for photographs", "f/%1", QLocale().toString(value.toDouble(), 'g', 2));
}
