#ifdef _WIN32
#define _CRT_SECURE_NO_WARNINGS
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32")
typedef SOCKET socket_t;
#define close closesocket
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
typedef int socket_t;
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#define PORT 8080
#define MAX_PLAYERS 100
#define PLAYERS_FILE "players.txt"

typedef struct {
    char username[64];
    int level1_score;
    int level2_score;
    int level3_score; // <-- MODIFICATION: Added Level 3 score
    int total_score;
    int current_level;
} Player;

Player players[MAX_PLAYERS];
int player_count = 0;

// *** MODIFICATION: New Level 1 Questions (from mcq1.txt) ***
//
const char* L1_Q[] = {
    "Which data structure uses LIFO?", /* */
    "Which is used in recursion?", /* */
    "Traversal of BST giving sorted order?", /* */
    "Time complexity of binary search?", /* */
    "Data structure for BFS?", /* */
    "Sorting best for nearly sorted data?", /* */
    "In max heap, largest element is at?", /* */
    "Operation not possible in queue?", /* */
    "Data structure to check balanced parentheses?", /* */
    "Which is not stable sorting?"
};
const char* L1_OPTS[][4] = {
    {"Queue", "Stack", "Array", "Linked List"}, /* */
    {"Queue", "Stack", "Linked List", "Array"}, /* */
    {"Preorder", "Inorder", "Postorder", "Level order"}, /* */
    {"O(n)", "O(log n)", "O(n^2)", "O(1)"}, /* */
    {"Stack", "Queue", "Tree", "Heap"}, /* */
    {"Merge Sort", "Insertion Sort", "Bubble Sort", "Selection Sort"}, /* */
    {"Left child", "Right child", "Root node", "Leaf node"}, /* */
    {"Insertion at rear", "Deletion from front", "Deletion from rear", "Traversing"}, /* */
    {"Queue", "Array", "Stack", "Linked List"}, /* */
    {"Bubble", "Insertion", "Quick", "Merge"}
};
// Answers converted from ASCII: B=66->1, C=67->2
int L1_ANS[] = {1, 1, 1, 1, 1, 1, 2, 2, 2, 2};

// *** MODIFICATION: New Level 2 Questions (from mcq2.txt) ***
//
const char* L2_Q[] = {
    "Best case Quick Sort?", /* */
    "Uses hashing?", /* */
    "Tree for DB indexing?", /* */
    "Max nodes in binary tree of height h?", /* */
    "Not a linear DS?", /* */
    "Space complexity of recursive factorial?", /* */
    "Traversal using stack?", /* */
    "Algorithm for shortest path?", /* */
    "Rear & front equal in circular queue when?", /* */
    "DS for function call management?"
};
const char* L2_OPTS[][4] = {
    {"O(n log n)", "O(n^2)", "O(log n)", "O(n)"}, /* */
    {"Linked List", "Stack", "Hash Table", "Queue"}, /* */
    {"Binary", "AVL", "B-Tree", "Heap"}, /* */
    {"2^h", "2^(h+1)-1", "2^(h-1)", "h^2"}, /* */
    {"Array", "Stack", "Queue", "Graph"}, /* */
    {"O(1)", "O(n)", "O(log n)", "O(n^2)"}, /* */
    {"Inorder", "Level order", "Postorder", "BFS"}, /* */
    {"Kruskal", "Dijkstra", "Prim", "Floyd–Warshall"}, /* */
    {"Full", "Empty", "One element", "None"}, /* */
    {"Queue", "Stack", "Array", "Tree"}
};
// Answers converted from ASCII: A=65->0, B=66->1, C=67->2, D=68->3
int L2_ANS[] = {0, 2, 2, 1, 3, 1, 0, 1, 1, 1};


// Level 3 LeetCode problems (Unchanged)
const char* L3_PROBLEMS[][3] = {
    {"Two Sum", "https://leetcode.com/problems/two-sum/", "Easy"},
    {"Add Two Numbers", "https://leetcode.com/problems/add-two-numbers/", "Medium"},
    {"Longest Substring", "https://leetcode.com/problems/longest-substring-without-repeating-characters/", "Medium"},
    {"Median of Two Sorted Arrays", "https://leetcode.com/problems/median-of-two-sorted-arrays/", "Hard"},
    {"Valid Parentheses", "https://leetcode.com/problems/valid-parentheses/", "Easy"},
    {"Merge Two Sorted Lists", "https://leetcode.com/problems/merge-two-sorted-lists/", "Easy"},
    {"Binary Tree Inorder", "https://leetcode.com/problems/binary-tree-inorder-traversal/", "Easy"},
    {"Maximum Subarray", "https://leetcode.com/problems/maximum-subarray/", "Medium"},
    {"Climbing Stairs", "https://leetcode.com/problems/climbing-stairs/", "Easy"},
    {"Reverse Linked List", "https://leetcode.com/problems/reverse-linked-list/", "Easy"}
};

void load_players() {
    FILE* f = fopen(PLAYERS_FILE, "r");
    if (!f) return;
    
    // MODIFICATION: Read 6 values now (added level3_score)
    while (fscanf(f, "%63[^,],%d,%d,%d,%d,%d\n", 
                  players[player_count].username,
                  &players[player_count].level1_score,
                  &players[player_count].level2_score,
                  &players[player_count].level3_score, // <-- Read L3 score
                  &players[player_count].total_score,
                  &players[player_count].current_level) == 6) { // <-- Check for 6 values
        player_count++;
        if (player_count >= MAX_PLAYERS) break;
    }
    fclose(f);
}

void save_players() {
    FILE* f = fopen(PLAYERS_FILE, "w");
    if (!f) return;
    
    for (int i = 0; i < player_count; i++) {
        // MODIFICATION: Write 6 values now (added level3_score)
        fprintf(f, "%s,%d,%d,%d,%d,%d\n",
                players[i].username,
                players[i].level1_score,
                players[i].level2_score,
                players[i].level3_score, // <-- Write L3 score
                players[i].total_score,
                players[i].current_level);
    }
    fclose(f);
}

int find_player(const char* username) {
    for (int i = 0; i < player_count; i++) {
        if (strcmp(players[i].username, username) == 0) return i;
    }
    return -1;
}

void url_decode(char* dst, const char* src) {
    while (*src) {
        if (*src == '%' && isxdigit(src[1]) && isxdigit(src[2])) {
            char hex[3] = {src[1], src[2], 0};
            *dst++ = (char)strtol(hex, NULL, 16);
            src += 3;
        } else if (*src == '+') {
            *dst++ = ' ';
            src++;
        } else {
            *dst++ = *src++;
        }
    }
    *dst = '\0';
}

char* get_param(const char* body, const char* key) {
    if (!body || !key) return NULL;
    char* buf = strdup(body);
    char* tok = strtok(buf, "&");
    while (tok) {
        char* eq = strchr(tok, '=');
        if (eq) {
            *eq = '\0';
            if (strcmp(tok, key) == 0) {
                char* val = strdup(eq + 1);
                free(buf);
                char* decoded = malloc(strlen(val) + 1);
                url_decode(decoded, val);
                free(val);
                return decoded;
            }
        }
        tok = strtok(NULL, "&");
    }
    free(buf);
    return NULL;
}

void send_response(socket_t client, const char* body) {
    char header[512];
    // MODIFICATION: Added CORS headers for methods and headers
    snprintf(header, sizeof(header),
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: application/json\r\n"
        "Access-Control-Allow-Origin: *\r\n"
        "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n"
        "Access-Control-Allow-Headers: Content-Type\r\n"
        "Content-Length: %d\r\n\r\n", (int)strlen(body));
    send(client, header, strlen(header), 0);
    send(client, body, strlen(body), 0);
}

// MODIFICATION: Added function to handle CORS preflight OPTIONS requests
void send_options_response(socket_t client) {
    const char* header =
        "HTTP/1.1 204 No Content\r\n"
        "Access-Control-Allow-Origin: *\r\n"
        "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n"
        "Access-Control-Allow-Headers: Content-Type\r\n"
        "Access-Control-Max-Age: 86400\r\n"
        "\r\n";
    send(client, header, strlen(header), 0);
}


void send_file(socket_t client, const char* path) {
    char filepath[256];
    if (strcmp(path, "/") == 0) {
        strcpy(filepath, "index.html");
    } else {
        snprintf(filepath, sizeof(filepath), "%s", path + 1);
    }
    
    FILE* f = fopen(filepath, "rb");
    if (!f) {
        const char* resp = "HTTP/1.1 404 Not Found\r\n\r\n";
        send(client, resp, strlen(resp), 0);
        return;
    }
    
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);
    
    char* content = malloc(size);
    fread(content, 1, size, f);
    fclose(f);
    
    const char* mime = "text/html";
    if (strstr(filepath, ".css")) mime = "text/css";
    else if (strstr(filepath, ".js")) mime = "application/javascript";
    
    char header[512];
    snprintf(header, sizeof(header),
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: %s\r\n"
        "Content-Length: %ld\r\n\r\n", mime, size);
    send(client, header, strlen(header), 0);
    send(client, content, size, 0);
    free(content);
}

void handle_register(socket_t client, const char* body) {
    char* name = get_param(body, "name");
    if (!name || strlen(name) == 0) {
        send_response(client, "{\"status\":\"error\",\"message\":\"Username required\"}");
        free(name);
        return;
    }
    
    if (find_player(name) >= 0) {
        send_response(client, "{\"status\":\"error\",\"message\":\"Username already exists\"}");
        free(name);
        return;
    }
    
    strcpy(players[player_count].username, name);
    players[player_count].level1_score = 0;
    players[player_count].level2_score = 0;
    players[player_count].level3_score = 0; // <-- MODIFICATION: Init L3 score
    players[player_count].total_score = 0;
    players[player_count].current_level = 1;
    player_count++;
    save_players();
    
    send_response(client, "{\"status\":\"ok\",\"message\":\"Registration successful\"}");
    free(name);
}

void handle_login(socket_t client, const char* body) {
    char* name = get_param(body, "name");
    if (!name) {
        send_response(client, "{\"status\":\"error\",\"message\":\"Username required\"}");
        return;
    }
    
    int idx = find_player(name);
    free(name);
    
    if (idx < 0) {
        send_response(client, "{\"status\":\"error\",\"message\":\"User not found\"}");
        return;
    }
    
    char resp[256];
    // MODIFICATION: Added level3_score to login response
    snprintf(resp, sizeof(resp),
        "{\"status\":\"ok\",\"current_level\":%d,\"level1_score\":%d,\"level2_score\":%d,\"level3_score\":%d,\"total_score\":%d}",
        players[idx].current_level,
        players[idx].level1_score,
        players[idx].level2_score,
        players[idx].level3_score, // <-- Send L3 score
        players[idx].total_score);
    send_response(client, resp);
}

void handle_questions(socket_t client, const char* body) {
    char* name = get_param(body, "name");
    char* level_str = get_param(body, "level");
    
    if (!name || !level_str) {
        send_response(client, "{\"error\":\"Missing parameters\"}");
        free(name); free(level_str);
        return;
    }
    
    int level = atoi(level_str);
    int idx = find_player(name);
    
    if (idx < 0) {
        send_response(client, "{\"error\":\"User not found\"}");
        free(name); free(level_str);
        return;
    }
    
    // Check if level is unlocked
    // NOTE: This logic allows retrying a level you have already passed.
    // It only blocks access to *new* levels.
    if (level > players[idx].current_level) {
        send_response(client, "{\"error\":\"Level is locked.\"}");
        free(name); free(level_str);
        return;
    }

    
    char json[16384] = "[";
    int pos = 1;
    
    // Select question set based on level
    const char** questions = NULL;
    const char* (*options)[4] = NULL;
    int* answers = NULL;
    
    if (level == 1) {
        questions = L1_Q;
        options = L1_OPTS;
        answers = L1_ANS;
    } else if (level == 2) { 
        questions = L2_Q;
        options = L2_OPTS;
        answers = L2_ANS;
    } else if (level == 3) { 
        // Level 3 is handled by GET /api/level3
        send_response(client, "{\"error\":\"Level 3 is not an MCQ quiz. Use GET /api/level3 to see problems.\"}");
        free(name); free(level_str);
        return;
    } else { 
        send_response(client, "{\"error\":\"Invalid level specified.\"}");
        free(name); free(level_str);
        return;
    }
    
    for (int i = 0; i < 10; i++) {
        pos += snprintf(json + pos, sizeof(json) - pos,
            "{\"id\":%d,\"q\":\"%s\",\"opts\":[\"%s\",\"%s\",\"%s\",\"%s\"]}",
            i, questions[i],
            options[i][0], options[i][1], options[i][2], options[i][3]);
        if (i < 9) json[pos++] = ',';
    }
    json[pos++] = ']';
    json[pos] = '\0';
    
    send_response(client, json);
    free(name); free(level_str);
}

void handle_submit(socket_t client, const char* body) {
    char* name = get_param(body, "name");
    char* level_str = get_param(body, "level");
    char* qid_str = get_param(body, "qid");
    char* ans_str = get_param(body, "ans");
    
    if (!name || !level_str || !qid_str || !ans_str) {
        send_response(client, "{\"correct\":false}");
        free(name); free(level_str); free(qid_str); free(ans_str);
        return;
    }
    
    int level = atoi(level_str);
    int qid = atoi(qid_str);
    int ans = atoi(ans_str);
    int idx = find_player(name);
    
    if (idx < 0 || qid < 0 || qid >= 10) {
        send_response(client, "{\"correct\":false}");
        free(name); free(level_str); free(qid_str); free(ans_str);
        return;
    }
    
    int* correct_ans = NULL;
 
    if (level == 1) {
        correct_ans = L1_ANS;
    } else if (level == 2) { 
        correct_ans = L2_ANS;
    } else { 
        // No MCQs to submit for level 3
        send_response(client, "{\"correct\":false, \"error\":\"Invalid level for submission\"}");
        free(name); free(level_str); free(qid_str); free(ans_str);
        return;
    }
    
    int correct = (correct_ans[qid] == ans);
    send_response(client, correct ? "{\"correct\":true}" : "{\"correct\":false}");
    
    free(name); free(level_str); free(qid_str); free(ans_str);
}

void handle_finish(socket_t client, const char* body) {
    char* name = get_param(body, "name");
    char* level_str = get_param(body, "level");
    char* score_str = get_param(body, "score");
    
    if (!name || !level_str || !score_str) {
        send_response(client, "{\"message\":\"Invalid request\",\"next_level\":null}");
        free(name); free(level_str); free(score_str);
        return;
    }
    
    int level = atoi(level_str);
    int score = atoi(score_str);
    int idx = find_player(name);
    
    if (idx < 0) {
        send_response(client, "{\"message\":\"User not found\",\"next_level\":null}");
        free(name); free(level_str); free(score_str);
        return;
    }
    
    // Only update score if it's a new high score for that level
    if (level == 1 && score > players[idx].level1_score) {
        players[idx].level1_score = score;
    } else if (level == 2 && score > players[idx].level2_score) {
        players[idx].level2_score = score;
    }
    
    // MODIFICATION: Recalculate total score based on all levels
    players[idx].total_score = players[idx].level1_score + players[idx].level2_score + players[idx].level3_score;
    
    char msg[512];
    int next = 0;
    int retry = 0;
    
    if (score >= 7) {
        if (level < 3) {
            next = level + 1;
            // Only update current_level if it's an advancement
            if (next > players[idx].current_level) {
                players[idx].current_level = next;
            }
            snprintf(msg, sizeof(msg), "Congratulations! You scored %d/10. Level %d unlocked!", score, next);
        } else {
            snprintf(msg, sizeof(msg), "Congratulations! You completed all quiz levels with %d/10!", score);
        }
    } else {
        retry = level; 
        snprintf(msg, sizeof(msg), "You scored %d/10. You need >=7 to unlock next level. Try again!", score);
    }
    
    save_players();
    
    char resp[1024];
    char next_level_str[10];
    char retry_level_str[10]; 

    // Send back the *highest* level unlocked, not just the next one
    snprintf(next_level_str, sizeof(next_level_str), "%d", players[idx].current_level);

    if (retry > 0) {
        snprintf(retry_level_str, sizeof(retry_level_str), "%d", retry);
    } else {
        strcpy(retry_level_str, "null");
    }

    // MODIFICATION: Added all scores to JSON response
    snprintf(resp, sizeof(resp),
        "{\"message\":\"%s\",\"next_level\":%s,\"retry_level\":%s,\"total_score\":%d,\"level1_score\":%d,\"level2_score\":%d,\"level3_score\":%d}",
        msg, next_level_str, retry_level_str, players[idx].total_score, players[idx].level1_score, players[idx].level2_score, players[idx].level3_score);
    send_response(client, resp);
    
    free(name); free(level_str); free(score_str);
}

void handle_level3(socket_t client) {
    char json[4096] = "[";
    int pos = 1;
    
    for (int i = 0; i < 10; i++) {
        pos += snprintf(json + pos, sizeof(json) - pos,
            "{\"title\":\"%s\",\"url\":\"%s\",\"difficulty\":\"%s\"}",
            L3_PROBLEMS[i][0], L3_PROBLEMS[i][1], L3_PROBLEMS[i][2]);
        if (i < 9) json[pos++] = ',';
    }
    json[pos++] = ']';
    json[pos] = '\0';
    
    send_response(client, json);
}

// *** MODIFICATION: New function to handle L3 completion ***
void handle_level3_complete(socket_t client, const char* body) {
    char* name = get_param(body, "name");
    if (!name) {
        send_response(client, "{\"status\":\"error\",\"message\":\"Username required\"}");
        return;
    }
    
    int idx = find_player(name);
    if (idx < 0) {
        send_response(client, "{\"status\":\"error\",\"message\":\"User not found\"}");
        free(name);
        return;
    }

    // <-- MODIFICATION: Award 1 point for L3 -->
    players[idx].level3_score = 1;
    
    // Recalculate total score
    players[idx].total_score = players[idx].level1_score + players[idx].level2_score + players[idx].level3_score;
    
    save_players();
    
    char resp[512];
    // <-- MODIFICATION: Updated message to "1 point" -->
    snprintf(resp, sizeof(resp),
        "{\"status\":\"ok\",\"message\":\"Congratulations! You earned 1 point for completing Level 3!\",\"total_score\":%d}",
        players[idx].total_score);
    send_response(client, resp);
    
    free(name);
}
// *** END MODIFICATION ***

void handle_leaderboard(socket_t client) {
    Player sorted[MAX_PLAYERS];
    memcpy(sorted, players, sizeof(Player) * player_count);
    
    for (int i = 0; i < player_count - 1; i++) {
        for (int j = 0; j < player_count - i - 1; j++) {
            if (sorted[j].total_score < sorted[j + 1].total_score) {
                Player temp = sorted[j];
                sorted[j] = sorted[j + 1];
                sorted[j + 1] = temp;
            }
        }
    }
    
    char json[8192] = "[";
    int pos = 1;
    
    for (int i = 0; i < player_count && i < 10; i++) {
        pos += snprintf(json + pos, sizeof(json) - pos,
            "{\"username\":\"%s\",\"score\":%d}",
            sorted[i].username, sorted[i].total_score);
        if (i < player_count - 1 && i < 9) json[pos++] = ',';
    }
    json[pos++] = ']';
    json[pos] = '\0';
    
    send_response(client, json);
}

void handle_client(socket_t client) {
    char buffer[8192];
    int n = recv(client, buffer, sizeof(buffer) - 1, 0);
    if (n <= 0) return;
    buffer[n] = '\0';
    
    char method[16], path[256];
    sscanf(buffer, "%s %s", method, path);
    
    // *** MODIFICATION: Strip query string from path ***
    char* q_mark = strchr(path, '?');
    if (q_mark) {
        *q_mark = '\0'; // Truncate the path string at the '?'
    }
    // *** END MODIFICATION ***
    
    char* body = strstr(buffer, "\r\n\r\n");
    if (body) body += 4;
    else body = "";
    
    // *** MODIFICATION: Handle CORS preflight requests ***
    if (strcmp(method, "OPTIONS") == 0) {
        send_options_response(client);
        return;
    }
    // *** END MODIFICATION ***

    if (strcmp(method, "GET") == 0) {
        if (strcmp(path, "/api/level3") == 0) {
            handle_level3(client);
        } else if (strcmp(path, "/api/leaderboard") == 0) {
            handle_leaderboard(client);
        } else {
            send_file(client, path);
        }
    } else if (strcmp(method, "POST") == 0) {
        if (strcmp(path, "/api/register") == 0) {
            handle_register(client, body);
        } else if (strcmp(path, "/api/login") == 0) {
            handle_login(client, body);
        } else if (strcmp(path, "/api/questions") == 0) {
            handle_questions(client, body);
        } else if (strcmp(path, "/api/submit_answer") == 0) {
            handle_submit(client, body);
        } else if (strcmp(path, "/api/finish_level") == 0) {
            handle_finish(client, body);
        }
        // *** MODIFICATION: Added route for L3 completion ***
        else if (strcmp(path, "/api/level3_complete") == 0) {
            handle_level3_complete(client, body);
        }
        // *** END MODIFICATION ***
    }
}

int main() {
    srand(time(NULL));
    load_players();
    
#ifdef _WIN32
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);
#endif
    
    socket_t server = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(server, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt));
    
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(PORT);
    
    bind(server, (struct sockaddr*)&addr, sizeof(addr));
    listen(server, 10);
    
    printf("BrainForge server running on http://localhost:%d\n", PORT);
    
    while (1) {
        struct sockaddr_in cli;
        socklen_t len = sizeof(cli);
        socket_t client = accept(server, (struct sockaddr*)&cli, &len);
        handle_client(client);
        close(client);
    }
    
    return 0;
}