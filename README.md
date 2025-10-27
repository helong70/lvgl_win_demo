# LVGL Windows Demo# LVGL Windows Demo



一个在 Windows 平台上运行的 LVGL 图形界面演示项目，使用 OpenGL 硬件加速渲染，具有现代化的无边框窗口设计和完整的输入支持。一个在Windows平台上运行的LVGL图形界面演示项目，使用OpenGL硬件加速渲染，具有现代化的无边框窗口设计。



## ✨ 功能特性## 🚀 功能特性



### 核心功能### 核心功能

- ✅ **OpenGL 硬件加速渲染** - 使用 OpenGL 纹理上传和 GPU 渲染，流畅显示- ✅ **OpenGL硬件加速渲染** - 使用OpenGL纹理上传和GPU渲染

- ✅ **无边框现代化窗口** - 自定义标题栏，圆角窗口设计- ✅ **无边框现代化窗口** - 自定义标题栏，圆角设计

- ✅ **完整的键盘支持** - 键盘输入、快捷键（Ctrl+A、Ctrl+C、Ctrl+V）- ✅ **流畅的窗口拖动** - 使用Windows原生拖动机制

- ✅ **完整的鼠标支持** - 点击、拖动、滑块控制等交互- ✅ **DPI感知缩放** - 自动适应高DPI显示器

- ✅ **窗口无闪烁恢复** - 优化的窗口最小化/恢复机制- ✅ **完整的鼠标输入** - 支持点击、拖动等交互

- ✅ **DPI 感知** - 自动适应高 DPI 显示器

### UI组件

### UI 组件展示- 📱 **自定义标题栏** - 带拖动功能的深色标题栏

- 📱 **自定义标题栏** - 可拖动的深色标题栏，带关闭和最小化按钮- 🔘 **关闭按钮** - 带淡出动画的关闭效果

- 🎛️ **交互式控件卡片**- 🔹 **最小化按钮** - 一键最小化到任务栏

  - 按钮 - 带状态切换和颜色变化- 🎛️ **交互式按钮** - 可点击的按钮组件

  - 滑块 - 实时数值显示和拖动- 🎚️ **滑块控件** - 数值调节滑块

  - 下拉框 - 主题选择菜单- 📝 **下拉选择框** - 多选项下拉菜单

- 📝 **登录表单卡片**

  - 用户名输入框 - 带聚焦状态### 视觉效果

  - 密码输入框 - 密码遮罩模式- 🎨 **圆角设计** - 窗口和UI元素统一圆角风格

  - Tab 键切换焦点- 🌈 **现代化配色** - 深色标题栏配浅色内容区

- 📊 **系统信息面板** - 显示系统状态和功能说明- ✨ **平滑动画** - 按钮点击和窗口操作动画

- ⚙️ **设置面板** - 动画速度、透明度调节- 🖼️ **抗锯齿渲染** - OpenGL多重采样抗锯齿



### 视觉效果## 🛠️ 技术栈

- 🎨 **现代化 UI 设计** - 卡片式布局，统一圆角风格

- 🌈 **精美配色方案** - 淡蓝标题配白色内容区- **UI框架**: LVGL 9.x

- ✨ **阴影效果** - 所有卡片带柔和阴影- **图形API**: OpenGL

- 🖼️ **抗锯齿渲染** - OpenGL 多重采样抗锯齿- **平台**: Windows (Win32 API)

- **编译器**: MinGW-w64 GCC

## 🛠️ 技术栈- **构建系统**: CMake + Ninja

- **绘图**: GDI+ (圆角效果)

- **UI 框架**: LVGL 9.x

- **图形 API**: OpenGL 2.0+## 📋 系统要求

- **平台**: Windows (Win32 API)

- **编译器**: MinGW-w64 GCC / MSVC- Windows 10 或更高版本

- **构建系统**: CMake + Ninja- 支持OpenGL 2.0+ 的显卡

- **图形库**: GDI+ (圆角窗口)- MinGW-w64 工具链

- CMake 3.16+

## 📁 项目结构- Ninja 构建工具



```## 🔧 编译说明

LVGL_WIN/

├── main.c                  # 主程序入口### 1. 克隆项目

├── CMakeLists.txt          # CMake 构建配置```bash

├── build_ninja.bat         # 快速构建脚本git clone https://github.com/helong70/lvgl_win_demo.git

├── run.bat                 # 运行脚本cd lvgl_win_demo

├── hal/                    # 硬件抽象层```

│   ├── lv_hal.c/h         # HAL 总接口

│   ├── lv_hal_disp.c/h    # 显示驱动### 2. 构建项目

│   └── lv_hal_indev.c/h   # 输入设备驱动```bash

├── platform/               # 平台相关代码# 使用构建脚本

│   └── windows/.\build.bat

│       ├── win32_platform.c/h  # Windows 平台实现

│       └── ...# 或手动构建

├── ui/                     # UI 组件mkdir build

│   ├── maincontainer.c/h  # 主容器cd build

│   ├── titlebar.c/h       # 标题栏cmake .. -G Ninja

│   ├── setting.c/h        # 设置面板ninja

│   └── custom_keys.h      # 自定义按键```

├── lvgl/                   # LVGL 源码

└── build/                  # 构建输出目录### 3. 运行演示

``````bash

.\build\bin\LVGL_Windows_Demo.exe

## 📋 系统要求```



- **操作系统**: Windows 10 或更高版本## 🎮 使用说明

- **显卡**: 支持 OpenGL 2.0+ 的显卡

- **开发工具**:### 窗口操作

  - MinGW-w64 工具链 或 MSVC- **拖动窗口**: 点击标题栏任意位置拖动

  - CMake 3.16+- **关闭窗口**: 点击右上角 ❌ 按钮

  - Ninja 构建工具- **最小化**: 点击右上角 ➖ 按钮



## 🔧 编译说明### UI交互

- **主按钮**: 点击切换文本和颜色

### 方法一：使用构建脚本（推荐）- **滑块**: 拖动调节数值，控制台显示当前值

- **下拉框**: 点击展开选择选项

```batch

# 构建项目### 键盘快捷键

build_ninja.bat- `1` - 模拟点击主按钮

- `2` - 模拟点击特定区域

# 运行程序- `q` - 退出程序

run.bat

```## 📁 项目结构



### 方法二：手动构建```

LVGL_WIN/

```bash├── main.c              # 主程序源码

# 1. 创建构建目录├── CMakeLists.txt      # CMake配置

mkdir build├── build.bat          # 构建脚本

cd build├── run.bat            # 运行脚本

├── lvgl/              # LVGL源码子模块

# 2. 配置 CMake（MinGW）├── build/             # 构建输出目录

cmake -G "Ninja" -DCMAKE_BUILD_TYPE=Release ..│   └── bin/           # 可执行文件

└── README.md          # 项目说明

# 3. 编译```

ninja

## 🔧 开发亮点

# 4. 运行

cd ..### 1. OpenGL集成

.\build\bin\LVGL_Windows_Demo.exe```c

```// OpenGL纹理上传和渲染

static void display_flush_cb(lv_display_t * disp, const lv_area_t * area, uint8_t * px_map)

### 方法三：使用 Visual Studio{

    // RGB565 -> RGBA8888 转换

```bash    // OpenGL纹理子图像更新

# 1. 生成 Visual Studio 解决方案    // 硬件加速渲染

cmake -G "Visual Studio 17 2022" -A x64 -S . -B build}

```

# 2. 打开生成的解决方案

start build\LVGL_Windows_Demo.sln### 2. 原生窗口拖动

``````c

case WM_NCHITTEST: {

## 🎮 使用说明    // 检测鼠标位置

    // 标题栏区域返回HTCAPTION

### 键盘操作    // 启用Windows原生拖动

- **Tab** - 在输入框之间切换焦点}

- **Ctrl+A** - 全选文本```

- **Ctrl+C** - 复制选中文本

- **Ctrl+V** - 粘贴文本### 3. GDI+圆角效果

- **Enter** - 确认输入```c

- **Backspace/Delete** - 删除字符// 使用GDI+绘制圆角窗口

- **方向键** - 移动光标Graphics graphics(hdc);

GraphicsPath path;

### 鼠标操作// 添加圆角路径

- **左键点击** - 选择控件、点击按钮graphics.FillPath(&brush, &path);

- **拖动标题栏** - 移动窗口```

- **拖动滑块** - 调节数值

- **点击下拉框** - 展开选项## 🐛 已知问题



### 窗口操作- ⚠️ 最小化后可能需要点击任务栏图标恢复

- **最小化按钮** - 最小化到任务栏- ⚠️ 高DPI下某些UI元素可能需要微调

- **关闭按钮** - 退出程序- ⚠️ GDI+圆角在某些显卡上可能有兼容性问题

- **拖动标题栏** - 移动窗口位置

## 🚧 开发计划

## 🏗️ 架构设计

- [ ] 添加键盘输入支持

### 模块分层- [ ] 实现更多LVGL控件示例

- [ ] 优化渲染性能

```- [ ] 添加主题切换功能

┌─────────────────────────────────────┐- [ ] 支持窗口大小调整

│          Application Layer          │

│  (main.c, UI components)           │## 📝 更新日志

├─────────────────────────────────────┤

│         LVGL Framework              │### v1.0.0 (2025-10-24)

│  (Widget rendering, event handling) │- ✅ 初始版本发布

├─────────────────────────────────────┤- ✅ OpenGL硬件加速渲染

│      Hardware Abstraction Layer     │- ✅ 无边框窗口设计

│  (HAL: Display + Input devices)     │- ✅ 标题栏拖动功能

├─────────────────────────────────────┤- ✅ 关闭和最小化按钮

│         Platform Layer              │- ✅ 基础UI控件演示

│  (Windows API, OpenGL)              │

└─────────────────────────────────────┘## 🤝 贡献指南

```

欢迎提交Issue和Pull Request！

### 关键特性

1. Fork 项目

#### 1. HAL 层设计2. 创建功能分支

- **显示 HAL** (`lv_hal_disp.c/h`) - 管理 OpenGL 纹理和渲染3. 提交更改

- **输入 HAL** (`lv_hal_indev.c/h`) - 管理鼠标和键盘输入队列4. 推送到分支

- **统一接口** (`lv_hal.c/h`) - 提供初始化和访问接口5. 创建Pull Request



#### 2. 平台抽象## 📄 许可证

- **Windows 平台** (`win32_platform.c/h`) - 封装 Win32 API

- **消息处理** - 统一的 Windows 消息循环本项目采用 MIT 许可证。详情请见 [LICENSE](LICENSE) 文件。

- **回调机制** - 将平台事件转换为 LVGL 事件

## 🙏 致谢

#### 3. UI 组件化

- **主容器** - 卡片式布局设计- [LVGL](https://lvgl.io/) - 优秀的嵌入式图形库

- **标题栏** - 独立的标题栏组件- [OpenGL](https://www.opengl.org/) - 跨平台图形API

- **设置面板** - 可扩展的设置界面- [MinGW-w64](https://www.mingw-w64.org/) - Windows下的GCC工具链

## 🐛 已知问题修复

### 已修复的问题
- ✅ **键盘输入无效** - 添加了键盘事件回调注册
- ✅ **窗口恢复闪烁** - 禁用 WM_PAINT 背景绘制，添加 WM_ERASEBKGND 处理
- ✅ **滑块无法拖动** - 修复 WM_MOUSEMOVE 消息中的按钮状态检测
- ✅ **卡片内容滚动** - 禁用卡片的 SCROLLABLE 标志

## 📝 开发笔记

### 性能优化
- 使用部分渲染模式减少 CPU 负载
- OpenGL 纹理更新采用分块上传
- 键盘输入采用队列机制，避免事件丢失

### 兼容性
- 支持 Windows 10/11
- 兼容高 DPI 显示器
- 适配不同 OpenGL 驱动实现

## 📄 许可证

本项目采用 **GPL-3.0**（GNU General Public License v3.0）许可证。

这意味着：
- ✅ 你可以自由使用、修改和分发本软件
- ✅ 你可以将本软件用于商业用途
- ⚠️ 任何基于本软件的衍生作品必须同样采用 GPL-3.0 许可证开源
- ⚠️ 必须保留原作者的版权声明
- ⚠️ 修改后的代码必须明确标注修改内容

详情请见 [LICENSE](LICENSE) 文件或访问 [GNU GPL v3.0](https://www.gnu.org/licenses/gpl-3.0.html)。

## 🙏 致谢

- [LVGL](https://lvgl.io/) - 轻量级图形库
- [OpenGL](https://www.opengl.org/) - 跨平台图形 API
- [MinGW-w64](https://www.mingw-w64.org/) - Windows GCC 工具链

## 📞 联系方式

- GitHub: [helong70/lvgl_win_demo](https://github.com/helong70/lvgl_win_demo)
- Issues: [提交问题](https://github.com/helong70/lvgl_win_demo/issues)

## 🔄 更新日志

### v1.0.0 (2025-10-27)
- ✨ 初始版本发布
- ✅ 实现 OpenGL 渲染
- ✅ 完整的键盘和鼠标支持
- ✅ 现代化 UI 设计
- ✅ HAL 层架构
- ✅ 平台抽象层
- ✅ 卡片式布局

---

**⭐ 如果这个项目对你有帮助，请给它一个 Star！**
