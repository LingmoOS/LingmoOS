#include "photoswindow.h"
#include <filesystem>
#include <ui/application.h>
#include <ui/theme.h>
#include <ui/test_input.h>
#include <ui/window.h>
int main(int argc, char** argv) { if (argc != 2) return 1; std::filesystem::create_directories(argv[1]); Lingmo::ui::Application app(argc, argv, Lingmo::ui::Application::Platform::Headless); Lingmo::ui::TestInput::setGlassAvailable(true); Lingmo::ui::Theme::setCurrent(Lingmo::ui::Theme::light()); Photos::PhotosWindow window(std::string(argv[1]) + "/photos.db"); window.window().setScale(1.0); window.show(); return window.window().grab().savePng(std::string(argv[1]) + "/photos-preview.png") ? 0 : 2; }
