#include "ddcworker.h"

#include <chrono>
#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "log.h"

static constexpr int DdcAddress = 0x37;
static constexpr uint8_t HostAddress = 0x51;
static constexpr uint8_t BrightnessCode = 0x10;
static constexpr int Attempts = 3;

static void sleepFor(int milliseconds)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
}

namespace {

class I2cDevice
{
public:
    explicit I2cDevice(const std::string &path)
        : m_fd(::open(path.c_str(), O_RDWR | O_CLOEXEC))
    {
        // EBUSY means a kernel ddcci driver owns the bus and exposes its own backlight.
        if (m_fd >= 0 && ioctl(m_fd, I2C_SLAVE, DdcAddress) < 0) {
            ::close(m_fd);
            m_fd = -1;
        }
    }

    ~I2cDevice()
    {
        if (m_fd >= 0)
            ::close(m_fd);
    }

    bool isValid() const { return m_fd >= 0; }

    bool write(const std::string &payload)
    {
        std::string message;
        message += char(HostAddress);
        message += char(0x80 | payload.size());
        message += payload;
        // The checksum covers the destination address 0x6e, which the kernel sends.
        uint8_t checksum = 0x6e;
        for (char byte : message)
            checksum ^= uint8_t(byte);
        message += char(checksum);
        return ::write(m_fd, message.data(), message.size()) == ssize_t(message.size());
    }

    bool getVcp(uint8_t code, int *value, int *maximum)
    {
        if (!write(std::string {'\x01', char(code)}))
            return false;
        sleepFor(40);

        uint8_t reply[11];
        if (::read(m_fd, reply, sizeof(reply)) != sizeof(reply))
            return false;
        uint8_t checksum = 0x50;
        for (int i = 0; i < 10; ++i)
            checksum ^= reply[i];
        if (reply[0] != 0x6e || reply[1] != 0x88 || reply[2] != 0x02 || reply[3] != 0x00
            || reply[4] != code || checksum != reply[10])
            return false;

        *maximum = (reply[6] << 8) | reply[7];
        *value = (reply[8] << 8) | reply[9];
        return *maximum > 0;
    }

    bool setVcp(uint8_t code, int value)
    {
        const bool ok = write(std::string {'\x03', char(code), char((value >> 8) & 0xff), char(value & 0xff)});
        // DDC/CI requires a 50 ms gap before the next message.
        sleepFor(50);
        return ok;
    }

private:
    int m_fd;
};

} // namespace

DdcWorker::DdcWorker(Probed probed)
    : m_probed(std::move(probed))
    , m_loop(Lingmo::Base::EventLoop::current()->handle())
    , m_thread([this] { run(); })
{
}

DdcWorker::~DdcWorker()
{
    {
        std::lock_guard locker(m_mutex);
        m_stopping = true;
        m_tasks.clear();
    }
    m_wake.notify_all();
    m_thread.join();
}

void DdcWorker::run()
{
    for (;;) {
        std::function<void()> task;
        {
            std::unique_lock locker(m_mutex);
            m_wake.wait(locker, [this] { return m_stopping || !m_tasks.empty(); });
            if (m_stopping)
                return;
            task = std::move(m_tasks.front());
            m_tasks.pop_front();
        }
        task();
    }
}

void DdcWorker::post(std::function<void()> task)
{
    {
        std::lock_guard locker(m_mutex);
        m_tasks.push_back(std::move(task));
    }
    m_wake.notify_one();
}

void DdcWorker::requestValue(const std::string &device, int value)
{
    bool queue = false;
    {
        std::lock_guard locker(m_mutex);
        m_pending[device] = value;
        queue = !m_flushQueued;
        m_flushQueued = true;
    }
    if (queue)
        post([this] { flush(); });
}

void DdcWorker::flush()
{
    for (;;) {
        std::string device;
        int value = 0;
        {
            std::lock_guard locker(m_mutex);
            if (m_pending.empty() || m_stopping) {
                m_flushQueued = false;
                return;
            }
            auto it = m_pending.begin();
            device = it->first;
            value = it->second;
            m_pending.erase(it);
        }

        I2cDevice i2c(device);
        if (!i2c.isValid() || !i2c.setVcp(BrightnessCode, value))
            LM_WARNING(services::log()) << "DDC/CI brightness write failed on" << device;
    }
}

void DdcWorker::probe(uint64_t generation, std::vector<DdcDisplay> candidates)
{
    post([this, generation, candidates = std::move(candidates)]() mutable {
        std::vector<DdcDisplay> displays;
        for (DdcDisplay display : candidates) {
            I2cDevice i2c(display.device);
            if (!i2c.isValid())
                continue;
            for (int attempt = 0; attempt < Attempts; ++attempt) {
                if (i2c.getVcp(BrightnessCode, &display.value, &display.maximum)) {
                    displays.push_back(display);
                    break;
                }
                sleepFor(50);
            }
        }
        // Dropped when the worker is gone by the time the loop runs it.
        m_loop.post([this, alive = std::weak_ptr<bool>(m_alive), generation, displays = std::move(displays)] {
            if (alive.lock())
                m_probed(generation, displays);
        });
    });
}
