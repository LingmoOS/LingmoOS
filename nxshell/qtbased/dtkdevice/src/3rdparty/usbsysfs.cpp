
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif


#include "config.h"
#include "hw.h"
#include "sysfs.h"
#include "usbsysfs.h"
#include "osutils.h"
#include "heuristics.h"
#include "options.h"
#include <stdio.h>
#include <stdlib.h>
#include <map>
#include <vector>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <cstring>

#define SYSBUSUSBDEVICES "/sys/bus/usb/devices"
#define USBID_PATH DATADIR"/usb.ids:/usr/share/lshw/usb.ids:/usr/local/share/usb.ids:/usr/share/usb.ids:/etc/usb.ids:/usr/share/hwdata/usb.ids:/usr/share/misc/usb.ids"


// refence to https://www.usb.org/defined-class-codes
static map<u_int16_t, string> usbvendors;
static map<u_int32_t, string> usbproducts;


struct _baseclassDesc {
    const string  ClassID;
    const string  Usage;
    const string  Description;
};

struct _InterfaceDesc {
    const string  ClassID;
    const string  SubClassID;
    const string  ProtocolID;
    hw::hwClass hwClass;
    const string  specMeaning;
};

static struct _baseclassDesc baseclassDesc[] = {
//{"Base Class",  "Descriptor Usage",    "Description" },
    {"00",    "Device",         "Use class information in the Interface Descriptors" },
    {"01",    "Interface",      "Audio" },
    {"02",    "Both",           "Communications and CDC Control" },
    {"03",    "Interface",      "HID_Human Interface Device" },
    {"05",    "Interface",      "Physical" },
    {"06",    "Interface",      "Image" },
    {"07",    "Interface",      "Printer" },
    {"08",    "Interface",      "Mass Storage" },
    {"09",    "Device",         "Hub" },
    {"0A",    "Interface",      "CDC-Data" },
    {"0B",    "Interface",      "Smart Card" },
    {"0D",    "Interface",      "Content Security" },
    {"0E",    "Interface",      "Video" },
    {"0F",    "Interface",      "Personal Healthcare" },
    {"10",    "Interface",      "Audio/Video Devices" },
    {"11",    "Device",         "Billboard Device Class" },
    {"12",    "Interface",      "USB Type-C Bridge Class" },
    {"3C",    "Interface",      "I3C Device Class" },
    {"DC",    "Both",           "Diagnostic Device" },
    {"E0",    "Interface",      "Wireless Controller" },
    {"EF",    "Both",           "Miscellaneous" },
    {"FE",    "Interface",      "Application Specific" },
    {"FF",    "Both",           "Vendor Specific" },
};
//https://www.usb.org/defined-class-codes.
const static  struct _InterfaceDesc InterfaceDesc[] = {
//{"Base","Sub",    "Protocol",hw::generic,  "Meaning"},
    {"00",  "00",   "00",   hw::generic,    "Default and undefine device"},
    {"01",  "01",   "xx",   hw::input,    "Audio Control device"},
    {"01",  "02",   "xx",   hw::input,    "Audio streaming"},
    {"01",  "03",   "xx",   hw::input,    "Audio MIDI"},
    {"01",  "xx",   "xx",   hw::input,    "Audio device"},
    {"02",  "01",   "xx",   hw::communication,    "Communication device "},
    {"02",  "02",   "01",   hw::communication,    "AT (Hayes) compatible Modem Communication "},
    {"02",  "02",   "02",   hw::communication,    "AT (Hayes) compatible Modem Communication "},
    {"02",  "02",   "03",   hw::communication,    "AT (Hayes) compatible Modem Communication "},
    {"02",  "02",   "04",   hw::communication,    "AT (Hayes) compatible Modem Communication "},
    {"02",  "02",   "05",   hw::communication,    "AT (Hayes) compatible Modem Communication "},
    {"02",  "02",   "06",   hw::communication,    "AT (Hayes) compatible Modem Communication "},
    {"02",  "06",   "xx",   hw::communication,    "Communication Ethernet networking"},
    {"02",  "0B",   "xx",   hw::communication,    "Communication OBEX networking"},
    {"02",  "xx",   "xx",   hw::communication,    "Communication device "},
    {"03",  "00",   "01",   hw::keyboard,    "HID Keyboard"},
    {"03",  "01",   "01",   hw::keyboard,    "HID Keyboard"},
    {"03",  "00",   "02",   hw::mouse,      "HID Mouse"},
    {"03",  "01",   "02",   hw::mouse,      "HID Mouse"},
    {"03",  "xx",   "xx",   hw::input,    "HID device class"},
    {"05",  "xx",   "xx",   hw::input,    "Physical device class"},
    {"06",  "01",   "01",   hw::input,    "Still Imaging device"},
    {"07",  "01",   "01",   hw::printer,    "Printer Unidirectional"},
    {"07",  "01",   "02",   hw::printer,    "Printer Bidirectional"},
    {"07",  "01",   "03",   hw::printer,    "Printer IEEE 1284.4 compatible bidirectional"},
    {"07",  "xx",   "xx",   hw::printer,    "Printer device"},
    {"08",  "01",   "xx",   hw::storage,    "Mass Storage flash"},
    {"08",  "02",   "xx",   hw::storage,    "Mass Storage atapi"},
    {"08",  "04",   "xx",   hw::storage,    "Mass Storage floppy"},
    {"08",  "06",   "xx",   hw::storage,    "Mass Storage scsi"},
    {"08",  "xx",   "xx",   hw::storage,    "Mass Storage device"},
    {"09",  "00",   "00",   hw::usb_hub,    "Full speed Hub"},
    {"09",  "00",   "00",   hw::usb_hub,    "Hi-speed hub with single TT"},
    {"09",  "00",   "00",   hw::usb_hub,    "Hi-speed hub with multiple TTs"},
    {"0A",  "xx",   "xx",   hw::input,    "CDC data device"},
    {"0B",  "xx",   "xx",   hw::input,    "Smart Card device"},
    {"0D",  "00",   "00",   hw::input,    "Content Security device"},
    {"0E",  "xx",   "xx",   hw::media_camera,    "Video device,maybe camera"},
    {"0F",  "xx",   "xx",   hw::input,    "Personal Healthcare device"},
    {"10",  "01",   "00",   hw::input,    "Audio/Video Device – AVControl Interface"},
    {"10",  "02",   "00",   hw::input,    "Audio/Video Device – AVData Video Streaming Interface"},
    {"10",  "03",   "00",   hw::input,    "Audio/Video Device – AVData Audio Streaming Interface"},
    {"11",  "00",   "00",   hw::input,    "Billboard Device"},
    {"12",  "00",   "00",   hw::input,    "USB Type-C Bridge Device"},
    {"3C",  "00",   "00",   hw::input,    "I3C Device"},
    {"3C",  "01",   "01",   hw::input,    "USB2 Compliance Device."},
    {"3C",  "02",   "00",   hw::input,    "Debug Target vendor defined. Please see "},
    {"3C",  "02",   "01",   hw::input,    "GNU Remote Debug Command Set."},
    {"3C",  "03",   "00",   hw::input,    "Undefined"},
    {"3C",  "03",   "01",   hw::input,    "Vendor defined Trace protocol on DbC."},
    {"3C",  "04",   "00",   hw::input,    "Undefined"},
    {"3C",  "04",   "01",   hw::input,    "Vendor defined Dfx protocol on DbC."},
    {"3C",  "05",   "00",   hw::input,    "Vendor defined Trace protocol over General Purpose (GP) endpoint on DvC."},
    {"3C",  "05",   "01",   hw::input,    "GNU Protocol protocol over General Purpose (GP) endpoint on DvC."},
    {"3C",  "06",   "00",   hw::input,    "Undefined"},
    {"3C",  "06",   "01",   hw::input,    "Vendor defined Dfx protocol on DvC."},
    {"3C",  "07",   "00",   hw::input,    "Undefined"},
    {"3C",  "07",   "01",   hw::input,    "Vendor defined Trace protocol on DvC."},
    {"3C",  "08",   "00",   hw::input,    "Undefined"},
    {"E0",  "01",   "01",   hw::communication,    "Bluetooth Programming Interface."},
    {"E0",  "01",   "02",   hw::communication,    "UWB Radio Control Interface."},
    {"E0",  "01",   "03",   hw::communication,    "Remote NDIS."},
    {"E0",  "01",   "04",   hw::communication,    "Bluetooth AMP Controller."},
    {"E0",  "02",   "01",   hw::communication,    "Host Wire Adapter Control/Data interface."},
    {"E0",  "02",   "02",   hw::communication,    "Device Wire Adapter Control/Data interface."},
    {"E0",  "02",   "03",   hw::communication,    "Device Wire Adapter Isochronous interface."},
    {"EF",  "01",   "01",   hw::input,    "Active Sync device."},
    {"EF",  "01",   "02",   hw::input,    "Palm Sync. T"},
    {"EF",  "02",   "01",   hw::input,    "Interface Association Descriptor."},
    {"EF",  "02",   "02",   hw::input,    "Wire Adapter Multifunction Peripheral programming interface."},
    {"EF",  "03",   "01",   hw::input,    "Cable Based Association Framework."},
    {"EF",  "04",   "01",   hw::input,    "RNDIS over Ethernet.Ethernet gateway device."},
    {"EF",  "04",   "02",   hw::input,    "RNDIS over WiFi."},
    {"EF",  "04",   "03",   hw::input,    "RNDIS over WiMAX"},
    {"EF",  "04",   "04",   hw::input,    "RNDIS over WWAN"},
    {"EF",  "04",   "05",   hw::input,    "RNDIS for Raw IPv4"},
    {"EF",  "04",   "06",   hw::input,    "RNDIS for Raw IPv6"},
    {"EF",  "04",   "07",   hw::input,    "RNDIS for GPRS"},
    {"EF",  "05",   "00",   hw::input,    "USB3 Vision Control Interface"},
    {"EF",  "05",   "01",   hw::input,    "USB3 Vision Event Interface"},
    {"EF",  "05",   "02",   hw::input,    "USB3 Vision Streaming Interface"},
    {"EF",  "06",   "01",   hw::input,    "STEP. Stream Transport Efficient Protocol for content protection."},
    {"EF",  "06",   "02",   hw::input,    "STEP RAW. Stream Transport Efficient Protocol for Raw content protection."},
    {"EF",  "07",   "01",   hw::input,    "Command Interface in IAD"},
    {"FE",  "01",   "01",   hw::input,    "Device Firmware Upgrade."},
    {"FE",  "02",   "00",   hw::input,    "IRDA Bridge device."},
    {"FE",  "03",   "00",   hw::input,    "USB Test and Measurement Device by usb.org ."},
    {"FE",  "03",   "01",   hw::input,    "USB Test and Measurement Device by USBTMC USB488 Subclass"},
    {"FF",  "xx",   "xx",   hw::generic,    "USB Vendor specific"},
};

#define ARRAY_LEN(a) ((sizeof(a)) / (sizeof(a[0])))

// static map < string, struct _baseclassDesc > baseclassDesc;
// static map < string, struct _InterfaceDesc > InterfaceDescriptors;

char setBaseClass(hwNode &device,  string ClassID)
{
    for (int i = 0; i < ARRAY_LEN(baseclassDesc); i++) {
        if (ClassID == baseclassDesc[i].ClassID) {
            return i;
        }
    }
    return  -1;
}

static string setUSBClass(hwNode &device, string cls, string sub, string prot)
{
    string cls1 = hw::strip(cls);
    string sub1 = hw::strip(sub);
    string prot1 = hw::strip(prot);
    for (int i = 0; i < ARRAY_LEN(InterfaceDesc); i++) {
        if (0 == strcasecmp(cls1.c_str(), InterfaceDesc[i].ClassID.c_str())) {
            if ((0 == strcasecmp(sub1.c_str(), InterfaceDesc[i].SubClassID.c_str())) || "xx" == InterfaceDesc[i].SubClassID) {
                if ((0 == strcasecmp(prot1.c_str(), InterfaceDesc[i].ProtocolID.c_str())) || "xx" == InterfaceDesc[i].ProtocolID) {
                    string InterfaceClass_0 = device.getConfig("InterfaceClass_0");
                    if (InterfaceClass_0.empty()) {
                        device.setClass(InterfaceDesc[i].hwClass);
                        device.setDescription(InterfaceDesc[i].specMeaning);
                    }
                    return InterfaceDesc[i].specMeaning;
                }
            }
        }
    }
    return string();
}



#define PRODID(x, y) ((x << 16) + y)

static string usbhandle(unsigned bus, unsigned level, unsigned dev)
{
    char buffer[10];

    snprintf(buffer, sizeof(buffer), "USB:%u:%u", bus, dev);

    return string(buffer);
}


static string usbbusinfo(string path)
{
    return string("usb@" + path);
}




static bool describeUSB(hwNode &device, unsigned vendor, unsigned prodid)
{
    if (usbvendors.find(vendor) == usbvendors.end()) return false;

    device.setVendor_name(usbvendors[vendor] + (enabled("output:numeric") ? " [" + to4hex(vendor) + "]" : ""));
    device.addHint("usb.idVendor", vendor);
    device.addHint("usb.idProduct", prodid);

    if (usbproducts.find(PRODID(vendor, prodid)) != usbproducts.end())
        device.setProduct_name(usbproducts[PRODID(vendor, prodid)] + (enabled("output:numeric") ? " [" + to4hex(vendor) + ":" + to4hex(prodid) + "]" : ""));

    return true;
}


static bool load_usbids(const string &name)
{
    FILE *usbids = NULL;
    u_int16_t vendorid = 0;

    usbids = fopen(name.c_str(), "r");
    if (!usbids) return false;

    while (!feof(usbids)) {
        char *buffer = NULL;
        size_t linelen;
        unsigned t = 0;
        char *description = NULL;

        if (getline(&buffer, &linelen, usbids) > 0) {
            if (buffer[linelen - 1] < ' ')
                buffer[linelen - 1] = '\0';               // chop \n
            string line = string(buffer);
            free(buffer);
            buffer = NULL;

            description = NULL;
            t = 0;
            if (line.length() > 0) {
                if (line[0] == '\t') {                    // product id entry
                    line.erase(0, 1);
                    if (matches(line, "^[[:xdigit:]][[:xdigit:]][[:xdigit:]][[:xdigit:]]"))
                        t = strtol(line.c_str(), &description, 16);
                    if (description && (description != line.c_str())) {
                        usbproducts[PRODID(vendorid, t)] = hw::strip(description);
                    }
                } else {                                  // vendor id entry
                    if (matches(line, "^[[:xdigit:]][[:xdigit:]][[:xdigit:]][[:xdigit:]]"))
                        t = strtol(line.c_str(), &description, 16);
                    if (description && (description != line.c_str())) {
                        vendorid = t;
                        usbvendors[t] = hw::strip(description);
                    }
                }
            }
        }
        if (buffer != NULL) free(buffer);
    }

    fclose(usbids);

    return true;
}

/* //hub
T是拓扑结构，Lev就是该设备位于拓扑结构中的层次level。
Prnt是父设备的设备号(root hub没有父设备，这里固定用0)。
Port是设备所挂接hub的端口。对于root hub，Port值固定用0，
对于其余设备，Port值用物理端口号减1(物理端口号一般是从1开始计数，减1之后就是从0开始)。
Cnt的一般说法是这个level上设备的总数，但这其实是不严谨的。对于root hub，Cnt的值固定为0，对于其余设备，
Cnt值更像是一个同level内不同设备的序号(从1开始)，因此只有同level内最后一个设备的Cnt值，才可以看做是该level上设备的总数。Dev是设备号，Spd是速度，MxCh是支持的最多子设备数(即hub设备包含的端口数，非hub设备这个值为0)。
*/

void set_usb_interface(hwNode &device, const sysfs::entry &ee)
{
    string sysfs_path_file = ee.sysfs_path();
    string bInterfaceSubClass = get_string(sysfs_path_file + "/bInterfaceSubClass");
    string bInterfaceClass = get_string(sysfs_path_file + "/bInterfaceClass");
    string bInterfaceProtocol = get_string(sysfs_path_file + "/bInterfaceProtocol");

    // string InterfaceClass_0 = device.getConfig("InterfaceClass_0");
    // if(InterfaceClass_0.empty())

    string USBClass = setUSBClass(device, bInterfaceClass, bInterfaceSubClass, bInterfaceProtocol);
    if (!USBClass.empty())
        device.setConfig("InterfaceClass_0", get_string(sysfs_path_file + "/bInterfaceClass"));
    string curpath_file = ee.name();
    vector < string > vecstring;
    splitlines(curpath_file, vecstring, ':');
    string interace_number = vecstring[vecstring.size() - 1];
    device.setConfig(string("IFdir_" + interace_number), curpath_file);
    // device.setConfig("IF_AlternateSetting"+interace_number, get_string(sysfs_path_file + "/bAlternateSetting"));
    // device.setConfig("IF_NumEndpoints"+interace_number, get_string(sysfs_path_file + "/bNumEndpoints"));
    // device.setConfig("IF_InterfaceNumber"+interace_number, get_string(sysfs_path_file + "/bInterfaceNumber"));
    device.setConfig("IF_Num:AltSet:NumEP" + interace_number, get_string(sysfs_path_file + "/bInterfaceNumber") + ':' \
                     + get_string(sysfs_path_file + "/bAlternateSetting") + ':' + get_string(sysfs_path_file + "/bNumEndpoints"));

    device.setConfig("IF_Class:sub:Prot" + interace_number, bInterfaceClass + ':' + bInterfaceSubClass + ':' + bInterfaceProtocol);
    device.setModalias(ee.modalias());
    string driver = ee.driver();
    if (!driver.empty())
        device.setConfig("driver", driver);

    if ((bInterfaceClass == "ff\n") && (device.getClass() == hw::generic)) {
        device.setClass(hw::input);
        device.setDescription(string("Vendor Spec:") + device.getVendor_name() + device.getVendor_name());
    }
}

void set_usb_device(hwNode &device, const sysfs::entry &ee)
{
    FILE *usbdevices = NULL;
    //return ;

    unsigned int bus, lev, prnt, port;
    float speed;

    unsigned int vendor, prodid;

    device.claim();
    string sysfs_path_file = ee.sysfs_path();
    // device.setLogicalName(xxxe);
    string speedstr = get_string(sysfs_path_file + "/speed");
    if (!speedstr.empty())
        device.setConfig("speed", string(speedstr + " Mbit/s"));
    string vid = get_string(sysfs_path_file + "/idVendor");
    string pid = get_string(sysfs_path_file + "/idProduct");
    device.setVendor_id(vid);
    device.setProduct_id(pid);
    device.setConfig("vid:pid", vid + ':' + pid);
    device.setVendor(get_string(sysfs_path_file + "/manufacturer"));
    device.setProduct(get_string(sysfs_path_file + "/product"));
    device.setHandle("usb" + get_string(sysfs_path_file + "/busnum") + "_" + get_string(sysfs_path_file + "/devnum"));

    vendor = strtoull(vid.c_str(), NULL, 16);
    prodid = strtoull(pid.c_str(), NULL, 16);
    describeUSB(device, vendor, prodid);
    string firmware_node;
//  if (exists(string(sysfs_path_file + "/firmware_node"))){
//       firmware_node = readlink(string(sysfs_path_file + "/firmware_node");
////    device.setConfig("firmware_node", shortname(firmware_node));
//  }
    device.setVersion(get_string(sysfs_path_file + "/version"));
    device.setConfig("maxchild", get_string(sysfs_path_file + "/maxchild"));

    device.setConfig("rx_lanes", get_string(sysfs_path_file + "/rx_lanes"));
    device.setConfig("tx_lanes", get_string(sysfs_path_file + "/tx_lanes"));
    device.setConfig("bConfigurationValue", get_string(sysfs_path_file + "/bConfigurationValue"));
    device.setConfig("devnum", get_string(sysfs_path_file + "/devnum"));
    device.setConfig("busnum", get_string(sysfs_path_file + "/busnum"));
    device.setConfig("bNumInterfaces", get_string(sysfs_path_file + "/bNumInterfaces"));
    device.setConfig("serial", get_string(sysfs_path_file + "/serial"));
    device.setConfig("bNumConfigurations", get_string(sysfs_path_file + "/bNumConfigurations"));
    device.setConfig("bmAttributes", get_string(sysfs_path_file + "/bmAttributes"));
    device.setConfig("bMaxPacketSize0", get_string(sysfs_path_file + "/bMaxPacketSize0"));
    device.setConfig("configuration", get_string(sysfs_path_file + "/configuration"));


    string bDeviceClass = get_string(sysfs_path_file + "/bDeviceClass");
    string bDeviceProtocol = get_string(sysfs_path_file + "/bDeviceProtocol");
    string bcdDevice = get_string(sysfs_path_file + "/bcdDevice");
    device.setConfig("Dev_Class:Prot:bcd", bDeviceClass + ':' + bDeviceProtocol + ':' + bcdDevice);

    string bMaxPower = get_string(sysfs_path_file + "/bMaxPower");
    device.setConfig("maxpower", bMaxPower);

    device.setHandle(sysfs_path_file);
    device.setModalias(ee.modalias());
    device.setSysFS_Path(sysfs_path_file);
    device.setVendor(ee.vendor());
    device.setProduct(ee.device());

    string driver = ee.driver();
    if (!driver.empty())
        device.setConfig("bus_driver", driver);
}

bool scan_usbsysfs(hwNode &nn)
{
    vector < string > filenames;
    splitlines(USBID_PATH, filenames, ':');
    for (int i = filenames.size() - 1; i >= 0; i--) {
        load_usbids(filenames[i]);
    }
    filenames.clear();

    vector < sysfs::entry > entries = sysfs::entries_by_bus("usb");
    if (entries.empty())
        return false;

    for (int i = 0; i < entries.size(); i++)  {
        const sysfs::entry &ee = entries[i];
        hwNode device("device");     //
        string businfo = ""; //floder
        string curpath_file = ee.name();
        string parent_path = ee.parent().name();
        string sysfs_path = ee.sysfs_path();

        if (matches(curpath_file, "usb+[[:digit:]]+$")) {
            // device = hwNode("usbhost", hw::bus);
            device = hwNode("usbhost");
            device.setPhysId(string("hub_" + curpath_file));
            device.setBusInfo(usbbusinfo(curpath_file));
            set_usb_device(device, ee);
            nn.addChild(device);
        }

    }//end for

    for (int i = 0; i < entries.size(); i++)  {
        const sysfs::entry &ee = entries[i];
        hwNode device("device");     //
        string businfo = ""; //floder
        string curpath_file = ee.name();
        string parent_path = ee.parent().name();
        string sysfs_path = ee.sysfs_path();

        if (matches(curpath_file, "^[0-9]+-[0-9]{1,}$") || matches(curpath_file, "^[0-9]+-[0-9]+\.[0-9]$")) { //port  1-10  1-10/1-10.4
            device = hwNode("usb");
            if (exists(string(sysfs_path) + "/port")) {
                string portname = shortname(readlink(string(sysfs_path) + "/port"));
                device.setConfig("port", portname);
                device.setPhysId(portname);
                set_usb_device(device, ee);
            }
            device.setBusInfo(usbbusinfo(curpath_file));
            hwNode *parent = NULL;
            parent = nn.findChildByBusInfo(usbbusinfo(parent_path));
            if (parent) {
                parent->addChild(device);
            } else
                nn.addChild(device);
        }

    }//end for

    // for (vector < sysfs::entry >::iterator it = entries.begin();  \
    //         it != entries.end(); ++it)  {
    for (int i = 0; i < entries.size(); i++)  {
        const sysfs::entry &ee = entries[i];
        hwNode device("interface");     //
        string businfo = ""; //floder
        string curpath_file = ee.name();
        string parent_path = ee.parent().name();
        string sysfs_path = ee.sysfs_path();

        if (strchr(curpath_file.c_str(), ':')) {
            hwNode *parent  = nn.findChildByBusInfo(usbbusinfo(parent_path));
            if (parent) {
                set_usb_interface(*parent, ee);
            }
        }
    }//end for
}


/*
static struct _baseclassDesc baseclassDesc[] =
{
//{"Base Class",  "Descriptor Usage",    "Description" },
    {"00",    "Device",         "Use class information in the Interface Descriptors" },
    {"01",    "Interface",      "Audio" },
    {"02",    "Both",           "Communications and CDC Control" },
    {"03",    "Interface",      "HID (Human Interface Device)" },
    {"05",    "Interface",      "Physical" },
    {"06",    "Interface",      "Image" },
    {"07",    "Interface",      "Printer" },
    {"08",    "Interface",      "Mass Storage" },
    {"09",    "Device",         "Hub" },
    {"0A",    "Interface",      "CDC-Data" },
    {"0B",    "Interface",      "Smart Card" },
    {"0D",    "Interface",      "Content Security" },
    {"0E",    "Interface",      "Video" },
    {"0F",    "Interface",      "Personal Healthcare" },
    {"10",    "Interface",      "Audio/Video Devices" },
    {"11",    "Device",         "Billboard Device Class" },
    {"12",    "Interface",      "USB Type-C Bridge Class" },
    {"3C",    "Interface",      "I3C Device Class" },
    {"DC",    "Both",           "Diagnostic Device" },
    {"E0",    "Interface",      "Wireless Controller" },
    {"EF",    "Both",           "Miscellaneous" },
    {"FE",    "Interface",      "Application Specific" },
    {"FF",    "Both",           "Vendor Specific" },
};
*/
