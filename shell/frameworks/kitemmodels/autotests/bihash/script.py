#! /usr/bin/env python
# -*- coding: utf-8 -*-

import os

_containers = (
               "KBiHash",
               "QHash",
               )
_types = (
          "int",
          "QString",
          )

for _container in _containers:
  for _type in _types:
    os.system("xmlpatterns aggregrator.xls -param container=" +
              _container.lower() + " -param cml_container=" + _container + " -param type=" + _type.lower() +
              " > output/" + _container.lower() + "_" + _type.lower() + "_" + _type.lower() + ".xml")
