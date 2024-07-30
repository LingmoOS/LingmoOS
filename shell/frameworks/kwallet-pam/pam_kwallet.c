/*
    SPDX-FileCopyrightText: 2014 Alejandro Fiestas Olivares <afiestas@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include <fcntl.h>
#include <gcrypt.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <grp.h>

#define PAM_SM_PASSWORD
#define PAM_SM_SESSION
#define PAM_SM_AUTH
#include <pwd.h>
#include <sys/stat.h>
#include <sys/syslog.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

/* PAM headers.
 *
 * There are three styles in play:
 *  - Apple, which has no pam_ext.h, does have pam_appl.h, does have pam_syslog
 *  - Linux, which has pam_ext.h, does have pam_appl.h, does have pam_syslog
 *  - BSD, which has no pam_ext.h, does have pam_appl.h, but no pam_syslog
 * In the latter case, #define pam_syslog away.
 */
#ifdef __APPLE__
#include "pam_darwin.h"
#include <security/pam_appl.h>
#else
#include <security/pam_modules.h>
#ifdef HAVE_PAM_EXT
/* "Linux style" */
#include <security/pam_ext.h>
#include <security/_pam_types.h>
#endif
#ifdef HAVE_PAM_APPL
/* "BSD style" .. see also __APPLE__, above */
#include <security/pam_appl.h>
#ifndef HAVE_PAM_EXT
/* FreeBSD has no pam_syslog(), va-macro it away */
#define pam_syslog(...)
#endif
#endif
#endif

#define KWALLET_PAM_KEYSIZE 56
#define KWALLET_PAM_SALTSIZE 56
#define KWALLET_PAM_ITERATIONS 50000

// Parameters
const static char *kdehome = NULL;
const static char *kwalletd = NULL;
const static char *socketPath = NULL;
static int force_run = 0;

const static char * const kwalletPamDataKey = "kwallet5_key";
const static char * const logPrefix = "pam_kwallet5";
const static char * const envVar = "PAM_KWALLET5_LOGIN";

static int argumentsParsed = -1;

int kwallet_hash(pam_handle_t *pamh, const char *passphrase, struct passwd *userInfo, char *key);

static void parseArguments(int argc, const char **argv)
{
    //If already parsed
    if (argumentsParsed != -1) {
        return;
    }

    int x = 0;
    for (;x < argc; ++x) {
        if (strstr(argv[x], "kdehome=") != NULL) {
            kdehome = argv[x] + 8;
        } else if (strstr(argv[x], "kwalletd=") != NULL) {
            kwalletd = argv[x] + 9;
        } else if (strstr(argv[x], "socketPath=") != NULL) {
            socketPath= argv[x] + 11;
        } else if (strcmp(argv[x], "force_run") == 0) {
            force_run = 1;
        }
    }
    if (kdehome == NULL) {
        kdehome = ".local/share";
    }
    if (kwalletd == NULL) {
        kwalletd = KWALLETD_BIN_PATH;
    }
}

static const char* get_env(pam_handle_t *ph, const char *name)
{
    const char *env = pam_getenv (ph, name);
    if (env && env[0]) {
        return env;
    }

    env = getenv (name);
    if (env && env[0]) {
        return env;
    }

    return NULL;
}

static int set_env(pam_handle_t *pamh, const char *name, const char *value)
{
    if (setenv(name, value, 1) < 0) {
        pam_syslog(pamh, LOG_WARNING, "%s: Couldn't setenv %s = %s", logPrefix, name, value);
        //We do not return because pam_putenv might work
    }

    size_t pamEnvSize = strlen(name) + strlen(value) + 2; //2 is for = and \0
    char *pamEnv = malloc(pamEnvSize);
    if (!pamEnv) {
        pam_syslog(pamh, LOG_WARNING, "%s: Impossible to allocate memory for pamEnv", logPrefix);
        return -1;
    }

    snprintf (pamEnv, pamEnvSize, "%s=%s", name, value);
    int ret = pam_putenv(pamh, pamEnv);
    free(pamEnv);

    return ret;
}

/**
 * Code copied from gkr-pam-module.c, GPL2+
 */
static void wipeString(char *str)
{
    if (!str) {
        return;
    }

    const size_t len = strlen (str);
#if HAVE_EXPLICIT_BZERO
    explicit_bzero(str, len);
#else
    volatile char *vp;

    /* Defeats some optimizations */
    memset (str, 0xAA, len);
    memset (str, 0xBB, len);

    /* Defeats others */
    vp = (volatile char*)str;
    while (*vp) {
        *(vp++) = 0xAA;
    }
#endif

    free (str);
}

static int prompt_for_password(pam_handle_t *pamh)
{
    int result;

    //Get the function we have to call
    const struct pam_conv *conv;
    result = pam_get_item(pamh, PAM_CONV, (const void**)&conv);
    if (result != PAM_SUCCESS) {
        return result;
    }

    //prepare the message
    struct pam_message message;
    memset (&message, 0, sizeof(message));
    message.msg_style = PAM_PROMPT_ECHO_OFF;
    message.msg = "Password: ";

    //We only need one message, but we still have to send it in an array
    const struct pam_message *msgs[1];
    msgs[0] = &message;


    //Sending the message, asking for password
    struct pam_response *response = NULL;
    memset (&response, 0, sizeof(response));
    result = (conv->conv) (1, msgs, &response, conv->appdata_ptr);
    if (result != PAM_SUCCESS) {
        goto cleanup;
    }

    //If we got no password, just return;
    if (response[0].resp == NULL) {
        result = PAM_CONV_ERR;
        goto cleanup;
    }

    //Set the password in PAM memory
    char *password = response[0].resp;
    result = pam_set_item(pamh, PAM_AUTHTOK, password);
    wipeString(password);

    if (result != PAM_SUCCESS) {
        goto cleanup;
    }

cleanup:
    free(response);
    return result;
}

static void cleanup_free(pam_handle_t *pamh, void *ptr, int error_status)
{
    free(ptr);
}

static int is_graphical_session(pam_handle_t *pamh)
{
    //Detect a graphical session
    const char *pam_tty = NULL, *pam_xdisplay = NULL,
               *xdg_session_type = NULL;

    pam_get_item(pamh, PAM_TTY, (const void**) &pam_tty);
#ifdef PAM_XDISPLAY
    pam_get_item(pamh, PAM_XDISPLAY, (const void**) &pam_xdisplay);
#endif
    xdg_session_type = get_env(pamh, "XDG_SESSION_TYPE");

    return (pam_xdisplay && strlen(pam_xdisplay) != 0)
           || (pam_tty && pam_tty[0] == ':')
           || (xdg_session_type && strcmp(xdg_session_type, "x11") == 0)
           || (xdg_session_type && strcmp(xdg_session_type, "wayland") == 0);
}

PAM_EXTERN int pam_sm_authenticate(pam_handle_t *pamh, int flags, int argc, const char **argv)
{
    pam_syslog(pamh, LOG_DEBUG, "%s: pam_sm_authenticate\n", logPrefix);
    if (get_env(pamh, envVar) != NULL) {
        pam_syslog(pamh, LOG_INFO, "%s: we were already executed", logPrefix);
        return PAM_IGNORE;
    }

    parseArguments(argc, argv);

    int result;

    //Fetch the user, needed to get user information
    const char *username;
    result = pam_get_user(pamh, &username, NULL);
    if (result != PAM_SUCCESS) {
        pam_syslog(pamh, LOG_ERR, "%s: Couldn't get username %s",
                   logPrefix, pam_strerror(pamh, result));
        return PAM_IGNORE;//Since we are not an essential module, just make pam ignore us
    }

    struct passwd *userInfo;
    userInfo = getpwnam(username);
    if (!userInfo) {
        pam_syslog(pamh, LOG_ERR, "%s: Couldn't get user info (passwd) info", logPrefix);
        return PAM_IGNORE;
    }

    if (userInfo->pw_uid == 0) {
        pam_syslog(pamh, LOG_DEBUG, "%s: Refusing to do anything for the root user", logPrefix);
        return PAM_IGNORE;
    }

    const char *password;
    result = pam_get_item(pamh, PAM_AUTHTOK, (const void**)&password);

    if (result != PAM_SUCCESS) {
        pam_syslog(pamh, LOG_ERR, "%s: Couldn't get password %s", logPrefix,
                   pam_strerror(pamh, result));
        return PAM_IGNORE;
    }

    if (!password) {
        pam_syslog(pamh, LOG_NOTICE, "%s: Couldn't get password (it is empty)", logPrefix);
        //Asking for the password ourselves
        result = prompt_for_password(pamh);
        if (result != PAM_SUCCESS) {
            pam_syslog(pamh, LOG_ERR, "%s: Prompt for password failed %s",
                       logPrefix, pam_strerror(pamh, result)
            );
            return PAM_IGNORE;
        }
    }

    //even though we just set it, better check to be 100% sure
    result = pam_get_item(pamh, PAM_AUTHTOK, (const void**)&password);
    if (result != PAM_SUCCESS || !password) {
        pam_syslog(pamh, LOG_ERR, "%s: Password is not there even though we set it %s", logPrefix,
                   pam_strerror(pamh, result));
        return PAM_IGNORE;
    }

    if (password[0] == '\0') {
        pam_syslog(pamh, LOG_NOTICE, "%s: Empty or missing password, doing nothing", logPrefix);
        return PAM_IGNORE;
    }

    char *key = strdup(password);
    result = pam_set_data(pamh, kwalletPamDataKey, key, cleanup_free);

    if (result != PAM_SUCCESS) {
        free(key);
        pam_syslog(pamh, LOG_ERR, "%s: Impossible to store the password: %s", logPrefix
            , pam_strerror(pamh, result));
        return PAM_IGNORE;
    }

    //TODO unlock kwallet that is already executed
    return PAM_IGNORE;
}

static int drop_privileges(struct passwd *userInfo)
{
    /* When dropping privileges from root, the `setgroups` call will
    * remove any extraneous groups. If we don't call this, then
    * even though our uid has dropped, we may still have groups
    * that enable us to do super-user things. This will fail if we
    * aren't root, so don't bother checking the return value, this
    * is just done as an optimistic privilege dropping function.
    */
    setgroups(0, NULL);

    //Change to the user in case we are not it yet
    if (setgid (userInfo->pw_gid) < 0 || setuid (userInfo->pw_uid) < 0 ||
        setegid (userInfo->pw_gid) < 0 || seteuid (userInfo->pw_uid) < 0) {
        return -1;
    }

    return 0;
}

static void execute_kwallet(pam_handle_t *pamh, struct passwd *userInfo, int toWalletPipe[2], char *fullSocket)
{
    //In the child pam_syslog does not work, using syslog directly

    //keep stderr open so socket doesn't returns us that fd
    int x = 3;
    //Set FD_CLOEXEC on fd that are not of interest of kwallet
    for (; x < 64; ++x) {
        if (x != toWalletPipe[0]) {
            fcntl(x, F_SETFD, FD_CLOEXEC);
        }
    }

    //This is the side of the pipe PAM will send the hash to
    close (toWalletPipe[1]);

    //Change to the user in case we are not it yet
    if (drop_privileges(userInfo) < 0) {
        syslog(LOG_ERR, "%s: could not set gid/uid/euid/egit for kwalletd", logPrefix);
        goto cleanup;
    }

    int envSocket;
    if ((envSocket = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        syslog(LOG_ERR, "%s: couldn't create socket", logPrefix);
        goto cleanup;
    }

    struct sockaddr_un local = {};
    local.sun_family = AF_UNIX;

    if (strlen(fullSocket) > sizeof(local.sun_path)) {
        syslog(LOG_ERR, "%s: socket path %s too long to open",
                   logPrefix, fullSocket);
        free(fullSocket);
        goto cleanup;
    }
    strcpy(local.sun_path, fullSocket);
    unlink(local.sun_path);//Just in case it exists from a previous login

    syslog(LOG_DEBUG, "%s: final socket path: %s", logPrefix, local.sun_path);

    if (bind(envSocket, (struct sockaddr *)&local, sizeof(local)) == -1) {
        syslog(LOG_INFO, "%s-kwalletd: Couldn't bind to local file\n", logPrefix);
        goto cleanup;
    }

    if (listen(envSocket, 5) == -1) {
        syslog(LOG_INFO, "%s-kwalletd: Couldn't listen in socket: %d-%s\n", logPrefix, errno, strerror(errno));
        goto cleanup;
    }
    //finally close stderr
    close(2);

    // Fork twice to daemonize kwallet
    setsid();
    pid_t pid = fork();
    if (pid != 0) {
        if (pid == -1) {
            exit(EXIT_FAILURE);
        } else {
            exit(0);
        }
    }

    //TODO use a pam argument for full path kwalletd
    char pipeInt[4];
    sprintf(pipeInt, "%d", toWalletPipe[0]);
    char sockIn[4];
    sprintf(sockIn, "%d", envSocket);

    char *args[] = {strdup(kwalletd), "--pam-login", pipeInt, sockIn, NULL, NULL};
    execve(args[0], args, pam_getenvlist(pamh));
    syslog(LOG_ERR, "%s: could not execute kwalletd from %s", logPrefix, kwalletd);

cleanup:
    exit(EXIT_FAILURE);
}

static int better_write(int fd, const char *buffer, int len)
{
    size_t writtenBytes = 0;
    while(writtenBytes < len) {
        ssize_t result = write(fd, buffer + writtenBytes, len - writtenBytes);
        if (result < 0) {
            if (errno != EAGAIN && errno != EINTR) {
                return -1;
            }
        }
        writtenBytes += result;
    }

    return writtenBytes;
}

static void start_kwallet(pam_handle_t *pamh, struct passwd *userInfo, const char *kwalletKey)
{
    //Just in case we get broken pipe, do not break the pam process..
    struct sigaction sigPipe, oldSigPipe;
    memset (&sigPipe, 0, sizeof (sigPipe));
    memset (&oldSigPipe, 0, sizeof (oldSigPipe));
    sigPipe.sa_handler = SIG_IGN;
    sigaction (SIGPIPE, &sigPipe, &oldSigPipe);

    int toWalletPipe[2] = { -1, -1};
    if (pipe(toWalletPipe) < 0) {
        pam_syslog(pamh, LOG_ERR, "%s: Couldn't create pipes", logPrefix);
    }

    const char *socketPrefix = "kwallet5";

    char *fullSocket = NULL;
    if (socketPath) {
        size_t needed = snprintf(NULL, 0, "%s/%s_%s%s", socketPath, socketPrefix, userInfo->pw_name, ".socket");
        needed += 1;
        fullSocket = malloc(needed);
        snprintf(fullSocket, needed, "%s/%s_%s%s", socketPath, socketPrefix, userInfo->pw_name, ".socket");
    } else {
        socketPath = get_env(pamh, "XDG_RUNTIME_DIR");
        // Check whether XDG_RUNTIME_DIR is usable
        if (socketPath) {
            struct stat rundir_stat;
            if (stat(socketPath, &rundir_stat) != 0) {
                pam_syslog(pamh, LOG_ERR, "%s: Failed to stat %s", logPrefix, socketPath);
                socketPath = NULL;
            } else if(!S_ISDIR(rundir_stat.st_mode) || (rundir_stat.st_mode & ~S_IFMT) != 0700
                      || rundir_stat.st_uid != userInfo->pw_uid) {
                pam_syslog(pamh, LOG_ERR, "%s: %s has wrong type, perms or ownership", logPrefix, socketPath);
                socketPath = NULL;
            }
        }

        if (socketPath) {
            size_t needed = snprintf(NULL, 0, "%s/%s%s", socketPath, socketPrefix, ".socket");
            needed += 1;
            fullSocket = malloc(needed);
            snprintf(fullSocket, needed, "%s/%s%s", socketPath, socketPrefix, ".socket");
        } else {
            size_t needed = snprintf(NULL, 0, "/tmp/%s_%s%s", socketPrefix, userInfo->pw_name, ".socket");
            needed += 1;
            fullSocket = malloc(needed);
            snprintf(fullSocket, needed, "/tmp/%s_%s%s", socketPrefix, userInfo->pw_name, ".socket");
        }
    }

    int result = set_env(pamh, envVar, fullSocket);
    if (result != PAM_SUCCESS) {
        pam_syslog(pamh, LOG_ERR, "%s: Impossible to set %s env, %s",
                   logPrefix, envVar, pam_strerror(pamh, result));
        free(fullSocket);
        return;
    }

    pid_t pid;
    int status;
    switch (pid = fork ()) {
    case -1:
        pam_syslog(pamh, LOG_ERR, "%s: Couldn't fork to execv kwalletd", logPrefix);
        free(fullSocket);
        return;

    //Child fork, will contain kwalletd
    case 0:
        execute_kwallet(pamh, userInfo, toWalletPipe, fullSocket);
        /* Should never be reached */
        break;

    //Parent
    default:
        waitpid(pid, &status, 0);
        if (status != 0) {
            pam_syslog(pamh, LOG_ERR, "%s: Couldn't fork to execv kwalletd", logPrefix);
            return;
        }
        break;
    };

    free(fullSocket);

    close(toWalletPipe[0]);//Read end of the pipe, we will only use the write
    if (better_write(toWalletPipe[1], kwalletKey, KWALLET_PAM_KEYSIZE) < 0) {
        pam_syslog(pamh, LOG_ERR, "%s: Impossible to write walletKey to walletPipe", logPrefix);
        return;
    }

    close(toWalletPipe[1]);
}

PAM_EXTERN int pam_sm_open_session(pam_handle_t *pamh, int flags, int argc, const char **argv)
{
    pam_syslog(pamh, LOG_DEBUG, "%s: pam_sm_open_session\n", logPrefix);

    if (get_env(pamh, envVar) != NULL) {
        pam_syslog(pamh, LOG_INFO, "%s: we were already executed", logPrefix);
        return PAM_SUCCESS;
    }

    parseArguments(argc, argv);

    if (!force_run && !is_graphical_session(pamh)) {
        pam_syslog(pamh, LOG_INFO, "%s: not a graphical session, skipping. Use force_run parameter to ignore this.", logPrefix);
        return PAM_IGNORE;
    }

    //Fetch the user, needed to get user information
    const char *username;
    int result = pam_get_user(pamh, &username, NULL);
    if (result != PAM_SUCCESS) {
        pam_syslog(pamh, LOG_ERR, "%s: Couldn't get username %s",
                   logPrefix, pam_strerror(pamh, result));
        return PAM_IGNORE;//Since we are not an essential module, just make pam ignore us
    }

    struct passwd *userInfo;
    userInfo = getpwnam(username);
    if (!userInfo) {
        pam_syslog(pamh, LOG_ERR, "%s: Couldn't get user info (passwd) info", logPrefix);
        return PAM_IGNORE;
    }

    if (userInfo->pw_uid == 0) {
        pam_syslog(pamh, LOG_DEBUG, "%s: Refusing to do anything for the root user", logPrefix);
        return PAM_IGNORE;
    }

    char *password;
    result = pam_get_data(pamh, kwalletPamDataKey, (const void **)&password);

    if (result != PAM_SUCCESS) {
        pam_syslog(pamh, LOG_INFO, "%s: open_session called without %s", logPrefix, kwalletPamDataKey);
        return PAM_IGNORE;
    }

    char *key = malloc(KWALLET_PAM_KEYSIZE);
    if (!key || kwallet_hash(pamh, password, userInfo, key) != 0) {
        free(key);
        pam_syslog(pamh, LOG_ERR, "%s: Fail into creating the hash", logPrefix);
        return PAM_IGNORE;
    }

    start_kwallet(pamh, userInfo, key);

    return PAM_SUCCESS;
}

PAM_EXTERN int pam_sm_close_session(pam_handle_t *pamh, int flags, int argc, const char **argv)
{
    pam_syslog(pamh, LOG_DEBUG, "%s: pam_sm_close_session", logPrefix);
    return PAM_SUCCESS;
}

PAM_EXTERN int pam_sm_setcred(pam_handle_t *pamh, int flags, int argc, const char **argv)
{
    pam_syslog(pamh, LOG_DEBUG, "%s: pam_sm_setcred", logPrefix);
    return PAM_SUCCESS;
}


PAM_EXTERN int pam_sm_chauthtok(pam_handle_t *pamh, int flags, int argc, const char **argv)
{
    pam_syslog(pamh, LOG_DEBUG, "%s: pam_sm_chauthtok", logPrefix);
    return PAM_SUCCESS;
}

static int mkpath(char *path)
{
    struct stat sb;
    char *slash;
    int done = 0;

    slash = path;

    while (!done) {
        slash += strspn(slash, "/");
        slash += strcspn(slash, "/");

        done = (*slash == '\0');
        *slash = '\0';

        if (stat(path, &sb)) {
            if (errno != ENOENT || (mkdir(path, 0777) &&
                errno != EEXIST)) {
                syslog(LOG_ERR, "%s: Couldn't create directory: %s because: %d-%s", logPrefix, path, errno, strerror(errno));
                return (-1);
            }
        } else if (!S_ISDIR(sb.st_mode)) {
            return (-1);
        }

        *slash = '/';
    }

    return (0);
}

static void createNewSalt(pam_handle_t *pamh, const char *path, struct passwd *userInfo)
{
    const pid_t pid = fork();
    if (pid == -1) {
        pam_syslog(pamh, LOG_ERR, "%s: Couldn't fork to create salt file", logPrefix);
    } else if (pid == 0) {
        // Child process
        if (drop_privileges(userInfo) < 0) {
            syslog(LOG_ERR, "%s: could not set gid/uid/euid/egit for salt file creation", logPrefix);
            exit(-1);
        }

        // Don't re-create it if it already exists
        struct stat info;
        if (stat(path, &info) == 0 &&
            info.st_size != 0 &&
            S_ISREG(info.st_mode)) {
            exit(0);
        }

        unlink(path);//in case the file already exists

        char *dir = strdup(path);
        dir[strlen(dir) - 14] = '\0';//remove kdewallet.salt
        mkpath(dir); //create the path in case it does not exists
        free(dir);

        char *salt = gcry_random_bytes(KWALLET_PAM_SALTSIZE, GCRY_STRONG_RANDOM);
        const int fd = open(path, O_CREAT | O_WRONLY | O_TRUNC | O_CLOEXEC, 0600);

        //If the file can't be created
        if (fd == -1) {
            syslog(LOG_ERR, "%s: Couldn't open file: %s because: %d-%s", logPrefix, path, errno, strerror(errno));
            exit(-2);
        }

        const ssize_t wlen = write(fd, salt, KWALLET_PAM_SALTSIZE);
        close(fd);
        if (wlen != KWALLET_PAM_SALTSIZE) {
            syslog(LOG_ERR, "%s: Short write to file: %s", logPrefix, path);
            unlink(path);
            exit(-2);
        }

        exit(0); // success
    } else {
        // pam process, just wait for child to finish
        int status;
        waitpid(pid, &status, 0);
        if (status != 0) {
            pam_syslog(pamh, LOG_ERR, "%s: Couldn't create salt file", logPrefix);
        }
    }
}

static int readSaltFile(pam_handle_t *pamh, char *path, struct passwd *userInfo, char *saltOut)
{
    int readSaltPipe[2];
    if (pipe(readSaltPipe) < 0) {
        pam_syslog(pamh, LOG_ERR, "%s: Couldn't create read salt pipes", logPrefix);
        return 0;
    }

    const pid_t pid = fork();
    if (pid == -1) {
        syslog(LOG_ERR, "%s: Couldn't fork to read salt file", logPrefix);
        close(readSaltPipe[0]);
        close(readSaltPipe[1]);
        return 0;
    } else if (pid == 0) {
        // Child process
        close(readSaltPipe[0]); // we won't be reading from the pipe
        if (drop_privileges(userInfo) < 0) {
            syslog(LOG_ERR, "%s: could not set gid/uid/euid/egit for salt file reading", logPrefix);
            free(path);
            close(readSaltPipe[1]);
            exit(-1);
        }

        struct stat info;
        if (stat(path, &info) != 0 || info.st_size == 0 || !S_ISREG(info.st_mode)) {
            syslog(LOG_ERR, "%s: Failed to ensure %s looks like a salt file", logPrefix, path);
            free(path);
            close(readSaltPipe[1]);
            exit(-1);
        }

        const int fd = open(path, O_RDONLY | O_CLOEXEC);
        if (fd == -1) {
            syslog(LOG_ERR, "%s: Couldn't open file: %s because: %d-%s", logPrefix, path, errno, strerror(errno));
            free(path);
            close(readSaltPipe[1]);
            exit(-1);
        }
        free(path);
        char salt[KWALLET_PAM_SALTSIZE] = {};
        const ssize_t bytesRead = read(fd, salt, KWALLET_PAM_SALTSIZE);
        close(fd);
        if (bytesRead != KWALLET_PAM_SALTSIZE) {
            syslog(LOG_ERR, "%s: Couldn't read the full salt file contents from file. %zd:%d", logPrefix, bytesRead, KWALLET_PAM_SALTSIZE);
            exit(-1);
        }

        const ssize_t written = better_write(readSaltPipe[1], salt, KWALLET_PAM_SALTSIZE);

        close(readSaltPipe[1]);
        if (written != KWALLET_PAM_SALTSIZE) {
            syslog(LOG_ERR, "%s: Couldn't write the full salt file contents to pipe", logPrefix);
            exit(-1);
        }

        exit(0);
    }

    close(readSaltPipe[1]); // we won't be writing from the pipe

    // pam process, just wait for child to finish
    int status;
    waitpid(pid, &status, 0);
    int success = 1;
    if (status == 0) {
        const ssize_t readBytes = read(readSaltPipe[0], saltOut, KWALLET_PAM_SALTSIZE);
        if (readBytes != KWALLET_PAM_SALTSIZE) {
            pam_syslog(pamh, LOG_ERR, "%s: Couldn't read the full salt file contents from pipe", logPrefix);
            success = 0;
        }
    } else {
        pam_syslog(pamh, LOG_ERR, "%s: Couldn't read salt file", logPrefix);
        success = 0;
    }

    close(readSaltPipe[0]);

    return success;
}

int kwallet_hash(pam_handle_t *pamh, const char *passphrase, struct passwd *userInfo, char *key)
{
    if (!gcry_check_version("1.5.0")) {
        syslog(LOG_ERR, "%s-kwalletd: libcrypt version is too old", logPrefix);
        return 1;
    }

    struct stat info;
    if (stat(userInfo->pw_dir, &info) != 0 || !S_ISDIR(info.st_mode)) {
        syslog(LOG_ERR, "%s-kwalletd: user home folder does not exist", logPrefix);
        return 1;
    }

    const char *fixpath = "kwalletd/kdewallet.salt";
    size_t pathSize = strlen(userInfo->pw_dir) + strlen(kdehome) + strlen(fixpath) + 3;//3 == /, / and \0
    char *path = (char*) malloc(pathSize);
    sprintf(path, "%s/%s/%s", userInfo->pw_dir, kdehome, fixpath);

    createNewSalt(pamh, path, userInfo);

    char salt[KWALLET_PAM_SALTSIZE] = {};
    const int readSaltSuccess = readSaltFile(pamh, path, userInfo, salt);
    free(path);
    if (!readSaltSuccess) {
        syslog(LOG_ERR, "%s-kwalletd: Couldn't create or read the salt file", logPrefix);
        return 1;
    }

    gcry_error_t error;

    /* We cannot call GCRYCTL_INIT_SECMEM as it drops privileges if getuid() != geteuid().
     * PAM modules are in many cases executed through setuid binaries, which this call
     * would break.
     * It was never effective anyway as neither key nor passphrase are in secure memory,
     * which is a prerequisite for secure operation...
    error = gcry_control(GCRYCTL_INIT_SECMEM, 32768, 0);
    if (error != 0) {
        free(salt);
        syslog(LOG_ERR, "%s-kwalletd: Can't get secure memory: %d", logPrefix, error);
        return 1;
    }
    */

    gcry_control (GCRYCTL_INITIALIZATION_FINISHED, 0);

    error = gcry_kdf_derive(passphrase, strlen(passphrase),
                            GCRY_KDF_PBKDF2, GCRY_MD_SHA512,
                            salt, KWALLET_PAM_SALTSIZE,
                            KWALLET_PAM_ITERATIONS,KWALLET_PAM_KEYSIZE, key);

    return (int) error; // gcry_kdf_derive returns 0 on success
}
