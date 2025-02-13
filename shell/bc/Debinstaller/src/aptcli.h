#ifndef APTCLI_H
#define APTCLI_H

#include <apt-pkg/cmndline.h>
#include <apt-pkg/configuration.h>
#include <apt-pkg/error.h>
#include <apt-pkg/init.h>

#include <apt-pkg/pkgsystem.h>
#include <apt-pkg/strutl.h>

#include <apt-private/private-cmndline.h>
#include <apt-private/private-depends.h>
#include <apt-private/private-download.h>
#include <apt-private/private-install.h>
#include <apt-private/private-list.h>
#include <apt-private/private-main.h>
#include <apt-private/private-moo.h>
#include <apt-private/private-output.h>
#include <apt-private/private-search.h>
#include <apt-private/private-show.h>
#include <apt-private/private-source.h>
#include <apt-private/private-sources.h>
#include <apt-private/private-update.h>
#include <apt-private/private-upgrade.h>

#include <vector>
#include <iostream>
#include "thirdparty/Apt/apt-private/private-cmndline.h"

// 函数声明
void DoList();
void DoSearch();
void ShowPackage();
void DoInstall();
void DoUpdate();
void DoUpgrade();
void DoDistUpgrade();

// 声明 DoDistClean 函数
void DoDistClean();

// 定义 Args_t 结构体
struct Args_t {
    int argc;
    const char **argv;

    // 默认构造函数
    Args_t() : argc(0), argv(nullptr) {}

    // 带参数的构造函数
    Args_t(int count, const char **arguments) : argc(count), argv(arguments) {}
};

class AptCli {
public:
    Args_t m_args{}; // 使用 Args_t 类型
    AptCli(int argc, const char **argv);

    static bool m_showHelp(CommandLine &);

    static std::vector<aptDispatchWithHelp> m_getCommands();
protected:
    std::vector<CommandLine::Dispatch> m_cmds;
};

#endif // APTCLI_H
