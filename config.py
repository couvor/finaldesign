# =============================================================================
# 文件名: config.py
# 功  能: 系统全局配置文件
#         定义所有模块共用的路径、常量、枚举值和默认参数
# 说  明: 本文件是项目的唯一配置入口，所有硬编码常量必须在此处定义
#         修改此文件后无需重启，下次调用相关函数时自动生效
# =============================================================================

import os

# -----------------------------------------------------------------------------
# 数据目录配置
# DATA_DIR  : 所有 CSV 数据文件的存放根目录（相对于本文件所在目录）
# 若目录不存在，系统启动时会自动创建
# -----------------------------------------------------------------------------
BASE_DIR = os.path.dirname(os.path.abspath(__file__))   # 项目根目录的绝对路径
DATA_DIR = os.path.join(BASE_DIR, "data")               # CSV 数据文件夹路径

# -----------------------------------------------------------------------------
# CSV 文件路径 —— 各业务模块对应的数据文件
# -----------------------------------------------------------------------------
CHARGING_STATIONS_FILE = os.path.join(DATA_DIR, "charging_stations.csv")   # 充电桩基础信息
CHARGING_RECORDS_FILE  = os.path.join(DATA_DIR, "charging_records.csv")    # 充电记录流水
USERS_FILE             = os.path.join(DATA_DIR, "users.csv")               # 用户信息
FAULT_RECORDS_FILE     = os.path.join(DATA_DIR, "fault_records.csv")       # 故障记录
FEE_RULES_FILE         = os.path.join(DATA_DIR, "fee_rules.csv")           # 计费规则

# -----------------------------------------------------------------------------
# 充电桩状态枚举（存入 CSV 时使用字符串，便于人工阅读和编辑）
# STATUS_AVAILABLE : 空闲，可立即使用
# STATUS_OCCUPIED  : 占用，正在充电
# STATUS_FAULT     : 故障，暂停服务
# STATUS_MAINTAIN  : 维护，计划性停机
# STATUS_OFFLINE   : 离线，通信中断
# -----------------------------------------------------------------------------
STATUS_AVAILABLE = "空闲"     # 充电桩可以接受新的充电请求
STATUS_OCCUPIED  = "占用"     # 充电桩正在为某辆车服务
STATUS_FAULT     = "故障"     # 充电桩检测到硬件/软件异常
STATUS_MAINTAIN  = "维护"     # 人工设置的计划维护状态
STATUS_OFFLINE   = "离线"     # 充电桩与服务器失去连接

# 所有合法状态列表，用于输入校验
VALID_STATUSES = [
    STATUS_AVAILABLE,
    STATUS_OCCUPIED,
    STATUS_FAULT,
    STATUS_MAINTAIN,
    STATUS_OFFLINE,
]

# -----------------------------------------------------------------------------
# 充电桩类型枚举
# TYPE_DC  : 直流快充（DC），功率大，充电速度快，适合商业场所
# TYPE_AC  : 交流慢充（AC），功率小，充电速度慢，适合居民区
# TYPE_DUAL: 双枪充电桩，同时支持 DC 和 AC 两种充电方式
# -----------------------------------------------------------------------------
TYPE_DC   = "直流快充"    # DC 充电，功率通常在 30kW–350kW
TYPE_AC   = "交流慢充"    # AC 充电，功率通常在 3.3kW–22kW
TYPE_DUAL = "双枪充电"    # 一台设备支持两种接口

# 所有合法类型列表
VALID_TYPES = [TYPE_DC, TYPE_AC, TYPE_DUAL]

# -----------------------------------------------------------------------------
# 充电记录状态枚举
# RECORD_CHARGING  : 充电进行中
# RECORD_COMPLETED : 充电正常结束
# RECORD_INTERRUPTED: 充电异常中断
# -----------------------------------------------------------------------------
RECORD_CHARGING    = "充电中"    # 充电会话仍在进行
RECORD_COMPLETED   = "已完成"    # 充电会话正常结束
RECORD_INTERRUPTED = "已中断"    # 充电会话因故障或手动停止而中断

# -----------------------------------------------------------------------------
# 数值常量
# MAX_POWER_KW      : 允许的最大充电功率（单位：千瓦）
# MIN_POWER_KW      : 允许的最小充电功率（单位：千瓦）
# DEFAULT_UNIT_PRICE: 默认电费单价（单位：元/度）
# DEFAULT_SERVICE_FEE: 默认服务费单价（单位：元/度）
# MAX_CHARGING_HOURS: 单次充电最长时间限制（单位：小时）
# -----------------------------------------------------------------------------
MAX_POWER_KW       = 350.0    # 超大功率直流充电桩上限
MIN_POWER_KW       = 3.3      # 家用级交流充电桩下限
DEFAULT_UNIT_PRICE = 0.85     # 默认电费单价：0.85 元/度
DEFAULT_SERVICE_FEE = 0.35    # 默认服务费：0.35 元/度
MAX_CHARGING_HOURS = 24.0     # 超过此时长视为异常记录

# -----------------------------------------------------------------------------
# CSV 文件列名定义（字段顺序与 CSV 文件头部完全一致）
# 修改列名时，同步修改对应的读写函数
# -----------------------------------------------------------------------------

# 充电桩基础信息表字段
STATION_FIELDS = [
    "station_id",        # 充电桩唯一编号（格式：CS001）
    "station_name",      # 充电桩名称（人工可读标签）
    "location",          # 物理位置描述（城市/区域/具体地址）
    "station_type",      # 充电类型（直流快充/交流慢充/双枪充电）
    "power_kw",          # 额定功率（单位：kW）
    "status",            # 当前状态
    "install_date",      # 安装日期（格式：YYYY-MM-DD）
    "operator",          # 运营商名称
    "contact_phone",     # 运维联系电话
    "total_charging_times",  # 累计充电次数（整数）
    "total_energy_kwh",  # 累计充电电量（单位：kWh，保留2位小数）
    "latitude",          # 地理纬度（WGS84坐标系，保留6位小数）
    "longitude",         # 地理经度（WGS84坐标系，保留6位小数）
    "remark",            # 备注信息（可为空）
]

# 充电记录流水表字段
RECORD_FIELDS = [
    "record_id",         # 记录唯一编号（格式：REC000001）
    "station_id",        # 关联的充电桩编号
    "user_id",           # 充电用户ID（关联用户表）
    "start_time",        # 充电开始时间（格式：YYYY-MM-DD HH:MM:SS）
    "end_time",          # 充电结束时间（格式：YYYY-MM-DD HH:MM:SS）
    "duration_minutes",  # 充电时长（分钟，整数）
    "energy_kwh",        # 本次充电电量（单位：kWh，保留2位小数）
    "unit_price",        # 电费单价（元/kWh）
    "service_fee",       # 服务费单价（元/kWh）
    "total_cost",        # 本次充电总费用（元，保留2位小数）
    "record_status",     # 充电状态（充电中/已完成/已中断）
    "payment_method",    # 支付方式（微信/支付宝/银行卡/APP余额）
]

# 用户信息表字段
USER_FIELDS = [
    "user_id",           # 用户唯一ID（格式：U000001）
    "username",          # 用户登录名
    "real_name",         # 真实姓名
    "phone",             # 手机号码（11位）
    "register_date",     # 注册日期（格式：YYYY-MM-DD）
    "balance",           # 账户余额（元，保留2位小数）
    "total_charges",     # 累计充电次数
    "total_spent",       # 累计消费金额（元，保留2位小数）
    "vehicle_type",      # 