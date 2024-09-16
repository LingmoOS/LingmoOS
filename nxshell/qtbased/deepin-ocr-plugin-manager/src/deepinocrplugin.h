#pragma once

#include "deepinocrplugindef.h"

#include <vector>
#include <string>

namespace DeepinOCRPlugin {

class DeepinOCRDriver_impl;

class DEEPIN_EXPORTS DeepinOCRDriver
{
public:
    DeepinOCRDriver();
    ~DeepinOCRDriver();

    //插件扫描与加载服务

    //告知上层应用目前已安装了哪些插件
    //输入：无
    //输出：插件名称列表，不包括默认插件
    std::vector<std::string> getPluginNames();
    
    //告知上层应用根据目前的硬件环境推荐用哪几个插件
    //输入：无
    //输出：推荐使用的插件名，返回为空则表示功能不支持或仅存在默认插件
    //注意：由于多个插件可能都适合当前环境使用，因此这里采用多输出的方式
    std::vector<std::string> getBestPluginNames();
    
    //设置加载系统默认的插件，此插件由本二次开发库直接提供
    //输入：无
    //输出：是否加载成功
    bool loadDefaultPlugin();
    
    //设置要加载的插件名
    //输入：插件名
    //输出：是否加载成功
    bool loadPlugin(const std::string &pluginName);
    
    //插件是否已加载
    //输入：无
    //输出：是否已加载插件
    bool pluginIsLoaded() const;
    
    //硬件加速设置
    
    //设置需要使用的硬件
    //输入：硬件类型、硬件编号
    //输出：是否设置成功
    bool setUseHardware(const std::vector<std::pair<DeepinOCRPlugin::HardwareID, int> > &hardwareUsed);
    
    //获取当前已加载插件所支持的硬件
    //输入：无
    //输出：支持的硬件列表
    std::vector<HardwareID> getHardwareSupportList();
    
    //设置可使用的最大线程数
    //输入：最大线程数
    //输出：是否设置成功
    bool setUseMaxThreadsCount(unsigned int n);
    
    //鉴权设置
    
    //获取需要的鉴权要素，比如UserName、APISecret、APPID等
    //输入：无
    //输出：鉴权要素关键字
    std::vector<std::string> getAuthKeys();
    
    //设置鉴权参数
    //输入：与鉴权要素关键字对应的鉴权参数
    //输出：是否设置成功
    bool setAuth(std::initializer_list<std::string> params);
    
    //语种设置
    
    //获取支持的识别语种
    //输入：无
    //输出：支持的语种列表
    std::vector<std::string> getLanguageSupport();
    
    //设置需要的语种
    //输入：希望使用的语种
    //输出：是否设置成功
    bool setLanguage(const std::string &language);
    
    //符合算法要求的图像

    //获取支持的图片文件格式
    //输入：无
    //输出：支持的图片格式
    std::vector<std::string> getImageFileSupportFormats();
    
    //获取图片路径
    //输入：图片路径
    //输出：是否设置成功
    bool setImageFile(const std::string &filePath);
    
    //设置图像矩阵，因为大部分的图像处理都会涉及OpenCV，因此这个地方直接模仿cv::Mat的构造函数
    //输入：height：矩阵的高，width：矩阵的宽，data：指向矩阵的数据指针，step：矩阵每一行的字节数，type：传入矩阵的数据格式
    //输出：是否设置成功
    bool setMatrix(int height, int width, unsigned char *data, size_t step, PixelType type);
    
    //万能拓展接口
    
    //设置数据
    //输入：关键字，值
    //输出：是否设置成功
    bool setValue(const std::string &key, const std::string &value);
    
    //获取数据
    //输入：关键字
    //输出：值
    std::string getValue(const std::string &key);
    
    //执行 OCR 识别
    
    //执行OCR识别（开始分析图片），此处需要实现为阻塞模式
    //输入：无
    //输出：是否识别到文本
    bool analyze();

    //终止OCR识别，此处会由另外一个线程发起，需要插件设计者考虑线程安全
    //但部分场景可能无法或不好实现终止（比如在线识别），因此该接口为选择性实现
    //输入：无
    //输出：是否终止成功
    bool breakAnalyze();
    
    //识别进行中
    //输入：无
    //输出：是否正在进行识别
    bool isRunning();
    
    //文本块的位置
    
    //获取每一个文本块的位置
    //输入：无
    //输出：全部的文本块的位置
    std::vector<TextBox> getTextBoxes() const;
    
    //文本块中每一个字符单元的位置
    
    //获取每一个文本块中每一个字符单元的位置
    //输入：文本块的编号，和textBoxes成员函数输出的vector一一对应
    //输出：该文本块下的字符位置
    std::vector<TextBox> getCharBoxes(size_t index) const;
    
    //字符含义
    
    //获取整张图片输出的文本含义
    //输入：无
    //输出：整张图的总识别结果
    std::string getAllResult();
    
    //获取单个文本块中的字符含义
    //输入：文本块的编号，和textBoxes成员函数输出的vector一一对应
    //输出：对应文本块的全部字符含义
    std::string getResultFromBox(size_t index);

private:
    DeepinOCRDriver_impl *impl;
};

}
