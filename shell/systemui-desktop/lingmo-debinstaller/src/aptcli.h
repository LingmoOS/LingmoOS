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

/**
 * @brief A struct to store command line args.
 */
typedef struct Args_s {
    int argc;
    const char** argv;
} Args_t;

class AptCli {
public:
    explicit AptCli(int argc = 0, const char **argv = nullptr);

    static bool m_showHelp(CommandLine &);

    static std::vector<aptDispatchWithHelp> m_getCommands();
protected:
    Args_t m_args{};

    std::vector<CommandLine::Dispatch> m_cmds;
};

#endif // APTCLI_H
