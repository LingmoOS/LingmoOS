
#include "hw.h"
// #include "print.h"


#include "options.h"
#include "mem.h"
#include "dmi.h"
#include "cpuinfo.h"
#include "cpuid.h"
#include "pci.h"

#include "network.h"

#include "pnp.h"
#include "fb.h"
// #include "usb.h"
#include "usbsysfs.h"
#include "sysfs.h"
#include "display.h"

#include "cpufreq.h"
#include "disk.h"
#include "mounts.h"

#include "mmc.h"
#include "input.h"
#include "sound.h"
#include "graphics.h"
#include "smp.h"

#include "top_process.h"

#include <unistd.h>
#include <stdio.h>

#include <string.h>
// #include <stdlib.h>
// #include <iostream>

void status(const char *);

void status(const char *message)
{
    static size_t lastlen = 0;

    if (enabled("output:quiet") || disabled("output:verbose"))
        return;

    if (isatty(2)) {
        fprintf(stderr, "\r");
        for (size_t i = 0; i < lastlen; i++)
            fprintf(stderr, " ");
        fprintf(stderr, "\r%s\r", message);
        fflush(stderr);
        lastlen = strlen(message);
    }
}

bool scan_system(hwNode &sys_tem)
{

    disable("isapnp");

    disable("output:list");
    disable("output:json");
    disable("output:db");
    disable("output:xml");
    disable("output:html");
    disable("output:hwpath");
    // disable("output:businfo");
    disable("output:X");
    disable("output:quiet");
    disable("output:sanitize");
    disable("output:numeric");
    enable("output:time");

// define some aliases for nodes classes
    alias("disc", "disk");
    alias("cpu", "processor");
    alias("lan", "network");
    alias("net", "network");
    alias("video", "display");
    alias("sound", "multimedia");
    alias("modem", "communication");

    disable("output:quiet");
    enable("output:verbose");
    enable("output:businfo");
    // enable("output:hwpath");
    // enable("output:sanitize");
    enable("output:numeric");
    // disable("output:time");
    // enable("output:list");

    char hostname[80];

    if (gethostname(hostname, sizeof(hostname)) == 0) {
        // hwNode computer(::enabled("output:sanitize")?"computer":hostname,
        //   hw::sys_tem);
        hwNode computer("computer", hw::sys_tem);

        // status("DMI");
        // if (enabled("dmi"))
        scan_dmi(computer);
        // status("SMP");
        // if (enabled("smp"))
        scan_smp(computer);

        //status("memory");
        // if (enabled("memory"))
        scan_memory(computer);
        //status("/proc/cpuinfo");
        // if (enabled("cpuinfo"))
        scan_cpuinfo(computer);
        //status("CPUID");
        // if (enabled("cpuid"))
        scan_cpuid(computer);
//         //status("PCI (sysfs)");
//         // if (enabled("pci"))
        scan_pci(computer);

//        //status("PnP (sysfs)");
//        // if (enabled("pnp"))
//        scan_pnp(computer);

        //status("kernel device tree (sysfs)");
        // if (enabled("sysfs"))
        scan_sysfs(computer);
        //status("USB");
        // if (enabled("usb")){
        // scan_usb(computer);
        scan_usbsysfs(computer);

        scan_diskstorage(computer);
        //status("MMC");
        // if (enabled("mmc"))
        scan_mmc(computer);
        //status("sound");
        // if (enabled("sound"))
        scan_sound(computer);
        //status("graphics");
        // if (enabled("graphics"))
        scan_graphics(computer);
        // //status("input");
        // // if (enabled("input"))
        // scan_input(computer);

        // if (enabled("mounts"))
        scan_mounts(computer);
        //status("Network interfaces");
        // if (enabled("network"))
        scan_network(computer);
        //status("Framebuffer devices");
        // if (enabled("fb"))
        scan_fb(computer);
        //status("Display");
        // if (enabled("display"))
        scan_display(computer);
        //status("CPUFreq");
        // if (enabled("cpufreq"))
        scan_cpufreq(computer);

        // scan_top_process(computer);
        //status("");

        if (computer.getDescription() == "")
            computer.setDescription("Computer");
        computer.assignPhysIds();
        computer.fixInconsistencies();

        sys_tem = computer;
    } else
        return false;

    return true;
}
