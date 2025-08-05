#include <fpu_control.h>

int main()
{
    int cw;

    _FPU_GETCW(cw);

    return !((cw & (_FPU_ABS2008 | _FPU_NAN2008)) == _FPU_DEFAULT);
}
