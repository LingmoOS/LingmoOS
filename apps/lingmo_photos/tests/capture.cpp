#include "photoswindow.h"
#include <cstdlib>
#include <filesystem>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <ui/application.h>
#include <ui/theme.h>
#include <ui/test_input.h>
#include <ui/window.h>

int main(int argc, char** argv)
{
    if (argc != 2)
        return 1;
    std::filesystem::create_directories(argv[1]);
    const auto testHome = std::filesystem::path(argv[1]) / "home";
    std::filesystem::create_directories(testHome / "Pictures");
    setenv("HOME", testHome.c_str(), 1);
    const auto samplePath = std::filesystem::path(argv[1]) / "sample.png";
    GdkPixbuf* sample = gdk_pixbuf_new(GDK_COLORSPACE_RGB, false, 8, 8, 8);
    if (!sample)
        return 2;
    gdk_pixbuf_fill(sample, 0x335577ff);
    GError* error = nullptr;
    const bool saved = gdk_pixbuf_save(sample, samplePath.c_str(), "png", &error, nullptr);
    if (error)
        g_error_free(error);
    g_object_unref(sample);
    if (!saved)
        return 3;

    Lingmo::ui::Application app(argc, argv, Lingmo::ui::Application::Platform::Headless);
    Lingmo::ui::TestInput::setGlassAvailable(true);
    Lingmo::ui::Theme::setCurrent(Lingmo::ui::Theme::light());
    Photos::PhotosWindow window((std::filesystem::path(argv[1]) / "photos.db").string());
    window.window().setScale(1.0);
    window.openFile(samplePath.string());
    window.show();
    return window.window().grab().savePng((std::filesystem::path(argv[1]) / "photos-preview.png").string()) ? 0 : 4;
}
