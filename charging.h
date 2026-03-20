/*
 * ============================================================
 *  文件名: charging.h
 *  功  能: 充电桩管理系统 —— 全局头文件
 *          定义所有模块共用的宏、枚举、结构体及函数声明
 *  存储方式: 所有持久化数据均以 CSV（逗号分隔值）文本文件保存，
 *            不再使用二进制 .dat 文件，便于人工查阅和外部工具处理
 *  编码规范: UTF-8，Windows 控制台需设置 CP65001
 * ============================================================
 */

#ifndef CHARGING_H
#define CHARGING_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* ============================================================
 *  容量宏 —— 内存中最多同时持有的记录条数
 *  如需扩展，仅修改此处数字即可，无需改动其他逻辑
 * ============================================================ */
#define MAX_STATIONS   200    /* 充电桩最大数量 */
#define MAX_RECORDS    2000   /* 充电记录最大条数 */
#define MAX_USERS      20     /* 系统用户最大数量 */

/* ============================================================
 *  字符串缓冲区长度宏
 *  所有 char 数组声明均使用这些宏，禁止直接写魔法数字
 * ============================================================ */
#define NAME_LEN   64    /* 名称/用户名缓冲区长度（含终止符）*/
#define LOC_LEN    128   /* 位置描述缓冲区长度 */
#define TIME_LEN   24    /* 时间字符串长度，格式 yyyy-mm-dd hh:mm:ss */
#define PASS_LEN   64    /* 密码缓冲区长度 */
#define TYPE_LEN   32    /* 充电类型字符串长度 */
#define PHONE_LEN  16    /* 联系电话长度 */
#define PAY_LEN    32    /* 支付方式字符串长度 */

/* ============================================================
 *  CSV 数据文件路径宏
 *  所有文件统一存放在程序工作目录下的 data\ 子目录
 *  修改存储路径时，仅需更改此处
 * ============================================================ */
#define DATA_DIR          "data"
#define STATION_FILE      DATA_DIR "\\stations.csv"   /* 充电桩基础信息 */
#define RECORD_FILE       DATA_DIR "\\records.csv"    /* 充电记录流水   */
#define USER_FILE         DATA_DIR "\\users.csv"      /* 系统用户账户   */
#define TESTDATA_FILE     DATA_DIR "\\test_data.csv"  /* 100条测试数据  */

/* ============================================================
 *  充电桩状态枚举
 *  STATUS_IDLE     : 空闲，可立即接受充电请求
 *  STATUS_CHARGING : 充电中，正在为车辆服务
 *  STATUS_FAULT    : 故障，暂停对外服务
 * ============================================================ */
typedef enum {
    STATUS_IDLE     = 0,   /* 空闲 */
    STATUS_CHARGING = 1,   /* 充电中 */
    STATUS_FAULT    = 2    /* 故障 */
} StationStatus;

/* ============================================================
 *  用户角色枚举
 *  ROLE_ADMIN : 管理员，可执行所有操作（含用户管理）
 *  ROLE_USER  : 普通用户，只能进行充电业务操作
 * ============================================================ */
typedef enum {
    ROLE_ADMIN = 0,   /* 管理员 */
    ROLE_USER  = 1    /* 普通用户 */
} UserRole;

/* ============================================================
 *  充电桩结构体
 *  对应 CSV 文件 stations.csv 中的每一行数据
 *  字段顺序与 CSV 列顺序完全一致，修改时两处同步更新
 * ============================================================ */
typedef struct {
    int           id;                     /* 充电桩唯一编号（自增整数）*/
    char          name[NAME_LEN];         /* 充电桩名称，如"A区01号桩"*/
    char          location[LOC_LEN];      /* 物理位置描述，如"北京市朝阳区xx停车场"*/
    char          station_type[TYPE_LEN]; /* 充电类型：直流快充 / 交流慢充 / 双枪充电 */
    float         power_kw;              /* 额定充电功率，单位：千瓦（kW）*/
    float         price_per_kwh;         /* 电费单价，单位：元/度（kWh）*/
    float         service_fee;           /* 服务费单价，单位：元/度 */
    StationStatus status;                /* 当前运行状态（见 StationStatus 枚举）*/
    char          install_date[TIME_LEN];/* 安装日期，格式：yyyy-mm-dd */
    char          operator_name[NAME_LEN];/* 运营商名称 */
    char          contact_phone[PHONE_LEN];/* 运维联系电话 */
    int           total_sessions;        /* 累计充电次数（整型）*/
    float         total_energy_kwh;      /* 累计充电总电量，单位：度（kWh）*/
    float         total_income;          /* 累计总收入，单位：元 */
    float         latitude;             /* 地理纬度（WGS84，保留6位小数）*/
    float         longitude;            /* 地理经度（WGS84，保留6位小数）*/
    char          remark[LOC_LEN];       /* 备注信息，可为空 */
} Station;

/* ============================================================
 *  充电记录结构体
 *  每次充电会话产生一条记录，对应 records.csv 中的一行
 * ============================================================ */
typedef struct {
    int   record_id;               /* 记录唯一编号（自增整数）*/
    int   station_id;              /* 关联的充电桩编号 */
    char  user_name[NAME_LEN];     /* 充电用户名 */
    char  start_time[TIME_LEN];    /* 充电开始时间，格式：yyyy-mm-dd hh:mm:ss */
    char  end_time[TIME_LEN];      /* 充电结束时间，格式：yyyy-mm-dd hh:mm:ss */
    float duration_h;             /* 充电时长，单位：小时（h）*/
    float energy_kwh;             /* 本次充电电量，单位：度（kWh）*/
    float unit_price;             /* 本次结算电价，元/度 */
    float service_fee;            /* 本次服务费单价，元/度 */
    float cost;                   /* 本次总费用，单位：元 */
    int   finished;               /* 会话状态：0=进行中，1=已完成，2=已中断 */
    char  payment_method[PAY_LEN];/* 支付方式：微信/支付宝/银行卡/APP余额 */
} ChargeRecord;

/* ============================================================
 *  用户账户结构体
 *  对应 users.csv 中的每一行
 * ============================================================ */
typedef struct {
    char     username[NAME_LEN];   /* 登录用户名（唯一）*/
    char     password[PASS_LEN];   /* 登录密码（明文存储，仅供教学演示）*/
    UserRole role;                 /* 用户角色：0=管理员，1=普通用户 */
    char     real_name[NAME_LEN];  /* 真实姓名 */
    char     phone[PHONE_LEN];     /* 手机号码 */
    char     register_date[TIME_LEN]; /* 注册日期 */
} User;

/* ============================================================
 *  全局数据数组及计数器
 *  所有模块通过 extern 声明访问，定义在 main.c 中
 * ============================================================ */
extern Station      g_stations[MAX_STATIONS];   /* 充电桩数据数组 */
extern ChargeRecord g_records[MAX_RECORDS];     /* 充电记录数组   */
extern User         g_users[MAX_USERS];         /* 用户账户数组   */
extern int          g_station_count;            /* 当前充电桩数量 */
extern int          g_record_count;             /* 当前记录条数   */
extern int          g_user_count;               /* 当前用户数量   */
extern int          g_next_station_id;          /* 下一个可用桩ID */
extern int          g_next_record_id;           /* 下一个可用记录ID */
extern User        *g_current_user;             /* 指向当前登录用户 */

/* ============================================================
 *  函数声明 —— utils.c
 *  通用工具函数，供所有模块调用
 * ============================================================ */
void        clear_input(void);                          /* 清空标准输入缓冲区 */
void        now_time_str(char out[TIME_LEN]);            /* 获取当前时间字符串 */
const char *status_str(StationStatus s);                /* 状态枚举→中文字符串 */
int         find_station_idx(int id);                   /* 按ID查找充电桩下标 */
int         find_active_record_by_station(int sid);     /* 查找桩的进行中记录 */
void        press_any_key(void);                        /* 等待用户按Enter继续 */
void        print_title(const char *title);             /* 打印带边框的标题栏 */
void        ensure_data_dir(void);                      /* 确保data目录存在 */

/* ============================================================
 *  函数声明 —— file_io.c
 *  所有 I/O 均使用 CSV 文本格式，每行一条记录，字段间逗号分隔
 * ============================================================ */
void save_stations(void);   /* 将内存中的充电桩数组写入 stations.csv */
void load_stations(void);   /* 从 stations.csv 读取充电桩数据到内存 */
void save_records(void);    /* 将充电记录数组写入 records.csv */
void load_records(void);    /* 从 records.csv 读取充电记录到内存 */
void save_users(void);      /* 将用户数组写入 users.csv */
void load_users(void);      /* 从 users.csv 读取用户数据到内存 */
void save_all(void);        /* 一次性保存三张表的全部数据 */
void load_all(void);        /* 启动时一次性加载三张表的全部数据 */

/* ============================================================
 *  函数声明 —— station.c
 *  充电桩的增删改查和菜单入口
 * ============================================================ */
void station_add(void);     /* 新增一个充电桩 */
void station_list(void);    /* 列出全部充电桩 */
void station_update(void);  /* 修改某个充电桩的信息 */
void station_delete(void);  /* 删除某个充电桩 */
void station_search(void);  /* 多条件查询充电桩 */
void station_menu(void);    /* 充电桩管理子菜单循环 */

/* ============================================================
 *  函数声明 —— record.c
 *  充电会话的开始/结束/查询
 * ============================================================ */
void record_start(void);    /* 开始一次充电会话 */
void record_end(void);      /* 结束（停止）当前充电会话 */
void record_list(void);     /* 列出所有充电记录 */
void record_search(void);   /* 按条件查询充电记录 */
void record_menu(void);     /* 充电业务子菜单循环 */

/* ============================================================
 *  函数声明 —— stats.c
 *  统计汇总、排序和报表打印
 * ============================================================ */
void stats_summary(void);       /* 综合统计：总收入/总电量/桩状态分布 */
void stats_by_location(void);   /* 按位置分组统计 */
void sort_stations_menu(void);  /* 充电桩多字段排序 */
void sort_records_menu(void);   /* 充电记录多字段排序 */
void print_report(void);        /* 打印完整数据报表 */
void stats_menu(void);          /* 统计报表子菜单循环 */

/* ============================================================
 *  函数声明 —— auth.c
 *  用户登录、密码管理和用户管理
 * ============================================================ */
int  login(void);               /* 用户登录，成功返回1，失败返回0 */
void change_password(void);     /* 修改当前用户密码 */
void user_manage_menu(void);    /* 管理员用户管理子菜单 */

#endif /* CHARGING_H */ 