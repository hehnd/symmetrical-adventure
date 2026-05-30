#ifndef CF_API_H
#define CF_API_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// 颜色定义 - Codeforces等级分对应颜色
typedef struct {
    int rating;
    const char *color;
    const char *title;
} RatingColor;

static const RatingColor rating_colors[] = {
    {0,    "#808080", "未参赛"},      // Gray - Unrated
    {1200, "#808080", "Newbie"},       // Gray
    {1400, "#008000", "Pupil"},        // Green
    {1600, "#03A89E", "Specialist"},   // Cyan
    {1900, "#0000FF", "Expert"},       // Blue
    {2100, "#AA00AA", "Candidate Master"}, // Purple
    {2300, "#FF8C00", "Master"},       // Orange
    {2400, "#FF8C00", "International Master"}, // Orange
    {2600, "#FF0000", "Grandmaster"},  // Red
    {3000, "#FF0000", "International Grandmaster"}, // Red
    {4000, "#CC0000", "Legendary Grandmaster"}, // Dark Red
    {9999, "#CC0000", "Legendary Grandmaster"}, // Dark Red
};

// 获取等级分对应颜色
static const char* get_rating_color(int rating) {
    int i;
    for (i = 0; i < sizeof(rating_colors)/sizeof(rating_colors[0]) - 1; i++) {
        if (rating >= rating_colors[i].rating && rating < rating_colors[i+1].rating) {
            return rating_colors[i].color;
        }
    }
    return "#808080";
}

// 获取等级分对应头衔
static const char* get_rating_title(int rating) {
    int i;
    for (i = 0; i < sizeof(rating_colors)/sizeof(rating_colors[0]) - 1; i++) {
        if (rating >= rating_colors[i].rating && rating < rating_colors[i+1].rating) {
            return rating_colors[i].title;
        }
    }
    return "Unrated";
}

// 用户信息结构体
typedef struct {
    char handle[64];
    char title[64];
    char avatar[256];
    int rating;
    int max_rating;
    int contribution;
    int friend_of_count;
    int rank;
    int max_rank;
    long long last_online;
    long long registration_time;
} UserInfo;

// 比赛信息结构体
typedef struct {
    int id;
    char name[256];
    int type; // 0: CF, 1: ICPC
    int phase; // 0: BEFORE, 1: CODING, 2: PENDING_SYSTEM_TEST, 3: SYSTEM_TEST, 4: FINISHED
    long long start_time;
    int duration_seconds;
    int difficulty;
} ContestInfo;

// 用户比赛记录结构体
typedef struct {
    int contest_id;
    char contest_name[256];
    long long contest_time;
    int old_rating;
    int new_rating;
    int rank;
    int rating_change;
    double points;
    int problem_count;
    char **problem_indices;    // 题目索引 A, B, C...
    double *problem_points;    // 各题得分
    int *problem_solved;       // 是否通过
    int upsolved_count;        // 补题数量
    char **upsolved_indices;   // 补题索引
} UserContestRecord;

// 提交记录结构体
typedef struct {
    int id;
    int contest_id;
    char problem_index[8];
    char verdict[32];
    long long submit_time;
    int passed_test_count;
} Submission;

// 用户题目通过统计
typedef struct {
    char index[8];
    int rating;
    int solved_count;
    int solved_1year;
    int solved_180days;
    int solved_30days;
} ProblemRatingStat;

// 内存安全写入回调
static size_t write_callback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    char **buffer = (char**)userp;
    
    if (*buffer == NULL) {
        *buffer = (char*)malloc(realsize + 1);
        if (*buffer == NULL) return 0;
        memcpy(*buffer, contents, realsize);
        (*buffer)[realsize] = '\0';
        return realsize;
    }
    
    size_t old_len = strlen(*buffer);
    char *new_buffer = (char*)realloc(*buffer, old_len + realsize + 1);
    if (new_buffer == NULL) return 0;
    *buffer = new_buffer;
    memcpy(*buffer + old_len, contents, realsize);
    (*buffer)[old_len + realsize] = '\0';
    return realsize;
}

// URL编码
static char* url_encode(const char *str) {
    size_t len = strlen(str);
    char *encoded = (char*)malloc(len * 3 + 1);
    if (encoded == NULL) return NULL;
    
    size_t j = 0;
    for (size_t i = 0; i < len; i++) {
        unsigned char c = (unsigned char)str[i];
        if (c == ' ' || c == ',' || c == ';') {
            sprintf(encoded + j, "%%%02X", c);
            j += 3;
        } else {
            encoded[j++] = c;
        }
    }
    encoded[j] = '\0';
    return encoded;
}

// 时间戳转字符串
static void timestamp_to_str(long long ts, char *buf, size_t buf_size) {
    time_t t = (time_t)ts;
    struct tm *tm_info = localtime(&t);
    strftime(buf, buf_size, "%Y-%m-%d %H:%M", tm_info);
}

// 获取当前时间戳（秒）
static long long get_current_timestamp() {
    return (long long)time(NULL);
}

// 判断是否在180天内
static int is_within_days(long long timestamp, int days) {
    long long now = get_current_timestamp();
    return (now - timestamp) <= (long long)days * 24 * 3600;
}

#endif
