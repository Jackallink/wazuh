/*
 * Wazuh Module for Agent Upgrading
 * Copyright (C) 2015-2020, Wazuh Inc.
 * July 3, 2020.
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General Public
 * License (version 2) as published by the FSF - Free Software
 * Foundation.
 */
#include "wazuh_modules/wmodules.h"
#include "wm_agent_upgrade_parsing.h"
#include "wm_agent_upgrade_tasks.h"
#include "wazuh_db/wdb.h"
#include "os_net/os_net.h"

/**
 * 
 * 
 * */
static cJSON* wm_agent_upgrade_analyze_agent(int agent_id, wm_agent_task *agent_task, wm_upgrade_error_code *error_code);

typedef enum _upgrade_results_codes {
    STATUS_UPDATED = 0,
    STATUS_UPDATING,
    STATUS_OUTDATED,
    STATUS_ERROR
} upgrade_results_codes;

static const char* upgrade_results_status[] = {
    [STATUS_UPDATED] = "UPDATED",
    [STATUS_UPDATING] = "UPDATING",
    [STATUS_OUTDATED] = "OUTDATED",
    [STATUS_ERROR]    = "ERROR"
};

static const char* upgrade_results_messages[] = {
    [STATUS_UPDATED]  = "Agent is updated",
    [STATUS_UPDATING] = "Agent is updating",
    [STATUS_OUTDATED] = "Agent is outdated",
    [STATUS_ERROR]    = "Agent upgrade process failed"
};

static const char* invalid_platforms[] = {
    "darwin",
    "solaris",
    "aix",
    "hpux",
    "bsd"
};

char* wm_agent_upgrade_process_upgrade_command(const int* agent_ids, wm_upgrade_task* task) {
    char* response = NULL;
    int agent = 0;
    int agent_id = 0;
    cJSON* json_response = cJSON_CreateArray();
    cJSON *json_task_module_request = cJSON_CreateArray();

    while (agent_id = agent_ids[agent++], agent_id) {
        wm_upgrade_error_code error_code;
        cJSON *task_request = NULL;
        wm_agent_task *agent_task = NULL;
        wm_upgrade_task *upgrade_task = NULL;

        agent_task = wm_agent_upgrade_init_agent_task();

        // Task information
        os_calloc(1, sizeof(wm_upgrade_task), upgrade_task);
        memcpy(upgrade_task, task, sizeof(wm_upgrade_task));

        agent_task->task_info = wm_agent_upgrade_init_task_info();
        agent_task->task_info->command = WM_UPGRADE_UPGRADE;
        agent_task->task_info->task = upgrade_task;

        if (task_request = wm_agent_upgrade_analyze_agent(agent_id, agent_task, &error_code), task_request) {
            cJSON_AddItemToArray(json_task_module_request, task_request);
        } else {
            cJSON *error_message = wm_agent_upgrade_parse_response_message(error_code, upgrade_error_codes[error_code], &agent_id, NULL, NULL);
            cJSON_AddItemToArray(json_response, error_message);
            wm_agent_upgrade_free_agent_task(agent_task);
        }
    }

    // Send request to task module and store task ids
    if (!wm_agent_upgrade_parse_task_module_task_ids(json_response, json_task_module_request)) {


        // TODO: Send WPK to agents and update task to UPDATING/ERROR


    } else {
        mtwarn(WM_AGENT_UPGRADE_LOGTAG, WM_UPGRADE_NO_AGENTS_TO_UPGRADE);
    }

    response = cJSON_PrintUnformatted(json_response);

    cJSON_Delete(json_task_module_request);
    cJSON_Delete(json_response);

    return response;
}

char* wm_agent_upgrade_process_upgrade_custom_command(const int* agent_ids, wm_upgrade_custom_task* task) {
    char* response = NULL;
    int agent = 0;
    int agent_id = 0;
    cJSON* json_response = cJSON_CreateArray();
    cJSON *json_task_module_request = cJSON_CreateArray();

    while (agent_id = agent_ids[agent++], agent_id) {
        wm_upgrade_error_code error_code;
        cJSON *task_request = NULL;
        wm_agent_task *agent_task = NULL;
        wm_upgrade_custom_task *upgrade_custom_task = NULL;

        agent_task = wm_agent_upgrade_init_agent_task();

        // Task information
        os_calloc(1, sizeof(wm_upgrade_custom_task), upgrade_custom_task);
        memcpy(upgrade_custom_task, task, sizeof(wm_upgrade_custom_task));

        agent_task->task_info = wm_agent_upgrade_init_task_info();
        agent_task->task_info->command = WM_UPGRADE_UPGRADE_CUSTOM;
        agent_task->task_info->task = upgrade_custom_task;

        if (task_request = wm_agent_upgrade_analyze_agent(agent_id, agent_task, &error_code), task_request) {
            cJSON_AddItemToArray(json_task_module_request, task_request);
        } else {
            cJSON *error_message = wm_agent_upgrade_parse_response_message(error_code, upgrade_error_codes[error_code], &agent_id, NULL, NULL);
            cJSON_AddItemToArray(json_response, error_message);
            wm_agent_upgrade_free_agent_task(agent_task);
        }
    }

    // Send request to task module and store task ids
    if (!wm_agent_upgrade_parse_task_module_task_ids(json_response, json_task_module_request)) {


        // TODO: Send WPK to agents and update task to UPDATING/ERROR


    } else {
        mtwarn(WM_AGENT_UPGRADE_LOGTAG, WM_UPGRADE_NO_AGENTS_TO_UPGRADE);
    }

    response = cJSON_PrintUnformatted(json_response);

    cJSON_Delete(json_task_module_request);
    cJSON_Delete(json_response);

    return response;
}

char* wm_agent_upgrade_process_upgrade_result_command(const int* agent_ids) {
    char* response = NULL;
    cJSON* json_response = cJSON_CreateArray();
    int agent = 0;
    int agent_id = 0;

    while (agent_id = agent_ids[agent++], agent_id) {

        // TODO: Implement upgrade_result command

        cJSON_AddItemToArray(json_response, wm_agent_upgrade_parse_response_message(WM_UPGRADE_SUCCESS, upgrade_results_messages[STATUS_OUTDATED], &agent_id, NULL, upgrade_results_status[STATUS_OUTDATED]));
    }

    response = cJSON_PrintUnformatted(json_response);
    cJSON_Delete(json_response);

    return response;
}

cJSON* wm_agent_upgrade_analyze_agent(int agent_id, wm_agent_task *agent_task, wm_upgrade_error_code *error_code) {
    cJSON *task_request = NULL;

    // Agent information
    agent_task->agent_info = wm_agent_upgrade_init_agent_info();
    agent_task->agent_info->agent_id = agent_id;

    if (wdb_agent_info(agent_id,
                       &agent_task->agent_info->platform,
                       &agent_task->agent_info->major_version,
                       &agent_task->agent_info->minor_version,
                       &agent_task->agent_info->architecture,
                       &agent_task->agent_info->wazuh_version,
                       &agent_task->agent_info->last_keep_alive)) {
            *error_code = WM_UPGRADE_GLOBAL_DB_FAILURE;
    } else {

        // TODO: Validate agent platform, version, architecture

        // TODO: Validate WPK for agent and download it if necessary

        // Save task entry for agent
        int result = wm_agent_upgrade_create_task_entry(agent_id, agent_task);

        if (result == OSHASH_SUCCESS) {
            task_request = wm_agent_upgrade_parse_task_module_request(agent_task->task_info->command, agent_id);
        } else if (result == OSHASH_DUPLICATED) {
            *error_code = WM_UPGRADE_UPGRADE_ALREADY_IN_PROGRESS;
        } else {
            *error_code = WM_UPGRADE_UNKNOWN_ERROR;
        }
    }

    return task_request;
}
