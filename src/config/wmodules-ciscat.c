/*
 * Wazuh Module Configuration
 * Copyright (C) 2015-2019, Wazuh Inc.
 * December, 2017.
 *
 * This program is a free software; you can redistribute it
 * and/or modify it under the terms of the GNU General Public
 * License (version 2) as published by the FSF - Free Software
 * Foundation.
 */
#ifdef ENABLE_CISCAT
#include "wazuh_modules/wmodules.h"

static const char *XML_CONTENT = "content";
static const char *XML_CONTENT_TYPE = "type";
static const char *XML_XCCDF = "xccdf";
static const char *XML_OVAL = "oval";
static const char *XML_PATH = "path";
static const char *XML_TIMEOUT = "timeout";
static const char *XML_INTERVAL = "interval";
static const char *XML_SCAN_DAY = "day";
static const char *XML_SCAN_WDAY = "wday";
static const char *XML_SCAN_TIME = "time";
static const char *XML_SCAN_ON_START = "scan-on-start";
static const char *XML_PROFILE = "profile";
static const char *XML_JAVA_PATH = "java_path";
static const char *XML_CISCAT_PATH = "ciscat_path";
static const char *XML_DISABLED = "disabled";

// Parse XML

int wm_ciscat_read(const OS_XML *xml, xml_node **nodes, wmodule *module, char **output)
{
    int i = 0;
    int j = 0;
    xml_node **children = NULL;
    wm_ciscat *ciscat;
    wm_ciscat_eval *cur_eval = NULL;
    int month_interval = 0;
    char message[OS_FLSIZE];


    // Create module

    os_calloc(1, sizeof(wm_ciscat), ciscat);
    ciscat->flags.enabled = 1;
    ciscat->flags.scan_on_start = 1;
    ciscat->scan_wday = -1;
    ciscat->scan_time = NULL;
    module->context = &WM_CISCAT_CONTEXT;
    module->tag = strdup(module->context->name);
    module->data = ciscat;

    if (!nodes)
        return 0;

    // Iterate over module subelements

    for (i = 0; nodes[i]; i++){
        if (!nodes[i]->element) {
            if (output  == NULL){
                merror(XML_ELEMNULL);
            } else {
                wm_strcat(output, "Invalid NULL element in the configuration.", '\n');
            }
            return OS_INVALID;
        } else if (!strcmp(nodes[i]->element, XML_TIMEOUT)) {
            ciscat->timeout = atol(nodes[i]->content);

            if (ciscat->timeout <= 0 || ciscat->timeout >= UINT_MAX) {
                if (output == NULL) {
                    merror("Invalid timeout at module '%s'.", WM_CISCAT_CONTEXT.name);
                } else {
                    snprintf(message, OS_FLSIZE + 1,
                        "Invalid timeout at module '%s'.",
                        WM_CISCAT_CONTEXT.name);
                    wm_strcat(output, message, '\n');
                }
                return OS_INVALID;
            }
        } else if (!strcmp(nodes[i]->element, XML_CONTENT)) {

            // Create policy node

            if (cur_eval) {
                os_calloc(1, sizeof(wm_ciscat_eval), cur_eval->next);
                cur_eval = cur_eval->next;
            } else {
                // First policy
                os_calloc(1, sizeof(wm_ciscat_eval), cur_eval);
                ciscat->evals = cur_eval;
            }

            // Parse policy attributes

            for (j = 0; nodes[i]->attributes && nodes[i]->attributes[j]; j++) {
                if (!strcmp(nodes[i]->attributes[j], XML_PATH))
                    cur_eval->path = strdup(nodes[i]->values[j]);
                else if (!strcmp(nodes[i]->attributes[j], XML_CONTENT_TYPE)) {
                    if (!strcmp(nodes[i]->values[j], XML_XCCDF))
                        cur_eval->type = WM_CISCAT_XCCDF;
                    else if (!strcmp(nodes[i]->values[j], XML_OVAL))
                        cur_eval->type = WM_CISCAT_OVAL;
                    else if (output == NULL) {
                        merror("Invalid content for attribute '%s' at module '%s'.", XML_CONTENT_TYPE, WM_CISCAT_CONTEXT.name);
                        return OS_INVALID;
                    } else {
                        snprintf(message, OS_FLSIZE + 1,
                            "Invalid content for attribute '%s' at module '%s'.",
                            XML_CONTENT_TYPE, WM_CISCAT_CONTEXT.name);
                        wm_strcat(output, message, '\n');
                        return OS_INVALID;
                    }
                } else if (output == NULL) {
                    merror("Invalid attribute '%s' at module '%s'.", nodes[i]->attributes[0], WM_CISCAT_CONTEXT.name);
                    return OS_INVALID;
                } else {
                    snprintf(message, OS_FLSIZE + 1,
                        "Invalid attribute '%s' at module '%s'.",
                        nodes[i]->attributes[0], WM_CISCAT_CONTEXT.name);
                    wm_strcat(output, message, '\n');
                    return OS_INVALID;
                }
            }

            // Set 'xccdf' type by default.

            if (!cur_eval->type) {
                cur_eval->type = WM_CISCAT_XCCDF;
            }

            // Expand policy children (optional)

            if (!(children = OS_GetElementsbyNode(xml, nodes[i]))) {
                continue;
            }

            for (j = 0; children[j]; j++) {
                if (!children[j]->element) {
                    if (output  == NULL){
                        merror(XML_ELEMNULL);
                    } else {
                        wm_strcat(output, "Invalid NULL element in the configuration.", '\n');
                    }
                    OS_ClearNode(children);
                    return OS_INVALID;
                }

                if (!strcmp(children[j]->element, XML_PROFILE)) {
                    if (cur_eval->type != WM_CISCAT_XCCDF) {
                        if (output == NULL) {
                            merror("Tag '%s' on incorrect content type at module '%s'", children[j]->element, WM_CISCAT_CONTEXT.name);
                        } else {
                            snprintf(message, OS_FLSIZE + 1,
                                "Tag '%s' on incorrect content type at module '%s'.",
                                children[j]->element, WM_CISCAT_CONTEXT.name);
                            wm_strcat(output, message, '\n');
                        }
                        OS_ClearNode(children);
                        return OS_INVALID;
                    }

                    cur_eval->profile = strdup(children[j]->content);
                } else if (!strcmp(children[j]->element, XML_TIMEOUT)) {
                    cur_eval->timeout = atol(children[j]->content);

                    if (cur_eval->timeout <= 0 || cur_eval->timeout >= UINT_MAX) {
                        if (output == NULL) {
                            merror("Invalid timeout at module '%s'", WM_CISCAT_CONTEXT.name);
                        } else {
                            snprintf(message, OS_FLSIZE + 1,
                                "Invalid timeout at module '%s'",
                                WM_CISCAT_CONTEXT.name);
                            wm_strcat(output, message, '\n');
                        }
                        OS_ClearNode(children);
                        return OS_INVALID;
                    }
                } else if (!strcmp(children[j]->element, XML_PATH)) {
                    if (cur_eval->path) {
                        if (output == NULL) {
                            mwarn("Duplicate path for content at module '%s'", WM_CISCAT_CONTEXT.name);
                        } else {
                            snprintf(message, OS_FLSIZE + 1,
                                "WARNING: Duplicate path for content at module '%s'",
                                WM_CISCAT_CONTEXT.name);
                            wm_strcat(output, message, '\n');
                        }
                        free(cur_eval->path);
                    }

                    cur_eval->path = strdup(children[j]->content);
                } else if (output == NULL){
                    merror("No such tag '%s' at module '%s'.", children[j]->element, WM_CISCAT_CONTEXT.name);
                    OS_ClearNode(children);
                    return OS_INVALID;
                } else {
                    snprintf(message, OS_FLSIZE + 1,
                        "No such tag '%s' at module '%s'.",
                        children[j]->element, WM_CISCAT_CONTEXT.name);
                    wm_strcat(output, message, '\n');
                    OS_ClearNode(children);
                    return OS_INVALID;
                }
            }

            OS_ClearNode(children);

            if (!cur_eval->path) {
                if (output == NULL) {
                    merror("No such content path at module '%s'.", WM_CISCAT_CONTEXT.name);
                } else {
                    snprintf(message, OS_FLSIZE + 1,
                        "No such content path at module '%s'.",
                        WM_CISCAT_CONTEXT.name);
                    wm_strcat(output, message, '\n');
                }
                return OS_INVALID;
            }

        } else if (!strcmp(nodes[i]->element, XML_INTERVAL)) {
            char *endptr;
            ciscat->interval = strtoul(nodes[i]->content, &endptr, 0);

            if (ciscat->interval <= 0 || ciscat->interval >= UINT_MAX) {
                if (output == NULL) {
                    merror("Invalid interval at module '%s'", WM_CISCAT_CONTEXT.name);
                } else {
                    snprintf(message, OS_FLSIZE + 1,
                        "Invalid interval at module '%s'.",
                        WM_CISCAT_CONTEXT.name);
                    wm_strcat(output, message, '\n');
                }
                return OS_INVALID;
            }

            switch (*endptr) {
            case 'M':
                month_interval = 1;
                ciscat->interval *= 60; // We can`t calculate seconds of a month
                break;
            case 'w':
                ciscat->interval *= 604800;
                break;
            case 'd':
                ciscat->interval *= 86400;
                break;
            case 'h':
                ciscat->interval *= 3600;
                break;
            case 'm':
                ciscat->interval *= 60;
                break;
            case 's':
            case '\0':
                break;
            default:
                if (output == NULL) {
                    merror("Invalid interval at module '%s'", WM_CISCAT_CONTEXT.name);
                } else {
                    snprintf(message, OS_FLSIZE + 1,
                        "Invalid interval at module '%s'.",
                        WM_CISCAT_CONTEXT.name);
                    wm_strcat(output, message, '\n');
                }
                return OS_INVALID;
            }

            if (ciscat->interval < 60) {
                mwarn("At module '%s': Interval must be greater than 60 seconds. New interval value: 60s.", WM_CISCAT_CONTEXT.name);
                ciscat->interval = 60;
            }
        } else if (!strcmp(nodes[i]->element, XML_SCAN_DAY)) {
            if (!OS_StrIsNum(nodes[i]->content)) {
                if (output == NULL) {
                    merror(XML_VALUEERR, nodes[i]->element, nodes[i]->content);
                } else {
                    snprintf(message, OS_FLSIZE + 1,
                        "Invalid value for element '%s': %s.",
                        nodes[i]->element, nodes[i]->content);
                    wm_strcat(output, message, '\n');
                }
                return (OS_INVALID);
            } else {
                ciscat->scan_day = atoi(nodes[i]->content);
                if (ciscat->scan_day < 1 || ciscat->scan_day > 31) {
                    if (output == NULL) {
                        merror(XML_VALUEERR, nodes[i]->element, nodes[i]->content);
                    } else {
                        snprintf(message, OS_FLSIZE + 1,
                            "Invalid value for element '%s': %s.",
                            nodes[i]->element, nodes[i]->content);
                        wm_strcat(output, message, '\n');
                    }
                    return (OS_INVALID);
                }
            }
        } else if (!strcmp(nodes[i]->element, XML_SCAN_WDAY)) {
            ciscat->scan_wday = w_validate_wday(nodes[i]->content);
            if (ciscat->scan_wday < 0 || ciscat->scan_wday > 6) {
                if (output == NULL) {
                    merror(XML_VALUEERR, nodes[i]->element, nodes[i]->content);
                } else {
                    snprintf(message, OS_FLSIZE + 1,
                        "Invalid value for element '%s': %s.",
                        nodes[i]->element, nodes[i]->content);
                    wm_strcat(output, message, '\n');
                }
                return (OS_INVALID);
            }
        } else if (!strcmp(nodes[i]->element, XML_SCAN_TIME)) {
            ciscat->scan_time = w_validate_time(nodes[i]->content);
            if (!ciscat->scan_time) {
                if (output == NULL) {
                    merror(XML_VALUEERR, nodes[i]->element, nodes[i]->content);
                } else {
                    snprintf(message, OS_FLSIZE + 1,
                        "Invalid value for element '%s': %s.",
                        nodes[i]->element, nodes[i]->content);
                    wm_strcat(output, message, '\n');
                }
                return (OS_INVALID);
            }
        } else if (!strcmp(nodes[i]->element, XML_SCAN_ON_START)) {
            if (!strcmp(nodes[i]->content, "yes"))
                ciscat->flags.scan_on_start = 1;
            else if (!strcmp(nodes[i]->content, "no"))
                ciscat->flags.scan_on_start = 0;
            else if (output) {
                merror("Invalid content for tag '%s' at module '%s'.", XML_SCAN_ON_START, WM_CISCAT_CONTEXT.name);
                return OS_INVALID;
            } else {
                snprintf(message, OS_FLSIZE + 1,
                    "Invalid content for tag '%s' at module '%s'.",
                    XML_SCAN_ON_START, WM_CISCAT_CONTEXT.name);
                wm_strcat(output, message, '\n');
                return OS_INVALID;
            }
        } else if (!strcmp(nodes[i]->element, XML_DISABLED)) {
            if (!strcmp(nodes[i]->content, "yes"))
                ciscat->flags.enabled = 0;
            else if (!strcmp(nodes[i]->content, "no"))
                ciscat->flags.enabled = 1;
            else if (output == NULL) {
                merror("Invalid content for tag '%s' at module '%s'.", XML_DISABLED, WM_CISCAT_CONTEXT.name);
                return OS_INVALID;
            } else {
                snprintf(message, OS_FLSIZE + 1,
                    "Invalid content for tag '%s' at module '%s'.",
                    XML_DISABLED, WM_CISCAT_CONTEXT.name);
                wm_strcat(output, message, '\n');
                return OS_INVALID;
            }
        } else if (!strcmp(nodes[i]->element, XML_JAVA_PATH)) {
            ciscat->java_path = strdup(nodes[i]->content);
        } else if (!strcmp(nodes[i]->element, XML_CISCAT_PATH)) {
            ciscat->ciscat_path = strdup(nodes[i]->content);
        } else if (output == NULL) {
            merror("No such tag '%s' at module '%s'.", nodes[i]->element, WM_CISCAT_CONTEXT.name);
            return OS_INVALID;
        } else {
            snprintf(message, OS_FLSIZE + 1,
                "No such tag '%s' at module '%s'.",
                nodes[i]->element, WM_CISCAT_CONTEXT.name);
            wm_strcat(output, message, '\n');
            return OS_INVALID;
        }
    }

    // Validate scheduled scan parameters and interval value

    if (ciscat->scan_day && (ciscat->scan_wday >= 0)) {
        if (output == NULL) {
            merror("At module '%s': 'day' is not compatible with 'wday'.", WM_CISCAT_CONTEXT.name);
        } else {
            snprintf(message, OS_FLSIZE + 1,
                "At module '%s': 'day' is not compatible with 'wday'.",
                WM_CISCAT_CONTEXT.name);
            wm_strcat(output, message, '\n');
        }
        return OS_INVALID;
    } else if (ciscat->scan_day) {
        if (!month_interval) {
            mwarn("At module '%s': Interval must be a multiple of one month. New interval value: 1M.", WM_CISCAT_CONTEXT.name);
            ciscat->interval = 60; // 1 month
        }
        if (!ciscat->scan_time)
            ciscat->scan_time = strdup("00:00");
    } else if (ciscat->scan_wday >= 0) {
        if (w_validate_interval(ciscat->interval, 1) != 0) {
            ciscat->interval = 604800;  // 1 week
            mwarn("At module '%s': Interval must be a multiple of one week. New interval value: 1w.", WM_CISCAT_CONTEXT.name);
        }
        if (ciscat->interval == 0)
            ciscat->interval = 604800;
        if (!ciscat->scan_time)
            ciscat->scan_time = strdup("00:00");
    } else if (ciscat->scan_time) {
        if (w_validate_interval(ciscat->interval, 0) != 0) {
            ciscat->interval = WM_DEF_INTERVAL;  // 1 day
            mwarn("At module '%s': Interval must be a multiple of one day. New interval value: 1d.", WM_CISCAT_CONTEXT.name);
        }
    }
    if (!ciscat->interval)
        ciscat->interval = WM_DEF_INTERVAL;

    return 0;
}
#endif
