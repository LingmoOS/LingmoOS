// 
#include <deepinocrplugin.h>
#include <deepinocrplugin_p.h>
#include <toolkits.h>

#include <opencv2/opencv.hpp>

#include <algorithm>
#include <atomic>
#include <filesystem>

#include <dlfcn.h>

namespace DeepinOCRPlugin {

class DeepinOCRDriver_impl
{
public:
    ~DeepinOCRDriver_impl();

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

DeepinOCRDriver_impl::~DeepinOCRDriver_impl()
{
    resetDlHandle();
}

bool DeepinOCRDriver_impl::isCompatible(int version)
{
    if(version > VERSION) {
        DEEPIN_LOG("plugin version is too big");
        DEEPIN_LOG("plugin version: %d, manager version: %d", version, VERSION);
        return false;
    }

    return true;
}

void DeepinOCRDriver_impl::resetDlHandle()
{
    if(pluginIsLoaded) {
        if(isRunning || unloadPlugin(pluginImpl) == 0) { //正在运行或卸载失败
            DEEPIN_LOG("unload plugin failed");
            return;
        }
    }

    if(dlHandle != nullptr) { //pluginIsLoaded为false的时候，dlHandle可能不为nullptr
        dlclose(dlHandle);
        dlHandle = nullptr;
    }

    pluginIsLoaded = false;
}

DeepinOCRDriver::DeepinOCRDriver()
    : impl(new DeepinOCRDriver_impl)
{
    impl->pluginInstallDir = getCurrentModuleDir() + "/deepin-ocr-plugin-manager/";
}

DeepinOCRDriver::~DeepinOCRDriver()
{
    delete impl;
}

std::vector<std::string> DeepinOCRDriver::getPluginNames()
{
    return getSubDirNames(impl->pluginInstallDir);
}

std::vector<std::string> DeepinOCRDriver::getBestPluginNames()
{
    //TODO：需要deepin-GPUInfo支持
    DEEPIN_LOG("getBestPluginNames is not support yet");
    return std::vector<std::string>();
}

bool DeepinOCRDriver::loadDefaultPlugin()
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
        DEEPIN_LOG("default plugin load failed");
        return false;
    }
}

//加载符号
template <typename Func>
void loadSymbol(void *handle, Func** funcPtr, const std::string &symbol)
{
    *funcPtr = reinterpret_cast<Func*>(dlsym(handle, symbol.c_str()));
}

bool DeepinOCRDriver::loadPlugin(const std::string &pluginName)
{
    //校验插件名
    auto names = getPluginNames();
    bool pluginExist = std::any_of(names.begin(), names.end(), [pluginName](const std::string & name) {
        return pluginName == name;
    });
    if (!pluginExist) {
        DEEPIN_LOG("plugin %s is not existed", pluginName.c_str());
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
    DEEPIN_LOG("try to load .so: %s", pluginDlFilePath.c_str());
    auto tempHandle = dlopen(pluginDlFilePath.c_str(), RTLD_LAZY);
    if (tempHandle == nullptr) {
        DEEPIN_LOG("plugin %s dlopen failed", pluginName.c_str());
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
        DEEPIN_LOG("plugin %s load symbol failed", pluginName.c_str());
        return false;
    }

    //2.3 加载插件数据

    //获取版本号
    int version = verFunc();
    if(!impl->isCompatible(version)) { //兼容性判断，如果是完全不可兼容的，则直接放弃加载
        impl->resetDlHandle();
        DEEPIN_LOG("plugin %s version check failed", pluginName.c_str());
        return false;
    }
    impl->pluginVersion = version;

    //插件加载接口
    impl->pluginImpl = reinterpret_cast<Plugin *>(loadFunc());
    if(impl->pluginImpl == nullptr) { //插件加载失败
        impl->resetDlHandle();
        DEEPIN_LOG("plugin %s plugin impl load failed", pluginName.c_str());
        return false;
    }

    //插件卸载接口
    impl->unloadPlugin = unloadFunc;

    impl->pluginIsLoaded = true;

    DEEPIN_LOG("plugin %s load successed", pluginName.c_str());
    return true;
}

bool DeepinOCRDriver::pluginIsLoaded() const
{
    return impl->pluginIsLoaded;
}

bool DeepinOCRDriver::setUseHardware(const std::vector<std::pair<DeepinOCRPlugin::HardwareID, int> > &hardwareUsed)
{
    if(!pluginIsLoaded()) {
        DEEPIN_LOG("you need load a plugin first");
        return false;
    }

    return impl->pluginImpl->setUseHardware(hardwareUsed);
}

std::vector<HardwareID> DeepinOCRDriver::getHardwareSupportList()
{
    if(!pluginIsLoaded()) {
        DEEPIN_LOG("you need load a plugin first");
        return std::vector<HardwareID>();
    }

    return impl->pluginImpl->getHardwareSupportList();
}

bool DeepinOCRDriver::setUseMaxThreadsCount(unsigned int n)
{
    if(!pluginIsLoaded()) {
        DEEPIN_LOG("you need load a plugin first");
        return false;
    }

    return impl->pluginImpl->setUseMaxThreadsCount(n);
}

std::vector<std::string> DeepinOCRDriver::getAuthKeys()
{
    if(!pluginIsLoaded()) {
        DEEPIN_LOG("you need load a plugin first");
        return std::vector<std::string>();
    }

    return impl->pluginImpl->getAuthKeys();
}

bool DeepinOCRDriver::setAuth(std::initializer_list<std::string> params)
{
    if(!pluginIsLoaded()) {
        DEEPIN_LOG("you need load a plugin first");
        return false;
    }

    return impl->pluginImpl->setAuth(params);
}

std::vector<std::string> DeepinOCRDriver::getLanguageSupport()
{
    if(!pluginIsLoaded()) {
        DEEPIN_LOG("you need load a plugin first");
        return std::vector<std::string>();
    }

    return impl->pluginImpl->getLanguageSupport();
}

bool DeepinOCRDriver::setLanguage(const std::string &language)
{
    if(!pluginIsLoaded()) {
        DEEPIN_LOG("you need load a plugin first");
        return false;
    }

    return impl->pluginImpl->setLanguage(language);
}

std::vector<std::string> DeepinOCRDriver::getImageFileSupportFormats()
{
    if(!pluginIsLoaded()) {
        DEEPIN_LOG("you need load a plugin first");
        return std::vector<std::string>();
    } else {
        return impl->pluginImpl->getImageFileSupportFormats();
    }
}

bool DeepinOCRDriver::setImageFile(const std::string &filePath)
{
    if(!pluginIsLoaded()) {
        DEEPIN_LOG("you need load a plugin first");
        return false;
    }

    if(!std::filesystem::exists(filePath)) {
        DEEPIN_LOG("file %s is not exists", filePath.c_str());
        return false;
    } else {
        return impl->pluginImpl->setImageFile(filePath);
    }
}

bool DeepinOCRDriver::setMatrix(int height, int width, unsigned char *data, size_t step, PixelType type)
{
    if(!pluginIsLoaded()) {
        DEEPIN_LOG("you need load a plugin first");
        return false;
    }

    auto requestPixelType = impl->pluginImpl->getPixelType();

    if(requestPixelType == PixelType::Pixel_Unknown || type == PixelType::Pixel_Unknown) {
        if(requestPixelType == PixelType::Pixel_Unknown) {
            DEEPIN_LOG("plugin request pixel type is unknown, try setImageFile");
        } else {
            DEEPIN_LOG("your pixel type is unknown");
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
        DEEPIN_LOG("pixel convert failed");
        return false;
    } else {
        cv::cvtColor(mat, mat, cvtCode);
        return impl->pluginImpl->setMatrix(mat.rows, mat.cols, mat.data, mat.step);
    }
}

bool DeepinOCRDriver::setValue(const std::string &key, const std::string &value)
{
    if(!pluginIsLoaded()) {
        DEEPIN_LOG("you need load a plugin first");
        return false;
    }

    return impl->pluginImpl->setValue(key, value);
}

std::string DeepinOCRDriver::getValue(const std::string &key)
{
    if(!pluginIsLoaded()) {
        DEEPIN_LOG("you need load a plugin first");
        return "";
    }

    return impl->pluginImpl->getValue(key);
}

bool DeepinOCRDriver::analyze()
{
    if(!pluginIsLoaded()) {
        DEEPIN_LOG("you need load a plugin first");
        return false;
    }

    impl->isRunning = true;

    auto result = impl->pluginImpl->analyze();

    impl->isRunning = false;

    return result;
}

bool DeepinOCRDriver::breakAnalyze()
{
    if(!pluginIsLoaded()) {
        DEEPIN_LOG("you need load a plugin first");
        return false;
    }

    if(!impl->isRunning) {
        DEEPIN_LOG("you need start analyze first");
        return false;
    }

    return impl->pluginImpl->breakAnalyze();
}

bool DeepinOCRDriver::isRunning()
{
    if(!pluginIsLoaded()) {
        DEEPIN_LOG("you need load a plugin first");
        return false;
    }

    return impl->isRunning;
}

std::vector<TextBox> DeepinOCRDriver::getTextBoxes() const
{
    if(!pluginIsLoaded()) {
        DEEPIN_LOG("you need load a plugin first");
        return std::vector<TextBox>();
    }

    return impl->pluginImpl->getTextBoxes();
}

std::vector<TextBox> DeepinOCRDriver::getCharBoxes(size_t index) const
{
    if(!pluginIsLoaded()) {
        DEEPIN_LOG("you need load a plugin first");
        return std::vector<TextBox>();
    }

    return impl->pluginImpl->getCharBoxes(index);
}

std::string DeepinOCRDriver::getAllResult()
{
    if(!pluginIsLoaded()) {
        DEEPIN_LOG("you need load a plugin first");
        return "";
    }

    return impl->pluginImpl->getAllResult();
}

std::string DeepinOCRDriver::getResultFromBox(size_t index)
{
    if(!pluginIsLoaded()) {
        DEEPIN_LOG("you need load a plugin first");
        return "";
    }

    return impl->pluginImpl->getResultFromBox(index);
}

}
