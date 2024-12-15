// 
#include <lingmoocrplugin.h>
#include <lingmoocrplugin_p.h>
#include <toolkits.h>

#include <opencv2/opencv.hpp>

#include <algorithm>
#include <atomic>
#include <filesystem>

#include <dlfcn.h>

namespace LingmoOCRPlugin {

class LingmoOCRDriver_impl
{
public:
    ~LingmoOCRDriver_impl();

    //判断版本是否兼容
    bool isCompatible(int version);

    //重置动态库状态
    void resetDlHandle();

    //插件安装位置
    std::string pluginInstallDir;

    //插件名
    std::string pluginName;

    //动态库handle
    void *dlHandle = nullptr;

    //插件接口
    Plugin *pluginImpl;

    //插件卸载接口
    int (*unloadPlugin)(void *);

    //插件版本号
    int pluginVersion;

    //插件加载标记
    bool pluginIsLoaded = false;

    //运行标记
    std::atomic_bool isRunning = false;

    //占位
    char r[2];
};

LingmoOCRDriver_impl::~LingmoOCRDriver_impl()
{
    resetDlHandle();
}

bool LingmoOCRDriver_impl::isCompatible(int version)
{
    if(version > VERSION) {
        LINGMO_LOG("plugin version is too big");
        LINGMO_LOG("plugin version: %d, manager version: %d", version, VERSION);
        return false;
    }

    return true;
}

void LingmoOCRDriver_impl::resetDlHandle()
{
    if(pluginIsLoaded) {
        if(isRunning || unloadPlugin(pluginImpl) == 0) { //正在运行或卸载失败
            LINGMO_LOG("unload plugin failed");
            return;
        }
    }

    if(dlHandle != nullptr) { //pluginIsLoaded为false的时候，dlHandle可能不为nullptr
        dlclose(dlHandle);
        dlHandle = nullptr;
    }

    pluginIsLoaded = false;
}

LingmoOCRDriver::LingmoOCRDriver()
    : impl(new LingmoOCRDriver_impl)
{
    impl->pluginInstallDir = getCurrentModuleDir() + "/lingmo-ocr-plugin-manager/";
}

LingmoOCRDriver::~LingmoOCRDriver()
{
    delete impl;
}

std::vector<std::string> LingmoOCRDriver::getPluginNames()
{
    return getSubDirNames(impl->pluginInstallDir);
}

std::vector<std::string> LingmoOCRDriver::getBestPluginNames()
{
    //TODO：需要lingmo-GPUInfo支持
    LINGMO_LOG("getBestPluginNames is not support yet");
    return std::vector<std::string>();
}

bool LingmoOCRDriver::loadDefaultPlugin()
{
    //重置handle
    impl->resetDlHandle();

    //加载默认插件
    impl->unloadPlugin = ::unloadPlugin;
    impl->pluginImpl = reinterpret_cast<Plugin *>(::loadPlugin());
    if(impl->pluginImpl != nullptr) {
        impl->pluginIsLoaded = true;
        return true;
    } else {
        LINGMO_LOG("default plugin load failed");
        return false;
    }
}

//加载符号
template <typename Func>
void loadSymbol(void *handle, Func** funcPtr, const std::string &symbol)
{
    *funcPtr = reinterpret_cast<Func*>(dlsym(handle, symbol.c_str()));
}

bool LingmoOCRDriver::loadPlugin(const std::string &pluginName)
{
    //校验插件名
    auto names = getPluginNames();
    bool pluginExist = std::any_of(names.begin(), names.end(), [pluginName](const std::string & name) {
        return pluginName == name;
    });
    if (!pluginExist) {
        LINGMO_LOG("plugin %s is not existed", pluginName.c_str());
        return false;
    }

    //执行加载步骤

    //0.重置状态
    impl->resetDlHandle();

    //1.定位插件位置
    auto pluginRootPath = impl->pluginInstallDir + pluginName;
    auto pluginDlFilePath = pluginRootPath + "/libload.so";

    //2.执行加载

    //2.1 打开库
    LINGMO_LOG("try to load .so: %s", pluginDlFilePath.c_str());
    auto tempHandle = dlopen(pluginDlFilePath.c_str(), RTLD_LAZY);
    if (tempHandle == nullptr) {
        LINGMO_LOG("plugin %s dlopen failed", pluginName.c_str());
        return false;
    }

    impl->dlHandle = tempHandle;

    //2.2 加载符号
    void*(*loadFunc)();
    int(*unloadFunc)(void*);
    int(*verFunc)();
    loadSymbol(impl->dlHandle, &loadFunc, "loadPlugin");
    loadSymbol(impl->dlHandle, &unloadFunc, "unloadPlugin");
    loadSymbol(impl->dlHandle, &verFunc, "pluginVersion");
    if(loadFunc == nullptr || unloadFunc == nullptr || verFunc == nullptr) {
        impl->resetDlHandle();
        LINGMO_LOG("plugin %s load symbol failed", pluginName.c_str());
        return false;
    }

    //2.3 加载插件数据

    //获取版本号
    int version = verFunc();
    if(!impl->isCompatible(version)) { //兼容性判断，如果是完全不可兼容的，则直接放弃加载
        impl->resetDlHandle();
        LINGMO_LOG("plugin %s version check failed", pluginName.c_str());
        return false;
    }
    impl->pluginVersion = version;

    //插件加载接口
    impl->pluginImpl = reinterpret_cast<Plugin *>(loadFunc());
    if(impl->pluginImpl == nullptr) { //插件加载失败
        impl->resetDlHandle();
        LINGMO_LOG("plugin %s plugin impl load failed", pluginName.c_str());
        return false;
    }

    //插件卸载接口
    impl->unloadPlugin = unloadFunc;

    impl->pluginIsLoaded = true;

    LINGMO_LOG("plugin %s load successed", pluginName.c_str());
    return true;
}

bool LingmoOCRDriver::pluginIsLoaded() const
{
    return impl->pluginIsLoaded;
}

bool LingmoOCRDriver::setUseHardware(const std::vector<std::pair<LingmoOCRPlugin::HardwareID, int> > &hardwareUsed)
{
    if(!pluginIsLoaded()) {
        LINGMO_LOG("you need load a plugin first");
        return false;
    }

    return impl->pluginImpl->setUseHardware(hardwareUsed);
}

std::vector<HardwareID> LingmoOCRDriver::getHardwareSupportList()
{
    if(!pluginIsLoaded()) {
        LINGMO_LOG("you need load a plugin first");
        return std::vector<HardwareID>();
    }

    return impl->pluginImpl->getHardwareSupportList();
}

bool LingmoOCRDriver::setUseMaxThreadsCount(unsigned int n)
{
    if(!pluginIsLoaded()) {
        LINGMO_LOG("you need load a plugin first");
        return false;
    }

    return impl->pluginImpl->setUseMaxThreadsCount(n);
}

std::vector<std::string> LingmoOCRDriver::getAuthKeys()
{
    if(!pluginIsLoaded()) {
        LINGMO_LOG("you need load a plugin first");
        return std::vector<std::string>();
    }

    return impl->pluginImpl->getAuthKeys();
}

bool LingmoOCRDriver::setAuth(std::initializer_list<std::string> params)
{
    if(!pluginIsLoaded()) {
        LINGMO_LOG("you need load a plugin first");
        return false;
    }

    return impl->pluginImpl->setAuth(params);
}

std::vector<std::string> LingmoOCRDriver::getLanguageSupport()
{
    if(!pluginIsLoaded()) {
        LINGMO_LOG("you need load a plugin first");
        return std::vector<std::string>();
    }

    return impl->pluginImpl->getLanguageSupport();
}

bool LingmoOCRDriver::setLanguage(const std::string &language)
{
    if(!pluginIsLoaded()) {
        LINGMO_LOG("you need load a plugin first");
        return false;
    }

    return impl->pluginImpl->setLanguage(language);
}

std::vector<std::string> LingmoOCRDriver::getImageFileSupportFormats()
{
    if(!pluginIsLoaded()) {
        LINGMO_LOG("you need load a plugin first");
        return std::vector<std::string>();
    } else {
        return impl->pluginImpl->getImageFileSupportFormats();
    }
}

bool LingmoOCRDriver::setImageFile(const std::string &filePath)
{
    if(!pluginIsLoaded()) {
        LINGMO_LOG("you need load a plugin first");
        return false;
    }

    if(!std::filesystem::exists(filePath)) {
        LINGMO_LOG("file %s is not exists", filePath.c_str());
        return false;
    } else {
        return impl->pluginImpl->setImageFile(filePath);
    }
}

bool LingmoOCRDriver::setMatrix(int height, int width, unsigned char *data, size_t step, PixelType type)
{
    if(!pluginIsLoaded()) {
        LINGMO_LOG("you need load a plugin first");
        return false;
    }

    auto requestPixelType = impl->pluginImpl->getPixelType();

    if(requestPixelType == PixelType::Pixel_Unknown || type == PixelType::Pixel_Unknown) {
        if(requestPixelType == PixelType::Pixel_Unknown) {
            LINGMO_LOG("plugin request pixel type is unknown, try setImageFile");
        } else {
            LINGMO_LOG("your pixel type is unknown");
        }
        return false;
    }

    //一样则直接传输数据
    if(type == requestPixelType) {
        return impl->pluginImpl->setMatrix(height, width, data, step);
    }

    //不一样的时候先执行转换操作，此处使用opencv完成

    //构建
    cv::Mat mat;

    switch(type)
    {
    default:
        break;
    case PixelType::Pixel_GRAY:
        mat = cv::Mat(height, width, CV_8UC1, data, step);
        break;
    case PixelType::Pixel_RGB:
    case PixelType::Pixel_BGR:
        mat = cv::Mat(height, width, CV_8UC3, data, step);
        break;
    case PixelType::Pixel_RGBA:
    case PixelType::Pixel_BGRA:
        mat = cv::Mat(height, width, CV_8UC4, data, step);
        break;
    };

    //获得转换代码
    int cvtCode = -1;
    if(type == PixelType::Pixel_GRAY) {//灰度图
        if(requestPixelType == PixelType::Pixel_BGR || requestPixelType == PixelType::Pixel_RGB) {
            cvtCode = cv::COLOR_GRAY2RGB;
        } else if(requestPixelType == PixelType::Pixel_BGRA || requestPixelType == PixelType::Pixel_RGBA) {
            cvtCode = cv::COLOR_GRAY2RGBA;
        }
    } else if(type == PixelType::Pixel_BGR || type == PixelType::Pixel_RGB) { //三通道图
        if(type == PixelType::Pixel_BGR && requestPixelType == PixelType::Pixel_GRAY) {
            cvtCode = cv::COLOR_BGR2GRAY;
        } else if(type == PixelType::Pixel_RGB && requestPixelType == PixelType::Pixel_GRAY) {
            cvtCode = cv::COLOR_RGB2GRAY;
        } else if(type == PixelType::Pixel_BGR && requestPixelType == PixelType::Pixel_RGB) {
            cvtCode = cv::COLOR_BGR2RGB;
        } else if(type == PixelType::Pixel_RGB && requestPixelType == PixelType::Pixel_BGR) {
            cvtCode = cv::COLOR_RGB2BGR;
        } else if(type == PixelType::Pixel_BGR && requestPixelType == PixelType::Pixel_RGBA) {
            cvtCode = cv::COLOR_BGR2RGBA;
        } else if(type == PixelType::Pixel_RGB && requestPixelType == PixelType::Pixel_BGRA) {
            cvtCode = cv::COLOR_RGB2BGRA;
        } else if((type == PixelType::Pixel_RGB && requestPixelType == PixelType::Pixel_RGBA) ||
                  (type == PixelType::Pixel_BGR && requestPixelType == PixelType::Pixel_BGRA)) {
            cvtCode = cv::COLOR_RGB2RGBA;
        }
    } else if(type == PixelType::Pixel_BGRA || type == PixelType::Pixel_RGBA) { //四通道图
        if(type == PixelType::Pixel_BGRA && requestPixelType == PixelType::Pixel_GRAY) {
            cvtCode = cv::COLOR_BGRA2GRAY;
        } else if(type == PixelType::Pixel_RGBA && requestPixelType == PixelType::Pixel_GRAY) {
            cvtCode = cv::COLOR_RGBA2GRAY;
        } else if(type == PixelType::Pixel_BGRA && requestPixelType == PixelType::Pixel_RGB) {
            cvtCode = cv::COLOR_BGRA2RGB;
        } else if(type == PixelType::Pixel_RGBA && requestPixelType == PixelType::Pixel_BGR) {
            cvtCode = cv::COLOR_RGBA2BGR;
        } else if(type == PixelType::Pixel_BGRA && requestPixelType == PixelType::Pixel_RGBA) {
            cvtCode = cv::COLOR_BGRA2RGBA;
        } else if(type == PixelType::Pixel_RGBA && requestPixelType == PixelType::Pixel_BGRA) {
            cvtCode = cv::COLOR_RGBA2BGRA;
        } else if((type == PixelType::Pixel_RGBA && requestPixelType == PixelType::Pixel_RGB) ||
                  (type == PixelType::Pixel_BGRA && requestPixelType == PixelType::Pixel_BGR)) {
            cvtCode = cv::COLOR_RGBA2RGB;
        }
    }

    //执行转换
    if(cvtCode == -1) {
        LINGMO_LOG("pixel convert failed");
        return false;
    } else {
        cv::cvtColor(mat, mat, cvtCode);
        return impl->pluginImpl->setMatrix(mat.rows, mat.cols, mat.data, mat.step);
    }
}

bool LingmoOCRDriver::setValue(const std::string &key, const std::string &value)
{
    if(!pluginIsLoaded()) {
        LINGMO_LOG("you need load a plugin first");
        return false;
    }

    return impl->pluginImpl->setValue(key, value);
}

std::string LingmoOCRDriver::getValue(const std::string &key)
{
    if(!pluginIsLoaded()) {
        LINGMO_LOG("you need load a plugin first");
        return "";
    }

    return impl->pluginImpl->getValue(key);
}

bool LingmoOCRDriver::analyze()
{
    if(!pluginIsLoaded()) {
        LINGMO_LOG("you need load a plugin first");
        return false;
    }

    impl->isRunning = true;

    auto result = impl->pluginImpl->analyze();

    impl->isRunning = false;

    return result;
}

bool LingmoOCRDriver::breakAnalyze()
{
    if(!pluginIsLoaded()) {
        LINGMO_LOG("you need load a plugin first");
        return false;
    }

    if(!impl->isRunning) {
        LINGMO_LOG("you need start analyze first");
        return false;
    }

    return impl->pluginImpl->breakAnalyze();
}

bool LingmoOCRDriver::isRunning()
{
    if(!pluginIsLoaded()) {
        LINGMO_LOG("you need load a plugin first");
        return false;
    }

    return impl->isRunning;
}

std::vector<TextBox> LingmoOCRDriver::getTextBoxes() const
{
    if(!pluginIsLoaded()) {
        LINGMO_LOG("you need load a plugin first");
        return std::vector<TextBox>();
    }

    return impl->pluginImpl->getTextBoxes();
}

std::vector<TextBox> LingmoOCRDriver::getCharBoxes(size_t index) const
{
    if(!pluginIsLoaded()) {
        LINGMO_LOG("you need load a plugin first");
        return std::vector<TextBox>();
    }

    return impl->pluginImpl->getCharBoxes(index);
}

std::string LingmoOCRDriver::getAllResult()
{
    if(!pluginIsLoaded()) {
        LINGMO_LOG("you need load a plugin first");
        return "";
    }

    return impl->pluginImpl->getAllResult();
}

std::string LingmoOCRDriver::getResultFromBox(size_t index)
{
    if(!pluginIsLoaded()) {
        LINGMO_LOG("you need load a plugin first");
        return "";
    }

    return impl->pluginImpl->getResultFromBox(index);
}

}
