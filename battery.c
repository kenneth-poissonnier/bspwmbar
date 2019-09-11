/* See LICENSE file for copyright and license details. */

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <string.h>

#include "bspwmbar.h"
#include "util.h"


static char *format = "%s%s %d%%";


static void
get_state(const char *battery, char **state) {
    // Check if charging or not!
    static char spath[128];
    static int fstatus_found = -1;
    static char status[16];

    snprintf(spath, 128, "/sys/class/power_supply/%s/status", battery);

    check_file(spath, &fstatus_found);

    if (!fstatus_found || pscanf(spath, "%s%", &status) == -1) {
        *state = "";
        return;
    }

    *state = (strcmp(status, "Charging") == 0 || strcmp(status, "Full") == 0) ? " " : "";
}


void
battery(DC dc, const char *battery)
{
    static char *symbol, *state;
    static time_t prevtime;
    static uintmax_t capacity;
    static int battery_found = -1;
    static int color_number = WHITE;

    static char cpath[128];
    snprintf(cpath, 128, "/sys/class/power_supply/%s/capacity", battery);

    check_file(cpath, &battery_found);
    if (!battery_found)
        return;  // No battery found

    time_t curtime = time(NULL);
    if (curtime - prevtime < 1)
        goto DRAW_BATTERY;
    prevtime = curtime;

    if (pscanf(cpath, "%ju", &capacity) == -1)
        return;  // Cannot read capacity

    get_state(battery, &state);

    switch (capacity) {
        case 0 ... 10:
            color_number = RED;
            symbol = "!  !";  // empty symbol with expression marks
            break;
        case 11 ... 19:
            color_number = RED;
            symbol = "";  // empty symbol
            break;
        case 20 ... 39:
            // 1/3 full
            color_number = ORANGE;
            symbol = "";  // quarter full symbol
            break;
        case 40 ... 59:
            // 1/2 full
            color_number = GREEN;
            symbol = "";  // half full symbol
            break;
        case 60 ... 85:
            // 3/4 full
            color_number = LIGHT_GREEN;
            symbol = "";  // 2/3th full symboll
            break;
        case 86 ... 100:
            // full
            color_number = WHITE;
            symbol = "";  // full symbol
            break;
    }

DRAW_BATTERY:
    sprintf(buf, format, state, symbol, capacity);
    draw_colored_text(dc, color_number, buf);
}
