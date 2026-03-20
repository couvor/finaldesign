/*
 * 充电桩管理系统 - 头文件
 */

#ifndef CHARGING_STATION_H
#define CHARGING_STATION_H

#define MAX_STATIONS 100
#define MAX_RECORDS 500
#define MAX_NAME 50

/* 充电桩状态 */
typedef enum {
    IDLE = 0,      /* 空闲 */
    CHARGING = 1,  /* 充电中 */
    FAULT = 2      /* 故障 */
} StationStatus;

/* 充电桩信息结构 */
typedef struct {
    int id;
    char location[MAX_NAME];
    float power;
    float price;
    StationStatus status;
    int useCount;
    float totalIncome;
} ChargingStation;

/* 充电记录结构 */
typedef struct {
    int recordId;
    int stationId;
    char userName[MAX_NAME];
    char startTime[20];
    char endTime[20];
    float duration;
    float energy;
    float cost;
} ChargingRecord;

/* 全局变量声明 */
extern ChargingStation stations[MAX_STATIONS];
extern ChargingRecord records[MAX_RECORDS];
extern int stationCount;
extern int recordCount;

/* 函数声明 */
void initSystem();
void mainMenu();
void addStation();
void viewStations();
void updateStation();
void deleteStation();
void searchStation();
void startCharging();
void endCharging();
void viewRecords();
void statistics();
void saveData();
void loadData();
void clearInputBuffer();
void getCurrentTime(char *timeStr);

#endif
