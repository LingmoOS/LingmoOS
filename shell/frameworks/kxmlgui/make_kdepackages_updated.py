#!/usr/bin/python
# -*- coding: utf-8 -*-
#
# Script to automatically update the "kdepackages.h" file
# FIXME - This is a slow script. Rewrite me using a smart logic. Thanks!
#
import string
import urllib
import re

def unescape(text):
    text = text.replace("&nbsp;"," ")
    text = text.replace("&#8209;","-")
    text = text.replace("&amp;","&")
    return text


print "Fetching products and components from bugs.kde.org..."

pkg = open("src/kdepackages.h","w")
pkg.write("// DO NOT EDIT - EDIT products in bugs.kde.org and run ./make_kdepackages_updated.py in kxmlgui to update\n")
pkg.write("const char * const packages[] = {\n")

data = urllib.urlopen('https://bugs.kde.org/describecomponents.cgi').read()

for line in string.split(data,'\n'):
  print "====parsing:"
  #print line
  match = re.search('(describecomponents.cgi\?product=.*)">(.*)</a>', line)
  if match:
    product = match.group(2)
    link = match.group(1)

    link = 'https://bugs.kde.org/' + link
    data2 = urllib.urlopen(link).read()

    productname = unescape(product)
    print productname
    pkg.write("    \"" + productname + "\",\n")
    data2 = string.split(data2,'\n')
    iter = 0
    end = len(data2)
    print "link: " + link
    while( iter < end-1 ):
      iter = iter+1
      line = data2[iter]
      match = re.search('amp;resolution=---">(.*)</a>', line)
      if match:
        product = match.group(1)
        product = unescape(product)
        print "found component: " + product
        if product!="general":
          pkg.write("    \"" + productname + "/" + product + "\",\n")
          print productname + "/" + product

pkg.write("0 };\n")
pkg.close()
