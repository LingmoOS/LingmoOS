// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef HAVE_CUPSIPP_H
#define HAVE_CUPSIPP_H

#include <cups/ipp.h>
#include <string>
#include <vector>

union IPPValue {
    bool b;
    int i;
    const char *s;
};

class IPPAttribute
{
public:
    IPPAttribute(void);
    ~IPPAttribute(void);
    void init(ipp_tag_t group_tag,
              ipp_tag_t value_tag,
              const char *name,
              const std::vector<IPPValue> *list);
    int getGroupTag(void);
    int getValueTag(void);
    std::string getName(void);
    std::vector<IPPValue> getValues(void);

    ipp_tag_t group_tag;
    ipp_tag_t value_tag;
    std::string name;
    std::vector<IPPValue> values;
};

class IPPRequest
{
public:
    IPPRequest(ipp_op_t op);
    ~IPPRequest(void);
    IPPAttribute *addSeparator(void);
    void add(IPPAttribute *attribute);
    int readIO(void *ctx, bool blocking);
    int writeIO(void *ctx, bool blocking);
    std::vector<IPPAttribute *> getAttributes(void);
    int getOperation(void);
    int getStatuscode(void);
    void setState(ipp_state_t state);
    int getState(void);
    void setStatuscode(ipp_status_t statuscode);

private:
    ipp_t *ipp;
};

#endif /* HAVE_CUPSIPP_H */
