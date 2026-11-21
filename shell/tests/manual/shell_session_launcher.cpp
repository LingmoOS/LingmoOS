// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Development harness: does what lingmo-session does for the file manager's
// desktop, without starting a compositor or a session.
//
// It connects to the compositor's shell control socket, creates a Wayland
// socketpair, hands one end to the compositor over SCM_RIGHTS and execs the
// given program with `--desktop-socket <fd>` appended for the other end. That
// makes it possible to run `lingmo-filemanager --daemon` against a headless
// compositor from a test script.
//
// Usage: lingmo-shell-session-launcher <control-socket> <program> [args...]
//
// This duplicates a few lines of core/session's ShellConnection on purpose: the
// compositor repository must not depend on the session repository.

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <string>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <vector>

namespace {

int connectToControl(const char* path)
{
    sockaddr_un address {};
    address.sun_family = AF_UNIX;
    if (std::strlen(path) >= sizeof(address.sun_path)) {
        std::fprintf(stderr, "control socket path is too long\n");
        return -1;
    }
    std::strcpy(address.sun_path, path);

    const int fd = socket(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0);
    if (fd < 0)
        return -1;

    if (connect(fd, reinterpret_cast<sockaddr*>(&address), sizeof(address)) != 0) {
        std::perror("connect");
        close(fd);
        return -1;
    }
    return fd;
}

bool sendFd(int socket, int fd)
{
    char payload = 'S';
    iovec iov {};
    iov.iov_base = &payload;
    iov.iov_len = 1;

    union {
        char buffer[CMSG_SPACE(sizeof(int))];
        cmsghdr alignment;
    } control {};

    msghdr message {};
    message.msg_iov = &iov;
    message.msg_iovlen = 1;
    message.msg_control = control.buffer;
    message.msg_controllen = sizeof(control.buffer);

    cmsghdr* header = CMSG_FIRSTHDR(&message);
    header->cmsg_level = SOL_SOCKET;
    header->cmsg_type = SCM_RIGHTS;
    header->cmsg_len = CMSG_LEN(sizeof(int));
    std::memcpy(CMSG_DATA(header), &fd, sizeof(fd));

    return sendmsg(socket, &message, MSG_NOSIGNAL) == 1;
}

} // namespace

int main(int argc, char* argv[])
{
    if (argc < 3) {
        std::fprintf(stderr, "usage: %s <control-socket> <program> [args...]\n", argv[0]);
        return 2;
    }

    const int control = connectToControl(argv[1]);
    if (control < 0)
        return 1;

    int pair[2];
    if (socketpair(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0, pair) != 0) {
        std::perror("socketpair");
        return 1;
    }

    if (!sendFd(control, pair[0])) {
        std::perror("sendmsg");
        return 1;
    }
    close(pair[0]);

    // The compositor has to have turned the fd into a wl_client before the
    // program writes its first request, which it does as soon as it starts.
    usleep(200 * 1000);

    // Only the exec'd program keeps the connection.
    if (fcntl(pair[1], F_SETFD, 0) != 0) {
        std::perror("fcntl");
        return 1;
    }
    std::string fd = std::to_string(pair[1]);
    std::vector<char *> arguments(argv + 2, argv + argc);
    std::string flag = "--desktop-socket";
    arguments.push_back(flag.data());
    arguments.push_back(fd.data());
    arguments.push_back(nullptr);

    execvp(arguments.front(), arguments.data());
    std::perror("execvp");
    return 1;
}
