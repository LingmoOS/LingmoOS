#include "dareader/reader.h"

#include <unistd.h>

#include <iostream>
#include <thread>

const uint32_t message_type_frame = 0;
const DA_img_type DA_img_type_RGB24 = 0;

void do_read_frames(int fd, void *const context, DA_read_frames_cb cb);
bool read_with_retry(int fd, char *buf, size_t len)
{
    while (true) {
        errno = 0;
        ssize_t n = read(fd, buf, len);
        if (n == 0)
            return false;
        if (n < 0) {
            if (errno == EAGAIN) {
                usleep(1);
                continue;
            } else {
                return false;
            }
        }
        if (n == len)
            break;
        if (n != len) {
            buf += n;
            len -= n;
        }
    }
    return true;
}

void DA_read_frames(int fd, void *const context, DA_read_frames_cb cb)
{
    std::thread t(do_read_frames, fd, context, cb);
    t.detach();
}

class img_reader
{
    int fd;
    uint32_t width, height;
    void *const context;
    DA_read_frames_cb const cb;
    DA_img_type type;

public:
    img_reader(int fd, void *const context, DA_read_frames_cb cb)
        : fd(fd)
        , context(context)
        , cb(cb)
    {
        if (cb == nullptr) {
            std::cerr << "empty cb" << std::endl;
            throw std::string("empty cb");
        }
        width = 0;
        height = 0;
    }
    // TODO 回调增加状态码，告知控制中心录入中获取图片状态
    bool readHead()
    {
        char *buf = (char *)malloc(16);
        if (!read_with_retry(fd, buf, 16)) {
            std::cerr << "init failed" << std::endl;
            return false;
        }
        uint32_t *P = (uint32_t *)buf;
        if (P[ 0 ] != message_type_frame) {
            std::cerr << "Wrong message type! not frame" << std::endl;
            return false;
        }
        type = P[ 1 ];
        width = P[ 2 ];
        height = P[ 3 ];
        free(buf);
        return true;
    }

    void run()
    {
        /*
       * data from fd:
       * [0:3]               message_type
       * [4:7]               image_type
       * [8:11]              width
       * [12:15]             height
       * --- loop begin ---
       * [16:19]             size1
       * [20:19+size]        img1 data
       * [20+size:23+size]   size2
       * [24+size:23+2*size] img2 data
       * ...
       */
        char *const header_buf = (char *)malloc(4);
        char *body_buf = nullptr;

        char *header_ptr = header_buf;
        char *body_ptr = body_buf;

        size_t body_size = 0, header_size = 4;

        DA_img img {};

        char *const msg = (char *)malloc(1);
        *msg = ' ';

        while (true) {
            if (write(fd, msg, 1) == 1) {
            } else {
                std::cerr << "ping server error, errno:" << errno;
                break;
            }
            if (read_with_retry(fd, header_buf, header_size)) {
                uint32_t *P = (uint32_t *)header_buf;
                img.depth = 8;
                img.channel = 3;
                img.width = width;
                img.height = height;
                img.size = P[ 0 ];
                img.type = type;
            } else {
                break;
            }
            if (body_size < img.size) {
                free(body_buf);
                body_buf = (char *)malloc(img.size);
                body_ptr = body_buf;
                body_size = img.size;
            }
            if (read_with_retry(fd, body_buf, img.size)) {
                img.data = body_ptr;
                cb(context, &img);
            } else {
                break;
            }
        }
        free(header_buf);
        free(body_buf);
    }
};

void do_read_frames(int fd, void *const context, DA_read_frames_cb cb)
{
    img_reader reader(fd, context, cb);

    if (!reader.readHead()) {
        return;
    }

    reader.run();
}
