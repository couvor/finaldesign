/*
 * visual.c  --  充电桩管理系统  C语言控制台可视化界面
 * 地点固定为三个校区:
 *   西安邮电大学东区 / 西安邮电大学西区 / 西安邮电大学雁塔校区
 */
#include "charging.h"
#include <windows.h>

#define C_RESET    7
#define C_TITLE   11
#define C_BORDER  14
#define C_BAR1    10
#define C_BAR2     9
#define C_BAR3    13
#define C_IDLE    10
#define C_CHG      9
#define C_FAULT   12
#define C_LABEL   15
#define C_DIM      8
#define C_INCOME  14
#define C_HEAD     6
#define C_PRICE   13
#define C_TYPE1   10
#define C_TYPE2    3
#define C_TYPE3   13

static const char *g_locs[3] = {
    "西安邮电大学东区",
    "西安邮电大学西区",
    "西安邮电大学雁塔校区"
};
static const char *g_labels[3] = {"东区", "西区", "雁塔校区"};
static const int   g_cols[3]   = {C_BAR1, C_BAR2, C_BAR3};
static HANDLE hCon;

static void setColor(int c) { SetConsoleTextAttribute(hCon, (WORD)c); }
static void hline(int w, char ch) {
    for (int i = 0; i < w; i++) putchar(ch);
    putchar('\n');
}
static void vis_title(const char *t) {
    printf("\n");
    setColor(C_BORDER); printf("  "); hline(68, '=');
    setColor(C_TITLE);
    int len = (int)strlen(t), pad = (68 - len) / 2;
    if (pad < 0) pad = 0;
    printf("  ");
    for (int i = 0; i < pad; i++) putchar(' ');
    printf("%s\n", t);
    setColor(C_BORDER); printf("  "); hline(68, '=');
    setColor(C_RESET);
}
static void draw_bar(int filled, int total, int col) {
    if (filled > total) filled = total;
    if (filled < 0)     filled = 0;
    setColor(col);
    for (int k = 0; k < filled; k++) putchar('#');
    setColor(C_DIM);
    for (int k = filled; k < total; k++) putchar('-');
    setColor(C_RESET);
}
static int loc_index(const char *loc) {
    for (int j = 0; j < 3; j++)
        if (strstr(loc, g_locs[j])) return j;
    return -1;
}

/* [1] 总览仪表盘 */
static void chart_dashboard(void) {
    vis_title("[1/10]  总览仪表盘");
    int idle=0, chg=0, fault=0, sess=0;
    double inc=0.0, eng=0.0;
    for (int i=0; i<g_station_count; i++) {
        Station *s = &g_stations[i];
        if      (s->status==STATUS_IDLE)     idle++;
        else if (s->status==STATUS_CHARGING) chg++;
        else                                 fault++;
        inc  += s->total_income;
        eng  += s->total_energy_kwh;
        sess += s->total_sessions;
    }
    int tot = g_station_count>0 ? g_station_count : 1;
    printf("\n");
    setColor(C_HEAD); printf("  ===== 充电桩概况 =====\n"); setColor(C_RESET);
    setColor(C_LABEL); printf("  总桩数   : "); setColor(C_TITLE); printf("%6d 台\n", g_station_count);
    setColor(C_LABEL); printf("  空  闲   : "); setColor(C_IDLE);  printf("%6d 台  [", idle);
    draw_bar(idle*20/tot,20,C_IDLE);   setColor(C_IDLE);  printf("]  %.1f%%\n", idle*100.0/tot);
    setColor(C_LABEL); printf("  充电中   : "); setColor(C_CHG);   printf("%6d 台  [", chg);
    draw_bar(chg*20/tot,20,C_CHG);    setColor(C_CHG);   printf("]  %.1f%%\n", chg*100.0/tot);
    setColor(C_LABEL); printf("  故  障   : "); setColor(C_FAULT); printf("%6d 台  [", fault);
    draw_bar(fault*20/tot,20,C_FAULT); setColor(C_FAULT); printf("]  %.1f%%\n", fault*100.0/tot);
    printf("\n");
    setColor(C_HEAD); printf("  ===== 运营数据 =====\n"); setColor(C_RESET);
    setColor(C_LABEL); printf("  累计充电次数 : "); setColor(C_INCOME); printf("%12d 次\n",  sess);
    setColor(C_LABEL); printf("  累计充电电量 : "); setColor(C_INCOME); printf("%15.2f 度\n", eng);
    setColor(C_LABEL); printf("  累计总收入   : "); setColor(C_INCOME); printf("%15.2f 元\n", inc);
    if (g_station_count>0) { setColor(C_LABEL); printf("  单桩均收入   : "); setColor(C_DIM); printf("%15.2f 元\n", inc/g_station_count); }
    setColor(C_LABEL); printf("  充电记录总数 : "); setColor(C_TITLE); printf("%12d 条\n", g_record_count);
    printf("\n");
    setColor(C_HEAD); printf("  ===== 校区速览 =====\n"); setColor(C_RESET);
    int cnt3[3]={0,0,0};
    for (int i=0; i<g_station_count; i++) { int j=loc_index(g_stations[i].location); if (j>=0) cnt3[j]++; }
    for (int j=0; j<3; j++) {
        setColor(g_cols[j]); printf("  %-10s: %4d 台  [", g_labels[j], cnt3[j]);
        draw_bar(cnt3[j]*30/tot,30,g_cols[j]); printf("]\n");
    }
    setColor(C_RESET);
}

/* [2] 各校区充电桩数量对比 */
static void chart_station_by_loc(void) {
    vis_title("[2/10]  各校区充电桩数量对比");
    int cnt[3]={0,0,0};
    for (int i=0; i<g_station_count; i++) { int j=loc_index(g_stations[i].location); if (j>=0) cnt[j]++; }
    int maxv=1; for (int j=0; j<3; j++) if (cnt[j]>maxv) maxv=cnt[j];
    int total=cnt[0]+cnt[1]+cnt[2]; if (total==0) total=1;
    printf("\n");
    setColor(C_HEAD); printf("  %-10s %5s  %6s   柱状图(满格=%d台)\n","校区","数量","占比",maxv);
    setColor(C_BORDER); printf("  "); hline(66,'-'); setColor(C_RESET);
    for (int j=0; j<3; j++) {
        setColor(g_cols[j]); printf("  %-10s %4d  %5.1f%%  [", g_labels[j], cnt[j], cnt[j]*100.0/total);
        draw_bar(cnt[j]*40/maxv,40,g_cols[j]); printf("]\n"); setColor(C_RESET);
    }
    setColor(C_DIM); printf("  东区=%d  西区=%d  雁塔=%d  合计=%d\n",cnt[0],cnt[1],cnt[2],cnt[0]+cnt[1]+cnt[2]); setColor(C_RESET);
    printf("\n"); setColor(C_HEAD); printf("  -- 各校区状态细分 --\n"); setColor(C_RESET);
    setColor(C_BORDER); printf("  "); hline(50,'-'); setColor(C_RESET);
    setColor(C_HEAD); printf("  %-10s %8s %8s %8s\n","校区","空闲","充电中","故障");
    setColor(C_BORDER); printf("  "); hline(50,'-'); setColor(C_RESET);
    for (int j=0; j<3; j++) {
        int id=0,ch=0,fa=0;
        for (int i=0; i<g_station_count; i++) {
            if (loc_index(g_stations[i].location)!=j) continue;
            if      (g_stations[i].status==STATUS_IDLE)     id++;
            else if (g_stations[i].status==STATUS_CHARGING) ch++;
            else                                             fa++;
        }
        setColor(g_cols[j]); printf("  %-10s",g_labels[j]);
        setColor(C_IDLE); printf(" %8d",id); setColor(C_CHG); printf(" %8d",ch); setColor(C_FAULT); printf(" %8d\n",fa);
        setColor(C_RESET);
    }
}

/* [3] 充电桩状态分布 */
static void chart_status_dist(void) {
    vis_title("[3/10]  充电桩状态分布");
    int cnt[3]={0,0,0};
    for (int i=0; i<g_station_count; i++) { int st=(int)g_stations[i].status; if (st>=0&&st<=2) cnt[st]++; }
    int total=cnt[0]+cnt[1]+cnt[2]; if (total==0) total=1;
    static const char *names[3]={"空闲  ","充电中","故障  "};
    int cols3[3]={C_IDLE,C_CHG,C_FAULT};
    printf("\n"); setColor(C_HEAD); printf("  %-8s %6s %8s   柱状图\n","状态","数量","占比");
    setColor(C_BORDER); printf("  "); hline(62,'-'); setColor(C_RESET);
    for (int j=0; j<3; j++) {
        setColor(cols3[j]); printf("  %s  ",names[j]);
        setColor(C_LABEL); printf("%5d  ",cnt[j]);
        setColor(C_DIM); printf("%6.1f%%  [",cnt[j]*100.0/total);
        draw_bar(cnt[j]*34/total,34,cols3[j]); printf("]\n"); setColor(C_RESET);
    }
    printf("\n"); setColor(C_HEAD); printf("  -- 状态饼图 (字符模拟) --\n  ");
    setColor(C_BORDER); hline(50,'-');
    printf("  [");
    int bwp=46, fp0=cnt[0]*bwp/total, fp1=cnt[1]*bwp/total, fp2=bwp-fp0-fp1;
    setColor(C_IDLE);  for (int k=0;k<fp0;k++) putchar('O');
    setColor(C_CHG);   for (int k=0;k<fp1;k++) putchar('C');
    setColor(C_FAULT); for (int k=0;k<fp2;k++) putchar('X');
    setColor(C_BORDER); printf("]\n"); setColor(C_DIM);
    printf("  图例: O=空闲(%.1f%%)  C=充电中(%.1f%%)  X=故障(%.1f%%)\n",
           cnt[0]*100.0/total,cnt[1]*100.0/total,cnt[2]*100.0/total);
    setColor(C_RESET);
}

/* [4] 各校区累计收入 */
static void chart_income_by_loc(void) {
    vis_title("[4/10]  各校区累计收入");
    double inc[3]={0,0,0}, eng[3]={0,0,0};
    int sess[3]={0,0,0}, cnt[3]={0,0,0};
    for (int i=0; i<g_station_count; i++) {
        int j=loc_index(g_stations[i].location); if (j<0) continue;
        inc[j]+=g_stations[i].total_income; eng[j]+=g_stations[i].total_energy_kwh;
        sess[j]+=g_stations[i].total_sessions; cnt[j]++;
    }
    double maxInc=1.0; for (int j=0;j<3;j++) if (inc[j]>maxInc) maxInc=inc[j];
    printf("\n"); setColor(C_HEAD); printf("  %-10s %16s   柱状图\n","校区","累计收入(元)");
    setColor(C_BORDER); printf("  "); hline(64,'-'); setColor(C_RESET);
    for (int j=0;j<3;j++) {
        setColor(g_cols[j]); printf("  %-10s",g_labels[j]);
        setColor(C_LABEL); printf(" %14.2f  [",inc[j]);
        int bwi=28, fi=(int)(inc[j]*bwi/maxInc); if (fi>bwi) fi=bwi;
        draw_bar(fi,bwi,g_cols[j]); printf("]\n"); setColor(C_RESET);
    }
    printf("\n"); setColor(C_HEAD);
    printf("  %-10s %14s %14s %10s %6s\n","校区","累计电量(度)","累计收入(元)","充电次数","桩数");
    setColor(C_BORDER); printf("  "); hline(62,'-'); setColor(C_RESET);
    for (int j=0;j<3;j++) {
        setColor(g_cols[j]); printf("  %-10s",g_labels[j]);
        setColor(C_LABEL); printf(" %14.2f %14.2f %10d %6d\n",eng[j],inc[j],sess[j],cnt[j]);
        setColor(C_RESET);
    }
    double totalInc=inc[0]+inc[1]+inc[2]; if (totalInc<1.0) totalInc=1.0;
    printf("\n"); setColor(C_HEAD); printf("  -- 收入占比饼图 --\n  ");
    setColor(C_BORDER); hline(50,'-'); printf("  [");
    int bw=46, s0=(int)(inc[0]*bw/totalInc), s1=(int)(inc[1]*bw/totalInc), s2=bw-s0-s1;
    setColor(C_BAR1); for (int k=0;k<s0;k++) putchar('E');
    setColor(C_BAR2); for (int k=0;k<s1;k++) putchar('W');
    setColor(C_BAR3); for (int k=0;k<s2;k++) putchar('Y');
    setColor(C_BORDER); printf("]\n"); setColor(C_DIM);
    printf("  图例: E=东区(%.1f%%)  W=西区(%.1f%%)  Y=雁塔(%.1f%%)\n",
           inc[0]*100.0/totalInc,inc[1]*100.0/totalInc,inc[2]*100.0/totalInc);
    setColor(C_RESET);
}

/* [5] 各校区充电电量对比 */
static void chart_energy_by_loc(void) {
    vis_title("[5/10]  各校区充电电量对比");
    double eng[3]={0,0,0};
    for (int i=0;i<g_station_count;i++) { int j=loc_index(g_stations[i].location); if (j>=0) eng[j]+=g_stations[i].total_energy_kwh; }
    double maxe=1.0; for (int j=0;j<3;j++) if (eng[j]>maxe) maxe=eng[j];
    double total=eng[0]+eng[1]+eng[2]; if (total<1.0) total=1.0;
    printf("\n"); setColor(C_HEAD); printf("  %-10s %16s  %6s   柱状图\n","校区","累计电量(度)","占比");
    setColor(C_BORDER); printf("  "); hline(66,'-'); setColor(C_RESET);
    for (int j=0;j<3;j++) {
        setColor(g_cols[j]); printf("  %-10s",g_labels[j]);
        setColor(C_LABEL); printf(" %14.2f  %5.1f%%  [",eng[j],eng[j]*100.0/total);
        int bwi=26, fi=(int)(eng[j]*bwi/maxe); if (fi>bwi) fi=bwi;
        draw_bar(fi,bwi,g_cols[j]); printf("]\n"); setColor(C_RESET);
    }
    setColor(C_DIM); printf("  总电量: %.2f 度  平均每桩: %.2f 度\n",
           total, g_station_count>0 ? total/g_station_count : 0.0);
    setColor(C_RESET);
}

/* [6] 充电类型分布 */
static void chart_type_dist(void) {
    vis_title("[6/10]  充电类型分布");
    int cnt[3]={0,0,0};
    double inc[3]={0,0,0}, eng[3]={0,0,0};
    static const char *tnames[3]={"直流快充","交流慢充","双枪充电"};
    int tcols[3]={C_TYPE1,C_TYPE2,C_TYPE3};
    for (int i=0;i<g_station_count;i++) {
        int 