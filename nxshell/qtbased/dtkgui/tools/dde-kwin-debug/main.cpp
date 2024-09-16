// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <cstring>
#include <iostream>
#include <thread>
#include <unistd.h>
#include <signal.h>
#include <map>

#include <sys/time.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>

// 记录日志打印的格式
char *outputFormat = nullptr;
// 完成启动参数测试后是否自动推出进程
bool killWindow = false;
// 监听所有窗口
bool watchAll = false;
bool noCache = false;
int repeatTime = 1;
// 启动时间的偏移值
int time_offset = 0;

const std::string timer_interval_env = "_D_CHECKER_TIMER_INTERVAL";
const std::string ping_time_env = "_D_CHECKER_PING_TIME";
const std::string valid_count_env = "_D_CHECKER_VALID_COUNT";
const std::string damage_count_env = "_D_CHECKER_DAMAGE_COUNT";

std::map<std::string, std::string> checkArgMap {
    {"--ci", ""},
    {"--cpt", ""},
    {"--cvc", ""},
    {"--cdc", ""}
};

std::map<std::string, std::string> checkEnvMap {
    {"--cil", timer_interval_env},
    {"--cpt", ping_time_env},
    {"--cvc", valid_count_env},
    {"--cdc", damage_count_env}
};

uint64_t timeSinceEpochMillisec()
{
    using namespace std::chrono;
    return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
}

bool checkCommand(int argc, char *argv[], const char *arg)
{
    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], arg) == 0)
            return true;
    }

    return false;
}

int xerrorHandler(Display*		/* display */,
                  XErrorEvent*	/* error_event */)
{
    return 0;
}

int exec(const char *process)
{
    // 准备环境变量
    char env[100] = {};
    auto time = timeSinceEpochMillisec();
    sprintf(env, "D_KWIN_DEBUG_APP_START_TIME=%lu", time);
    putenv(env);

    char *displayname = getenv("DISPLAY");
    Display *dpy = XOpenDisplay(displayname);
    if (dpy) {
        XSetErrorHandler(xerrorHandler);
    }

    // deepin-trubo 启动的程序无法对外暴露环境变量，因此通过root窗口设置启动时间
    XChangeProperty(dpy, XDefaultRootWindow(dpy), XInternAtom(dpy, "D_KWIN_DEBUG_APP_START_TIME", false),
                    XA_CARDINAL, 32, PropModeReplace, reinterpret_cast<unsigned char*>(&time), 2);
    XFlush(dpy);

    for (auto it = checkArgMap.cbegin(); it != checkArgMap.cend(); ++it) {
        if (!it->second.empty()) {
            setenv(checkEnvMap[it->first].data(), it->second.data(), 0);
        }
    }

    if (noCache) {
      std::string clearCache = "sync && sudo bash -c \"echo 3 > /proc/sys/vm/drop_caches\"";
      system(clearCache.c_str());
    }

    // 启动进程，并等待其退出
    int code = system(process);

    // 清理窗口属性
    XDeleteProperty(dpy, XDefaultRootWindow(dpy), XInternAtom(dpy, "D_KWIN_DEBUG_APP_START_TIME", true));
    XFlush(dpy);
    XFree(dpy);

    return code;
}

// 检测这个进程是不是自己的子进程
bool checkPid(__pid_t pid)
{
    auto self = getpid();

    while (true) {
        char status[100] = {0};
        sprintf(status, "/proc/%d/exe", pid);
        // 检测是不是deepin-turbo-booster进程
        char exe_path[100];
        readlink(status, exe_path, 100);
        // 对于deepin-turbo的进程无法检测是否为子进程，因此默认认为是被dde-kwin-debug启动的
        if (strstr(exe_path, "/usr/lib/deepin-turbo") == exe_path)
            return true;

        __pid_t ppid = 1;
        sprintf(status, "/proc/%d/status", pid);
        auto file = fopen(status, "ro");

        if (!file)
            return false;

        char data[1024] = {};
        fread(data, sizeof(char), 1024, file);

        // 查找父进程id
        if (const char *i = strstr(data, "\nPPid:")) {
            i += 6;
            // 移除空白
            while (i[0] == ' ' || i[0] == '\t') {
                ++i;
            }

            const char *ppid_begin = i;
            char ppid_string[100] = {};

            // 查找ppid的结尾
            while (i[0] <= '9' && i[0] >= '0') {
                ppid_string[i - ppid_begin] = i[0];
                ++i;
            }

            ppid = atoi(ppid_string);
        }

        fclose(file);

        if (ppid == self)
            return true;

        // 继续向上查找
        pid = ppid;

        if (ppid == 1)
            break;
    }

    return false;
}

__pid_t getWindowPid(Display *dpy, Window window)
{
    Atom actual_type;
    int actual_format;
    unsigned long nitems;
    unsigned long bytes_after;
    static unsigned char *prop = NULL;
    int status;

    status = XGetWindowProperty(dpy, window, XInternAtom(dpy, "_NET_WM_PID", false), 0, 1024,
                                false, AnyPropertyType, &actual_type,
                                &actual_format, &nitems, &bytes_after,
                                &prop);

    if (status == Success && prop) {
        // 获取pid
        return *(uint32_t*)(prop);
    }

    return 0;
}

bool event_queue_done = false;
Display *dpy = nullptr;
void processXEvent()
{
    if (!dpy) {
        char *displayname = getenv("DISPLAY");
        dpy = XOpenDisplay(displayname);

        if (!dpy) {
            fprintf(stderr, "%s:  unable to open display '%s'\n",
                    "dde-kwin-startup-debug", XDisplayName (displayname));
            exit(1);
        }
    }

    int screen = DefaultScreen(dpy);
    Window root = RootWindow(dpy, screen);
    XSelectInput(dpy, root, SubstructureNotifyMask);

    // 事件队列
    for (; !event_queue_done; ) {
        XEvent event;
        XNextEvent(dpy, &event);

        switch (event.type) {
        case PropertyNotify: {
            XPropertyEvent *pe = (XPropertyEvent*)&event;
            static Atom atom_D_APP_STARTUP_TIME = XInternAtom(dpy, "_D_APP_STARTUP_TIME", false);

            if (pe->atom == atom_D_APP_STARTUP_TIME) {
                auto pid = getWindowPid(dpy, pe->window);

                // 检测这个窗口是否属于我们启动的应用程序
                if (!watchAll) {
                    // 不是监听的窗口则退出
                    if (!pid || !checkPid(pid))
                        break;
                }

                Atom actual_type;
                int actual_format;
                unsigned long nitems;
                unsigned long bytes_after;
                static unsigned char *prop = NULL;
                int status;

                status = XGetWindowProperty(dpy, pe->window, atom_D_APP_STARTUP_TIME, 0, 1024,
                                            false, AnyPropertyType, &actual_type,
                                            &actual_format, &nitems, &bytes_after,
                                            &prop);

                if (status == Success && prop) {
                    uint32_t time = *(uint32_t*)prop - time_offset;

                    // 打印数据
                    if (!outputFormat) {
                        std::cout << time << std::endl;
                    } else {
                        int count = strlen(outputFormat);
                        for (int i = 0; i < count; ++i) {
                            if (outputFormat[i] == '%') {
                                switch (outputFormat[i + 1]) {
                                case 'p':
                                    std::cout << pid;
                                    break;
                                case 't':
                                    std::cout << time;
                                    break;
                                case 'n': {
                                    char command[100] = {};
                                    sprintf(command, "/proc/%u/cmdline", pid);
                                    auto file = fopen(command, "ro");
                                    if (file) {
                                        char data[1024] = {};
                                        if (fread(data, sizeof(char), 1024, file))
                                            std::cout << data;
                                    }
                                    break;
                                }
                                default:
                                    break;
                                }

                                ++i;
                            } else {
                                std::cout << outputFormat[i];
                            }
                        }

                        std::cout << std::endl;
                    }

                    if (killWindow) {
                        // 退出对应的程序
                        char command[100];
                        sprintf(command, "xkill -id %lu > /dev/null", pe->window);
                        system(command);
                    }
                }
            }

            break;
        }
        case CreateNotify: {
            XCreateWindowEvent *e = (XCreateWindowEvent*)&event;
            Window new_window = e->window;
            // 监听这个窗口的属性变化事件
            XSelectInput(dpy, new_window, PropertyChangeMask);
        }
        default: break;
        }
    }

    XCloseDisplay(dpy);
}

void signal_handle(int sig)
{
    (void)sig;
    exit(0);
}

int main(int argc, char *argv[])
{
    if (argc ==1 || checkCommand(argc, argv, "--help")) {
        auto help_message = R"help(
Eg: dde-kwin-debug -k --format "%p %t %n" qtcreator

--help     Show the help message.
-k         Auto kill the application on test finished.
-L         Disable outputs of applications. (redirect standard outputs to /dev/null)
-a         Watch all windows.
--format   Log formats:
               %p: application pid
               %t: startup time(unit: ms)
               %n: application name
-no--cache Disable cache
--r        Repeat time
--ci       Set check interval, default 100 ms
--cpt      Set window ping reply time, default 50 ms
--cvc      Set check valid count, default 10
--cdc      Set check damage count, default 20

Arguments:
The applications.
Eg: dde-kwin-debug -k --format "%t %n" "app1" "app2" "app3"
)help";

        std::cout << help_message << std::endl;
        return 0;
    }

    // 初始化环境
    killWindow = checkCommand(argc, argv, "-k");
    watchAll = checkCommand(argc, argv, "-a");
    noCache = checkCommand(argc, argv, "-no-cache");

    for (int i = 1; i < argc; ++i) {
        const std::string currentArg = argv[i];

        if (currentArg == "--r") {
            repeatTime = atoi(argv[++i]);
            continue;
        }
        if (currentArg == "--format") {
            outputFormat = argv[++i];
            continue;
        }

        if (checkArgMap.count(currentArg) > 0) {
            checkArgMap[currentArg] = argv[++i];
            continue;
        }
    }

    signal(SIGINT, signal_handle);
    signal(SIGABRT, signal_handle);
    signal(SIGTERM, signal_handle);
    signal(SIGHUP, signal_handle);
    signal(SIGQUIT, signal_handle);
    signal(SIGKILL, signal_handle);
    signal(SIGBUS, signal_handle);
    signal(SIGSYS, signal_handle);
    signal(SIGPIPE, signal_handle);
    signal(SIGKILL, signal_handle);

    // 获取全局启动时间的环境变量
    auto time_env = getenv("D_KWIN_DEBUG_APP_START_TIME");
    int64_t kwin_start_time = 0;

    if (time_env && strlen(time_env))
        sscanf(time_env, "%ld", &kwin_start_time);

    if (kwin_start_time) {
        struct  timeval time;
        gettimeofday(&time, nullptr);
        time_offset = time.tv_sec * 1000 + (1.0 * time.tv_usec) / 1000 - kwin_start_time;
    }

    // 监听X11事件
    std::thread x11_thread(processXEvent);
    x11_thread.detach();

    bool disable_outpus = checkCommand(argc, argv, "-L");
    for (int i = 1; i < argc; ++i) {
        const char *arg = argv[i];

        if (arg[0] == '-') {
            if (arg[1] == '-')
                ++i; // 跳过参数携带的值

            continue;
        }

        for (int i = 0; i < repeatTime; ++i) {
            // 不以'-'开头的参数认为三要启动的应用程序名称
            if (disable_outpus) {
                char command[1024] = {};
                sprintf(command, "%s 2>/dev/null 1> /dev/null", arg);
                exec(command);
            } else {
                exec(arg);
            }
        }
    }

    if (watchAll) {
        sleep(INT32_MAX);
    }

    event_queue_done = false;

    return 0;
}
