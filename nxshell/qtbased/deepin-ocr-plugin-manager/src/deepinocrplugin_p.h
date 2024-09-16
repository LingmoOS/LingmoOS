#pragma once

#include "deepinocrplugindef.h"

#include <vector>
#include <string>
#include <initializer_list>

//插件加载接口，用于加载插件的基础类
//输入：无
//输出：插件基础类指针 DeepinOCRPlugin::Plugin * ，使用 static_cast 即可转换到 void * 类型
extern "C" void* loadPlugin();

//插件卸载接口，用于释放插件相关资源
//输入：由插件加载接口返回的插件基础类指针
//输出：是否释放成功，0为卸载失败，1为卸载成功
extern "C" int unloadPlugin(void *plugin);

//插件版本号接口，用于判断插件是否和开发库的版本匹配，如果不匹配则执行兼容性配置或直接加载失败
//版本号为形如0xXXYYZZ的十六进制数，其中XX为大版本号，YY为中版本号，ZZ为小版本号。如果使用的开发库的版本号为1.0.23，则此处应该返回0x010023
//建议直接返回DeepinOCRPlugin::VERSION，这个变量位于deepinocrplugindef.h中
//输入：无
//输出：插件的版本号
extern "C" int pluginVersion(void);

namespace DeepinOCRPlugin {

class Plugin
{
public:
    Plugin() = default;
    virtual ~Plugin() = default;

    //硬件加速设置

    //设置需要使用的硬件
    //输入：硬件类型、硬件编号
    //输出：是否设置成功
    virtual bool setUseHardware(const std::vector<std::pair<DeepinOCRPlugin::HardwareID, int> > &hardwareUsed);

    //获取支持的硬件
    //输入：无
    //输出：支持的硬件列表，此处的输出结果将影响上层接口中的getBestPluginNames的结果
    //推荐策略：专有GPU > 通用GPU > 专有CPU > 通用CPU > 在线OCR
    //示例：GPU_AMD > GPU_Vulkan > CPU_Intel > CPU_Any > Network
    //注意：如果两个插件均满足当前环境，则会被getBestPluginNames一起返回
    virtual std::vector<HardwareID> getHardwareSupportList() = 0;

    //设置可使用的最大线程数
    //输入：最大线程数
    //输出：是否设置成功
    virtual bool setUseMaxThreadsCount(unsigned int n);

    //图像数据设置

    //获取支持的图片文件格式
    //输入：无
    //输出：支持的图片格式
    virtual std::vector<std::string> getImageFileSupportFormats();

    //获取图片路径
    //输入：图片路径
    //输出：是否设置成功
    virtual bool setImageFile(const std::string &filePath);

    //获取图像数据格式
    //输入：无
    //输出：插件内部接受的图像数据格式
    //注意：对于只能使用文件传输的插件，可以返回Pixel_Unknown，然后不实现setMatrix
    virtual PixelType getPixelType() = 0;

    //设置图像矩阵，因为大部分的图像处理都会涉及OpenCV，因此这个地方直接模仿cv::Mat的构造函数
    //输入：height：矩阵的高，width：矩阵的宽，data：指向矩阵的数据指针，step：矩阵每一行的字节数
    //输出：是否设置成功
    virtual bool setMatrix(int height, int width, unsigned char *data, size_t step);

    //鉴权设置

    //获取需要的鉴权要素
    //输入：无
    //输出：鉴权要素关键字
    virtual std::vector<std::string> getAuthKeys();

    //设置鉴权参数
    //输入：与鉴权要素关键字对应的鉴权参数
    //输出：是否设置成功
    virtual bool setAuth(std::initializer_list<std::string> params);

    //语种设置

    //获取支持的识别语种
    //输入：无
    //输出：支持的语种列表
    virtual std::vector<std::string> getLanguageSupport() = 0;

    //设置需要的语种
    //输入：希望使用的语种
    //输出：是否设置成功
    virtual bool setLanguage(const std::string &language) = 0;

    //万能拓展接口

    //设置数据
    //输入：关键字，值
    //输出：是否设置成功
    virtual bool setValue(const std::string &key, const std::string &value);

    //获取数据
    //输入：关键字
    //输出：值
    virtual std::string getValue(const std::string &key);

    //执行 OCR 识别

    //执行OCR识别（开始分析图片），此处需要实现为阻塞模式
    //输入：无
    //输出：是否识别到文本
    virtual bool analyze() = 0;

    //终止OCR识别，此处会由另外一个线程发起，需要插件设计者考虑线程安全
    //但部分场景可能无法或不好实现终止（比如在线识别），因此该接口为选择性实现
    //输入：无
    //输出：是否终止成功
    virtual bool breakAnalyze();

    //文本块位置输出

    //获取每一个文本块的位置
    //输入：无
    //输出：全部的文本块的位置
    virtual std::vector<TextBox> getTextBoxes() = 0;

    //文本块中每一个字符单元的位置输出

    //获取每一个文本块中每一个字符单元的位置
    //输入：文本块的编号，和textBoxes成员函数输出的vector一一对应
    //输出：该文本块下的字符位置
    virtual std::vector<TextBox> getCharBoxes(size_t index);

    //字符含义输出

    //获取整张图片输出的文本含义
    //输入：无
    //输出：整张图的总识别结果
    virtual std::string getAllResult() = 0;

    //获取单个文本块中的字符含义
    //输入：文本块的编号，和textBoxes成员函数输出的vector一一对应
    //输出：对应文本块的全部字符含义
    virtual std::string getResultFromBox(size_t index) = 0;
};

}
