/*
 *   SPDX-FileCopyrightText: 2007-2009 Petri Damstén <damu@iki.fi>
 *   SPDX-FileCopyrightText: 2014 John Layt <jlayt@kde.org>
 *
 *   SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "unit_p.h"
#include "volume_p.h"

#include <KLocalizedString>

namespace KUnitConversion
{

UnitCategory Volume::makeCategory()
{
    auto c = UnitCategoryPrivate::makeCategory(VolumeCategory, i18n("Volume"), i18n("Volume"));
    auto d = UnitCategoryPrivate::get(c);
    KLocalizedString symbolString = ki18nc("%1 value, %2 unit symbol (volume)", "%1 %2");

    d->addUnit(UnitPrivate::makeUnit(VolumeCategory,
                       CubicYottameter,
                       1e+72,
                       i18nc("volume unit symbol", "Ym³"),
                       i18nc("unit description in lists", "cubic yottameters"),
                       i18nc("unit synonyms for matching user input", "cubic yottameter;cubic yottameters;Ym³;Ym/-3;Ym^3;Ym3"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 cubic yottameters"),
                       ki18ncp("amount in units (integer)", "%1 cubic yottameter", "%1 cubic yottameters")));

    d->addUnit(UnitPrivate::makeUnit(VolumeCategory,
                       CubicZettameter,
                       1e+63,
                       i18nc("volume unit symbol", "Zm³"),
                       i18nc("unit description in lists", "cubic zettameters"),
                       i18nc("unit synonyms for matching user input", "cubic zettameter;cubic zettameters;Zm³;Zm/-3;Zm^3;Zm3"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 cubic zettameters"),
                       ki18ncp("amount in units (integer)", "%1 cubic zettameter", "%1 cubic zettameters")));

    d->addUnit(UnitPrivate::makeUnit(VolumeCategory,
                       CubicExameter,
                       1e+54,
                       i18nc("volume unit symbol", "Em³"),
                       i18nc("unit description in lists", "cubic exameters"),
                       i18nc("unit synonyms for matching user input", "cubic exameter;cubic exameters;Em³;Em/-3;Em^3;Em3"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 cubic exameters"),
                       ki18ncp("amount in units (integer)", "%1 cubic exameter", "%1 cubic exameters")));

    d->addUnit(UnitPrivate::makeUnit(VolumeCategory,
                       CubicPetameter,
                       1e+45,
                       i18nc("volume unit symbol", "Pm³"),
                       i18nc("unit description in lists", "cubic petameters"),
                       i18nc("unit synonyms for matching user input", "cubic petameter;cubic petameters;Pm³;Pm/-3;Pm^3;Pm3"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 cubic petameters"),
                       ki18ncp("amount in units (integer)", "%1 cubic petameter", "%1 cubic petameters")));

    d->addUnit(UnitPrivate::makeUnit(VolumeCategory,
                       CubicTerameter,
                       1e+36,
                       i18nc("volume unit symbol", "Tm³"),
                       i18nc("unit description in lists", "cubic terameters"),
                       i18nc("unit synonyms for matching user input", "cubic terameter;cubic terameters;Tm³;Tm/-3;Tm^3;Tm3"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 cubic terameters"),
                       ki18ncp("amount in units (integer)", "%1 cubic terameter", "%1 cubic terameters")));

    d->addUnit(UnitPrivate::makeUnit(VolumeCategory,
                       CubicGigameter,
                       1e+27,
                       i18nc("volume unit symbol", "Gm³"),
                       i18nc("unit description in lists", "cubic gigameters"),
                       i18nc("unit synonyms for matching user input", "cubic gigameter;cubic gigameters;Gm³;Gm/-3;Gm^3;Gm3"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 cubic gigameters"),
                       ki18ncp("amount in units (integer)", "%1 cubic gigameter", "%1 cubic gigameters")));

    d->addUnit(UnitPrivate::makeUnit(VolumeCategory,
                       CubicMegameter,
                       1e+18,
                       i18nc("volume unit symbol", "Mm³"),
                       i18nc("unit description in lists", "cubic megameters"),
                       i18nc("unit synonyms for matching user input", "cubic megameter;cubic megameters;Mm³;Mm/-3;Mm^3;Mm3"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 cubic megameters"),
                       ki18ncp("amount in units (integer)", "%1 cubic megameter", "%1 cubic megameters")));

    d->addUnit(UnitPrivate::makeUnit(VolumeCategory,
                       CubicKilometer,
                       1e+09,
                       i18nc("volume unit symbol", "km³"),
                       i18nc("unit description in lists", "cubic kilometers"),
                       i18nc("unit synonyms for matching user input", "cubic kilometer;cubic kilometers;km³;km/-3;km^3;km3"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 cubic kilometers"),
                       ki18ncp("amount in units (integer)", "%1 cubic kilometer", "%1 cubic kilometers")));

    d->addUnit(UnitPrivate::makeUnit(VolumeCategory,
                       CubicHectometer,
                       1e+06,
                       i18nc("volume unit symbol", "hm³"),
                       i18nc("unit description in lists", "cubic hectometers"),
                       i18nc("unit synonyms for matching user input", "cubic hectometer;cubic hectometers;hm³;hm/-3;hm^3;hm3"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 cubic hectometers"),
                       ki18ncp("amount in units (integer)", "%1 cubic hectometer", "%1 cubic hectometers")));

    d->addUnit(UnitPrivate::makeUnit(VolumeCategory,
                       CubicDecameter,
                       1000,
                       i18nc("volume unit symbol", "dam³"),
                       i18nc("unit description in lists", "cubic decameters"),
                       i18nc("unit synonyms for matching user input", "cubic decameter;cubic decameters;dam³;dam/-3;dam^3;dam3"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 cubic decameters"),
                       ki18ncp("amount in units (integer)", "%1 cubic decameter", "%1 cubic decameters")));

    d->addCommonUnit(UnitPrivate::makeUnit(VolumeCategory,
                             CubicMeter,
                             1,
                             i18nc("volume unit symbol", "m³"),
                             i18nc("unit description in lists", "cubic meters"),
                             i18nc("unit synonyms for matching user input", "cubic meter;cubic meters;m³;m/-3;m^3;m3"),
                             symbolString,
                             ki18nc("amount in units (real)", "%1 cubic meters"),
                             ki18ncp("amount in units (integer)", "%1 cubic meter", "%1 cubic meters")));

    d->addUnit(UnitPrivate::makeUnit(VolumeCategory,
                       CubicDecimeter,
                       0.001,
                       i18nc("volume unit symbol", "dm³"),
                       i18nc("unit description in lists", "cubic decimeters"),
                       i18nc("unit synonyms for matching user input", "cubic decimeter;cubic decimeters;dm³;dm/-3;dm^3;dm3"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 cubic decimeters"),
                       ki18ncp("amount in units (integer)", "%1 cubic decimeter", "%1 cubic decimeters")));

    d->addUnit(UnitPrivate::makeUnit(VolumeCategory,
                       CubicCentimeter,
                       1e-06,
                       i18nc("volume unit symbol", "cm³"),
                       i18nc("unit description in lists", "cubic centimeters"),
                       i18nc("unit synonyms for matching user input", "cubic centimeter;cubic centimeters;cm³;cm/-3;cm^3;cm3"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 cubic centimeters"),
                       ki18ncp("amount in units (integer)", "%1 cubic centimeter", "%1 cubic centimeters")));

    d->addUnit(UnitPrivate::makeUnit(VolumeCategory,
                       CubicMillimeter,
                       1e-09,
                       i18nc("volume unit symbol", "mm³"),
                       i18nc("unit description in lists", "cubic millimeters"),
                       i18nc("unit synonyms for matching user input", "cubic millimeter;cubic millimeters;mm³;mm/-3;mm^3;mm3"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 cubic millimeters"),
                       ki18ncp("amount in units (integer)", "%1 cubic millimeter", "%1 cubic millimeters")));

    d->addUnit(UnitPrivate::makeUnit(VolumeCategory,
                       CubicMicrometer,
                       1e-18,
                       i18nc("volume unit symbol", "µm³"),
                       i18nc("unit description in lists", "cubic micrometers"),
                       i18nc("unit synonyms for matching user input", "cubic micrometer;cubic micrometers;µm³;um³;µm/-3;µm^3;µm3"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 cubic micrometers"),
                       ki18ncp("amount in units (integer)", "%1 cubic micrometer", "%1 cubic micrometers")));

    d->addUnit(UnitPrivate::makeUnit(VolumeCategory,
                       CubicNanometer,
                       1e-27,
                       i18nc("volume unit symbol", "nm³"),
                       i18nc("unit description in lists", "cubic nanometers"),
                       i18nc("unit synonyms for matching user input", "cubic nanometer;cubic nanometers;nm³;nm/-3;nm^3;nm3"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 cubic nanometers"),
                       ki18ncp("amount in units (integer)", "%1 cubic nanometer", "%1 cubic nanometers")));

    d->addUnit(UnitPrivate::makeUnit(VolumeCategory,
                       CubicPicometer,
                       1e-36,
                       i18nc("volume unit symbol", "pm³"),
                       i18nc("unit description in lists", "cubic picometers"),
                       i18nc("unit synonyms for matching user input", "cubic picometer;cubic picometers;pm³;pm/-3;pm^3;pm3"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 cubic picometers"),
                       ki18ncp("amount in units (integer)", "%1 cubic picometer", "%1 cubic picometers")));

    d->addUnit(UnitPrivate::makeUnit(VolumeCategory,
                       CubicFemtometer,
                       1e-45,
                       i18nc("volume unit symbol", "fm³"),
                       i18nc("unit description in lists", "cubic femtometers"),
                       i18nc("unit synonyms for matching user input", "cubic femtometer;cubic femtometers;fm³;fm/-3;fm^3;fm3"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 cubic femtometers"),
                       ki18ncp("amount in units (integer)", "%1 cubic femtometer", "%1 cubic femtometers")));

    d->addUnit(UnitPrivate::makeUnit(VolumeCategory,
                       CubicAttometer,
                       1e-54,
                       i18nc("volume unit symbol", "am³"),
                       i18nc("unit description in lists", "cubic attometers"),
                       i18nc("unit synonyms for matching user input", "cubic attometer;cubic attometers;am³;am/-3;am^3;am3"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 cubic attometers"),
                       ki18ncp("amount in units (integer)", "%1 cubic attometer", "%1 cubic attometers")));

    d->addUnit(UnitPrivate::makeUnit(VolumeCategory,
                       CubicZeptometer,
                       1e-63,
                       i18nc("volume unit symbol", "zm³"),
                       i18nc("unit description in lists", "cubic zeptometers"),
                       i18nc("unit synonyms for matching user input", "cubic zeptometer;cubic zeptometers;zm³;zm/-3;zm^3;zm3"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 cubic zeptometers"),
                       ki18ncp("amount in units (integer)", "%1 cubic zeptometer", "%1 cubic zeptometers")));

    d->addUnit(UnitPrivate::makeUnit(VolumeCategory,
                       CubicYoctometer,
                       1e-72,
                       i18nc("volume unit symbol", "ym³"),
                       i18nc("unit description in lists", "cubic yoctometers"),
                       i18nc("unit synonyms for matching user input", "cubic yoctometer;cubic yoctometers;ym³;ym/-3;ym^3;ym3"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 cubic yoctometers"),
                       ki18ncp("amount in units (integer)", "%1 cubic yoctometer", "%1 cubic yoctometers")));

    d->addUnit(UnitPrivate::makeUnit(VolumeCategory,
                       Yottaliter,
                       1e+21,
                       i18nc("volume unit symbol", "Yl"),
                       i18nc("unit description in lists", "yottaliters"),
                       i18nc("unit synonyms for matching user input", "yottaliter;yottaliters;Yl"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 yottaliters"),
                       ki18ncp("amount in units (integer)", "%1 yottaliter", "%1 yottaliters")));

    d->addUnit(UnitPrivate::makeUnit(VolumeCategory,
                       Zettaliter,
                       1e+18,
                       i18nc("volume unit symbol", "Zl"),
                       i18nc("unit description in lists", "zettaliters"),
                       i18nc("unit synonyms for matching user input", "zettaliter;zettaliters;Zl"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 zettaliters"),
                       ki18ncp("amount in units (integer)", "%1 zettaliter", "%1 zettaliters")));

    d->addUnit(UnitPrivate::makeUnit(VolumeCategory,
                       Exaliter,
                       1e+15,
                       i18nc("volume unit symbol", "El"),
                       i18nc("unit description in lists", "exaliters"),
                       i18nc("unit synonyms for matching user input", "exaliter;exaliters;El"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 exaliters"),
                       ki18ncp("amount in units (integer)", "%1 exaliter", "%1 exaliters")));

    d->addUnit(UnitPrivate::makeUnit(VolumeCategory,
                       Petaliter,
                       1e+12,
                       i18nc("volume unit symbol", "Pl"),
                       i18nc("unit description in lists", "petaliters"),
                       i18nc("unit synonyms for matching user input", "petaliter;petaliters;Pl"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 petaliters"),
                       ki18ncp("amount in units (integer)", "%1 petaliter", "%1 petaliters")));

    d->addUnit(UnitPrivate::makeUnit(VolumeCategory,
                       Teraliter,
                       1e+09,
                       i18nc("volume unit symbol", "Tl"),
                       i18nc("unit description in lists", "teraliters"),
                       i18nc("unit synonyms for matching user input", "teraliter;teraliters;Tl"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 teraliters"),
                       ki18ncp("amount in units (integer)", "%1 teraliter", "%1 teraliters")));

    d->addUnit(UnitPrivate::makeUnit(VolumeCategory,
                       Gigaliter,
                       1e+06,
                       i18nc("volume unit symbol", "Gl"),
                       i18nc("unit description in lists", "gigaliters"),
                       i18nc("unit synonyms for matching user input", "gigaliter;gigaliters;Gl"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 gigaliters"),
                       ki18ncp("amount in units (integer)", "%1 gigaliter", "%1 gigaliters")));

    d->addUnit(UnitPrivate::makeUnit(VolumeCategory,
                       Megaliter,
                       1000,
                       i18nc("volume unit symbol", "Ml"),
                       i18nc("unit description in lists", "megaliters"),
                       i18nc("unit synonyms for matching user input", "megaliter;megaliters;Ml"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 megaliters"),
                       ki18ncp("amount in units (integer)", "%1 megaliter", "%1 megaliters")));

    d->addDefaultUnit(UnitPrivate::makeUnit(VolumeCategory,
                              Kiloliter,
                              1,
                              i18nc("volume unit symbol", "kl"),
                              i18nc("unit description in lists", "kiloliters"),
                              i18nc("unit synonyms for matching user input", "kiloliter;kiloliters;kl"),
                              symbolString,
                              ki18nc("amount in units (real)", "%1 kiloliters"),
                              ki18ncp("amount in units (integer)", "%1 kiloliter", "%1 kiloliters")));

    d->addUnit(UnitPrivate::makeUnit(VolumeCategory,
                       Hectoliter,
                       0.1,
                       i18nc("volume unit symbol", "hl"),
                       i18nc("unit description in lists", "hectoliters"),
                       i18nc("unit synonyms for matching user input", "hectoliter;hectoliters;hl"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 hectoliters"),
                       ki18ncp("amount in units (integer)", "%1 hectoliter", "%1 hectoliters")));

    d->addUnit(UnitPrivate::makeUnit(VolumeCategory,
                       Decaliter,
                       0.01,
                       i18nc("volume unit symbol", "dal"),
                       i18nc("unit description in lists", "decaliters"),
                       i18nc("unit synonyms for matching user input", "decaliter;decaliters;dal"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 decaliters"),
                       ki18ncp("amount in units (integer)", "%1 decaliter", "%1 decaliters")));

    d->addCommonUnit(UnitPrivate::makeUnit(VolumeCategory,
                             Liter,
                             0.001,
                             i18nc("volume unit symbol", "l"),
                             i18nc("unit description in lists", "liters"),
                             i18nc("unit synonyms for matching user input", "liter;liters;l"),
                             symbolString,
                             ki18nc("amount in units (real)", "%1 liters"),
                             ki18ncp("amount in units (integer)", "%1 liter", "%1 liters")));

    d->addUnit(UnitPrivate::makeUnit(VolumeCategory,
                       Deciliter,
                       0.0001,
                       i18nc("volume unit symbol", "dl"),
                       i18nc("unit description in lists", "deciliters"),
                       i18nc("unit synonyms for matching user input", "deciliter;deciliters;dl"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 deciliters"),
                       ki18ncp("amount in units (integer)", "%1 deciliter", "%1 deciliters")));

    d->addUnit(UnitPrivate::makeUnit(VolumeCategory,
                       Centiliter,
                       1e-05,
                       i18nc("volume unit symbol", "cl"),
                       i18nc("unit description in lists", "centiliters"),
                       i18nc("unit synonyms for matching user input", "centiliter;centiliters;cl"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 centiliters"),
                       ki18ncp("amount in units (integer)", "%1 centiliter", "%1 centiliters")));

    d->addCommonUnit(UnitPrivate::makeUnit(VolumeCategory,
                             Milliliter,
                             1e-06,
                             i18nc("volume unit symbol", "ml"),
                             i18nc("unit description in lists", "milliliters"),
                             i18nc("unit synonyms for matching user input", "milliliter;milliliters;ml"),
                             symbolString,
                             ki18nc("amount in units (real)", "%1 milliliters"),
                             ki18ncp("amount in units (integer)", "%1 milliliter", "%1 milliliters")));

    d->addUnit(UnitPrivate::makeUnit(VolumeCategory,
                       Microliter,
                       1e-09,
                       i18nc("volume unit symbol", "µl"),
                       i18nc("unit description in lists", "microliters"),
                       i18nc("unit synonyms for matching user input", "microliter;microliters;µl;ul"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 microliters"),
                       ki18ncp("amount in units (integer)", "%1 microliter", "%1 microliters")));

    d->addUnit(UnitPrivate::makeUnit(VolumeCategory,
                       Nanoliter,
                       1e-12,
                       i18nc("volume unit symbol", "nl"),
                       i18nc("unit description in lists", "nanoliters"),
                       i18nc("unit synonyms for matching user input", "nanoliter;nanoliters;nl"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 nanoliters"),
                       ki18ncp("amount in units (integer)", "%1 nanoliter", "%1 nanoliters")));

    d->addUnit(UnitPrivate::makeUnit(VolumeCategory,
                       Picoliter,
                       1e-15,
                       i18nc("volume unit symbol", "pl"),
                       i18nc("unit description in lists", "picoliters"),
                       i18nc("unit synonyms for matching user input", "picoliter;picoliters;pl"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 picoliters"),
                       ki18ncp("amount in units (integer)", "%1 picoliter", "%1 picoliters")));

    d->addUnit(UnitPrivate::makeUnit(VolumeCategory,
                       Femtoliter,
                       1e-18,
                       i18nc("volume unit symbol", "fl"),
                       i18nc("unit description in lists", "femtoliters"),
                       i18nc("unit synonyms for matching user input", "femtoliter;femtoliters;fl"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 femtoliters"),
                       ki18ncp("amount in units (integer)", "%1 femtoliter", "%1 femtoliters")));

    d->addUnit(UnitPrivate::makeUnit(VolumeCategory,
                       Attoliter,
                       1e-21,
                       i18nc("volume unit symbol", "al"),
                       i18nc("unit description in lists", "attoliters"),
                       i18nc("unit synonyms for matching user input", "attoliter;attoliters;al"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 attoliters"),
                       ki18ncp("amount in units (integer)", "%1 attoliter", "%1 attoliters")));

    d->addUnit(UnitPrivate::makeUnit(VolumeCategory,
                       Zeptoliter,
                       1e-24,
                       i18nc("volume unit symbol", "zl"),
                       i18nc("unit description in lists", "zeptoliters"),
                       i18nc("unit synonyms for matching user input", "zeptoliter;zeptoliters;zl"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 zeptoliters"),
                       ki18ncp("amount in units (integer)", "%1 zeptoliter", "%1 zeptoliters")));

    d->addUnit(UnitPrivate::makeUnit(VolumeCategory,
                       Yoctoliter,
                       1e-27,
                       i18nc("volume unit symbol", "yl"),
                       i18nc("unit description in lists", "yoctoliters"),
                       i18nc("unit synonyms for matching user input", "yoctoliter;yoctoliters;yl"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 yoctoliters"),
                       ki18ncp("amount in units (integer)", "%1 yoctoliter", "%1 yoctoliters")));

    d->addUnit(UnitPrivate::makeUnit(VolumeCategory,
                       CubicFoot,
                       0.0283168,
                       i18nc("volume unit symbol", "ft³"),
                       i18nc("unit description in lists", "cubic feet"),
                       i18nc("unit synonyms for matching user input", "cubic foot;cubic feet;ft³;cubic ft;cu foot;cu ft;cu feet;feet³"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 cubic feet"),
                       ki18ncp("amount in units (integer)", "%1 cubic foot", "%1 cubic feet")));

    d->addUnit(UnitPrivate::makeUnit(VolumeCategory,
                       CubicInch,
                       1.63871e-05,
                       i18nc("volume unit symbol", "in³"),
                       i18nc("unit description in lists", "cubic inches"),
                       i18nc("unit synonyms for matching user input", "cubic inch;cubic inches;in³;cubic inch;cubic in;cu inches;cu inch;cu in;inch³"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 cubic inches"),
                       ki18ncp("amount in units (integer)", "%1 cubic inch", "%1 cubic inches")));

    d->addUnit(UnitPrivate::makeUnit(VolumeCategory,
                       CubicMile,
                       4.16818e+09,
                       i18nc("volume unit symbol", "mi³"),
                       i18nc("unit description in lists", "cubic miles"),
                       i18nc("unit synonyms for matching user input", "cubic mile;cubic miles;mi³;cubic mile;cubic mi;cu miles;cu mile;cu mi;mile³"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 cubic miles"),
                       ki18ncp("amount in units (integer)", "%1 cubic mile", "%1 cubic miles")));

    d->addCommonUnit(UnitPrivate::makeUnit(VolumeCategory,
                             FluidOunce,
                             2.95735e-05,
                             i18nc("volume unit symbol", "fl.oz."),
                             i18nc("unit description in lists", "fluid ounces"),
                             i18nc("unit synonyms for matching user input", "fluid ounce;fluid ounces;fl.oz.;oz.fl.;oz. fl.;fl. oz.;fl oz;fluid ounce"),
                             symbolString,
                             ki18nc("amount in units (real)", "%1 fluid ounces"),
                             ki18ncp("amount in units (integer)", "%1 fluid ounce", "%1 fluid ounces")));

    d->addCommonUnit(UnitPrivate::makeUnit(VolumeCategory,
                             Cup,
                             0.000236588,
                             i18nc("volume unit symbol", "cp"),
                             i18nc("unit description in lists", "cups"),
                             i18nc("unit synonyms for matching user input", "cup;cups;cp"),
                             symbolString,
                             ki18nc("amount in units (real)", "%1 cups"),
                             ki18ncp("amount in units (integer)", "%1 cup", "%1 cups")));

    d->addCommonUnit(UnitPrivate::makeUnit(VolumeCategory,
                                           Teaspoon,
                                           0.000005, // FDA standard 5ml
                                           i18nc("volume unit symbol", "tsp"),
                                           i18nc("unit description in lists", "teaspoon"),
                                           i18nc("unit synonyms for matching user input", "teaspoon;teaspoons;tsp;tsps"),
                                           symbolString,
                                           ki18nc("amount in units (real)", "%1 teaspoons"),
                                           ki18ncp("amount in units (integer)", "%1 teaspoon", "%1 teaspoons")));

    d->addCommonUnit(UnitPrivate::makeUnit(VolumeCategory,
                                           Tablespoon,
                                           0.000015, // FDA standard 15ml
                                           i18nc("volume unit symbol", "tbsp"),
                                           i18nc("unit description in lists", "tablespoon"),
                                           i18nc("unit synonyms for matching user input", "tablespoon;tablespoons;tbsp;tbsps"),
                                           symbolString,
                                           ki18nc("amount in units (real)", "%1 tablespoons"),
                                           ki18ncp("amount in units (integer)", "%1 tablespoon", "%1 tablespoons")));

    d->addCommonUnit(UnitPrivate::makeUnit(VolumeCategory,
                             GallonUS,
                             0.00378541,
                             i18nc("volume unit symbol", "gal"),
                             i18nc("unit description in lists", "gallons (U.S. liquid)"),
                             i18nc("unit synonyms for matching user input", "gallon (U.S. liquid);gallons (U.S. liquid);gal;gallon;gallons"),
                             symbolString,
                             ki18nc("amount in units (real)", "%1 gallons (U.S. liquid)"),
                             ki18ncp("amount in units (integer)", "%1 gallon (U.S. liquid)", "%1 gallons (U.S. liquid)")));

    d->addUnit(UnitPrivate::makeUnit(VolumeCategory,
                       GallonImperial,
                       0.00454609,
                       i18nc("volume unit symbol", "imp gal"),
                       i18nc("unit description in lists", "gallons (imperial)"),
                       i18nc("unit synonyms for matching user input",
                             "gallon (imperial);gallons (imperial);imp gal;imp gallon;imp gallons;imperial gal;imperial gallon;imperial gallons"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 gallons (imperial)"),
                       ki18ncp("amount in units (integer)", "%1 gallon (imperial)", "%1 gallons (imperial)")));

    d->addCommonUnit(UnitPrivate::makeUnit(VolumeCategory,
                             PintImperial,
                             0.000568261,
                             i18nc("volume unit symbol", "pt"),
                             i18nc("unit description in lists", "pints (imperial)"),
                             i18nc("unit synonyms for matching user input", "pint (imperial);pints (imperial);pt;pint;pints;p"),
                             symbolString,
                             ki18nc("amount in units (real)", "%1 pints (imperial)"),
                             ki18ncp("amount in units (integer)", "%1 pint (imperial)", "%1 pints (imperial)")));

    d->addUnit(UnitPrivate::makeUnit(VolumeCategory,
                       PintUS,
                       0.000473176,
                       i18nc("volume unit symbol", "US pt"),
                       i18nc("unit description in lists", "pints (U.S. liquid)"),
                       i18nc("unit synonyms for matching user input", "pint (U.S. liquid);pints (U.S. liquid);US pt;US pint;US pints;US p"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 pints (U.S. liquid)"),
                       ki18ncp("amount in units (integer)", "%1 pint (U.S. liquid)", "%1 pints (U.S. liquid)")));

    d->addUnit(UnitPrivate::makeUnit(VolumeCategory,
                       OilBarrel,
                       0.158987294,
                       i18nc("volume unit symbol", "bbl"),
                       i18nc("unit description in lists", "oil barrels"),
                       i18nc("unit synonyms for matching user input", "oil barrels;oil barrel;bbl"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 oil barrels"),
                       ki18ncp("amount in units (integer)", "%1 oil barrel", "%1 oil barrels")));

    return c;
}

} // KUnitConversion namespace
