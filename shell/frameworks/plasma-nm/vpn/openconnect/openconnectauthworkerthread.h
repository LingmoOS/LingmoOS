/*
    SPDX-FileCopyrightText: 2011 Ilia Kats <ilia-kats@gmx.net>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef OPENCONNECTAUTHWORKERTHREAD_H
#define OPENCONNECTAUTHWORKERTHREAD_H

extern "C" {
#include <openconnect.h>
}

#if OPENCONNECT_CHECK_VER(3, 0)
#define NEWGROUP_SUPPORTED 1
#define AUTHGROUP_OPT(form) (void *)(form)->authgroup_opt
#define AUTHGROUP_SELECTION(form) (form)->authgroup_selection
#define FORMCHOICE(sopt, i) ((sopt)->choices[i])
#define IGNORE_OPT(opt) ((opt)->flags & OC_FORM_OPT_IGNORE)
#else
#define NEWGROUP_SUPPORTED 0
#define AUTHGROUP_OPT(form) nullptr
#define AUTHGROUP_SELECTION(form) 0
#define FORMCHOICE(sopt, i) (&(sopt)->choices[i])
#define IGNORE_OPT(opt) 0

#define OC_FORM_RESULT_ERR -1
#define OC_FORM_RESULT_OK 0
#define OC_FORM_RESULT_CANCELLED 1
#define OC_FORM_RESULT_NEWGROUP 2
#endif

#if OPENCONNECT_CHECK_VER(4, 0)
#define OC3DUP(x) (x)
#else
#define openconnect_set_option_value(opt, val)                                                                                                                 \
    do {                                                                                                                                                       \
        struct oc_form_opt *_o = (opt);                                                                                                                        \
        free(_o->value);                                                                                                                                       \
        _o->value = strdup(val);                                                                                                                               \
    } while (0)
#define openconnect_free_cert_info(v, x) ::free(x)
#define OC3DUP(x) strdup(x)
#endif

#include <QThread>

class QMutex;
class QSemaphore;
class QWaitCondition;
struct openconnect_info;

class OpenconnectAuthWorkerThread : public QThread
{
    Q_OBJECT
    friend class OpenconnectAuthStaticWrapper;

public:
    OpenconnectAuthWorkerThread(QMutex *, QWaitCondition *, bool *, bool *, int);
    ~OpenconnectAuthWorkerThread() override;
    struct openconnect_info *getOpenconnectInfo();

Q_SIGNALS:
    void validatePeerCert(const QString &, const QString &, const QString &, bool *);
    void processAuthForm(struct oc_auth_form *);
    void updateLog(const QString &, const int &);
    void writeNewConfig(const QString &);
    void cookieObtained(const int &);
    void initTokens();
    void openWebEngine(const char *, QSemaphore *);

protected:
    void run() override;

private:
    int writeNewConfig(const char *, int);
    int validatePeerCert(void *, const char *);
    int processAuthFormP(struct oc_auth_form *);
    void writeProgress(int level, const char *, va_list);
    int openWebEngineP(struct openconnect_info *, const char *, void *);
    int openUri(struct openconnect_info *, const char *, void *);

    QMutex *m_mutex;
    QWaitCondition *m_waitForUserInput;
    bool *m_userDecidedToQuit;
    bool *m_formGroupChanged;
    struct openconnect_info *m_openconnectInfo;
};

#endif
