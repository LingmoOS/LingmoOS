#include "recorder.h"

#include "base/command_line.h"
#include "base/event_loop.h"
#include "base/strings.h"

#include <csignal>
#include <cstdio>
#include <string_view>

#include <fcntl.h>
#include <sys/epoll.h>
#include <sys/signalfd.h>
#include <unistd.h>

namespace base = Lingmo::Base;

// Prints "started" at the first frame; "stop" or the end of standard input ends
// the recording, and the exit status says whether the file was completed.
int main(int argc, char **argv)
{
    // Blocked before any thread exists, so every thread inherits it and the
    // signals arrive only through the signalfd.
    sigset_t blocked;
    sigemptyset(&blocked);
    sigaddset(&blocked, SIGINT);
    sigaddset(&blocked, SIGTERM);
    sigaddset(&blocked, SIGHUP);
    pthread_sigmask(SIG_BLOCK, &blocked, nullptr);
    std::signal(SIGPIPE, SIG_IGN);

    base::EventLoop loop;

    base::CommandLineParser parser;
    parser.setApplicationDescription("Records the screen to an MP4 file.");
    parser.addHelpOption();
    const base::CommandLineOption outputOption("output", "Output to record.", "name");
    const base::CommandLineOption regionOption("region", "Part of the output, in global logical coordinates.",
                                               "x,y,width,height");
    const base::CommandLineOption windowOption("window", "Window to record.", "identifier");
    const base::CommandLineOption cursorOption("cursor", "Include the pointer.");
    const base::CommandLineOption microphoneOption("microphone", "Record this input, or \"default\".", "node");
    for (const base::CommandLineOption &option :
         {outputOption, regionOption, windowOption, cursorOption, microphoneOption})
        parser.addOption(option);
    parser.addPositionalArgument("file", "The MP4 file to write.");
    parser.process(argc, argv);

    Recorder::Options options;
    options.output = parser.value(outputOption);
    options.window = parser.value(windowOption);
    options.cursor = parser.isSet(cursorOption);
    if (parser.isSet(microphoneOption)) {
        options.microphone = true;
        const std::string target = parser.value(microphoneOption);
        if (target != "default")
            options.microphoneTarget = target;
    }
    if (parser.isSet(regionOption)) {
        const std::vector<std::string> parts = base::strings::split(parser.value(regionOption), ',');
        if (parts.size() == 4) {
            const auto number = [](const std::string &text) {
                return base::strings::toNumber<int>(base::strings::trimmed(text)).value_or(0);
            };
            options.region = base::Rect(number(parts[0]), number(parts[1]), number(parts[2]), number(parts[3]));
        }
    }
    if (parser.positionalArguments().size() != 1 || (options.output.empty() == options.window.empty())) {
        fprintf(stderr, "Give one file and either --output or --window.\n");
        return 2;
    }
    options.path = parser.positionalArguments().front();

    Recorder recorder(options, loop);
    recorder.onStarted = [] {
        printf("started\n");
        fflush(stdout);
    };
    recorder.onFinished = [&loop](bool ok, const std::string &error) {
        if (!ok)
            fprintf(stderr, "%s\n", error.c_str());
        loop.quit(ok ? 0 : 1);
    };

    std::string error;
    if (!recorder.start(&error)) {
        fprintf(stderr, "%s\n", error.c_str());
        return 1;
    }

    const int signalFd = signalfd(-1, &blocked, SFD_CLOEXEC | SFD_NONBLOCK);
    loop.watch(signalFd, EPOLLIN, [&](uint32_t) {
        signalfd_siginfo info;
        while (read(signalFd, &info, sizeof(info)) == sizeof(info)) { }
        recorder.stop();
    });

    int input = 0;
    input = loop.watch(STDIN_FILENO, EPOLLIN, [&](uint32_t) {
        char buffer[64];
        const ssize_t length = read(STDIN_FILENO, buffer, sizeof(buffer));
        if (length <= 0 || std::string_view(buffer, size_t(length)).find("stop") != std::string_view::npos) {
            loop.unwatch(input);
            recorder.stop();
        }
    });
    // A file or /dev/null cannot be watched; read, it would end at once.
    if (!input && fcntl(STDIN_FILENO, F_GETFD) >= 0)
        recorder.stop();

    const int result = loop.run();
    close(signalFd);
    return result;
}
