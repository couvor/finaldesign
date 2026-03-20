/*
 * ============================================================
 *  文件名: station.c
 *  功  能: 充电桩管理模块
 *          实现充电桩的增加、列表、修改、删除、查询功能
 * ============================================================
 */

#include "charging.h"

/* -------------------------------------------------------
 *  print_station_header  (内部函数)
 *  打印充电桩列表的列标题行和分隔线
 * ------------------------------------------------------- */
static void print_station_header(void) {
    printf("%-4s %-14s %-16s %-10s %-7s %-7s %-8s %-6s %-9s %-9s\n",
           "ID","名称","位置","类型",
           "功率kW","电价","状态","次数",
           "累计电量","累计收入");
    printf("--------------------------------------------------------------------"
           "-----------------------\n");
}

/* -------------------------------------------------------
 *  print_station_row  (内部函数)
 *  打印单条充电桩信息行，与标题列宽对齐
 * ------------------------------------------------------- */
static void print_station_row(const Station *s) {
    printf("%-4d %-14s %-16s %-10s %-7.1f %-7.2f %-8s %-6d %-9.2f %-9.2f\n",
           s->id, s->name, s->location, s->station_type,
           s->power_kw, s->price_per_kwh, status_str(s->status),
           s->total_sessions, s->total_energy_kwh, s->total_income);
}

/* ============================================================
 *  station_add
 *  功能: 交互式新增一个充电桩
 *  流程: 检查容量 -> 逐字段输入 -> 分配ID -> 保存CSV
 * ============================================================ */
void station_add(void) {
    if (g_station_count >= MAX_STATIONS) {
        printf("充电桩数量已达上限（%d）。\n", MAX_STATIONS);
        return;
    }
    print_title("添加充电桩");

    Station s;
    memset(&s, 0, sizeof(s));        /* 清零防止脏数据 */
    s.id     = g_next_station_id++;  /* 分配自增ID */
    s.status = STATUS_IDLE;          /* 新桩初始为空闲状态 */

    printf("名称: ");
    fgets(s.name, NAME_LEN, stdin);
    s.name[strcspn(s.name, "\n")] = '\0';

    printf("位置: ");
    fgets(s.location, LOC_LEN, stdin);
    s.location[strcspn(s.location, "\n")] = '\0';

    /* 充电类型选择 */
    printf("类型: 1.直流快充  2.交流慢充  3.双枪充电\n选择: ");
    int t; scanf("%d", &t); clear_input();
    if      (t == 1) strcpy(s.station_type, "直流快充");
    else if (t == 2) strcpy(s.station_type, "交流慢充");
    else             strcpy(s.station_type, "双枪充电");

    printf("额定功率(kW): ");  scanf("%f", &s.power_kw);      clear_input();
    printf("电价(元/度): ");   scanf("%f", &s.price_per_kwh); clear_input();
    printf("服务费(元/度): "); scanf("%f", &s.service_fee);   clear_input();

    printf("安装日期(yyyy-mm-dd): ");
    fgets(s.install_date, TIME_LEN, stdin);
    s.install_date[strcspn(s.install_date, "\n")] = '\0';
    if (s.install_date[0] == '\0') strcpy(s.install_date, "2025-01-01");

    printf("运营商名称: ");
    fgets(s.operator_name, NAME_LEN, stdin);
    s.operator_name[strcspn(s.operator_name, "\n")] = '\0';

    printf("联系电话: ");
    fgets(s.contact_phone, PHONE_LEN, stdin);
    s.contact_phone[strcspn(s.contact_phone, "\n")] = '\0';

    printf("纬度(可填0): "); scanf("%f", &s.latitude);  clear_input();
    printf("经度(可填0): "); scanf("%f", &s.longitude); clear_input();

    printf("备注(回车跳过): ");
    fgets(s.remark, LOC_LEN, stdin);
    s.remark[strcspn(s.remark, "\n")] = '\0';

    g_stations[g_station_count++] = s;  /* 写入内存数组 */
    save_stations();                     /* 立即保存到 CSV */
    printf("[√] 充电桩添加成功！ID=%d\n", s.id);
}

/* ============================================================
 *  station_list  —— 列出全部充电桩
 * ============================================================ */
void station_list(void) {
    print_title("充电桩列表");
    if (g_station_count == 0) { printf("暂无数据。\n"); return; }
    print_station_header();
    for (int i = 0; i < g_station_count; i++)
        print_station_row(&g_stations[i]);
    printf("共 %d 条\n", g_station_count);
}

/* ============================================================
 *  station_update  —— 修改指定充电桩某一字段
 * ============================================================ */
void station_update(void) {
    print_title("修改充电桩信息");
    int id;
    printf("请输入充电桩ID: "); scanf("%d", &id); clear_input();

    int idx = find_station_idx(id);
    if (idx < 0) { printf("未找到ID=%d的充电桩。\n", id); return; }

    Station *s = &g_stations[idx];  /* 获取指针，修改直接影响原数组 */
    printf("当前信息:\n"); print_station_header(); print_station_row(s);

    printf("\n修改字段:\n");
    printf(" 1.名称  2.位置  3.类型  4.功率  5.电价  6.服务费  7.状态  0.取消\n");
    printf("选择: ");
    int op; scanf("%d", &op); clear_input();

    switch (op) {
        case 1:
            printf("新名称: ");
            fgets(s->name, NAME_LEN, stdin);
            s->name[strcspn(s->name, "\n")] = '\0';
            break;
        case 2:
            printf("新位置: ");
            fgets(s->location, LOC_LEN, stdin);
            s->location[strcspn(s->location, "\n")] = '\0';
            break;
        case 3:
            printf("类型: 1.直流快充  2.交流慢充  3.双枪充电\n选择: ");
            int t; scanf("%d", &t); clear_input();
            if      (t == 1) strcpy(s->station_type, "直流快充");
            else if (t == 2) strcpy(s->station_type, "交流慢充");
            else             strcpy(s->station_type, "双枪充电");
            break;
        case 4:
            printf("新功率(kW): "); scanf("%f", &s->power_kw); clear_input();
            break;
        case 5:
            printf("新电价(元/度): "); scanf("%f", &s->price_per_kwh); clear_input();
            break;
        case 6:
            printf("新服务费(元/度): "); scanf("%f", &s->service_fee); clear_input();
            break;
        case 7: {
            printf("状态: 0.空闲  1.充电中  2.故障\n选择: ");
            int st; scanf("%d", &st); clear_input();
            if (st >= 0 && st <= 2) s->status = (StationStatus)st;
            else { printf("无效状态。\n"); return; }
            break;
        }
        case 0: return;
        default: printf("无效选项。\n"); return;
    }
    save_stations();  /* 修改后立即保存到 CSV */
    printf("[√] 修改成功。\n");
}

/* ============================================================
 *  station_delete  —— 删除指定充电桩
 *  限制: 充电中的桩不允许删除（防止数据不一致）
 * ============================================================ */
void station_delete(void) {
    print_title("删除充电桩");
    int id;
    printf("请输入要删除的充电桩ID: "); scanf("%d", &id); clear_input();

    int idx = find_station_idx(id);
    if (idx < 0) { printf("未找到ID=%d的充电桩。\n", id); return; }

    /* 充电中的桩禁止删除，防止产生孤立的进行中记录 */
    if (g_stations[idx].status == STATUS_CHARGING) {
        printf("该充电桩正在充电中，无法删除！\n");
        return;
    }

    printf("确认删除 [%s]？(y/n): ", g_stations[idx].name);
    char c; scanf("%c", &c); clear_input();
    if (c != 'y' && c != 'Y') { printf("操作已取消。\n"); return; }

    /* 将后续元素向前移动，覆盖被删除的槽位 */
    for (int i = idx; i < g_station_count - 1; i++)
        g_stations[i] = g_stations[i + 1];
    g_station_count--;

    save_stations();  /* 立即同步到 CSV */
    printf("[√] 删除成功。\n");
}

/* ============================================================
 *  station_search  —— 多条件查询充电桩
 *  支持: 按名称关键字 / 按位置关键字 / 按ID / 按状态 / 组合查询
 * ============================================================ */
void station_search(void) {
    print_title("查询充电桩");
    printf(" 1.按名称关键字\n 2.按位置关键字\n 3.按ID\n"
           " 4.按状态\n 5.组合(位置+状态)\n选择: ");
    int op; scanf("%d", &op); clear_input();

    int found = 0;
    print_station_header();

    if (op == 1) {
        /* 按名称关键字模糊搜索 */
        char key[NAME_LEN];
        printf("名称关键字: "); fgets(key, NAME_LEN, stdin);
        key[strcspn(key, "\n")] = '\0';
        for (int i = 0; i < g_station_count; i++)
            if (strstr(g_stations[i].name, key))
                { print_station_row(&g_stations[i]); found++; }

    } else if (op == 2) {
        /* 按位置关键字模糊搜索 */
        char key[LOC_LEN];
        printf("位置关键字: "); fgets(key, LOC_LEN, stdin);
        key[strcspn(key, "\n")] = '\0';
        for (int i = 0; i < g_station_count; i++)
            if (strstr(g_stations[i].location, key))
                { print_station_row(&g_stations[i]); found++; }

    } else if (op == 3) {
        /* 按精确ID查找 */
        int id; printf("充电桩ID: "); scanf("%d", &id); clear_input();
        int idx = find_station_idx(id);
        if (idx >= 0) { print_station_row(&g_stations[idx]); found = 1; }

    } else if (op == 4) {
        /* 按运行状态过滤 */
        printf("状态: 0.空闲  1.充电中  2.故障\n选择: ");
        int st; scanf("%d", &st); clear_input();
        for (int i = 0; i < g_station_count; i++)
            if (g_stations[i].status == (StationStatus)st)
                { print_station_row(&g_stations[i]); found++; }

    } else if (op == 5) {
        /* 组合查询：位置关键字 AND 状态 */
        char key[LOC_LEN]; int st;
        printf("位置关键字: "); fgets(key, LOC_LEN, stdin);
        key[strcspn(key, "\n")] = '\0';
        printf("状态: 0.空闲  1.充电中  2.故障\n选择: ");
        scanf("%d", &st); clear_input();
        for (int i = 0; i < g_station_count; i++)
            if (strstr(g_stations[i].location, key)
                && g_stations[i].status == (StationStatus)st)
                { print_station_row(&g_stations[i]); found++; }
    } else {
        printf("无效选项。\n"); return;
    }

    if (!found) printf("未找到匹配记录。\n");
    else        printf("共找到 %d 条。\n", found);
}

/* ============================================================
 *  station_menu  —— 充电桩管理子菜单循环
 * ============================================================ */
void station_menu(void) {
    int op;
    while (1) {
        print_title("充电桩管理");
        printf(" 1. 添加充电桩\n");
        printf(" 2. 查看全部\n");
        printf(" 3. 修改信息\n");
        printf(" 4. 删除充电桩\n");
        printf(" 5. 查询充电桩\n");
        printf(" 0. 返回主菜单\n");
        printf("请选择: ");

        if (scanf("%d", &op) != 1) { clear_input(); continue; }
        clear_input();
        if (op == 0) break;

        switch (op) {
            case 1: station_add();    break;
            case 2: station_list();   break;
            case 3: station_update(); break;
            case 4: station_delete(); break;
            case 5: station_search(); break;
            default: printf("无效选项。\n");
        }
        press_any_key();
    }
}
