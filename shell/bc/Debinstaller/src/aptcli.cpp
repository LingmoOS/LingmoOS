#include "aptcli.h"

AptCli::AptCli(int argc, const char **argv) : m_args{argc, argv} {
  auto _cmdl = CommandLine();
  m_cmds = ParseCommandLine(_cmdl, APT_CMD::APT, &_config, &_system, argc, argv, &AptCli::m_showHelp, &AptCli::m_getCommands);
}
bool AptCli::m_showHelp(CommandLine &) {
  std::cout <<
      ("Usage: lingmo-debinstaller [options] command\n"
        "\n"
      "lingmo-debinstaller is a commandline and GUI package manager and provides commands for\n"
      "searching and managing as well as querying information about packages.\n"
      "It provides the same functionality as the specialized APT tools,\n"
      "like apt-get and apt-cache, but enables options more suitable for\n"
      "interactive use by default.\n");
  return true;
}
std::vector<aptDispatchWithHelp> AptCli::m_getCommands() {
  return {
    {"list", &DoList, "list packages based on package names"},
    {"search", &DoSearch, "search in package descriptions"},
    {"show", &ShowPackage, "show package details"},
    {"install", &DoInstall, "install packages"},
    {"reinstall", &DoInstall, "reinstall packages"},
    {"remove", &DoInstall, "remove packages"},
    {"update", &DoUpdate, "update list of available packages"},
    {"upgrade", &DoUpgrade, "upgrade the system by installing/upgrading packages"},
    {"full-upgrade", &DoDistUpgrade, "upgrade the system by removing/installing/upgrading packages"},
    {nullptr, nullptr, nullptr}
  };
}
