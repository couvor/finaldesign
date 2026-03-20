#include "charging.h"

static void print_record_header(void) {
    printf("%-4s %-4s %-10s %-19s %-19s %-7s %-8s %-8s %-6s\n",
           "RID","SID","用户","开始时间","结束时间","时长h","电量度","费用元","状态");
    printf("-------------------------------------------------------------------------------------------\n");
}

static void print_record_row(const ChargeRecord *r) {
    printf("%-4d %-4d %-10s %-19s %-19s %-7.2f %-8.2f %-8.2f %-6s\n",
           r->record_id, r->station_id, r->user_name,
           r->start_time, r->finished ? r->end_time : "进行中",
           r->duration_h, r->energy_kwh, r->cost,
           r->finished ? "完成" : "进行中");
}

/* ---------- 开始充电 ---------- */
void record_start(void) {
    if (g_record_count >= MAX_RECORDS) { printf("记录已满。\n"); return; }
    print_title("开始充电");

    int sid;
    printf("输入充电桩ID: "); scanf("%d", &sid); clear_input();
    int sidx = find_station_idx(sid);
    if (sidx < 0) { printf("充电桩不存在。\n"); return; }
    if (g_stations[sidx].status != STATUS_IDLE) {
        printf("该桩状态为[%s]，无法充电。\n",
               status_str(g_stations[sidx].status)); return;
    }

    ChargeRecord r;
    memset(&r, 0, sizeof(r));
    r.record_id  = g_next_record_id++;
    r.station_id = sid;
    r.finished   = 0;

    printf("用户名: "); fgets(r.user_name, NAME_LEN, stdin);
    r.user_name[strcspn(r.user_name, "\n")] = '\0';
    now_time_str(r.start_time);
    strcpy(r.end_time, "-");

    g_records[g_record_count++] = r;
    g_stations[sidx].status = STATUS_CHARGING;
    save_records();
    save_stations();
    printf("[OK] 充电开始。记录ID=%d，开始=%s\n", r.record_id, r.start_time);
}

/* ---------- 结束充电 ---------- */
void record_end(void) {
    print_title("结束充电");
    int sid;
    printf("输入充电桩ID: "); scanf("%d", &sid); clear_input();
    int sidx = find_station_idx(sid);
    if (sidx < 0) { printf("充电桩不存在。\n"); return; }
    if (g_stations[sidx].status != STATUS_CHARGING) {
        printf("该桩当前状态为[%s]，不是充电中。\n",
               status_str(g_stations[sidx].status)); return;
    }

    int ridx = find_active_record_by_station(sid);
    if (ridx < 0) { printf("未找到进行中的充电记录。\n"); return; }

    ChargeRecord *r = &g_records[ridx];
    Station      *s = &g_stations[sidx];

    printf("输入充电时长(小时): "); scanf("%f", &r->duration_h); clear_input();
    if (r->duration_h <= 0) { printf("时长必须>0。\n"); return; }

    r->energy_kwh = s->power_kw * r->duration_h;
    r->cost       = r->energy_kwh * s->price_per_kwh;
    now_time_str(r->end_time);
    r->finished   = 1;

    s->status            = STATUS_IDLE;
    s->total_sessions   += 1;
    s->total_energy_kwh += r->energy_kwh;
    s->total_income     += r->cost;

    save_records();
    save_stations();
    printf("[OK] 充电结束。\n");
    printf("     用户:%-10s  时长:%.2fh  电量:%.2f度  费用:%.2f元\n",
           r->user_name, r->duration_h, r->energy_kwh, r->cost);
}

/* ---------- 查看全部记录 ---------- */
void record_list(void) {
    print_title("充电记录列表");
    if (g_record_count == 0) { printf("暂无记录。\n"); return; }
    print_record_header();
    for (int i = 0; i < g_record_count; i++)
        print_record_row(&g_records[i]);
    printf("共 %d 条。\n", g_record_count);
}

/* ---------- 查询记录（简单+组合） ---------- */
void record_search(void) {
    print_title("查询充电记录");
    printf("1.按记录ID  2.按充电桩ID  3.按用户名  4.组合(用户名+桩ID)\n选择: ");
    int op; scanf("%d", &op); clear_input();

    int found = 0;
    print_record_header();

    if (op == 1) {
        int rid; printf("记录ID: "); scanf("%d", &rid); clear_input();
        for (int i = 0; i < g_record_count; i++)
            if (g_records[i].record_id == rid) {
                print_record_row(&g_records[i]); found++;
            }
    } else if (op == 2) {
        int sid; printf("充电桩ID: "); scanf("%d", &sid); clear_input();
        for (int i = 0; i < g_record_count; i++)
            if (g_records[i].station_id == sid) {
                print_record_row(&g_records[i]); found++;
            }
    } else if (op == 3) {
        char uname[NAME_LEN];
        printf("用户名关键字: "); fgets(uname, NAME_LEN, stdin);
        uname[strcspn(uname, "\n")] = '\0';
        for (int i = 0; i < g_record_count; i++)
            if (strstr(g_records[i].user_name, uname)) {
                print_record_row(&g_records[i]); found++;
            }
    } else if (op == 4) {
        char uname[NAME_LEN]; int sid;
        printf("用户名关键字: "); fgets(uname, NAME_LEN, stdin);
        uname[strcspn(uname, "\n")] = '\0';
        printf("充电桩ID: "); scanf("%d", &sid); clear_input();
        for (int i = 0; i < g_record_count; i++)
            if (g_records[i].station_id == sid &&
                strstr(g_records[i].user_name, uname)) {
                print_record_row(&g_records[i]); found++;
            }
    } else {
        printf("无效选项。\n"); return;
    }

    if (!found) printf("无匹配结果。\n");
    else        printf("共 %d 条。\n", found);
}

/* ---------- 充电记录子菜单 ---------- */
void record_menu(void) {
    int op;
    while (1) {
        print_title("充电业务管理");
        printf("1. 开始充电\n");
        printf("2. 结束充电\n");
        printf("3. 查看全部记录\n");
        printf("4. 查询记录\n");
        printf("0. 返回主菜单\n");
        printf("请选择: ");
        if (scanf("%d", &op) != 1) { clear_input(); continue; }
        clear_input();
        if (op == 0) break;
        switch (op) {
            case 1: record_start();  break;
            case 2: record_end();    break;
            case 3: record_list();   break;
            case 4: record_search(); break;
            default: printf("无效选项。\n");
        }
        press_any_key();
    }
}
