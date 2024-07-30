Locale Data QML API   {#locale_data_qml_api}
==================

-   [Data Types](#data_types)
-   [Country Lookup](#country_lookup)
-   [Country Subdivision Lookup](#subdiv_lookup)
-   [Timezone Lookup](#timezone_lookup)
-   [Further References](#refs)

<a name="data_types">

## Data Types

The basic types providing locale data in the C++ API are also
available in QML as `Q_GADGET`s and offer the same properties
in QML:

\li KCountry
\li KCountrySubdivision

<a name="country_lookup">

## Country Lookup

The static country lookup methods found in `KCountry` in the C++ API
are available from the `Country` singleton in QML, as part of the `org.kde.i18n.localeData` module.

### Listing all

For listing all known countries there is the `Country.allCountries` property,
providing a list of `KCountry` objects.

~~~
import org.kde.i18n.localeData 1.0

... {
    ComboBox {
        model: Country.allCountries
        displayText: currentValue.emojiFlag + " " + currentValue.name
    }
}
~~~

### Lookup by code, name or location

The following methods allow to obtain a `KCountry` object:

\li `Country.fromAlpha2`
\li `Country.fromAlpha3`
\li `Country.fromName`
\li `Country.fromLocation`

See the corresponding C++ API with the same name in `KCountry` for further details.


<a name="subdiv_lookup">

## Country Subdivision Lookup

The static country subdivision lookup methods found in `KCountrySubdivision` in the C++ API
are available from the `CountrySubdivision` singleton in QML, as part of the `org.kde.i18n.localeData` module.

The following methods allow to obtain a `KCountrySubdivision` object:

\li `CountrySubdivision.fromCode`
\li `CountrySubdivision.fromLocation`

See the corresponding C++ API with the same name in `KCountrySubdivision` for further details.


<a name="timezone_lookup">

## Timezone Lookup

The static country timezone lookup methods found in `KTimeZone` in the C++ API
are available from the `TimeZone` singleton in QML, as part of the `org.kde.i18n.localeData` module.

The following methods are provided:

\li `TimeZone.country`
\li `TimeZone.fromLocation`

See the corresponding C++ API with the same name in `KTimeZone` for further details.


<a name="refs">

## Further References

There's a standalone QML example showing most of the above mentioned functionality in `tests/demo.qml`
provided as part of the KI18n source code.

