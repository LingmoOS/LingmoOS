#!/usr/bin/env python3
#
# SPDX-FileCopyrightText: 2014 Weng Xuetian <wengxt@gmail.com>
#
# SPDX-License-Identifier: LGPL-2.1-only
#

"""
This is a ugly code generate script, which is intended to be used
against i18nIMProto.c in IMdkit
it will generate all serialization code against xim protocol frame
"""
import fileinput
import collections
import sys
funcs = collections.OrderedDict()
static_size = dict()

funcname = None
for line in fileinput.input(sys.argv[1]):
    if "XimFrameRec" in line:
        tokens = line.strip().split()
        token = None
        for token in tokens:
            if "[]" in token:
                break
        if token is not None:
            funcname = "xcb_im_" + token[:-2]
            funcs[funcname] = []
            fieldidx = 0
    elif "FRAME(EOL)" in line:
        funcname = None
    elif funcname is not None:
        line = line.strip()

        try:
            tp = line[:line.rindex("),")+1]
            if tp == "_FRAME(POINTER)":
                continue
            if line.find("/*") != -1:
                name = line[line.find("/*")+2:line.find("*/")]
                name = name.strip().replace(" ", "_").replace(".", "_").replace("-", "_")
            else:
                name = "field{0}".format(fieldidx)
                fieldidx += 1
            funcs[funcname].append((tp,name))
        except Exception as e:
            pass

def gettypename(attr):
    if "BIT8" in attr:
        return ("uint8_t")
    elif "BIT16" in attr:
        return ("uint16_t")
    elif "BIT32" in attr:
        return ("uint32_t")
    elif attr == "_FRAME(BARRAY)":
        return ("xcb_im_bytearray_t")
    elif "_PTR" in attr:
        return "xcb_im_" + attr[attr.find("(") + 1:attr.find(")")]
    else:
        print(attr)
        assert(False)

for funcname, attrs in funcs.items():
    size = 0
    for attr, name in attrs:
        if "BIT8" in attr:
            size += 1
        elif "BIT16" in attr:
            size += 2
        elif "BIT32" in attr:
            size += 4
        elif "PAD" in attr:
            pad = int(attr[4])
            size += 0 if size % pad == 0 else (pad - size % pad)
        elif "PTR" in attr:
            if static_size[gettypename(attr)] < 0:
                size = -1
                break
            else:
                size += static_size[gettypename(attr)]
        else:
            size = -1
            break
    static_size[funcname] = size



def getsize(attr, name):
    if "BIT8" in attr:
        return "1"
    elif "BIT16" in attr:
        return "2"
    elif "BIT32" in attr:
        return "4"
    elif "_PTR" in attr:
        return "{1}_size(&frame->{0})".format(name, gettypename(attr))


def search_barray_length(attrs, i):
    for attr, name in reversed(attrs[0:i]):
        if "length" in name:
            return (attr, name)

    return attrs[i - 1]

if len(sys.argv) > 2:
    for funcname, attrs in funcs.items():
        print("typedef struct _{0}_t".format(funcname))
        print("{")
        skip = False
        for i, (attr, name) in enumerate(attrs):
            if skip:
                skip = False
                continue
            if "_FRAME(B" in attr:
                print("    {1} {0};".format(name, gettypename(attr)))
            if "_PTR" in attr:
                print("    {1}_t {0};".format(name, gettypename(attr)))
            elif "_PAD" in attr:
                pass
            elif "_BYTE_COUNTER" in attr:
                pass
            elif attr == "_FRAME(ITER)":
                (iterattr, itername) = attrs[i + 1]
                print("    struct {")
                print("        uint32_t size;")
                if "_FRAME(B" in iterattr:
                    print("        {0}* items;".format(gettypename(iterattr)))
                if "_PTR" in iterattr:
                    print("        {0}_t* items;".format(gettypename(iterattr)))
                print("    }} {0};".format(name))
                skip = True
            
        print("}} {0}_t;".format(funcname))
        print("")
    def print_generic(cat):
        print("#define frame_{0}_func(FRAME) _Generic((FRAME), \\".format(cat))
        first = True
        for funcname, attrs in funcs.items():
            if first:
                first=False
            else:
                print(",\\")
            if cat == "size":
                if static_size[funcname] >= 0:
                    print("    {0}_t : {1}".format(funcname, static_size[funcname]), end='')
                else:
                    print("    {0}_t : {0}_{1}(({0}_t*) &(FRAME))".format(funcname, cat), end='')
            else:
                print("    {0}_t : {0}_{1}".format(funcname, cat), end='')
        print(")")
        print("")
    for cat in ["read", "write", "size", "free"]:
        print_generic(cat)
    def print_generic_size():
        print("#define frame_has_static_size(FRAME) _Generic((FRAME), \\".format(cat))
        first = True
        for funcname, attrs in funcs.items():
            if first:
                first=False
            else:
                print(",\\")
            print("    {0}_t : {1}".format(funcname, "true" if static_size[funcname] >= 0 else "false"), end='')
        print(")")
        print("")
    print_generic_size()

    for funcname, attrs in funcs.items():
        usecounter = any("_BYTE_COUNTER" in attr for attr, name in attrs)
        usecounter8 = any("_BYTE_COUNTER(BIT8" in attr for attr, name in attrs)
        usecounter16 = any("_BYTE_COUNTER(BIT16" in attr for attr, name in attrs)
        usecounter32 = any("_BYTE_COUNTER(BIT32" in attr for attr, name in attrs)
        print(("""void {0}_read({0}_t *frame, uint8_t **data, size_t *len, bool swap);\n"""
               """uint8_t* {0}_write({0}_t *frame, uint8_t *data, bool swap);\n"""
               """void {0}_free({0}_t *frame);""").format(funcname))
        if static_size[funcname] >= 0:
            print("""#define {0}_size(ARG...) (((void)(ARG)), ({1}))""".format(funcname, static_size[funcname]))
        else:
            print("""size_t {0}_size({0}_t *frame);""".format(funcname))
        print("")
else:
    print("#include <string.h>")
    print("#include \"parser.h\"")
    print("#include \"ximproto.h\"")
    print("")

    for funcname, attrs in funcs.items():
        usecounter = any("_BYTE_COUNTER" in attr for attr, name in attrs)
        usecounter8 = any("_BYTE_COUNTER(BIT8" in attr for attr, name in attrs)
        usecounter16 = any("_BYTE_COUNTER(BIT16" in attr for attr, name in attrs)
        usecounter32 = any("_BYTE_COUNTER(BIT32" in attr for attr, name in attrs)
        usepad = any("_PAD" in attr for attr, name in attrs)
        print("void {0}_read({0}_t *frame, uint8_t **data, size_t *len, bool swap)".format(funcname))
        print("{")
        if len(attrs) > 0:
            print("    memset(frame, 0, sizeof(*frame));")
        if usepad:
            print("    uint8_t* start = *data;")
        if usecounter:
            if usecounter8:
                print("    uint8_t counter8 = 0;")
            if usecounter16:
                print("    uint16_t counter16 = 0;")
            if usecounter32:
                print("    uint32_t counter32 = 0;")
            print("    size_t counter = 0;")
        skip = False
        for i, (attr, name) in enumerate(attrs):
            if skip:
                skip = False
                continue
            if "_FRAME(BIT" in attr or "_PTR" in attr:
                print("    {1}_read(&frame->{0}, data, len, swap);".format(name, gettypename(attr)))
                print("    if (!*data) { return; }")
            elif "_FRAME(BARRAY" in attr:
                (lenattr, lenname) = search_barray_length(attrs, i)
                print("    {1}_read(&frame->{0}, frame->{2}, data, len, swap);".format(name, gettypename(attr), lenname))
                print("    if (!*data) { return; }")
            elif "_PAD" in attr:
                print("    *data = (uint8_t*) align_to_{0}((uintptr_t) *data, *data - start, len);".format(attr[4]))
                print("    if (!*data) { return; }")
            elif "_BYTE_COUNTER" in attr:
                if "BIT8" in attr:
                    countername = "counter8"
                elif "BIT16" in attr:
                    countername = "counter16"
                elif "BIT32" in attr:
                    countername = "counter32"
                print("    {1}_read(&{0}, data, len, swap);".format(countername, gettypename(attr)))
                print("    if (!*data) { return; }")
                if countername != "counter":
                    print("    counter = {0};".format(countername))
            elif attr == "_FRAME(ITER)":
                (iterattr, itername) = attrs[i + 1]
                print("    if (counter > *len) { *data = NULL; return; } else { *len -= counter; } ")
                print("    frame->{0}.items = NULL;".format(name, gettypename(iterattr)))
                print("    frame->{0}.size = 0;".format(name))
                print("    while (counter != 0) {")
                if "_PTR" in iterattr:
                    print("        void* temp = realloc(frame->{0}.items, (frame->{0}.size + 1) * sizeof({1}_t));".format(name, gettypename(iterattr)))
                else:
                    print("        void* temp = realloc(frame->{0}.items, (frame->{0}.size + 1) * sizeof({1}));".format(name, gettypename(iterattr)))
                print("        if (!temp) {")
                print("            *data = NULL;")
                print("            return;")
                print("        }")
                print("        frame->{0}.items = temp;".format(name))
                print("        {1}_read(&frame->{0}.items[frame->{0}.size], data, &counter, swap);".format(name, gettypename(iterattr)))
                print("        if (!*data) { return; }")
                print("        frame->{0}.size++;".format(name))
                print("    }")
                skip = True

        print("}")
        print("")
        print("uint8_t* {0}_write({0}_t *frame, uint8_t *data, bool swap)".format(funcname))
        print("{")
        if usepad:
            print("    uint8_t* start = data;")
        if usecounter:
            if usecounter8:
                print("    uint8_t counter8 = 0;")
            if usecounter16:
                print("    uint16_t counter16 = 0;")
            if usecounter32:
                print("    uint32_t counter = 0;")
        skip = False
        for i, (attr, name) in enumerate(attrs):
            if skip:
                skip = False
                continue
            if "_FRAME(BIT" in attr or "_PTR" in attr:
                print("    data = {1}_write(&frame->{0}, data, swap);".format(name, gettypename(attr)))
            elif "_FRAME(BARRAY" in attr:
                (lenattr, lenname) = search_barray_length(attrs, i)
                print("    data = {1}_write(&frame->{0}, frame->{2}, data, swap);".format(name, gettypename(attr), lenname))
            elif "_PAD" in attr:
                print("    data = (uint8_t*) align_to_{0}((uintptr_t) data, data - start, NULL);".format(attr[4]))
            elif "_BYTE_COUNTER" in attr:
                for j, (targetattr, targetname) in enumerate(attrs):
                    if j < i + 1:
                        continue
                    if targetattr == "_FRAME(ITER)":
                        (iterattr, itername) = attrs[j + 1]
                        break
                if "BIT8" in attr:
                    countername = "counter8"
                elif "BIT16" in attr:
                    countername = "counter16"
                elif "BIT32" in attr:
                    countername = "counter"
                if "_PTR" in iterattr:
                    print("    {0} = 0;".format(countername));
                    print("    for (uint32_t i = 0; i < frame->{0}.size; i++) {{".format(targetname))
                    print("        {2} += {1}_size(&frame->{0}.items[i]);".format(targetname, gettypename(iterattr), countername))
                    print("    }")
                else:
                    print("    {0} = frame->{1}.size * {2};".format(countername, targetname, getsize(iterattr, itername)))
                print("    data = {1}_write(&{0}, data, swap);".format(countername, gettypename(attr)))
            elif attr == "_FRAME(ITER)":
                (iterattr, itername) = attrs[i + 1]
                print("    for (uint32_t i = 0; i < frame->{0}.size; i++) {{".format(name))
                print("        data = {1}_write(&frame->{0}.items[i], data, swap);".format(name, gettypename(iterattr)))
                print("    }")
                skip = True

        print("    return data;");

        print("}")
        print("")

        if static_size[funcname] < 0:
            print("size_t {0}_size({0}_t *frame)".format(funcname))
            print("{")
            print("    size_t size = 0;")
            skip = False
            for i, (attr, name) in enumerate(attrs):
                if skip:
                    skip = False
                    continue
                if "_FRAME(BIT" in attr or "_BYTE_COUNTER" in attr:
                    print("    size += {0};".format(getsize(attr, name)))
                elif "_PTR" in attr:
                    print("    size += {1}_size(&frame->{0});".format(name, gettypename(attr)))
                elif "_FRAME(BARRAY" in attr:
                    (lenattr, lenname) = search_barray_length(attrs, i)
                    print("    size += frame->{0};".format(lenname))
                elif "_PAD" in attr:
                    print("    size = align_to_{0}(size, size, NULL);".format(attr[4]))
                elif attr == "_FRAME(ITER)":
                    (iterattr, itername) = attrs[i + 1]
                    if "_PTR" in iterattr:
                        print("    for (uint32_t i = 0; i < frame->{0}.size; i++) {{".format(name))
                        print("        size += {1}_size(&frame->{0}.items[i]);".format(name, gettypename(iterattr)))
                        print("    }")

                    else:
                        print("    size += frame->{0}.size * {1};".format(name, getsize(iterattr, itername)))
                    skip = True
            print("    return size;")


            print("}")
            print("")

        print("void {0}_free({0}_t *frame)".format(funcname))
        print("{")
        for i, (attr, name) in enumerate(attrs):
            if skip:
                skip = False
                continue
            if "_PTR" in attr:
                print("    {1}_free(&frame->{0});".format(name, gettypename(attr)))
            elif "_FRAME(BARRAY" in attr:
                pass
            elif attr == "_FRAME(ITER)":
                (iterattr, itername) = attrs[i + 1]
                if "_PTR" in iterattr:
                    print("    if (frame->{0}.items) {{".format(name))
                    print("        for (uint32_t i = 0; i < frame->{0}.size; i++) {{".format(name))
                    print("            {1}_free(&frame->{0}.items[i]);".format(name, gettypename(iterattr)))
                    print("        }")
                    print("    }")
                print("    free(frame->{0}.items);".format(name))
                skip = True


        print("}")
        print("")
