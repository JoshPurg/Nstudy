// ═══════════════════════════════════════════════════════════
//  NStudy Hub — NJUPT Xianlin Campus Study Assistant
//  Runnable C++ console app. No server. No database.
//  Compile: g++ -std=c++17 -o nstudy nstudy.cpp
//  Run:     ./nstudy
// ═══════════════════════════════════════════════════════════

#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <algorithm>
#include <iomanip>
#include <ctime>
#include <sstream>
#include <limits>
#include <random>

using namespace std;

// ───────────────────────────────────────────────
// Utility helpers
// ───────────────────────────────────────────────
void clearInput() {
    cin.clear();
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
}

string getLine(const string& prompt) {
    cout << prompt;
    string s;
    getline(cin, s);
    return s;
}

string timeNow() {
    time_t now = time(nullptr);
    struct tm* t = localtime(&now);
    char buf[20];
    strftime(buf, sizeof(buf), "%H:%M", t);
    return string(buf);
}

int hourNow() {
    time_t now = time(nullptr);
    struct tm* t = localtime(&now);
    return t->tm_hour;
}

void printLine(char c = '-', int n = 60) {
    cout << string(n, c) << "\n";
}

void printHeader(const string& title) {
    printLine('=');
    cout << "  NStudy Hub  //  " << title << "\n";
    printLine('=');
}

void pause() {
    cout << "\n  Press ENTER to continue...";
    clearInput();
}

// ───────────────────────────────────────────────
// Data Structures
// ───────────────────────────────────────────────

enum class CrowdLevel { EMPTY, QUIET, MODERATE, BUSY, FULL };

string crowdLabel(CrowdLevel c) {
    switch (c) {
        case CrowdLevel::EMPTY:    return "EMPTY   ";
        case CrowdLevel::QUIET:    return "QUIET   ";
        case CrowdLevel::MODERATE: return "MODERATE";
        case CrowdLevel::BUSY:     return "BUSY    ";
        case CrowdLevel::FULL:     return "FULL    ";
    }
    return "UNKNOWN ";
}

string crowdIcon(CrowdLevel c) {
    switch (c) {
        case CrowdLevel::EMPTY:    return "[ ○○○○○ ]";
        case CrowdLevel::QUIET:    return "[ ●○○○○ ]";
        case CrowdLevel::MODERATE: return "[ ●●○○○ ]";
        case CrowdLevel::BUSY:     return "[ ●●●○○ ]";
        case CrowdLevel::FULL:     return "[ ●●●●● ]";
    }
    return "[       ]";
}

// ── Study Area ──
struct StudyArea {
    int id;
    string name;
    string building;
    string floor;
    bool isLibrary;
    bool hasOutlet;
    bool hasWifi;
    bool isQuiet;
    string baiduMapLink;   // deep link for navigation
    CrowdLevel crowd;
    string lastReportedBy;
    string lastReportTime;
    int reportCount;

    // hourly pattern: 0-23, value = typical crowd 0-4
    vector<int> hourlyPattern;

    StudyArea(int i, const string& n, const string& b, const string& f,
              bool lib, bool outlet, bool wifi, bool quiet, const string& link)
        : id(i), name(n), building(b), floor(f), isLibrary(lib),
          hasOutlet(outlet), hasWifi(wifi), isQuiet(quiet),
          baiduMapLink(link), crowd(CrowdLevel::QUIET),
          lastReportTime("--:--"), reportCount(0),
          hourlyPattern(24, 0) {}
};

// ── Student ──
struct Student {
    string studentId;   // e.g. B23012345
    string name;
    string major;
    int postsCount;
    int reportsCount;
    int helpfulVotes;   // total upvotes received

    Student(const string& id, const string& n, const string& m)
        : studentId(id), name(n), major(m),
          postsCount(0), reportsCount(0), helpfulVotes(0) {}
};

// ── Comment ──
struct Comment {
    int id;
    string authorId;
    string authorName;
    string content;
    string timestamp;
};

// ── Post ──
enum class PostTag {
    QUESTION, STUDY_BUDDY, LEAVING_SOON, RESOURCE, LOST_FOUND, GENERAL
};

string tagLabel(PostTag t) {
    switch (t) {
        case PostTag::QUESTION:    return "[❓ Question  ]";
        case PostTag::STUDY_BUDDY: return "[👥 StudyBuddy]";
        case PostTag::LEAVING_SOON:return "[🚪 LeavingSoon]";
        case PostTag::RESOURCE:    return "[📄 Resource  ]";
        case PostTag::LOST_FOUND:  return "[🔍 Lost&Found]";
        case PostTag::GENERAL:     return "[💬 General   ]";
    }
    return "[        ]";
}

struct Post {
    int id;
    string authorId;
    string authorName;
    PostTag tag;
    string title;
    string content;
    string timestamp;
    int upvotes;
    vector<string> upvotedBy;
    vector<Comment> comments;
    bool expired;   // for leaving_soon posts, auto-expire
    int expiresInMins;

    Post(int i, const string& aId, const string& aName,
         PostTag t, const string& title_, const string& content_,
         int expMins = 0)
        : id(i), authorId(aId), authorName(aName), tag(t),
          title(title_), content(content_), timestamp(timeNow()),
          upvotes(0), expired(false), expiresInMins(expMins) {}
};

// ───────────────────────────────────────────────
// Main System
// ───────────────────────────────────────────────
class NStudyHub {
    vector<StudyArea> areas;
    vector<Student> students;
    vector<Post> posts;
    Student* currentUser = nullptr;
    int nextPostId = 1;
    int nextCommentId = 1;

    // ── seeding realistic NJUPT Xianlin data ──
    void seedAreas() {
        // Library floors
        areas.emplace_back(1, "图书馆 1F — 综合阅览区", "图书馆", "1F",
            true, true, true, false,
            "https://map.baidu.com/search/南京邮电大学仙林校区图书馆");
        areas.emplace_back(2, "图书馆 2F — 文学期刊区", "图书馆", "2F",
            true, true, true, true,
            "https://map.baidu.com/search/南京邮电大学仙林校区图书馆");
        areas.emplace_back(3, "图书馆 3F — 自习静读区", "图书馆", "3F",
            true, true, true, true,
            "https://map.baidu.com/search/南京邮电大学仙林校区图书馆");
        areas.emplace_back(4, "图书馆 4F — 理工科阅览区", "图书馆", "4F",
            true, true, true, true,
            "https://map.baidu.com/search/南京邮电大学仙林校区图书馆");
        areas.emplace_back(5, "图书馆 5F — 研讨室区", "图书馆", "5F",
            true, true, true, false,
            "https://map.baidu.com/search/南京邮电大学仙林校区图书馆");

        // Alternative spots
        areas.emplace_back(6, "教学楼A — 自习教室", "教学楼A", "2F",
            false, true, true, true,
            "https://map.baidu.com/search/南京邮电大学仙林校区教学楼");
        areas.emplace_back(7, "教学楼B — 空教室", "教学楼B", "3F",
            false, true, true, true,
            "https://map.baidu.com/search/南京邮电大学仙林校区教学楼");
        areas.emplace_back(8, "学生活动中心 — 安静角落", "学生活动中心", "1F",
            false, true, true, false,
            "https://map.baidu.com/search/南京邮电大学仙林校区学生活动中心");
        areas.emplace_back(9, "工科楼 — 走廊自习区", "工科楼", "2F",
            false, false, true, false,
            "https://map.baidu.com/search/南京邮电大学仙林校区");
        areas.emplace_back(10, "行政楼 — 大厅休息区", "行政楼", "1F",
            false, false, true, false,
            "https://map.baidu.com/search/南京邮电大学仙林校区");

        // Set realistic hourly patterns
        // Library: quiet mornings, packed afternoons
        int libPattern[] = {0,0,0,0,0,0,1,2,3,4,4,4,3,3,4,4,4,3,3,2,2,1,0,0};
        for (int i = 1; i <= 5; i++)
            for (int h = 0; h < 24; h++)
                areas[i-1].hourlyPattern[h] = libPattern[h];

        // Teaching buildings: busy during class hours, empty otherwise
        int classPattern[] = {0,0,0,0,0,0,0,1,3,3,1,3,3,1,3,3,1,3,3,1,0,0,0,0};
        for (int i = 6; i <= 10; i++)
            for (int h = 0; h < 24; h++)
                areas[i-1].hourlyPattern[h] = classPattern[h];

        // Seed some initial crowd levels based on time
        int hour = hourNow();
        for (auto& a : areas) {
            int pattern = a.hourlyPattern[hour];
            a.crowd = static_cast<CrowdLevel>(pattern);
        }

        // Pre-seed some sample posts for demo
        posts.emplace_back(nextPostId++, "B23001234", "张同学",
            PostTag::LEAVING_SOON, "图书馆3F 快有空位了",
            "靠窗位置，还有2个空位，我再坐20分钟就走了", 20);
        posts.back().upvotes = 4;

        posts.emplace_back(nextPostId++, "B22005678", "李同学",
            PostTag::STUDY_BUDDY, "找人一起备考数据结构",
            "明天下午2点在教学楼A，有要一起刷题的吗？专业计算机科学", 0);
        posts.back().upvotes = 2;

        posts.emplace_back(nextPostId++, "B23009999", "王同学",
            PostTag::QUESTION, "图书馆几点开门？",
            "明天周六图书馆几点开门，有人知道吗", 0);
        posts.back().upvotes = 1;
        posts.back().comments.push_back({nextCommentId++, "B22001111", "陈同学",
            "周六8点开，比平时晚一小时", timeNow()});

        posts.emplace_back(nextPostId++, "B21003344", "刘同学",
            PostTag::RESOURCE, "信号与系统复习资料",
            "期末复习资料整理好了，在这个帖子下面问我要", 0);
        posts.back().upvotes = 8;
    }

    Student* findStudent(const string& id) {
        for (auto& s : students)
            if (s.studentId == id) return &s;
        return nullptr;
    }

    StudyArea* findArea(int id) {
        for (auto& a : areas)
            if (a.id == id) return &a;
        return nullptr;
    }

    Post* findPost(int id) {
        for (auto& p : posts)
            if (p.id == id) return &p;
        return nullptr;
    }

    // ── Auth ──
    void registerStudent() {
        printHeader("CREATE ACCOUNT");
        cout << "\n";
        clearInput();

        string sid = getLine("  Student ID (e.g. B23012345) : ");
        if (findStudent(sid)) {
            cout << "  Account already exists.\n\n";
            return;
        }
        if (sid.size() < 8) {
            cout << "  Invalid student ID format.\n\n";
            return;
        }

        string name  = getLine("  Your name                  : ");
        string major = getLine("  Your major                 : ");

        students.emplace_back(sid, name, major);
        currentUser = &students.back();
        cout << "\n  ✓ Welcome, " << name << "! Account created.\n\n";
    }

    void loginStudent() {
        printHeader("LOGIN");
        cout << "\n";
        clearInput();

        string sid = getLine("  Student ID : ");
        Student* s = findStudent(sid);
        if (!s) {
            cout << "  Account not found. Please register first.\n\n";
            return;
        }
        currentUser = s;
        cout << "\n  ✓ Welcome back, " << s->name << "!\n\n";
    }

    // ── Library Status ──
    void viewLibraryStatus() {
        printHeader("LIBRARY STATUS — NJUPT Xianlin");
        cout << "\n  Last updated: " << timeNow() << "\n\n";

        cout << "  " << left
             << setw(36) << "Floor / Zone"
             << setw(12) << "Status"
             << setw(12) << "Crowd"
             << "Reported\n";
        printLine();

        bool anyFull = true;
        for (auto& a : areas) {
            if (!a.isLibrary) continue;
            if (a.crowd != CrowdLevel::FULL) anyFull = false;

            string features = "";
            if (a.hasOutlet) features += "⚡";
            if (a.isQuiet)   features += "🔇";

            cout << "  " << left
                 << setw(34) << a.name
                 << "  " << setw(10) << crowdLabel(a.crowd)
                 << crowdIcon(a.crowd)
                 << "\n";

            if (a.reportCount > 0)
                cout << "      ↳ reported by " << a.lastReportedBy
                     << " at " << a.lastReportTime << "\n";
        }

        // Check if library is generally full
        int fullCount = 0;
        int libCount = 0;
        for (auto& a : areas) {
            if (!a.isLibrary) continue;
            libCount++;
            if (a.crowd == CrowdLevel::FULL || a.crowd == CrowdLevel::BUSY)
                fullCount++;
        }

        cout << "\n";
        if (fullCount >= libCount - 1) {
            printLine('*');
            cout << "  ⚠  Library is mostly FULL right now!\n";
            cout << "  → Go to option [3] for alternative study spots.\n";
            printLine('*');
        } else {
            cout << "  📚 Some floors still available — check above.\n";
        }
        cout << "\n";
    }

    // ── Report crowd ──
    void reportCrowd() {
        if (!currentUser) { cout << "  Please login first.\n\n"; return; }
        printHeader("REPORT CROWD LEVEL");

        cout << "\n  Which area are you at?\n\n";
        for (auto& a : areas) {
            cout << "  [" << a.id << "] " << a.name << "\n";
        }
        cout << "\n  Area number : ";
        int areaId; cin >> areaId;
        clearInput();

        StudyArea* area = findArea(areaId);
        if (!area) { cout << "  Area not found.\n\n"; return; }

        cout << "\n  How crowded is it right now?\n";
        cout << "  [1] Empty    [2] Quiet    [3] Moderate\n";
        cout << "  [4] Busy     [5] Full\n";
        cout << "\n  Your report : ";
        int level; cin >> level;
        clearInput();

        if (level < 1 || level > 5) {
            cout << "  Invalid choice.\n\n"; return;
        }

        area->crowd = static_cast<CrowdLevel>(level - 1);
        area->lastReportedBy = currentUser->name;
        area->lastReportTime = timeNow();
        area->reportCount++;
        currentUser->reportsCount++;

        // update hourly pattern with this real data
        int h = hourNow();
        area->hourlyPattern[h] = level - 1;

        cout << "\n  ✓ Thanks " << currentUser->name << "! "
             << area->name << " marked as "
             << crowdLabel(area->crowd) << "\n\n";
    }

    // ── Alternative spots ──
    void viewAlternatives() {
        printHeader("ALTERNATIVE STUDY SPOTS");
        cout << "\n  Library full? Try these instead:\n\n";

        // Sort by crowd level (least busy first)
        vector<StudyArea*> alts;
        for (auto& a : areas)
            if (!a.isLibrary) alts.push_back(&a);

        sort(alts.begin(), alts.end(),
             [](StudyArea* a, StudyArea* b){
                 return static_cast<int>(a->crowd) <
                        static_cast<int>(b->crowd);
             });

        cout << "  " << left
             << setw(32) << "Spot"
             << setw(12) << "Crowd"
             << setw(8)  << "Outlet"
             << setw(8)  << "WiFi"
             << "Quiet\n";
        printLine();

        for (auto* a : alts) {
            cout << "  " << left
                 << setw(30) << a->name
                 << "  " << setw(10) << crowdLabel(a->crowd)
                 << setw(8)  << (a->hasOutlet ? "Yes" : "No")
                 << setw(8)  << (a->hasWifi   ? "Yes" : "No")
                 << (a->isQuiet ? "Yes" : "No") << "\n";
        }

        cout << "\n  🗺  Navigation (Baidu Maps links):\n";
        for (auto* a : alts) {
            cout << "  → " << a->building << " : " << a->baiduMapLink << "\n";
        }
        cout << "\n";

        // Show "I'm leaving soon" posts
        bool hasLeaving = false;
        for (auto& p : posts) {
            if (p.tag == PostTag::LEAVING_SOON && !p.expired) {
                if (!hasLeaving) {
                    cout << "  🚪 LEAVING SOON posts (grab their spot!):\n\n";
                    hasLeaving = true;
                }
                cout << "  ● " << p.title << "\n";
                cout << "    " << p.content << "\n";
                cout << "    Posted at " << p.timestamp << " by " << p.authorName << "\n\n";
            }
        }
    }

    // ── Heatmap ──
    void viewHeatmap() {
        printHeader("BEST TIME TO STUDY — Today's Pattern");
        cout << "\n  Based on student reports (library floors avg)\n\n";

        cout << "  Hour  ";
        for (int h = 7; h <= 23; h++)
            cout << setw(3) << h;
        cout << "\n  ";

        // Calculate average crowd per hour across library floors
        for (int h = 7; h <= 23; h++) {
            float avg = 0;
            int count = 0;
            for (auto& a : areas) {
                if (a.isLibrary) {
                    avg += a.hourlyPattern[h];
                    count++;
                }
            }
            avg /= count;
            string bar = (avg < 1) ? "░" :
                         (avg < 2) ? "▒" :
                         (avg < 3) ? "▓" : "█";
            cout << "       ";
            for (int h2 = 7; h2 <= 23; h2++) {
                float a2 = 0; int c2 = 0;
                for (auto& a : areas)
                    if (a.isLibrary) { a2 += a.hourlyPattern[h2]; c2++; }
                a2 /= c2;
                cout << "  " << ((a2 < 1) ? "░" : (a2 < 2) ? "▒" : (a2 < 3) ? "▓" : "█");
            }
            cout << "\n";
            break; // only print once
        }

        cout << "\n  ";
        for (int h = 7; h <= 23; h++)
            cout << setw(3) << h;
        cout << "\n\n";
        cout << "  ░ Empty   ▒ Quiet   ▓ Moderate   █ Busy/Full\n";
        cout << "\n  💡 Best times to study: 7-8am, 8-9pm\n";
        cout << "     Worst: 2-5pm on weekdays\n\n";
    }

    // ── Community Board ──
    void viewBoard() {
        printHeader("NSTUDY COMMUNITY BOARD");
        cout << "\n  " << posts.size() << " posts\n\n";

        cout << "  Filter by tag:\n";
        cout << "  [0] All  [1] Question  [2] StudyBuddy  ";
        cout << "[3] LeavingSoon  [4] Resource  [5] Lost&Found\n\n";
        cout << "  Filter : ";
        int filter; cin >> filter;
        clearInput();

        int shown = 0;
        for (auto& p : posts) {
            if (filter > 0 && static_cast<int>(p.tag) != filter - 1) continue;
            if (p.expired) continue;

            printLine('-');
            cout << "  #" << p.id << "  " << tagLabel(p.tag) << "\n";
            cout << "  " << p.title << "\n";
            cout << "  " << p.content << "\n";
            cout << "  ↑ " << p.upvotes << "   💬 " << p.comments.size()
                 << "   by " << p.authorName << " @ " << p.timestamp;
            if (p.tag == PostTag::LEAVING_SOON)
                cout << "  (expires in ~" << p.expiresInMins << " min)";
            cout << "\n";

            if (!p.comments.empty()) {
                cout << "\n  Comments:\n";
                for (auto& c : p.comments)
                    cout << "    ↳ " << c.authorName << ": " << c.content << "\n";
            }
            shown++;
        }

        if (shown == 0)
            cout << "  No posts found.\n";
        cout << "\n";

        cout << "  [Enter post ID to interact, or 0 to go back] : ";
        int postId; cin >> postId;
        clearInput();

        if (postId > 0) interactPost(postId);
    }

    void interactPost(int postId) {
        Post* post = findPost(postId);
        if (!post) { cout << "  Post not found.\n\n"; return; }
        if (!currentUser) { cout << "  Login to interact.\n\n"; return; }

        cout << "\n  [1] Upvote   [2] Comment   [3] Delete (own posts only)\n";
        cout << "  Action : ";
        int action; cin >> action;
        clearInput();

        if (action == 1) {
            auto it = find(post->upvotedBy.begin(), post->upvotedBy.end(),
                          currentUser->studentId);
            if (it != post->upvotedBy.end()) {
                post->upvotes--;
                post->upvotedBy.erase(it);
                cout << "  ↓ Upvote removed.\n\n";
            } else {
                post->upvotes++;
                post->upvotedBy.push_back(currentUser->studentId);
                // give author reputation
                Student* author = findStudent(post->authorId);
                if (author) author->helpfulVotes++;
                cout << "  ↑ Upvoted!\n\n";
            }
        } else if (action == 2) {
            string content = getLine("  Your comment : ");
            if (content.empty()) return;
            Comment c;
            c.id = nextCommentId++;
            c.authorId = currentUser->studentId;
            c.authorName = currentUser->name;
            c.content = content;
            c.timestamp = timeNow();
            post->comments.push_back(c);
            cout << "  ✓ Comment added!\n\n";
        } else if (action == 3) {
            if (post->authorId != currentUser->studentId) {
                cout << "  Can only delete your own posts.\n\n"; return;
            }
            post->expired = true;
            cout << "  ✓ Post deleted.\n\n";
        }
    }

    void createPost() {
        if (!currentUser) { cout << "  Please login first.\n\n"; return; }
        printHeader("CREATE POST");
        cout << "\n";
        clearInput();

        cout << "  Post type:\n";
        cout << "  [1] Question   [2] Study Buddy  [3] Leaving Soon\n";
        cout << "  [4] Resource   [5] Lost & Found [6] General\n\n";
        cout << "  Type : ";
        int type; cin >> type;
        clearInput();

        if (type < 1 || type > 6) {
            cout << "  Invalid type.\n\n"; return;
        }

        PostTag tag = static_cast<PostTag>(type - 1);
        string title   = getLine("  Title   : ");
        string content = getLine("  Content : ");

        if (title.empty() || content.empty()) {
            cout << "  Title and content cannot be empty.\n\n"; return;
        }

        int expMins = 0;
        if (tag == PostTag::LEAVING_SOON) {
            cout << "  Leaving in how many minutes? : ";
            cin >> expMins;
            clearInput();
        }

        posts.emplace_back(nextPostId++, currentUser->studentId,
                          currentUser->name, tag, title, content, expMins);
        currentUser->postsCount++;

        cout << "\n  ✓ Post created! The community will see it now.\n\n";
    }

    // ── Profile ──
    void viewProfile() {
        if (!currentUser) { cout << "  Please login first.\n\n"; return; }
        printHeader("MY PROFILE");
        cout << "\n";
        cout << "  Name         : " << currentUser->name << "\n";
        cout << "  Student ID   : " << currentUser->studentId << "\n";
        cout << "  Major        : " << currentUser->major << "\n";
        cout << "  Posts made   : " << currentUser->postsCount << "\n";
        cout << "  Reports made : " << currentUser->reportsCount << "\n";
        cout << "  Helpful votes: " << currentUser->helpfulVotes << "\n";
        cout << "\n  The more you report & help, the more others benefit!\n\n";
    }

    // ── Main menu ──
    void printMainMenu() {
        printLine('=');
        cout << "  📚 NStudy Hub — NJUPT Xianlin\n";
        if (currentUser)
            cout << "  Logged in as: " << currentUser->name
                 << " (" << currentUser->studentId << ")\n";
        else
            cout << "  Not logged in\n";
        printLine('=');
        cout << "  [1] Library status        [2] Report crowd level\n";
        cout << "  [3] Alternative spots     [4] Best time heatmap\n";
        cout << "  [5] Community board       [6] Create a post\n";
        cout << "  [7] My profile\n";
        cout << "  [8] Login                 [9] Register\n";
        cout << "  [0] Exit\n";
        printLine();
        cout << "  > ";
    }

public:
    NStudyHub() {
        seedAreas();
    }

    void run() {
        cout << "\n";
        printLine('=');
        cout << "  ███╗   ██╗███████╗████████╗██╗   ██╗██████╗ ██╗   ██╗\n";
        cout << "  ████╗  ██║██╔════╝╚══██╔══╝██║   ██║██╔══██╗╚██╗ ██╔╝\n";
        cout << "  ██╔██╗ ██║███████╗   ██║   ██║   ██║██║  ██║ ╚████╔╝ \n";
        cout << "  ██║╚██╗██║╚════██║   ██║   ██║   ██║██║  ██║  ╚██╔╝  \n";
        cout << "  ██║ ╚████║███████║   ██║   ╚██████╔╝██████╔╝   ██║   \n";
        cout << "  ╚═╝  ╚═══╝╚══════╝   ╚═╝    ╚═════╝ ╚═════╝    ╚═╝   \n";
        printLine('=');
        cout << "  NJUPT Xianlin Campus Study Hub\n";
        cout << "  Library status · Crowd reports · Community Q&A\n";
        printLine('=');
        cout << "\n";

        int choice = -1;
        while (choice != 0) {
            printMainMenu();
            string input;
            cin >> input;
            try { choice = stoi(input); } catch (...) { choice = -1; }
            cout << "\n";

            switch (choice) {
                case 1: viewLibraryStatus(); pause(); break;
                case 2: reportCrowd();       break;
                case 3: viewAlternatives();  pause(); break;
                case 4: viewHeatmap();       pause(); break;
                case 5: viewBoard();         break;
                case 6: createPost();        break;
                case 7: viewProfile();       pause(); break;
                case 8: loginStudent();      break;
                case 9: registerStudent();   break;
                case 0: cout << "  再见! Happy studying.\n\n"; break;
                default: cout << "  Invalid option.\n\n";
            }
        }
    }
};

int main() {
    NStudyHub hub;
    hub.run();
    return 0;
}