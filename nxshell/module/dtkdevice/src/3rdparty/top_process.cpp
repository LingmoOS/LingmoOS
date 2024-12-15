
#include "config.h"
#include "hw.h"
#include "osutils.h"
#include <unistd.h>
#include <stdlib.h>
#include <dirent.h>
#include <limits.h>


#define PROC_PATH "/proc"

bool scan_top_process(hwNode &nn)
{

    pushd(PROC_PATH);
    {
        int i, n, j;
        struct dirent **namelist;

//  hwNode disk("disk", hw::storage);
        j = 0;
        n = scandir(".", &namelist, selectdir, alphasort);

        for (i = 0; i < n; i++) {
            if (matches(namelist[i]->d_name, "^[0-9]+$")) {
                string pidpath = string(namelist[i]->d_name);
                string value = get_string(pidpath + "/stat");
                if (value.empty())
                    continue;
                j++;
                vector < string > vecstring;
                splitlines(value, vecstring, ' ');
                if (2 < vecstring.size()) {
                    string pidname = vecstring[1];
//      pidname = pidname.replace(pidname.find("("), 1, "");
//      pidname = pidname.replace(pidname.find(")"), 1, "");

                    hwNode  pid("pidtop" + pidpath, hw::sys_tem);
                    // pid.setConfig("procValue","cat /proc/<pid>/stat");
                    pid.setConfig("pidname", pidname);
                    // pid.setDescription();
                    pid.setProduct_name(pidname);
                    pid.setVendor_name("SystemPIDShow");
                    // nn.addChild(pid);
                } else
                    continue;
            }

            free(namelist[i]);
        }
        if (namelist)
            free(namelist);
    }
    popd();
    //pid.describeCapability("vsyscall64", _("64-bit processes"));

    return true;
}
