/*
 * gen_data.c
 * 生成1000条充电桩测试数据 + 1000条充电记录测试数据
 * 地点仅限西安邮电大学三个校区:
 *   西安邮电大学东区 / 西安邮电大学西区 / 西安邮电大学雁塔校区
 * 编译: gcc -o gen_data.exe gen_data.c
 * 运行: gen_data.exe
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <direct.h>

#define DATA_DIR     "data"
#define STATION_FILE DATA_DIR "\\stations.csv"
#define RECORD_FILE  DATA_DIR "\\records.csv"

/* ===== 严格限定三个校区（location字段只允许这三个值）===== */
static const char *locations[3] = {
    "西安邮电大学东区",
    "西安邮电大学西区",
    "西安邮电大学雁塔校区"
};
#define LOC_COUNT 3

/* 各校区停车场备注 */
static const char *venues[3][5] = {
    {"东区A区停车场","东区B区停车场","东区图书馆停车场","东区体育场停车场","东区行政楼停车场"},
    {"西区1号停车场","西区2号停车场","西区宿舍停车场","西区行政楼停车场","西区实验楼停车场"},
    {"雁塔校区南门停车场","雁塔校区北门停车场","雁塔校区地下停车场","雁塔校区实验楼停车场","雁塔校区学生服务中心"}
};
#define VEN_COUNT 5

/* 各校区经纬度基准 (WGS84近似) */
static const float base_lat[3] = {34.2330f, 34.2335f, 34.2200f};
static const float base_lon[3] = {108.9270f, 108.9240f, 108.9540f};

static const char *types[3]     = {"直流快充", "交流慢充", "双枪充电"};
static const char *operators[5] = {
    "国家电网", "特来电", "星星充电", "云快充", "校园能源管理中心"
};
static const char *payments[4]  = {"微信", "支付宝", "银行卡", "校园一卡通"};
static const char *usernames[20] = {
    "张三","李四","王五","赵六","钱七","孙八","周九","吴十",
    "郑一","王二","陈大","林小","黄明","刘强","杨帆",
    "朱磊","胡涛","郭伟","何勇","高峰"
};
#define USER_COUNT 20

int main(void) {
    _mkdir(DATA_DIR);
    srand(20250320);

    /* ====== 生成1000条充电桩数据 ====== */
    FILE *fs = fopen(STATION_FILE, "w");
    if (!fs) { printf("无法创建 stations.csv\n"); return 1; }

    /* 写CSV表头 */
    fprintf(fs,
        "id,name,location,station_type,power_kw,price_per_kwh,"
        "service_fee,status,install_date,operator_name,"
        "contact_phone,total_sessions,total_energy_kwh,"
        "total_income,latitude,longitude,remark\n");

    for (int i = 1; i <= 1000; i++) {
        /* 严格均匀循环分配到三个校区：0=东区 1=西区 2=雁塔校区 */
        int   loc_i  = (i - 1) % LOC_COUNT;  /* 0,1,2,0,1,2 ... */
        int   ven_i  = rand() % VEN_COUNT;
        int   typ_i  = rand() % 3;
        int   op_i   = rand() % 5;

        /* 功率按充电类型区间随机 */
        float power;
        if      (typ_i == 0) power = (float)(60  + rand() % 121); /* 直流快充 60~180kW */
        else if (typ_i == 1) power = (float)(7   + rand() % 16);  /* 交流慢充 7~22kW  */
        else                 power = (float)(120 + rand() % 121); /* 双枪充电 120~240kW*/

        float price  = 0.80f + (rand() % 80) * 0.01f;   /* 电价 0.80~1.59 元/度 */
        float sfee   = 0.30f + (rand() % 40) * 0.01f;   /* 服务费 0.30~0.69 元/度 */

        /* 状态随机：70%空闲，20%充电中，10%故障 */
        int rnd    = rand() % 10;
        int status = (rnd < 7) ? 0 : (rnd < 9) ? 1 : 2;

        int year   = 2021 + rand() % 5;
        int month  = 1   + rand() % 12;
        int day    = 1   + rand() % 28;
        int sess   = rand() % 600;
        float energy = (float)sess * power * (0.3f + (rand() % 50) * 0.01f);
        float income = energy * (price + sfee);
        float lat    = base_lat[loc_i] + (rand() % 100) * 0.0001f;
        float lon    = base_lon[loc_i] + (rand() % 100) * 0.0001f;

        fprintf(fs,
            "%d,%s-%04d号桩,%s,%s,%.1f,%.2f,%.2f,%d,"
            "%04d-%02d-%02d,%s,029-%04d-%04d,%d,%.2f,%.2f,%.6f,%.6f,%s\n",
            i,
            types[typ_i], i,
            locations[loc_i],        /* location 严格为三个校区之一 */
            types[typ_i],
            power, price, sfee, status,
            year, month, day,
            operators[op_i],
            rand() % 10000, rand() % 10000,
            sess, energy, income,
            lat, lon,
            venues[loc_i][ven_i]     /* remark = 所在停车场 */
        );
    }
    fclose(fs);
    printf("[OK] 已生成1000条充电桩数据 -> %s\n", STATION_FILE);
    printf("      地点分布: 东区=%d台 | 西区=%d台 | 雁塔校区=%d台\n",
           334, 333, 333); /* 1000/3 分配: 334+333+333 */

    /* ====== 生成1000条充电记录数据 ====== */
    FILE *fr = fopen(RECORD_FILE, "w");
    if (!fr) { printf("无法创建 records.csv\n"); return 1; }

    fprintf(fr,
        "record_id,station_id,user_name,start_time,end_time,"
        "duration_h,energy_kwh,unit_price,service_fee,cost,"
        "finished,payment_method\n");

    for (int i = 1; i <= 1000; i++) {
        int   sid    = 1 + rand() % 1000;
        int   usr_i  = rand() % USER_COUNT;
        int   pay_i  = rand() % 4;
        int   year   = 2024 + rand() % 2;
        int   month  = 1   + rand() % 12;
        int   day    = 1   + rand() % 28;
        int   hour   = rand() % 24;
        int   minute = rand() % 60;
        float dur    = 0.5f + (rand() % 80) * 0.1f;
        float price  = 0.80f + (rand() % 80) * 0.01f;
        float sfee   = 0.30f + (rand() % 40) * 0.01f;
        float power  = 30.0f + (float)(rand() % 151);
        float energy = power * dur * (0.70f + (rand() % 30) * 0.01f);
        float cost   = energy * (price + sfee);
        int   fin    = (rand() % 10 < 9) ? 1 : 0;
        int   end_h  = (hour + (int)dur) % 24;
        int   end_m  = minute;

        fprintf(fr,
            "%d,%d,%s,"
            "%04d-%02d-%02d %02d:%02d:00,"
            "%04d-%02d-%02d %02d:%02d:00,"
            "%.2f,%.2f,%.2f,%.2f,%.2f,%d,%s\n",
            i, sid, usernames[usr_i],
            year, month, day, hour, minute,
            year, month, day, end_h, end_m,
            dur, energy, price, sfee, cost,
            fin, payments[pay_i]
        );
    }
    fclose(fr);
    printf("[OK] 已生成1000条充电记录数据 -> %s\n", RECORD_FILE);
    printf("\n数据生成完毕！充电桩地址已严格限定为西安邮电大学三个校区。\n");
    printf("直接运行 charging_system.exe 即可查看完整可视化界面。\n");
    return 0;
}
