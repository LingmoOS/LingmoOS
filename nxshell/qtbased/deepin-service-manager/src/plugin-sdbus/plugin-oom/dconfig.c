// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "dconfig.h"

#include "3rdparty/earlyoom/msg.h"

#include <systemd/sd-bus.h>
#include <systemd/sd-event.h>

void get_config_value(
    sd_bus *bus, const char *path, const char *key, const char *outtype, void *output);
int match_handler(sd_bus_message *m, void *userdata, sd_bus_error *ret_error);

void compile_regex(const char *regex_str, regex_t **reg)
{
    bool regex_is_empty = (regex_str == NULL || strcmp(regex_str, "") == 0);
    if (!regex_is_empty) {
        if (*reg == NULL) {
            *reg = (regex_t *)malloc(sizeof(regex_t));
        }
        regcomp(*reg, regex_str, REG_EXTENDED);
    } else if (*reg != NULL) {
        free(*reg);
        *reg = NULL;
    }
}

void parse_config(sd_bus *bus, poll_loop_args_t *args)
{
    sd_bus_error error = SD_BUS_ERROR_NULL;
    sd_bus_message *m = NULL;
    const char *path;
    int r;
    /* Issue the method call and store the response message in m */
    r = sd_bus_call_method(bus,
                           "org.desktopspec.ConfigManager", /* service to contact */
                           "/",                             /* object path */
                           "org.desktopspec.ConfigManager", /* interface name */
                           "acquireManager",                /* method name */
                           &error,                          /* object to return error in */
                           &m,                              /* return message on success */
                           "sss",                           /* input signature */
                           "org.deepin.oom",                /* first argument */
                           "org.deepin.oom",                /* second argument */
                           "");
    if (r < 0) {
        warn("Failed to issue method call: %s\n", error.message);
        goto finish;
    }

    /* Parse the response message */
    r = sd_bus_message_read(m, "o", &path);
    if (r < 0) {
        warn("Failed to parse response message: %s\n", strerror(-r));
        goto finish;
    }

    /* Get config value */
    get_config_value(bus, path, "mem_term_percent", "d", &args->mem_term_percent);
    get_config_value(bus, path, "mem_kill_percent", "d", &args->mem_kill_percent);
    get_config_value(bus, path, "swap_term_percent", "d", &args->swap_term_percent);
    get_config_value(bus, path, "swap_kill_percent", "d", &args->swap_kill_percent);
    get_config_value(bus, path, "report_interval_ms", "d", &args->report_interval_ms);
    get_config_value(bus, path, "kill_process_group", "b", &args->kill_process_group);
    get_config_value(bus, path, "ignore_root_user", "b", &args->ignore_root_user);

    char *regex_str = NULL;
    get_config_value(bus, path, "prefer_regex", "s", &regex_str);
    compile_regex(regex_str, &args->prefer_regex);
    get_config_value(bus, path, "avoid_regex", "s", &regex_str);
    compile_regex(regex_str, &args->avoid_regex);
    get_config_value(bus, path, "ignore_regex", "s", &regex_str);
    compile_regex(regex_str, &args->ignore_regex);

    /* connect signal */
    r = sd_bus_match_signal(bus,
                            NULL,
                            "org.desktopspec.ConfigManager",
                            path,
                            "org.desktopspec.ConfigManager.Manager",
                            "valueChanged",
                            match_handler,
                            args);

finish:
    sd_bus_error_free(&error);
    sd_bus_message_unref(m);
}

void get_config_value(
    sd_bus *bus, const char *path, const char *key, const char *outtype, void *output)
{
    sd_bus_error error = SD_BUS_ERROR_NULL;
    sd_bus_message *m = NULL;
    int r;
    r = sd_bus_call_method(bus,
                           "org.desktopspec.ConfigManager",         /* service to contact */
                           path,                                    /* object path */
                           "org.desktopspec.ConfigManager.Manager", /* interface name */
                           "value",                                 /* method name */
                           &error,                                  /* object to return error in */
                           &m,                                      /* return message on success */
                           "s",                                     /* input signature */
                           key);
    if (r < 0) {
        warn("Failed to issue method call: %s\n", error.message);
        goto finish;
    }

    /* Parse the response message */
    r = sd_bus_message_read(m, "v", outtype, output);

    if (r < 0) {
        warn("Failed to parse response message: %s\n", strerror(-r));
        goto finish;
    }
finish:
    sd_bus_error_free(&error);
    sd_bus_message_unref(m);
}

typedef struct
{
    char *name;
    char *type;
    void *value;
} config_value_t;

typedef struct
{
    char *name;
    regex_t **rex;
} regex_value_t;

int match_handler(sd_bus_message *m, void *userdata, sd_bus_error *ret_error)
{
    poll_loop_args_t *args = userdata;
    const char *value;
    sd_bus *bus = sd_bus_message_get_bus(m);
    const char *path = sd_bus_message_get_path(m);
    const config_value_t configs[] = {
        { "mem_term_percent", "d", &args->mem_term_percent },
        { "mem_kill_percent", "d", &args->mem_kill_percent },
        { "swap_term_percent", "d", &args->swap_term_percent },
        { "swap_kill_percent", "d", &args->swap_kill_percent },
        { "report_interval_ms", "d", &args->report_interval_ms },
        { "kill_process_group", "b", &args->kill_process_group },
        { "ignore_root_user", "b", &args->ignore_root_user },
    };
    int r;
    r = sd_bus_message_read(m, "s", &value);
    if (r < 0) {
        warn("Failed to parse response message: %s\n", strerror(-r));
        return r;
    }
    bool find = false;
    for (unsigned long i = 0; i < sizeof(configs) / sizeof(config_value_t); i++) {
        if (strcmp(configs[i].name, value) == 0) {
            get_config_value(bus, path, value, configs[i].type, configs[i].value);
            find = true;
            printf("update config %s = %d\n", configs[i].name, *(int *)configs[i].value);
            break;
        }
    }

    if (!find) {
        const regex_value_t regex_configs[] = {
            { "prefer_regex", &args->prefer_regex },
            { "avoid_regex", &args->avoid_regex },
            { "ignore_regex", &args->ignore_regex },
        };
        for (unsigned long i = 0; i < sizeof (regex_configs) / sizeof (regex_value_t); i++) {
            if (strcmp(regex_configs[i].name, value) == 0) {
                char *regexStr = NULL;
                get_config_value(bus, path, value, "s", &regexStr);
                compile_regex(regexStr, regex_configs[i].rex);
                find = true;
                printf("update config %s = %s\n", configs[i].name, regexStr);
                break;
            }
        }
    }

    return 0;
}
