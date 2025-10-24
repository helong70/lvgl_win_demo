# LVGL Windows Demo

一个在Windows平台上运行的LVGL图形界面演示项目，使用OpenGL硬件加速渲染，具有现代化的无边框窗口设计。

## 🚀 功能特性

### 核心功能
- ✅ **OpenGL硬件加速渲染** - 使用OpenGL纹理上传和GPU渲染
- ✅ **无边框现代化窗口** - 自定义标题栏，圆角设计
- ✅ **流畅的窗口拖动** - 使用Windows原生拖动机制
- ✅ **DPI感知缩放** - 自动适应高DPI显示器
- ✅ **完整的鼠标输入** - 支持点击、拖动等交互

### UI组件
- 📱 **自定义标题栏** - 带拖动功能的深色标题栏
- 🔘 **关闭按钮** - 带淡出动画的关闭效果
- 🔹 **最小化按钮** - 一键最小化到任务栏
- 🎛️ **交互式按钮** - 可点击的按钮组件
- 🎚️ **滑块控件** - 数值调节滑块
- 📝 **下拉选择框** - 多选项下拉菜单

### 视觉效果
- 🎨 **圆角设计** - 窗口和UI元素统一圆角风格
- 🌈 **现代化配色** - 深色标题栏配浅色内容区
- ✨ **平滑动画** - 按钮点击和窗口操作动画
- 🖼️ **抗锯齿渲染** - OpenGL多重采样抗锯齿

## 🛠️ 技术栈

- **UI框架**: LVGL 9.x
- **图形API**: OpenGL
- **平台**: Windows (Win32 API)
- **编译器**: MinGW-w64 GCC
- **构建系统**: CMake + Ninja
- **绘图**: GDI+ (圆角效果)

## 📋 系统要求

- Windows 10 或更高版本
- 支持OpenGL 2.0+ 的显卡
- MinGW-w64 工具链
- CMake 3.16+
- Ninja 构建工具

## 🔧 编译说明

### 1. 克隆项目
```bash
git clone https://github.com/helong70/lvgl_win_demo.git
cd lvgl_win_demo
```

### 2. 构建项目
```bash
# 使用构建脚本
.\build.bat

# 或手动构建
mkdir build
cd build
cmake .. -G Ninja
ninja
```

### 3. 运行演示
```bash
.\build\bin\LVGL_Windows_Demo.exe
```

## 🎮 使用说明

### 窗口操作
- **拖动窗口**: 点击标题栏任意位置拖动
- **关闭窗口**: 点击右上角 ❌ 按钮
- **最小化**: 点击右上角 ➖ 按钮

### UI交互
- **主按钮**: 点击切换文本和颜色
- **滑块**: 拖动调节数值，控制台显示当前值
- **下拉框**: 点击展开选择选项

### 键盘快捷键
- `1` - 模拟点击主按钮
- `2` - 模拟点击特定区域
- `q` - 退出程序

## 📁 项目结构

```
LVGL_WIN/
├── main.c              # 主程序源码
├── CMakeLists.txt      # CMake配置
├── build.bat          # 构建脚本
├── run.bat            # 运行脚本
├── lvgl/              # LVGL源码子模块
├── build/             # 构建输出目录
│   └── bin/           # 可执行文件
└── README.md          # 项目说明
```

## 🔧 开发亮点

### 1. OpenGL集成
```c
// OpenGL纹理上传和渲染
static void display_flush_cb(lv_display_t * disp, const lv_area_t * area, uint8_t * px_map)
{
    // RGB565 -> RGBA8888 转换
    // OpenGL纹理子图像更新
    // 硬件加速渲染
}
```

### 2. 原生窗口拖动
```c
case WM_NCHITTEST: {
    // 检测鼠标位置
    // 标题栏区域返回HTCAPTION
    // 启用Windows原生拖动
}
```

### 3. GDI+圆角效果
```c
// 使用GDI+绘制圆角窗口
Graphics graphics(hdc);
GraphicsPath path;
// 添加圆角路径
graphics.FillPath(&brush, &path);
```

## 🐛 已知问题

- ⚠️ 最小化后可能需要点击任务栏图标恢复
- ⚠️ 高DPI下某些UI元素可能需要微调
- ⚠️ GDI+圆角在某些显卡上可能有兼容性问题

## 🚧 开发计划

- [ ] 添加键盘输入支持
- [ ] 实现更多LVGL控件示例
- [ ] 优化渲染性能
- [ ] 添加主题切换功能
- [ ] 支持窗口大小调整

## 📝 更新日志

### v1.0.0 (2025-10-24)
- ✅ 初始版本发布
- ✅ OpenGL硬件加速渲染
- ✅ 无边框窗口设计
- ✅ 标题栏拖动功能
- ✅ 关闭和最小化按钮
- ✅ 基础UI控件演示

## 🤝 贡献指南

欢迎提交Issue和Pull Request！

1. Fork 项目
2. 创建功能分支
3. 提交更改
4. 推送到分支
5. 创建Pull Request

## 📄 许可证

本项目采用 MIT 许可证。详情请见 [LICENSE](LICENSE) 文件。

## 🙏 致谢

- [LVGL](https://lvgl.io/) - 优秀的嵌入式图形库
- [OpenGL](https://www.opengl.org/) - 跨平台图形API
- [MinGW-w64](https://www.mingw-w64.org/) - Windows下的GCC工具链