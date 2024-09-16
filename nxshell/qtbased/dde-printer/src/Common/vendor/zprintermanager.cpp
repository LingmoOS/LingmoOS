// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "zprintermanager.h"
#include "cupsattrnames.h"
#include "common.h"
#include "config.h"
#include "zdevicemanager.h"
#include "cupsconnection.h"
#include "qtconvert.h"

#include <QStringList>
#include <QProcess>
#include <QFileInfo>
#include <QRegularExpression>

#include <stdlib.h>
#include <string.h>

int GetSystemPrinters()
{
    map<string, map<string, string>> printers;
    map<string, map<string, string>>::iterator itmap;

    try {
        printers = g_cupsConnection->getPrinters();

        for (itmap = printers.begin(); itmap != printers.end(); itmap++) {
            QString name = STQ(itmap->first);
            map<string, string> infomap = itmap->second;

            qCDebug(COMMONMOUDLE) << "printer :" << name;
            dumpStdMapValue(infomap);
        }
    } catch (const std::exception &ex) {
        qCWarning(COMMONMOUDLE) << "Got execpt: " << QString::fromUtf8(ex.what());
        return -1;
    };

    return 0;
}

int RemoveSystemPrinter(QString printername)
{
    ipp_t *request; /* IPP Request */
    char uri[HTTP_MAX_URI]; /* URI for printer/class */

    qCDebug(COMMONMOUDLE) << "delete_printer " << printername;

    request = ippNewRequest(CUPS_DELETE_PRINTER);

    httpAssembleURIf(HTTP_URI_CODING_ALL, uri, sizeof(uri), "ipp", NULL,
                     "localhost", 0, "/printers/%s", STR_T_UTF8(printername));
    ippAddString(request, IPP_TAG_OPERATION, IPP_TAG_URI,
                 "printer-uri", NULL, uri);
    ippAddString(request, IPP_TAG_OPERATION, IPP_TAG_NAME, "requesting-user-name",
                 NULL, cupsUser());
    qCDebug(COMMONMOUDLE) << "uri: " << uri << " user: " << cupsUser();

    ippDelete(cupsDoRequest(CUPS_HTTP_DEFAULT, request, "/admin/"));

    if (cupsLastError() > IPP_STATUS_OK_CONFLICTING) {
        qCWarning(COMMONMOUDLE) << "delete_printer failed: " << cupsLastErrorString();

        return -1;
    }

    return 0;
}

static cups_ptype_t /* O - printer-type value */
get_printer_type(http_t *http, /* I - Server connection */
                 const char *printer, /* I - Printer name */
                 char *uri, /* I - URI buffer */
                 size_t urisize) /* I - Size of URI buffer */
{
    ipp_t *request, /* IPP request */
        *response; /* IPP response */
    ipp_attribute_t *attr; /* printer-type attribute */
    cups_ptype_t type; /* printer-type value */

    httpAssembleURIf(HTTP_URI_CODING_ALL, uri, (int)urisize, "ipp", NULL, "localhost", ippPort(), "/printers/%s", printer);

    request = ippNewRequest(IPP_OP_GET_PRINTER_ATTRIBUTES);
    ippAddString(request, IPP_TAG_OPERATION, IPP_TAG_URI,
                 "printer-uri", NULL, uri);
    ippAddString(request, IPP_TAG_OPERATION, IPP_TAG_KEYWORD,
                 "requested-attributes", NULL, "printer-type");
    ippAddString(request, IPP_TAG_OPERATION, IPP_TAG_NAME,
                 "requesting-user-name", NULL, cupsUser());

    /*
    * Do the request...
    */

    response = cupsDoRequest(http, request, "/");
    if ((attr = ippFindAttribute(response, "printer-type",
                                 IPP_TAG_ENUM))
        != NULL) {
        type = ippGetInteger(attr, 0);

        if (type & CUPS_PRINTER_CLASS)
            httpAssembleURIf(HTTP_URI_CODING_ALL, uri, (int)urisize, "ipp", NULL, "localhost", ippPort(), "/classes/%s", printer);
    } else
        type = CUPS_PRINTER_LOCAL;

    ippDelete(response);

    return (type);
}

static int /* O - 0 on success, 1 on fail */
set_printer_options(
    http_t *http, /* I - Server connection */
    const char *printer, /* I - Printer */
    int num_options, /* I - Number of options */
    cups_option_t *options /* I - Options */)
{
    ipp_t *request; /* IPP Request */
    char uri[HTTP_MAX_URI];
    int copied_options = 0; /* Copied options? */
    const char *protocol;

    if (0 == num_options || !options)
        return 1;

    for (int i = 0; i < num_options; i++) {
        qCDebug(COMMONMOUDLE) << options[i].name << " " << options[i].value;
    }

    if (get_printer_type(http, printer, uri, sizeof(uri)) & CUPS_PRINTER_CLASS)
        request = ippNewRequest(IPP_OP_CUPS_ADD_MODIFY_CLASS);
    else
        request = ippNewRequest(IPP_OP_CUPS_ADD_MODIFY_PRINTER);

    ippAddString(request, IPP_TAG_OPERATION, IPP_TAG_URI, "printer-uri", NULL, uri);
    ippAddString(request, IPP_TAG_OPERATION, IPP_TAG_NAME, "requesting-user-name", NULL, cupsUser());

    /*
    * Add the options...
    */

    cupsEncodeOptions2(request, num_options, options, IPP_TAG_OPERATION);
    cupsEncodeOptions2(request, num_options, options, IPP_TAG_PRINTER);

    if ((protocol = cupsGetOption("protocol", num_options, options)) != NULL) {
        QString lowerpro = UTF8_T_S(protocol).toLower();
        if (lowerpro == "bcp")
            ippAddString(request, IPP_TAG_PRINTER, IPP_TAG_NAME, "port-monitor",
                         NULL, "bcp");
        else if (lowerpro == "tbcp")
            ippAddString(request, IPP_TAG_PRINTER, IPP_TAG_NAME, "port-monitor",
                         NULL, "tbcp");
    }

    ippDelete(cupsDoRequest(http, request, "/admin/"));

    if (copied_options)
        cupsFreeOptions(num_options, options);

    /*
    * Check the response...
    */

    if (cupsLastError() > IPP_STATUS_OK_CONFLICTING) {
        qCDebug(COMMONMOUDLE) << "Set printer options failed: " << cupsLastErrorString();
        return -1;
    } else
        return (0);
}

int DuplicateSystemPrinter(TDeviceInfo printer, QString newname)
{
    QString strUri;
    int iRet = 0;

    qCDebug(COMMONMOUDLE) << printer.strName << "-->" << newname;
    if (printer.strName == newname)
        return 0;

    //    if (printer.pCupsDest)
    //        strUri = UTF8_T_S(cupsGetOption(CUPS_OP_URI, printer.pCupsDest[0].num_options, printer.pCupsDest[0].options));
    if (strUri.isEmpty())
        strUri = printer.uriList[0];

    printer.strName = newname;

    //    iRet = add_printer(printer, printer.strPPd, strUri);
    //    if (0 == iRet)
    //        iRet = SetSytemPrinterInfo(printer);

    return iRet;
}

int RenameSystemPrinter(TDeviceInfo printer, QString newname)
{
    int iRet = 0;

    qCDebug(COMMONMOUDLE) << printer.strName << "-->" << newname;
    iRet = DuplicateSystemPrinter(printer, newname);
    if (0 == iRet)
        iRet = RemoveSystemPrinter(printer.strName);

    return 0;
}

int SetDefaultPrinter(QString printername)
{
    ipp_t *request; /* IPP Request */
    char uri[HTTP_MAX_URI]; /* URI for printer/class */

    qCDebug(COMMONMOUDLE) << printername;

    httpAssembleURIf(HTTP_URI_CODING_ALL, uri, sizeof(uri), "ipp", NULL,
                     "localhost", 0, "/printers/%s", STR_T_UTF8(printername));

    request = ippNewRequest(IPP_OP_CUPS_SET_DEFAULT);

    ippAddString(request, IPP_TAG_OPERATION, IPP_TAG_URI,
                 "printer-uri", NULL, uri);
    ippAddString(request, IPP_TAG_OPERATION, IPP_TAG_NAME, "requesting-user-name",
                 NULL, cupsUser());

    ippDelete(cupsDoRequest(CUPS_HTTP_DEFAULT, request, "/admin/"));

    if (cupsLastError() > IPP_STATUS_OK_CONFLICTING) {
        qCWarning(COMMONMOUDLE) << cupsLastErrorString();

        return (1);
    }

    return (0);
}

const char *GetDefaultPrinter()
{
    return cupsGetDefault();
}

int SetPrinterShared(QString printername, bool bShared)
{
    char name[32];
    char value[8];
    cups_option_t option;

    memset(name, 0, sizeof(name));
    strcpy(name, CUPS_OP_ISSHAED);
    option.name = name;
    memset(value, 0, sizeof(value));
    strcpy(value, bShared ? "true" : "false");
    option.value = value;

    qCDebug(COMMONMOUDLE) << printername << " " << option.value;
    return set_printer_options(CUPS_HTTP_DEFAULT, STR_T_UTF8(printername), 1, &option);
}

int setPrinterAcceptingJobs(const char *name, bool bAccept)
{
    char opName[32];
    char value[8];
    cups_option_t option;

    memset(opName, 0, sizeof(name));
    strcpy(opName, CUPS_OP_ISACCEPT);
    option.name = opName;
    memset(value, 0, sizeof(value));
    strcpy(value, bAccept ? "true" : "false");
    option.value = value;

    qCDebug(COMMONMOUDLE) << name << " " << option.value;
    return set_printer_options(CUPS_HTTP_DEFAULT, name, 1, &option);
}

int setPrinterDisable(const char *name, bool bDisable)
{
    char opName[32];
    char value[8];
    cups_option_t option;

    memset(opName, 0, sizeof(name));
    strcpy(opName, CUPS_OP_STATE);
    option.name = opName;
    memset(value, 0, sizeof(value));
    strcpy(value, bDisable ? "5" : "3");
    option.value = value;

    qCDebug(COMMONMOUDLE) << name << " " << option.value;
    return set_printer_options(CUPS_HTTP_DEFAULT, name, 1, &option);
}

DPrinterManager::DPrinterManager()
{
    m_conn = new Connection;

    if (nullptr == m_conn) {
        assert(false && "alloc memery failed");
    }
}

DPrinterManager::~DPrinterManager()
{
    if (nullptr != m_conn) {
        delete m_conn;
        m_conn = nullptr;
    }
}

DPrinterManager *DPrinterManager::getInstance()
{
    static DPrinterManager *instance = nullptr;
    if (nullptr == instance) {
        instance = new DPrinterManager;
    }

    return instance;
}

bool DPrinterManager::InitConnection(const char *host_uri, int port, int encryption)
{
    bool bRet = false;

    try {
        bRet = (m_conn->init(host_uri, port, encryption) == 0);
    } catch (const std::runtime_error &e) {
        qCWarning(COMMONMOUDLE) << "Got execpt: " << QString::fromUtf8(e.what());
        bRet = false;
    }

    return bRet;
}

Connection *DPrinterManager::getConnection()
{
    return m_conn;
}
