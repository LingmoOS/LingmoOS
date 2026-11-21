// One login, PAM to session end, outside the threaded daemon (greetd's session
// worker, sddm-helper). stdout carries only replies; the session writes stderr.

#include <security/pam_appl.h>

#include <cerrno>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <dirent.h>
#include <fcntl.h>
#include <grp.h>
#include <optional>
#include <pwd.h>
#include <string>
#include <sys/prctl.h>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

#include "base/i18n.h"
#include "base/locale.h"
#include "base/strings.h"

#include "../common/wire.h"

using Lingmo::i18n::tr;
using dm::Json;

static constexpr size_t kMaxLine = 64 * 1024;

// After authentication nobody answers; a session module's motd would hang.
static bool g_conversing = true;

// How long a session may take over SIGTERM before its process group is killed.
static constexpr unsigned kSessionStopSeconds = 10;

// The session's pid until it is reaped: unreaped, it cannot belong to anyone else.
static volatile sig_atomic_t g_session = 0;

// From pam_open_session to the fork, a SIGTERM waits for the session to exist:
// exiting then would leave the PAM session open.
static volatile sig_atomic_t g_opening = 0;
static volatile sig_atomic_t g_stopRequested = 0;

// A PAM transaction is open: a stop must let the main flow end it, so the
// credentials it established are dropped and the transaction is closed.
static volatile sig_atomic_t g_pam = 0;
static volatile sig_atomic_t g_killed = 0;

// SIGTERM ends the session, which this process then closes, or the login before one.
static void onTerminate(int)
{
    if (g_session > 0) {
        kill(pid_t(g_session), SIGTERM);
        if (!g_stopRequested)
            alarm(kSessionStopSeconds);
        g_stopRequested = 1;
    } else if (g_opening || g_pam) {
        g_stopRequested = 1;
    } else {
        _exit(128 + SIGTERM);
    }
}

// setsid made the session its group's leader; the group goes down with it.
static void onStopDeadline(int)
{
    if (g_session > 0) {
        kill(-pid_t(g_session), SIGKILL);
        g_killed = 1;
    }
}

// Whether a process other than `leader` is still in its process group. Zombies
// count: a killed process's main thread is one while its other threads still
// hold its files.
static bool groupHasOthers(pid_t leader)
{
    DIR *proc = opendir("/proc");
    if (!proc)
        return false;

    bool found = false;
    while (const dirent *entry = readdir(proc)) {
        char *end = nullptr;
        const long pid = strtol(entry->d_name, &end, 10);
        if (*end || pid <= 0 || pid == leader)
            continue;

        const std::string path = "/proc/" + std::to_string(pid) + "/stat";
        FILE *file = fopen(path.c_str(), "re");
        if (!file)
            continue;

        char buffer[512];
        const size_t size = fread(buffer, 1, sizeof(buffer) - 1, file);
        fclose(file);
        buffer[size] = 0;

        // The name may hold spaces and parentheses; the fields after it do not.
        const char *name = strrchr(buffer, ')');
        int group = 0;
        if (name && sscanf(name + 1, " %*c %*d %d", &group) == 1 && group == leader) {
            found = true;
            break;
        }
    }

    closedir(proc);
    return found;
}

static std::optional<Json> receive()
{
    // A SIGTERM taken outside read() interrupted nothing; the message is not coming.
    if (g_stopRequested)
        return std::nullopt;

    std::string line;
    char c = 0;
    ssize_t count = 0;
    while ((count = read(STDIN_FILENO, &c, 1)) == 1 || (count < 0 && errno == EINTR)) {
        if (count < 0) {
            // SIGTERM interrupted the read: the message is not coming.
            if (g_stopRequested)
                return std::nullopt;
            continue;
        }

        if (c == '\n')
            break;

        line.push_back(c);
        if (line.size() > kMaxLine)
            return std::nullopt;
    }

    if (line.empty())
        return std::nullopt;

    auto message = dm::decodeLine(line);
    dm::wipe(line);
    return message;
}

static void send(const Json &message)
{
    std::string bytes = dm::encodeLine(message);
    size_t written = 0;
    while (written < bytes.size()) {
        const ssize_t count = write(STDOUT_FILENO, bytes.data() + written, bytes.size() - written);
        if (count < 0 && errno == EINTR)
            continue;
        if (count <= 0)
            break;

        written += size_t(count);
    }
    dm::wipe(bytes);
}

// PAM's text is the system's; the greeter's protocol carries UTF-8.
static std::string text(const char *message)
{
    return Lingmo::base::strings::sanitizedUtf8(message ? message : "");
}

static void fail(bool authentication, const std::string &message)
{
    send(Json::object({{"type", "error"}, {"auth", authentication}, {"message", message}}));
}

static void freeResponses(pam_response *responses, int count)
{
    for (int i = 0; i < count; ++i) {
        if (responses[i].resp) {
            explicit_bzero(responses[i].resp, strlen(responses[i].resp));
            free(responses[i].resp);
        }
    }

    free(responses);
}

// Every PAM message goes to the greeter and waits for its answer, as in greetd.
static int converse(int count, const pam_message **messages, pam_response **result, void *)
{
    if (count <= 0 || count > PAM_MAX_NUM_MSG)
        return PAM_CONV_ERR;

    auto *responses = static_cast<pam_response *>(calloc(size_t(count), sizeof(pam_response)));
    if (!responses)
        return PAM_BUF_ERR;

    for (int i = 0; i < count; ++i) {
        const int kind = messages[i]->msg_style;
        if (!g_conversing) {
            if (kind == PAM_PROMPT_ECHO_OFF || kind == PAM_PROMPT_ECHO_ON) {
                freeResponses(responses, count);
                return PAM_CONV_ERR;
            }

            fprintf(stderr, "PAM: %s\n", messages[i]->msg);
            continue;
        }

        const char *style = nullptr;
        switch (kind) {
        case PAM_PROMPT_ECHO_OFF: style = "secret"; break;
        case PAM_PROMPT_ECHO_ON: style = "visible"; break;
        case PAM_ERROR_MSG: style = "error"; break;
        case PAM_TEXT_INFO: style = "info"; break;
        default:
            freeResponses(responses, count);
            return PAM_CONV_ERR;
        }

        send(Json::object({{"type", "question"}, {"style", style}, {"text", text(messages[i]->msg)}}));

        auto answer = receive();
        if (!answer || (*answer)["type"].toString() != "answer") {
            freeResponses(responses, count);
            return PAM_CONV_ERR;
        }

        if (kind == PAM_PROMPT_ECHO_OFF || kind == PAM_PROMPT_ECHO_ON) {
            std::string response = (*answer)["response"].toString();
            responses[i].resp = strdup(response.c_str());
            dm::wipe(response);
            if (!responses[i].resp) {
                freeResponses(responses, count);
                return PAM_BUF_ERR;
            }
        }

        answer->insert("response", nullptr);
    }

    *result = responses;
    return PAM_SUCCESS;
}

namespace {

struct Account {
    std::string name;
    std::string home;
    std::string shell;
    uid_t uid = 0;
    gid_t gid = 0;
};

} // namespace

static std::optional<Account> account(pam_handle_t *pam)
{
    // PAM_USER, since a module may have changed it.
    const void *item = nullptr;
    if (pam_get_item(pam, PAM_USER, &item) != PAM_SUCCESS || !item)
        return std::nullopt;

    const passwd *entry = getpwnam(static_cast<const char *>(item));
    if (!entry)
        return std::nullopt;

    return Account {entry->pw_name, entry->pw_dir, entry->pw_shell, entry->pw_uid, entry->pw_gid};
}

static std::vector<std::string> strings(const Json &array)
{
    std::vector<std::string> result;
    if (!array.isArray())
        return result;
    for (const Json &value : array.toArray())
        result.push_back(value.toString());
    return result;
}

int main()
{
    Lingmo::base::Locale::setProcessLocale();
    Lingmo::i18n::setDomain("lingmo-dm");

    // The daemon's signalfd mask is inherited; without this SIGTERM never lands.
    sigset_t none;
    sigemptyset(&none);
    sigprocmask(SIG_SETMASK, &none, nullptr);

    struct sigaction action = {};
    action.sa_handler = onTerminate;
    sigemptyset(&action.sa_mask);
    sigaction(SIGTERM, &action, nullptr);
    action.sa_handler = onStopDeadline;
    sigaction(SIGALRM, &action, nullptr);

    // The daemon's death ends the login like its SIGTERM; no password goes to a core file.
    prctl(PR_SET_PDEATHSIG, SIGTERM);
    prctl(PR_SET_DUMPABLE, 0);
    signal(SIGPIPE, SIG_IGN);

    const auto login = receive();
    if (!login || (*login)["type"].toString() != "login") {
        fail(false, tr("Expected a login request"));
        return 1;
    }

    const std::string service = (*login)["service"].toString();
    const std::string user = (*login)["user"].toString();
    const bool authenticate = (*login)["authenticate"].toBool();
    const int vt = int((*login)["vt"].toInt());
    if (service.empty() || user.empty() || vt <= 0) {
        fail(false, tr("Incomplete login request"));
        return 1;
    }

    // The login screen's own login has nobody to answer its PAM messages either.
    g_conversing = authenticate;

    const pam_conv conversation {converse, nullptr};
    pam_handle_t *pam = nullptr;
    int status = pam_start(service.c_str(), user.c_str(), &conversation, &pam);
    if (status != PAM_SUCCESS) {
        fail(false, text(pam_strerror(pam, status)));
        return 1;
    }

    g_pam = 1;
    const auto finish = [&](int result) {
        g_pam = 0;
        pam_end(pam, result);
        return 1;
    };

    if (authenticate) {
        status = pam_authenticate(pam, 0);
        if (status != PAM_SUCCESS) {
            fail(true, text(pam_strerror(pam, status)));
            return finish(status);
        }
    }

    status = pam_acct_mgmt(pam, 0);
    if (status == PAM_NEW_AUTHTOK_REQD)
        status = pam_chauthtok(pam, PAM_CHANGE_EXPIRED_AUTHTOK);
    if (status != PAM_SUCCESS) {
        fail(true, text(pam_strerror(pam, status)));
        return finish(status);
    }

    status = pam_setcred(pam, PAM_ESTABLISH_CRED);
    if (status != PAM_SUCCESS) {
        fail(false, text(pam_strerror(pam, status)));
        return finish(status);
    }

    g_conversing = false;
    send(Json::object({{"type", "authenticated"}}));

    const auto request = receive();
    if (!request || (*request)["type"].toString() != "start") {
        pam_setcred(pam, PAM_DELETE_CRED);
        g_pam = 0;
        pam_end(pam, PAM_SUCCESS);
        return 0;
    }

    const std::vector<std::string> command = strings((*request)["cmd"]);
    const auto owner = account(pam);
    if (command.empty() || !owner) {
        fail(false, tr("No command or no account to start"));
        pam_setcred(pam, PAM_DELETE_CRED);
        return finish(PAM_SESSION_ERR);
    }

    // pam_systemd takes the seat, VT, type and class from here.
    const std::string tty = "tty" + std::to_string(vt);
    pam_set_item(pam, PAM_TTY, tty.c_str());
    for (const std::string &variable : strings((*request)["env"]))
        pam_putenv(pam, variable.c_str());

    const std::string home = "HOME=" + owner->home;
    const std::string name = "USER=" + owner->name;
    const std::string logname = "LOGNAME=" + owner->name;
    // The session wrapper picks the login shell from it.
    const std::string shell = "SHELL=" + owner->shell;
    pam_putenv(pam, home.c_str());
    pam_putenv(pam, name.c_str());
    pam_putenv(pam, logname.c_str());
    pam_putenv(pam, shell.c_str());
    if (!pam_getenv(pam, "PATH"))
        pam_putenv(pam, "PATH=/usr/local/bin:/usr/bin:/bin");

    g_opening = 1;
    status = pam_open_session(pam, 0);
    if (status != PAM_SUCCESS) {
        fail(false, text(pam_strerror(pam, status)));
        pam_setcred(pam, PAM_DELETE_CRED);
        return finish(status);
    }

    const char *sessionId = pam_getenv(pam, "XDG_SESSION_ID");
    char **environment = pam_getenvlist(pam);

    std::vector<char *> arguments;
    for (const std::string &argument : command)
        arguments.push_back(const_cast<char *>(argument.c_str()));
    arguments.push_back(nullptr);

    // Out of the OOM killer's way, inherited by the session and everything it
    // starts; this process's end ends the session too. The session puts its
    // applications back to 0. Not -1000: a runaway session process must remain
    // the kernel's last resort. Written as root, it is also the floor no
    // descendant can go below.
    const int protection = open("/proc/self/oom_score_adj", O_WRONLY | O_CLOEXEC);
    if (protection >= 0) {
        if (write(protection, "-900", 4) != 4)
            perror("oom_score_adj");
        close(protection);
    }

    // The session runs in a child; this process stays to close the PAM session.
    const pid_t child = fork();
    if (child == 0) {
        setsid();
        const int null = open("/dev/null", O_RDONLY | O_CLOEXEC);
        if (null >= 0)
            dup2(null, STDIN_FILENO);
        dup2(STDERR_FILENO, STDOUT_FILENO);

        // Not the session's: fds PAM modules left open, the ignored SIGPIPE, the
        // stop handlers.
        close_range(3, ~0U, 0);
        signal(SIGPIPE, SIG_DFL);
        signal(SIGTERM, SIG_DFL);
        signal(SIGALRM, SIG_DFL);

        if (initgroups(owner->name.c_str(), owner->gid) != 0 || setgid(owner->gid) != 0
            || setuid(owner->uid) != 0)
            _exit(127);
        // setuid resets it.
        prctl(PR_SET_PDEATHSIG, SIGTERM);
        if (chdir(owner->home.c_str()) != 0 && chdir("/") != 0)
            _exit(127);

        execve(arguments.front(), arguments.data(), environment);
        _exit(127);
    }

    // The child took its copy across execve; this side is done with it.
    for (char **variable = environment; variable && *variable; ++variable)
        free(*variable);
    free(environment);

    if (child < 0) {
        fail(false, text(strerror(errno)));
        pam_close_session(pam, 0);
        pam_setcred(pam, PAM_DELETE_CRED);
        return finish(PAM_SESSION_ERR);
    }

    g_session = child;
    g_opening = 0;
    if (g_stopRequested) {
        g_stopRequested = 0;
        raise(SIGTERM);
    }

    send(Json::object({{"type", "started"}, {"pid", int64_t(child)}, {"session", text(sessionId)}}));

    // Not reaped yet, so the deadline can still only hit the session's group.
    siginfo_t exited = {};
    while (waitid(P_PID, id_t(child), &exited, WEXITED | WNOWAIT) < 0 && errno == EINTR) {
    }

    // The session is over; closing it is not to be cut short.
    signal(SIGTERM, SIG_IGN);
    alarm(0);

    // The killed leader can exit before the rest of its group, whose compositor
    // still holds the Wayland socket's lock the next login needs.
    for (int waited = 0; g_killed && waited < 5000 && groupHasOthers(child); waited += 20)
        usleep(20 * 1000);

    g_session = 0;
    int wait = 0;
    while (waitpid(child, &wait, 0) < 0 && errno == EINTR) {
    }

    // Closing the PAM session needs root.
    pam_close_session(pam, 0);
    pam_setcred(pam, PAM_DELETE_CRED);
    g_pam = 0;
    pam_end(pam, PAM_SUCCESS);

    const int exitStatus = WIFEXITED(wait) ? WEXITSTATUS(wait) : 128 + (WIFSIGNALED(wait) ? WTERMSIG(wait) : 0);
    send(Json::object({{"type", "finished"}, {"status", exitStatus}}));
    return 0;
}
