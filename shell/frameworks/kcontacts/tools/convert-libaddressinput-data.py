#!/usr/bin/env python3
# SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
# SPDX-License-Identifier: LGPL-2.0-or-later

import json
import os
import requests

# converts libaddressinput data to our addressbookformatrc format
# Note: This is not meant to just blindly overwrite our data, we have a number
# of additions and fixes that libaddressinput doesn't have. It is merely meant
# as an easier way to diff our data to that of libaddressinput.

# get a list of all 3166-1 alpha 2 ISO codes
isoCodesFile = open('/usr/share/iso-codes/json/iso_3166-1.json')
isoCodesJson = json.loads(isoCodesFile.read())
isoCodes = []
for isoCode in isoCodesJson['3166-1']:
    isoCodes.append(isoCode['alpha_2'])
isoCodes.sort()

outFile = open('addressformatrc', 'w')

# for each ISO code, see if libaddressinput has data
for isoCode in isoCodes:
    req = requests.get(f"https://www.gstatic.com/chrome/autofill/libaddressinput/chromium-i18n/ssl-address/data/{isoCode}")
    if req.status_code == 404:
        print (f"{isoCode} has no format information")
        continue
    data = json.loads(req.text)
    if not 'fmt' in data:
        print(f"{isoCode} has no format")
        continue

    outFile.write(f"[{isoCode}]\n")
    outFile.write(f"AddressFormat={data['fmt']}\n")
    if 'lfmt' in data:
        outFile.write(f"LatinAddressFormat={data['lfmt']}\n")
    if 'upper' in data:
        outFile.write(f"Upper={data['upper']}\n")
    if 'require' in data:
        outFile.write(f"Required={data['require']}\n")
    if 'zip' in data:
        outFile.write(f"PostalCodeFormat={data['zip']}\n")
    outFile.write('\n')
