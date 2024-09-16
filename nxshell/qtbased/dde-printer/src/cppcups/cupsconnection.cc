// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "cupsconnection.h"

#ifndef __SVR4
#include <paths.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <linux/limits.h> // PATH_MAX=4096

#include <mutex>

#ifndef _PATH_TMP
#define _PATH_TMP P_tmpdir
#endif

#define MAX_CONN 100

static std::mutex g_lock;
static Connection **g_connections = nullptr;
static int g_numConnections = 0;

#define checkIppAnswer(answer, msg) \
    do { \
        if (!answer || ippGetStatusCode(answer) > IPP_OK_CONFLICT) { \
            string err = get_ipp_error(answer ? ippGetStatusCode(answer) : cupsLastError(), \
                                       answer ? nullptr : cupsLastErrorString()); \
            if (answer) { \
                ippDelete(answer); \
            } \
            if (msg) \
                debugprintf(msg); \
            throw runtime_error(err); \
        } \
    } while (0);

static void set_http_error(http_status_t status)
{
    debugprintf("set_http_error: %d\n", (int)status);
    throw runtime_error(string_format("http error(%d)", status));
}

string get_ipp_error(ipp_status_t status, const char *message)
{
    if (!message)
        message = ippErrorString(status);

    return string_format("ipp error(%d, %s)", status, message);
}

static void construct_uri(char *buffer, size_t buflen, const char *base, const char *value)
{
    char *d = buffer;
    const unsigned char *s = (const unsigned char *)value;
    if (strlen(base) < buflen) {
        strcpy(buffer, base);
        d += strlen(base);
    } else {
        strncpy(buffer, base, buflen);
        d += buflen;
    }

    while (*s && d < buffer + buflen) {
        if (isalpha(*s) || isdigit(*s) || *s == '-')
            *d++ = *s++;
        else if (*s == ' ') {
            *d++ = '+';
            s++;
        } else {
            if (d + 2 < buffer + buflen) {
                *d++ = '%';
                *d++ = "0123456789ABCDEF"[((*s) & 0xf0) >> 4];
                *d++ = "0123456789ABCDEF"[((*s) & 0x0f)];
                s++;
            } else {
                break;
            }
        }
    }

    if (d < buffer + buflen)
        *d = '\0';
}

static int copy_dest(Dest *dst, cups_dest_t *src)
{
    int i;
    dst->is_default = src->is_default;
    dst->destname = src->name;
    dst->instance = (src->instance ? src->instance : "");
    dst->options.clear();
    for (i = 0; i < src->num_options; i++) {
        dst->options[src->options[i].name] = src->options[i].value;
    }

    return 0;
}

#ifdef HAVE_CUPS_1_6
int cups_dest_cb(void *user_data, unsigned flags, cups_dest_t *dest)
{
    Dest destobj;
    int ret = 0;
    (void)user_data;
    (void)flags;

    debugprintf("-> cups_dest_cb\n");
    copy_dest(&destobj, dest);
    // TODO: callback user function!
    debugprintf("<- cups_dest_cb (%d)\n", ret);

    // return 1 to continue enumeration and 0 to cancel.
    return ret;
}
#endif /* HAVE_CUPS_1_6 */

string string_from_attr_value(ipp_attribute_t *attr, int i)
{
    int lower, upper;
    int xres, yres;
    ipp_res_t units;
    string val;
    switch (ippGetValueTag(attr)) {
    case IPP_TAG_NAME:
    case IPP_TAG_TEXT:
    case IPP_TAG_KEYWORD:
    case IPP_TAG_URI:
    case IPP_TAG_CHARSET:
    case IPP_TAG_MIMETYPE:
    case IPP_TAG_LANGUAGE:
        val = string_format("s%s", ippGetString(attr, i, nullptr));
        break;
    case IPP_TAG_INTEGER:
    case IPP_TAG_ENUM:
        val = string_format("i%d", ippGetInteger(attr, i));
        break;
    case IPP_TAG_BOOLEAN:
        val = string_format("b%d", ippGetBoolean(attr, i));
        break;
    case IPP_TAG_RANGE:
        lower = ippGetRange(attr, i, &upper);
        val = string_format("i%d,%d", lower, upper);
        break;
    case IPP_TAG_NOVALUE:
        break;

    case IPP_TAG_DATE:
        // TODO: parse IPP_TAG_DATE
        val = string_format("s%s", "(IPP_TAG_DATE)");
        break;
    case IPP_TAG_RESOLUTION:
        xres = ippGetResolution(attr, i, &yres, &units);
        val = string_format("i%d,%d,%d", xres, yres, units);
        break;
    default:
        val = string_format("s(unknown IPP value tag 0x%x)", ippGetValueTag(attr));
        break;
    }

    return val;
}

string list_from_attr_values(ipp_attribute_t *attr)
{
    string list;
    int i;
    debugprintf("-> list_from_attr_values()\n");
    for (i = 0; i < ippGetCount(attr); i++) {
        string val = string_from_attr_value(attr, i);
        if (!val.empty()) {
            list += '`'; // each sub string separate with ` char
            list += val;
        }
    }

    debugprintf("<- list_from_attr_values()\n");
    return list;
}

////////////////
// Connection //
////////////////

#ifdef HAVE_CUPS_1_4
static const char *password_callback(int newstyle,
                                     const char *prompt,
                                     http_t *http,
                                     const char *method,
                                     const char *resource,
                                     void *user_data)
{
    struct TLS *tls = get_TLS();
    Connection *self = nullptr;
    std::lock_guard<std::mutex> guard(g_lock);

    debugprintf("-> password_callback for http=%p, newstyle=%d\n",
                http, newstyle);
    for (int i = 0; i < g_numConnections; i++) {
        if (g_connections[i]->http == http) {
            self = g_connections[i];
            break;
        }
    }

    if (!self) {
        debugprintf("cannot find self!\n");
        return "";
    }

    // STORE password in slef->cb_password
    tls->cups_password_callback(prompt, self, method, resource, user_data);
    if (self->cb_password.empty()) {
        debugprintf("<- password_callback (empty/null)\n");
        return nullptr;
    }

    debugprintf("<- password_callback\n");

    return self->cb_password.data();
}

const char *password_callback_oldstyle(const char *prompt,
                                       http_t *http,
                                       const char *method,
                                       const char *resource,
                                       void *user_data)
{
    return password_callback(0, prompt, http, method, resource, user_data);
}

const char *password_callback_newstyle(const char *prompt,
                                       http_t *http,
                                       const char *method,
                                       const char *resource,
                                       void *user_data)
{
    return password_callback(1, prompt, http, method, resource, user_data);
}
#endif /* !HAVE_CUPS_1_4 */

ServerSettings::ServerSettings()
{
}

ServerSettings &ServerSettings::enableDebugLogging(bool enabled)
{
    settings_[CUPS_SERVER_DEBUG_LOGGING] = enabled ? "1" : "0";
    return *this;
}

ServerSettings &ServerSettings::enableRemoteAdmin(bool enabled)
{
    settings_[CUPS_SERVER_REMOTE_ADMIN] = enabled ? "1" : "0";
    return *this;
}

ServerSettings &ServerSettings::enableRemoteAny(bool enabled)
{

    settings_[CUPS_SERVER_REMOTE_ANY] = enabled ? "1" : "0";
    return *this;
}

ServerSettings &ServerSettings::enableSharePrinters(bool enabled)
{
    settings_[CUPS_SERVER_SHARE_PRINTERS] = enabled ? "1" : "0";
    return *this;
}

ServerSettings &ServerSettings::enableUserCancelAny(bool enabled)
{
    settings_[CUPS_SERVER_USER_CANCEL_ANY] = enabled ? "1" : "0";
    return *this;
}

bool ServerSettings::isDebugLoggingEnabled() const
{
    return atoi(settings_.at(CUPS_SERVER_DEBUG_LOGGING).c_str()) == 1;
}

bool ServerSettings::isRemoteAdminEnabled() const
{
    return atoi(settings_.at(CUPS_SERVER_REMOTE_ADMIN).c_str()) == 1;
}

bool ServerSettings::isRemoteAnyEnabled() const
{
    return atoi(settings_.at(CUPS_SERVER_REMOTE_ANY).c_str()) == 1;
}

bool ServerSettings::isSharePrintersEnabled() const
{
    return atoi(settings_.at(CUPS_SERVER_SHARE_PRINTERS).c_str()) == 1;
}

bool ServerSettings::isUserCancelAnyEnabled() const
{
    return atoi(settings_.at(CUPS_SERVER_USER_CANCEL_ANY).c_str()) == 1;
}

void ServerSettings::updateSettings(map<string, string> set)
{
    if (!set.empty())
        std::swap(set, settings_);
}

void ServerSettings::commit(const char *host_str, int port_n, int encryption_n, const map<string, string> &options)
{

    Connection c;
    if (0 != c.init(host_str, port_n, encryption_n)) {
        return;
    }
    c.adminSetServerSettings(&options);
}

Connection::Connection(void)
{
    http = nullptr;
}

Connection::~Connection(void)
{
    int i, j;
    std::lock_guard<std::mutex> guard(g_lock);

    for (j = 0; j < g_numConnections; j++) {
        if (g_connections[j] == this)
            break;
    }

    if (j < g_numConnections) {
        if (g_numConnections > 1) {
            Connection **new_array = (Connection **)calloc(g_numConnections - 1, sizeof(Connection *));

            if (new_array) {
                int k;
                for (i = 0, k = 0; i < g_numConnections; i++) {
                    if (i == j)
                        continue;

                    new_array[k++] = g_connections[i];
                }

                free(g_connections);
                g_connections = new_array;
                g_numConnections--;
            } else {
                /* Failed to allocate memory. Just clear out the reference. */
                g_connections[j] = nullptr;
            }
        } else {
            /* The only element is the one we no longer need. */
            free(g_connections);
            g_connections = nullptr;
            g_numConnections = 0;
        }
    }

    if (this->http) {
        debugprintf("httpClose()\n");
        httpClose(this->http);
    }
}

int Connection::init(const char *host_str, int port_n, int encryption_n)
{
    const char *host = host_str ? host_str : cupsServer();
    int port = port_n ? port_n : ippPort();
    int encryption = encryption_n ? encryption_n : (http_encryption_t)cupsEncryption();

    debugprintf("-> Connection::init(host=%s, port=%d, enc=%d)\n", host, port, encryption);
    this->host = host;
    if (this->host.empty()) {
        debugprintf("<- Connection::init() = -1\n");
        return -1;
    }

    debugprintf("httpConnectEncrypt(...)\n");
    int cancel = 0;
    this->http = httpConnect2(host, port, nullptr, AF_UNSPEC,
                              (http_encryption_t)encryption, 1, 30000, &cancel);
    if (!this->http) {
        debugprintf("<- Connection::init() = -1\n");
        throw runtime_error("failed to connect to server");
    }

    {
        std::lock_guard<std::mutex> guard(g_lock);
        if (g_numConnections == 0) {
            g_connections = (Connection **)malloc(sizeof(Connection *));
            if (g_connections == nullptr) {
                debugprintf("<- Connection::init() = -1\n");
                throw runtime_error("insufficient memory");
            }
        } else {
            Connection **old_array = g_connections;

            if ((1 + g_numConnections) >= MAX_CONN) {
                debugprintf("<- Connection::init() == -1\n");
                throw runtime_error("too many connections");
            }

            g_connections = (Connection **)realloc(g_connections, (1 + g_numConnections) * sizeof(Connection *));
            if (g_connections == nullptr) {
                g_connections = old_array;
                debugprintf("<- Connection::init() = -1\n");
                throw runtime_error("insufficient memory");
            }
        }

        g_connections[g_numConnections++] = this;
    }

    debugprintf("<- Connection::init() = 0\n");
    return 0;
}

vector<Dest> Connection::getDests(void)
{
    cups_dest_t *dests = nullptr;
    int num_dests;

    debugprintf("-> Connection::getDests()\n");
    debugprintf("cupsGetDests2()\n");
    num_dests = cupsGetDests2(this->http, &dests);

    // Create a dict indexed by(name,instance)
    Dest destobj;
    vector<Dest> ret;
    for (int i = 0; i <= num_dests; i++) {
        cups_dest_t *dest = nullptr;
        if (i == num_dests) {
            // Add a(None,None) entry for the default printer.
            dest = cupsGetDest(nullptr, nullptr, num_dests, dests);
            if (dest == nullptr) {
                /* No default printer. */
                break;
            }
        } else {
            dest = dests + i;
            copy_dest(&destobj, dest);
            ret.push_back(destobj);
        }
    }

    debugprintf("cupsFreeDests()\n");
    cupsFreeDests(num_dests, dests);
    debugprintf("<- Connection::getDests()\n");

    return ret;
}

map<string, map<string, string>> Connection::getPrinters(void)
{
    ipp_t *request = ippNewRequest(CUPS_GET_PRINTERS);
    ipp_t *answer = nullptr;
    ipp_attribute_t *attr = nullptr;
    const char *attributes[] = {
        "printer-name",
        "printer-type",
        "printer-location",
        "printer-info",
        "printer-make-and-model",
        "printer-state",
        "printer-state-message",
        "printer-state-reasons",
        "printer-uri-supported",
        "device-uri",
        "printer-is-shared",
    };
    map<string, map<string, string>> ret;

    debugprintf("-> Connection::getPrinters()\n");

    ippAddStrings(request, IPP_TAG_OPERATION, IPP_TAG_KEYWORD,
                  "requested-attributes",
                  sizeof(attributes) / sizeof(attributes[0]),
                  nullptr, attributes);
    debugprintf("cupsDoRequest(\"/\")\n");
    answer = cupsDoRequest(this->http, request, "/");
    if (answer && ippGetStatusCode(answer) == IPP_NOT_FOUND) {
        // No printers.
        debugprintf("<- Connection::getPrinters() = {}(no printers)\n");
        ippDelete(answer);
        return ret;
    }

    checkIppAnswer(answer, "<- Connection::getPrinters()(error)\n");

    for (attr = ippFirstAttribute(answer); attr; attr = ippNextAttribute(answer)) {
        const char *printer = nullptr;

        while (attr && ippGetGroupTag(attr) != IPP_TAG_PRINTER) {
            attr = ippNextAttribute(answer);
        }

        if (!attr) {
            break;
        }

        map<string, string> dict;
        for (; attr && ippGetGroupTag(attr) == IPP_TAG_PRINTER; attr = ippNextAttribute(answer)) {
            const char *attrName = ippGetName(attr);
            ipp_tag_t valTag = ippGetValueTag(attr);
            debugprintf("Attribute: %s\n", attrName);

            string val;
            if (!strcmp(attrName, "printer-name") && valTag == IPP_TAG_NAME) {
                printer = ippGetString(attr, 0, nullptr);
            } else if ((!strcmp(attrName, "printer-type") || !strcmp(attrName, "printer-state")) && valTag == IPP_TAG_ENUM) {
                val = string_format("i%d", ippGetInteger(attr, 0));
            } else if ((!strcmp(attrName, "printer-make-and-model") || !strcmp(attrName, "printer-info") || !strcmp(attrName, "printer-location") || !strcmp(attrName, "printer-state-message")) && valTag == IPP_TAG_TEXT) {
                val = string_format("s%s", ippGetString(attr, 0, nullptr));
            } else if (!strcmp(attrName, "printer-state-reasons") && valTag == IPP_TAG_KEYWORD) {
                val = list_from_attr_values(attr);
            } else if (!strcmp(attrName, "printer-is-accepting-jobs") && valTag == IPP_TAG_BOOLEAN) {
                val = string_format("b%d", ippGetBoolean(attr, 0));
            } else if ((!strcmp(attrName, "printer-up-time") || !strcmp(attrName, "queued-job-count")) && valTag == IPP_TAG_INTEGER) {
                val = string_format("i%d", ippGetInteger(attr, 0));
            } else if ((!strcmp(attrName, "device-uri") || !strcmp(attrName, "printer-uri-supported")) && valTag == IPP_TAG_URI) {
                val = string_format("s%s", ippGetString(attr, 0, nullptr));
            } else if (!strcmp(attrName, "printer-is-shared") && valTag == IPP_TAG_BOOLEAN) {
                val = string_format("b%d", ippGetBoolean(attr, 0));
            }

            if (!val.empty()) {
                debugprintf("Added %s to dict\n", attrName);
                dict[attrName] = val;
            }
        }

        if (printer) {
            ret[printer] = dict;
        }

        if (!attr) {
            break;
        }
    }

    ippDelete(answer);
    debugprintf("<- Connection::getPrinters() = dict\n");

    return ret;
}

map<string, string> Connection::getClasses(void)
{
    ipp_t *request = ippNewRequest(CUPS_GET_CLASSES);
    ipp_t *answer = nullptr;
    ipp_attribute_t *attr = nullptr;
    const char *attributes[] = {
        "printer-name",
        "member-names",
    };
    map<string, string> ret;

    debugprintf("-> Connection::getClasses()\n");
    ippAddStrings(request, IPP_TAG_OPERATION, IPP_TAG_KEYWORD,
                  "requested-attributes",
                  sizeof(attributes) / sizeof(attributes[0]),
                  nullptr, attributes);
    debugprintf("cupsDoRequest(\"/\")\n");
    answer = cupsDoRequest(this->http, request, "/");
    if (!answer || ippGetStatusCode(answer) > IPP_OK_CONFLICT) {
        if (answer && ippGetStatusCode(answer) == IPP_NOT_FOUND) {
            // No classes.
            debugprintf("<- Connection::getClasses() = {}(no classes)\n");
            ippDelete(answer);
            return ret;
        }

        string err = get_ipp_error(answer ? ippGetStatusCode(answer) : cupsLastError(),
                                   answer ? nullptr : cupsLastErrorString());
        if (answer) {
            ippDelete(answer);
        }
        debugprintf("<- Connection::getClasses()(error)\n");

        throw runtime_error(err);
    }

    for (attr = ippFirstAttribute(answer); attr; attr = ippNextAttribute(answer)) {
        char *classname = nullptr;
        char *printer_uri = nullptr;
        string members;

        while (attr && ippGetGroupTag(attr) != IPP_TAG_PRINTER) {
            attr = ippNextAttribute(answer);
        }

        if (!attr) {
            break;
        }

        for (; attr && ippGetGroupTag(attr) == IPP_TAG_PRINTER; attr = ippNextAttribute(answer)) {
            const char *attrName = ippGetName(attr);
            ipp_tag_t valTag = ippGetValueTag(attr);
            debugprintf("Attribute: %s\n", attrName);
            if (!strcmp(attrName, "printer-name") && valTag == IPP_TAG_NAME)
                classname = (char *)ippGetString(attr, 0, nullptr);
            else if (!strcmp(attrName, "printer-uri-supported") && valTag == IPP_TAG_URI)
                printer_uri = (char *)ippGetString(attr, 0, nullptr);
            else if (!strcmp(attrName, "member-names") && valTag == IPP_TAG_NAME) {
                members = list_from_attr_values(attr);
            }
        }

        if (printer_uri) {
            members = printer_uri;
        }

        if (classname) {
            debugprintf("Added class %s\n", classname);
            ret[classname] = members;
        }

        if (!attr) {
            break;
        }
    }

    ippDelete(answer);
    debugprintf("<- Connection::getClasses() = dict\n");

    return ret;
}

map<string, map<string, string>>
                              Connection::do_getPPDs(int limit,
                                                     const vector<string> *exclude_schemes,
                                                     const vector<string> *include_schemes,
                                                     const char *ppd_natural_language,
                                                     const char *ppd_device_id,
                                                     const char *ppd_make,
                                                     const char *ppd_make_and_model,
                                                     int ppd_model_number,
                                                     const char *ppd_product,
                                                     const char *ppd_psversion,
                                                     const char *ppd_type,
                                                     bool all_lists)
{
    ipp_t *request = nullptr, *answer = nullptr;
    ipp_attribute_t *attr = nullptr;

    request = ippNewRequest(CUPS_GET_PPDS);
    if (limit > 0) {
        ippAddInteger(request, IPP_TAG_OPERATION, IPP_TAG_INTEGER,
                      "limit", limit);
    }

    if (exclude_schemes) {
        size_t i, n;
        char **ss = nullptr;

        n = exclude_schemes->size();
        ss = (char **)calloc(n + 1, sizeof(char *));
        for (i = 0; i < n; i++) {
            ss[i] = (char *)exclude_schemes->at(i).data();
        }
        ss[n] = nullptr;
        ippAddStrings(request, IPP_TAG_OPERATION, IPP_TAG_NAME,
                      "exclude-schemes", n, nullptr, (const char **)ss);
        free(ss);
    }

    if (include_schemes) {
        size_t i, n;
        char **ss = nullptr;
        n = include_schemes->size();
        ss = (char **)calloc(n + 1, sizeof(char *));
        for (i = 0; i < n; i++) {
            ss[i] = (char *)include_schemes->at(i).data();
        }
        ss[n] = nullptr;
        ippAddStrings(request, IPP_TAG_OPERATION, IPP_TAG_NAME,
                      "include-schemes", n, nullptr, (const char **)ss);
        free(ss);
    }

    if (ppd_device_id) {
        ippAddString(request, IPP_TAG_OPERATION, IPP_TAG_TEXT,
                     "ppd-device-id", nullptr, ppd_device_id);
    }

    if (ppd_make) {
        ippAddString(request, IPP_TAG_OPERATION, IPP_TAG_TEXT,
                     "ppd-make", nullptr, ppd_make);
    }

    if (ppd_make_and_model) {
        ippAddString(request, IPP_TAG_OPERATION, IPP_TAG_TEXT,
                     "ppd-make-and-model", nullptr, ppd_make_and_model);
    }

    if (ppd_model_number >= 0) {
        ippAddInteger(request, IPP_TAG_OPERATION, IPP_TAG_INTEGER,
                      "ppd-model-number", ppd_model_number);
    }

    if (ppd_product) {
        ippAddString(request, IPP_TAG_OPERATION, IPP_TAG_TEXT,
                     "ppd-product", nullptr, ppd_product);
    }

    if (ppd_psversion) {
        ippAddString(request, IPP_TAG_OPERATION, IPP_TAG_TEXT,
                     "ppd-psversion", nullptr, ppd_psversion);
    }

    if (ppd_natural_language) {
        ippAddString(request, IPP_TAG_OPERATION, IPP_TAG_LANGUAGE,
                     "ppd-natural-language", nullptr, ppd_natural_language);
    }

    if (ppd_type) {
        ippAddString(request, IPP_TAG_OPERATION, IPP_TAG_KEYWORD,
                     "ppd-type", nullptr, ppd_type);
    }

    debugprintf("-> Connection::getPPDs()\n");
    debugprintf("cupsDoRequest(\"/\")\n");
    answer = cupsDoRequest(this->http, request, "/");
    checkIppAnswer(answer, "<- Connection::getPPDs()(error)\n");

    map<string, map<string, string>> ret;
    for (attr = ippFirstAttribute(answer); attr; attr = ippNextAttribute(answer)) {
        char *ppdname = nullptr;

        while (attr && ippGetGroupTag(attr) != IPP_TAG_PRINTER) {
            attr = ippNextAttribute(answer);
        }

        if (!attr) {
            break;
        }

        map<string, string> dict;
        for (; attr && ippGetGroupTag(attr) == IPP_TAG_PRINTER;
                attr = ippNextAttribute(answer)) {
            const char *attrName = ippGetName(attr);
            debugprintf("Attribute: %s\n", attrName);
            if (!strcmp(attrName, "ppd-name") && ippGetValueTag(attr) == IPP_TAG_NAME) {
                ppdname = (char *)ippGetString(attr, 0, nullptr);
            } else {
                string val;
                if (all_lists)
                    val = list_from_attr_values(attr);
                else
                    val = string_from_attr_value(attr, 0);

                if (!val.empty()) {
                    debugprintf("Adding %s to ppd dict\n", attrName);
                    dict[attrName] = val;
                }
            }
        }

        if (ppdname) {
            debugprintf("Adding %s to result dict\n", ppdname);
            ret[ppdname] = dict;
        }

        if (!attr) {
            break;
        }
    }

    ippDelete(answer);
    debugprintf("<- Connection::getPPDs() = dict\n");

    return ret;
}

map<string, map<string, string>> Connection::getPPDs(int limit,
                                                     const vector<string> *exclude_schemes,
                                                     const vector<string> *include_schemes,
                                                     const char *ppd_natural_language,
                                                     const char *ppd_device_id,
                                                     const char *ppd_make,
                                                     const char *ppd_make_and_model,
                                                     int ppd_model_number,
                                                     const char *ppd_product,
                                                     const char *ppd_psversion,
                                                     const char *ppd_type)
{
    return do_getPPDs(limit, exclude_schemes, include_schemes,
                      ppd_natural_language, ppd_device_id, ppd_make, ppd_make_and_model,
                      ppd_model_number, ppd_product, ppd_psversion, ppd_type, false);
}

map<string, map<string, string>> Connection::getPPDs2(int limit,
                                                      const vector<string> *exclude_schemes,
                                                      const vector<string> *include_schemes,
                                                      const char *ppd_natural_language,
                                                      const char *ppd_device_id,
                                                      const char *ppd_make,
                                                      const char *ppd_make_and_model,
                                                      int ppd_model_number,
                                                      const char *ppd_product,
                                                      const char *ppd_psversion,
                                                      const char *ppd_type)
{
    return do_getPPDs(limit, exclude_schemes, include_schemes,
                      ppd_natural_language, ppd_device_id, ppd_make, ppd_make_and_model,
                      ppd_model_number, ppd_product, ppd_psversion, ppd_type, true);
}

string Connection::getServerPPD(const char *ppd_name)
{
#if CUPS_VERSION_MAJOR > 1 || (CUPS_VERSION_MAJOR == 1 && CUPS_VERSION_MINOR >= 3)
    const char *filename = nullptr;
    if (!ppd_name)
        return string();

    debugprintf("-> Connection::getServerPPD()\n");
    filename = cupsGetServerPPD(this->http, ppd_name);
    if (!filename) {
        string err = get_ipp_error(cupsLastError(), cupsLastErrorString());
        debugprintf("<- Connection::getServerPPD()(error)\n");
        throw runtime_error(err);
    }
    debugprintf("<- Connection::getServerPPD(\"%s\") = \"%s\"\n",
                ppd_name, filename);
    return filename;
#else /* earlier than CUPS 1.3 */
    throw runtime_error("Operation not supported - recompile against CUPS 1.3 or later");
#endif /* CUPS 1.3 */
}

map<string, string> Connection::getDocument(const char *uri, int jobid, int docnum)
{
#if CUPS_VERSION_MAJOR > 1 || (CUPS_VERSION_MAJOR == 1 && CUPS_VERSION_MINOR >= 4)
    ipp_t *request = nullptr, *answer = nullptr;
    ipp_attribute_t *attr = nullptr;
    const char *format = nullptr;
    const char *name = nullptr;
    char docfilename[PATH_MAX];
    int fd;
    map<string, string> dict;

    if (!uri)
        return dict;

    debugprintf("-> Connection::getDocument(\"%s\",%d)\n", uri, jobid);
    request = ippNewRequest(CUPS_GET_DOCUMENT);
    ippAddString(request, IPP_TAG_OPERATION, IPP_TAG_URI,
                 "printer-uri", nullptr, uri);
    ippAddInteger(request, IPP_TAG_OPERATION, IPP_TAG_INTEGER,
                  "job-id", jobid);
    ippAddInteger(request, IPP_TAG_OPERATION, IPP_TAG_INTEGER,
                  "document-number", docnum);

    snprintf(docfilename, sizeof(docfilename), "%s/jobdoc-XXXXXX", _PATH_TMP);
    fd = mkstemp(docfilename);
    if (fd < 0) {
        debugprintf("<- Connection::getDocument() EXCEPTION\n");
        ippDelete(request);
        throw runtime_error(string_format("failed to create %s, error=%d", docfilename, errno));
    }

    answer = cupsDoIORequest(this->http, request, "/", -1, fd);

    close(fd);
    if (!answer || ippGetStatusCode(answer) > IPP_OK_CONFLICT) {
        unlink(docfilename);
        string err = get_ipp_error(answer ? ippGetStatusCode(answer) : cupsLastError(),
                                   answer ? nullptr : cupsLastErrorString());
        if (answer) {
            ippDelete(answer);
        }
        debugprintf("<- Connection::getDocument()(error)\n");
        throw runtime_error(err);
    }

    if ((attr = ippFindAttribute(answer, "document-format",
                                 IPP_TAG_MIMETYPE))
            != nullptr) {
        format = ippGetString(attr, 0, nullptr);
    }

    if ((attr = ippFindAttribute(answer, "document-name",
                                 IPP_TAG_NAME))
            != nullptr) {
        name = ippGetString(attr, 0, nullptr);
    }

    dict["file"] = docfilename;

    if (format) {
        dict["document-format"] = format;
    }

    if (name) {
        dict["document-name"] = name;
    }

    debugprintf("<- Connection::getDocument() = {'file':\"%s\","
                "'document-format':\"%s\",'document-name':\"%s\"}\n",
                docfilename, format ? format : "(nul)",
                name ? name : "(nul)");
    ippDelete(answer);

    return dict;
#else /* earlier than CUPS 1.4 */
    throw runtime_error("Operation not supported - recompile against CUPS 1.4 or later");
#endif /* CUPS 1.4 */
}

map<string, map<string, string>> Connection::getDevices(
                                  const vector<string> *exclude_schemes,
                                  const vector<string> *include_schemes,
                                  int limit,
                                  int timeout)
{
    ipp_t *request = nullptr, *answer = nullptr;
    ipp_attribute_t *attr = nullptr;
    request = ippNewRequest(CUPS_GET_DEVICES);
    if (limit > 0) {
        ippAddInteger(request, IPP_TAG_OPERATION, IPP_TAG_INTEGER,
                      "limit", limit);
    }

    if (exclude_schemes) {
        size_t n = exclude_schemes->size();
        char **ss = (char **)calloc(n + 1, sizeof(char *));
        for (size_t i = 0; i < n; i++) {
            ss[i] = (char *)exclude_schemes->at(i).data();
        }

        ss[n] = nullptr;
        ippAddStrings(request, IPP_TAG_OPERATION, IPP_TAG_NAME,
                      "exclude-schemes", n, nullptr, (const char **)ss);
        free(ss);
    }

    if (include_schemes) {
        size_t n = include_schemes->size();
        char **ss = (char **)calloc(n + 1, sizeof(char *));
        for (size_t i = 0; i < n; i++) {
            ss[i] = (char *)include_schemes->at(i).data();
        }

        ss[n] = nullptr;
        ippAddStrings(request, IPP_TAG_OPERATION, IPP_TAG_NAME,
                      "include-schemes", n, nullptr, (const char **)ss);
        free(ss);
    }

    if (timeout > 0) {
        ippAddInteger(request, IPP_TAG_OPERATION, IPP_TAG_INTEGER,
                      "timeout", timeout);
    }

    debugprintf("-> Connection::getDevices()\n");
    debugprintf("cupsDoRequest(\"/\")\n");
    answer = cupsDoRequest(this->http, request, "/");
    checkIppAnswer(answer, "<- Connection::getDevices()(error)\n");

    map<string, map<string, string>> ret;
    for (attr = ippFirstAttribute(answer); attr; attr = ippNextAttribute(answer)) {
        char *device_uri = nullptr;

        while (attr && ippGetGroupTag(attr) != IPP_TAG_PRINTER) {
            attr = ippNextAttribute(answer);
        }

        if (!attr) {
            break;
        }

        map<string, string> dict;
        for (; attr && ippGetGroupTag(attr) == IPP_TAG_PRINTER; attr = ippNextAttribute(answer)) {
            const char *attrName = ippGetName(attr);
            debugprintf("Attribute: %s\n", attrName);
            if (!strcmp(attrName, "device-uri") && ippGetValueTag(attr) == IPP_TAG_URI) {
                device_uri = (char *)ippGetString(attr, 0, nullptr);
            } else {
                string val = string_from_attr_value(attr, 0);
                if (!val.empty()) {
                    debugprintf("Adding %s to device dict\n", attrName);
                    dict[attrName] = val;
                }
            }
        }

        if (device_uri) {
            debugprintf("Adding %s to result dict\n", device_uri);
            ret[device_uri] = dict;
        }

        if (!attr) {
            break;
        }
    }

    ippDelete(answer);
    debugprintf("<- Connection::getDevices() = dict\n");

    return ret;
}

static void free_requested_attrs(size_t n_attrs, char **attrs)
{
#ifdef DUP_STRING
    for (size_t i = 0; i < n_attrs; i++) {
        free(attrs[i]);
    }
#else
    (void)n_attrs;
#endif
    free(attrs);
}

static int get_requested_attrs(const vector<string> *requested_attrs,
                               size_t *n_attrs, char ***attrs)
{
    size_t i;
    size_t n;
    char **as = nullptr;

    n = requested_attrs->size();
    as = (char **)malloc((n + 1) * sizeof(char *));
    for (i = 0; i < n; i++) {
#ifdef DUP_STRING
        as[i] = strdup(requested_attrs->at(i).data());
#else
        as[i] = (char *)requested_attrs->at(i).data();
#endif
    }
    as[n] = nullptr;

    debugprintf("Requested attributes:\n");
    for (i = 0; as[i] != nullptr; i++) {
        debugprintf("  %s\n", as[i]);
    }

    *n_attrs = n;
    *attrs = as;

    return 0;
}

map<int, map<string, string>> Connection::getJobs(const char *which,
                                                  int my_jobs, int limit, int first_job_id,
                                                  const vector<string> *requested_attrs)
{
    ipp_t *request = nullptr, *answer = nullptr;
    ipp_attribute_t *attr = nullptr;
    char **attrs = nullptr; /* initialised to calm compiler */
    size_t n_attrs = 0; /* initialised to calm compiler */
    map<int, map<string, string>> ret;
    const char *cupsuser = cupsUser();

    debugprintf("-> Connection::getJobs(%s,%d)\n",
                which ? which : "(null)", my_jobs);
    request = ippNewRequest(IPP_GET_JOBS);
    ippAddString(request, IPP_TAG_OPERATION, IPP_TAG_URI, "printer-uri",
                 nullptr, "ipp://localhost/printers/");

    ippAddString(request, IPP_TAG_OPERATION, IPP_TAG_KEYWORD, "which-jobs",
                 nullptr, which ? which : "not-completed");

    ippAddString(request, IPP_TAG_OPERATION, IPP_TAG_NAME,
                 "requesting-user-name", nullptr, cupsuser);

    if (limit > 0) {
        ippAddInteger(request, IPP_TAG_OPERATION, IPP_TAG_INTEGER,
                      "limit", limit);
    }

    if (first_job_id > 0) {
        ippAddInteger(request, IPP_TAG_OPERATION, IPP_TAG_INTEGER,
                      "first-job-id", first_job_id);
    }

    if (requested_attrs) {
        if (get_requested_attrs(requested_attrs, &n_attrs, &attrs) == -1) {
            ippDelete(request);
            return ret;
        }

        ippAddStrings(request, IPP_TAG_OPERATION, IPP_TAG_KEYWORD,
                      "requested-attributes", n_attrs, nullptr,
                      (const char **)attrs);
        free_requested_attrs(n_attrs, attrs);
    }

    debugprintf("cupsDoRequest(\"/\")\n");
    answer = cupsDoRequest(this->http, request, "/");
    checkIppAnswer(answer, "<- Connection::getJobs()(error)\n");

    for (attr = ippFirstAttribute(answer); attr; attr = ippNextAttribute(answer)) {
        int job_id = -1;

        while (attr && ippGetGroupTag(attr) != IPP_TAG_JOB) {
            attr = ippNextAttribute(answer);
        }

        if (!attr) {
            break;
        }

        map<string, string> dict;
        for (; attr && ippGetGroupTag(attr) == IPP_TAG_JOB; attr = ippNextAttribute(answer)) {
            const char *attrName = ippGetName(attr);
            ipp_tag_t valTag = ippGetValueTag(attr);
            debugprintf("Attribute: %s\n", attrName);

            string val;
            if (!strcmp(attrName, "job-id") && valTag == IPP_TAG_INTEGER) {
                job_id = ippGetInteger(attr, 0);
            } else if (((!strcmp(attrName, "job-k-octets") || !strcmp(attrName, "job-priority") || !strcmp(attrName, "time-at-creation") || !strcmp(attrName, "time-at-processing") || !strcmp(attrName, "time-at-completed") || !strcmp(attrName, "job-media-sheets") || !strcmp(attrName, "job-media-sheets-completed")) && valTag == IPP_TAG_INTEGER) || (!strcmp(attrName, "job-state") && valTag == IPP_TAG_ENUM)) {
                val = string_format("i%d", ippGetInteger(attr, 0));
            } else if ((!strcmp(attrName, "job-name") && valTag == IPP_TAG_NAME) || (!strcmp(attrName, "job-printer-uri") && valTag == IPP_TAG_URI)) {
                val = string_format("s%s", ippGetString(attr, 0, nullptr));
            } else if (!strcmp(attrName, "job-preserved") && valTag == IPP_TAG_BOOLEAN) {
                val = string_format("i%d", ippGetInteger(attr, 0));
            } else if (!strcmp(attrName, "job-originating-user-name") && valTag == IPP_TAG_NAME) {
                const char *jobuser = ippGetString(attr, 0, nullptr);

                if (my_jobs && strcmp(cupsuser, jobuser)) {
                    debugprintf("Remove %s job, current user is %s\n", jobuser, cupsuser);
                    job_id = -1;
                    while (attr && ippGetGroupTag(attr) == IPP_TAG_JOB) {
                        attr = ippNextAttribute(answer);
                    }

                    break;
                }
                val = string_format("s%s", jobuser);
            } else {
                if (ippGetCount(attr) > 1)
                    val = list_from_attr_values(attr);
                else
                    val = string_from_attr_value(attr, 0);
            }

            if (!val.empty()) {
                debugprintf("Adding %s to job dict\n", attrName);
                dict[attrName] = val;
            }
        }

        if (job_id != -1) {
            debugprintf("Adding %d to result dict\n", job_id);
            ret[job_id] = dict;
        }

        if (!attr) {
            break;
        }
    }

    ippDelete(answer);
    debugprintf("<- Connection::getJobs() = dict\n");

    return ret;
}

map<string, string> Connection::getJobAttributes(int job_id,
                                                 const vector<string> *requested_attrs)
{
    ipp_t *request = nullptr, *answer = nullptr;
    ipp_attribute_t *attr = nullptr;
    char **attrs = nullptr; /* initialised to calm compiler */
    size_t n_attrs = 0; /* initialised to calm compiler */
    char uri[1024];
    map<string, string> ret;

    if (requested_attrs) {
        if (get_requested_attrs(requested_attrs, &n_attrs, &attrs) == -1) {
            return ret;
        }
    }

    debugprintf("-> Connection::getJobAttributes(%d)\n", job_id);
    request = ippNewRequest(IPP_GET_JOB_ATTRIBUTES);
    snprintf(uri, sizeof(uri), "ipp://localhost/jobs/%d", job_id);
    ippAddString(request, IPP_TAG_OPERATION, IPP_TAG_URI, "job-uri",
                 nullptr, uri);
    if (requested_attrs) {
        ippAddStrings(request, IPP_TAG_OPERATION, IPP_TAG_KEYWORD,
                      "requested-attributes", n_attrs, nullptr,
                      (const char **)attrs);
    }

    debugprintf("cupsDoRequest(\"/\")\n");
    answer = cupsDoRequest(this->http, request, "/");
    if (requested_attrs) {
        free_requested_attrs(n_attrs, attrs);
    }
    checkIppAnswer(answer, "<- Connection::getJobAttributes()(error)\n");

    for (attr = ippFirstAttribute(answer); attr; attr = ippNextAttribute(answer)) {
        const char *attrName = ippGetName(attr);
        debugprintf("Attr: %s\n", attrName);

        string val;
        if (ippGetCount(attr) > 1 || !strcmp(attrName, "job-printer-state-reasons"))
            val = list_from_attr_values(attr);
        else
            val = string_from_attr_value(attr, 0);

        if (val.empty()) {
            // Can't represent this.
            continue;
        }

        ret[attrName] = val;
    }

    ippDelete(answer);
    debugprintf("<- Connection::getJobAttributes() = dict\n");

    return ret;
}

static int do_job_op(http_t *http, int job_id, ipp_op_t op, const char *dest = nullptr);
static int do_job_op(http_t *http, int job_id, ipp_op_t op, const char *dest)
{
    ipp_t *request = nullptr; /* IPP request */
    char uri[HTTP_MAX_URI]; /* Job URI */

    request = ippNewRequest(op);

    snprintf(uri, sizeof(uri), "ipp://localhost/jobs/%d", job_id);
    ippAddString(request, IPP_TAG_OPERATION, IPP_TAG_URI, "job-uri",
                 nullptr, uri);

    ippAddString(request, IPP_TAG_OPERATION, IPP_TAG_NAME,
                 "requesting-user-name", nullptr, cupsUser());

    if (IPP_PURGE_JOBS == op) {
        memset(uri, 0, sizeof(uri));
        if (!dest)
            strcpy(uri, "ipp://localhost/printers");
        else if (0 == strcmp("all jobs", dest))
            sprintf(uri, "ipp://localhost/printers/%s", "");
        else
            sprintf(uri, "ipp://localhost/jobs/%d", job_id);

        ippAddString(request, IPP_TAG_OPERATION, IPP_TAG_URI,
                     "printer-uri", nullptr, uri);

        ippAddBoolean(request, IPP_TAG_OPERATION, "purge-jobs", 1);
    }

    ippDelete(cupsDoRequest(http, request, "/admin/"));

    if (cupsLastError() > IPP_STATUS_OK_CONFLICTING) {
        return -1;
    }
    return 0;
}

int Connection::holdJob(int job_id)
{
    return do_job_op(this->http, job_id, IPP_HOLD_JOB);
}

int Connection::releaseJob(int job_id)
{
    return do_job_op(this->http, job_id, IPP_RELEASE_JOB);
}

void Connection::cancelJob(int job_id, int purge_job)
{
    ipp_t *request = nullptr, *answer = nullptr;
    char uri[1024];

    debugprintf("-> Connection::cancelJob(%d)\n", job_id);
    request = ippNewRequest(IPP_CANCEL_JOB);
    snprintf(uri, sizeof(uri), "ipp://localhost/jobs/%d", job_id);
    ippAddString(request, IPP_TAG_OPERATION, IPP_TAG_URI, "job-uri", nullptr, uri);
    ippAddString(request, IPP_TAG_OPERATION, IPP_TAG_NAME,
                 "requesting-user-name", nullptr, cupsUser());
    if (purge_job) {
        ippAddBoolean(request, IPP_TAG_OPERATION, "purge-job", 1);
    }
    debugprintf("cupsDoRequest(\"/jobs/\")\n");
    answer = cupsDoRequest(this->http, request, "/jobs/");
    checkIppAnswer(answer, "<- Connection::cancelJob()(error)\n");
    ippDelete(answer);
    debugprintf("<- Connection::cancelJob() = None\n");
}

void Connection::cancelAllJobs(const char *name,
                               const char *uri, int my_jobs, int purge_jobs)
{
    char consuri[HTTP_MAX_URI];
    ipp_t *request = nullptr, *answer = nullptr;
    int i;

    debugprintf("-> Connection::cancelAllJobs(%s, my_jobs=%d, purge_jobs=%d)\n",
                name ? name : uri, my_jobs, purge_jobs);
    if (name) {
        construct_uri(consuri, sizeof(consuri),
                      "ipp://localhost/printers/", name);
        uri = consuri;
    }

    for (i = 0; i < 2; i++) {
        request = ippNewRequest(IPP_PURGE_JOBS);
        ippAddString(request, IPP_TAG_OPERATION, IPP_TAG_URI, "printer-uri",
                     nullptr, uri);

        if (my_jobs) {
            ippAddBoolean(request, IPP_TAG_OPERATION, "my-jobs", my_jobs);
            ippAddString(request, IPP_TAG_OPERATION, IPP_TAG_NAME,
                         "requesting-user-name", nullptr, cupsUser());
        }

        ippAddBoolean(request, IPP_TAG_OPERATION, "purge-jobs", purge_jobs);
        debugprintf("cupsDoRequest(\"/admin/\") with printer-uri=%s\n", uri);
        answer = cupsDoRequest(this->http, request, "/admin/");
        if (answer && ippGetStatusCode(answer) == IPP_NOT_POSSIBLE) {
            ippDelete(answer);
            if (!name)
                break;

            // Perhaps it's a class, not a printer.
            construct_uri(consuri, sizeof(consuri),
                          "ipp://localhost/classes/", name);
        } else {
            break;
        }
    }

    checkIppAnswer(answer, "<- Connection::cancelAllJobs()(error)\n");
    ippDelete(answer);

    debugprintf("<- Connection::cancelAllJobs() = None\n");
}

int Connection::createJob(const char *printer, const char *title,
                          const map<string, string> *options)
{
    int num_settings = 0;
    cups_option_t *settings = nullptr;
    int jobid;

    debugprintf("-> Connection::createJob(printer=%s, title=%s)\n", printer, title);

    if (options) {
        for (const auto &it : *options) {
            num_settings = cupsAddOption(it.first.data(),
                                         it.second.data(),
                                         num_settings,
                                         &settings);
        }
    }

    jobid = cupsCreateJob(this->http, printer, title, num_settings, settings);
    if (jobid == 0) {
        cupsFreeOptions(num_settings, settings);
        string err = get_ipp_error(cupsLastError(), cupsLastErrorString());
        debugprintf("<- Connection::createJob() = nullptr\n");
        throw runtime_error(err);
    }

    cupsFreeOptions(num_settings, settings);
    debugprintf("<- Connection::createJob() = %d\n", jobid);

    return jobid;
}

int Connection::startDocument(const char *printer, int jobid,
                              const char *doc_name, const char *format, int last_document)
{
    http_status_t status; /* Write status */

    debugprintf("-> Connection::startDocument(printer=%s, jobid=%d, doc_name=%s, format=%s)\n",
                printer, jobid, doc_name, format);

    status = cupsStartDocument(this->http, printer, jobid, doc_name, format, last_document);
    if (status != HTTP_CONTINUE) {
        string err = get_ipp_error(cupsLastError(), cupsLastErrorString());
        debugprintf("<- Connection::startDocument() = nullptr\n");
        throw runtime_error(err);
    }

    debugprintf("<- Connection::startDocument() = %d\n", status);

    return status;
}

int Connection::writeRequestData(const char *buffer, int length)
{
    http_status_t status; /* Write status */

    debugprintf("-> Connection::writeRequestData(length=%d)\n", length);

    status = cupsWriteRequestData(this->http, buffer, length);
    if (status != HTTP_CONTINUE) {
        string err = get_ipp_error(cupsLastError(), cupsLastErrorString());
        debugprintf("<- Connection::writeRequestData() = nullptr\n");
        throw runtime_error(err);
    }

    debugprintf("<- Connection::writeRequestData() = %d\n", status);

    return status;
}

int Connection::finishDocument(const char *printer)
{
    int answer;

    debugprintf("-> Connection::finishDocument(printer=%s)\n", printer);
    answer = cupsFinishDocument(this->http, printer);
    if (answer != IPP_OK) {
        string err = get_ipp_error(cupsLastError(), cupsLastErrorString());
        debugprintf("<- Connection::finishDocument() = nullptr\n");
        throw runtime_error(err);
    }

    debugprintf("<- Connection::finishDicument() = %d\n", answer);

    return answer;
}

void Connection::moveJob(const char *printeruri,
                         int job_id, const char *jobprinteruri)
{
    ipp_t *request = nullptr, *answer = nullptr;

    if (!jobprinteruri) {
        throw runtime_error("No job_printer_uri(destination) given");
    }

    if (printeruri) {
    } else if (job_id == -1) {
        throw runtime_error("job_id or printer_uri required");
    }

    request = ippNewRequest(CUPS_MOVE_JOB);
    if (!printeruri) {
        char joburi[HTTP_MAX_URI];
        snprintf(joburi, sizeof(joburi), "ipp://localhost/jobs/%d", job_id);
        ippAddString(request, IPP_TAG_OPERATION, IPP_TAG_URI, "job-uri", nullptr,
                     joburi);
    } else {
        ippAddString(request, IPP_TAG_OPERATION, IPP_TAG_URI, "printer-uri", nullptr,
                     printeruri);

        if (job_id != -1) {
            ippAddInteger(request, IPP_TAG_OPERATION, IPP_TAG_INTEGER, "job-id",
                          job_id);
        }
    }

    ippAddString(request, IPP_TAG_JOB, IPP_TAG_URI, "job-printer-uri", nullptr,
                 jobprinteruri);
    answer = cupsDoRequest(this->http, request, "/jobs");
    checkIppAnswer(answer, nullptr);
    ippDelete(answer);
}

void Connection::authenticateJob(int job_id,
                                 const vector<string> *auth_info_list)
{
    ipp_t *request = nullptr, *answer = nullptr;
    size_t num_auth_info = 0; /* initialised to calm compiler */
    char *auth_info_values[3];
    size_t i;
    char uri[1024];

    if (auth_info_list) {
        num_auth_info = auth_info_list->size();
        debugprintf("sizeof values = %Zd\n", sizeof(auth_info_values));
        if (num_auth_info > sizeof(auth_info_values)) {
            num_auth_info = sizeof(auth_info_values);
        }

        for (i = 0; i < num_auth_info; i++) {
            auth_info_values[i] = (char *)auth_info_list->at(i).data();
        }
    }

    debugprintf("-> Connection::authenticateJob(%d)\n", job_id);
    request = ippNewRequest(CUPS_AUTHENTICATE_JOB);
    snprintf(uri, sizeof(uri), "ipp://localhost/jobs/%d", job_id);
    ippAddString(request, IPP_TAG_OPERATION, IPP_TAG_URI, "job-uri", nullptr, uri);
    ippAddString(request, IPP_TAG_OPERATION, IPP_TAG_NAME,
                 "requesting-user-name", nullptr, cupsUser());
    if (auth_info_list) {
        ippAddStrings(request, IPP_TAG_OPERATION, IPP_TAG_TEXT, "auth-info",
                      num_auth_info, nullptr,
                      (const char *const *)auth_info_values);
        for (i = 0; i < num_auth_info; i++) {
            free(auth_info_values[i]);
        }
    }

    debugprintf("cupsDoRequest(\"/jobs/\")\n");
    answer = cupsDoRequest(this->http, request, "/jobs/");
    checkIppAnswer(answer, "<- Connection::authenticateJob()(error)\n");
    ippDelete(answer);

    debugprintf("<- Connection::authenticateJob() = None\n");
}

static void set_job_attrs(http_t *http, int job_id, int num_options, cups_option_t *options)
{
    ipp_t *request = nullptr, *answer = nullptr;
    char uri[HTTP_MAX_URI];

    if (num_options == 0)
        return;

    request = ippNewRequest(IPP_SET_JOB_ATTRIBUTES);
    sprintf(uri, "ipp://localhost/jobs/%d", job_id);
    ippAddString(request, IPP_TAG_OPERATION, IPP_TAG_URI,
                 "job-uri", nullptr, uri);
    ippAddString(request, IPP_TAG_OPERATION, IPP_TAG_NAME,
                 "requesting-user-name", nullptr, cupsUser());
    cupsEncodeOptions(request, num_options, options);

    answer = cupsDoRequest(http, request, "/jobs/");
    checkIppAnswer(answer, "<- set job attributes(error)\n");
    ippDelete(answer);

    return;
}

void Connection::setJobPriority(int job_id, int iPriority)
{
    cups_option_t *options = nullptr;
    int num_options = 0;
    char buf[16] = {0};

    sprintf(buf, "%d", iPriority);
    num_options = cupsAddOption("job-priority", buf,
                                num_options, &options);

    debugprintf("-> Connection::setJobPriority(%d,%d)\n",
                job_id, iPriority);

    set_job_attrs(this->http, job_id, num_options, options);

    debugprintf("<- Connection::setJobPriority() = None\n");
}

void Connection::setJobHoldUntil(int job_id, const char *job_hold_until)
{
    cups_option_t *options = nullptr;
    int num_options = 0;

    num_options = cupsAddOption("job-hold-until", job_hold_until,
                                num_options, &options);

    debugprintf("-> Connection::setJobHoldUntil(%d,%s)\n",
                job_id, job_hold_until);

    set_job_attrs(this->http, job_id, num_options, options);

    debugprintf("<- Connection::setJobHoldUntil() = None\n");
}

void Connection::restartJob(int job_id, const char *job_hold_until)
{
    ipp_t *request = nullptr, *answer = nullptr;
    char uri[1024];

    debugprintf("-> Connection::restartJob(%d)\n", job_id);
    request = ippNewRequest(IPP_RESTART_JOB);
    snprintf(uri, sizeof(uri), "ipp://localhost/jobs/%d", job_id);
    ippAddString(request, IPP_TAG_OPERATION, IPP_TAG_URI, "job-uri", nullptr, uri);
    ippAddString(request, IPP_TAG_OPERATION, IPP_TAG_NAME,
                 "requesting-user-name", nullptr, cupsUser());
    if (job_hold_until) {
        ippAddString(request, IPP_TAG_OPERATION, IPP_TAG_NAME,
                     "job-hold-until", nullptr, job_hold_until);
    }

    debugprintf("cupsDoRequest(\"/jobs/\")\n");
    answer = cupsDoRequest(this->http, request, "/jobs/");
    checkIppAnswer(answer, "<- Connection::restartJob()(error)\n");
    ippDelete(answer);

    debugprintf("<- Connection::restartJob() = None\n");
}

void Connection::getFile(const char *resource, const char *filename, int fd)
{
    http_status_t status;

    if (fd > -1 && filename) {
        throw runtime_error("Only one destination type may be specified");
    }

    if (filename) {
        debugprintf("-> Connection::getFile(%s, %s)\n", resource, filename);
        debugprintf("cupsGetFile()\n");
        status = cupsGetFile(this->http, resource, filename);
    } else {
        debugprintf("-> Connection::getFile(%s, %d)\n", resource, fd);
        debugprintf("cupsGetFd()\n");
        status = cupsGetFd(this->http, resource, fd);
    }

    if (status != HTTP_OK) {
        debugprintf("<- Connection::getFile()(error)\n");
        set_http_error(status);
    }

    debugprintf("<- Connection::getFile() = None\n");
}

void Connection::putFile(const char *resource, const char *filename, int fd)
{
    http_status_t status;

    if (fd > -1 && filename) {
        throw runtime_error("Only one destination type may be specified");
    }

    if (filename) {
        debugprintf("-> Connection::putFile(%s, %s)\n", resource, filename);
        debugprintf("cupsPutFile()\n");
        status = cupsPutFile(this->http, resource, filename);
    } else {
        debugprintf("-> Connection::putFile(%s, %d)\n", resource, fd);
        debugprintf("cupsPutFd()\n");
        status = cupsPutFd(this->http, resource, fd);
    }

    if (status != HTTP_OK && status != HTTP_CREATED) {
        debugprintf("<- Connection::putFile()(error)\n");
        set_http_error(status);
    }

    debugprintf("<- Connection::putFile() = None\n");
}

static ipp_t *add_modify_printer_request(const char *name)
{
    char uri[HTTP_MAX_URI];
    ipp_t *request = ippNewRequest(CUPS_ADD_MODIFY_PRINTER);
    construct_uri(uri, sizeof(uri), "ipp://localhost/printers/", name);
    ippAddString(request, IPP_TAG_OPERATION, IPP_TAG_URI,
                 "printer-uri", NULL, uri);
    return request;
}

static ipp_t *add_modify_class_request(const char *name)
{
    char uri[HTTP_MAX_URI];
    ipp_t *request = ippNewRequest(CUPS_ADD_MODIFY_CLASS);
    construct_uri(uri, sizeof(uri), "ipp://localhost/classes/", name);
    ippAddString(request, IPP_TAG_OPERATION, IPP_TAG_URI,
                 "printer-uri", NULL, uri);
    return request;
}

void Connection::addPrinter(const char *name,
                            const char *info,
                            const char *location,
                            const char *device,
                            const char *ppdfile,
                            const char *ppdname,
                            PPD *ppd)
{
    ipp_t *request = nullptr, *answer = nullptr;
    int ppds_specified = 0;
    char templatestr[PATH_MAX] = {0};

    debugprintf("-> Connection::addPrinter(%s,%s,%s,%s,%s,%s,%s)\n",
                name, ppdfile ? ppdfile : "", ppdname ? ppdname : "",
                info ? info : "", location ? location : "",
                device ? device : "", ppd ? "(PPD object)" : "");

    if (ppdfile)
        ppds_specified++;
    if (ppdname)
        ppds_specified++;
    if (ppd) {
        ppds_specified++;
    }
    if (ppds_specified > 1) {
        debugprintf("<- Connection::addPrinter() EXCEPTION\n");
        throw runtime_error("Only one PPD may be given");
    }

    snprintf(templatestr, sizeof(templatestr), "%s/scp-ppd-XXXXXX", _PATH_TMP);

    if (ppd) {
        // We've been given a cups.PPD object.  Construct a PPD file.
        int fd;

        ppdfile = templatestr;
        fd = mkstemp((char *)ppdfile);
        if (fd < 0) {
            debugprintf("<- Connection::addPrinter() EXCEPTION\n");
            throw runtime_error(string_format("failed to create %s, error=%d", ppdfile, errno));
        }

        try {
            ppd->writeFd(fd);
        } catch (...) {
            unlink(ppdfile);
            debugprintf("<- Connection::addPrinter() EXCEPTION\n");
            return;
        }
        close(fd);
    }

    request = add_modify_printer_request(name);
    if (ppdname) {
        ippAddString(request, IPP_TAG_OPERATION, IPP_TAG_NAME,
                     "ppd-name", nullptr, ppdname);
    }
    if (info) {
        ippAddString(request, IPP_TAG_PRINTER, IPP_TAG_TEXT,
                     "printer-info", nullptr, info);
    }
    if (location) {
        ippAddString(request, IPP_TAG_PRINTER, IPP_TAG_TEXT,
                     "printer-location", nullptr, location);
    }
    if (device) {
        ippAddString(request, IPP_TAG_PRINTER, IPP_TAG_URI,
                     "device-uri", nullptr, device);
    }
    if (ppds_specified) {
        ippAddString(request, IPP_TAG_PRINTER, IPP_TAG_KEYWORD,
                     "printer-state-reasons", nullptr, "none");
    }

    if (ppdfile) {
        answer = cupsDoFileRequest(this->http, request, "/admin/", ppdfile);
    } else {
        answer = cupsDoRequest(this->http, request, "/admin/");
    }

    if (ppd) {
        unlink(ppdfile);
    }

    checkIppAnswer(answer, "<- Connection::addPrinter() EXCEPTION\n");
    ippDelete(answer);
    debugprintf("<- Connection::addPrinter() = None\n");
}

void Connection::setPrinterDevice(const char *name, const char *device_uri)
{
    ipp_t *request = nullptr, *answer = nullptr;

    request = add_modify_printer_request(name);
    ippAddString(request, IPP_TAG_PRINTER, IPP_TAG_URI,
                 "device-uri", nullptr, device_uri);
    answer = cupsDoRequest(this->http, request, "/admin/");
    checkIppAnswer(answer, nullptr);
    ippDelete(answer);
}

void Connection::setPrinterInfo(const char *name, const char *info)
{
    ipp_t *request = nullptr, *answer = nullptr;
    int i;

    request = add_modify_printer_request(name);
    for (i = 0; i < 2; i++) {
        ippAddString(request, IPP_TAG_PRINTER, IPP_TAG_TEXT,
                     "printer-info", nullptr, info);
        answer = cupsDoRequest(this->http, request, "/admin/");
        if (answer && ippGetStatusCode(answer) == IPP_NOT_POSSIBLE) {
            // Perhaps it's a class, not a printer.
            ippDelete(answer);
            request = add_modify_class_request(name);
        } else {
            break;
        }
    }

    checkIppAnswer(answer, nullptr);
    ippDelete(answer);
}

void Connection::setPrinterLocation(const char *name, const char *location)
{
    ipp_t *request = nullptr, *answer = nullptr;
    int i;

    request = add_modify_printer_request(name);
    for (i = 0; i < 2; i++) {
        ippAddString(request, IPP_TAG_PRINTER, IPP_TAG_TEXT,
                     "printer-location", nullptr, location);
        answer = cupsDoRequest(this->http, request, "/admin/");
        if (answer && ippGetStatusCode(answer) == IPP_NOT_POSSIBLE) {
            // Perhaps it's a class, not a printer.
            ippDelete(answer);
            request = add_modify_class_request(name);
        } else {
            break;
        }
    }

    checkIppAnswer(answer, nullptr);
    ippDelete(answer);
}

void Connection::setPrinterShared(const char *name, int sharing)
{
    ipp_t *request = nullptr, *answer = nullptr;
    int i;

    request = add_modify_printer_request(name);
    for (i = 0; i < 2; i++) {
        ippAddBoolean(request, IPP_TAG_OPERATION, "printer-is-shared", sharing);
        answer = cupsDoRequest(this->http, request, "/admin/");
        if (answer && ippGetStatusCode(answer) == IPP_NOT_POSSIBLE) {
            // Perhaps it's a class, not a printer.
            ippDelete(answer);
            request = add_modify_class_request(name);
        } else {
            break;
        }
    }

    checkIppAnswer(answer, nullptr);
    ippDelete(answer);
}

void Connection::setPrinterJobSheets(char *name, char *start, char *end)
{
    ipp_t *request = nullptr, *answer = nullptr;
    ipp_attribute_t *a = nullptr;
    int i;

    request = add_modify_printer_request(name);
    for (i = 0; i < 2; i++) {
        a = ippAddStrings(request, IPP_TAG_PRINTER, IPP_TAG_NAME,
                          "job-sheets-default", 2, nullptr, nullptr);
        ippSetString(request, &a, 0, start);
        ippSetString(request, &a, 1, end);
        answer = cupsDoRequest(this->http, request, "/admin/");
        if (answer && ippGetStatusCode(answer) == IPP_NOT_POSSIBLE) {
            ippDelete(answer);
            // Perhaps it's a class, not a printer.
            request = add_modify_class_request(name);
        } else {
            break;
        }
    }

    checkIppAnswer(answer, nullptr);
    ippDelete(answer);
}

void Connection::setPrinterErrorPolicy(const char *name, const char *policy)
{
    ipp_t *request = nullptr, *answer = nullptr;
    int i;

    request = add_modify_printer_request(name);
    for (i = 0; i < 2; i++) {
        ippAddString(request, IPP_TAG_PRINTER, IPP_TAG_NAME,
                     "printer-error-policy", nullptr, policy);
        answer = cupsDoRequest(this->http, request, "/admin/");
        if (answer && ippGetStatusCode(answer) == IPP_NOT_POSSIBLE) {
            ippDelete(answer);
            // Perhaps it's a class, not a printer.
            request = add_modify_class_request(name);
        } else {
            break;
        }
    }

    checkIppAnswer(answer, nullptr);
    ippDelete(answer);
}

void Connection::setPrinterOpPolicy(const char *name, const char *policy)
{
    ipp_t *request = nullptr, *answer = nullptr;
    int i;

    request = add_modify_printer_request(name);
    for (i = 0; i < 2; i++) {
        ippAddString(request, IPP_TAG_PRINTER, IPP_TAG_NAME,
                     "printer-op-policy", nullptr, policy);
        answer = cupsDoRequest(this->http, request, "/admin/");
        if (answer && ippGetStatusCode(answer) == IPP_NOT_POSSIBLE) {
            ippDelete(answer);
            // Perhaps it's a class, not a printer.
            request = add_modify_class_request(name);
        } else {
            break;
        }
    }

    checkIppAnswer(answer, nullptr);
    ippDelete(answer);
}

void Connection::do_requesting_user_names(const char *name,
                                          const vector<string> *users, const char *requeststr)
{
    int num_users, i, j;
    ipp_t *request = nullptr, *answer = nullptr;
    ipp_attribute_t *attr = nullptr;

    request = add_modify_printer_request(name);
    for (i = 0; i < 2; i++) {
        num_users = users->size();
        if (num_users) {
            attr = ippAddStrings(request, IPP_TAG_PRINTER, IPP_TAG_NAME,
                                 requeststr, num_users, nullptr, nullptr);
            for (j = 0; j < num_users; j++) {
                const char *username = users->at(j).data();
                ippSetString(request, &attr, j, username);
            }
        } else {
            attr = ippAddStrings(request, IPP_TAG_PRINTER, IPP_TAG_NAME,
                                 requeststr, 1, nullptr, nullptr);
            if (strstr(requeststr, "denied"))
                ippSetString(request, &attr, 0, "none");
            else
                ippSetString(request, &attr, 0, "all");
        }

        answer = cupsDoRequest(this->http, request, "/admin/");
        if (answer && ippGetStatusCode(answer) == IPP_NOT_POSSIBLE) {
            ippDelete(answer);
            // Perhaps it's a class, not a printer.
            request = add_modify_class_request(name);
        } else {
            break;
        }
    }

    checkIppAnswer(answer, nullptr);
    ippDelete(answer);
}

void Connection::setPrinterUsersAllowed(const char *name,
                                        const vector<string> *users)
{
    return do_requesting_user_names(name, users, "requesting-user-name-allowed");
}

void Connection::setPrinterUsersDenied(const char *name,
                                       const vector<string> *users)
{
    return do_requesting_user_names(name, users, "requesting-user-name-denied");
}

void Connection::addPrinterOptionDefault(const char *name, const char *option,
                                         const vector<string> *values)
{
    const char suffix[] = "-default";
    std::string opt;
    opt.append(option);
    opt.append(suffix);
    ipp_t *request = nullptr, *answer = nullptr;
    int i;

    request = add_modify_printer_request(name);
    for (i = 0; i < 2; i++) {
        ipp_attribute_t *attr = nullptr;
        int len = values->size();
        int j;
        attr = ippAddStrings(request, IPP_TAG_PRINTER, IPP_TAG_NAME,
                             opt.c_str(), len, nullptr, nullptr);
        for (j = 0; j < len; j++) {
            ippSetString(request, &attr, j, values->at(j).data());
        }

        answer = cupsDoRequest(this->http, request, "/admin/");
        if (answer && ippGetStatusCode(answer) == IPP_NOT_POSSIBLE) {
            ippDelete(answer);
            // Perhaps it's a class, not a printer.
            request = add_modify_class_request(name);
        } else {
            break;
        }
    }

    checkIppAnswer(answer, nullptr);
    ippDelete(answer);
}

void Connection::deletePrinterOptionDefault(const char *name, const char *option)
{
    const char suffix[] = "-default";
    char *opt = nullptr;
    ipp_t *request = nullptr, *answer = nullptr;
    int i;
    size_t optionlen;

    optionlen = strlen(option);
    opt = (char *)malloc(optionlen + sizeof(suffix) + 1);
    memcpy(opt, option, optionlen);
    strcpy(opt + optionlen, suffix);
    request = add_modify_printer_request(name);
    for (i = 0; i < 2; i++) {
        ippAddString(request, IPP_TAG_PRINTER, IPP_TAG_DELETEATTR,
                     opt, nullptr, nullptr);
        answer = cupsDoRequest(this->http, request, "/admin/");
        if (answer && ippGetStatusCode(answer) == IPP_NOT_POSSIBLE) {
            ippDelete(answer);
            // Perhaps it's a class, not a printer.
            request = add_modify_class_request(name);
        } else {
            break;
        }
    }

    checkIppAnswer(answer, nullptr);
    ippDelete(answer);
}

void Connection::do_printer_request(const char *name, const char *reason, ipp_op_t op)
{
    char uri[HTTP_MAX_URI];
    ipp_t *request = nullptr, *answer = nullptr;

    debugprintf("-> do_printer_request(op:%d, name:%s)\n", (int)op, name);

    request = ippNewRequest(op);
    construct_uri(uri, sizeof(uri), "ipp://localhost/printers/", name);

    ippAddString(request, IPP_TAG_OPERATION, IPP_TAG_URI,
                 "printer-uri", NULL, uri);

    if (reason) {
        debugprintf("reason: %s\n", reason);
        ippAddString(request, IPP_TAG_OPERATION, IPP_TAG_TEXT,
                     "printer-state-message", NULL, reason);
    }

    debugprintf("cupsDoRequest(\"/admin/\")\n");
    answer = cupsDoRequest(this->http, request, "/admin/");
    checkIppAnswer(answer, "<- do_printer_request(error)\n");

    ippDelete(answer);
    debugprintf("<- do_printer_request(None)\n");
}

void Connection::deletePrinter(const char *name, const char *reason)
{
    return do_printer_request(name, reason, CUPS_DELETE_PRINTER);
}

map<string, string> Connection::getPrinterAttributes(const char *name,
                                                     const char *uri, const vector<string> *requested_attrs)
{
    char **attrs = nullptr; /* initialised to calm compiler */
    size_t n_attrs = 0; /* initialised to calm compiler */
    ipp_t *request = nullptr, *answer = nullptr;
    ipp_attribute_t *attr = nullptr;
    char consuri[HTTP_MAX_URI] = {0};

    int i;
    map<string, string> ret;

    if (name && uri) {
        throw runtime_error("name or uri must be specified but not both");
    }

    if (name) {
    } else if (uri) {
    } else {
        throw runtime_error("name or uri must be specified");
    }

    if (requested_attrs) {
        if (get_requested_attrs(requested_attrs, &n_attrs, &attrs) == -1) {
            return ret;
        }
    }

    debugprintf("-> Connection::getPrinterAttributes(%s)\n",
                name ? name : uri);

    if (name) {
        construct_uri(consuri, sizeof(consuri),
                      "ipp://localhost/printers/", name);
        uri = consuri;
    }

    for (i = 0; i < 2; i++) {
        request = ippNewRequest(IPP_GET_PRINTER_ATTRIBUTES);
        ippAddString(request, IPP_TAG_OPERATION, IPP_TAG_URI,
                     "printer-uri", nullptr, uri);
        if (requested_attrs) {
            ippAddStrings(request, IPP_TAG_OPERATION, IPP_TAG_KEYWORD,
                          "requested-attributes", n_attrs, nullptr,
                          (const char **)attrs);
        }
        debugprintf("trying request with uri %s\n", uri);
        answer = cupsDoRequest(this->http, request, "/");
        if (answer && ippGetStatusCode(answer) == IPP_NOT_POSSIBLE) {
            ippDelete(answer);
            answer = nullptr;
            if (!name)
                break;

            // Perhaps it's a class, not a printer.
            construct_uri(consuri, sizeof(consuri),
                          "ipp://localhost/classes/", name);
        } else {
            break;
        }
    }

    if (requested_attrs) {
        free_requested_attrs(n_attrs, attrs);
    }

    checkIppAnswer(answer, "<- Connection::getPrinterAttributes()(error)\n");

    for (attr = ippFirstAttribute(answer); attr; attr = ippNextAttribute(answer)) {
        while (attr && ippGetGroupTag(attr) != IPP_TAG_PRINTER) {
            attr = ippNextAttribute(answer);
        }

        if (!attr) {
            break;
        }

        for (; attr && ippGetGroupTag(attr) == IPP_TAG_PRINTER; attr = ippNextAttribute(answer)) {
            const char *attrName = ippGetName(attr);
            size_t namelen = strlen(attrName);
            bool is_list = ippGetCount(attr) > 1;
            ipp_tag_t valTag = ippGetValueTag(attr);

            debugprintf("Attribute: %s\n", attrName);
            // job-sheets-default is special, since it is always two values.
            // Make it a tuple.
            if (!strcmp(attrName, "job-sheets-default") && valTag == IPP_TAG_NAME) {
                const char *start = nullptr, *end = nullptr;
                start = ippGetString(attr, 0, nullptr);
                if (ippGetCount(attr) >= 2)
                    end = ippGetString(attr, 1, nullptr);
                else
                    end = "";

                ret["job-sheets-default"] = string_format("`s%s`s%s", start, end);
                continue;
            }

            // Check for '-supported' suffix.  Any xxx-supported attribute
            // that is a text type must be a list.
            //
            // Also check for attributes that are known to allow multiple
            // string values, and make them lists.
            if (!is_list && namelen > 10) {
                const char *multivalue_options[] = {
                    "notify-events-default",
                    "requesting-user-name-allowed",
                    "requesting-user-name-denied",
                    "printer-state-reasons",
                    "marker-colors",
                    "marker-names",
                    "marker-types",
                    "marker-levels",
                    "member-names",
                    nullptr
                };

                switch (valTag) {
                case IPP_TAG_NAME:
                case IPP_TAG_TEXT:
                case IPP_TAG_KEYWORD:
                case IPP_TAG_URI:
                case IPP_TAG_CHARSET:
                case IPP_TAG_MIMETYPE:
                case IPP_TAG_LANGUAGE:
                case IPP_TAG_ENUM:
                case IPP_TAG_INTEGER:
                case IPP_TAG_RESOLUTION:
                    is_list = !strcmp(attrName + namelen - 10, "-supported");

                    if (!is_list) {
                        const char **opt = nullptr;
                        for (opt = multivalue_options; !is_list && *opt; opt++) {
                            is_list = !strcmp(attrName, *opt);
                        }
                    }
                    break;
                default:
                    break;
                }
            }

            if (is_list) {
                ret[attrName] = list_from_attr_values(attr);
            } else {
                ret[attrName] = string_from_attr_value(attr, i);
            }
        }

        if (!attr) {
            break;
        }
    }
    if (answer)
        ippDelete(answer);
    debugprintf("<- Connection::getPrinterAttributes() = dict\n");

    return ret;
}

void Connection::addPrinterToClass(const char *printername, const char *classname)
{
    char classuri[HTTP_MAX_URI];
    char printeruri[HTTP_MAX_URI];
    ipp_t *request = nullptr, *answer = nullptr;

    // Does the class exist, and is the printer already in it?
    request = ippNewRequest(IPP_GET_PRINTER_ATTRIBUTES);
    construct_uri(classuri, sizeof(classuri),
                  "ipp://localhost/classes/", classname);

    ippAddString(request, IPP_TAG_OPERATION, IPP_TAG_URI,
                 "printer-uri", nullptr, classuri);
    answer = cupsDoRequest(this->http, request, "/");
    if (answer) {
        ipp_attribute_t *printers = nullptr;
        printers = ippFindAttribute(answer, "member-names", IPP_TAG_NAME);
        if (printers) {
            int i;
            for (i = 0; i < ippGetCount(printers); i++) {
                if (!strcasecmp(ippGetString(printers, i, nullptr), printername)) {
                    ippDelete(answer);
                    throw runtime_error("Printer already in class");
                }
            }
        }
    }

    request = ippNewRequest(CUPS_ADD_CLASS);
    ippAddString(request, IPP_TAG_OPERATION, IPP_TAG_URI,
                 "printer-uri", nullptr, classuri);
    construct_uri(printeruri, sizeof(printeruri),
                  "ipp://localhost/printers/", printername);
    if (answer) {
        ipp_attribute_t *printers = nullptr;
        printers = ippFindAttribute(answer, "member-uris", IPP_TAG_URI);
        if (printers) {
            ipp_attribute_t *attr = nullptr;
            int i;
            attr = ippAddStrings(request, IPP_TAG_PRINTER, IPP_TAG_URI,
                                 "member-uris", ippGetCount(printers) + 1,
                                 nullptr, nullptr);
            for (i = 0; i < ippGetCount(printers); i++) {
                ippSetString(request, &attr, i,
                             ippGetString(printers, i, nullptr));
            }
            ippSetString(request, &attr, ippGetCount(printers), (printeruri));
        }

        ippDelete(answer);
    }

    // If the class didn't exist, create a new one.
    if (!ippFindAttribute(request, "member-uris", IPP_TAG_URI)) {
        ippAddString(request, IPP_TAG_PRINTER, IPP_TAG_URI,
                     "member-uris", nullptr, printeruri);
    }

    answer = cupsDoRequest(this->http, request, "/admin/");
    checkIppAnswer(answer, nullptr);
    ippDelete(answer);
}

void Connection::deletePrinterFromClass(const char *printername, const char *classname)
{
    const char *requested_attrs[] = {
        "member-names",
        "member-uris"
    };
    char classuri[HTTP_MAX_URI];
    ipp_t *request = nullptr, *answer = nullptr;
    ipp_attribute_t *printers = nullptr;
    int i;

    // Does the class exist, and is the printer in it?
    request = ippNewRequest(IPP_GET_PRINTER_ATTRIBUTES);
    construct_uri(classuri, sizeof(classuri),
                  "ipp://localhost/classes/", classname);
    ippAddString(request, IPP_TAG_OPERATION, IPP_TAG_URI,
                 "printer-uri", nullptr, classuri);
    ippAddStrings(request, IPP_TAG_OPERATION, IPP_TAG_KEYWORD,
                  "requested-attributes",
                  sizeof(requested_attrs) / sizeof(requested_attrs[0]),
                  nullptr, requested_attrs);
    answer = cupsDoRequest(this->http, request, "/");
    if (!answer) {
        string err = get_ipp_error(cupsLastError(), cupsLastErrorString());
        throw runtime_error(err);
    }

    printers = ippFindAttribute(answer, "member-names", IPP_TAG_NAME);
    for (i = 0; printers && i < ippGetCount(printers); i++) {
        if (!strcasecmp(ippGetString(printers, i, nullptr), printername)) {
            break;
        }
    }

    if (!printers || i == ippGetCount(printers)) {
        ippDelete(answer);
        throw runtime_error("Printer not in class");
    }

    printers = ippFindAttribute(answer, "member-uris", IPP_TAG_URI);
    if (!printers || i >= ippGetCount(printers)) {
        ippDelete(answer);
        throw runtime_error("No member URIs returned");
    }

    request = ippNewRequest(CUPS_ADD_CLASS);
    ippAddString(request, IPP_TAG_OPERATION, IPP_TAG_URI,
                 "printer-uri", nullptr, classuri);

    // Only printer in class?  Delete the class.
    if (ippGetCount(printers) == 1) {
        ippSetOperation(request, CUPS_DELETE_CLASS);
    } else {
        // Trim the printer from the list.
        ipp_attribute_t *newlist = nullptr;
        int j;
        newlist = ippAddStrings(request, IPP_TAG_PRINTER, IPP_TAG_URI,
                                "member-uris", ippGetCount(printers) - 1,
                                nullptr, nullptr);
        for (j = 0; j < i; j++) {
            ippSetString(request, &newlist, j,
                         ippGetString(printers, j, nullptr));
        }
        for (j = i; j < ippGetCount(newlist); j++) {
            ippSetString(request, &newlist, j,
                         ippGetString(printers, j + 1, nullptr));
        }
    }

    ippDelete(answer);
    answer = cupsDoRequest(this->http, request, "/admin/");
    checkIppAnswer(answer, nullptr);
    ippDelete(answer);
}

void Connection::deleteClass(const char *classname)
{
    char classuri[HTTP_MAX_URI];
    ipp_t *request = nullptr, *answer = nullptr;

    request = ippNewRequest(CUPS_DELETE_CLASS);
    construct_uri(classuri, sizeof(classuri),
                  "ipp://localhost/classes/", classname);
    ippAddString(request, IPP_TAG_OPERATION, IPP_TAG_URI,
                 "printer-uri", nullptr, classuri);
    answer = cupsDoRequest(this->http, request, "/admin/");
    checkIppAnswer(answer, nullptr);
    ippDelete(answer);
}

void Connection::enablePrinter(const char *name, const char *reason)
{
    return do_printer_request(name, reason, IPP_RESUME_PRINTER);
}

void Connection::disablePrinter(const char *name, const char *reason)
{
    return do_printer_request(name, reason, IPP_PAUSE_PRINTER);
}

void Connection::acceptJobs(const char *name, const char *reason)
{
    return do_printer_request(name, reason, CUPS_ACCEPT_JOBS);
}

void Connection::rejectJobs(const char *name, const char *reason)
{
    return do_printer_request(name, reason, CUPS_REJECT_JOBS);
}

string Connection::getDefault(void)
{
    const char *def = nullptr;
    debugprintf("-> Connection::getDefault()\n");
    def = cupsGetDefault2(this->http);
    if (def == nullptr) {
        debugprintf("<- Connection::getDefault() = None\n");
    } else {
        debugprintf("<- Connection::getDefault() = \"%s\"\n", def);
    }

    return def != nullptr ? string(def) : string();
}

void Connection::setDefault(const char *name, const char *reason)
{
    return do_printer_request(name, reason, CUPS_SET_DEFAULT);
}

string Connection::getPPD(const char *printer)
{
    const char *ppdfile = nullptr;

    debugprintf("-> Connection::getPPD()\n");
    ppdfile = cupsGetPPD2(this->http, printer);
    if (!ppdfile) {
        ipp_status_t err = cupsLastError();
        if (err) {
            string err2 = get_ipp_error(err, cupsLastErrorString());
            throw runtime_error(err2);
        } else {
            throw runtime_error("cupsGetPPD2 failed");
        }

        debugprintf("<- Connection::getPPD()(error)\n");
        return string();
    }

    debugprintf("<- Connection::getPPD() = %s\n", ppdfile);

    return ppdfile;
}

#ifdef HAVE_CUPS_1_4
string Connection::getPPD3(const char *printer, time_t *modtime, const char *filename)
{
    char fname[PATH_MAX];
    http_status_t status;

    if (filename) {
        if (strlen(filename) > sizeof(fname)) {
            throw invalid_argument("overlength filename");
        }
        strcpy(fname, filename);
    } else {
        fname[0] = '\0';
    }

    debugprintf("-> Connection::getPPD3()\n");
    status = cupsGetPPD3(this->http, printer, modtime,
                         fname, sizeof(fname));
    debugprintf("<- Connection::getPPD3() = (%d,%ld,%s)\n",
                status, *modtime, fname);
    if (status != HTTP_STATUS_OK && status != HTTP_STATUS_NOT_MODIFIED) {
        set_http_error(status);
    }

    return fname;
}
#endif /* HAVE_CUPS_1_4 */

int Connection::printTestPage(const char *printer, const char *file,
                              const char *title, const char *format, const char *user)
{
    const char *datadir = nullptr;
    char filename[PATH_MAX];
    char uri[HTTP_MAX_URI];
    ipp_t *request = nullptr, *answer = nullptr;
    ipp_attribute_t *attr = nullptr;
    char *resource = nullptr;
    int jobid = 0;
    int i;

    if (!file) {
        const char *testprint[] = {"%s/data/testprint",
                                   "%s/data/testprint.ps",
                                   nullptr
                                  };
        if ((datadir = getenv("CUPS_DATADIR")) != nullptr) {
            const char **pattern = nullptr;
            for (pattern = testprint; *pattern != nullptr; pattern++) {
                snprintf(filename, sizeof(filename), *pattern, datadir);
                if (access(filename, R_OK) == 0) {
                    break;
                }
            }
        } else {
            const char *const dirs[] = {"/usr/share/cups",
                                        "/usr/local/share/cups",
                                        nullptr
                                       };
            int found = 0;
            int i;
            for (i = 0; (datadir = dirs[i]) != nullptr; i++) {
                const char **pattern = nullptr;
                for (pattern = testprint; *pattern != nullptr; pattern++) {
                    snprintf(filename, sizeof(filename), *pattern, datadir);
                    if (access(filename, R_OK) == 0) {
                        found = 1;
                        break;
                    }
                }

                if (found) {
                    break;
                }
            }

            if (datadir == nullptr) {
                /* We haven't found the testprint.ps file, so just pick a path
                 * and try it.  This will certainly fail with
                 * client-error-not-found, but we'll let that happen rather
                 * than raising an exception so as to be consistent with the
                 * case where CUPS_DATADIR is set and we trust it. */
                snprintf(filename, sizeof(filename), testprint[0], dirs[0]);
            }
        }

        file = filename;
    }

    if (!title) {
        title = "Test Page";
    }

    if (!user) {
        user = (char *)cupsUser();
    }

    construct_uri(uri, sizeof(uri),
                  "ipp://localhost/printers/", printer);
    resource = uri + strlen("ipp://localhost");
    for (i = 0; i < 2; i++) {
        request = ippNewRequest(IPP_PRINT_JOB);
        ippAddString(request, IPP_TAG_OPERATION, IPP_TAG_URI, "printer-uri",
                     nullptr, uri);
        ippAddString(request, IPP_TAG_OPERATION, IPP_TAG_NAME,
                     "requesting-user-name", nullptr, user);
        ippAddString(request, IPP_TAG_OPERATION, IPP_TAG_NAME, "job-name",
                     nullptr, title);
        if (format) {
            ippAddString(request, IPP_TAG_JOB, IPP_TAG_MIMETYPE, "document-format",
                         nullptr, format);
        }

        answer = cupsDoFileRequest(this->http, request, resource, file);
        if (answer && ippGetStatusCode(answer) == IPP_NOT_POSSIBLE) {
            ippDelete(answer);
            // Perhaps it's a class, not a printer.
            construct_uri(uri, sizeof(uri),
                          "ipp://localhost/classes/", printer);
        } else {
            break;
        }
    }

    checkIppAnswer(answer, nullptr);

    attr = ippFindAttribute(answer, "job-id", IPP_TAG_INTEGER);
    if (attr) {
        jobid = ippGetInteger(attr, 0);
    }
    ippDelete(answer);

    return jobid;
}

void Connection::adminExportSamba(const char *name,
                                  const char *server, const char *user, const char *password)
{
    int ret;
    char ppdfile[1024];
    FILE *tmpfile(void);
    FILE *tf = nullptr;
    char str[80];

    if (!name || !server || !user || !password) {
        throw runtime_error("name, samba_server, samba_username, samba_password "
                            "must be specified");
    }

    if (!cupsAdminCreateWindowsPPD(this->http, name, ppdfile, sizeof(ppdfile))) {
        throw runtime_error("No PPD file found for the printer");
    }

    debugprintf("-> Connection::adminExportSamba()\n");
    tf = tmpfile();
    ret = cupsAdminExportSamba(name, ppdfile, server, user, password, tf);
    unlink(ppdfile);

    if (!ret) {
        rewind(tf);
        // Read logfile line by line to get Exit status message on the last line.
        while (fgets(str, sizeof(str), tf) != nullptr) {}
        fclose(tf);
        if (str[strlen(str) - 1] == '\n') {
            str[strlen(str) - 1] = '\0';
        }
        debugprintf("<- Connection::adminExportSamba() EXCEPTION\n");
        throw runtime_error(str);
    }
    fclose(tf);
    debugprintf("<- Connection::adminExportSamba()\n");
}

map<string, string> Connection::adminGetServerSettings(void)
{
    map<string, string> ret;
    int num_settings, i;
    cups_option_t *settings = nullptr;
    debugprintf("-> Connection::adminGetServerSettings()\n");
    cupsAdminGetServerSettings(this->http, &num_settings, &settings);
    for (i = 0; i < num_settings; i++) {
        ret[settings[i].name] = settings[i].value;
    }

    cupsFreeOptions(num_settings, settings);
    debugprintf("<- Connection::adminGetServerSettings()\n");

    return ret;
}

void Connection::adminSetServerSettings(const map<string, string> *dict)
{
    int ret;
    int num_settings = 0;
    cups_option_t *settings = nullptr;

    debugprintf("-> Connection::adminSetServerSettings()\n");
    for (const auto &it : *dict) {
        const char *name = it.first.data();
        const char *value = it.second.data();
        debugprintf("%s: %s\n", name, value);
        num_settings = cupsAddOption(name,
                                     value,
                                     num_settings,
                                     &settings);
    }

    debugprintf("num_settings=%d, settings=%p\n", num_settings, settings);
    ret = cupsAdminSetServerSettings(this->http, num_settings, settings);
    if (!ret) {
        cupsFreeOptions(num_settings, settings);
        debugprintf("<- Connection::adminSetServerSettings() EXCEPTION\n");
        throw runtime_error("Failed to set settings");
    }

    cupsFreeOptions(num_settings, settings);
    debugprintf("<- Connection::adminSetServerSettings()\n");
}

vector<map<string, string>> Connection::getSubscriptions(const char *uri,
                                                         bool my_subscriptions, int job_id)
{
    ipp_t *request = nullptr, *answer = nullptr;
    ipp_attribute_t *attr = nullptr;

    debugprintf("-> Connection::getSubscriptions()\n");
    request = ippNewRequest(IPP_GET_SUBSCRIPTIONS);
    ippAddString(request, IPP_TAG_OPERATION, IPP_TAG_URI,
                 "printer-uri", nullptr, uri);
    ippAddString(request, IPP_TAG_OPERATION, IPP_TAG_NAME,
                 "requesting-user-name", nullptr, cupsUser());

    if (my_subscriptions) {
        ippAddBoolean(request, IPP_TAG_OPERATION, "my-subscriptions", 1);
    }

    if (job_id != -1) {
        ippAddInteger(request, IPP_TAG_OPERATION, IPP_TAG_INTEGER,
                      "job-id", job_id);
    }

    answer = cupsDoRequest(this->http, request, "/");
    checkIppAnswer(answer, nullptr);

    for (attr = ippFirstAttribute(answer); attr; attr = ippNextAttribute(answer)) {
        if (ippGetGroupTag(attr) == IPP_TAG_SUBSCRIPTION) {
            break;
        }
    }

    vector<map<string, string>> ret;
    map<string, string> subscription;
    for (; attr; attr = ippNextAttribute(answer)) {
        string val;
        if (ippGetGroupTag(attr) == IPP_TAG_ZERO) {
            // End of subscription.
            if (!subscription.empty()) {
                ret.push_back(subscription);
                subscription.clear();
            }

            continue;
        }

        const char *attrName = ippGetName(attr);
        if (ippGetCount(attr) > 1 || !strcmp(attrName, "notify-events"))
            val = list_from_attr_values(attr);
        else if (!strcmp(attrName, "notify-user-data")) {
            if (ippGetValueTag(attr) == IPP_TAG_STRING) {
                int length = strlen("dde-printer");
                const char *temp = (char *)ippGetOctetString(attr, 0, &length);
                val = string_format("s%s", temp);
            }
        } else
            val = string_from_attr_value(attr, 0);

        if (val.empty()) {
            // Can't represent this.
            continue;
        }

        subscription[attrName] = val;
    }

    if (!subscription.empty()) {
        ret.push_back(subscription);
    }

    ippDelete(answer);
    debugprintf("<- Connection::getSubscriptions()\n");

    return ret;
}

int Connection::createSubscription(const char *resource_uri,
                                   const vector<string> *events,
                                   int job_id,
                                   const char *recipient_uri,
                                   int lease_duration,
                                   int time_interval,
                                   const char *user_data)
{
    ipp_t *request = nullptr, *answer = nullptr;
    int i = 0;
    ipp_attribute_t *attr = nullptr;
    char **attrs = nullptr; /* initialised to calm compiler */
    size_t n_attrs = 0; /* initialised to calm compiler */

    debugprintf("-> Connection::createSubscription(%s)\n", resource_uri);
    request = ippNewRequest(IPP_CREATE_PRINTER_SUBSCRIPTION);
    ippAddString(request, IPP_TAG_OPERATION, IPP_TAG_URI,
                 "printer-uri", nullptr, resource_uri);
    ippAddString(request, IPP_TAG_SUBSCRIPTION, IPP_TAG_KEYWORD,
                 "notify-pull-method", nullptr, "ippget");
    ippAddString(request, IPP_TAG_SUBSCRIPTION, IPP_TAG_CHARSET,
                 "notify-charset", nullptr, "utf-8");
    ippAddString(request, IPP_TAG_SUBSCRIPTION, IPP_TAG_NAME,
                 "requesting-user-name", nullptr, cupsUser());

    if (recipient_uri) {
        ippAddString(request, IPP_TAG_SUBSCRIPTION, IPP_TAG_URI,
                     "notify-recipient-uri", nullptr, recipient_uri);
    }

    if (user_data) {
        int length = strlen(user_data);
        ippAddOctetString(request, IPP_TAG_SUBSCRIPTION, "notify-user-data", user_data, length);
    }

    if (events) {
        if (get_requested_attrs(events, &n_attrs, &attrs) == -1) {
            ippDelete(request);
            return -1;
        }
        ippAddStrings(request, IPP_TAG_SUBSCRIPTION,
                      IPP_TAG_KEYWORD, "notify-events",
                      n_attrs, nullptr, attrs);
        free_requested_attrs(n_attrs, attrs);
    }

    if (lease_duration != -1) {
        ippAddInteger(request, IPP_TAG_SUBSCRIPTION, IPP_TAG_INTEGER,
                      "notify-lease-duration", lease_duration);
    }

    if (time_interval != -1) {
        ippAddInteger(request, IPP_TAG_SUBSCRIPTION, IPP_TAG_INTEGER,
                      "notify-time-interval", time_interval);
    }

    if (job_id != -1) {
        ippAddInteger(request, IPP_TAG_SUBSCRIPTION, IPP_TAG_INTEGER,
                      "notify-job-id", job_id);
    }

    answer = cupsDoRequest(this->http, request, "/");
    checkIppAnswer(answer, "<- Connection::createSubscription() EXCEPTION\n");

    i = -1;
    for (attr = ippFirstAttribute(answer); attr; attr = ippNextAttribute(answer)) {
        if (ippGetGroupTag(attr) == IPP_TAG_SUBSCRIPTION) {
            ipp_tag_t valTag = ippGetValueTag(attr);
            const char *attrName = ippGetName(attr);
            if (valTag == IPP_TAG_INTEGER && !strcmp(attrName, "notify-subscription-id")) {
                i = ippGetInteger(attr, 0);
            } else if (valTag == IPP_TAG_ENUM && !strcmp(attrName, "notify-status-code")) {
                debugprintf("notify-status-code = %d\n", ippGetInteger(attr, 0));
            }
        }
    }

    ippDelete(answer);
    debugprintf("<- Connection::createSubscription() = %d\n", i);

    return i;
}

vector<map<string, string>> Connection::getNotifications(int subscription_id,
                                                         int sequence_number,
                                                         long *notify_get_interval, long *printer_up_time)
{
    ipp_t *request = nullptr, *answer = nullptr;
    ipp_attribute_t *attr = nullptr;

    debugprintf("-> Connection::getNotifications()\n");
    request = ippNewRequest(IPP_GET_NOTIFICATIONS);
    ippAddString(request, IPP_TAG_OPERATION, IPP_TAG_URI,
                 "printer-uri", nullptr, "/");
    ippAddString(request, IPP_TAG_OPERATION, IPP_TAG_NAME,
                 "requesting-user-name", nullptr, cupsUser());

    ippAddInteger(request, IPP_TAG_OPERATION, IPP_TAG_INTEGER,
                  "notify-subscription-ids", subscription_id);

    if (sequence_number) {
        ippAddInteger(request, IPP_TAG_OPERATION, IPP_TAG_INTEGER,
                      "notify-sequence-numbers", sequence_number);
    }

    answer = cupsDoRequest(this->http, request, "/");
    checkIppAnswer(answer, "<- Connection::getNotifications() EXCEPTION\n");

    // Result-wide attributes.
    attr = ippFindAttribute(answer, "notify-get-interval", IPP_TAG_INTEGER);
    if (attr && notify_get_interval) {
        *notify_get_interval = ippGetInteger(attr, 0);
    }

    attr = ippFindAttribute(answer, "printer-up-time", IPP_TAG_INTEGER);
    if (attr && printer_up_time) {
        *printer_up_time = ippGetInteger(attr, 0);
    }

    for (attr = ippFirstAttribute(answer); attr; attr = ippNextAttribute(answer)) {
        if (ippGetGroupTag(attr) == IPP_TAG_EVENT_NOTIFICATION) {
            break;
        }
    }

    vector<map<string, string>> events;
    map<string, string> event;
    for (; attr; attr = ippNextAttribute(answer)) {
        if (ippGetGroupTag(attr) == IPP_TAG_ZERO) {
            // End of event notification.
            if (!event.empty()) {
                events.push_back(event);
                event.clear();
            }

            continue;
        }

        string val;
        const char *attrName = ippGetName(attr);
        if (ippGetCount(attr) > 1 || !strcmp(attrName, "notify-events") || !strcmp(attrName, "printer-state-reasons") || !strcmp(attrName, "job-printer-state-reasons")) {
            val = list_from_attr_values(attr);
        } else {
            val = string_from_attr_value(attr, 0);
        }

        if (val.empty()) {
            // Can't represent this.
            continue;
        }

        event[attrName] = val;
    }

    if (!event.empty()) {
        events.push_back(event);
    }

    ippDelete(answer);
    debugprintf("<- Connection::getNotifications()\n");

    return events;
}

void Connection::renewSubscription(int id, int lease_duration)
{
    ipp_t *request = nullptr, *answer = nullptr;

    debugprintf("-> Connection::renewSubscription()\n");
    request = ippNewRequest(IPP_RENEW_SUBSCRIPTION);
    ippAddString(request, IPP_TAG_OPERATION, IPP_TAG_URI,
                 "printer-uri", nullptr, "/");
    ippAddString(request, IPP_TAG_OPERATION, IPP_TAG_NAME,
                 "requesting-user-name", nullptr, cupsUser());
    ippAddInteger(request, IPP_TAG_OPERATION, IPP_TAG_INTEGER,
                  "notify-subscription-id", id);

    if (lease_duration != -1) {
        ippAddInteger(request, IPP_TAG_OPERATION, IPP_TAG_INTEGER,
                      "notify-lease-duration", lease_duration);
    }

    answer = cupsDoRequest(this->http, request, "/");
    checkIppAnswer(answer, nullptr);

    ippDelete(answer);
    debugprintf("<- Connection::renewSubscription()\n");
}

void Connection::cancelSubscription(int id)
{
    ipp_t *request = nullptr, *answer = nullptr;

    debugprintf("-> Connection::cancelSubscription()\n");
    request = ippNewRequest(IPP_CANCEL_SUBSCRIPTION);
    ippAddString(request, IPP_TAG_OPERATION, IPP_TAG_URI,
                 "printer-uri", nullptr, "/");
    ippAddString(request, IPP_TAG_OPERATION, IPP_TAG_NAME,
                 "requesting-user-name", nullptr, cupsUser());
    ippAddInteger(request, IPP_TAG_OPERATION, IPP_TAG_INTEGER,
                  "notify-subscription-id", id);

    answer = cupsDoRequest(this->http, request, "/");
    checkIppAnswer(answer, "<- Connection::cancelSubscription() EXCEPTION\n");

    ippDelete(answer);
    debugprintf("<- Connection::cancelSubscription()\n");
}

int Connection::printFile(const char *printer,
                          const char *filename, const char *title,
                          const map<string, string> *options)
{
    int num_settings = 0;
    cups_option_t *settings = nullptr;
    int jobid;

    if (options) {
        for (const auto &it : *options) {
            num_settings = cupsAddOption(it.first.data(),
                                         it.second.data(),
                                         num_settings,
                                         &settings);
        }
    }

    jobid = cupsPrintFile2(this->http, printer, filename, title, num_settings,
                           settings);
    if (jobid == 0) {
        cupsFreeOptions(num_settings, settings);
        string err = get_ipp_error(cupsLastError(), cupsLastErrorString());
        throw runtime_error(err);
    }

    cupsFreeOptions(num_settings, settings);

    return jobid;
}

int Connection::printFiles(const char *printer,
                           const vector<string> *filenames_obj, const char *title,
                           const map<string, string> *options)
{
    int num_filenames;
    char **filenames = nullptr;
    int num_settings = 0;
    cups_option_t *settings = nullptr;
    int jobid;

    num_filenames = filenames_obj->size();
    if (num_filenames == 0) {
        throw runtime_error("filenames list is empty");
    }
    filenames = (char **)malloc(num_filenames * sizeof(char *));
    for (int i = 0; i < num_filenames; ++i) {
        filenames[i] = (char *)filenames_obj->at(i).data();
    }

    if (options) {
        for (const auto &it : *options) {
            num_settings = cupsAddOption(it.first.data(),
                                         it.second.data(),
                                         num_settings,
                                         &settings);
        }
    }

    jobid = cupsPrintFiles2(this->http, printer, num_filenames,
                            (const char **)filenames, title, num_settings,
                            settings);
    free(filenames);
    if (jobid < 0) {
        cupsFreeOptions(num_settings, settings);
        string err = get_ipp_error(cupsLastError(), cupsLastErrorString());
        throw runtime_error(err);
    }

    cupsFreeOptions(num_settings, settings);

    return jobid;
}
