/*
 * ============================================================
 *  文件名: auth.c
 *  功  能: 用户认证与账户管理模块
 *          包含登录验证、密码修改、管理员用户管理三大功能
 *  安全说明: 本系统密码以明文存储在 CSV 文件中，
 *            仅供教学演示使用，生产环境应使用哈希加密（如 bcrypt）
 * ============================================================
 */

#include "charging.h"

/* ============================================================
 *  login
 *  功能: 用户登录验证
 *  流程:
 *    1. 提示输入用户名和密码
 *    2. 遍历 g_users[] 数组进行比对
 *    3. 匹配成功则设置 g_current_user 指针并返回 1
 *    4. 最多允许 3 次错误尝试，超过则退出程序
 *  返回值: 1=登录成功, 0=登录失败（已用尽尝试次数）
 * ============================================================ */
int login(void) {
    char uname[NAME_LEN];  /* 用户输入的用户名缓冲区 */
    char pass[PASS_LEN];   /* 用户输入的密码缓冲区 */
    int attempts = 3;      /* 最大允许尝试次数 */

    print_title("用户登录");
    printf("  默认管理员账户: admin / admin123\n\n");

    /* 循环最多 3 次，attempts-- 在判断前执行，0次时退出 */
    while (attempts-- > 0) {
        printf("用户名: ");
        fgets(uname, sizeof(uname), stdin);          /* fgets 安全读取，防止溢出 */
        uname[strcspn(uname, "\n")] = '\0';          /* 去除 fgets 读入的换行符 */

        printf("密  码: ");
        fgets(pass, sizeof(pass), stdin);
        pass[strcspn(pass, "\n")] = '\0';

        /* 线性遍历用户表，比对用户名和密码 */
        for (int i = 0; i < g_user_count; i++) {
            if (strcmp(g_users[i].username, uname) == 0 &&
                strcmp(g_users[i].password, pass)  == 0) {

                /* 登录成功：设置全局当前用户指针 */
                g_current_user = &g_users[i];
                printf("\n欢迎，%s！角色：%s\n",
                       uname,
                       g_users[i].role == ROLE_ADMIN ? "管理员" : "普通用户");
                return 1;  /* 登录成功，返回主流程 */
            }
        }

        /* 本次尝试失败 */
        if (attempts > 0)
            printf("用户名或密码错误，剩余尝试次数：%d\n", attempts);
    }

    printf("登录失败次数过多，程序退出。\n");
    return 0;  /* 登录失败 */
}

/* ============================================================
 *  change_password
 *  功能: 修改当前登录用户的密码
 *  流程:
 *    1. 验证旧密码（防止他人在用户离开时趁机改密码）
 *    2. 输入新密码并二次确认（防止误输入）
 *    3. 更新内存中的密码并立即保存到 CSV
 *  前置条件: g_current_user 不为 NULL（已登录）
 * ============================================================ */
void change_password(void) {
    if (!g_current_user) return;  /* 未登录时直接返回，防止空指针 */

    char old_pass[PASS_LEN];   /* 旧密码输入缓冲区 */
    char new_pass[PASS_LEN];   /* 新密码输入缓冲区 */
    char confirm[PASS_LEN];    /* 新密码确认缓冲区 */

    print_title("修改密码");

    /* 第一步：验证旧密码 */
    printf("请输入旧密码: ");
    fgets(old_pass, sizeof(old_pass), stdin);
    old_pass[strcspn(old_pass, "\n")] = '\0';

    if (strcmp(old_pass, g_current_user->password) != 0) {
        printf("旧密码错误，修改失败。\n");
        return;
    }

    /* 第二步：输入新密码 */
    printf("请输入新密码: ");
    fgets(new_pass, sizeof(new_pass), stdin);
    new_pass[strcspn(new_pass, "\n")] = '\0';

    /* 新密码长度校验：至少6位 */
    if (strlen(new_pass) < 6) {
        printf("新密码长度不能少于6位。\n");
        return;
    }

    /* 第三步：二次确认新密码 */
    printf("确认新密码: ");
    fgets(confirm, sizeof(confirm), stdin);
    confirm[strcspn(confirm, "\n")] = '\0';

    if (strcmp(new_pass, confirm) != 0) {
        printf("两次输入不一致，修改失败。\n");
        return;
    }

    /* 更新内存中的密码 */
    strcpy(g_current_user->password, new_pass);
    /* 立即保存到 CSV，防止程序异常退出时丢失修改 */
    save_users();
    printf("密码修改成功！\n");
}

/* ============================================================
 *  user_manage_menu
 *  功能: 管理员专用的用户管理子菜单
 *  权限: 仅 ROLE_ADMIN 角色可进入，普通用户直接返回
 *  功能列表:
 *    1. 查看所有用户列表
 *    2. 新增用户（用户名、密码、角色）
 *    3. 删除用户（不能删除 admin 账户）
 * ============================================================ */
void user_manage_menu(void) {
    /* 权限检查：非管理员直接拒绝 */
    if (!g_current_user || g_current_user->role != ROLE_ADMIN) {
        printf("权限不足，仅管理员可操作。\n");
        return;
    }

    int op;
    while (1) {
        print_title("用户管理（管理员）");
        printf(" 1. 查看所有用户\n");
        printf(" 2. 添加新用户\n");
        printf(" 3. 删除用户\n");
        printf(" 0. 返回主菜单\n");
        printf("请选择: ");

        if (scanf("%d", &op) != 1) { clear_input(); continue; }
        clear_input();
        if (op == 0) break;

        if (op == 1) {
            /* --- 列出所有用户 --- */
            printf("\n%-4s %-16s %-10s %-12s %-14s\n",
                   "序号", "用户名", "角色", "真实姓名", "注册日期");
            printf("------------------------------------------------------------\n");
            for (int i = 0; i < g_user_count; i++) {
                printf("%-4d %-16s %-10s %-12s %-14s\n",
                       i + 1,
                       g_users[i].username,
                       g_users[i].role == ROLE_ADMIN ? "管理员" : "普通用户",
                       g_users[i].real_name,
                       g_users[i].register_date);
            }

        } else if (op == 2) {
            /* --- 新增用户 --- */
            if (g_user_count >= MAX_USERS) {
                printf("用户数量已达上限（%d）。\n", MAX_USERS);
                press_any_key(); continue;
            }
            User u;
            memset(&u, 0, sizeof(u));

            printf("用户名: ");
            fgets(u.username, sizeof(u.username), stdin);
            u.username[strcspn(u.username, "\n")] = '\0';

            /* 检查用户名是否已存在 */
            int dup = 0;
            for (int i = 0; i < g_user_count; i++)
                if (strcmp(g_users[i].username, u.username) == 0) { dup = 1; break; }
            if (dup) { printf("用户名已存在。\n"); press_any_key(); continue; }

            printf("密  码: ");
            fgets(u.password, sizeof(u.password), stdin);
            u.password[strcspn(u.password, "\n")] = '\0';

            printf("真实姓名: ");
            fgets(u.real_name, sizeof(u.real_name), stdin);
            u.real_name[strcspn(u.real_name, "\n")] = '\0';

            printf("手机号: ");
            fgets(u.phone, sizeof(u.phone), stdin);
            u.phone[strcspn(u.phone, "\n")] = '\0';

            printf("角色(0=管理员 1=普通用户): ");
            int r; scanf("%d", &r); clear_input();
            u.role = (r == 0) ? ROLE_ADMIN : ROLE_USER;

            now_time_str(u.register_date);  /* 记录注册时间 */
            g_users[g_user_count++] = u;
            save_users();
            printf("[√] 用户 %s 添加成功。\n", u.username);

        } else if (op == 3) {
            /* --- 删除用户 --- */
            char uname[NAME_LEN];
            printf("输入要删除的用户名: ");
            fgets(uname, sizeof(uname), stdin);
            uname[strcspn(uname, "\n")] = '\0';

            /* 保护 admin 账户不被删除 */
            if (strcmp(uname, "admin") == 0) {
                printf("不能删除系统内置 admin 账户。\n");
                press_any_key(); continue;
            }

            int found = 0;
            for (int i = 0; i < g_user_count; i++) {
                if (strcmp(g_users[i].username, uname) == 0) {
                    /* 将后续元素向前移动，覆盖被删除的元素 */
                    for (int j = i; j < g_user_count - 1; j++)
                        g_users[j] = g_users[j + 1];
                    g_user_count--;
                    save_users();
                    printf("[√] 用户 %s 已删除。\n", uname);
                    found = 1;
                    break;
                }
            }
            if (!found) printf("用户 %s 不存在。\n", uname);
        } else {
            printf("无效选项。\n");
        }
        press_any_key();
    }
}