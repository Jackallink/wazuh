/*
 * Authd settings manager
 * Copyright (C) 2015-2021, Wazuh Inc.
 * May 29, 2017.
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General Public
 * License (version 2) as published by the FSF - Free Software
 * Foundation.
 */

#include "shared.h"
#include "auth.h"
#include "config/config.h"

// Read configuration
int authd_read_config(const char *path) {
    config.port = DEFAULT_PORT;
    config.force_options.connection_time = -1;
    config.key_request.compatibility_flag = 0;
    config.key_request.exec_path = NULL;
    config.key_request.socket = NULL;

    mdebug2("Reading configuration '%s'", path);

    if (ReadConfig(CAUTHD, path, &config, NULL) < 0) {
        return OS_INVALID;
    }

    if (!config.force_options.enabled) {
        config.force_options.connection_time = -1;
    }

    if (!config.ciphers) {
        config.ciphers = strdup(DEFAULT_CIPHERS);
    }

    switch (config.flags.disabled) {
    case AD_CONF_UNPARSED:
        config.flags.disabled = 1;
        break;
    case AD_CONF_UNDEFINED:
        config.flags.disabled = 0;
    }

    config.timeout_sec = getDefine_Int("auth", "timeout_seconds", 0, INT_MAX);
    config.timeout_usec = getDefine_Int("auth", "timeout_microseconds", 0, 999999);

    return 0;
}

#ifndef CLIENT

cJSON *getAuthdConfig(void) {

    cJSON *root = cJSON_CreateObject();
    cJSON *auth = cJSON_CreateObject();

    cJSON_AddNumberToObject(auth,"port",config.port);
    if (config.force_options.connection_time ==  -1)
        cJSON_AddStringToObject(auth,"force_time","no");
    else if (config.force_options.connection_time ==  0)
        cJSON_AddStringToObject(auth,"force_time","always");
    else
        cJSON_AddNumberToObject(auth,"force_time",config.force_options.connection_time);
    if (config.flags.disabled) cJSON_AddStringToObject(auth,"disabled","yes"); else cJSON_AddStringToObject(auth,"disabled","no");
    if (config.flags.remote_enrollment) cJSON_AddStringToObject(auth,"remote_enrollment","yes"); else cJSON_AddStringToObject(auth,"remote_enrollment","no");
    if (config.flags.use_source_ip) cJSON_AddStringToObject(auth,"use_source_ip","yes"); else cJSON_AddStringToObject(auth,"use_source_ip","no");
    if (config.force_options.enabled) cJSON_AddStringToObject(auth,"force_insert","yes"); else cJSON_AddStringToObject(auth,"force_insert","no");
    if (config.flags.clear_removed) cJSON_AddStringToObject(auth,"purge","yes"); else cJSON_AddStringToObject(auth,"purge","no");
    if (config.flags.use_password) cJSON_AddStringToObject(auth,"use_password","yes"); else cJSON_AddStringToObject(auth,"use_password","no");
    if (config.flags.verify_host) cJSON_AddStringToObject(auth,"ssl_verify_host","yes"); else cJSON_AddStringToObject(auth,"ssl_verify_host","no");
    if (config.flags.auto_negotiate) cJSON_AddStringToObject(auth,"ssl_auto_negotiate","yes"); else cJSON_AddStringToObject(auth,"ssl_auto_negotiate","no");
    if (config.ciphers) cJSON_AddStringToObject(auth,"ciphers",config.ciphers);
    if (config.agent_ca) cJSON_AddStringToObject(auth,"ssl_agent_ca",config.agent_ca);
    if (config.manager_cert) cJSON_AddStringToObject(auth,"ssl_manager_cert",config.manager_cert);
    if (config.manager_key) cJSON_AddStringToObject(auth,"ssl_manager_key",config.manager_key);
    if (config.key_request.enabled) {
        cJSON_AddStringToObject(auth,"enabled","yes");
        if (config.key_request.exec_path) cJSON_AddStringToObject(auth,"exec_path",config.key_request.exec_path);
        if (config.key_request.socket) cJSON_AddStringToObject(auth,"socket",config.key_request.socket);
        if (config.key_request.timeout) cJSON_AddNumberToObject(auth,"timeout",config.key_request.timeout);
        if (config.key_request.threads) cJSON_AddNumberToObject(auth,"threads",config.key_request.threads);
        if (config.key_request.queue_size) cJSON_AddNumberToObject(auth,"queue_size",config.key_request.queue_size);
    } else cJSON_AddStringToObject(auth,"enabled","no");

    cJSON_AddItemToObject(root,"auth",auth);

    return root;
}

#endif
