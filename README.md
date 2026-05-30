# NJUPT Library Hub — Backend
## 南邮图书馆智慧自习系统

A C++ (Drogon) REST API backend for the NJUPT library availability
and student community platform.

---

## Setup

### Prerequisites
```bash
# Drogon framework
sudo apt install libdrogon-dev   # or build from source
# SQLite3
sudo apt install libsqlite3-dev
# OpenSSL (for password hashing)
sudo apt install libssl-dev
```

### Build
```bash
mkdir build && cd build
cmake ..
cmake --build . -j4
```

### Run
```bash
./build/NJUPTLibraryHub
# Server starts on http://localhost:8080
```

---

## API Reference

### Auth
| Method | Endpoint | Description |
|--------|----------|-------------|
| POST | `/api/auth/register` | New student account |
| POST | `/api/auth/login` | Get token |
| POST | `/api/auth/logout` | Invalidate token |

**Register body:**
```json
{ "username": "张三", "student_id": "B23012345", "password": "mypass" }
```
**Login body:**
```json
{ "student_id": "B23012345", "password": "mypass" }
```
**Login response:**
```json
{ "success": true, "token": "abc123...", "username": "张三" }
```

---

### Study Areas
| Method | Endpoint | Description |
|--------|----------|-------------|
| GET | `/api/areas` | All areas with live status |
| GET | `/api/areas/:id` | Single area |
| POST | `/api/areas/:id/report` | Report crowd level (auth) |
| GET | `/api/alternatives` | Suggest when library is full |
| GET | `/api/areas/:id/history` | Crowd pattern by hour |
| POST | `/api/notify` | Notify me when space opens (auth) |

**Status values:** `empty` → `available` → `busy` → `full`

**Report body:**
```json
{ "status": "full" }
```

**Area response example:**
```json
{
  "id": 3,
  "name": "图书馆 3F — 安静阅读区",
  "building": "图书馆",
  "floor": 3,
  "capacity": 200,
  "type": "library",
  "status": "full",
  "recent_reports": 5,
  "map_url": "bdmap://...",
  "lat": 32.1097,
  "lng": 118.9095
}
```

---

### Community Board
| Method | Endpoint | Description |
|--------|----------|-------------|
| GET | `/api/posts?tag=question&page=1` | Post feed |
| POST | `/api/posts` | Create post (auth) |
| GET | `/api/posts/:id` | Post + comments |
| POST | `/api/posts/:id/comments` | Reply (auth) |
| POST | `/api/posts/:id/upvote` | Toggle upvote (auth) |
| DELETE | `/api/posts/:id` | Delete own post (auth) |

**Post tags:**
- `question` — academic help requests
- `resources` — share notes/past papers
- `study_buddy` — find study partners
- `leaving_soon` — "leaving in 20 min, 3F window seat"
- `lost_found` — campus lost & found
- `general` — anything else

**Create post body:**
```json
{
  "title": "有没有数据结构历年卷？",
  "content": "期末快到了，急需...",
  "tag": "resources"
}
```

---

## Project Structure
```
njupt_library_hub/
├── CMakeLists.txt
├── config.json
├── main.cpp
└── src/
    ├── database/
    │   ├── Database.h          # SQLite wrapper
    │   └── Database.cpp        # Schema + NJUPT seed data
    ├── controllers/
    │   ├── AuthController      # Register / Login
    │   ├── RoomController      # Library status + alternatives
    │   └── PostController      # Community board
    └── utils/
        └── TokenUtils          # Token generation + auth
```
