/*
    SPDX-FileCopyrightText: 2011 Ilia Kats <ilia-kats@gmx.net>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "openconnectauthworkerthread.h"

#include <QByteArray>
#include <QDesktopServices>
#include <QMutex>
#include <QSemaphore>
#include <QString>
#include <QUrl>
#include <QWaitCondition>

extern "C" {
#include <cerrno>
#include <cstdlib>
}

#include <cstdarg>

class OpenconnectAuthStaticWrapper
{
public:
    static int openWebEngine(struct openconnect_info *vpninfo, const char *loginUri, void *obj)
    {
        if (obj) {
            return static_cast<OpenconnectAuthWorkerThread *>(obj)->openWebEngineP(vpninfo, loginUri, obj);
        }
        return -1;
    }
    static int openUri(struct openconnect_info *vpninfo, const char *loginUri, void *obj)
    {
        if (obj) {
            return static_cast<OpenconnectAuthWorkerThread *>(obj)->openUri(vpninfo, loginUri, obj);
        }
        return -1;
    }
#if OPENCONNECT_CHECK_VER(5, 0)
    static int writeNewConfig(void *obj, const char *str, int num)
    {
        if (obj) {
            return static_cast<OpenconnectAuthWorkerThread *>(obj)->writeNewConfig(str, num);
        }
        return -1;
    }
    static int validatePeerCert(void *obj, const char *str)
    {
        if (obj) {
            return static_cast<OpenconnectAuthWorkerThread *>(obj)->validatePeerCert(nullptr, str);
        }
        return -1;
    }
#else
    static int writeNewConfig(void *obj, char *str, int num)
    {
        if (obj) {
            return static_cast<OpenconnectAuthWorkerThread *>(obj)->writeNewConfig(str, num);
        }
        return -1;
    }
    static int validatePeerCert(void *obj, OPENCONNECT_X509 *cert, const char *str)
    {
        if (obj) {
            return static_cast<OpenconnectAuthWorkerThread *>(obj)->validatePeerCert(cert, str);
        }
        return -1;
    }
#endif
    static int processAuthForm(void *obj, struct oc_auth_form *form)
    {
        if (obj) {
            return static_cast<OpenconnectAuthWorkerThread *>(obj)->processAuthFormP(form);
        }
        return OC_FORM_RESULT_ERR;
    }
    static void writeProgress(void *obj, int level, const char *str, ...)
    {
        if (obj) {
            va_list argPtr;
            va_start(argPtr, str);
            static_cast<OpenconnectAuthWorkerThread *>(obj)->writeProgress(level, str, argPtr);
            va_end(argPtr);
        }
    }
};

OpenconnectAuthWorkerThread::OpenconnectAuthWorkerThread(QMutex *mutex,
                                                         QWaitCondition *waitForUserInput,
                                                         bool *userDecidedToQuit,
                                                         bool *formGroupChanged,
                                                         int cancelFd)
    : QThread()
    , m_mutex(mutex)
    , m_waitForUserInput(waitForUserInput)
    , m_userDecidedToQuit(userDecidedToQuit)
    , m_formGroupChanged(formGroupChanged)
{
    m_openconnectInfo = openconnect_vpninfo_new((char *)"OpenConnect VPN Agent (PlasmaNM - running on KDE)",
                                                OpenconnectAuthStaticWrapper::validatePeerCert,
                                                OpenconnectAuthStaticWrapper::writeNewConfig,
                                                OpenconnectAuthStaticWrapper::processAuthForm,
                                                OpenconnectAuthStaticWrapper::writeProgress,
                                                this);
    openconnect_set_cancel_fd(m_openconnectInfo, cancelFd);
#if OPENCONNECT_CHECK_VER(5, 7)
    openconnect_set_webview_callback(m_openconnectInfo, OpenconnectAuthStaticWrapper::openWebEngine);
#endif
#if OPENCONNECT_CHECK_VER(5, 8)
    openconnect_set_external_browser_callback(m_openconnectInfo, OpenconnectAuthStaticWrapper::openUri);
#endif
}

OpenconnectAuthWorkerThread::~OpenconnectAuthWorkerThread()
{
    openconnect_vpninfo_free(m_openconnectInfo);
}

void OpenconnectAuthWorkerThread::run()
{
    openconnect_init_ssl();
    Q_EMIT initTokens();
    int ret = openconnect_obtain_cookie(m_openconnectInfo);
    if (*m_userDecidedToQuit) {
        return;
    }
    Q_EMIT cookieObtained(ret);
}

struct openconnect_info *OpenconnectAuthWorkerThread::getOpenconnectInfo()
{
    return m_openconnectInfo;
}

int OpenconnectAuthWorkerThread::writeNewConfig(const char *buf, int buflen)
{
    Q_UNUSED(buflen)
    if (*m_userDecidedToQuit) {
        return -EINVAL;
    }
    Q_EMIT writeNewConfig(QString(QByteArray(buf).toBase64()));
    return 0;
}

int OpenconnectAuthWorkerThread::validatePeerCert(void *cert, const char *reason)
{
    if (*m_userDecidedToQuit) {
        return -EINVAL;
    }

#if OPENCONNECT_CHECK_VER(5, 0)
    (void)cert;
    const char *fingerprint = openconnect_get_peer_cert_hash(m_openconnectInfo);
    char *details = openconnect_get_peer_cert_details(m_openconnectInfo);
#else
    char fingerprint[41];
    int ret = 0;

    ret = openconnect_get_cert_sha1(m_openconnectInfo, cert, fingerprint);
    if (ret) {
        return ret;
    }

    char *details = openconnect_get_cert_details(m_openconnectInfo, cert);
#endif
    bool accepted = false;
    m_mutex->lock();
    QString qFingerprint(fingerprint);
    QString qCertinfo(details);
    QString qReason(reason);
    Q_EMIT validatePeerCert(qFingerprint, qCertinfo, qReason, &accepted);
    m_waitForUserInput->wait(m_mutex);
    m_mutex->unlock();
    openconnect_free_cert_info(m_openconnectInfo, details);
    if (*m_userDecidedToQuit) {
        return -EINVAL;
    }

    if (accepted) {
        return 0;
    } else {
        return -EINVAL;
    }
}

int OpenconnectAuthWorkerThread::processAuthFormP(struct oc_auth_form *form)
{
    if (*m_userDecidedToQuit) {
        return -1;
    }

    m_mutex->lock();
    *m_formGroupChanged = false;
    Q_EMIT processAuthForm(form);
    m_waitForUserInput->wait(m_mutex);
    m_mutex->unlock();
    if (*m_userDecidedToQuit) {
        return OC_FORM_RESULT_CANCELLED;
    }

    if (*m_formGroupChanged) {
        return OC_FORM_RESULT_NEWGROUP;
    }
    return OC_FORM_RESULT_OK;
}

void OpenconnectAuthWorkerThread::writeProgress(int level, const char *fmt, va_list argPtr)
{
    if (*m_userDecidedToQuit) {
        return;
    }
    const QString msg = QString::vasprintf(fmt, argPtr);
    Q_EMIT updateLog(msg, level);
}

int OpenconnectAuthWorkerThread::openWebEngineP(__attribute__((unused)) struct openconnect_info *vpninfo,
                                                const char *loginUri,
                                                __attribute__((unused)) void *privdata)
{
    QSemaphore waitForWebEngineFinish;

    Q_EMIT openWebEngine(loginUri, &waitForWebEngineFinish);
    waitForWebEngineFinish.acquire();

    return 0;
}

int OpenconnectAuthWorkerThread::openUri(__attribute__((unused)) struct openconnect_info *vpninfo, const char *loginUri, __attribute__((unused)) void *privdata)
{
    bool opened = QDesktopServices::openUrl(QUrl(loginUri, QUrl::TolerantMode));
    if (!opened) {
        OpenconnectAuthStaticWrapper::writeProgress(this, PRG_ERR, "Failed to invoke QDesktopServices::openUrl.");
        return -1;
    }

    return 0;
}

#include "moc_openconnectauthworkerthread.cpp"
