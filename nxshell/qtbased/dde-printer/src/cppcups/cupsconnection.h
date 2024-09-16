// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef HAVE_CUPSCONNECTION_H
#define HAVE_CUPSCONNECTION_H

#include <cups/http.h>

#include "cupsppd.h"
#include "cupsmodule.h"

#include <list>
#include <map>
#include <vector>
#include <string>
#include <stdexcept>
using namespace std;

typedef struct tagDest {
    bool is_default;
    string destname;
    string instance;

    map<string, string> options;
} Dest;

class Connection;
class ServerSettings
{
public:
    ServerSettings();
    ServerSettings &enableDebugLogging(bool enabled);
    ServerSettings &enableRemoteAdmin(bool enabled);
    ServerSettings &enableRemoteAny(bool enabled);
    ServerSettings &enableSharePrinters(bool enabled);
    ServerSettings &enableUserCancelAny(bool enabled);

    bool isDebugLoggingEnabled() const;
    bool isRemoteAdminEnabled() const;
    bool isRemoteAnyEnabled() const;
    bool isSharePrintersEnabled() const;
    bool isUserCancelAnyEnabled() const;

    void updateSettings(map<string, string> set);
    void commit(const char *host_str, int port_n, int encryption_n, const map<string, string> &options);

private:
    map<string, string> settings_;
};

class Connection
{
public:
    Connection();
    ~Connection();

    // @description: Initialize the Connection object with connect to cups server.
    // @param host_uri: the uri of cups server will be connecting, can be null value.
    // @param port: the port of cups server listening
    // @param encryption: forces encryption when connecting to the cups server
    // @return: 0, OK; -1,failed;
    // @exception: runtime_error
    int init(const char *host_uri,
             int port,
             int encryption);

    // @description: Get all print destination include printer and class.
    // @return: a vector with Dest elements
    // @exception: none
    vector<Dest> getDests(void);

    // @description: Get all printer, with each printer has these attribute:
    //  "printer-name",
    //  "printer-type",
    //  "printer-location",
    //  "printer-info",
    //  "printer-make-and-model",
    //  "printer-state",
    //  "printer-state-message",
    //  "printer-state-reasons",
    //  "printer-uri-supported",
    //  "device-uri",
    //  "printer-is-shared",
    //
    // @return: a map with the key is printer name, and the value is a attribute map.
    //          The key is the above attribute name, and the value is the attribute value
    //          with a type string format, for attribute map.
    // @type string: the first char is a data type, e.g.
    // b -> bool
    // i -> int
    // l -> long
    // f -> float
    // d -> double
    // s -> raw utf-8 string
    // ` -> type string list
    // @exception: runtime_error
    map<string, map<string, string>> getPrinters(void);

    // @description: Get all the printer class, with each class has a printer list:
    // @return: a map with the key is the class name, and the value is a printer list with
    //          a type string format.
    // @exception: runtime_error
    map<string, string> getClasses(void);

    // @description: Copy ppd file from /usr/share/cups/model/ with the given name to /tmp,
    //               and get the new file name which can use to construct a PPD object.
    // @return: the full path of new temp PPD file
    // @exception: runtime_error
    string getServerPPD(const char *ppd_name);

    // @description: Get all PPD file managed by cups server, to read the PPD file see getServerPPD.
    // @param limit: how many PPD file to get
    // @param exclude_schemes: like http, https, ipp, dnssd, snmp, driverless,socket,...
    // @param include_schemes: like exclude_schemes
    // @return: a map with the key is the ppd name, and the value is a ppd attribute map.
    //          The key of ppd attribute map is the one of these attributes:
    //          "ppd-device-id"
    //          "ppd-make"
    //          "ppd-make-and-model"
    //          "ppd-model-number"
    //          "ppd-natural-language"
    //          "ppd-product"
    //          "ppd-psversion"
    //          "ppd-type"
    //          the value of ppd attribute map is a type string.
    // @exception: runtime_error
    map<string, map<string, string>> getPPDs(int limit,
                                             const vector<string> *exclude_schemes,
                                             const vector<string> *include_schemes,
                                             const char *ppd_natural_language,
                                             const char *ppd_device_id,
                                             const char *ppd_make,
                                             const char *ppd_make_and_model,
                                             int ppd_model_number,
                                             const char *ppd_product,
                                             const char *ppd_psversion,
                                             const char *ppd_type);

    // @description: like getPPDs except the value of ppd attribute map is a type string list
    map<string, map<string, string>> getPPDs2(int limit,
                                              const vector<string> *exclude_schemes,
                                              const vector<string> *include_schemes,
                                              const char *ppd_natural_language,
                                              const char *ppd_device_id,
                                              const char *ppd_make,
                                              const char *ppd_make_and_model,
                                              int ppd_model_number,
                                              const char *ppd_product,
                                              const char *ppd_psversion,
                                              const char *ppd_type);

    // @description: Get the printing file specified by jobid and docnum by printer `uir`
    // @uri: the printer
    // @jobid: the print job id
    // @docnum: the document-number
    // @return: a map with the key is one of these attributes:
    //         "file",
    //         "document-format",
    //         "document-name",
    // @exception: runtime_error
    map<string, string> getDocument(const char *uri,
                                    int jobid,
                                    int docnum);

    // @description: Get all device can auto detected by cups server.
    // @param exclude_schemes: like http, https, ipp, dnssd, snmp, driverless,socket,...
    // @param include_schemes: like exclude_schemes
    // @param limit: how many device to get
    // @param timeout: how many second
    // @return: a map with the key is the device uri, and the value is one of these
    //          attributes:
    //           "device-class",
    //           "device-id",
    //           "device-info",
    //           "device-location",
    //           "device-make-and-model",
    // @exception: runtime_error
    map<string, map<string, string>> getDevices(const vector<string> *exclude_schemes,
                                                const vector<string> *include_schemes,
                                                int limit,
                                                int timeout);

    // @description: Get specified job
    // @param which: job type, can be of these: no-completed, completed, all
    // @param my_jobs: boolean , get current user's print job if set true.
    // @param limit: how many print job to get
    // @param first_job_id:
    // @requested_attrs: can be one or more of these attributes:
    //       "job-id",
    //       "job-printer-uri",
    //       "job-state",
    //       "job-originating-user-name",
    //       "job-k-octets",
    //       "job-name",
    //       "time-at-creation"
    // @return: a map with the key is job-id, the value is the job map.
    //          The key of job map is one of the above attributes, the value of
    //          job map is a type string.
    // @exception: runtime_error
    map<int, map<string, string>> getJobs(const char *which,
                                          int my_jobs,
                                          int limit,
                                          int first_job_id,
                                          const vector<string> *requested_attrs);

    // @param requested_attrs: only get attribute by specified.
    // @exception: runtime_error
    map<string, string> getJobAttributes(int job_id,
                                         const vector<string> *requested_attrs);

    // @param name or param uri must be specified, but can't both specified.
    // @param requested_attrs: only get attribute by specified.
    // @exception: runtime_error
    map<string, string> getPrinterAttributes(const char *name,
                                             const char *uri,
                                             const vector<string> *requested_attrs);

    // @param notify_get_interval: receive the notify-get-interval
    // @param printer_up_time: receive the printer-up-time
    // @return: a map list, every map save all attribute of a notify event.
    // @exception: runtime_error
    vector<map<string, string>> getNotifications(int subscription_id,
                                                 int sequence_number,
                                                 long *notify_get_interval,
                                                 long *printer_up_time);

    // @param uri: printer uri, must be not null.
    // @return: a map list, every map save all attribute of a subscription.
    // @exception: runtime_error
    vector<map<string, string>> getSubscriptions(const char *uri,
                                                 bool my_subscriptions,
                                                 int job_id);

    int holdJob(int job_id);

    int releaseJob(int job_id);

    // @exception: runtime_error
    void cancelJob(int job_id,
                   int purge_job);

    // @param name or param uri must be specified, but can't both specified.
    // @exception: runtime_error
    void cancelAllJobs(const char *name,
                       const char *uri,
                       int my_jobs,
                       int purge_jobs);

    // @param printer: must be not null!
    // @param title: must be not null!
    // @return: id of new job
    // @exception: runtime_error
    int createJob(const char *printer,
                  const char *title,
                  const map<string, string> *options);

    // @description: Add a document to a job specified by jobid.
    // @param doc_name: must be not null!
    // @param format: must be not null!
    // @return: HTTP status code
    // @exception: runtime_error
    int startDocument(const char *printer,
                      int jobid,
                      const char *doc_name,
                      const char *format,
                      int last_document);

    // @description: sending data in [buffer, buffer+length]
    // @return: HTTP status code
    // @exception: runtime_error
    int writeRequestData(const char *buffer,
                         int length);

    // @description: finish sending a document
    // @return: HTTP status code
    // @exception: runtime_error
    int finishDocument(const char *printer);

    // @description: move job specified by job_id or printeruri to new printer specified
    //               by jobprinteruri .
    // @param jobprinteruri: must be not null!
    // @exception: runtime_error
    void moveJob(const char *printeruri,
                 int job_id,
                 const char *jobprinteruri);

    // @description: Authenticate a job specified by job_id
    // @param: auth_info_list is username/password pair
    // @exception: runtime_error
    void authenticateJob(int job_id,
                         const vector<string> *auth_info_list);

    // @param job_hold_until: must be not null!
    // @exception: runtime_error
    void setJobHoldUntil(int job_id,
                         const char *job_hold_until);

    void setJobPriority(int job_id, int iPriority);

    // @param job_hold_until: must be not null!
    // @exception: runtime_error
    void restartJob(int job_id,
                    const char *job_hold_until);

    // @description: Get a file specified by @resource from cups server to file
    //               specified by @fd or @filename.
    // @exception: runtime_error
    void getFile(const char *resource,
                 const char *filename,
                 int fd);

    // @description: put a file specified by @fd or @filename to cups server
    //               specified by @resource.
    // @param resource: like "/admin/conf/cupsd.conf"
    // @exception: runtime_error
    void putFile(const char *resource,
                 const char *filename,
                 int fd);

    // @param name: must be not null!
    // @ppdfile, @ppdname, @ppd, only give one!
    // @exception: runtime_error
    void addPrinter(const char *name,
                    const char *info,
                    const char *location,
                    const char *device,
                    const char *ppdfile,
                    const char *ppdname,
                    PPD *ppd);

    // @param name: must be not null!
    // @param device_uri: must be not null!
    // @exception: runtime_error
    void setPrinterDevice(const char *name,
                          const char *device_uri);

    // @description: Change the description of the printer specified by @name
    // @param name: must be not null!
    // @param info: must be not null!
    // @exception: runtime_error
    void setPrinterInfo(const char *name,
                        const char *info);

    // @param name: must be not null!
    // @param location: must be not null!
    // @exception: runtime_error
    void setPrinterLocation(const char *name,
                            const char *location);

    // @param name: must be not null!
    // @exception: runtime_error
    void setPrinterShared(const char *name,
                          int sharing);

    // @param name: must be not null!
    // @param start: must be not null!
    // @param end: must be not null!
    // @exception: runtime_error
    void setPrinterJobSheets(char *name,
                             char *start,
                             char *end);

    // @param name: must be not null!
    // @param policy: must be not null!
    // @exception: runtime_error
    void setPrinterErrorPolicy(const char *name,
                               const char *policy);

    // @param name: must be not null!
    // @param policy: must be not null!
    // @exception: runtime_error
    void setPrinterOpPolicy(const char *name,
                            const char *policy);

    // @param name: must be not null!
    // @param users: must be not null!
    // @exception: runtime_error
    void setPrinterUsersAllowed(const char *name,
                                const vector<string> *users);

    // @param name: must be not null!
    // @param users: must be not null!
    // @exception: runtime_error
    void setPrinterUsersDenied(const char *name,
                               const vector<string> *users);

    // @description: Change option of the printer specified by @name where are not stored
    //              into the PPD file.
    // @param name: must be not null!
    // @param option: must be not null!
    // @param values: must be not null!
    // @exception: runtime_error
    // refer lpoptions command
    void addPrinterOptionDefault(const char *name,
                                 const char *option,
                                 const vector<string> *values);

    // @param name: must be not null!
    // @param option: must be not null!
    // @exception: runtime_error
    // refer lpoptions command
    void deletePrinterOptionDefault(const char *name,
                                    const char *option);

    // @param name: must be not null!
    // @param reason: must be not null!
    // @exception: runtime_error
    void deletePrinter(const char *name,
                       const char *reason);

    // @param printername: must be not null!
    // @param classname: must be not null!
    // @exception: runtime_error
    void addPrinterToClass(const char *printername,
                           const char *classname);

    // @param printername: must be not null!
    // @param classname: must be not null!
    // @exception: runtime_error
    void deletePrinterFromClass(const char *printername,
                                const char *classname);

    // @param classname: must be not null!
    // @exception: runtime_error
    void deleteClass(const char *classname);

    // @param name: must be not null!
    // @param reason: must be not null!
    // @exception: runtime_error
    void enablePrinter(const char *name,
                       const char *reason);

    // @param name: must be not null!
    // @param reason: must be not null!
    // @exception: runtime_error
    void disablePrinter(const char *name,
                        const char *reason);

    // @param name: must be not null!
    // @param reason: must be not null!
    // @exception: runtime_error
    void acceptJobs(const char *name,
                    const char *reason);

    // @param name: must be not null!
    // @param reason: must be not null!
    // @exception: runtime_error
    void rejectJobs(const char *name,
                    const char *reason);

    // @param name: must be not null!
    // @param reason: must be not null!
    // @exception: runtime_error
    void setDefault(const char *name,
                    const char *reason);

    // @return: the name of the default printer; return empty if failed.
    // @exception: none
    string getDefault(void);

    // @param printer: must be not null!
    // @return: the name of the ppd file match to the specified printer; return empty if failed.
    // @exception: runtime_error
    string getPPD(const char *printer);

    // @param printer: must be not null!
    // @param modtime: specified the modified time, and update if the ppd file has changed.
    //                 must be not null!
    // @param filename: optional. if not empty, the file will be overwritten.
    // @return: the name of the ppd file match to the specified printer; return empty if failed.
    // @exception: runtime_error
    string getPPD3(const char *printer,
                   time_t *modtime,
                   const char *filename);

    // @param ppd_name: must be not null!
    // @param samba_server: must be not null!
    // @param samba_user: must be not null!
    // @param samba_password: must be not null!
    // @exception: runtime_error
    void adminExportSamba(const char *ppd_name,
                          const char *samba_server,
                          const char *samba_user,
                          const char *samba_password);

    // @description: get cups server's global setting
    // @exception: none
    map<string, string> adminGetServerSettings(void);

    // @param settings: must be not null! refer to adminGetServerSettings
    // @exception: runtime_error
    void adminSetServerSettings(const map<string, string> *settings);

    // @param resource_uri: must be not null!
    // @param events: optional. Can be one or more of these:
    //            "printer-added",
    //            "printer-modified",
    //            "printer-deleted",
    //            "printer-state-changed",
    //            "job-created",
    //            "job-completed",
    //            "job-stopped",
    //            "job-progress",
    //            "job-state-changed"
    // @param lease_duration: unit is second
    // @return: subscription id
    // @exception: runtime_error
    int createSubscription(const char *resource_uri,
                           const vector<string> *events,
                           int job_id,
                           const char *recipient_uri,
                           int lease_duration,
                           int time_interval,
                           const char *user_data);

    // @param lease_duration: unit is second
    // @exception: runtime_error
    void renewSubscription(int id,
                           int lease_duration);

    // @exception: runtime_error
    void cancelSubscription(int id);

    // @param printer: must be not null!
    // @return: the new job id
    // @exception: runtime_error
    int printTestPage(const char *printer,
                      const char *file,
                      const char *title,
                      const char *format,
                      const char *user);

    // @param printer: must be not null!
    // @param filename: must be not null!
    // @param title: must be not null!
    // @return: the new job id
    // @exception: runtime_error
    int printFile(const char *printer,
                  const char *filename,
                  const char *title,
                  const map<string, string> *options);

    // @param printer: must be not null!
    // @param filenames: must be not null!
    // @param title: must be not null!
    // @return: the new job id
    // @exception: runtime_error
    int printFiles(const char *printer,
                   const vector<string> *filenames,
                   const char *title,
                   const map<string, string> *options);

private:
    void do_requesting_user_names(const char *name,
                                  const vector<string> *users,
                                  const char *requeststr);

    void do_printer_request(const char *name,
                            const char *reason,
                            ipp_op_t op);

    map<string, map<string, string>> do_getPPDs(int limit,
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
                                                bool all_lists);

public:
    // member
    http_t *http;
    string host;
#ifdef HAVE_CUPS_1_4
    string cb_password; // CUPS_PASSWD_CALLBACK will update this value
#endif /* HAVE_CUPS_1_4 */
    string resource;
};

const char *password_callback_newstyle(const char *prompt,
                                       http_t *http,
                                       const char *method,
                                       const char *resource,
                                       void *user_data);

const char *password_callback_oldstyle(const char *prompt,
                                       http_t *http,
                                       const char *method,
                                       const char *resource,
                                       void *user_data);

#ifdef HAVE_CUPS_1_6
int cups_dest_cb(void *user_data,
                 unsigned flags,
                 cups_dest_t *dest);
#endif /* HAVE_CUPS_1_6 */

string get_ipp_error(ipp_status_t status,
                     const char *message);
#endif /* HAVE_CUPSCONNECTION_H */
