#include <gbm.h>

int main(int argc, char *argv[])
{
    gbm_format_name_desc ret;
    gbm_format_get_name(GBM_BO_FORMAT_XRGB8888, &ret);
    return 0;
}