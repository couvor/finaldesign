#include "charging.h"

/* === 排序比较函数 === */
static int cmp_st_income_asc(const void *a,const void *b){
    float d=((Station*)a)->total_income-((Station*)b)->total_income;
    return (d>0)-(d<0);
}
static int cmp_st_income_desc(const void *a,const void *b){return cmp_st_income_asc(b,a);}
static int cmp_st_sess_asc(const void *a,const void *b){
    return ((Station*)a)->total_sessions-((Station*)b)->total_sessions;
}
static int cmp_st_sess_desc(const void *a,const void *b){return cmp_st_sess_asc(b,a);}
static int cmp_st_power_asc(const void *a,const void *b){
    float d=((Station*)a)->power_kw-((Station*)b)->power_kw;
    return (d>0)-(d<0);
}
static int cmp_st_power_desc(const void *a,const void *b){return cmp_st_power_asc(b,a);}

static int cmp_rec_cost_asc(const void *a,const void *b){
    float d=((ChargeRecord*)a)->cost-((ChargeRecord*)b)->cost;
    return (d>0)-(d<0);
}
static int cmp_rec_cost_desc(const void *a,const void *b){return cmp_rec_cost_asc(b,a);}
static int cmp_rec_energy_asc(const void *a,const void *b){
    float d=((ChargeRecord*)a)->energy_kwh-((ChargeRecord*)b)->energy_kwh;
    return (d>0)-(d<0);
}
static int cmp_rec_energy_desc(const void *a,const void *b){return cmp_rec_energy_asc(b,a);}
static int cmp_rec_dur_asc(const void *a,const void *b){
    float d=((ChargeRecord*)a)->duration_h-((ChargeRecord*)b)->duration_h;
    return (d>0)-(d<0);
}
static int cmp_rec_dur_desc(const void *a,const void *b){return cmp_rec_dur_asc(b,a);}

/* === 综合统计 === */
void stats_summary(void){
    print_title("综合统计");
    if(g_station_count==0){printf("暂无数据。\n");return;}

    int idle=0,chg=0,fault=0,sum_sess=0,finished=0;
    float sum_inc=0,sum_eng=0;

    for(int i=0;i<g_station_count;i++){
        Station *s=&g_stations[i];
        if(s->status==STATUS_IDLE)idle++;
        else if(s->status==STATUS_CHARGING)chg++;
        else fault++;
        sum_inc +=s->total_income;
        sum_eng +=s->total_energy_kwh;
        sum_sess+=s->total_sessions;
    }
    for(int i=0;i<g_record_count;i++)
        if(g_records[i].finished) finished++;

    printf("充电桩总数  : %d  (空闲:%d 充电中:%d 故障:%d)\n",
           g_station_count,idle,chg,fault);
    printf("充电记录总数: %d  (完成:%d 进行中:%d)\n",
           g_record_count,finished,g_record_count-finished);
    printf("累计充电次数: %d\n",sum_sess);
    printf("累计充电电量: %.2f 度\n",sum_eng);
    printf("累计总收入  : %.2f 元\n",sum_inc);
    if(g_station_count>0){
        printf("单桩均收入  : %.2f 元\n",sum_inc/g_station_count);
        printf("单桩均次数  : %.2f\n",(float)sum_sess/g_station_count);
    }
    int best=0;
    for(int i=1;i<g_station_count;i++)
        if(g_stations[i].total_income>g_stations[best].total_income) best=i;
    printf("收入最高桩  : ID=%d [%s] %.2f元\n",
           g_stations[best].id,g_stations[best].name,g_stations[best].total_income);
    if(finished>0){
        int br=0;
        for(int i=1;i<g_record_count;i++)
            if(g_records[i].cost>g_records[br].cost) br=i;
        printf("最高单次费用: 记录ID=%d 用户=%s %.2f元\n",
               g_records[br].record_id,g_records[br].user_name,g_records[br].cost);
    }
}

/* === 按位置统计 === */
void stats_by_location(void){
    print_title("按位置统计充电桩");
    if(g_station_count==0){printf("暂无数据。\n");return;}
    printf("%-4s %-12s %-16s %-8s %-8s %-10s\n",
           "ID","名称","位置","次数","电量度","收入元");
    printf("----------------------------------------------------------\n");
    for(int i=0;i<g_station_count;i++){
        Station *s=&g_stations[i];
        printf("%-4d %-12s %-16s %-8d %-8.2f %-10.2f\n",
               s->id,s->name,s->location,
               s->total_sessions,s->total_energy_kwh,s->total_income);
    }
}

/* === 充电桩排序 === */
void sort_stations_menu(void){
    print_title("充电桩排序");
    if(g_station_count==0){printf("暂无数据。\n");return;}
    printf("排序字段: 1.收入  2.使用次数  3.功率\n");
    printf("排序方向: a.升序  b.降序\n");
    int f; char d;
    printf("字段(1/2/3): "); scanf("%d",&f); clear_input();
    printf("方向(a/b): ");  scanf("%c",&d); clear_input();

    Station tmp[MAX_STATIONS];
    memcpy(tmp,g_stations,sizeof(Station)*g_station_count);

    if(f==1)
        qsort(tmp,g_station_count,sizeof(Station),
              (d=='a'||d=='A')?cmp_st_income_asc:cmp_st_income_desc);
    else if(f==2)
        qsort(tmp,g_station_count,sizeof(Station),
              (d=='a'||d=='A')?cmp_st_sess_asc:cmp_st_sess_desc);
    else if(f==3)
        qsort(tmp,g_station_count,sizeof(Station),
              (d=='a'||d=='A')?cmp_st_power_asc:cmp_st_power_desc);
    else{printf("无效字段。\n");return;}

    printf("\n%-4s %-12s %-16s %-8s %-8s %-10s\n",
           "ID","名称","位置","功率kW","次数","收入元");
    printf("--------------------------------------------------------------\n");
    for(int i=0;i<g_station_count;i++)
        printf("%-4d %-12s %-16s %-8.1f %-8d %-10.2f\n",
               tmp[i].id,tmp[i].name,tmp[i].location,
               tmp[i].power_kw,tmp[i].total_sessions,tmp[i].total_income);
}

/* === 充电记录排序 === */
void sort_records_menu(void){
    print_title("充电记录排序");
    if(g_record_count==0){printf("暂无记录。\n");return;}
    printf("排序字段: 1.费用  2.电量  3.时长\n");
    printf("排序方向: a.升序  b.降序\n");
    int f; char d;
    printf("字段(1/2/3): "); scanf("%d",&f); clear_input();
    printf("方向(a/b): ");  scanf("%c",&d); clear_input();

    ChargeRecord tmp[MAX_RECORDS];
    memcpy(tmp,g_records,sizeof(ChargeRecord)*g_record_count);

    if(f==1)
        qsort(tmp,g_record_count,sizeof(ChargeRecord),
              (d=='a'||d=='A')?cmp_rec_cost_asc:cmp_rec_cost_desc);
    else if(f==2)
        qsort(tmp,g_record_count,sizeof(ChargeRecord),
              (d=='a'||d=='A')?cmp_rec_energy_asc:cmp_rec_energy_desc);
    else if(f==3)
        qsort(tmp,g_record_count,sizeof(ChargeRecord),
              (d=='a'||d=='A')?cmp_rec_dur_asc:cmp_rec_dur_desc);
    else{printf("无效字段。\n");return;}

    printf("\n%-4s %-4s %-10s %-7s %-8s %-8s\n",
           "RID","SID","用户","时长h","电量度","费用元");
    printf("----------------------------------------------------\n");
    for(int i=0;i<g_record_count;i++)
        printf("%-4d %-4d %-10s %-7.2f %-8.2f %-8.2f\n",
               tmp[i].record_id,tmp[i].station_id,tmp[i].user_name,
               tmp[i].duration_h,tmp[i].energy_kwh,tmp[i].cost);
}

/* === 数据报表 === */
void print_report(void){
    print_title("充电桩管理系统  数据报表");
    char now[TIME_LEN]; now_time_str(now);
    printf("打印时间: %s\n\n",now);

    printf("[一] 充电桩信息汇总\n");
    printf("%-4s %-12s %-14s %-7s %-7s %-8s %-6s %-8s %-8s\n",
           "ID","名称","位置","功率","电价","状态","次数","电量","收入");
    printf("------------------------------------------------------------------------------\n");
    float t_inc=0,t_eng=0; int t_sess=0;
    for(int i=0;i<g_station_count;i++){
        Station *s=&g_stations[i];
        printf("%-4d %-12s %-14s %-7.1f %-7.2f %-8s %-6d %-8.2f %-8.2f\n",
               s->id,s->name,s->location,s->power_kw,s->price_per_kwh,
               status_str(s->status),s->total_sessions,
               s->total_energy_kwh,s->total_income);
        t_inc+=s->total_income;
        t_eng+=s->total_energy_kwh;
        t_sess+=s->total_sessions;
    }
    printf("------------------------------------------------------------------------------\n");
    printf("合计: 桩数=%d  次数=%d  电量=%.2f度  收入=%.2f元\n\n",
           g_station_count,t_sess,t_eng,t_inc);

    printf("[二] 充电记录汇总 (共%d条)\n",g_record_count);
    int fin=0; float rec_cost=0;
    for(int i=0;i<g_record_count;i++)
        if(g_records[i].finished){fin++; rec_cost+=g_records[i].cost;}
    printf("已完成:%d条  进行中:%d条  已完成总费用:%.2f元\n",
           fin,g_record_count-fin,rec_cost);
    printf("\n========== 报表结束 ==========\n");
}

/* === 统计报表子菜单 === */
void stats_menu(void){
    int op;
    while(1){
        print_title("统计与报表");
        printf("1. 综合统计\n");
        printf("2. 按位置统计\n");
        printf("3. 充电桩排序\n");
        printf("4. 充电记录排序\n");
        printf("5. 打印数据报表\n");
        printf("0. 返回主菜单\n");
        printf("请选择: ");
        if(scanf("%d",&op)!=1){clear_input();continue;}
        clear_input();
        if(op==0) break;
        switch(op){
            case 1: stats_summary();      break;
            case 2: stats_by_location();  break;
            case 3: sort_stations_menu(); break;
            case 4: sort_records_menu();  break;
            case 5: print_report();       break;
            default: printf("无效选项。\n");
        }
        press_any_key();
    }
}
