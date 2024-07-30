# -*- coding: utf-8 -*-
#
# SPDX-FileCopyrightText: 2014 Aurélien Gâteau <agateau@kde.org>
#
# SPDX-License-Identifier: BSD-2-Clause

from contextlib import contextmanager


class Block(object):
    INDENT_SIZE = 4

    def __init__(self, out, depth=0):
        self.out = out
        self.depth = depth

    def writeln(self, text):
        self.out.write(self.depth * Block.INDENT_SIZE * " " + text + "\n")

    def write_attrs(self, **attrs):
        for key, value in attrs.items():
            self.writeln('"{}" = "{}";'.format(key, value))

    def write_list_attrs(self, name, **attrs):
        with self.square_block(name) as b:
            for key, value in attrs.items():
                b.writeln('"{}" = "{}"'.format(key, value))

    def write_nodes(self, nodes):
        for node in sorted(nodes):
            self.writeln('"{}";'.format(node))

    @contextmanager
    def block(self, opener, closer, **attrs):
        self.writeln(opener)
        block = Block(self.out, depth=self.depth + 1)
        block.write_attrs(**attrs)
        yield block
        self.writeln(closer)

    def square_block(self, prefix, **attrs):
        return self.block(prefix + " [", "]", **attrs)

    def curly_block(self, prefix, **attrs):
        return self.block(prefix + " {", "}", **attrs)

    def cluster_block(self, title, **attrs):
        return self.curly_block("subgraph " + quote("cluster_" + title), label=title, **attrs)


def quote(txt):
    return '"{}"'.format(txt)
