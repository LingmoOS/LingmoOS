#!/usr/bin/env python3
#
# SPDX-FileCopyrightText: 2024 Oliver Beard <olib141@outlook.com>
#
# SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
#

# Script to update the list of supporters in Supporters.qml using exported Donorbox data
# Sorts by date, removes anonymous supporters and performs de-duplication (on email)
# Usage: ./UpdateSupporters.py ~/Downloads/rawList.csv

# N.B. Supporters as written will not match the CSV in order, because some may have donated earlier (their earliest donation is used to order by time, not their most recent).

import csv
from datetime import datetime
import os
import sys

def import_csv(input_file):
    columns = ["Name", "Donor Email", "Make Donation Anonymous", "Donated At"]
    data = []

    input_file = os.path.expanduser(input_file)

    with open(input_file, "r") as infile:
        reader = csv.DictReader(infile)

        for row in reader:
            # At some point, the locale was changed to German, so...
            # Rename German keys to their English versions
            if "E Mail" in row:
                row["Donor Email"] = row.pop("E Mail")

            if "Anonym Spenden" in row:
                row["Make Donation Anonymous"] = row.pop("Anonym Spenden")

            if "Gespendet Am" in row:
                row["Donated At"] = row.pop("Gespendet Am")

            donated_at = datetime.strptime(row["Donated At"], "%m/%d/%Y %H:%M:%S")
            row["Donated At"] = donated_at
            data.append({key: row[key] for key in columns})

    return data

def sort_date(data):
    return sorted(data, key=lambda x: x["Donated At"])

def remove_anon(data):
    out_data = []

    for row in data:
        for row in data:
            if row["Make Donation Anonymous"] == "no":
                out_data.append(row)

    return out_data

def dedup_email(data):
    unique_emails = set()
    out_data = []

    for row in data:
        email = row["Donor Email"]
        if email not in unique_emails:
            unique_emails.add(email)
            out_data.append(row)

    return out_data

def js_arr(data):
    return '["' + '", "'.join(row['Name'] for row in data) + '"]'

# Take input
if len(sys.argv) > 1:
    input_file = sys.argv[1]
else:
    input_file = input("Path of rawList CSV: ")

# Process data
raw_data = sort_date(import_csv(input_file))
donations = len(raw_data)

supporters_data = dedup_email(raw_data)
supporters = len(supporters_data)

data = dedup_email(remove_anon(raw_data))
nonanon_supporters = len(data)

# Output stats
print("Donations:", donations)
print("Supporters:", supporters, "({0!s} non-anonymous, {1!s} anonymous)".format(nonanon_supporters, supporters - nonanon_supporters))

# Update Supporters.qml
with open("Supporters.qml", "r+") as file:
    lines = file.readlines()

    supporters_line = "    readonly property var supporters: "
    for i, line in enumerate(lines):
        if supporters_line in line:
            lines[i] = supporters_line + js_arr(data) + "\n"
            break

    file.seek(0)
    file.writelines(lines)
    file.truncate()
    file.close()

print("Updated Supporters.qml")
