#include "nlohmann/json.hpp"
#include <cctype>
#include <cmath>
#include <fcntl.h>
#include <algorithm>
#include <cstdint>
#include <fstream>
#include <map>
#include <string>
#include <utility>
#include <vector>
#include <unistd.h>
#include <sys/wait.h>

using namespace std;
using json = nlohmann::json;

struct relevant {
    uint64_t id;
    string username;
    string content;
};

struct author {
    uint64_t author_id;
    string username;
    bool is_bot;
};

struct message {
    uint64_t id;
    string content;
    author author;
};

map<string, uint64_t> leaderboard;

string authorization;

const uint64_t MEE6_ID = 159985870458322944;
uint64_t channel_id;
uint64_t limit;
uint64_t from;
uint64_t to;
uint64_t max_threads;
uint64_t chunks;
uint64_t vector_reserve;
uint64_t prev_id = MEE6_ID; //default for no-collide.
uint64_t prev_m_id;

vector<pair<string,pair<uint64_t, uint64_t>>> multisend;
vector<pair<uint64_t,string>> not_num;
vector<pair<pair<uint64_t, string>, pair<uint64_t, string>>> violations;
vector<relevant> msg;

void from_json(const json& j, author& a) {
    a.author_id = stoull(j.at("id").get<string>());
    a.username = j.at("username").get<string>();
    if (j.contains("bot")) {
        a.is_bot = j.at("bot").get<bool>();
    } else a.is_bot = false;
}

void from_json(const json& j, message& m) {
    m.id = stoull(j.at("id").get<string>());
    m.content = j.at("content").get<string>();
    m.author = j.at("author").get<author>();
}

inline string create_fetch_before_link(uint64_t channel, uint64_t before, uint64_t limit) {
    return "https://ptb.discord.com/api/v9/channels/" + to_string(channel) + "/messages?before=" + to_string(before) + "&limit=" + to_string(limit);
}

string get_channel_name() {
    pid_t pid = fork();
    if (pid == 0) {
        int fd = open(((string) "meta.txt").c_str(), O_CREAT|O_TRUNC|O_WRONLY, 0644);
        dup2(fd, STDOUT_FILENO);
        close(fd);
        execlp("curl", "curl",
            "-s",
            "-D", "headers.txt",
            "-o", "response.json",
            "-w", "%{http_code}\n",
            "-H", ("authorization: " + authorization).c_str(),
            create_fetch_before_link(channel_id, before, limit).c_str(),
            nullptr);
        exit(-1);
    } else {
        // parent
        int status;
        waitpid(pid, &status, 0);
        if (!WIFEXITED(status)) exit(-1);
        int exit_code = WEXITSTATUS(status);
        if (exit_code) exit(-1);
        ifstream reader("meta.txt");
        int code;
        reader >> code;
        json body;
        if (code == 200) {
            ifstream f("response.json");
            f >> body;
            vector<message> monosodium_glutamate = body.get<vector<message>>();
            if (!monosodium_glutamate.size()) return;
            uint64_t nxt = before;
            sort(monosodium_glutamate.begin(), monosodium_glutamate.end(), [](const struct message& a, const struct message& b){ return a.id > b.id; });
            for (message& m : monosodium_glutamate) {
                nxt = min(m.id, nxt);
                if (m.id < from || m.author.author_id == MEE6_ID) continue;
                ++leaderboard[m.author.username];
                if (!all_of(m.content.begin(), m.content.end(), ::isdigit)) {
                    not_num.emplace_back(m.id, m.content);
                    continue;
                }
                msg.push_back({
                    m.id,
                    m.author.username,
                    m.content
                });
                if (m.author.author_id == prev_id) multisend.push_back({
                    m.author.username,
                    {
                        prev_m_id,
                        m.id
                    }
                });
                prev_id = m.author.author_id;
                prev_m_id = m.id;
            }
            recurse(nxt);
            return;
        } else if (code == 429) {
            ifstream f("response.json");
            f >> body;
            if (body.contains("retry_after")) {
                sleep((int) ceil(body.at("retry_after").get<long double>()));
                recurse(before);
                return;
            }
        } else {
            exit(-1);
        }
    }
}

string generate_leaderboard(int l, int r) {
    vector<pair<uint64_t, string>> nya;
    string payload = format("```\nRange: [{}, {}]\nChannel: {}\n", l, r, get_channel_name());
    size_t messages_sent_width = ((string) "Messages Sent").size();
    size_t username_width = ((string)"Username").size();
    for (auto& [uwu, owo] : leaderboard) nya.emplace_back(owo,uwu), username_width = max(username_width, uwu.size()), messages_sent_width = max(messages_sent_width, (size_t)ceil(log10(owo+1)));
    sort(nya.begin(), nya.end());
    reverse(nya.begin(), nya.end());
    size_t rank_width = max(((string)"Rank").size(), (size_t)(1+ceil(log10(1+nya.size()))));
    string bar = format("{:>{}} {:<{}} | {:<{}}", "Rank", rank_width, "Username", username_width, "Messages Sent", messages_sent_width);
    string top = format("{:^{}}", "Leaderboard", bar.size() + (bar.size()&1));
    payload += top + "\n" + bar + "\n";
    int i = 1;
    for (auto [uwu, owo] : nya) payload += format("{:>{}}. {:<{}} | {:<{}}\n", i++, rank_width-1, owo, username_width, uwu, messages_sent_width);
    payload += "```";
    return payload;
}

void recurse(uint64_t before) {
    if (before <= from) return;
    pid_t pid = fork();
    if (pid == 0) {
        int fd = open(((string) "meta.txt").c_str(), O_CREAT|O_TRUNC|O_WRONLY, 0644);
        dup2(fd, STDOUT_FILENO);
        close(fd);
        execlp("curl", "curl",
            "-s",
            "-D", "headers.txt",
            "-o", "response.json",
            "-w", "%{http_code}\n",
            "-H", ("authorization: " + authorization).c_str(),
            create_fetch_before_link(channel_id, before, limit).c_str(),
            nullptr);
        exit(-1);
    } else {
        // parent
        int status;
        waitpid(pid, &status, 0);
        if (!WIFEXITED(status)) exit(-1);
        int exit_code = WEXITSTATUS(status);
        if (exit_code) exit(-1);
        ifstream reader("meta.txt");
        int code;
        reader >> code;
        json body;
        if (code == 200) {
            ifstream f("response.json");
            f >> body;
            vector<message> monosodium_glutamate = body.get<vector<message>>();
            if (!monosodium_glutamate.size()) return;
            uint64_t nxt = before;
            sort(monosodium_glutamate.begin(), monosodium_glutamate.end(), [](const struct message& a, const struct message& b){ return a.id > b.id; });
            for (message& m : monosodium_glutamate) {
                nxt = min(m.id, nxt);
                if (m.id < from || m.author.author_id == MEE6_ID) continue;
                ++leaderboard[m.author.username];
                if (!all_of(m.content.begin(), m.content.end(), ::isdigit)) {
                    not_num.emplace_back(m.id, m.content);
                    continue;
                }
                msg.push_back({
                    m.id,
                    m.author.username,
                    m.content
                });
                if (m.author.author_id == prev_id) multisend.push_back({
                    m.author.username,
                    {
                        prev_m_id,
                        m.id
                    }
                });
                prev_id = m.author.author_id;
                prev_m_id = m.id;
            }
            recurse(nxt);
            return;
        } else if (code == 429) {
            ifstream f("response.json");
            f >> body;
            if (body.contains("retry_after")) {
                sleep((int) ceil(body.at("retry_after").get<long double>()));
                recurse(before);
                return;
            }
        } else {
            exit(-1);
        }
    }

}
//note,. only required things for post are data-raw, content-type, and authorization

bool send_payload(string payload) {

}

void init() {
    ifstream f("info.json");
    json info;
    f >> info;
    channel_id = info.at("channel_id").get<uint64_t>();
    authorization = info.at("authorization").get<string>();
    limit = info.at("limit").get<uint64_t>();
    from = info.at("range").at("from_id").get<uint64_t>();
    to = info.at("range").at("to_id").get<uint64_t>();
    max_threads = info.at("max_threads").get<uint64_t>();
    chunks = info.at("chunks").get<uint64_t>();
    vector_reserve = info.at("reserve").get<uint64_t>();
}

signed main() {
    init();
    msg.reserve(vector_reserve);
    recurse(to);
    unsigned long long mx = 1;
    sort(msg.begin(), msg.end(), [](const relevant& a, const relevant& b){ return a.id < b.id; });
    for (int i = 1; i < msg.size(); ++i) {
        if (stoull(msg[i-1].content) != stoull(msg[i].content)-1) {
            violations.push_back({
                {
                    msg[i-1].id,
                    msg[i-1].content
                },
                {
                    msg[i].id,
                    msg[i].content
                }
            });
        }
        mx = max(mx, stoull(msg[i].content));
    }
    string payload = "";
    if (violations.size()) for (auto violation : violations) payload += format(
            "Counting Violation: '{}' -> '{}' msg_ids: {{{},{}}}\n",
            violation.first.second,
            violation.second.second,
            violation.first.first,
            violation.second.first
        );
    else payload.append("No counting violations!\n");
    if (not_num.size()) for (auto idiot : not_num) payload += format(
            "Not a number: '{}' msg_id: {}\n",
            idiot.second,
            idiot.first
        );
    else payload.append("Everything is a number!\n");
    if (multisend.size()) for (auto idiot : multisend) payload += format(
            "{} is an idiot and sent messages {} and {} back to back!\n",
            idiot.first,
            idiot.second.first,
            idiot.second.second
        );
    if (violations.size() || not_num.size() || multisend.size());
    else payload.append(generate_leaderboard(1, mx));
    for (int i = 0; i++ < 3 && !send_payload(payload););
    return 0;
}