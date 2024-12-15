// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "catchnetflow.h"
#include "nethogs.cpp"
#include "line.h"

#include <DApplication>

#include <fcntl.h>
#include <vector>
#ifdef __linux__
#include <linux/capability.h>
#include <linux/limits.h>
#include <sys/types.h>
#include <sys/xattr.h>
#include <unistd.h>
#endif
#include <sys/capability.h>
#include <QDebug>

extern ProcList *processes;

// The self_pipe is used to interrupt the select() in the main loop
static std::pair<int, int> self_pipe = std::make_pair(-1, -1);
static time_t last_refresh_time = 0;
// selectable file descriptors for the main loop
static fd_set pc_loop_fd_set;
static std::vector<int> pc_loop_fd_list;
static bool pc_loop_use_select = true;
static void versiondisplay(void) { std::cout << version << "\n"; }
static void help(bool iserror);

void refreshData(WriteDBusData *dbusData);
void quit_cb(int /* i */);
void forceExit(bool success, const char *msg, ...);
std::pair<int, int> create_self_pipe();
bool wait_for_next_trigger();
void clean_up();
void catchLibpcap(int argc, char *argv[], WriteDBusData *widget);
void listCaps();
void setRootPermission();

CatchNetFlow::CatchNetFlow(int argc, char *argv[], WriteDBusData *dbusData, QObject *parent)
    : QThread(parent)
    , m_dbusData(dbusData)
    , m_argc(argc)
    , m_argv((char **)argv)
{

}

CatchNetFlow::~CatchNetFlow()
{

}

void CatchNetFlow::run()
{
    catchLibpcap(m_argc, m_argv, m_dbusData);
}

void listCaps()
{
  cap_t caps = cap_get_proc();
  ssize_t y = 0;
  printf("The process %d was give capabilities %s\n", (int) getpid(), cap_to_text(caps, &y));
  fflush(0);
  cap_free(caps);
}

void setRootPermission()
{
    printf("uid=%i  euid=%i  gid=%i\n", getuid(), geteuid(), getgid());

//    int stat = setuid(geteuid());
    pid_t parentPid = getpid();

    if(!parentPid)
      return;

    cap_t caps = cap_init();

    //CAP_DAC_READ_SEARCH 覆盖有关读取和搜索文件的所有DAC限制和目录，包括ACL限制（如果[_POSIX_ACL]为定义。 不包括CAP_LINUX_IMMUTABLE涵盖的DAC访问
    //CAP_SYS_PTRACE :允许任何进程的ptrace
    cap_value_t capList[4] = {
        CAP_DAC_READ_SEARCH,
        CAP_NET_ADMIN,
        CAP_NET_RAW,
        CAP_SYS_PTRACE
    };

    unsigned num_caps = 4;
    cap_set_flag(caps, CAP_EFFECTIVE, num_caps, capList, CAP_SET);
    cap_set_flag(caps, CAP_INHERITABLE, num_caps, capList, CAP_SET);
    cap_set_flag(caps, CAP_PERMITTED, num_caps, capList, CAP_SET);

    if (cap_set_proc(caps)) {
      perror("capset()");
      return;
    }

    listCaps();
}

void catchLibpcap(int argc, char *argv[], WriteDBusData *dbusData)
{
    int promisc = 0;
    bool all = true;
    char *filter = nullptr;

    int opt;
    while ((opt = getopt(argc, argv, "Vhbtpsd:v:c:laf:C")) != -1) {
        switch (opt) {
        case 'V':
            versiondisplay();
            exit(0);
        case 'h':
            help(false);
            exit(0);
        case 'b':
            bughuntmode = true;
            tracemode = true;
            break;
        case 't':
            tracemode = true;
            break;
        case 'p':
            promisc = 1;
            break;
        case 's':
            sortRecv = false;
            break;
        case 'd':
            refreshdelay = (time_t)atoi(optarg);
            break;
        case 'v':
            viewMode = atoi(optarg) % VIEWMODE_COUNT;
            break;
        case 'c':
            refreshlimit = atoi(optarg);
            break;
        case 'l':
            showcommandline = true;
            break;
        case 'a':
            all = true;
            break;
        case 'f':
            filter = optarg;
            break;
        case 'C':
            catchall = true;
            break;
        default:
            help(true);
            exit(EXIT_FAILURE);
        }
    }

    process_init();
    device *devices = get_devices(argc - optind, argv + optind, all);
    if (devices == nullptr)
        forceExit(false, "No devices to monitor. Use '-a' to allow monitoring "
                  "loopback interfaces or devices that are not up/running");

    if (geteuid() != 0) {
#ifdef __linux__
        char exe_path[PATH_MAX];
        ssize_t len;
        unsigned int caps[5] = {0, 0, 0, 0, 0};

        if ((len = readlink("/proc/self/exe", exe_path, PATH_MAX)) == -1)
            forceExit(false, "Failed to locate nethogs binary.");
        exe_path[len] = '\0';

        getxattr(exe_path, "security.capability", (char *)caps, sizeof(caps));

        //CAP_NET_ADMIN:允许执行网络管理任务(允许执行网络管理任务:接口,防火墙和路由等)
        //CAP_NET_RAW:允许使用原始(raw)套接字
        if ((((caps[1] >> CAP_NET_ADMIN) & 1) != 1) ||
                (((caps[1] >> CAP_NET_RAW) & 1) != 1))
            forceExit(false, "To run nethogs without being root you need to enable "
                      "capabilities on the program (cap_net_admin, "
                      "cap_net_raw), see the documentation for details.");
#else
        forceExit(false, "You need to be root to run NetHogs!");
#endif
    }

    // use the Self-Pipe trick to interrupt the select() in the main loop
    self_pipe = create_self_pipe();
    if (self_pipe.first == -1 || self_pipe.second == -1) {
        forceExit(false, "Error creating pipe file descriptors\n");
    } else {
        // add the self-pipe to allow interrupting select()
        pc_loop_fd_list.push_back(self_pipe.first);
    }

    char errbuf[PCAP_ERRBUF_SIZE];

    int nb_devices = 0;
    int nb_failed_devices = 0;

    handle *handles = nullptr;
    device *current_dev = devices;
    while (current_dev != nullptr) {
        ++nb_devices;
        printf("[device] current_dev->name : %s ", current_dev->name);
        if (!getLocal(current_dev->name, tracemode)) {
            forceExit(false, "getifaddrs failed while establishing local IP.");
        }

        //huo qu jian kong she bei ju bing
        dp_handle *newhandle =
            dp_open_live(current_dev->name, BUFSIZ, promisc, 100, filter, errbuf);

        fprintf(stdout, "get newhandle : %2x\n", newhandle);

        //注册回调函数
        if (newhandle != nullptr) {
            dp_addcb(newhandle, dp_packet_ip, process_ip);
            dp_addcb(newhandle, dp_packet_ip6, process_ip6);
            dp_addcb(newhandle, dp_packet_tcp, process_tcp);
            dp_addcb(newhandle, dp_packet_udp, process_udp);

            /* The following code solves sf.net bug 1019381, but is only available
            * in newer versions (from 0.8 it seems) of libpcap
            *
            * update: version 0.7.2, which is in debian stable now, should be ok
            * also.
            */
            if (dp_setnonblock(newhandle, 1, errbuf) == -1) {
                fprintf(stderr, "Error putting libpcap in nonblocking mode\n");
            }
            handles = new handle(newhandle, current_dev->name, handles);

            if (pc_loop_use_select) {
                // some devices may not support pcap_get_selectable_fd
                int const fd = pcap_get_selectable_fd(newhandle->pcap_handle);
                if (fd != -1) {
                    pc_loop_fd_list.push_back(fd);
                } else {
                    pc_loop_use_select = false;
                    pc_loop_fd_list.clear();
                    fprintf(stderr, "failed to get selectable_fd for %s\n", current_dev->name);
                }
            }
        } else {
            fprintf(stderr, "Error opening handler for device %s\n", current_dev->name);
            ++nb_failed_devices;
        }

        current_dev = current_dev->next;

        if (QCoreApplication::closingDown())
            return;
    }

    if (nb_devices == nb_failed_devices) {
        forceExit(false, "Error opening pcap handlers for all devices.\n");
    }

    signal(SIGINT, &quit_cb);

    struct dpargs *userdata = (dpargs *)malloc(sizeof(struct dpargs));

    // Main loop:
    while (true) {
        if (QCoreApplication::closingDown()) {
            printf(">>>>>>>>>>>>>>> QCoreApplication::closingDown : %d ", QCoreApplication::closingDown());
            return;
        } else {
            Q_EMIT dbusData->notifyWorking();
        }

        bool packets_read = false;

        for (handle *current_handle = handles; current_handle != nullptr;
                current_handle = current_handle->next) {
            userdata->device = current_handle->devicename;
            userdata->sa_family = AF_UNSPEC;

            //抓包,函数实现里面会触发回调函数
            int retval = dp_dispatch(current_handle->content, -1, (u_char *)userdata,
                                     sizeof(struct dpargs));
            if (retval == -1)
                std::cerr << "Error dispatching for device "
                          << current_handle->devicename << ": "
                          << dp_geterr(current_handle->content) << std::endl;
            else if (retval < 0)
                std::cerr << "Error dispatching for device "
                          << current_handle->devicename << ": " << retval << std::endl;
            else if (retval != 0)
                packets_read = true;
        }

        time_t const now = ::time(nullptr);
        //定时器刷新数据，以refreshdelay为间隔(单位：秒)
        if (last_refresh_time + refreshdelay <= now) {
            last_refresh_time = now;

            //刷新数据
            refreshData(dbusData);
        }

        // if not packets, do a select() until next packet
        if (!packets_read)
            if (!wait_for_next_trigger())
                // Shutdown requested - exit the loop
                break;

        usleep(1000);
    }

    clean_up();
}

void calculateGlobalFlow(Line *lines[], int nproc, WriteDBusData *dbusData)
{
    double sent_global = 0;
    double recv_global = 0;

    for (int i = 0; i < nproc; i++) {
        recv_global += lines[i]->recv_value;
        sent_global += lines[i]->sent_value;
        delete lines[i];
    }

    Q_EMIT dbusData->notifyGlobalData(recv_global, sent_global);
}

void refreshData(WriteDBusData *dbusData)
{
    refreshconninode();
    refreshcount++;

    if (viewMode == VIEWMODE_KBPS || viewMode == VIEWMODE_MBPS ||
            viewMode == VIEWMODE_GBPS) {
        remove_timed_out_processes();
    }

    ProcList *curproc = processes;
    int nproc = processes->size();

    /* initialize to null pointers */
    Line *lines[nproc];
    for (int i = 0; i < nproc; i++)
        lines[i] = nullptr;

    int n = 0;

    while (curproc != nullptr) {
        // walk though its connections, summing up their data, and
        // throwing away connections that haven't received a package
        // in the last CONNTIMEOUT seconds.
        assert(curproc->getVal() != nullptr);
        assert(nproc == processes->size());

        double value_sent = 0.0, value_recv = 0.0;

        if (viewMode == VIEWMODE_KBPS) {
            curproc->getVal()->getkbps(&value_recv, &value_sent);
            //用于通知传输的数据：kb
            Q_EMIT dbusData->notifyDBusKbData(curproc->getVal()->name, value_recv, value_sent, curproc->getVal()->pid, curproc->getVal()->devicename, "kb/s");
        } else if (viewMode == VIEWMODE_MBPS) {
            curproc->getVal()->getmbps(&value_recv, &value_sent);
        } else if (viewMode == VIEWMODE_GBPS) {
            curproc->getVal()->getgbps(&value_recv, &value_sent);
        } else if (viewMode == VIEWMODE_TOTAL_KB) { //默认使用kb,用于流量统计
            curproc->getVal()->gettotalkb(&value_recv, &value_sent);
        } else if (viewMode == VIEWMODE_TOTAL_MB) {
            curproc->getVal()->gettotalmb(&value_recv, &value_sent);
        } else if (viewMode == VIEWMODE_TOTAL_B) {
            curproc->getVal()->gettotalb(&value_recv, &value_sent);
        } else if (viewMode == VIEWMODE_KB) {
            curproc->getVal()->getkb(&value_recv, &value_sent);
        } else {
            forceExit(false, "Invalid viewMode: %d", viewMode);
        }
        uid_t uid = curproc->getVal()->getUid();
        assert(curproc->getVal()->pid >= 0);
        assert(n < nproc);

        //此处用于统计总消耗流量(kb)
        curproc->getVal()->gettotalkb(&value_recv, &value_sent);

        //用于通知传输的数据：kb
        Q_EMIT dbusData->notifyDBusKbData(curproc->getVal()->name, value_recv, value_sent, curproc->getVal()->pid, curproc->getVal()->devicename, "total_kb");
        lines[n] = new Line(curproc->getVal()->name, curproc->getVal()->cmdline,
                            value_recv, value_sent, curproc->getVal()->pid, uid,
                            curproc->getVal()->devicename);

        curproc = curproc->next;
        n++;
    }

    calculateGlobalFlow(lines, nproc, dbusData);

    if (refreshlimit != 0 && refreshcount >= refreshlimit)
        quit_cb(0);
}

void quit_cb(int /* i */)
{
    if (self_pipe.second != -1) {
        write(self_pipe.second, "x", 1);
    } else {
        exit(0);
    }
}

void forceExit(bool success, const char *msg, ...)
{
    va_list argp;
    va_start(argp, msg);
    vfprintf(stderr, msg, argp);
    va_end(argp);
    std::cerr << std::endl;

    if (success)
        exit(EXIT_SUCCESS);
    else
        exit(EXIT_FAILURE);
}

std::pair<int, int> create_self_pipe()
{
    int pfd[2];
    if (pipe(pfd) == -1)
        return std::make_pair(-1, -1);

    if (fcntl(pfd[0], F_SETFL, fcntl(pfd[0], F_GETFL) | O_NONBLOCK) == -1)
        return std::make_pair(-1, -1);

    if (fcntl(pfd[1], F_SETFL, fcntl(pfd[1], F_GETFL) | O_NONBLOCK) == -1)
        return std::make_pair(-1, -1);

    return std::make_pair(pfd[0], pfd[1]);
}

bool wait_for_next_trigger()
{
    if (pc_loop_use_select) {
        FD_ZERO(&pc_loop_fd_set);
        int nfds = 0;
        for (std::vector<int>::const_iterator it = pc_loop_fd_list.begin();
                it != pc_loop_fd_list.end(); ++it) {
            int const fd = *it;
            nfds = std::max(nfds, *it + 1);
            FD_SET(fd, &pc_loop_fd_set);
        }
        timeval timeout = {refreshdelay, 0};
        if (select(nfds, &pc_loop_fd_set, 0, 0, &timeout) != -1) {
            if (FD_ISSET(self_pipe.first, &pc_loop_fd_set)) {
                return false;
            }
        }
    } else {
        // If select() not possible, pause to prevent 100%
        usleep(1000);
    }
    return true;
}

void clean_up()
{
    // close file descriptors
    for (std::vector<int>::const_iterator it = pc_loop_fd_list.begin();
            it != pc_loop_fd_list.end(); ++it) {
        close(*it);
    }

    procclean();
}

static void help(bool iserror)
{
    std::ostream &output = (iserror ? std::cerr : std::cout);

    // output << "usage: nethogs [-V] [-b] [-d seconds] [-t] [-p] [-f (eth|ppp))]
    // [device [device [device ...]]]\n";
    output << "usage: nethogs [-V] [-h] [-b] [-d seconds] [-v mode] [-c count] "
           "[-t] [-p] [-s] [-a] [-l] [-f filter] [-C]"
           "[device [device [device ...]]]\n";
    output << "		-V : prints version.\n";
    output << "		-h : prints this help.\n";
    output << "		-b : bughunt mode - implies tracemode.\n";
    output << "		-d : delay for update refresh rate in seconds. default "
           "is 1.\n";
    output << "		-v : view mode (0 = KB/s, 1 = total KB, 2 = total B, 3 "
           "= total MB, 4 = MB/s, 5 = GB/s). default is 0.\n";
    output << "		-c : number of updates. default is 0 (unlimited).\n";
    output << "		-t : tracemode.\n";
    // output << "        -f : format of packets on interface, default is
    // eth.\n";
    output << "		-p : sniff in promiscious mode (not recommended).\n";
    output << "		-s : sort output by sent column.\n";
    output << "		-l : display command line.\n";
    output << "		-a : monitor all devices, even loopback/stopped "
           "ones.\n";
    output << "		-C : capture TCP and UDP.\n";
    output << "		-f : EXPERIMENTAL: specify string pcap filter (like "
           "tcpdump)."
           " This may be removed or changed in a future version.\n";
    output << "		device : device(s) to monitor. default is all "
           "interfaces up and running excluding loopback\n";
    output << std::endl;
    output << "When nethogs is running, press:\n";
    output << " q: quit\n";
    output << " s: sort by SENT traffic\n";
    output << " r: sort by RECEIVE traffic\n";
    output << " l: display command line\n";
    output << " m: switch between total (KB, B, MB) and throughput (KB/s, MB/s, "
           "GB/s) mode\n";
}
