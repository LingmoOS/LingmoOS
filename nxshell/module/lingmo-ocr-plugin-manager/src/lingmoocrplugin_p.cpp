#include <lingmoocrplugin_p.h>
#include <toolkits.h>

namespace LingmoOCRPlugin {

bool Plugin::setUseHardware(const std::vector<std::pair<LingmoOCRPlugin::HardwareID, int> > &hardwareUsed)
{
    (void)hardwareUsed;
    LINGMO_LOG("current plugin do not implement this function: %s", __FUNCTION__);

    return false;
}

bool Plugin::setUseMaxThreadsCount(unsigned int n)
{
    (void)n;
    LINGMO_LOG("current plugin do not implement this function: %s", __FUNCTION__);

    return false;
}

std::vector<std::string> Plugin::getImageFileSupportFormats()
{
    LINGMO_LOG("current plugin do not implement this function: %s", __FUNCTION__);

    return std::vector<std::string>();
}

bool Plugin::setImageFile(const std::string &filePath)
{
    (void)filePath;
    LINGMO_LOG("current plugin do not implement this function: %s", __FUNCTION__);

    return false;
}

bool Plugin::setMatrix(int height, int width, unsigned char *data, size_t step)
{
    (void)height;
    (void)width;
    (void)data;
    (void)step;
    LINGMO_LOG("current plugin do not implement this function: %s", __FUNCTION__);

    return false;
}

std::vector<std::string> Plugin::getAuthKeys()
{
    LINGMO_LOG("current plugin do not implement this function: %s", __FUNCTION__);

    return std::vector<std::string>();
}

bool Plugin::setAuth(std::initializer_list<std::string> params)
{
    (void)params;
    LINGMO_LOG("current plugin do not implement this function: %s", __FUNCTION__);

    return false;
}

bool Plugin::setValue(const std::string &key, const std::string &value)
{
    (void)key;
    (void)value;
    LINGMO_LOG("current plugin do not implement this function: %s", __FUNCTION__);

    return false;
}

std::string Plugin::getValue(const std::string &key)
{
    (void)key;
    LINGMO_LOG("current plugin do not implement this function: %s", __FUNCTION__);

    return "";
}

bool Plugin::breakAnalyze()
{
    LINGMO_LOG("current plugin do not implement this function: %s", __FUNCTION__);

    return false;
}

std::vector<TextBox> Plugin::getCharBoxes(size_t index)
{
    (void)index;
    LINGMO_LOG("current plugin do not implement this function: %s", __FUNCTION__);

    return std::vector<TextBox>();
}

}
