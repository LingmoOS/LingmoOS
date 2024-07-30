#! /usr/bin/env python
# -*- coding: utf-8 -*-

import os

files = {"kbihash_int_int" : "KBiHash(int, int)",
         "qhash_int_int" : "QHash(int, int)",
         "qhash_string_string" : "QHash(QString, QString)",
         "kbihash_string_string" : "KBiHash(QString, QString)",
        }

for _file, _type in files.items():
    for i in range(10):
        xml_filename = _file + str(i + 1) + ".xml"
        print _type + " : ", i
        os.system("./" + _file + " -callgrind -xml > " + xml_filename)
        os.system("sed -i 's/>4.7.0</>" + _type + str(i) + "</' " + xml_filename)

