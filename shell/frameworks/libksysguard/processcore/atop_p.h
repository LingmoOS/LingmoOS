/*
    SPDX-FileCopyrightText: 2010 John Tapsell <john.tapsell@kdemail.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/
/*
** structure which describes the raw file contents
**
** layout raw file:    rawheader
**
**                     rawrecord                           \
**                     compressed system-level statistics   | sample 1
**                     compressed process-level statistics /
**
**                     rawrecord                           \
**                     compressed system-level statistics   | sample 2
**                     compressed process-level statistics /
**
** etcetera .....
*/
#ifndef KSYSGUARD_ATOP_P_H
#define KSYSGUARD_ATOP_P_H

#define ATOPLOGMAGIC (unsigned int)0xfeedbeef
#define PNAMLEN 15
#define CMDLEN 68

#include <sys/utsname.h>
#include <time.h>
typedef long long count_t;

/* These structures come from rawlog.c in ATop source */

struct RawHeader {
    unsigned int magic;

    unsigned short aversion; /* creator atop version with MSB */
    unsigned short sstatlen; /* length of struct sstat        */
    unsigned short pstatlen; /* length of struct pstat        */
    unsigned short rawheadlen; /* length of struct rawheader    */
    unsigned short rawreclen; /* length of struct rawrecord    */
    unsigned short hertz; /* clock interrupts per second   */
    unsigned short sfuture[5]; /* future use                    */
    struct utsname utsname; /* info about this system        */
    char cfuture[8]; /* future use                    */

    unsigned int pagesize; /* size of memory page (bytes)   */
    int supportflags; /* used features                 */
    int osrel; /* OS release number             */
    int osvers; /* OS version number             */
    int ossub; /* OS version subnumber          */
    int ifuture[6]; /* future use                    */
};

struct RawRecord {
    time_t curtime; /* current time (epoch)         */

    unsigned short flags; /* various flags                */
    unsigned short sfuture[3]; /* future use                   */

    unsigned int scomplen; /* length of compressed sstat   */
    unsigned int pcomplen; /* length of compressed pstat's */
    unsigned int interval; /* interval (number of seconds) */
    unsigned int nlist; /* number of processes in list  */
    unsigned int npresent; /* total number of processes    */
    unsigned int nexit; /* number of exited processes   */
    unsigned int nzombie; /* number of zombie processes   */
    unsigned int ifuture[6]; /* future use                   */
};

/*
** structure containing only relevant process-info extracted
** from kernel's process-administration
*/
struct PStat {
    /* GENERAL PROCESS INFO                     */
    struct gen {
        int pid; /* process identification   */
        int ruid; /* real user  identification    */
        int rgid; /* real group identification    */
        int ppid; /* parent process identification*/
        int nthr; /* number of threads in tgroup  */
        char name[PNAMLEN + 1]; /* process name string          */
        char state; /* process state ('E' = exited) */
        int excode; /* process exit status      */
        time_t btime; /* process start time (epoch)   */
        char cmdline[CMDLEN + 1]; /* command-line string        */
        int nthrslpi; /* # threads in state 'S'       */
        int nthrslpu; /* # threads in state 'D'       */
        int nthrrun; /* # threads in state 'R'       */
        int ifuture[1]; /* reserved                     */
    } gen;

    /* CPU STATISTICS                       */
    struct cpu {
        count_t utime; /* time user   text (ticks)     */
        count_t stime; /* time system text (ticks)     */
        int nice; /* nice value                   */
        int prio; /* priority                     */
        int rtprio; /* realtime priority            */
        int policy; /* scheduling policy            */
        int curcpu; /* current processor            */
        int sleepavg; /* sleep average percentage     */
        int ifuture[4]; /* reserved for future use  */
        count_t cfuture[4]; /* reserved for future use  */
    } cpu;

    /* DISK STATISTICS                      */
    struct dsk {
        count_t rio; /* number of read requests  */
        count_t rsz; /* cumulative # sectors read    */
        count_t wio; /* number of write requests     */
        count_t wsz; /* cumulative # sectors written */
        count_t cwsz; /* cumulative # written sectors */
        /* being cancelled              */
        count_t cfuture[4]; /* reserved for future use  */
    } dsk;

    /* MEMORY STATISTICS                        */
    struct mem {
        count_t minflt; /* number of page-reclaims  */
        count_t majflt; /* number of page-faults    */
        count_t shtext; /* text     memory (Kb)         */
        count_t vmem; /* virtual  memory (Kb)     */
        count_t rmem; /* resident memory (Kb)     */
        count_t vgrow; /* virtual  growth (Kb)     */
        count_t rgrow; /* resident growth (Kb)         */
        count_t cfuture[4]; /* reserved for future use  */
    } mem;

    /* NETWORK STATISTICS                       */
    struct net {
        count_t tcpsnd; /* number of TCP-packets sent   */
        count_t tcpssz; /* cumulative size packets sent */
        count_t tcprcv; /* number of TCP-packets received */
        count_t tcprsz; /* cumulative size packets rcvd */
        count_t udpsnd; /* number of UDP-packets sent   */
        count_t udpssz; /* cumulative size packets sent */
        count_t udprcv; /* number of UDP-packets received */
        count_t udprsz; /* cumulative size packets sent */
        count_t rawsnd; /* number of raw packets sent   */
        count_t rawrcv; /* number of raw packets received */
        count_t cfuture[4]; /* reserved for future use  */
    } net;
};

struct PInfo;
struct PInfo {
    PInfo *phnext; /* next process in hash    chain */
    PInfo *prnext; /* next process in residue chain */
    PInfo *prprev; /* prev process in residue chain */
    PStat pstat; /* per-process statistics        */
};

#endif // KSYSGUARD_ATOP_P_H
