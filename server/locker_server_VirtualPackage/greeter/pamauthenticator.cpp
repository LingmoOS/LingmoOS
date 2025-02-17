/*
    SPDX-FileCopyrightText: 2020 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "pamauthenticator.h"

#include <QDebug>
#include <QEventLoop>
#include <QMetaMethod>
#include <security/pam_appl.h>

#include "kscreenlocker_greet_logging.h"

class PamWorker : public QObject
{
    Q_OBJECT
public:
    PamWorker();
    ~PamWorker();
    void start(const QString &service, const QString &user);
    void authenticate();

Q_SIGNALS:
    void busyChanged(bool busy);
    void promptForSecret(const QString &msg);
    void prompt(const QString &msg);
    void infoMessage(const QString &msg);
    void errorMessage(const QString &msg);
    void failed();
    void succeeded();

    // internal
    void promptResponseReceived(const QByteArray &prompt);
    void cancelled();

private:
    static int converse(int n, const struct pam_message **msg, struct pam_response **resp, void *data);

    pam_handle_t *m_handle = nullptr; //< the actual PAM handle
    struct pam_conv m_conv;

    bool m_inAuthenticate = false;
    int m_result;
};

int PamWorker::converse(int n, const struct pam_message **msg, struct pam_response **resp, void *data)
{
    PamWorker *c = static_cast<PamWorker *>(data);

    if (!resp) {
        return PAM_BUF_ERR;
    }

    *resp = (struct pam_response *)calloc(n, sizeof(struct pam_response));

    for (int i = 0; i < n; i++) {
        bool isSecret = false;
        switch (msg[i]->msg_style) {
        case PAM_PROMPT_ECHO_OFF: {
            isSecret = true;
            Q_FALLTHROUGH();
        case PAM_PROMPT_ECHO_ON:
            Q_EMIT c->busyChanged(false);

            const QString prompt = QString::fromLocal8Bit(msg[i]->msg);
            if (isSecret) {
                Q_EMIT c->promptForSecret(prompt);
            } else {
                Q_EMIT c->prompt(prompt);
            }

            QByteArray response;
            QEventLoop e;
            QObject::connect(c, &PamWorker::promptResponseReceived, &e, [&](const QByteArray &_response) {
                response = _response;
                e.exit(0);
            });
            QObject::connect(c, &PamWorker::cancelled, &e, [&]() {
                e.exit(PAM_CONV_ERR);
            });

            int rc = e.exec();
            if (rc != 0) {
                return rc;
            }

            Q_EMIT c->busyChanged(true);

            resp[i]->resp = (char *)malloc(response.length() + 1);
            // on error, get rid of everything
            if (!resp[i]->resp) {
                for (int j = 0; j < n; j++) {
                    free(resp[i]->resp);
                    resp[i]->resp = nullptr;
                }
                free(*resp);
                *resp = nullptr;
                return PAM_BUF_ERR;
            }

            memcpy(resp[i]->resp, response.constData(), response.length());
            resp[i]->resp[response.length()] = '\0';

            break;
        }
        case PAM_ERROR_MSG:
            qCDebug(KSCREENLOCKER_GREET) << QString::fromLocal8Bit(msg[i]->msg);
            Q_EMIT c->errorMessage(QString::fromLocal8Bit(msg[i]->msg));
            break;
        case PAM_TEXT_INFO:
            // if there's only the info message, let's predict the prompts too
            qCDebug(KSCREENLOCKER_GREET) << QString::fromLocal8Bit(msg[i]->msg);
            Q_EMIT c->infoMessage(QString::fromLocal8Bit(msg[i]->msg));
        default:
            break;
        }
    }

    return PAM_SUCCESS;
}

PamWorker::PamWorker()
    : QObject(nullptr)
{
    m_conv = {&PamWorker::converse, this};
}

PamWorker::~PamWorker()
{
    if (m_handle) {
        pam_end(m_handle, PAM_SUCCESS);
    }
}

void PamWorker::authenticate()
{
    if (m_inAuthenticate) {
        return;
    }
    m_inAuthenticate = true;
    qCDebug(KSCREENLOCKER_GREET) << "Start auth";
    int rc = pam_authenticate(m_handle, 0); // PAM_SILENT);
    qCDebug(KSCREENLOCKER_GREET) << "Auth done RC" << rc;

    Q_EMIT busyChanged(false);

    if (rc == PAM_SUCCESS) {
        rc = pam_setcred(m_handle, PAM_REFRESH_CRED);
        /* ignore errors on refresh credentials. If this did not work we use the old ones. */
        Q_EMIT succeeded();
    } else {
        Q_EMIT failed();
    }
    m_inAuthenticate = false;
}

static void fail_delay(int retval, unsigned usec_delay, void *appdata_ptr)
{
    Q_UNUSED(retval);
    Q_UNUSED(usec_delay);
    Q_UNUSED(appdata_ptr);
}

void PamWorker::start(const QString &service, const QString &user)
{
    if (user.isEmpty())
        m_result = pam_start(qPrintable(service), nullptr, &m_conv, &m_handle);
    else
        m_result = pam_start(qPrintable(service), qPrintable(user), &m_conv, &m_handle);

    // get errors quicker
#ifdef PAM_FAIL_DELAY
    pam_set_item(m_handle, PAM_FAIL_DELAY, (void *)fail_delay);
#endif

    if (m_result != PAM_SUCCESS) {
        qCWarning(KSCREENLOCKER_GREET) << "[PAM] start" << pam_strerror(m_handle, m_result);
        return;
    } else {
        qCDebug(KSCREENLOCKER_GREET) << "[PAM] Starting...";
    }
}

PamAuthenticator::PamAuthenticator(const QString &service, const QString &user, QObject *parent)
    : QObject(parent)
    , m_signalsToMembers({
          {QMetaMethod::fromSignal(&PamAuthenticator::prompt), m_prompt},
          {QMetaMethod::fromSignal(&PamAuthenticator::promptForSecret), m_promptForSecret},
          {QMetaMethod::fromSignal(&PamAuthenticator::infoMessage), m_infoMessage},
          {QMetaMethod::fromSignal(&PamAuthenticator::errorMessage), m_errorMessage},
      })
{
    d = new PamWorker;

    d->moveToThread(&m_thread);

    connect(&m_thread, &QThread::finished, d, &QObject::deleteLater);

    connect(d, &PamWorker::busyChanged, this, &PamAuthenticator::setBusy);
    connect(d, &PamWorker::prompt, this, [this](const QString &msg) {
        m_prompt = msg;
        Q_EMIT prompt(msg);
    });
    connect(d, &PamWorker::promptForSecret, this, [this](const QString &msg) {
        m_promptForSecret = msg;
        Q_EMIT promptForSecret(msg);
    });
    connect(d, &PamWorker::infoMessage, this, [this](const QString &msg) {
        m_infoMessage = msg;
        Q_EMIT infoMessage(msg);
    });
    connect(d, &PamWorker::errorMessage, this, [this](const QString &msg) {
        m_errorMessage = msg;
        Q_EMIT errorMessage(msg);
    });

    connect(d, &PamWorker::succeeded, this, [this]() {
        m_unlocked = true;
        Q_EMIT succeeded();
    });
    // Failed is not a persistent state. When a view provides authentication that will either result in failure or success,
    // failure simply means that the prompt is getting delayed.
    connect(d, &PamWorker::failed, this, &PamAuthenticator::failed);

    m_thread.start();
    init(service, user);
}

PamAuthenticator::~PamAuthenticator()
{
    cancel();
    m_thread.quit();
    m_thread.wait();
}

void PamAuthenticator::init(const QString &service, const QString &user)
{
    QMetaObject::invokeMethod(d, [this, service, user]() {
        d->start(service, user);
    });
}

bool PamAuthenticator::isBusy() const
{
    return m_busy;
}

void PamAuthenticator::setBusy(bool busy)
{
    if (m_busy != busy) {
        m_busy = busy;
        Q_EMIT busyChanged();
    }
}

bool PamAuthenticator::isUnlocked() const
{
    return m_unlocked;
}

void PamAuthenticator::tryUnlock()
{
    m_unlocked = false;
    QMetaObject::invokeMethod(d, &PamWorker::authenticate);
}

void PamAuthenticator::respond(const QByteArray &response)
{
    QMetaObject::invokeMethod(
        d,
        [this, response]() {
            Q_EMIT d->promptResponseReceived(response);
        },
        Qt::QueuedConnection);
}

void PamAuthenticator::cancel()
{
    m_prompt.clear();
    m_promptForSecret.clear();
    m_infoMessage.clear();
    m_errorMessage.clear();
    QMetaObject::invokeMethod(d, &PamWorker::cancelled);
}

// Force emit the signals when a view connects to them. This prevents a race condition where screens appear after
// stateful signals (such as prompt) had been emitted and end up in an incorrect state.
// (e.g. https://bugs.kde.org/show_bug.cgi?id=456210 where the view ends up in a no-prompt state)
void PamAuthenticator::connectNotify(const QMetaMethod &signal)
{
    // TODO remove this function for Plasma 6. The properties should be bound to  so we don't need to force
    // emit them every time a view connects.

    // NOTE signals are queued because during connect qml is not yet ready to receive them

    if (m_unlocked && signal == QMetaMethod::fromSignal(&PamAuthenticator::succeeded)) {
        signal.invoke(this, Qt::QueuedConnection);
        return;
    }

    for (const auto &[signalMethod, memberString] : m_signalsToMembers)
    {
        if (signal != signalMethod) {
            continue;
        }

        if (!memberString.isNull()) {
            signalMethod.invoke(this, Qt::QueuedConnection, Q_ARG(QString, memberString));
            return;
        }
    }
}

QString PamAuthenticator::getPrompt() const
{
    return m_prompt;
}

QString PamAuthenticator::getPromptForSecret() const
{
    return m_promptForSecret;
}

QString PamAuthenticator::getInfoMessage() const
{
    return m_infoMessage;
}

QString PamAuthenticator::getErrorMessage() const
{
    return m_errorMessage;
}

#include "pamauthenticator.moc"
