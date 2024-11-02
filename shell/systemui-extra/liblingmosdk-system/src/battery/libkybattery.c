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

#include "libkybattery.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <lingmosdk/lingmosdk-base/cstring-extension.h>
#include "dbus/dbus-glib.h"
#include "libkylog.h"

#define SERVERNAME "org.freedesktop.UPower"
#define PROPERTIES_FACE "org.freedesktop.DBus.Properties"
#define DEVICEINTERFACE "org.freedesktop.UPower.Device"

struct power_device
{
    char name[256]; //设备名
    char native_path[32];
    bool power_supply;
    char updated[64];
    bool has_history;
    bool has_statistics;
    bool is_persent;
    bool is_rechargeable;
    char state[32];
    char warning_level[32];
    double energy;
    double energy_empty;
    double energy_full;
    double energy_full_design;
    double energy_rate;
    double voltage;
    long time_to_empty;
    long time_to_full;
    double percentage;
    double temperature;
    double capacity;
    char technology[32];
    bool online;
    char icon_name[64];
    char model[64];
    int battery_level;
    int type;
    struct power_device *next;
};

struct Power
{
    char daemon_version[32]; //版本
    bool on_battery; //是否存在电池
    bool lid_is_closed; 
    bool lid_is_present;
    char critical_action[32];
    struct power_device *devices;
};

static void kdk_hw_free_power_info(struct Power *info)
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

static struct Power *kdk_hw_get_powerinfo()
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

float kdk_battery_get_soc()
{
    float percentage = 0.0;
    struct Power * power = kdk_hw_get_powerinfo();

    struct power_device *tmp = power->devices;
    while (tmp)
    {
        if(strstr(tmp->name, "battery"))
        {
            percentage = tmp->percentage;
        }
        tmp = tmp->next;
    }
    kdk_hw_free_power_info(power);
    return percentage;
}

BatteryChargeState kdk_battery_get_charge_state()
{
    char *path = NULL;
    char charge_path[128] = "\0";
    char state[64] = "\0";
    struct Power * power = kdk_hw_get_powerinfo();

    struct power_device *tmp = power->devices;
    while (tmp)
    {
        if(strstr(tmp->name, "battery"))
        {
            if(tmp->native_path)
            {
                path = malloc(strlen(tmp->native_path) + 1);
                strcpy(path, tmp->native_path);
            }
        }
        tmp = tmp->next;
    }
    kdk_hw_free_power_info(power);
    if(path)
    {
        sprintf(charge_path, "/sys/class/power_supply/%s/status", path);
        free(path);
    }
    FILE *fp = NULL;
    fp = fopen(charge_path, "r");
    if (fp)
    {
        fgets(state, 64, fp);
    }
    strstripspace(state);
    if(strcmp(state, "Charging") == 0)
    {
        return ENABLE;
    }
    else if(strcmp(state, "Discharging") == 0)
    {
        return DISABLE;
    }
    else if(strcmp(state, "Full") == 0)
    {
        return FULL;
    }
    else if(strcmp(state, "Not charging") == 0)
    {
        return NOT_CHARGING;
    }
    else if(strcmp(state, "Unknown") == 0)
    {
        return NONE;
    }
    else
    {
        return NONE;
    }
}

BatteryHealthState kdk_battery_get_health_state()
{
    char *path = NULL;
    char health_path[128] = "\0";
    char state[64] = "\0";
    struct Power * power = kdk_hw_get_powerinfo();

    struct power_device *tmp = power->devices;
    while (tmp)
    {
        if(strstr(tmp->name, "battery"))
        {
            if(tmp->native_path)
            {
                path = malloc(strlen(tmp->native_path) + 1);
                strcpy(path, tmp->native_path);
            }
        }
        tmp = tmp->next;
    }
    kdk_hw_free_power_info(power);
    if(path)
    {
        sprintf(health_path, "/sys/class/power_supply/%s/health", path);
        free(path);
    }
    FILE *fp = NULL;
    fp = fopen(health_path, "r");
    if (fp)
    {
        fgets(state, 64, fp);
    }
    strstripspace(state);
    if(strcmp(state, "Good") == 0)
    {
        return GOOD;
    }
    else if(strcmp(state, "Overheat") == 0)
    {
        return OVERHEAT;
    }
    else if(strcmp(state, "Over voltage") == 0)
    {
        return OVERVOLTAGE;
    }
    else if(strcmp(state, "Cold") == 0)
    {
        return COLD;
    }
    else if(strcmp(state, "Dead") == 0)
    {
        return DEAD;
    }
    else if(strcmp(state, "Unspecified failure") == 0)
    {
        return UNSPEC_FAILURE;
    }
    else if(strcmp(state, "Watchdog timer expire") == 0)
    {
        return WATCHDOG_TIMER_EXPIRE;
    }
    else if(strcmp(state, "Safety timer expire") == 0)
    {
        return SAFETY_TIMER_EXPIRE;
    }
    else if(strcmp(state, "Over current") == 0)
    {
        return OVERCURRENT;
    }
    else if(strcmp(state, "Calibration required") == 0)
    {
        return CALIBRATION_REQUIRED;
    }
    else if(strcmp(state, "Warm") == 0)
    {
        return WARM;
    }
    else if(strcmp(state, "Cool") == 0)
    {
        return COOL;
    }
    else if(strcmp(state, "Unknown") == 0)
    {
        return UNKNOWN;
    }
    else
    {
        return UNKNOWN;
    }
}

BatteryPluggedType kdk_battery_get_plugged_type()
{
    char *path = NULL;
    char type_path[128] = "\0";
    char state[64] = "\0";
    struct Power * power = kdk_hw_get_powerinfo();

    struct power_device *tmp = power->devices;
    while (tmp)
    {
        if(strstr(tmp->name, "battery"))
        {
            if(tmp->native_path)
            {
                path = malloc(strlen(tmp->native_path) + 1);
                strcpy(path, tmp->native_path);
            }
        }
        tmp = tmp->next;
    }
    kdk_hw_free_power_info(power);
    if(path)
    {
        sprintf(type_path, "/sys/class/power_supply/%s/type", path);
        free(path);
    }
    FILE *fp = NULL;
    fp = fopen(type_path, "r");
    if (fp)
    {
        fgets(state, 64, fp);
    }
    strstripspace(state);
    
    if(strcmp(state, "Mains") == 0)
    {
        return MAINS;
    }
    else if(strcmp(state, "Battery") == 0)
    {
        return BATTERY;
    }
    else if(strcmp(state, "USB_DCP") == 0)
    {
        return USB_DCP;
    }
    else if(strcmp(state, "USB_CDP") == 0)
    {
        return USB_CDP;
    }
    else if(strcmp(state, "USB_ACA") == 0)
    {
        return USB_ACA;
    }
    else if(strcmp(state, "UPS") == 0)
    {
        return UPS;
    }
    else if(strcmp(state, "USB") == 0)
    {
        return USB;
    }
    else if(strcmp(state, "USB_C") == 0)
    {
        return USB_TYPE_C;
    }
    else if(strcmp(state, "USB_PD") == 0)
    {
        return USB_PD;
    }
    else if(strcmp(state, "USB_PD_DRP") == 0)
    {
        return USB_PD_DRP;
    }
    else if(strcmp(state, "BrickID") == 0)
    {
        return APPLE_BRICK_ID;
    }
    else if(strcmp(state, "Wireless") == 0)
    {
        return WIRELESS;
    }
    else if(strcmp(state, "Unknown") == 0)
    {
        return TYPE_NONE;
    }
    else{
        return TYPE_NONE;
    }
}

float kdk_battery_get_voltage()
{
    float voltage = 0.0;
    struct Power * power = kdk_hw_get_powerinfo();

    struct power_device *tmp = power->devices;
    while (tmp)
    {
        if(strstr(tmp->name, "battery"))
        {
            voltage = tmp->voltage;
        }
        tmp = tmp->next;
    }
    kdk_hw_free_power_info(power);
    return voltage;
}

char* kdk_battery_get_technology()
{
    char *model = NULL;
    struct Power * power = kdk_hw_get_powerinfo();

    struct power_device *tmp = power->devices;
    while (tmp)
    {
        if(strstr(tmp->name, "battery"))
        {
            if(tmp->model)
            {
                model = malloc(strlen(tmp->model) + 1);
                strcpy(model, tmp->model);
            }
        }
        tmp = tmp->next;
    }
    kdk_hw_free_power_info(power);
    return model;
}

float kdk_battery_get_temperature()
{
    float temperature = 0.0;
    struct Power * power = kdk_hw_get_powerinfo();

    struct power_device *tmp = power->devices;
    while (tmp)
    {
        if(strstr(tmp->name, "battery"))
        {
            temperature = tmp->temperature;
        }
        tmp = tmp->next;
    }
    kdk_hw_free_power_info(power);
    return temperature;
}

bool kdk_battery_is_present()
{
    bool on_battery = false;
    struct Power * power = kdk_hw_get_powerinfo();
    struct power_device *tmp = power->devices;
    while (tmp)
    {
        if(strstr(tmp->name, "battery"))
        {
            on_battery = tmp->is_persent;
        }
        tmp = tmp->next;
    }
    kdk_hw_free_power_info(power);
    return on_battery;
}

int kdk_battery_get_capacity_level()
{
    int level = 0;
    struct Power * power = kdk_hw_get_powerinfo();

    struct power_device *tmp = power->devices;
    while (tmp)
    {
        if(strstr(tmp->name, "battery"))
        {
            level = tmp->battery_level;
        }
        tmp = tmp->next;
    }
    kdk_hw_free_power_info(power);
    return level;
}