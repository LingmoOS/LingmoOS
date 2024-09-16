#include <deepinocrplugin_p.h>
#include <toolkits.h>

namespace DeepinOCRPlugin {

bool Plugin::setUseHardware(const std::vector<std::pair<DeepinOCRPlugin::HardwareID, int> > &hardwareUsed)
{
    (void)hardwareUsed;
    DEEPIN_LOG("current plugin do not implement this function: %s", __FUNCTION__);

    return false;
}

bool Plugin::setUseMaxThreadsCount(unsigned int n)
{
    (void)n;
    DEEPIN_LOG("current plugin do not implement this function: %s", __FUNCTION__);

    return false;
}

std::vector<std::string> Plugin::getImageFileSupportFormats()
{
    DEEPIN_LOG("current plugin do not implement this function: %s", __FUNCTION__);

    return std::vector<std::string>();
}

bool Plugin::setImageFile(const std::string &filePath)
{
    (void)filePath;
    DEEPIN_LOG("current plugin do not implement this function: %s", __FUNCTION__);

    return false;
}

bool Plugin::setMatrix(int height, int width, unsigned char *data, size_t step)
{
    (void)height;
    (void)width;
    (void)data;
    (void)step;
    DEEPIN_LOG("current plugin do not implement this function: %s", __FUNCTION__);

    return false;
}

std::vector<std::string> Plugin::getAuthKeys()
{
    DEEPIN_LOG("current plugin do not implement this function: %s", __FUNCTION__);

    return std::vector<std::string>();
}

bool Plugin::setAuth(std::initializer_list<std::string> params)
{
    (void)params;
    DEEPIN_LOG("current plugin do not implement this function: %s", __FUNCTION__);

    return false;
}

bool Plugin::setValue(const std::string &key, const std::string &value)
{
    (void)key;
    (void)value;
    DEEPIN_LOG("current plugin do not implement this function: %s", __FUNCTION__);

    return false;
}

std::string Plugin::getValue(const std::string &key)
{
    (void)key;
    DEEPIN_LOG("current plugin do not implement this function: %s", __FUNCTION__);

    return "";
}

bool Plugin::breakAnalyze()
{
    DEEPIN_LOG("current plugin do not implement this function: %s", __FUNCTION__);

    return false;
}

std::vector<TextBox> Plugin::getCharBoxes(size_t index)
{
    (void)index;
    DEEPIN_LOG("current plugin do not implement this function: %s", __FUNCTION__);

    return std::vector<TextBox>();
}

}
