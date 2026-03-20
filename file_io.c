/*
 * ============================================================
 *  文件名: file_io.c
 *  功  能: 数据持久化模块
 *          将内存中的充电桩、充电记录、用户数据
 *          以 CSV（逗号分隔值）文本格式读写到磁盘
 * ============================================================
 */

#include "charging.h"
#include <direct.h>    /* _mkdir() Windows专用 */

/* ensure_data_dir 定义在 utils.c，此处不重复定义 */

/* ============================================================
 *  save_stations
 *  功能: 将内存数组 g_stations[] 全量写入 stations.csv
 * ============================================================ */
void save_stations(void) {
    ensure_data_dir();

    FILE *fp = fopen(STATION_FILE, "w");
    if (!fp) { perror("[错误] 无法写入 stations.csv"); return; }

    fprintf(fp,
        "#id,name,location,station_type,power_kw,price_per_kwh,"
        "service_fee,status,install_date,operator_name,"
        "contact_phone,total_sessions,total_energy_kwh,"
        "total_income,latitude,longitude,remark\n");

    for (int i = 0; i < g_station_count; i++) {
        Station *s = &g_stations[i];
        fprintf(fp,
            "%d,%s,%s,%s,%.2f,%.2f,%.2f,%d,%s,%s,%s,%d,%.2f,%.2f,%.6f,%.6f,%s\n",
            s->id, s->name, s->location, s->station_type,
            s->power_kw, s->price_per_kwh, s->service_fee,
            (int)s->status,
            s->install_date, s->operator_name, s->contact_phone,
            s->total_sessions, s->total_energy_kwh, s->total_income,
            s->latitude, s->longitude,
            s->remark[0] ? s->remark : "无"
        );
    }
    fclose(fp);
}

/* ============================================================
 *  load_stations
 *  功能: 从 stations.csv 读取充电桩数据到 g_stations[]
 * ============================================================ */
void load_stations(void) {
    FILE *fp = fopen(STATION_FILE, "r");
    if (!fp) return;

    g_station_count   = 0;
    g_next_station_id = 1;
    char line[512];

    while (fgets(line, sizeof(line), fp)) {
        if (line[0] == '#' || line[0] == '\n' || line[0] == '\r') continue;
        if (g_station_count >= MAX_STATIONS) break;

        Station *s = &g_stations[g_station_count];
        memset(s, 0, sizeof(Station));
        int status_int = 0;

        int ret = sscanf(line,
            "%d,%63[^,],%127[^,],%31[^,],%f,%f,%f,%d,%23[^,],%63[^,],%15[^,],%d,%f,%f,%f,%f,%127[^\n]",
            &s->id, s->name, s->location, s->station_type,
            &s->power_kw, &s->price_per_kwh, &s->service_fee,
            &status_int,
            s->install_date, s->operator_name, s->contact_phone,
            &s->total_sessions, &s->total_energy_kwh, &s->total_income,
            &s->latitude, &s->longitude, s->remark
        );

        if (ret < 16) continue;

        s->status = (StationStatus)status_int;

        if (s->id >= g_next_station_id)
            g_next_station_id = s->id + 1;

        g_station_count++;
    }
    fclose(fp);
}

/* ============================================================
 *  save_records
 *  功能: 将 g_records[] 全量写入 records.csv
 * ============================================================ */
void save_records(void) {
    ensure_data_dir();

    FILE *fp = fopen(RECORD_FILE, "w");
    if (!fp) { perror("[错误] 无法写入 records.csv"); return; }

    fprintf(fp,
        "#record_id,station_id,user_name,start_time,end_time,"
        "duration_h,energy_kwh,unit_price,service_fee,cost,"
        "finished,payment_method\n");

    for (int i = 0; i < g_record_count; i++) {
        ChargeRecord *r = &g_records[i];
        fprintf(fp,
            "%d,%d,%s,%s,%s,%.4f,%.4f,%.2f,%.2f,%.2f,%d,%s\n",
            r->record_id, r->station_id, r->user_name,
            r->start_time, r->end_time,
            r->duration_h, r->energy_kwh,
            r->unit_price, r->service_fee, r->cost,
            r->finished,
            r->payment_method[0] ? r->payment_method : "未知"
        );
    }
    fclose(fp);
}

/* ============================================================
 *  load_records
 *  功能: 从 records.csv 读取充电记录到 g_records[]
 * ============================================================ */
void load_records(void) {
    FILE *fp = fopen(RECORD_FILE, "r");
    if (!fp) return;

    g_record_count   = 0;
    g_next_record_id = 1;
    char line[512];

    while (fgets(line, sizeof(line), fp)) {
        if (line[0] == '#' || line[0] == '\n' || line[0] == '\r') continue;
        if (g_record_count >= MAX_RECORDS) break;

        ChargeRecord *r = &g_records[g_record_count];
        memset(r, 0, sizeof(ChargeRecord));

        int ret = sscanf(line,
            "%d,%d,%63[^,],%23[^,],%23[^,],%f,%f,%f,%f,%f,%d,%31[^\n]",
            &r->record_id, &r->station_id, r->user_name,
            r->start_time, r->end_time,
            &r->duration_h, &r->energy_kwh,
            &r->unit_price, &r->service_fee, &r->cost,
            &r->finished, r->payment_method
        );

        if (ret < 11) continue;

        if (r->record_id >= g_next_record_id)
            g_next_record_id = r->record_id + 1;

        g_record_count++;
    }
    fclose(fp);
}

/* ============================================================
 *  save_users
 *  功能: 将 g_users[] 全量写入 users.csv
 * ============================================================ */
void save_users(void) {
    ensure_data_dir();

    FILE *fp = fopen(USER_FILE, "w");
    if (!fp) { perror("[错误] 无法写入 users.csv"); return; }

    fprintf(fp, "#username,password,role,real_name,phone,register_date\n");

    for (int i = 0; i < g_user_count; i++) {
        User *u = &g_users[i];
        fprintf(fp, "%s,%s,%d,%s,%s,%s\n",
            u->username, u->password,
            (int)u->role,
            u->real_name[0]      ? u->real_name      : "未填写",
            u->phone[0]          ? u->phone           : "未填写",
            u->register_date[0]  ? u->register_date   : "2025-01-01 00:00:00"
        );
    }
    fclose(fp);
}

/* ============================================================
 *  load_users
 *  功能: 从 users.csv 读取用户数据到 g_users[]
 *  特殊处理: 文件不存在时自动创建默认管理员
 * ============================================================ */
void load_users(void) {
    FILE *fp = fopen(USER_FILE, "r");
    if (!fp) {
        g_user_count = 1;
        strcpy(g_users[0].username,  "admin");
        strcpy(g_users[0].password,  "admin123");
        strcpy(g_users[0].real_name, "系统管理员");
        strcpy(g_users[0].phone,     "10000000000");
        g_users[0].role = ROLE_ADMIN;
        now_time_str(g_users[0].register_date);
        save_users();
        printf("[提示] 首次运行，已创建默认管理员账户 admin/admin123\n");
        return;
    }

    g_user_count = 0;
    char line[256];

    while (fgets(line, sizeof(line), fp)) {
        if (line[0] == '#' || line[0] == '\n' || line[0] == '\r') continue;
        if (g_user_count >= MAX_USERS) break;

        User *u = &g_users[g_user_count];
        memset(u, 0, sizeof(User));
        int role_int = 1;

        int ret = sscanf(line,
            "%63[^,],%63[^,],%d,%63[^,],%15[^,],%23[^\n]",
            u->username, u->password, &role_int,
            u->real_name, u->phone, u->register_date
        );

        if (ret < 3) continue;
        u->role = (UserRole)role_int;
        g_user_count++;
    }
    fclose(fp);
}

/* ============================================================
 *  save_all / load_all
 * ============================================================ */
void save_all(void) {
    save_stations();
    save_records();
    save_users();
    printf("[√] 所有数据已保存到 data\\ 目录下的 CSV 文件。\n");
}

void load_all(void) {
    ensure_data_dir();
    load_users();
    load_stations();
    load_records();
}
