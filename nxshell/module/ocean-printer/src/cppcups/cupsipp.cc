// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "cupsipp.h"
#include "cupsmodule.h"
#include <stdexcept>

using namespace std;

//////////////////
// IPPAttribute //
//////////////////

IPPAttribute::IPPAttribute(void)
{
    this->group_tag = IPP_TAG_ZERO;
    this->value_tag = IPP_TAG_ZERO;
}

void IPPAttribute::init(ipp_tag_t group_tag,
                        ipp_tag_t value_tag,
                        const char *name,
                        const vector<IPPValue> *list)
{
    this->group_tag = group_tag;
    this->value_tag = value_tag;
    if (list)
        this->values = *list;
    this->name = name;
}

//////////////////
// IPPAttribute // ATTRIBUTES
//////////////////

int IPPAttribute::getGroupTag(void)
{
    return this->group_tag;
}

int IPPAttribute::getValueTag(void)
{
    return this->value_tag;
}

string IPPAttribute::getName(void)
{
    return this->name;
}

vector<IPPValue> IPPAttribute::getValues(void)
{
    return this->values;
}

////////////////
// IPPRequest //
////////////////

IPPRequest::IPPRequest(ipp_op_t op)
{
    if (op == -1)
        this->ipp = ippNew();
    else
        this->ipp = ippNewRequest(op);
}

IPPRequest::~IPPRequest(void)
{
    if (this->ipp) {
        ippDelete(this->ipp);
    }
}

static IPPAttribute *build_IPPAttribute(ipp_attribute_t *attr)
{
    IPPValue value;
    vector<IPPValue> values;
    IPPAttribute *attribute = nullptr;
    const char *attrName = ippGetName(attr);

    debugprintf("%s: ", attrName);

    ipp_tag_t valTag = ippGetValueTag(attr);
    if (valTag == IPP_TAG_ZERO || valTag == IPP_TAG_NOVALUE || valTag == IPP_TAG_NOTSETTABLE || valTag == IPP_TAG_ADMINDEFINE) {
        debugprintf("no value\n");
    } else {
        int i;
        int unknown_value_tag = 0;

        for (i = 0; i < ippGetCount(attr); i++) {
            switch (valTag) {
            case IPP_TAG_INTEGER:
            case IPP_TAG_ENUM:
            case IPP_TAG_RANGE:
                value.i = ippGetInteger(attr, i);
                debugprintf("i%d", value.i);
                break;

            case IPP_TAG_BOOLEAN:
                value.b = ippGetBoolean(attr, i);
                debugprintf("b%d", value.b);
                break;

            case IPP_TAG_TEXT:
                // FIXME: UTF8?
                value.s = strdup(ippGetString(attr, i, nullptr));
                debugprintf("u%s", value.s);
                break;

            case IPP_TAG_NAME:
            case IPP_TAG_KEYWORD:
            case IPP_TAG_URI:
            case IPP_TAG_MIMETYPE:
            case IPP_TAG_CHARSET:
            case IPP_TAG_LANGUAGE:
                value.s = strdup(ippGetString(attr, i, nullptr));
                debugprintf("s%s", value.s);
                break;

            default:
                value.s = nullptr;
                unknown_value_tag = 1;
                debugprintf("Unable to encode value tag %d\n", valTag);
            }

            if (!value.s)
                break; /* out of values loop */

            values.push_back(value);
        }

        if (unknown_value_tag) {
            goto out;
        }

        debugprintf("\n");
    }

    attribute = new IPPAttribute;
    if (!attribute) {
        goto out;
    }

    attribute->init(ippGetGroupTag(attr), valTag, attrName, &values);

out:
    return attribute;
}

IPPAttribute *IPPRequest::addSeparator(void)
{
    ipp_attribute_t *attr = ippAddSeparator(this->ipp);
    return build_IPPAttribute(attr);
}

void IPPRequest::add(IPPAttribute *attribute)
{
    size_t num_values;
    void *values = nullptr;
    size_t value_size = 0;
    size_t i;

    num_values = attribute->values.size();
    switch (attribute->value_tag) {
    case IPP_TAG_INTEGER:
    case IPP_TAG_ENUM:
    case IPP_TAG_RANGE:
        value_size = sizeof(int);
        break;

    case IPP_TAG_BOOLEAN:
        value_size = sizeof(char);
        break;

    case IPP_TAG_NAME:
    case IPP_TAG_KEYWORD:
    case IPP_TAG_URI:
    case IPP_TAG_MIMETYPE:
    case IPP_TAG_CHARSET:
    case IPP_TAG_LANGUAGE:
        value_size = sizeof(char *);
        break;

    default:
        break;
    }

    values = calloc(num_values, value_size);
    if (!values) {
        throw runtime_error("Unable to allocate memory");
    }

    switch (attribute->value_tag) {
    case IPP_TAG_INTEGER:
    case IPP_TAG_ENUM:
    case IPP_TAG_RANGE:
        for (i = 0; i < num_values; i++) {
            ((int *)values)[i] = attribute->values[i].i;
        }
        ippAddIntegers(this->ipp,
                       attribute->group_tag,
                       attribute->value_tag,
                       (char *)attribute->name.data(),
                       num_values, (const int *)values);
        break;

    case IPP_TAG_BOOLEAN:
        for (i = 0; i < num_values; i++) {
            ((char *)values)[i] = attribute->values[i].b;
        }
        ippAddBooleans(this->ipp,
                       attribute->group_tag,
                       (char *)attribute->name.data(),
                       num_values, (const char *)values);
        break;

    case IPP_TAG_NAME:
    case IPP_TAG_KEYWORD:
    case IPP_TAG_URI:
    case IPP_TAG_MIMETYPE:
    case IPP_TAG_CHARSET:
    case IPP_TAG_LANGUAGE:
        for (i = 0; i < num_values; i++) {
            ((char **)values)[i] = (char *)attribute->values[i].s;
        }
        ippAddStrings(this->ipp,
                      attribute->group_tag,
                      attribute->value_tag,
                      (char *)attribute->name.data(),
                      num_values, nullptr, (const char **)values);
        break;

    default:
        break;
    }

    free(values);
}

static ssize_t
cupsipp_iocb_read(void *ctx, ipp_uchar_t *buffer, size_t len)
{
    (void)ctx;
    size_t got = -1;
    char *gotbuffer = nullptr;

    debugprintf("-> cupsipp_iocb_read\n");

    // TODO: len bytes here into gotbuffer

    if (got > len) {
        debugprintf("More data returned than requested!  Truncated...\n");
        got = len;
    }
    if (gotbuffer)
        memcpy(buffer, gotbuffer, got);

    debugprintf("<- cupsipp_iocb_read() == %zd\n", got);
    return got;
}

static ssize_t
cupsipp_iocb_write(void *ctx, ipp_uchar_t *buffer, size_t len)
{
    (void)ctx;
    (void)buffer;
    (void)len;
    size_t wrote = -1;

    debugprintf("-> cupsipp_iocb_write\n");

    // TODO: write len byte into buffer

    debugprintf("<- cupsipp_iocb_write()\n");

    return wrote;
}

int IPPRequest::readIO(void *ctx, bool blocking)
{
    ipp_state_t state;

    state = ippReadIO(ctx, (ipp_iocb_t)cupsipp_iocb_read,
                      blocking, nullptr, this->ipp);
    return state;
}

int IPPRequest::writeIO(void *ctx, bool blocking)
{
    ipp_state_t state;
    state = ippWriteIO(ctx, (ipp_iocb_t)cupsipp_iocb_write,
                       blocking, nullptr, this->ipp);
    return state;
}

/* Request properties */

vector<IPPAttribute *> IPPRequest::getAttributes(void)
{
    vector<IPPAttribute *> attrs;
    ipp_attribute_t *attr = nullptr;
    for (attr = ippFirstAttribute(this->ipp); attr;
         attr = ippNextAttribute(this->ipp)) {
        IPPAttribute *attribute = build_IPPAttribute(attr);
        if (!attribute)
            goto fail;

        attrs.push_back(attribute);
    }

fail:
    return attrs;
}

int IPPRequest::getOperation(void)
{
    return ippGetOperation(this->ipp);
}

int IPPRequest::getStatuscode(void)
{
    return ippGetStatusCode(this->ipp);
}

void IPPRequest::setState(ipp_state_t state)
{
    ippSetState(this->ipp, state);
}

int IPPRequest::getState(void)
{
    return ippGetState(this->ipp);
}

void IPPRequest::setStatuscode(ipp_status_t statuscode)
{
    ippSetStatusCode(this->ipp, statuscode);
}
