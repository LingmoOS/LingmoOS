// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef HAVE_CUPSMODULE_H
#define HAVE_CUPSMODULE_H

#include <cups/cups.h>
#include <cups/language.h>
#include <cups/adminutil.h>

#include <string>

/* GCC attributes */
#if !defined(__GNUC__) || __GNUC__ < 2 || (__GNUC__ == 2 && __GNUC_MINOR__ < 5) || __STRICT_ANSI__
#define FORMAT(x)
#else /* GNU C: */
#define FORMAT(x) __attribute__((__format__ x))
#endif

extern std::string string_format(const char *fmt, ...);

extern void debugprintf(const char *fmt, ...) FORMAT((__printf__, 1, 2));

#if (CUPS_VERSION_MAJOR > 1) || (CUPS_VERSION_MAJOR == 1 && CUPS_VERSION_MINOR >= 2)
#define HAVE_CUPS_1_2 1
#endif

#if (CUPS_VERSION_MAJOR > 1) || (CUPS_VERSION_MAJOR == 1 && CUPS_VERSION_MINOR >= 3)
#define HAVE_CUPS_1_3 1
#else
#define cupsAdminGetServerSettings _cupsAdminGetServerSettings
#define cupsAdminSetServerSettings _cupsAdminSetServerSettings
#endif

#if (CUPS_VERSION_MAJOR > 1) || (CUPS_VERSION_MAJOR == 1 && CUPS_VERSION_MINOR >= 4)
#define HAVE_CUPS_1_4 1
#endif

#if (CUPS_VERSION_MAJOR > 1) || (CUPS_VERSION_MAJOR == 1 && CUPS_VERSION_MINOR >= 5)
#define HAVE_CUPS_1_5 1
#endif

#if (CUPS_VERSION_MAJOR > 1) || (CUPS_VERSION_MAJOR == 1 && CUPS_VERSION_MINOR >= 6)
#define HAVE_CUPS_1_6 1
#endif

#if (CUPS_VERSION_MAJOR > 1) || (CUPS_VERSION_MAJOR == 1 && CUPS_VERSION_MINOR >= 7)
#define HAVE_CUPS_1_7 1
#endif

#ifndef HAVE_CUPS_1_2
#error cppcups requires CUPS 1.2.x
#endif

class Connection;

using CUPS_PASSWD_CALLBACK = void (*)(const char *prompt,
                                      Connection *conn,
                                      const char *method,
                                      const char *resource,
                                      void *user_data);

struct TLS {
    CUPS_PASSWD_CALLBACK cups_password_callback;
#ifdef HAVE_CUPS_1_4
    void *cups_password_callback_context;
#else /* !HAVE_CUPS_1_4 */
    void *g_current_connection;
#endif /* !HAVE_CUPS_1_4 */
};

extern struct TLS *get_TLS(void);

#ifdef HAVE_CUPS_1_6
typedef struct
{
    void *cb;
    void *user_data;
} CallbackContext;
#endif /* HAVE_CUPS_1_6 */

#ifndef HAVE_CUPS_1_6
int ippGetBoolean(ipp_attribute_t *attr, int element);
int ippGetCount(ipp_attribute_t *attr);
ipp_tag_t ippGetGroupTag(ipp_attribute_t *attr);
int ippGetInteger(ipp_attribute_t *attr, int element);
const char *ippGetName(ipp_attribute_t *attr);
ipp_op_t ippGetOperation(ipp_t *ipp);
int ippGetRange(ipp_attribute_t *attr, int element, int *uppervalue);
int ippGetResolution(ipp_attribute_t *attr, int element,
                     int *yres, ipp_res_t *units);
ipp_status_t ippGetStatusCode(ipp_t *ipp);
const char *ippGetString(ipp_attribute_t *attr, int element,
                         const char **language);
ipp_tag_t ippGetValueTag(ipp_attribute_t *attr);
ipp_attribute_t *ippFirstAttribute(ipp_t *ipp);
ipp_attribute_t *ippNextAttribute(ipp_t *ipp);
int ippSetInteger(ipp_t *ipp, ipp_attribute_t **attr,
                  int element, int intvalue);
int ippSetOperation(ipp_t *ipp, ipp_op_t op);
ipp_state_t ippGetState(ipp_t *ipp);
int ippSetState(ipp_t *ipp, ipp_state_t state);
int ippSetStatusCode(ipp_t *ipp, ipp_status_t status);
int ippSetString(ipp_t *ipp, ipp_attribute_t **attr,
                 int element, const char *strvalue);
#endif

// global settings
int cups_modelSort(const char *str, const char *str2);
void cups_setUser(const char *user);
void cups_setServer(const char *server);
void cups_setPort(int port);
void cups_setEncryption(int e);
std::string cups_getUser(void);
std::string cups_getServer(void);
int cups_getPort(void);
int cups_getEncryption(void);
void cups_setPasswordCB(CUPS_PASSWD_CALLBACK cb);
void cups_setPasswordCB2(CUPS_PASSWD_CALLBACK cb, void *cb_context);
void cups_ppdSetConformance(int level);

#endif /* HAVE_CUPSMODULE_H */
