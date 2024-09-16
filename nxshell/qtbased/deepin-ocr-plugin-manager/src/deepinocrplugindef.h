#pragma once

#include <vector>
#include <utility>

#define DEEPIN_EXPORTS __attribute__ ((visibility ("default")))

namespace DeepinOCRPlugin {

enum class HardwareID {
    Network = -1,    //需要使用网络的在线OCR
    CPU_Any = 0,     //任意CPU
    CPU_Intel,       //Intel的CPU
    CPU_AMD,         //AMD的CPU
    CPU_AArch64,     //ARM平台
    CPU_MIPS,        //MIPS平台
    CPU_LoongArch,   //LoongArch平台
    CPU_SW,          //Sunway平台

    GPU_Any = 100,   //任意GPU
    GPU_Vulkan,      //支持Vulkan的GPU
    GPU_NVIDIA,      //NVIDIA的GPU
    GPU_AMD,         //AMD的GPU
    GPU_MT,          //摩尔线程的GPU
    GPU_JM,          //景嘉微的GPU
    GPU_Loongson,    //龙芯的GPU
    GPU_Innosilicon, //芯动科技的GPU
    GPU_LM,          //中船重工/凌久微的GPU
    GPU_BR           //璧仞科技的GPU
};

enum class PixelType {
    Pixel_Unknown = -1, //未知模式
    Pixel_GRAY,         //8bit灰度模式
    Pixel_RGB,          //RGB888模式
    Pixel_BGR,          //BGR888模式
    Pixel_RGBA,         //RGBA8888模式
    Pixel_BGRA          //BGRA8888模式
};

struct TextBox {
    //文本块的点位，一般的OCR算法下vector的size为4, pair的first为x坐标，second为y坐标
    std::vector<std::pair<float, float>> points;

    //当前文本块的倾斜角度
    float angle;
};

constexpr int VERSION = 0x100000;

}
