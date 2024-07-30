# -*- coding: utf-8 -*-
#
# SPDX-FileCopyrightText: 2014 Aurélien Gâteau <agateau@kde.org>
#
# SPDX-License-Identifier: BSD-2-Clause

import logging
import itertools
from functools import cmp_to_key

from kapidox.depdiagram.block import Block, quote
from kapidox.depdiagram.framework import Framework
from kapidox.depdiagram.frameworkdb import FrameworkDb

__all__ = ('generate',)

ROOT_NODE_ATTRS = dict(fontsize=12, shape="box")

GROUP_ATTRS = dict(style="filled", fillcolor="grey95", color="grey85")

# Blocks within groups
OTHER_ATTRS = dict(style="filled", fillcolor="cornsilk", color="black")

QT_ATTRS = dict(style="filled", fillcolor="darkseagreen1", color="black")

FW_ATTRS = dict(style="filled", fillcolor="azure", color="black")

# Target blocks, used with --detailed
FW_TARGET_ATTRS = dict(style="filled", fillcolor="paleturquoise")

# Highlight the wanted framework, used with --framework
WANTED_FW_ATTRS = dict(penwidth=2)

class FrameworkCmp(object):
    def __init__(self, db):
        self.db = db
        self.src = set(db)
        self.dst = []

    def __call__(self, fw1, fw2):
        if self.depends_on(fw1, fw2):
            return 1
        if self.depends_on(fw2, fw1):
            return -1
        return 0

    def depends_on(self, depender_fw, provider_fw):
        for dep_target in depender_fw.get_all_target_dependencies():
            if provider_fw.has_target(dep_target):
                return True

            try:
                dep_fw = self.db.get_framework_for_target(dep_target)
            except KeyError:
                # No framework for this target, must be an external dependency,
                # carry on
                continue
            if dep_fw != depender_fw and self.depends_on(dep_fw, provider_fw):
                return True

        return False


class DotWriter(Block):
    def __init__(self, db, out, wanted_fw=None, detailed=False):
        Block.__init__(self, out)
        self.db = db
        self.detailed = detailed
        self.wanted_fw = wanted_fw

    def write(self):
        with self.curly_block("digraph Root") as root:
            root.write_list_attrs("node", **ROOT_NODE_ATTRS)

            other_targets = self.db.find_external_targets()
            qt_targets = set([x for x in other_targets if x.startswith("Qt")])
            other_targets.difference_update(qt_targets)

            if qt_targets:
                with root.cluster_block("Qt", **GROUP_ATTRS) as b:
                    b.write_list_attrs("node", **QT_ATTRS)
                    b.write_nodes(qt_targets)

            if other_targets:
                with root.cluster_block("Others", **GROUP_ATTRS) as b:
                    b.write_list_attrs("node", **OTHER_ATTRS)
                    b.write_nodes(other_targets)

            lst = sorted([x for x in self.db], key=lambda x: x.tier)
            for tier, frameworks in itertools.groupby(lst, lambda x: x.tier):
                cluster_title = "Tier {}".format(tier)
                with root.cluster_block(cluster_title, **GROUP_ATTRS) as tier_block:
                    tier_block.write_list_attrs("node", **FW_ATTRS)
                    # Sort frameworks within the tier to ensure frameworks which
                    # depend on other frameworks from that tier are listed after
                    # their dependees.
                    frameworks = list(frameworks)
                    for fw in sorted(frameworks, key=cmp_to_key(FrameworkCmp(self.db))):
                        if self.detailed:
                            self.write_detailed_framework(tier_block, fw)
                        else:
                            self.write_framework(tier_block, fw)

    def write_framework(self, tier_block, fw):
        if fw == self.wanted_fw:
            tier_block.write_list_attrs(quote(fw.name), **WANTED_FW_ATTRS)
        else:
            tier_block.write_nodes([fw.name])
        edges = set([])
        for target in fw.get_all_target_dependencies():
            try:
                target_fw = self.db.get_framework_for_target(target)
                if fw == target_fw:
                    continue
                dep = target_fw.name
            except KeyError:
                dep = target
            edges.add((fw.name, dep))
        for dep_fw in fw.get_extra_frameworks():
            edges.add((fw.name, dep_fw))
        for edge in edges:
            tier_block.writeln('"{}" -> "{}";'.format(*edge))

    def write_detailed_framework(self, tier_block, fw):
        with tier_block.cluster_block(fw.name, **FW_ATTRS) as fw_block:
            if fw == self.wanted_fw:
                fw_block.write_attrs(**WANTED_FW_ATTRS)
            fw_block.write_list_attrs("node", **FW_TARGET_ATTRS)
            targets = sorted(fw.get_targets())
            fw_block.write_nodes(targets)
            for target in targets:
                deps = fw.get_dependencies_for_target(target)
                for dep in sorted(deps):
                    fw_block.writeln('"{}" -> "{}";'.format(target, dep))


def generate(out, dot_files, framework=None, with_qt=False, detailed=False):
    db = FrameworkDb()
    db.populate(dot_files, with_qt=with_qt)

    if framework:
        wanted_fw = db.find_by_name(framework)
        if wanted_fw is None:
            logging.error("No framework named {}.".format(framework))
            return False
        db.remove_unused_frameworks(wanted_fw)
    else:
        wanted_fw = None

    writer = DotWriter(db, out, wanted_fw=wanted_fw, detailed=detailed)
    writer.write()

    return True

# vi: ts=4 sw=4 et
