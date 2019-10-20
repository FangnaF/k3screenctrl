#include <stdio.h>
#include <string.h>
#include <syslog.h>
#include <errno.h>

#include "handlers.h"
#include "mcu_proto.h"
#include "pages.h"
#include "requests.h"
#include "signals.h"
#include "config.h"

static MCU_VERSION g_mcu_version;
void handle_mcu_version(const unsigned char *payload, int len) {
    if (len < 4) {
        syslog(LOG_WARNING,
               "Got malformed MCU version response. Length is %d\n", len);
        return;
    }
    g_mcu_version.patch_ver =
        payload[0] |
        payload[1] << 8; /* Do we need this endian compatabitity? */
    g_mcu_version.minor_ver = payload[2];
    g_mcu_version.major_ver = payload[3];

    syslog(LOG_INFO, "MCU reported version as %hhd.%hhd.%hd\n",
           g_mcu_version.major_ver, g_mcu_version.minor_ver,
           g_mcu_version.patch_ver);
}
int handle_long_press(const char *script, int page_num)
{
    int ret = -1;
    int len = 0;
    static char cmd_with_param[256];

    memset(&cmd_with_param, 0, sizeof(cmd_with_param));
    len = snprintf(cmd_with_param, sizeof(cmd_with_param), "%s %d", script, page_num);
    if (len > sizeof(cmd_with_param)) {
        syslog(LOG_WARNING, "script path is too long, make it less than 256 chars: %s\n", script);
        return -1;
    }

    FILE *fp = popen(cmd_with_param, "r");
    if (fp == NULL) {
        syslog(LOG_ERR, "could not execute key long press script \"%s\": %s\n", script,
            strerror(errno));
        return -1;
    }

    return pclose(fp);
}

int g_is_screen_on = 1;
void handle_key_press(const unsigned char *payload, int len) {
    int ret = -1;

    if (len < 1) {
        syslog(LOG_WARNING, "Got malformed key press response. Length is %d\n",
               len);
        return;
    }
    refresh_screen_timeout();
    if (!g_is_screen_on) {
        /* Do not process key messages when waking up */
        request_notify_event(EVENT_WAKEUP);
        g_is_screen_on = 1;
        return;
    }
    switch (payload[0]) {
    case KEY_LEFT_SHORT:
        page_switch_prev();
        printf("KEY_LEFT_SHORT\n");
        break;
    case KEY_RIGHT_SHORT:
        page_switch_next();
        printf("KEY_RIGHT_SHORT\n");
        break;
    case KEY_MIDDLE_SHORT:
        if (CFG->home_page < PAGE_MIN || CFG->home_page > PAGE_MAX) {
            syslog(LOG_WARNING, "invalid home page : %d\n", CFG->home_page);
            return;
        }
        page_switch_to(CFG->home_page);
        printf("KEY_MIDDLE_SHORT %d\n", CFG->home_page);
        break;
    case KEY_MIDDLE_LONG:
        printf("KEY_MIDDLE_LONG\n");
        request_notify_event(EVENT_SLEEP);
        g_is_screen_on = 0;
        return;
    case KEY_LEFT_LONG:
        ret = handle_long_press(CFG->left_long_script, page_get_index());
        printf("KEY_LEFT_LONG script %s with status code %d\n", CFG->left_long_script, ret);
        return;
    case KEY_RIGHT_LONG:
        ret = handle_long_press(CFG->right_long_script, page_get_index());
        printf("KEY_RIGHT_LONG script %s with status code %d\n", CFG->right_long_script, ret);
        return;
    default:
        syslog(LOG_WARNING, "unknown key code: %hhx\n", payload[0]);
        return;
    }
}

RESPONSE_HANDLER g_response_handlers[] = {
    {RESPONSE_MCU_VERSION, handle_mcu_version},
    {RESPONSE_KEY_PRESS, handle_key_press},
};
