#!@PYTHON_EXECUTABLE@
#
# SPDX-FileCopyrightText: 2016 Varun Joshi <varunj.1011@gmail.com>
#
# SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL

import sys
import json

extractor_data = json.loads(sys.stdin.read())

def extract():
     path = extractor_data.get('path')
     mimetype = extractor_data.get('mimetype')

     doc = open(path)

     return_value = {}
     return_value['properties'] = {}
     return_value['properties']['text'] = doc.read()
     return_value['status'] = 'OK'
     return_value['error'] = ''

     print(json.dumps(return_value))

if __name__ == "__main__":
    extract()
