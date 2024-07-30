#!@Python3_EXECUTABLE@

# SPDX-License-Identifier: GPL-3.0-or-later
# SPDX-FileCopyrightText: 2010  Darwin M. Bautista <djclue917@gmail.com>
# SPDX-FileCopyrightText: 2011 Craig Drummond <craig.p.drummond@gmail.com>

# UFW KControl Module

import os
import sys

import locale
import gettext

import getopt
import shutil
import hashlib
import io

from xml.etree import ElementTree as etree
from copy import deepcopy

import ufw.common
import ufw.frontend
from ufw.util import valid_address
from ufw.common import UFWRule

ANY_ADDR       = '0.0.0.0/0'
ANY_PORT       = 'any'
ANY_PROTOCOL   = 'any'
OLD_DESCR_FILE = "/etc/ufw/descriptions"
DESCR_FILE     = "/etc/kcm_ufw/descriptions"
DEFAULTS_FILE  = "@DATA_INSTALL_DIR@/kcm_ufw/defaults"

ERROR_FAILED_TO_SET_STATUS      = -1
ERROR_INVALID_INDEX             = -2
ERROR_INVALID_XML_NO_RULE       = -3
ERROR_INVALID_XML_NO_ACTION_XML = -4
ERROR_INVALID_XML_NO_DEFAULTS   = -5
ERROR_INVALID_XML_NO_MODULES    = -6


class UFWFrontend(ufw.frontend.UFWFrontend):

    def __init__(self, dryrun):
        ufw.frontend.UFWFrontend.__init__(self, dryrun)
        # Compatibility for ufw 0.31
        # This is a better way of handling method renames instead of putting
        # try/except blocks all over the whole application code
        # Ref: http://code.google.com/p/ufw-frontends/issues/detail?id=20
        try:
            self.backend.get_default_policy
        except AttributeError:
            self.backend.get_default_policy = self.backend._get_default_policy
        try:
            self.backend._is_enabled
        except AttributeError:
            self.backend._is_enabled = self.backend.is_enabled


def localizeUfw():
    # Ref: http://code.google.com/p/ufw-frontends/issues/detail?id=19
    locale.setlocale(locale.LC_ALL, 'C')
    gettext.install(ufw.common.programName)

#define looad descriptions file
# descrsChanged=False
# def moveOldDescrFile():
#     try:
#         if os.path.exists(OLD_DESCR_FILE) and os.path.isfile(OLD_DESCR_FILE):
#             shutil.move(OLD_DESCR_FILE, DESCR_FILE)
#     except Exception as e:
#         return # Old file does not exist, or is not a file!...
#
# def loadDescriptions():
#     moveOldDescrFile();
#     try:
#         global descrs
#         descrs={"a":"b"}
#         descrData=open(DESCR_FILE).read()
#         descrs=eval(descrData)
#     except Exception as e:
#         return
#
# def removeDescriptions():
#     try:
#         os.remove(DESCR_FILE)
#         descrsChanged=False
#     except Exception as e:
#         return
#
# def saveDescriptions():
#     global descrsChanged
#     if descrsChanged:
#         try:
#             if ("a" in descrs):
#                 del descrs["a"]
#             if len(descrs)>0:
#                 descrFile = open(DESCR_FILE, 'w')
#                 descrFile.write(repr(descrs))
#                 descrsChanged=False
#             else:
#                 removeDescriptions()
#         except Exception as e:
#             return
#
# def getDescription(hashStr):
#     if (hashStr in descrs):
#         return descrs[hashStr]
#     return ''
#
# def removeDescription(hashStr):
#     if (hashStr in descrs):
#         del descrs[hashStr]
#         global descrsChanged
#         descrsChanged=True
#
# def updateDescription(hashStr, descrStr):
#     if ((hashStr in descrs)==False) or (descrs[hashStr]!=descrStr):
#         removeDescription(hashStr)
#         descrs[hashStr]=descrStr
#         global descrsChanged
#         descrsChanged=True

def loadDefaultSettings(ufw):
    try:
        defaultsFile=open(DEFAULTS_FILE)
        while 1:
            line = defaultsFile.readline()
            if line == '':
                break
            parts=line.split('=')
            if len(parts) == 2:
                value=parts[1].replace('\n', '')
                if parts[0] == 'incoming':
                    ufw.set_default_policy(value, 'incoming')
                elif parts[0] == 'outgoing':
                    ufw.set_default_policy(value, 'outgoing')
                elif parts[0] == 'loglevel':
                    ufw.set_loglevel(value)
                elif parts[0] == 'ipv6':
                    ufw.backend.set_default(ufw.backend.files['defaults'], 'IPV6', value.lower())
                elif parts[0] == 'modules':
                    ufw.backend.set_default(ufw.backend.files['defaults'], 'IPT_MODULES', '"' + value + '"')
    except Exception as e:
        return

# Localise UFW, and init the 'frontend'
localizeUfw()
ufw=UFWFrontend(False)

def getProtocol(rule):
    """Determine protocol of rule.
    Taken from ufw.parser.UFWCommandRule.parse
    """
    # Determine src type
    if rule.src == ANY_ADDR:
        from_type = 'any'
    else:
        from_type = ('v6' if valid_address(rule.src, '6') else 'v4')
    # Determine dst type
    if rule.dst == ANY_ADDR:
        to_type = 'any'
    else:
        to_type = ('v6' if valid_address(rule.dst, '6') else 'v4')
    # Figure out the type of rule (IPv4, IPv6, or both)
    if from_type == ANY_PROTOCOL and to_type == ANY_PROTOCOL:
        protocol = 'both'
    elif from_type != ANY_PROTOCOL and to_type != ANY_PROTOCOL and from_type != to_type:
        err_msg = _("Mixed IP versions for 'from' and 'to'")
        raise ufw.common.UFWError(err_msg)
    elif from_type != ANY_PROTOCOL:
        protocol = from_type
    elif to_type != ANY_PROTOCOL:
        protocol = to_type
    return protocol

def insertRule(ufw, rule, protocol=None):
    if protocol is None:
        protocol = getProtocol(rule)
    rule = rule.dup_rule()
    # Fix any inconsistency
    if rule.sapp or rule.dapp:
        rule.set_protocol(ANY_PROTOCOL)
        if rule.sapp:
            rule.sport = rule.sapp
        if rule.dapp:
            rule.dport = rule.dapp
    # If trying to insert beyond the end, just set position to 0
    if rule.position and not ufw.backend.get_rule_by_number(rule.position):
        rule.set_position(0)
    ufw.set_rule(rule, protocol)
    # Reset the positions of the recently inserted rule(s)
    if rule.position:
        s = rule.position - 1
        e = rule.position + 1
        for r in ufw.backend.get_rules()[s:e]:
            r.set_position(0)
    return rule

def getRulesList(ufw):
    app_rules = []
    for i, r in enumerate(ufw.backend.get_rules()):
        if r.dapp or r.sapp:
            t = r.get_app_tuple()
            if t in app_rules:
                continue
            else:
                app_rules.append(t)
        yield (i, r)

def encodeText(str):
    str=str.replace("&", "&amp;")
    str=str.replace("<", "&lt;")
    str=str.replace("\"", "&quot;")
    str=str.replace(">", "&gt;")
    return str

def ruleDetails(rule):
    xmlStr = io.StringIO()
    xmlStr.write("action=\"")
    xmlStr.write(rule.action.lower())
    xmlStr.write("\" direction=\"")
    xmlStr.write(rule.direction.lower())
    xmlStr.write("\" dapp=\"")
    xmlStr.write(rule.dapp)
    xmlStr.write("\" sapp=\"")
    xmlStr.write(rule.sapp)
    xmlStr.write("\" dport=\"")
    xmlStr.write(rule.dport)
    xmlStr.write("\" sport=\"")
    xmlStr.write(rule.sport)
    xmlStr.write("\" protocol=\"")
    xmlStr.write(rule.protocol.lower())
    xmlStr.write("\" dst=\"")
    xmlStr.write(rule.dst)
    xmlStr.write("\" src=\"")
    xmlStr.write(rule.src)
    xmlStr.write("\" interface_in=\"")
    xmlStr.write(rule.interface_in)
    xmlStr.write("\" interface_out=\"")
    xmlStr.write(rule.interface_out)
    xmlStr.write("\" v6=\"")
    if rule.v6:
        xmlStr.write('True')
    else:
        xmlStr.write('False')
    return xmlStr.getvalue()

# def detailsHash(details):
#     ruleHash = hashlib.md5()
#     ruleHash.update(details.encode('utf-8'))
#     return ruleHash.hexdigest()

# Convert a rule to an XML string...
def toXml(rule, xmlStr):
    xmlStr.write("<rule position=\"")
    xmlStr.write(str(rule.position))
    xmlStr.write("\" ")
    details=ruleDetails(rule)
    xmlStr.write(details)
#     hashStr=detailsHash(details)
#     descr=getDescription(hashStr)
#     if descr != '':
#         xmlStr.write("\" descr=\"")
#         xmlStr.write(encodeText(descr))
#         xmlStr.write("\" hash=\"")
#         xmlStr.write(hashStr)
    xmlStr.write("\" logtype=\"")
    xmlStr.write(rule.logtype)
    xmlStr.write("\" />")

# Create rule from XML...
def fromXml(str):
    elem = etree.XML(str)
    if elem.tag != 'rule':
        error("ERROR: Invalid XML, expected \'rule\' element", ERROR_INVALID_XML_NO_RULE)
    action=elem.get('action', '').lower()
    if action == '':
        error("ERROR: Invalid XML, no action specified", ERROR_INVALID_XML_NO_ACTION_XML)
    protocol=elem.get('protocol', ANY_PROTOCOL).lower()
    rule = UFWRule(action, protocol)
    rule.position=int(elem.get('position', 0))
    rule.direction=elem.get('direction', 'in').lower()
    rule.dapp=elem.get('dapp', '')
    rule.sapp=elem.get('sapp', '')
    rule.dport=elem.get('dport', ANY_PORT)
    rule.sport=elem.get('sport', ANY_PORT)
    rule.dst=elem.get('dst', ANY_ADDR)
    rule.src=elem.get('src', ANY_ADDR)
    rule.interface_in=elem.get('interface_in', '')
    rule.interface_out=elem.get('interface_out', '')
    rule.logtype=elem.get('logtype', '').lower()
    rule.v6=elem.get('v6', 'False').lower() == "true"
    return rule

def getStatus(ufw, xmlStr):
    xmlStr.write("<status enabled=\"")
    if ufw.backend._is_enabled():
        xmlStr.write('true')
    else:
        xmlStr.write('false')
    xmlStr.write("\" />")

def setEnabled(ufw, status):
    if status.lower() == "false":
        stat=False
    else:
        stat=True
    if stat != ufw.backend._is_enabled():
        ufw.set_enabled(stat)
        if ufw.backend._is_enabled() != stat:
            error("ERROR: Failed to set UFW status", ERROR_FAILED_TO_SET_STATUS)

def getDefaults(ufw, xmlStr):
    conf = ufw.backend.defaults
    xmlStr.write("<defaults incoming=\"")
    xmlStr.write(ufw.backend.get_default_policy('input'))
    xmlStr.write("\" outgoing=\"")
    xmlStr.write(ufw.backend.get_default_policy('output'))
    xmlStr.write("\" loglevel=\"")
    xmlStr.write(conf['loglevel'])
    xmlStr.write("\" ipv6=\"")
    xmlStr.write(conf['ipv6'])
    xmlStr.write("\" />")

def setDefaults(ufw, xml):
    elem = etree.XML(xml)
    if elem.tag != 'defaults':
        error("ERROR: Invalid XML, expected \'defaults\' element", ERROR_INVALID_XML_NO_DEFAULTS)
    enabled=ufw.backend._is_enabled()
    if enabled:
        ufw.set_enabled(False)
    ipv6=elem.get('ipv6', '').lower()
    if ipv6 != '':
        del ufw
        ufw=UFWFrontend(False)
        ufw.backend.set_default(ufw.backend.files['defaults'], 'IPV6', ipv6)
        del ufw
        ufw=UFWFrontend(False)
    policy=elem.get('incoming', '').lower()
    if policy != '':
        ufw.set_default_policy(policy, 'incoming')
    policy=elem.get('outgoing', '').lower()
    if policy != '':
        ufw.set_default_policy(policy, 'outgoing')
    loglevel=elem.get('loglevel', '').lower()
    if loglevel != '':
        ufw.set_loglevel(loglevel)
    if enabled:
        ufw.set_enabled(True)

def getRules(ufw, xmlStr):
    xmlStr.write("<rules>")
    for i, data in enumerate(getRulesList(ufw)):
        idx, rule = data
        toXml(rule.dup_rule(), xmlStr)
    xmlStr.write("</rules>")

# def updateRuleDescription(rule, xml):
#     elem=etree.XML(xml)
#     descr=elem.get('descr', '')
#     oldHashCode=elem.get('hash', '')
#     if descr != '':
#         details=ruleDetails(rule)
#         hashStr=detailsHash(details)
#         # For an update, we should be passed old hash code - if so, remove old entry...
#         if oldHashCode!= '':
#             removeDescription(oldHashCode)
#         updateDescription(hashStr, descr)
#     else:
#         if oldHashCode!= '':
#             removeDescription(oldHashCode)

def addRule(ufw, xml):
    rule=fromXml(xml)
    inserted=insertRule(ufw, rule)
#     updateRuleDescription(inserted, xml)

def updateRule(ufw, xml):
    rule=fromXml(xml)
    deleted=False
    try:
        prev=deepcopy(ufw.backend.get_rule_by_number(rule.position))
        ufw.delete_rule(rule.position, True)
        deleted=True
        inserted=insertRule(ufw, rule)
        deleted=False
#         updateRuleDescription(inserted, xml)
    except Exception as e:
        if deleted:
            insertRule(ufw, prev)

# def updateRuleDescr(ufw, xml):
#     rule=fromXml(xml)
#     details=ruleDetails(rule)
#     hashStr=detailsHash(details)
#     updateRuleDescription(rule, xml)

# Remove a rule. Index is either; just the index, or <index>:<hashcode>
def removeRule(ufw, index):
    parts=index.split(':')
    try:
        if 2==len(parts):
            idx=int(parts[0])
        else:
            idx=int(index)
        if idx<1 or idx>(ufw.backend.get_rules_count(False)+ufw.backend.get_rules_count(True)):
            error("ERROR: Invalid index", ERROR_INVALID_INDEX)
#         if 2==len(parts):
#             removeDescription(parts[1])
#         else:
#             rule=ufw.backend.get_rule_by_number(index)
#             if rule:
#                 details=ruleDetails(rule)
#                 hashStr=detailsHash(details)
#                 removeDescription(hashStr)
        ufw.delete_rule(idx, True)
    #except ufw.common.UFWError as e:
        #error("ERROR: UFW error", e.value)
    except ValueError:
        error("ERROR: Invalid input type", ERROR_INVALID_INDEX)

def moveRule(ufw, indexes):
    idx=indexes.split(':')
    if 2!= len(idx):
        error("ERROR: Invalid number of indexes", ERROR_INVALID_INDEX)
    fromIndex=int(idx[0])
    toIndex=int(idx[1])
    if fromIndex == toIndex:
        error("ERROR: Source and destination cannot be the same", ERROR_INVALID_INDEX)
    rule=ufw.backend.get_rule_by_number(fromIndex).dup_rule()
    ufw.delete_rule(fromIndex, True)
    rule.position=toIndex
    insertRule(ufw, rule)

def reset(ufw):
    loadDefaultSettings(ufw)
    clearRules(ufw)
    ufw.reset(True)
    if ufw.backend._is_enabled():
        ufw.set_enabled(False)
        ufw.set_enabled(True)

def clearRules(ufw):
#     removeDescriptions()
    count=ufw.backend.get_rules_count(False)+ufw.backend.get_rules_count(True)
    for num in range(0, count):
        try:
            ufw.delete_rule(1, True)
        except ufw.common.UFWError as e:
            pass

def getModules(ufw, xmlStr):
    xmlStr.write("<modules enabled=\"")
    if 'ipt_modules' in ufw.backend.defaults:
        modules=ufw.backend.defaults['ipt_modules']
        if modules != '':
            xmlStr.write(modules)
    xmlStr.write("\" />")

def setModules(ufw, xml):
    elem = etree.XML(xml)
    if elem.tag != 'modules':
        error("ERROR: Invalid XML, expected \'modules' element", ERROR_INVALID_XML_NO_MODULES)
    modules=elem.get('enabled', '').lower()
    modules = '"' + modules + '"'
    ufw.backend.set_default(ufw.backend.files['defaults'], 'IPT_MODULES', modules)

# def getProfiles(ufw, xmlStr):
#     xmlStr.write("<profiles names=\"")
#     first=True
#     for profile in ufw.backend.profiles.keys():
#         if profile != '':
#             if first:
#                 first=False
#             else:
#                 xmlStr.write(" ")
#             xmlStr.write(profile)
#     xmlStr.write("\" />")

def error(str, rv):
    print >> sys.stderr, str
    #sys.exit(rv)

def main():
    try:
#         opts, args = getopt.getopt(sys.argv[1:], "hse:df:la:u:U:r:m:tiI:x",
#                                    ["help", "status", "setEnabled=", "defaults", "setDefaults=", "list", "add=",
#                                     "update=", "updateDescr=", "remove=", "move=", "reset", "modules", "setModules=", "clearRules"])
        opts, args = getopt.getopt(sys.argv[1:], "hse:df:la:u:U:r:m:tiI:x",
                                   ["help", "status", "setEnabled=", "defaults", "setDefaults=", "list", "add=",
                                    "update=", "remove=", "move=", "reset", "modules", "setModules=", "clearRules"])
    except getopt.GetoptError as err:
        # print help information and exit:
        print >> sys.stderr, str(err) # will print something like "option -a not recognized"
        usage()
        sys.exit(1)
#     loadDescriptions()
    returnXml = False
    xmlOut = io.StringIO()
    xmlOut.write("<?xml version=\"1.0\" encoding=\"UTF-8\"?><ufw>")
    for o, a in opts:
        if o in ("-h", "--help"):
            usage()
            sys.exit()
        elif o in ("-s", "--status"):
            getStatus(ufw, xmlOut)
            returnXml=True
        elif o in ("-e", "--setEnabled"):
            setEnabled(ufw, a)
        elif o in ("-d", "--defaults"):
            getDefaults(ufw, xmlOut)
            returnXml=True
        elif o in ("-f", "--setDefaults"):
            setDefaults(ufw, a)
        elif o in ("-l", "--list"):
            getRules(ufw, xmlOut)
            returnXml=True
        elif o in ("-a", "--add"):
            addRule(ufw, a)
        elif o in ("-u", "--update"):
            updateRule(ufw, a)
#         elif o in ("-U", "--updateDescr"):
#             updateRuleDescr(ufw, a)
        elif o in ("-r", "--remove"):
            removeRule(ufw, a)
        elif o in ("-m", "--move"):
            moveRule(ufw, a)
        elif o in ("-t", "--reset"):
            reset(ufw)
        elif o in ("-i", "--modules"):
            getModules(ufw, xmlOut)
            returnXml=True
        elif o in ("-I", "--setModules"):
            setModules(ufw, a)
        elif o in ("-x", "--clearRules"):
            clearRules(ufw)
        else:
            usage()
#     saveDescriptions()
    if returnXml:
        xmlOut.write("</ufw>")
        print (xmlOut.getvalue())

def usage():
    print ("Python helper for UFW KCM")
    print ("")
    print ("(C) Craig Drummond, 2011")
    print ("")
    print ("Usage:")
    print ("    "+sys.argv[0]+" --status")
    print ("    "+sys.argv[0]+" --setEnabled <true/false>")
    print ("    "+sys.argv[0]+" --defaults")
    print ("    "+sys.argv[0]+" --setDefaults <xml>")
    print ("    "+sys.argv[0]+" --list")
    print ("    "+sys.argv[0]+" --add <xml>")
    print ("    "+sys.argv[0]+" --update <xml>")
#     print ("    "+sys.argv[0]+" --updateDescr <xml>")
    print ("    "+sys.argv[0]+" --remove <index>")
    print ("    "+sys.argv[0]+" --remove <index:hash>")
    print ("    "+sys.argv[0]+" --move <from:to>")
    print ("    "+sys.argv[0]+" --reset")
    print ("    "+sys.argv[0]+" --modules")
    print ("    "+sys.argv[0]+" --setModules <xml>")
    print ("    "+sys.argv[0]+" --clearRules")

if __name__ == "__main__":
    main()
