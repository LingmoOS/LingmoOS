# -*- coding: utf-8 -*-
#
# SPDX-FileCopyrightText: 2014 Aurélien Gâteau <agateau@kde.org>
#
# SPDX-License-Identifier: BSD-2-Clause

"""
A set of classes and functions to make it easier to work with Graphviz graphs
"""

import gv


class Node(object):
    def __init__(self, node_handle):
        self.handle = node_handle

    @property
    def name(self):
        return gv.nameof(self.handle)

    @property
    def label(self):
        return gv.getv(self.handle, "label")

    @property
    def shape(self):
        return gv.getv(self.handle, "shape")


class Edge(object):
    def __init__(self, edge_handle):
        self.handle = edge_handle

    @property
    def head(self):
        handle = gv.headof(self.handle)
        if handle is None:
            return None
        else:
            return Node(handle)

    @property
    def tail(self):
        handle = gv.tailof(self.handle)
        if handle is None:
            return None
        else:
            return Node(handle)


def get_node_list(graph_h):
    """Generator to iterate over all nodes of a graph"""
    handle = gv.firstnode(graph_h)
    while gv.ok(handle):
        yield handle
        handle = gv.nextnode(graph_h, handle)


def get_edge_list(graph_h):
    """Generator to iterate over all edges of a graph"""
    handle = gv.firstedge(graph_h)
    while gv.ok(handle):
        yield handle
        handle = gv.nextedge(graph_h, handle)
