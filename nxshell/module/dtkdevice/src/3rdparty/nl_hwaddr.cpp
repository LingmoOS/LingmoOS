
#include "osutils.h"
#include "nl_hwaddr.h"
#include <net/if.h>
#include <unistd.h>
#include <linux/sockios.h>
#include <sys/ioctl.h>
#include <net/if_arp.h>
#include <QObject>

static int maclen(unsigned family = ARPHRD_ETHER)
{
    switch (family) {
    case ARPHRD_INFINIBAND:
        return 20;
    case ARPHRD_ETHER:
        return 6;
    default:
        return 14;
    }
}
static string getmac(const unsigned char *mac, unsigned family = ARPHRD_ETHER)
{
    char buff[5];
    string result = "";
    bool valid = false;

    for (int i = 0; i < maclen(family); i++) {
        snprintf(buff, sizeof(buff), "%02x", mac[i]);

        valid |= (mac[i] != 0);

        if (i == 0)
            result = string(buff);
        else
            result += ":" + string(buff);
    }

    if (valid)
        return result;
    else
        return "";
}

// string uppercase(const string &s)
// {
//     string result;

//     for (unsigned int i = 0; i < s.length(); i++)
//         result += static_cast<char>(toupper(s[i]));

//     return result;
// }

static bool isVirtual(const string &MAC)
{
    if (MAC.length() < 8)
        return false;

    string manufacturer = uppercase(MAC.substr(0, 8));

    if ((manufacturer == "00:05:69") ||
            (manufacturer == "00:0C:29") || (manufacturer == "00:50:56"))
        return true;    // VMware
    if (manufacturer == "00:1C:42")
        return true;    // Parallels
    if (manufacturer == "0A:00:27")
        return true;    // VirtualBox

    return false;
}

static QString hwname(int t)
{
    switch (t) {
    case ARPHRD_ETHER:
        return "Ethernet";
    case ARPHRD_SLIP:
        return "SLIP";
    case ARPHRD_LOOPBACK:
        return "loopback";
    case ARPHRD_FDDI:
        return "FDDI";
    case ARPHRD_IEEE1394:
        return "IEEE1394";
    case ARPHRD_IRDA:
        return "IRDA";
    case ARPHRD_PPP:
        return "PPP";
    case ARPHRD_X25:
        return "X25";
    case ARPHRD_TUNNEL:
        return "IPtunnel";
    case ARPHRD_DLCI:
        return "Framerelay.DLCI";
    case ARPHRD_FRAD:
        return "Framerelay.AD";
    case ARPHRD_TUNNEL6:
        return "IP6tunnel";
    case ARPHRD_SIT:
        return "IP6inIP4";
    default:
        return "";
    }
}

NLHWAddr::NLHWAddr(const QByteArray &ifname)
{
    m_conn_type = "";
    m_ifname = ifname;
    m_is_virtual = false;
    initData();
}


void NLHWAddr::initData()
{
    struct ifreq ifr;
    memset(&ifr, 0, sizeof(ifr));
    strcpy(ifr.ifr_name, m_ifname);
    int fd = socket(PF_INET, SOCK_DGRAM, 0);
    if (ioctl(fd, SIOCGIFHWADDR, &ifr) == 0) {
        string hwaddr = getmac(reinterpret_cast<unsigned char *>(ifr.ifr_hwaddr.sa_data), ifr.ifr_hwaddr.sa_family);
        m_sa_family = ifr.ifr_hwaddr.sa_family;
        if (m_sa_family >= 256) {
            m_is_virtual = true;
        }
        m_conn_type = hwname(m_sa_family).toUtf8();
        m_is_virtual = isVirtual(hwaddr);
    }
    close(fd);
}


