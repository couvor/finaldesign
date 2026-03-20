/*
 * ============================================================
 *  文件名: main.c
 *  功  能: 程序入口与主菜单
 *          负责初始化控制台编码、加载数据、用户登录、
 *          以及主菜单的循环分发
 *  编译命令（MinGW）:
 *    gcc main.c file_io.c station.c record.c stats.c auth.c utils.c
 *        -o charging_system.exe -Wall -lm
 * ============================================================
 */

#include "charging.h"
#include <windows.h>   /* SetConsoleOutputCP / SetConsoleCP */

/* ============================================================
 *  全局数据定义
 *  所有模块通过 extern 声明使用，实际内存在此分配
 * ============================================================ */
Station      g_stations[MAX_STATIONS];   /* 充电桩信息数组 */
ChargeRecord g_records[MAX_RECORDS];     /* 充电记录数组   */
User         g_users[MAX_USERS];         /* 用户账户数组   */
int          g_station_count   = 0;      /* 当前充电桩数量 */
int          g_record_count    = 0;      /* 当前充电记录数量 */
int          g_user_count      = 0;      /* 当前用户数量   */
int          g_next_station_id = 1;      /* 下一个新充电桩的ID（自增）*/
int          g_next_record_id  = 1;      /* 下一条记录的ID（自增）*/
User        *g_current_user    = NULL;   /* 指向当前登录用户，NULL表示未登录 */

/* ============================================================
 *  main_menu
 *  功能: 登录成功后的主菜单循环
 *  说明: 根据当前用户角色动态显示「用户管理」选项
 *        选择 0 时保存所有数据并退出程序
 * ============================================================ */
static void main_menu(void) {
    int op;
    while (1) {
        print_title("充电桩管理系统  主菜单");
        printf(" 1. 充电桩管理\n");
        printf(" 2. 充电业务管理\n");
        printf(" 3. 统计与报表\n");
        printf(" 4. 修改密码\n");

        /* 仅管理员可见「用户管理」选项 */
        if (g_current_user && g_current_user->role == ROLE_ADMIN)
            printf(" 5. 用户管理 (管理员)\n");

        printf(" 0. 保存并退出\n");
        printf("请选择: ");

        if (scanf("%d", &op) != 1) { clear_input(); continue; }
        clear_input();

        switch (op) {
            case 1: station_menu();    break;   /* 进入充电桩管理子菜单 */
            case 2: record_menu();     break;   /* 进入充电业务子菜单   */
            case 3: stats_menu();      break;   /* 进入统计报表子菜单   */
            case 4:
                change_password();
                press_any_key();
                break;
            case 5:
                /* 普通用户选5无效 */
                if (g_current_user && g_current_user->role == ROLE_ADMIN)
                    user_manage_menu();
                else
                    printf("无效选项。\n");
                break;
            case 0:
                save_all();   /* 退出前保存所有数据到 CSV */
                printf("感谢使用充电桩管理系统，再见！\n");
                exit(0);      /* 正常退出，返回值 0 */
            default:
                printf("无效选项，请重新输入。\n");
        }
    }
}

/* ============================================================
 *  main
 *  功能: 程序入口
 *  流程:
 *    1. 设置控制台为 UTF-8（解决 Windows 中文乱码）
 *    2. 打印欢迎横幅
 *    3. 从 CSV 文件加载所有数据到内存
 *    4. 用户登录验证
 *    5. 进入主菜单循环
 * ============================================================ */
int main(void) {
    /* 设置控制台输入/输出编码为 UTF-8（代码页 65001）
     * 不设置则中文字符在 Windows cmd 中显示为乱码 */
    SetConsoleOutputCP(65001);
    SetConsoleCP(65001);

    /* 欢迎横幅 */
    printf("\n");
    printf("  ================================================\n");
    printf("  |       充电桩管理系统  v3.0 (CSV版)          |\n");
    printf("  |    Charging Station Management System       |\n");
    printf("  |   数据存储: CSV文本格式 (data\\ 目录)        |\n");
    printf("  ================================================\n\n");

    /* 从 CSV 文件加载数据 */
    load_all();
    printf("已加载: 充电桩 %d 条 | 充电记录 %d 条 | 用户 %d 个\n",
           g_station_count, g_record_count, g_user_count);
    printf("数据目录: data\\ (stations.csv / records.csv / users.csv)\n\n");

    /* 用户登录 */
    if (!login()) return 1;

    /* 进入主菜单循环（永不返回，除非用户选 0 退出）*/
    main_menu();
    return 0;
}
