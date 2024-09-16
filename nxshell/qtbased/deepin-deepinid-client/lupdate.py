#!/usr/bin/python

import glob
import subprocess

translatefile = glob.glob("translations/*.ts")


def lupdate(file: str):
    subprocess.run(
        ["lupdate", "-recursive", "./src/", "-ts", file])


for file in translatefile:
    lupdate(file)

# Finally run tx push -s --branch m23
