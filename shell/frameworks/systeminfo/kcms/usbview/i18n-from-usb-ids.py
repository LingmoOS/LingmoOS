#!/usr/bin/env python3

# SPDX-FileCopyrightText: 2021 Pino Toscano <pino@debian.org>
# SPDX-License-Identifier: GPL-2.0-or-later

import os
import re
import sys
from collections import namedtuple


UsbIds = namedtuple('UsbIds',
                    ['classes', 'subclasses', 'protocols', 'date'])


def parse(fn):
    classes = []
    subclasses = []
    protocols = []
    date = None

    class_re = re.compile("^C ([0-9a-fA-F]{2})\\s+(.+)$")
    subclass_re = re.compile("^\t([0-9a-fA-F]{2})\\s+(.+)$")
    protocol_re = re.compile("^\t\t([0-9a-fA-F]{2})\\s+(.+)$")

    with open(fn, 'r', encoding='iso-8859-1') as f:
        in_class = False
        in_subclass = False
        for line in f:
            line = line.rstrip('\n')
            line_len = len(line)

            if line_len > 0 and line[0] == '#':
                if line.startswith('# Date:'):
                    date = line.split(':', maxsplit=1)[1].strip()
                continue

            if line_len == 0 or (line[0].isalnum() and line[0] != 'C'):
                in_class = False
                in_subclass = False
                continue

            m = class_re.fullmatch(line)
            if m:
                classes.append(m[2].rstrip())
                in_class = True
                continue
            if in_class:
                m = subclass_re.fullmatch(line)
                if m:
                    in_subclass = True
                    subclasses.append(m[2].rstrip())
                    continue
            if in_class and in_subclass:
                m = protocol_re.fullmatch(line)
                if m:
                    protocols.append(m[2].rstrip())
                    continue

            in_class = False
            in_subclass = False

    parsed = UsbIds(classes, subclasses, protocols, date)

    return parsed


def output(strings, context):
    for s in strings:
        print('i18nc("{}", "{}");'.format(context, s))


if __name__ == '__main__':
    if len(sys.argv) != 2:
        print('Usage: {} /path/to/usb.ids'.format(sys.argv[0]),
              file=sys.stderr)
        sys.exit(1)

    filename = sys.argv[1]
    parsed = parse(filename)
    classes = sorted(list(set(parsed.classes)))
    subclasses = sorted(list(set(parsed.subclasses)))
    if parsed.date:
        date_str = parsed.date
    else:
        date_str = 'n/a'
    print('// Generated from {} with date \'{}\''.format(
              os.path.basename(filename), date_str))
    output(classes, 'USB device class')
    output(subclasses, 'USB device subclass')
