#include "stdio.h"
#include "../libkyglobal.h"
#include <stdlib.h>
#include <libkysysinfo.h>
int main()
{
    int index = 0;
    char **language = kdk_global_get_system_support_language();
    if (language)
    {
        while (language[index])
        {
            printf("index = %d, language = %s\n", index, language[index]);
            index++;
        }

        kdk_free_langlist(language);
    }

    bool match = kdk_global_get_region_match_language();
    printf("match = %d\n", match);

    bool rtl = kdk_global_get_rtl();
    printf("rtl = %d\n", rtl);

    int offset = kdk_global_get_raw_offset();
    printf("offset = %d\n", offset);

    char *eUser = kdk_system_get_eUser();
    char* system_language = kdk_global_get_system_language(eUser);
    printf("system_language = %s\n", system_language);
    return 0;
}
