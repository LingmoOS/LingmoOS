# SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
# SPDX-License-Identifier: LGPL-2.0-or-later

# data sources
TZDATA_VERSION = '2020d'
TZDATA_URL = f"https://github.com/evansiroky/timezone-boundary-builder/releases/download/{TZDATA_VERSION}/timezones.shapefile.zip"
ISO3166_1_VERSION = '2021-08-16'
ISO3166_1_URL = f"https://volkerkrause.eu/~vkrause/iso3166-boundaries/iso3166-1-boundaries.shp-{ISO3166_1_VERSION}.zip"
ISO3166_2_VERSION = ISO3166_1_VERSION
ISO3166_2_URL = f"https://volkerkrause.eu/~vkrause/iso3166-boundaries/iso3166-2-boundaries.shp-{ISO3166_2_VERSION}.zip"

# ISO 3166-1 code mappings
# This is for codes that should be unconditionally replaced, which is mainly useful
# for certain historical or political corner-cases or "non-countries" like Antarctica
ISO3166_1_MAP = {
    'AQ': None,
    'AX': 'FI',
    'SJ': 'NO',
    'UM': 'US'
}

# ISO 3166-1 overlap disambiguation
# This is mainly relevant for oversees territories that both have their own code
# as well that of the country they belong to
ISO3166_1_DISAMBIGUATION_MAP = {
    ( 'AS', 'US' ),
    ( 'AW', 'NL' ),
    ( 'BL', 'FR' ),
    ( 'BQ', 'NL' ),
    ( 'BT', 'CN' ),
    ( 'CC', 'AU' ),
    ( 'CW', 'NL' ),
    ( 'CX', 'AU' ),
    ( 'GF', 'FR' ),
    ( 'GP', 'FR' ),
    ( 'GU', 'US' ),
    ( 'HK', 'CN' ),
    ( 'HT', 'US' ),
    ( 'MF', 'FR' ),
    ( 'MO', 'CN' ),
    ( 'MP', 'US' ),
    ( 'MQ', 'FR' ),
    ( 'NC', 'FR' ),
    ( 'NF', 'AU' ),
    ( 'PF', 'FR' ),
    ( 'PM', 'FR' ),
    ( 'PR', 'US' ),
    ( 'RE', 'FR' ),
    ( 'SX', 'NL' ),
    ( 'TF', 'FR' ),
    ( 'VI', 'US' ),
    ( 'WF', 'FR' ),
    ( 'YT', 'FR' )
}

# ISO 3166-2 filter configuration
# most of those are sub-subdivisions
ISO3166_2_FILTER = [
    { 'country': 'BD', 'admin_level': 5 },
    { 'country': 'BE', 'admin_level': 6 },
    { 'country': 'BF', 'admin_level': 5 },
    { 'country': 'CZ', 'admin_level': 7 },
    { 'country': 'ES', 'admin_level': 6 },
    { 'country': 'FR', 'admin_level': 5 },
    { 'country': 'FR', 'admin_level': 6 },
    { 'country': 'GB', 'admin_level': 6 },
    { 'country': 'GB', 'admin_level': 8 },
    { 'country': 'GN', 'admin_level': 6 },
    { 'country': 'GQ', 'admin_level': 4 },
    { 'country': 'GW', 'admin_level': 4 },
    { 'country': 'IE', 'admin_level': 6 },
    { 'country': 'IE', 'admin_level': 7 },
    { 'country': 'IT', 'admin_level': 6 },
    { 'country': 'LK', 'admin_level': 5 },
    { 'country': 'LT', 'admin_level': 5 },
    { 'country': 'MA', 'admin_level': 5 },
    { 'country': 'MW', 'admin_level': 4 },
    { 'country': 'NP', 'admin_level': 4 },
    { 'country': 'NP', 'admin_level': 5 },
    { 'country': 'PH', 'admin_level': 4 },
    { 'country': 'UG', 'admin_level': 4 }
]


# Timezone mapping
# use this to replace timezone ids with an equivalent
# this is mainly useful for filtering out timezones of limited practical or merely historical relevance,
# trading historical accuracy for practical usability for current and future date/time values
TZID_MAP = {
    'America/Argentina/Catamarca': 'America/Argentina/Buenos_Aires',
    'America/Argentina/Cordoba': 'America/Argentina/Buenos_Aires',
    'America/Argentina/Jujuy': 'America/Argentina/Buenos_Aires',
    'America/Argentina/La_Rioja': 'America/Argentina/Buenos_Aires',
    'America/Argentina/Mendoza': 'America/Argentina/Buenos_Aires',
    'America/Argentina/Rio_Gallegos': 'America/Argentina/Buenos_Aires',
    'America/Argentina/Salta': 'America/Argentina/Buenos_Aires',
    'America/Argentina/San_Juan': 'America/Argentina/Buenos_Aires',
    'America/Argentina/San_Luis': 'America/Argentina/Buenos_Aires',
    'America/Argentina/Tucuman': 'America/Argentina/Buenos_Aires',
    'America/Argentina/Ushuaia': 'America/Argentina/Buenos_Aires',
    'America/Nipigon': 'America/Toronto',
    'Arctic/Longyearbyen': 'Europe/Oslo',
    'Asia/Famagusta': 'Asia/Nicosia',
    'Asia/Kuching': 'Asia/Kuala_Lumpur',
    'Europe/Busingen': 'Europe/Berlin',
    'Europe/Mariehamn': 'Europe/Helsinki'
}

#
# parameters for the spatial index
#
featureAreaRatioThreshold = 0.02 # 1% at zDepth 11 is ~150m
zDepth = 11 # minimum tile size is 1/(2^zdepth), amount of bits needed to store z index is 2*zDepth

# z-order curve coverage parameters
xStart = -180
xRange = 360
# cut out artic regions (starting at 60°S and 80°N), that saves about 23% z-order curve coverage which we
# can better use to increase precision in more relevant areas
yStart = -60
yRange = 140


# constants
LOG_CATEGORY = 'KI18n Data Generator'
