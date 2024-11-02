/*
 * liblingmosdk-system's Library
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Yunhe Liu <liuyunhe@kylinos.cn>
 *
 */

#include "libkyhw.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <dlfcn.h>
#include <stdlib.h>
#include <fcntl.h>

#include "hd.h"
#include "glib.h"
#include "pci/pci.h"
#include "libkylog.h"
#include "dbus/dbus-glib.h"

#define SERVERNAME "org.freedesktop.UPower"
#define PROPERTIES_FACE "org.freedesktop.DBus.Properties"
#define DEVICEINTERFACE "org.freedesktop.UPower.Device"

struct device
{
  struct device *next;
  struct pci_dev *dev;
  /* Bus topology calculated by grow_tree() */
  struct device *bus_next;
  struct bus *parent_bus;
  struct bridge *bridge;
  /* Cache */
  unsigned int config_cached, config_bufsize;
  unsigned char *config;  /* Cached configuration space data */
  unsigned char *present; /* Maps which configuration bytes are present */
};

struct bridge
{
  struct bridge *chain;        /* Single-linked list of bridges */
  struct bridge *next, *child; /* Tree of bridges */
  struct bus *first_bus;       /* List of buses connected to this bridge */
  unsigned int domain;
  unsigned int primary, secondary, subordinate; /* Bus numbers */
  struct device *br_dev;
};

struct bus
{
  unsigned int domain;
  unsigned int number;
  struct bus *sibling;
  struct bridge *parent_bridge;
  struct device *first_dev, **last_dev;
};

struct hw_pci_dev
{
  u_int16_t domain;   /* PCI domain (host bridge) */
  u_int16_t bus;      /* Higher byte can select host bridges */
  u_int8_t dev, func; /* Device and function */

  u_int16_t vendor_id, device_id; /* Identity of the device */
  unsigned int irq;               /* IRQ number */
  pciaddr_t base_addr[6];         /* Base addresses */
  pciaddr_t size[6];              /* Region sizes */
  pciaddr_t rom_base_addr;        /* Expansion ROM base address */
  pciaddr_t rom_size;             /* Expansion ROM size */

  u_int8_t config[256]; /* non-root users can only use first 64 bytes */
};

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 *
 *                      lingmosdk interface functions
 *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 */

bool scan_pci_legacy(struct hw_pci_dev *d, hd_t *hd)
{
  FILE *f;

  f = fopen("/proc/bus/pci/devices", "r");
  if (f)
  {
    char buf[512];

    while (fgets(buf, sizeof(buf) - 1, f))
    {
      unsigned int dfn, vend, cnt;
      char driver[50];
      memset(driver, 0, sizeof(driver));
      cnt = sscanf(buf,
                   "%x %x %x %lx %lx %lx %lx %lx %lx %lx %lx %lx %lx %lx %lx %lx %lx %[ -z]s",
                   &dfn,
                   &vend,
                   &d->irq,
                   &d->base_addr[0],
                   &d->base_addr[1],
                   &d->base_addr[2],
                   &d->base_addr[3],
                   &d->base_addr[4],
                   &d->base_addr[5],
                   &d->rom_base_addr,
                   &d->size[0],
                   &d->size[1],
                   &d->size[2],
                   &d->size[3], &d->size[4], &d->size[5], &d->rom_size, driver);

      if (cnt != 9 && cnt != 10 && cnt != 17 && cnt != 18)
        break;

      d->bus = dfn >> 8;
      d->dev = PCI_SLOT(dfn & 0xff);
      d->func = PCI_FUNC(dfn & 0xff);
      d->vendor_id = vend >> 16;
      d->device_id = vend & 0xffff;

      if ((d->vendor_id != (uint16_t)(hd->vendor.id)) || (d->device_id != (uint16_t)(hd->device.id)))
        continue;

      int fd = -1;
      char device_path[512];
      sprintf(device_path, "/proc/bus/pci/%02x/%02x.%x", d->bus, d->dev, d->func);
      fd = open(device_path, O_RDONLY);
      if (fd >= 0)
      {
        if (-1 == read(fd, d->config, sizeof(d->config)))
          memset(d->config, 0, sizeof(d->config));
        close(fd);
      }
      break;
    }
    fclose(f);
  }
  return true;
}

struct HWInfo *kdk_hw_get_hwinfo(int type)
{
  struct HWInfo *result = NULL;

  hd_data_t *hd_data;
  hd_data = (hd_data_t *)calloc(1, sizeof *hd_data);
  if (!hd_data)
  {
    klog_err("%s 申请内存失败于%s", "kdk_hw_get_sound", "hd_data");
    return result;
  }

  hd_data->progress = NULL;
  hd_data->debug = ~(HD_DEB_DRIVER_INFO | HD_DEB_HDDB);

  hd_t *hd, *hd0;
  hd0 = hd_list(hd_data, type, 1, NULL);
  for (hd = hd0; hd; hd = hd->next)
  {

    int isCamera = 0;
    if (hd->model && strstr(hd->model, "camera"))
      isCamera = 1;
    else if (hd->model && strstr(hd->model, "Camera"))
      isCamera = 1;
    else if (hd->model && strstr(hd->model, "webcam"))
      isCamera = 1;
    else if (hd->device.name && strstr(hd->device.name, "camera"))
      isCamera = 1;
    else if (hd->device.name && strstr(hd->device.name, "Camera"))
      isCamera = 1;
    else if (hd->driver && strstr(hd->driver, "uvcvideo"))
      isCamera = 1;

    if ((hw_usb == type) && !isCamera)
    {
      continue;
    }

    struct HWInfo *node = (struct HWInfo *)calloc(1, sizeof *node);
    if (!node)
    {
      klog_err("%s 申请内存失败于%s", "kdk_hw_get_sound", "struct HWInfo");
      kdk_hw_free_hw(result);
      result = NULL;
      goto out;
    }

    strcpy(node->model, hd->model ? hd->model : "none");
    strcpy(node->vendor, hd->vendor.name ? hd->vendor.name : "none");
    strcpy(node->device, hd->device.name ? hd->device.name : "none");
    strcpy(node->driver, hd->driver ? hd->driver : "none");
    strcpy(node->revision, hd->revision.name ? hd->revision.name : "none");
    if (0 == strcmp(node->revision, "none"))
      sprintf(node->revision, "0x%x", hd->revision.id);
    strcpy(node->busid, hd->bus.name ? hd->bus.name : "none");

    if (hw_sound == type)
    {
      struct hw_pci_dev d;
      scan_pci_legacy(&d, hd);
      u_int16_t status = d.config[PCI_STATUS] | (d.config[PCI_STATUS + 1] << 8);
      if (status & PCI_STATUS_66MHZ)
        strcpy(node->clock, "66 MHZ"); // 66MHz
      else
        strcpy(node->clock, "33 MHZ"); // 33MHz

      strcpy(node->width, "32 bits");
      for (int i = 0; i < 6; i++)
      {
        int p = PCI_BASE_ADDRESS_0 + 4 * i;

        u_int32_t flg = d.config[p] | (d.config[p + 1] << 8) |
                        (d.config[p + 2] << 16) | (d.config[p + 3] << 24);
        u_int32_t pos = d.base_addr[i];
        u_int32_t len = d.size[i];

        if (flg == 0xffffffff)
          flg = 0;

        if (!pos && !flg && !len)
          continue;

        if (pos && !flg) /* Reported by the OS, but not by the device */
        {
          flg = pos;
        }

        int t = flg & PCI_BASE_ADDRESS_MEM_TYPE_MASK;
        if (t == PCI_BASE_ADDRESS_MEM_TYPE_64)
        {
          strcpy(node->width, "64 bits");
          break;
        }
      }
    }
    else
    {
      strcpy(node->width, "none");
      strcpy(node->clock, "none");
    }

    hd_dev_num_t *d = &hd->unix_dev_num;
    if (d->type)
    {
      sprintf(node->devicenum, "%s %u:%u",
              d->type == 'b' ? "block" : "char",
              d->major, d->minor);
      if (d->range > 1)
      {
        sprintf(node->devicenum, "-%u:%u",
                d->major, d->minor + d->range - 1);
      }
    }
    else
    {
      sprintf(node->devicenum, "%s", "none");
    }
    node->next = result;
    result = node;
  }
out:
  hd_free_hd_data(hd_data);
  free(hd_data);
  hd_free_hd_list(hd0);
  return result;
}

void kdk_hw_free_hw(struct HWInfo *list)
{
  struct HWInfo *tmp = NULL;
  while (list)
  {
    tmp = list;
    list = list->next;
    free(tmp);
  }
}

static void _write_info(struct power_device *node, GHashTable *info_hash)
{
  GValue *attr = (GValue *)g_hash_table_lookup(info_hash, "NativePath");
  strcpy(node->native_path, attr ? g_value_get_string(attr) : "None");

  attr = (GValue *)g_hash_table_lookup(info_hash, "PowerSupply");
  node->power_supply = attr ? g_value_get_boolean(attr) : false;

  struct tm *time_tm;
  attr = (GValue *)g_hash_table_lookup(info_hash, "UpdateTime");
  time_t t = (time_t)g_value_get_uint64(attr);
  time_tm = localtime(&t);
  strftime(node->updated, sizeof node->updated, "%c", time_tm);

  attr = (GValue *)g_hash_table_lookup(info_hash, "HasHistory");
  node->has_history = attr ? g_value_get_boolean(attr) : false;

  attr = (GValue *)g_hash_table_lookup(info_hash, "HasStatistics");
  node->has_statistics = attr ? g_value_get_boolean(attr) : false;

  attr = (GValue *)g_hash_table_lookup(info_hash, "IsPresent");
  node->is_persent = attr ? g_value_get_boolean(attr) : false;

  attr = (GValue *)g_hash_table_lookup(info_hash, "IsRechargeable");
  node->is_rechargeable = attr ? g_value_get_boolean(attr) : false;

  attr = (GValue *)g_hash_table_lookup(info_hash, "State");
  switch (g_value_get_uint(attr))
  {
  case 1:
    strcpy(node->state, "charging");
    break;
  case 2:
    strcpy(node->state, "discharging");
    break;
  case 3:
    strcpy(node->state, "empty");
    break;
  case 4:
    strcpy(node->state, "fully-charged");
    break;
  case 5:
    strcpy(node->state, "pending-charge");
    break;
  case 6:
    strcpy(node->state, "pending-discharge");
    break;
  default:
    strcpy(node->state, "unknown");
    break;
  }

  attr = (GValue *)g_hash_table_lookup(info_hash, "WarningLevel");
  switch (g_value_get_uint(attr))
  {
  case 1:
    strcpy(node->warning_level, "none");
    break;
  case 2:
    strcpy(node->warning_level, "discharging");
    break;
  case 3:
    strcpy(node->warning_level, "low");
    break;
  case 4:
    strcpy(node->warning_level, "critical");
    break;
  case 5:
    strcpy(node->warning_level, "action");
    break;
  case 6:
    strcpy(node->warning_level, "normal");
    break;
  case 7:
    strcpy(node->warning_level, "high");
    break;
  case 8:
    strcpy(node->warning_level, "full");
    break;
  default:
    strcpy(node->warning_level, "unknown");
    break;
  }

  attr = (GValue *)g_hash_table_lookup(info_hash, "Energy");
  node->energy = attr ? g_value_get_double(attr) : 0.0f;

  attr = (GValue *)g_hash_table_lookup(info_hash, "EnergyEmpty");
  node->energy_empty = attr ? g_value_get_double(attr) : 0.0f;

  attr = (GValue *)g_hash_table_lookup(info_hash, "EnergyFull");
  node->energy_full = attr ? g_value_get_double(attr) : 0.0f;

  attr = (GValue *)g_hash_table_lookup(info_hash, "EnergyFullDesign");
  node->energy_full_design = attr ? g_value_get_double(attr) : 0.0f;

  attr = (GValue *)g_hash_table_lookup(info_hash, "EnergyRate");
  node->energy_rate = attr ? g_value_get_double(attr) : 0.0f;

  attr = (GValue *)g_hash_table_lookup(info_hash, "Voltage");
  node->voltage = attr ? g_value_get_double(attr) : 0.0f;

  attr = (GValue *)g_hash_table_lookup(info_hash, "TimeToEmpty");
  node->time_to_empty = attr ? g_value_get_int64(attr) : 0.0f;

  attr = (GValue *)g_hash_table_lookup(info_hash, "TimeToFull");
  node->time_to_full = attr ? g_value_get_int64(attr) : 0.0f;

  attr = (GValue *)g_hash_table_lookup(info_hash, "Percentage");
  node->percentage = attr ? g_value_get_double(attr) : 0.0f;

  attr = (GValue *)g_hash_table_lookup(info_hash, "Temperature");
  node->temperature = attr ? g_value_get_double(attr) : 0.0f;

  attr = (GValue *)g_hash_table_lookup(info_hash, "Capacity");
  node->capacity = attr ? g_value_get_double(attr) : 0.0f;

  attr = (GValue *)g_hash_table_lookup(info_hash, "Technology");
  switch (g_value_get_uint(attr))
  {
  case 1:
    strcpy(node->technology, "lithium-ion");
    break;
  case 2:
    strcpy(node->technology, "lithium-polymer");
    break;
  case 3:
    strcpy(node->technology, "lithium-iron-phosphate");
    break;
  case 4:
    strcpy(node->technology, "lead-acid");
    break;
  case 5:
    strcpy(node->technology, "nickel-cadmium");
    break;
  case 6:
    strcpy(node->technology, "nickel-metal-hydride");
    break;
  default:
    strcpy(node->technology, "unknown");
    break;
  }

  attr = (GValue *)g_hash_table_lookup(info_hash, "Online");
  node->online = attr ? g_value_get_boolean(attr) : false;

  attr = (GValue *)g_hash_table_lookup(info_hash, "IconName");
  strcpy(node->icon_name, attr ? g_value_get_string(attr) : "None");

  attr = (GValue *)g_hash_table_lookup(info_hash, "Model");
  strcpy(node->model, attr ? g_value_get_string(attr) : "None");

  attr = (GValue *)g_hash_table_lookup(info_hash, "BatteryLevel");
  node->battery_level = g_value_get_uint(attr);

  attr = (GValue *)g_hash_table_lookup(info_hash, "Type");
  node->type = g_value_get_uint(attr);
}

static void _daemon_forech(gpointer key, gpointer value, gpointer user_data)
{
  char *tmp = (char *)key;
  GValue *data = (GValue *)value;
  struct Power *result = (struct Power *)user_data;
  if (0 == strcmp(tmp, "DaemonVersion"))
    strcpy(result->daemon_version, g_value_get_string(data));
  if (0 == strcmp(tmp, "LidIsClosed"))
    result->lid_is_closed = g_value_get_boolean(data);
  if (0 == strcmp(tmp, "LidIsPresent"))
    result->lid_is_present = g_value_get_boolean(data);
  if (0 == strcmp(tmp, "OnBattery"))
    result->on_battery = g_value_get_boolean(data);
}

static bool get_daemon_info(DBusGConnection *bus, GError **error, struct Power *result, DBusGProxy **upower_proxy)
{
  bool success = true;
  char *critical_action = NULL;
  DBusGProxy *properties_porxy = NULL;
  GHashTable *info_hash = NULL;

  do
  {
    properties_porxy = dbus_g_proxy_new_for_name(bus,
                                                 SERVERNAME,                // server
                                                 "/org/freedesktop/UPower", // path
                                                 PROPERTIES_FACE);          // interface

    if (!dbus_g_proxy_call(properties_porxy, "GetAll", error,
                           G_TYPE_STRING, SERVERNAME,
                           G_TYPE_INVALID,
                           dbus_g_type_get_map("GHashTable", G_TYPE_STRING, G_TYPE_VALUE), &info_hash,
                           G_TYPE_INVALID))
    {
      klog_err("Failed to call GetAll in path %s : %s\n", "/org/freedesktop/UPower", (*error)->message);
      success = false;
      break;
    }

    // 轮询哈希表
    g_hash_table_foreach(info_hash, _daemon_forech, result);

    // 创建UPower代理
    *upower_proxy = dbus_g_proxy_new_for_name(bus,
                                              SERVERNAME,                // server
                                              "/org/freedesktop/UPower", // path
                                              SERVERNAME);               // interface
    // 获取CriticalAction
    if (!dbus_g_proxy_call(*upower_proxy, "GetCriticalAction", error,
                           G_TYPE_INVALID,
                           G_TYPE_STRING, &critical_action,
                           G_TYPE_INVALID))
    {
      klog_err("Failed to call GetCriticalAction in path %s : %s\n", "/org/freedesktop/UPower", (*error)->message);
      success = false;
      break;
    }
    strcpy(result->critical_action, critical_action);
  } while (0);

  if (properties_porxy)
    g_object_unref((properties_porxy));
  if (info_hash)
    g_hash_table_unref(info_hash);
  if (critical_action)
    free(critical_action);

  return success;
}

static bool get_display_info(DBusGConnection *bus, DBusGProxy *upower_proxy, GError **error, struct Power *result)
{
  bool success = true;
  DBusGProxy *display_proxy = NULL;
  GHashTable *info_hash = NULL;
  char *display_path = NULL;

  // 获取displaydevice
  do
  {
    if (!dbus_g_proxy_call(upower_proxy, "GetDisplayDevice", error,
                           G_TYPE_INVALID,
                           DBUS_TYPE_G_OBJECT_PATH, &display_path,
                           G_TYPE_INVALID))
    {
      klog_err("Failed to call GetDisplayDevice in path %s : %s\n",
               "/org/freedesktop/UPower",
               (*error)->message);
      success = false;
      break;
    }

    display_proxy = dbus_g_proxy_new_for_name(bus,
                                              SERVERNAME,       // server
                                              display_path,     // path
                                              PROPERTIES_FACE); // interface
    if (!dbus_g_proxy_call(display_proxy, "GetAll", error,
                           G_TYPE_STRING, DEVICEINTERFACE,
                           G_TYPE_INVALID,
                           dbus_g_type_get_map("GHashTable", G_TYPE_STRING, G_TYPE_VALUE), &info_hash,
                           G_TYPE_INVALID))
    {
      klog_err("Failed to call GetAll in path %s : %s\n", display_path, (*error)->message);
      success = false;
      break;
    }
    struct power_device *node = (struct power_device *)calloc(1, sizeof *node);
    if (!node)
    {
      klog_err("Failed to request memory %s\n", display_path);
      success = false;
      break;
    }

    // 写入display device数据
    result->devices = node;
    strcpy(node->name, display_path);
    _write_info(node, info_hash);

  } while (0);

  if (display_proxy)
    g_object_unref(display_proxy);
  if (display_path)
    free(display_path);
  if (info_hash)
    g_hash_table_unref(info_hash);

  return success;
}

static bool get_device_info(DBusGConnection *bus, GError **error, const char *p, struct Power *result)
{
  bool success = true;
  DBusGProxy *device_proxy = NULL;
  GHashTable *info_hash;
  do
  {
    device_proxy = dbus_g_proxy_new_for_name(bus,
                                             SERVERNAME,       // server
                                             p,                // path
                                             PROPERTIES_FACE); // interface
    if (!dbus_g_proxy_call(device_proxy, "GetAll", error,
                           G_TYPE_STRING, DEVICEINTERFACE,
                           G_TYPE_INVALID,
                           dbus_g_type_get_map("GHashTable", G_TYPE_STRING, G_TYPE_VALUE), &info_hash,
                           G_TYPE_INVALID))
    {
      klog_err("Failed to call GetAll in path %s : %s\n", p, (*error)->message);
      success = false;
      break;
    }

    struct power_device *node = (struct power_device *)calloc(1, sizeof *node);
    if (!node)
    {
      klog_err("Failed to request memory %s\n", p);
      success = false;
      break;
    }
    // 前插
    node->next = result->devices;
    result->devices = node;

    // 写入device device数据
    strcpy(node->name, p);
    _write_info(node, info_hash);
  } while (0);

  if (device_proxy)
    g_object_unref(device_proxy);
  if (info_hash)
    g_hash_table_unref(info_hash);
  return success;
}

struct Power *kdk_hw_get_powerinfo()
{
  bool success = true;
  DBusGProxy *upower_proxy = NULL;
  GError *error = NULL;

  DBusGConnection *bus = dbus_g_bus_get(DBUS_BUS_SYSTEM, &error);
  if (!bus)
  {
    klog_err("Couldn't connect to system bus : %s\n", error->message);
    return NULL;
  }

  struct Power *result = (struct Power *)calloc(1, sizeof *result);
  if (!result)
  {
    klog_err("Failed to request memory Power");
    dbus_g_connection_unref(bus);
    return NULL;
  }

  if (!(success = get_daemon_info(bus, &error, result, &upower_proxy)))
    goto out;

  if (!(success = get_display_info(bus, upower_proxy, &error, result)))
    goto out;

  // 获取Device列表
  GPtrArray *devices = NULL;
  if (!dbus_g_proxy_call(upower_proxy, "EnumerateDevices", &error,
                         G_TYPE_INVALID,
                         dbus_g_type_get_collection("GPtrArray", DBUS_TYPE_G_OBJECT_PATH), &devices,
                         G_TYPE_INVALID))
  {
    klog_err("Failed to call EnumerateDevices in path %s : %s\n", "/org/freedesktop/UPower", error->message);
    success = false;
    goto out;
  }

  for (guint i = 0; i < devices->len; i++)
  {
    if (!(success = get_device_info(bus, &error, (char *)g_ptr_array_index(devices, i), result)))
      goto out;
  }

out:
  if (!success)
  {
    kdk_hw_free_power_info(result);
    result = NULL;
  }
  if (error)
    g_error_free(error);
  if (devices)
    g_ptr_array_unref(devices);
  if (upower_proxy)
    g_object_unref(upower_proxy);
  if (bus)
    dbus_g_connection_unref(bus);
  return result;
}

void kdk_hw_free_power_info(struct Power *info)
{
  if (info)
  {
    struct power_device *tmp = NULL;
    while (info->devices)
    {
      tmp = info->devices;
      info->devices = info->devices->next;
      free(tmp);
    }
    free(info);
  }
}
