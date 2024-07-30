# Localized Address Formats

KContact's address formatting largely follows the approach taken by
[libaddressinput](https://github.com/google/libaddressinput), with adding
a few additional fields and formatting styles.

Address formatting rules are specified in the `addressformatrc` file in the `src`
directory, grouped by country. The formatting rules format is compatible
with [that of libaddressinput](https://github.com/google/libaddressinput/wiki/AddressValidationMetadata).

## Address Format Information

* AddressFormat: the default address format using native script, using the placeholders
  specified below (mandatory).
* LatinAddressFormat: for countries using a non-latin script this can provide
  an alternative format for addresses transliterated to a latin script (optional).
* BusinessAddressFormat: used for formatting postal addresses of an organization.
  This only needs to be specified if the format differs from the generic case.
* LatinBusinessAddressFormat: see LatinAddressFormat.

All of the above values may be specified in multiple variants for multi-lingual countries.
This is typically only needed when the format rule contains literal strings, e.g.
for specifying a post office box.

* Required: The address fields that are minimally required to form a valid address
  in this country.
* Upper: The address fields to be formatted in upper case regardless of the input
  for postal addresses.

The above values are specified as a string containing the single letter address field
identifiers listed below. E.g. "AZC" would indicate that a street address, postal code
and locality (city) are required.

* PostalCodeFormat: A regular expression matching all valid postal codes in
  this country.

## Address Field Identifiers

The following identifiers are used in the formatting rules to refer to specific
parts of an address:

* N - Name of a person/Recipient of a letter
* O - Organization
* A – Street address
* C – Locality (city)
* S – Region (country subdivision/administrative area/state/etc)
* Z – Postal code/zip code
* R - Country

Identifiers used by libaddressinput but not by KContacts:

* D – Dependent locality (may be an inner-city district or a suburb)
* X – Sorting code

## Address Formatting Rules

Address formatting rules consist of a sequence of elements of the following
three types:
* Fields - Insert the value of the corresponding address field, specified as a `%` sign
  followed by a field identifier (see above).
* Separators - Represented as `%n`. Depending on the formatting style those are replaced
  e.g. a line break or a script-dependent separator string.
* Literals - Any other string. Inserted as-is in the output, given it is neither preceded nor
  followed by an empty field.
