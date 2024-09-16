#!/usr/bin/env python3

# Copyright (C) 2017 Deepin Technology Co., Ltd.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.

import xml.etree.ElementTree as ET
import sys
import os
from glob import glob

# const
attr_xml_lang = "{http://www.w3.org/XML/1998/namespace}lang"

class Action:
    #msgs dict
    #descs dict
    #msg_src string
    #desc_src string
    def __init__(self, action_elem):
        self.id = action_elem.get("id")
        self.msgs = {}
        self.descs = {}
        for item in action_elem:
            if item.tag == "message":
                lang = item.get(attr_xml_lang)
                if lang is None:
                    self.msg_src = item.text
                else:
                    self.msgs[lang] = item.text
            if item.tag == "description":
                lang = item.get(attr_xml_lang)
                if lang is None:
                    self.desc_src = item.text
                else:
                    self.descs[lang] = item.text

    def get_desc_src(self):
        if hasattr(self, "desc_src"):
            return create_ts_msg(self.desc_src, None, self.id, "description")
        return None

    def get_msg_src(self):
        return create_ts_msg(self.msg_src, None, self.id, "message")

    def get_desc(self, lang):
        desc = self.descs.get(lang)
        return create_ts_msg(self.desc_src, desc, self.id, "description")

    def get_msg(self, lang):
        msg = self.msgs.get(lang)
        return create_ts_msg(self.msg_src, msg, self.id, "message")

    def get_langs(self):
        # return msgs and descs keys
        langs = {}
        for k in self.msgs:
            langs[k] = 1
        for k in self.descs:
            langs[k] = 1
        return langs.keys()

def policy2ts(policyFile, outputTsDir):
    print("policyFile:", policyFile)
    os.makedirs(outputTsDir, exist_ok=True)
    actions = []
    tree = ET.parse(policyFile)
    for action_elem in tree.iter("action"):
        action = Action(action_elem)
        actions.append(action)
    if len(actions) == 0:
        return
    ts, ctx = create_ts_doc()
    for action in actions:
        ctx.append(action.get_msg_src())
        if action.get_desc_src():
            ctx.append(action.get_desc_src())
    outputTsFile = outputTsDir + "/policy.ts"
    print("outputTsFile:", outputTsFile)
    write_ts(ts, outputTsFile)

def init(policyFile, outputTsDir):
    print("policyFile:", policyFile)
    print("outputTsDir:", outputTsDir)
    os.makedirs(outputTsDir, exist_ok=True)
    actions = []
    tree = ET.parse(policyFile)
    for action_elem in tree.iter("action"):
        action = Action(action_elem)
        actions.append(action)
    if len(actions) == 0:
        return
    ts, ctx = create_ts_doc()
    langs = {}
    for action in actions:
        ctx.append(action.get_msg_src())
        ctx.append(action.get_desc_src())
        l = action.get_langs()
        for k in l:
            langs[k] = 1
    write_ts(ts, outputTsDir + "/policy.ts")
    print(langs.keys())
    for lang in langs.keys():
        ts, ctx = create_ts_doc()
        ts.set("language", lang)
        for action in actions:
            ctx.append(action.get_msg(lang))
            ctx.append(action.get_desc(lang))
        write_ts(ts, outputTsDir + "/policy_" + lang +".ts")

def ts2policy(policyFile, tsDir, outputPolicyFile):
    print("policyFile:", policyFile)
    print("tsDir:", tsDir)
    print("outputPolicyFile:", outputPolicyFile)

    # load ts files
    ts_files = glob(tsDir + "/policy_*.ts")
    tr_dict = {}
    # print("ts_files:", ts_files)
    for filename in ts_files:
        lang, msgs = load_tr(filename)
        tr_dict[lang] = msgs
    tree = ET.parse(policyFile)

    remove_list = []
    for action_elem in tree.iter("action"):
        for item in action_elem:
            if item.tag == "message" or item.tag == "description":
                lang = item.get(attr_xml_lang)
                if lang is not None:
                    remove_list.append((action_elem, item))
    for i in remove_list:
        i[0].remove(i[1])

    for action_elem in tree.iter("action"):
        action = Action(action_elem)
        for lang in sorted(tr_dict):
            d = tr_dict[lang]
            src = ""
            for ty in ["description", "message"]:
                if ty == "message":
                    src = action.msg_src
                else:
                    src = action.desc_src if hasattr(action, 'desc_src') else None

                val = d.get(action.id + "!" + ty)
                if val != None:
                    source = val[0]
                    translation = val[1]
                    if source == src and translation != "" and translation is not None:
                        msg = ET.SubElement(action_elem, ty)
                        msg.set("xml:lang", lang)
                        msg.text = translation

    write_policy(tree.getroot(), outputPolicyFile)

def load_tr(tsFile):
    tree = ET.parse(tsFile)
    root = tree.getroot()
    lang = root.get("language")
    msgs = root.findall("./context/message")
    msgs_dict = {}
    for msg in msgs:
        source = msg.find("./source").text
        translation = msg.find("./translation").text
        key = msg.find("./location").get("filename")
        msgs_dict[key] = (source,translation)
    return lang, msgs_dict

def create_ts_doc():
    ts = ET.Element("TS")
    ts.set("version", "2.1")
    context = ET.SubElement(ts, "context")
    ctx_name = ET.SubElement(context, "name")
    ctx_name.text = "policy"
    return ts, context

def create_ts_msg(src, translation, action_id, ty):
    msg = ET.Element("message")
    location = ET.SubElement(msg, "location")
    location.set("filename", action_id + "!" + ty)
    location.set("line", "0")
    source = ET.SubElement(msg, "source")
    source.text = src
    tr = ET.SubElement(msg, "translation")
    if translation is None:
        tr.set("type", "unfinished")
    else:
        tr.text = translation
    return msg

def indent(elem, level=0, more_sibs=False):
    i = "\n"
    if level:
        i += (level-1) * "\t"
    num_kids = len(elem)
    if num_kids:
        if not elem.text or not elem.text.strip():
            elem.text = i + "\t"
            if level:
                elem.text += "\t"
        count = 0
        for kid in elem:
            indent(kid, level+1, count < num_kids - 1)
            count += 1
        if not elem.tail or not elem.tail.strip():
            elem.tail = i
            if more_sibs:
                elem.tail += "\t"
    else:
        if level and (not elem.tail or not elem.tail.strip()):
            elem.tail = i
            if more_sibs:
                elem.tail += "\t"

def write_xml(root, filename, header):
    indent(root)
    content = ET.tostring(root, encoding="unicode")
    with open(filename, 'w', encoding="utf-8") as f:
        f.write(header)
        f.write(content)

def write_ts(root, filename):
    header = '''<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE TS>
'''
    write_xml(root, filename, header)

def write_policy(root, filename):
    header = '''<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE policyconfig PUBLIC "-//freedesktop//DTD PolicyKit Policy Configuration 1.0//EN"
"http://www.freedesktop.org/standards/PolicyKit/1.0/policyconfig.dtd">
'''
    write_xml(root, filename, header)

argv = sys.argv
if len(argv) == 4 and argv[1] == "policy2ts":
    policy2ts(argv[2], argv[3])
elif len(argv) == 4 and argv[1] == "init":
    init(argv[2], argv[3])
elif len(argv) == 5 and argv[1] == "ts2policy":
    ts2policy(argv[2], argv[3], argv[4])
else:
    # print usage
    print('''
init policyFile outputTsDir

policy2ts policyFile outputTsDir

ts2policy policyFile tsDir outputPolicyFile
    ''')

