#include "cf_api.h"
#include "../lib/cJSON.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 使用系统curl.exe获取URL内容
static char* fetch_url(const char *url) {
    char cmd[1024];
    snprintf(cmd, sizeof(cmd), "curl.exe -s --ssl-no-revoke \"%s\"", url);
    
    FILE *fp = _popen(cmd, "r");
    if (!fp) return NULL;
    
    char *buffer = NULL;
    size_t len = 0;
    char chunk[4096];
    
    while (fgets(chunk, sizeof(chunk), fp)) {
        size_t chunk_len = strlen(chunk);
        char *new_buffer = (char*)realloc(buffer, len + chunk_len + 1);
        if (!new_buffer) { free(buffer); _pclose(fp); return NULL; }
        buffer = new_buffer;
        memcpy(buffer + len, chunk, chunk_len);
        len += chunk_len;
        buffer[len] = '\0';
    }
    _pclose(fp);
    return buffer;
}

// 获取比赛列表
static cJSON* fetch_contests_json() {
    char *json = fetch_url("https://codeforces.com/api/contest.list?gym=false");
    if (!json) return NULL;
    cJSON *root = cJSON_Parse(json);
    free(json);
    return root;
}

// 获取用户信息
static cJSON* fetch_user_info_json(const char *handle) {
    char url[512];
    snprintf(url, sizeof(url), "https://codeforces.com/api/user.info?handles=%s", handle);
    char *json = fetch_url(url);
    if (!json) return NULL;
    cJSON *root = cJSON_Parse(json);
    free(json);
    if (!root) return NULL;
    cJSON *status = cJSON_GetObjectItem(root, "status");
    if (!status || strcmp(status->valuestring, "OK") != 0) { cJSON_Delete(root); return NULL; }
    cJSON *result = cJSON_GetObjectItem(root, "result");
    if (!result || !cJSON_IsArray(result) || cJSON_GetArraySize(result) == 0) { cJSON_Delete(root); return NULL; }
    cJSON *user = cJSON_GetArrayItem(result, 0);
    cJSON *user_copy = cJSON_Duplicate(user, 1);
    cJSON_Delete(root);
    return user_copy;
}

// 获取用户比赛记录
static cJSON* fetch_user_rating_json(const char *handle) {
    char url[512];
    snprintf(url, sizeof(url), "https://codeforces.com/api/user.rating?handle=%s", handle);
    char *json = fetch_url(url);
    if (!json) return NULL;
    cJSON *root = cJSON_Parse(json);
    free(json);
    if (!root) return NULL;
    cJSON *status = cJSON_GetObjectItem(root, "status");
    if (!status || strcmp(status->valuestring, "OK") != 0) { cJSON_Delete(root); return NULL; }
    cJSON *result = cJSON_GetObjectItem(root, "result");
    cJSON *copy = cJSON_Duplicate(result, 1);
    cJSON_Delete(root);
    return copy;
}

// 获取用户提交记录
static cJSON* fetch_user_status_json(const char *handle) {
    char url[512];
    snprintf(url, sizeof(url), "https://codeforces.com/api/user.status?handle=%s&from=1&count=5000", handle);
    char *json = fetch_url(url);
    if (!json) return NULL;
    cJSON *root = cJSON_Parse(json);
    free(json);
    if (!root) return NULL;
    cJSON *status = cJSON_GetObjectItem(root, "status");
    if (!status || strcmp(status->valuestring, "OK") != 0) { cJSON_Delete(root); return NULL; }
    cJSON *result = cJSON_GetObjectItem(root, "result");
    cJSON *copy = cJSON_Duplicate(result, 1);
    cJSON_Delete(root);
    return copy;
}

// ============ 主函数 ============
int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "用法: %s <用户ID列表文件>\n", argv[0]);
        fprintf(stderr, "示例: %s data/users.txt\n", argv[0]);
        return 1;
    }
    
    // 读取用户ID列表
    FILE *fp = fopen(argv[1], "r");
    if (!fp) { fprintf(stderr, "无法打开文件: %s\n", argv[1]); return 1; }
    
    char handles[100][64];
    int handle_count = 0;
    char line[256];
    
    while (fgets(line, sizeof(line), fp) && handle_count < 100) {
        char *p = line;
        while (*p == ' ' || *p == '\t') p++;
        int len = (int)strlen(p);
        while (len > 0 && (p[len-1] == '\n' || p[len-1] == '\r' || p[len-1] == ' ')) p[--len] = '\0';
        if (len > 0) {
            strncpy(handles[handle_count], p, 63);
            handles[handle_count][63] = '\0';
            handle_count++;
        }
    }
    fclose(fp);
    
    if (handle_count == 0) { fprintf(stderr, "文件中没有有效的用户ID\n"); return 1; }
    fprintf(stderr, "读取到 %d 个用户\n", handle_count);
    
    // 获取比赛列表
    fprintf(stderr, "正在获取比赛列表...\n");
    cJSON *contests = fetch_contests_json();
    
    // 构建JSON输出
    cJSON *output = cJSON_CreateObject();
    
    if (contests) {
        cJSON *status = cJSON_GetObjectItem(contests, "status");
        cJSON *result = cJSON_GetObjectItem(contests, "result");
        if (status && result && strcmp(status->valuestring, "OK") == 0) {
            cJSON_AddItemToObject(output, "contests", cJSON_Duplicate(result, 1));
        }
        cJSON_Delete(contests);
    }
    
    cJSON *users_array = cJSON_AddArrayToObject(output, "users");
    
    for (int h = 0; h < handle_count; h++) {
        const char *handle = handles[h];
        fprintf(stderr, "\n正在处理用户: %s\n", handle);
        
        cJSON *user_obj = cJSON_CreateObject();
        cJSON_AddStringToObject(user_obj, "handle", handle);
        
        // 获取用户信息
        fprintf(stderr, "  获取用户信息...\n");
        cJSON *info = fetch_user_info_json(handle);
        if (info) {
            cJSON_AddItemToObject(user_obj, "info", info);
            fprintf(stderr, "  成功\n");
        } else {
            fprintf(stderr, "  失败\n");
        }
        
        // 获取比赛记录
        fprintf(stderr, "  获取比赛记录...\n");
        cJSON *ratings = fetch_user_rating_json(handle);
        if (ratings) {
            cJSON_AddItemToObject(user_obj, "ratings", ratings);
            int rcount = cJSON_GetArraySize(ratings);
            fprintf(stderr, "  获取到 %d 场比赛记录\n", rcount);
        } else {
            fprintf(stderr, "  获取比赛记录失败\n");
        }
        
        // 获取提交记录
        fprintf(stderr, "  获取提交记录...\n");
        cJSON *status = fetch_user_status_json(handle);
        if (status) {
            cJSON_AddItemToObject(user_obj, "submissions", status);
            fprintf(stderr, "  成功\n");
        } else {
            fprintf(stderr, "  失败\n");
        }
        
        cJSON_AddItemToArray(users_array, user_obj);
    }
    
    // 输出JSON到文件
    char *json_str = cJSON_Print(output);
    if (json_str) {
        // 保存JSON数据文件
        FILE *out = fopen("data/cf_data.json", "w");
        if (out) {
            fputs(json_str, out);
            fclose(out);
            fprintf(stderr, "\n数据已保存到 data/cf_data.json (共 %zu 字节)\n", strlen(json_str));
        }
        // 同时生成JS文件供HTML直接加载（解决file://协议下跨域问题）
        FILE *js_out = fopen("output/cf_data.js", "w");
        if (js_out) {
            fputs("var CF_DATA = ", js_out);
            fputs(json_str, js_out);
            fputs(";\n", js_out);
            fclose(js_out);
            fprintf(stderr, "JS数据已保存到 output/cf_data.js\n");
        }
        free(json_str);
    }
    
    cJSON_Delete(output);
    fprintf(stderr, "\n完成！请打开 output/cf_report.html 查看报告\n");
    return 0;
}
