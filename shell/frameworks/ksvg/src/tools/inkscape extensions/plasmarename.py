#!/usr/bin/env python3

'''
SPDX-FileCopyrightText: 2009 Marco Martin <notmart@gmail.com>

SPDX-License-Identifier: GPL-2.0-or-later
'''

import inkex

import pathmodifier


class PlasmaNamesEffect(pathmodifier.PathModifier):
    """
    Renames 9 selected elements as a plasma theme frame
    """

    def add_arguments(self, pars):
        pars.add_argument('--prefix', type=str, default='World', help='Prefix of the selected elements')

    def effect(self):
        # Get script's "--prefix" option value.
        prefix = self.options.prefix

        # 9 elements: is a frame. 4 elements: is a border hint
        positions = []
        if len(self.svg.selection) == 9:
            positions = ['topleft', 'left', 'bottomleft', 'top', 'center', 'bottom', 'topright', 'right', 'bottomright']
        elif len(self.svg.selection) == 4:
            positions = ['hint-left-margin', 'hint-top-margin', 'hint-bottom-margin', 'hint-right-margin']
        else:
            raise inkex.AbortExtension('This extension requires 4 or 9 selected elements.')

        # some heuristics to normalize the values, find the least coords and size
        minX = 9999
        minY = 9999
        minWidth = 9999
        minHeight = 9999
        for node in self.svg.selection.values():
            bbox = node.bounding_box()
            minX = min(minX, bbox.x.minimum)
            minY = min(minY, bbox.y.minimum)
            minWidth = min(minWidth, bbox.width)
            minHeight = min(minHeight, bbox.height)

        nodedictionary = {}
        for node in self.svg.selection.values():
            bbox = node.bounding_box()
            x = bbox.x.minimum/minWidth - minX
            y = bbox.y.minimum/minHeight - minY
            nodedictionary[x*1000 + y] = node

        i = 0
        for _, node in sorted(nodedictionary.items()):
            if prefix:
                name = '%s-%s' % (prefix, positions[i])
            else:
                name = '%s' % (positions[i])
            node.set('id', name)
            i += 1


if __name__ == '__main__':
    PlasmaNamesEffect().run()
