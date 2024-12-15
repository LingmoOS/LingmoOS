// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <stdarg.h>
#include <cups/cups.h>
#include <cups/language.h>

#include "cupsmodule.h"

#include <locale.h>
#include <pthread.h>
#include <wchar.h>
#include <wctype.h>

#include "cupsconnection.h"
#include "cupsppd.h"
#include "cupsipp.h"

#include <list>
#include <map>
#include <vector>
#include <string>
#include <stdexcept>

using namespace std;

static pthread_key_t tls_key = -1;
static pthread_once_t tls_key_once = PTHREAD_ONCE_INIT;

#if CUPS_VERSION_MAJOR > 1 || (CUPS_VERSION_MAJOR == 1 && CUPS_VERSION_MINOR < 3)
#define CUPS_PRINTER_DISCOVERED 0x1000000
#define CUPS_SERVER_REMOTE_ANY "_remote_any"
#endif /* CUPS < 1.3 */

#if CUPS_VERSION_MAJOR < 1 || (CUPS_VERSION_MAJOR == 1 && CUPS_VERSION_MINOR < 4)
#define HTTP_AUTHORIZATION_CANCELED 1000
#endif /* CUPS < 1.4 */

#if CUPS_VERSION_MAJOR < 1 || (CUPS_VERSION_MAJOR == 1 && CUPS_VERSION_MINOR < 5)
#define HTTP_PKI_ERROR 1001
#define IPP_AUTHENTICATION_CANCELED 0x1000
#define IPP_PKI_ERROR 0x1001
#endif /* CUPS < 1.5 */

#if HAVE_CUPS_1_6
#define CUPS_SERVER_REMOTE_PRINTERS "_remote_printers"
#endif /* CUPS > 1.6 */

//////////////////////
// Worker functions //
//////////////////////

static void destroy_TLS(void *value)
{
    struct TLS *tls = (struct TLS *)value;
    free(tls);
}

static void init_TLS(void)
{
    pthread_key_create(&tls_key, destroy_TLS);
}

struct TLS *get_TLS(void)
{
    struct TLS *tls = nullptr;
    pthread_once(&tls_key_once, init_TLS);
    tls = (struct TLS *)pthread_getspecific(tls_key);
    if (tls == nullptr) {
        tls = (struct TLS *)calloc(1, sizeof(struct TLS));
        pthread_setspecific(tls_key, tls);
    }

    return tls;
}

static int do_model_compare(const char *a, const char *b)
{
    const char *digits = "0123456789";
    char quick_a, quick_b;

    while ((quick_a = *a) != L'\0' && (quick_b = *b) != L'\0') {
        int end_a = strspn(a, digits);
        int end_b = strspn(b, digits);
        int min;
        int a_is_digit = 1;
        int cmp;

        if (quick_a != quick_b && !iswdigit(quick_a) && !iswdigit(quick_b)) {
            return (quick_a < quick_b) ? -1 : 1;
        }

        if (!end_a) {
            end_a = strcspn(a, digits);
            a_is_digit = 0;
        }

        if (!end_b) {
            if (a_is_digit)
                return -1;
            end_b = strcspn(b, digits);
        } else if (!a_is_digit) {
            return 1;
        }

        if (a_is_digit) {
            unsigned long n_a, n_b;
            n_a = strtoul(a, nullptr, 10);
            n_b = strtoul(b, nullptr, 10);
            if (n_a < n_b)
                cmp = -1;
            else if (n_a == n_b)
                cmp = 0;
            else
                cmp = 1;
        } else {
            min = end_a < end_b ? end_a : end_b;
            cmp = strncmp(a, b, min);
        }

        if (!cmp) {
            if (end_a != end_b)
                return end_a < end_b ? -1 : 1;

            a += end_a;
            b += end_b;
            continue;
        }

        return cmp;
    }

    if (quick_a == '\0') {
        if (*b == '\0')
            return 0;

        return -1;
    }

    return 1;
}

#ifndef HAVE_CUPS_1_6
int ippGetBoolean(ipp_attribute_t *attr, int element)
{
    return (attr->values[element].boolean);
}

int ippGetCount(ipp_attribute_t *attr)
{
    return (attr->num_values);
}

ipp_tag_t ippGetGroupTag(ipp_attribute_t *attr)
{
    return (attr->group_tag);
}

int ippGetInteger(ipp_attribute_t *attr, int element)
{
    return (attr->values[element].integer);
}

const char *ippGetName(ipp_attribute_t *attr)
{
    return (attr->name);
}

ipp_op_t ippGetOperation(ipp_t *ipp)
{
    return (ipp->request.op.operation_id);
}

int ippGetRange(ipp_attribute_t *attr, int element, int *uppervalue)
{
    if (uppervalue)
        *uppervalue = attr->values[element].range.upper;

    return (attr->values[element].range.lower);
}

int ippGetResolution(ipp_attribute_t *attr,
                     int element, int *yres, ipp_res_t *units)
{
    if (yres)
        *yres = attr->values[element].resolution.yres;

    if (units)
        *units = attr->values[element].resolution.units;

    return (attr->values[element].resolution.xres);
}

ipp_state_t ippGetState(ipp_t *ipp)
{
    return (ipp->state);
}

ipp_status_t ippGetStatusCode(ipp_t *ipp)
{
    return (ipp->request.status.status_code);
}

const char *ippGetString(ipp_attribute_t *attr,
                         int element, const char **language)
{
    return (attr->values[element].string.text);
}

ipp_tag_t ippGetValueTag(ipp_attribute_t *attr)
{
    return (attr->value_tag);
}

ipp_attribute_t *ippFirstAttribute(ipp_t *ipp)
{
    if (!ipp)
        return (nullptr);
    return (ipp->current = ipp->attrs);
}

ipp_attribute_t *ippNextAttribute(ipp_t *ipp)
{
    if (!ipp || !ipp->current)
        return (nullptr);
    return (ipp->current = ipp->current->next);
}

int ippSetInteger(ipp_t *ipp,
                  ipp_attribute_t **attr, int element, int intvalue)
{
    (*attr)->values[element].integer = intvalue;
    return (1);
}

int ippSetOperation(ipp_t *ipp, ipp_op_t op)
{
    ipp->request.op.operation_id = op;
    return (1);
}

int ippSetState(ipp_t *ipp, ipp_state_t state)
{
    ipp->state = state;
    return (1);
}

int ippSetStatusCode(ipp_t *ipp, ipp_status_t status)
{
    ipp->request.status.status_code = status;
    return (1);
}

int ippSetString(ipp_t *ipp,
                 ipp_attribute_t **attr, int element, const char *strvalue)
{
    (*attr)->values[element].string.text = (char *)strvalue;
    return (1);
}

#endif

//////////////////////////
// Module-level methods //
//////////////////////////

int cups_modelSort(const char *str, const char *str2)
{
    return do_model_compare(str, str2);
}

void cups_setUser(const char *user)
{
    cupsSetUser(user);
}

void cups_setServer(const char *server)
{
    cupsSetServer(server);
}

void cups_setPort(int port)
{
    ippSetPort(port);
}

void cups_setEncryption(int e)
{
    cupsSetEncryption((http_encryption_t)e);
}

string cups_getUser(void)
{
    return cupsUser();
}

string cups_getServer(void)
{
    return cupsServer();
}

int cups_getPort(void)
{
    return ippPort();
}

int cups_getEncryption(void)
{
    return cupsEncryption();
}

void cups_setPasswordCB(CUPS_PASSWD_CALLBACK cb)
{
    struct TLS *tls = get_TLS();

    debugprintf("-> cups_setPasswordCB\n");
#ifdef HAVE_CUPS_1_4
    tls->cups_password_callback_context = nullptr;
#endif /* HAVE_CUPS_1_4 */

    tls->cups_password_callback = cb;

#ifdef HAVE_CUPS_1_4
    cupsSetPasswordCB2(password_callback_oldstyle, nullptr);
#else
    cupsSetPasswordCB(do_password_callback);
#endif

    debugprintf("<- cups_setPasswordCB\n");
}

#ifdef HAVE_CUPS_1_4
void cups_setPasswordCB2(CUPS_PASSWD_CALLBACK cb, void *cb_context)
{
    struct TLS *tls = get_TLS();

    debugprintf("-> cups_setPasswordCB2\n");

    tls->cups_password_callback_context = cb_context;

    if (!cb) {
        tls->cups_password_callback = nullptr;
        cupsSetPasswordCB2(nullptr, nullptr);
    } else {
        tls->cups_password_callback = cb;
        cupsSetPasswordCB2(password_callback_newstyle, cb_context);
    }

    debugprintf("<- cups_setPasswordCB2\n");
}
#endif /* HAVE_CUPS_1_4 */

void cups_ppdSetConformance(int level)
{
    ppdSetConformance((ppd_conform_t)level);
}

#ifdef HAVE_CUPS_1_6
void cups_enumDests(void *cb, int flags, int msec, int type, int mask, void *user_data)
{
    CallbackContext context;
    int ret;

    context.cb = cb;
    context.user_data = user_data;
    ret = cupsEnumDests(flags,
                        msec, nullptr, type, mask, cups_dest_cb, &context);

    if (!ret) {
        throw runtime_error("cupsEnumDests failed");
    }
}
#endif /* HAVE_CUPS_1_6 */

#ifdef HAVE_CUPS_1_6
Connection *cups_connectDest(const Dest *dest_o, void *cb, int flags, int msec, void *user_data)
{
    char resource[HTTP_MAX_URI];
    CallbackContext context;
    http_t *conn = nullptr;
    Connection *connobj = nullptr;
    cups_dest_t dest;

    context.cb = cb;
    context.user_data = user_data;
    resource[0] = '\0';

    dest.is_default = dest_o->is_default;
    dest.name = (char *)dest_o->destname.data();
    dest.instance = (char *)dest_o->instance.data();
    dest.num_options = dest_o->options.size();
    dest.options = (cups_option_t *)malloc(dest.num_options * sizeof(dest.options[0]));
    int i = 0;
    for (const auto &it : dest_o->options) {
        dest.options[i].name = (char *)it.first.data();
        dest.options[i].value = (char *)it.second.data();
        ++i;
    }

    conn = cupsConnectDest(&dest,
                           flags,
                           msec,
                           nullptr,
                           resource,
                           sizeof(resource), cups_dest_cb, &context);
    free(dest.options);

    if (!conn) {
        string err = get_ipp_error(cupsLastError(), cupsLastErrorString());
        throw runtime_error(err);
    }

    connobj = (Connection *)malloc(sizeof(Connection));
    connobj->http = conn;
    connobj->resource = resource;
    // TODO: connobj->init(host, port, encryption);

    return connobj;
}
#endif /* HAVE_CUPS_1_6 */

string cups_ippErrorString(int op)
{
    return ippErrorString((ipp_status_t)op);
}

string cups_ippOpString(int op)
{
    return ippOpString((ipp_op_t)op);
}

///////////////
// Debugging //
///////////////

#define ENVAR "PYCUPS_DEBUG"
static int debugging_enabled = -1;

void debugprintf(const char *fmt, ...)
{
    if (!debugging_enabled)
        return;

    if (debugging_enabled == -1) {
        if (!getenv(ENVAR)) {
            debugging_enabled = 0;
            return;
        }

        debugging_enabled = 1;
    }

    {
        va_list ap;
        va_start(ap, fmt);
        vfprintf(stderr, fmt, ap);
        va_end(ap);
    }
}

std::string string_format(const char *fmt, ...)
{
    std::string str;
    size_t size = 0;
    va_list ap;
    va_start(ap, fmt);
    size = vsnprintf(nullptr, 0, fmt, ap) + 1;
    va_end(ap);

    str.resize(size, 0);

    va_start(ap, fmt);
    vsnprintf((char *)str.data(), size, fmt, ap);
    va_end(ap);

    str.erase(size);

    return str;
}
