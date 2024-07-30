# -*- coding: utf-8 -*-
#
# SPDX-FileCopyrightText: 2014 Aurélien Gâteau <agateau@kde.org>
#
# SPDX-License-Identifier: BSD-2-Clause

import logging
import fnmatch
import os
import re

import gv
import yaml

from kapidox.depdiagram import gvutils
from kapidox.depdiagram.framework import Framework


TARGET_SHAPES = [
    "polygon", # lib
    "house",   # executable
    "octagon", # module (aka plugin)
    "diamond", # static lib
    ]

DEPS_SHAPE = "ellipse"

DEPS_BLACKLIST = [
    "-l*", "-W*", # link flags
    "/*", # absolute dirs
    "m", "pthread", "util", "nsl", "resolv", # generic libs
    "*example*", "*demo*", "*test*", "*Test*", "*debug*" # helper targets
    ]


def preprocess(fname):
    graph_handle = gv.read(fname)
    txt = open(fname).read()
    targets = []

    # Replace the generated node names with their label. CMake generates a graph
    # like this:
    #
    # "node0" [ label="KF5DNSSD" shape="polygon"];
    # "node1" [ label="Qt5::Network" shape="ellipse"];
    # "node0" -> "node1"
    #
    # And we turn it into this:
    #
    # "KF5DNSSD" [ label="KF5DNSSD" shape="polygon"];
    # "Qt5::Network" [ label="Qt5::Network" shape="ellipse"];
    # "KF5DNSSD" -> "Qt5::Network"
    #
    # Using real framework names as labels makes it possible to merge multiple
    # .dot files.
    for node_handle in gvutils.get_node_list(graph_handle):
        node = gvutils.Node(node_handle)
        label = node.label.replace("KF5::", "")
        if node.shape in TARGET_SHAPES:
            targets.append(label)
        txt = txt.replace('"' + node.name + '"', '"' + label + '"')

    # Sometimes cmake will generate an entry for the target alias, something
    # like this:
    #
    # "node9" [ label="KParts" shape="polygon"];
    # ...
    # "node15" [ label="KF5::KParts" shape="ellipse"];
    # ...
    #
    # After our node renaming, this ends up with a second "KParts" node
    # definition, which we need to get rid of.
    for target in targets:
        rx = r' *"' + target + '".*label="KF5::' + target + '".*shape="ellipse".*;'
        txt = re.sub(rx, '', txt)
    return txt


def _add_extra_dependencies(fw, dct):
    lst = dct.get("framework-dependencies")
    if lst is None:
        return
    for dep in lst:
        fw.add_extra_framework(dep)


class DotFileParser(object):
    def __init__(self, with_qt):
        self._with_qt = with_qt

    def parse(self, tier, dot_file):
        name = os.path.basename(dot_file).replace(".dot", "")
        fw = Framework(tier, name)

        # Preprocess dot files so that they can be merged together.
        dot_data =  preprocess(dot_file)
        self._init_fw_from_dot_data(fw, dot_data, self._with_qt)

        return fw

    def _init_fw_from_dot_data(self, fw, dot_data, with_qt):
        def target_from_node(node):
            return node.name.replace("KF5", "")

        src_handle = gv.readstring(dot_data)

        targets = set()
        for node_handle in gvutils.get_node_list(src_handle):
            node = gvutils.Node(node_handle)
            if node.shape in TARGET_SHAPES and self._want(node):
                target = target_from_node(node)
                targets.add(target)
                fw.add_target(target)

        for edge_handle in gvutils.get_edge_list(src_handle):
            edge = gvutils.Edge(edge_handle)
            target = target_from_node(edge.tail)
            if target in targets and self._want(edge.head):
                dep_target = target_from_node(edge.head)
                fw.add_target_dependency(target, dep_target)

    def _want(self, node):
        if node.shape not in TARGET_SHAPES and node.shape != DEPS_SHAPE:
            return False
        name = node.name

        for pattern in DEPS_BLACKLIST:
            if fnmatch.fnmatchcase(node.name, pattern):
                return False
        if not self._with_qt and name.startswith("Qt"):
            return False
        return True


class FrameworkDb(object):
    def __init__(self):
        self._fw_list = []
        self._fw_for_target = {}

    def populate(self, dot_files, with_qt=False):
        """
        Init db from dot files
        """
        parser = DotFileParser(with_qt)
        for dot_file in dot_files:
            yaml_file = dot_file.replace(".dot", ".yaml")
            with open(yaml_file) as f:
                dct = yaml.safe_load(f)

            if 'tier' not in dct:
                # This mean it's not a frameworks
                continue

            tier = dct["tier"]
            fw = parser.parse(tier, dot_file)

            _add_extra_dependencies(fw, dct)
            self._fw_list.append(fw)
        self._update_fw_for_target()

    def _update_fw_for_target(self):
        self._fw_for_target = {}
        for fw in self._fw_list:
            for target in fw.get_targets():
                self._fw_for_target[target] = fw

    def find_by_name(self, name):
        for fw in self._fw_list:
            if fw.name == name:
                return fw
        return None

    def remove_unused_frameworks(self, wanted_fw):
        def add_to_set(fw_set, wanted_fw):
            fw_set.add(wanted_fw)

            for target in wanted_fw.get_all_target_dependencies():
                fw = self._fw_for_target.get(target)
                if fw is not None and not fw in fw_set:
                    add_to_set(fw_set, fw)

            for fw_name in wanted_fw.get_extra_frameworks():
                fw = self.find_by_name(fw_name)
                if not fw:
                    logging.warning("Framework {} has an extra dependency on {}, but there is no such framework".format(wanted_fw, fw_name))
                    continue
                if not fw in fw_set:
                    add_to_set(fw_set, fw)

        fw_set = set()
        add_to_set(fw_set, wanted_fw)
        self._fw_list = list(fw_set)

    def find_external_targets(self):
        all_targets = set([])
        fw_targets = set([])
        for fw in self._fw_list:
            fw_targets.update(fw.get_targets())
            all_targets.update(fw.get_all_target_dependencies())
        return all_targets.difference(fw_targets)

    def get_framework_for_target(self, target):
        return self._fw_for_target[target]

    def __iter__(self):
        return iter(self._fw_list)
