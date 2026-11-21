#include "tools.h"

#include <filesystem>

const Lingmo::base::LogCategory qtermwidgetLogger("qtermwidget", Lingmo::base::LogLevel::Warning);

static std::string kb_layout_dir = TERMINAL_DATA_DIR "/kb-layouts";
static std::string color_scheme_dir = TERMINAL_DATA_DIR "/color-schemes";

static bool directoryExists(const std::string& path)
{
    std::error_code error;
    return !path.empty() && std::filesystem::is_directory(path, error);
}

void set_kb_layout_dir(const std::string& dir)
{
    kb_layout_dir = dir;
}

void set_color_scheme_dir(const std::string& dir)
{
    color_scheme_dir = dir;
}

/*! Helper function to get possible location of layout files.
*/
std::string get_kb_layout_dir()
{
    if (directoryExists(kb_layout_dir))
        return kb_layout_dir + '/';

    return std::string();
}

/*! Helper function to add custom location of color schemes.
*/
static std::vector<std::string> custom_color_schemes_dirs;

void add_custom_color_scheme_dir(const std::string& custom_dir)
{
    if (std::find(custom_color_schemes_dirs.begin(), custom_color_schemes_dirs.end(), custom_dir)
            == custom_color_schemes_dirs.end())
        custom_color_schemes_dirs.push_back(custom_dir);
}

/*! Helper function to get possible locations of color schemes.
*/
const std::vector<std::string> get_color_schemes_dirs()
{
    std::vector<std::string> rval;

    if (directoryExists(color_scheme_dir))
        rval.push_back(color_scheme_dir + '/');

    for (const std::string& custom_dir : custom_color_schemes_dirs)
    {
        if (directoryExists(custom_dir))
            rval.push_back(custom_dir);
    }
    return rval;
}
