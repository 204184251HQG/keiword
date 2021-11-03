#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <linux/termios.h>

#include "log/keilog.h"

int ioctl(int fd, unsigned long request, ...);

#define MAX_WIN_W 200
int32_t get_win_width()
{
    int32_t width, height;
    struct winsize ws;
    memset(&ws, 0x00, sizeof(struct winsize));
    ioctl(STDIN_FILENO, TIOCGWINSZ, &ws);
    width = ws.ws_col;
    height = ws.ws_row;
    // KLOG_D("%d ", width);
    // KLOG_D("%d\n", height);
    return width;
}
int32_t get_win_remain_width(int32_t offset)
{
    int32_t remain_w = get_win_width() - offset;
    remain_w = remain_w < 0 ? 0 : remain_w;
    return remain_w;
}

int32_t print_proc_bar(int32_t persent, const char *color)
{
    static char tmp[5] = {'-', '\\', '|', '/', '\0'};
    static uint32_t count = 0;
    static char str[MAX_WIN_W] = {0};
    uint32_t remain_len = get_win_remain_width(16);
    uint32_t print_pos = (persent * remain_len) / 100;
    print_pos = print_pos > remain_len ? (remain_len) : print_pos;
    memset(str, '#', print_pos);
    str[print_pos] = '\0';

    KLOG_I("%s"
           "[%3d%%] %c %s\r" NONE,
           color, persent, tmp[count++ % 4], str);
    fflush(stdout);
    return 0;
}