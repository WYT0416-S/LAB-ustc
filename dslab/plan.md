我在 Plan 模式下没有直接写入工作区文件的权限。不过我可以把完整的计划内容以 Markdown 代码块的形式提供给你，你可以在 VS Code 中新建文件并粘贴保存。

以下是从 `/memories/session/plan.md` 中读取的完整计划内容：

Read memory [](file:///Users/h/Library/Application%20Support/Code/User/workspaceStorage/81bd09b0234271ed96480aa80390fa31/GitHub.copilot-chat/memory-tool/memories/NTUwNmM0OTItYWYzYy00NmY2LTg0OTYtMzcxZDAxMzQxOWFj/plan.md)

我在 Plan 模式下没有直接写入工作区文件的权限。不过你可以通过以下两种方式获取 `.md` 文件：

**方式一**：复制下方的完整 Markdown（已包含全部计划内容），在 VS Code 中新建文件粘贴保存即可。

**方式二**：退出 Plan 模式进入实现模式，我可以直接帮你把文件写入 `/Users/h/project/魔术社团管理系统-详细计划.md`。

以下是完整文档内容，你可以直接全选复制保存：

```markdown
# 魔术社团管理系统 — 详细实施计划

## 技术栈

| 层 | 技术 | 版本 |
|---|------|------|
| 🔙 后端 | Django + Django REST Framework | Django 4.2 LTS, DRF 3.14 |
| 🔐 认证 | djangorestframework-simplejwt | JWT (access 30min / refresh 7d) |
| 🖥️ 前端 | Vue 3 + Vite | Vue 3.4+, Vite 5 |
| 🧩 UI | Element Plus | 2.x |
| 📊 图表 | ECharts (vue-echarts) | 5.x |
| 📦 状态管理 | Pinia | 2.x |
| 🌐 HTTP | Axios | 1.x |
| 🗄️ 数据库 | MySQL | 8.0, utf8mb4 |

---

## 一、项目结构初始化

### Step 1 — 创建后端 Django 项目（与 Step 2 可并行）

```bash
mkdir magic-club-system && cd magic-club-system
mkdir backend && cd backend
python -m venv venv && source venv/bin/activate
pip install django djangorestframework djangorestframework-simplejwt django-cors-headers django-filter mysqlclient
django-admin startproject magic_club .
python manage.py startapp club
```

**`backend/requirements.txt`**：
```
Django>=4.2,<5.0
djangorestframework>=3.14
djangorestframework-simplejwt>=5.3
django-cors-headers>=4.3
django-filter>=23.5
mysqlclient>=2.2
```

**`backend/init.sql`**：
```sql
CREATE DATABASE IF NOT EXISTS magic_club CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;
```

**`backend/magic_club/settings.py` 关键配置**：
- `DATABASES` → MySQL，数据库名 `magic_club`
- `INSTALLED_APPS` 追加：`rest_framework`, `corsheaders`, `django_filters`, `club`
- `MIDDLEWARE` 追加：`corsheaders.middleware.CorsMiddleware`（置于 CommonMiddleware 之前）
- `CORS_ALLOW_ALL_ORIGINS = True`（开发环境）
- `REST_FRAMEWORK`：JWT 认证、分页（PAGE_SIZE=20）、过滤后端
- `SIMPLE_JWT`：ACCESS_TOKEN_LIFETIME=30min, REFRESH_TOKEN_LIFETIME=7d
- 语言/时区：`LANGUAGE_CODE='zh-hans'`, `TIME_ZONE='Asia/Shanghai'`

### Step 2 — 创建前端 Vue 项目（与 Step 1 可并行）

```bash
cd magic-club-system
npm create vite@latest frontend -- --template vue
cd frontend
npm install
npm install vue-router@4 pinia axios element-plus @element-plus/icons-vue vue-echarts echarts
```

**`frontend/vite.config.js`** — 配置开发代理：
```js
server: {
  port: 3000,
  proxy: {
    '/api': { target: 'http://127.0.0.1:8000', changeOrigin: true }
  }
}
```

**`frontend/src/main.js`** — 注册 Element Plus（中文 locale）、Vue Router、Pinia。

创建目录：`views/`, `views/members/`, `views/props/`, `views/activities/`, `views/borrows/`, `components/`, `stores/`, `router/`, `api/`, `utils/`

---

## 二、后端数据模型设计

### Step 3 — 定义 Django Models（`backend/club/models.py`）

#### 3.1 Member（社团成员）

| 字段 | 类型 | 约束 | 说明 |
|------|------|------|------|
| `member_id` | CharField(10) | PK | 格式 M001，save() 自动生成 |
| `name` | CharField(50) | 非空 | 姓名 |
| `gender` | CharField(4) | choices=['男','女'] | 性别 |
| `grade` | CharField(20) | 可空 | 年级 |
| `major` | CharField(50) | 可空 | 专业 |
| `phone` | CharField(20) | 可空 | 联系方式 |
| `join_date` | DateField | default=date.today | 入社时间 |
| `position` | CharField(20) | choices=['社长','副社长','干事','普通社员'] | 职位 |
| `user` | OneToOneField(User) | null=True, on_delete=SET_NULL | 关联登录账号 |
| `created_at` | DateTimeField | auto_now_add=True | 创建时间 |

#### 3.2 Prop（魔术道具）

| 字段 | 类型 | 约束 | 说明 |
|------|------|------|------|
| `prop_id` | CharField(10) | PK | 格式 P001 |
| `name` | CharField(100) | 非空 | 道具名称 |
| `prop_type` | CharField(30) | choices=['扑克','硬币','绳子','丝巾','近景道具','舞台道具','其他'] | 道具类型 |
| `quantity` | IntegerField | default=1, MinValueValidator(0) | 库存数量 |
| `status` | CharField(10) | choices=['可用','已借出','维修中','报废'] | 状态 |
| `location` | CharField(100) | 可空 | 存放位置 |
| `tutorial` | TextField | blank=True | 道具教学说明 |
| `created_at` | DateTimeField | auto_now_add=True | 入库时间 |

#### 3.3 Activity（社团活动）

| 字段 | 类型 | 约束 | 说明 |
|------|------|------|------|
| `activity_id` | CharField(14) | PK | 格式 A20240601001 |
| `name` | CharField(200) | 非空 | 活动名称 |
| `date` | DateTimeField | 非空 | 活动时间 |
| `location` | CharField(200) | 非空 | 活动地点 |
| `organizer` | ForeignKey(User) | on_delete=SET_NULL | 负责人 |
| `description` | TextField | blank=True | 活动内容 |
| `created_at` | DateTimeField | auto_now_add=True | 发布时间 |

#### 3.4 BorrowRecord（借用记录）

| 字段 | 类型 | 约束 | 说明 |
|------|------|------|------|
| `borrow_id` | CharField(10) | PK | 格式 B001 |
| `member` | ForeignKey(Member) | on_delete=CASCADE | 借用人 |
| `prop` | ForeignKey(Prop) | on_delete=CASCADE | 借用的道具 |
| `borrow_date` | DateTimeField | auto_now_add=True | 申请时间 |
| `expected_return_date` | DateField | 可空 | 预计归还日期 |
| `return_date` | DateTimeField | null=True | 实际归还时间 |
| `status` | CharField(10) | choices=['pending','approved','rejected','returned'] | 审核状态 |
| `review_comment` | TextField | blank=True | 审核备注 |
| `damage_note` | TextField | blank=True | 损坏说明 |

#### 3.5 实体关系

```
User ──1:N──> Activity (organizer)
Member ──1:N──> BorrowRecord (member)
Prop ──1:N──> BorrowRecord (prop)
User ──1:1──> Member (user, optional)
```

#### 3.6 Model 信号逻辑

- `BorrowRecord.save()`：状态变更为 approved → Prop.quantity-=1，若归零→Prop.status='已借出'
- 归还时（return_date 被设置）→ Prop.quantity+=1, Prop.status→'可用'
- 损坏登记 → Prop.status→'维修中'

### Step 4 — 数据库迁移

```bash
cd backend
python manage.py makemigrations club
python manage.py migrate
```

---

## 三、后端 API 层

### Step 5 — 自定义权限类（`backend/club/permissions.py`）

| 权限类 | 逻辑 |
|------|------|
| `IsAdminUser` | user.groups 含 'admin' |
| `IsPropAdminUser` | user.groups 含 'prop_admin' |
| `IsMemberUser` | user.groups 含 'member' |
| `IsAdminOrPropAdmin` | admin 或 prop_admin |
| `IsOwnerOrAdmin` | 资源所有者或 admin |

**角色权限矩阵**：

| 操作 | admin | prop_admin | member |
|------|:---:|:---:|:---:|
| 成员管理 CRUD | ✅ | ❌ | 仅自己 |
| 道具查看 | ✅ | ✅ | ✅ |
| 道具增删改 | ✅ | ✅(增/改) | ❌ |
| 活动 CRUD | ✅ | ❌ | ❌ |
| 活动查看 | ✅ | ✅ | ✅ |
| 借用申请 | ❌ | ❌ | ✅ |
| 借用审核 | ✅ | ✅ | ❌ |
| 归还登记 | ✅ | ✅ | ❌ |
| 损坏登记 | ✅ | ✅ | ❌ |
| 查看统计 | ✅ | ❌ | ❌ |

### Step 6 — 认证 API

| 方法 | 路径 | 权限 | 说明 |
|------|------|------|------|
| POST | `/api/auth/register/` | AllowAny | 注册（自动加入 member 组+创建 Member 记录） |
| POST | `/api/auth/login/` | AllowAny | JWT 登录，返回 `{access, refresh, user: {id, username, groups, member_id, name}}` |
| POST | `/api/auth/refresh/` | AllowAny | 刷新 token |
| GET | `/api/auth/profile/` | IsAuthenticated | 获取当前用户信息 |
| PUT/PATCH | `/api/auth/profile/` | IsAuthenticated | 更新个人信息 |

### Step 7 — 成员管理 API

| 方法 | 路径 | 权限 | 说明 |
|------|------|------|------|
| GET | `/api/members/` | IsAdminUser | 列表，支持 `?search=&position=&ordering=` |
| POST | `/api/members/` | IsAdminUser | 添加成员 |
| GET | `/api/members/{id}/` | IsAdminUser/本人 | 详情（含借用历史+借用次数） |
| PUT | `/api/members/{id}/` | IsAdminUser | 全量更新 |
| PATCH | `/api/members/{id}/` | IsAdminUser/本人 | 部分更新 |
| DELETE | `/api/members/{id}/` | IsAdminUser | 删除 |

### Step 8 — 道具管理 API

| 方法 | 路径 | 权限 | 说明 |
|------|------|------|------|
| GET | `/api/props/` | IsAuthenticated | 列表，支持 `?search=&prop_type=&status=` |
| POST | `/api/props/` | IsAdminOrPropAdmin | 添加道具 |
| GET | `/api/props/{id}/` | IsAuthenticated | 详情（含教学+最近10条借用记录） |
| PUT | `/api/props/{id}/` | IsAdminOrPropAdmin | 更新 |
| PATCH | `/api/props/{id}/` | IsAdminOrPropAdmin | 部分更新 |
| DELETE | `/api/props/{id}/` | IsAdminUser | 删除 |
| PUT | `/api/props/{id}/update_tutorial/` | IsAdminOrPropAdmin | 单独更新教学说明 |

### Step 9 — 活动管理 API

| 方法 | 路径 | 权限 | 说明 |
|------|------|------|------|
| GET | `/api/activities/` | IsAuthenticated | 列表，支持 `?search=&ordering=-date` |
| POST | `/api/activities/` | IsAdminUser | 创建（organizer=当前用户） |
| GET | `/api/activities/{id}/` | IsAuthenticated | 详情 |
| PUT/PATCH | `/api/activities/{id}/` | IsAdminUser | 更新 |
| DELETE | `/api/activities/{id}/` | IsAdminUser | 删除 |

### Step 10 — 借用管理 API

| 方法 | 路径 | 权限 | 说明 |
|------|------|------|------|
| GET | `/api/borrows/` | IsAuthenticated | 列表（admin/prop_admin看全部，member看自己） |
| POST | `/api/borrows/` | IsMemberUser | 提交借用申请 |
| GET | `/api/borrows/{id}/` | IsAuthenticated | 详情 |
| PATCH | `/api/borrows/{id}/review/` | IsAdminOrPropAdmin | 审核（approved/rejected） |
| PATCH | `/api/borrows/{id}/return/` | IsAdminOrPropAdmin | 登记归还 |
| PATCH | `/api/borrows/{id}/damage/` | IsAdminOrPropAdmin | 登记损坏 |

**借用流程**：申请(pending) → 审核(approved)【扣库存】 → 归还(returned)【恢复库存】 / 损坏【标记维修中】

### Step 11 — 数据统计 API

`GET /api/stats/dashboard/`（权限：IsAdminUser），返回：

```json
{
  "member_stats": { "total": 45, "by_position": [...] },
  "prop_stats": { "total": 120, "by_status": [...], "by_type": [...] },
  "borrow_stats": { "total_borrows": 200, "active_borrows": 30, "top_props": [...] },
  "activity_stats": { "total": 15, "this_month": 2 }
}
```

### Step 12 — 全局搜索 API

`GET /api/search/?q=关键词`，返回分类结果（members/props/activities 各取 Top 5）。

### Step 13 — URL 路由汇总

```python
# magic_club/urls.py
urlpatterns = [
    path('admin/', admin.site.urls),
    path('api/', include('club.urls')),
]

# club/urls.py
router = DefaultRouter()
router.register(r'members', MemberViewSet)
router.register(r'props', PropViewSet)
router.register(r'activities', ActivityViewSet)
router.register(r'borrows', BorrowRecordViewSet)

urlpatterns = [
    path('auth/register/', RegisterView.as_view()),
    path('auth/login/', CustomTokenObtainPairView.as_view()),
    path('auth/refresh/', TokenRefreshView.as_view()),
    path('auth/profile/', ProfileView.as_view()),
    path('stats/dashboard/', dashboard_stats),
    path('search/', global_search),
    path('', include(router.urls)),
]
```

---

## 四、前端实现

### Step 14 — 核心配置

**Axios 封装**（`utils/request.js`）：
- 请求拦截器：自动附加 JWT token
- 响应拦截器：401 自动刷新 token，失败则清除登录态；统一 `ElMessage.error()` 错误提示

**Pinia Auth Store**（`stores/auth.js`）：
- State: `user`, `token`, `refreshToken`, `isLoggedIn`
- Actions: `login()`, `register()`, `logout()`, `fetchProfile()`, `refreshAccessToken()`
- Getters: `isAdmin`, `isPropAdmin`, `isMember`, `memberId`

**Vue Router**（`router/index.js`）：

| 路径 | 组件 | 角色 |
|------|------|------|
| `/login` | Login | 无 |
| `/register` | Register | 无 |
| `/` `/dashboard` | Dashboard | admin |
| `/members` | MemberList | admin |
| `/members/:id` | MemberDetail | admin/本人 |
| `/props` | PropList | 登录用户 |
| `/props/:id` | PropDetail | 登录用户 |
| `/activities` | ActivityList | 登录用户 |
| `/activities/:id` | ActivityDetail | 登录用户 |
| `/borrows` | BorrowList | 登录用户 |
| `/borrows/apply` | BorrowApply | member |
| `/borrows/:id/review` | BorrowReview | admin/prop_admin |
| `/profile` | Profile | 登录用户 |
| `/search` | SearchResult | 登录用户 |

路由守卫 `beforeEach`：未登录→限访登录/注册页；角色不足→重定向+提示。

**API 函数**（`api/` 目录）：auth.js, members.js, props.js, activities.js, borrows.js, stats.js, search.js

### Step 15 — 布局组件

**`AppLayout.vue`**：经典后台布局（`el-container` → header + aside + main）
- Header：Logo + SearchBar + 用户下拉菜单（个人信息/退出）
- Sidebar：`el-menu` 根据角色动态渲染（admin全菜单 / prop_admin道具+借用 / member道具+活动+借用）
- Main：`<router-view>` + 过渡动画

### Step 16-22 — 页面组件

#### 登录/注册/个人中心
- **Login.vue**：居中卡片，username+password，调用 authStore.login()
- **Register.vue**：username, password, password2, name, phone
- **Profile.vue**：信息卡片 + 编辑表单（phone, email）

#### 成员管理（admin）
- **MemberList.vue**：搜索+职位筛选+表格分页，对话框添加/编辑，popconfirm 删除
- **MemberDetail.vue**：descriptions + 借用历史表格
- **MemberForm.vue**：复用对话框（create/edit），name/gender/grade/major/phone/position/join_date

#### 道具管理
- **PropList.vue**：搜索+类型/状态筛选，表格/卡片切换视图，角色区分操作按钮
- **PropDetail.vue**：基本信息+教学说明卡片+借用记录表格，「申请借用」按钮
- **PropForm.vue**：name/prop_type/quantity/status/location/tutorial

#### 活动管理
- **ActivityList.vue**：搜索+创建按钮，时间线/表格切换视图
- **ActivityDetail.vue**：descriptions
- **ActivityForm.vue**：name/date/location/description

#### 借用管理
- **BorrowList.vue**：状态 radio 筛选，角色区分数据范围，审核/归还/损坏操作
- **BorrowApply.vue**：道具搜索选择+预计归还日期，校验可用性
- **BorrowReview.vue**：通过/拒绝 radio + 备注

#### 仪表盘（admin）
- **Dashboard.vue**：4 个 StatCard + 3 个 ECharts 图表
  - 职位分布饼图
  - 道具状态柱状图
  - 借用次数排行横向柱状图

#### 搜索结果
- **SearchResult.vue**：三个 el-card 分区（成员/道具/活动），点击跳转详情

---

## 五、集成与收尾

### Step 23 — 全局搜索组件（SearchBar.vue）
- 嵌入 Header，`el-autocomplete` 300ms 去抖，分组下拉，点击跳转

### Step 24 — 种子数据（init_data 管理命令）
- 3 个测试用户：admin/admin123, prop_admin/prop123, member/member123
- 10 个道具 + 5 个活动 + 5 条借用记录

### Step 25 — README.md
- 技术栈、环境要求、快速开始步骤、测试账号、项目结构说明

---

## 六、项目文件结构

```
magic-club-system/
├── README.md
├── backend/
│   ├── manage.py
│   ├── requirements.txt
│   ├── init.sql
│   ├── magic_club/
│   │   ├── settings.py
│   │   ├── urls.py
│   │   └── wsgi.py
│   └── club/
│       ├── models.py
│       ├── serializers.py
│       ├── views.py
│       ├── urls.py
│       ├── permissions.py
│       ├── filters.py
│       ├── admin.py
│       └── management/commands/init_data.py
└── frontend/
    ├── package.json
    ├── vite.config.js
    └── src/
        ├── main.js
        ├── App.vue
        ├── router/index.js
        ├── stores/auth.js
        ├── utils/request.js
        ├── api/{auth,members,props,activities,borrows,stats,search}.js
        ├── components/{AppLayout,SearchBar,StatCard}.vue
        └── views/
            ├── Login.vue, Register.vue, Dashboard.vue, Profile.vue, SearchResult.vue
            ├── members/{MemberList,MemberDetail,MemberForm}.vue
            ├── props/{PropList,PropDetail,PropForm}.vue
            ├── activities/{ActivityList,ActivityDetail,ActivityForm}.vue
            └── borrows/{BorrowList,BorrowApply,BorrowReview}.vue
```

---

## 七、实施顺序

```
Phase 1: Step 1+2 并行（双项目初始化）
   ↓
Phase 2: Step 3→4（模型+迁移）→ Step 5-13（全部后端 API）
   ↓
Phase 3: Step 14（前端 Router+Store+Axios+API 封装）
   ↓
Phase 4: Step 15（布局）→ Step 16-22 并行（所有页面）
   ↓
Phase 5: Step 23→24→25（搜索集成+种子数据+README）
```

---

## 八、验证方案

### 后端
1. `python manage.py check` + `migrate` 无错误
2. `python manage.py init_data` 种子数据成功
3. curl/Postman 逐 API 测试 + 完整借用流程 + 库存联动验证

### 前端
1. `npm run dev` 无编译错误
2. 三角色登录 → 权限隔离 + CRUD + 路由守卫验证
3. 借用全流程：申请→审核→归还 → 库存联动正确
4. 仪表盘图表 vs 实际数据对照
5. 搜索跳转 + 响应式布局

### 集成
1. `npm run build` 生产构建成功
2. 前后端联调

---

## 九、决策记录

| 决策 | 说明 |
|------|------|
| 前后端分离 | Django 纯 API + Vue 3 SPA，JWT 认证 |
| Element Plus | Vue 3 最成熟的组件库 |
| ECharts | vue-echarts 集成成熟，图表丰富 |
| User + Group 角色 | 利用 Django 内置系统 |
| 自定义 ID (M001格式) | CharField PK + save() 自动生成 |
| 库存联动 | Model save() 重写 + 信号 |
| Pinia | Vue 3 官方推荐状态管理 |
| 排除范围 | 密码重置、图片上传、日志审计、Docker、SSR、移动端、i18n、单元测试 |
```