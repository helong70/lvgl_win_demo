# LVGL Windows Demo

这是一个简单的LVGL Windows移植示例，使用LVGL内置的Windows驱动。

## 依赖要求

### 必要的依赖：
1. **CMake** (版本 3.16 或更高)
   - 下载: https://cmake.org/download/

2. **Microsoft Visual Studio** 或 **Visual Studio Build Tools**
   - Visual Studio 2019 或 2022 (Community版本免费)
   - 需要安装"使用C++的桌面开发"工作负载
   - 或者只安装Visual Studio Build Tools

### 可选编译器：
- **MinGW-w64** (如果不使用Visual Studio)
- **MSYS2** (包含MinGW-w64)

## 编译步骤

### 方法 1: 使用Ninja构建 (推荐，最快)

```cmd
.\build_ninja.bat
```

需要安装Ninja构建工具。如果您还没有安装Ninja，可以：
1. 从 https://github.com/ninja-build/ninja/releases 下载ninja.exe
2. 解压到某个目录（如 `D:\ninja-win`）
3. 修改 `build_ninja.bat` 中的 `NINJA_PATH` 变量

### 方法 2: 使用MinGW Makefiles

```cmd
.\build_mingw.bat
```

### 方法 3: 自动检测编译器 (兼容性最好)

```cmd
.\build.bat
```

这个脚本会自动检测可用的编译器并选择最合适的构建工具。

### 方法 4: 手动编译

#### 使用Visual Studio：

```cmd
# 创建构建目录
mkdir build
cd build

# 使用Visual Studio 2022
cmake .. -G "Visual Studio 17 2022" -A x64

# 或使用Visual Studio 2019
cmake .. -G "Visual Studio 16 2019" -A x64

# 编译项目
cmake --build . --config Release
```

#### 使用MinGW (如果已安装)：

```cmd
mkdir build
cd build
cmake .. -G "MinGW Makefiles"
mingw32-make
```

## 运行

编译成功后，可执行文件将位于：
- `build/bin/LVGL_Windows_Demo.exe`

运行方式：
```cmd
# 直接运行
.\run.bat

# 或手动运行
.\build\bin\LVGL_Windows_Demo.exe
```

程序运行后，您应该能看到：
- 一个可点击的按钮（点击会改变颜色和文字）
- 一个可拖动的滑块
- 窗口标题
- 完整的鼠标和键盘交互支持

## 项目结构

```
LVGL_WIN/
├── lvgl/                 # LVGL库源码
│   ├── lv_conf.h        # LVGL配置文件 (已配置)
│   └── src/             # LVGL源码
├── main.c               # 主程序
├── CMakeLists.txt       # 主项目CMake配置
├── build_ninja.bat     # Ninja构建脚本 (推荐，最快)
├── build_mingw.bat     # MinGW构建脚本
├── build.bat           # 自动检测编译器的构建脚本
├── run.bat             # 运行程序的脚本
└── README.md           # 本文件
```

## 配置说明

### LVGL配置 (lv_conf.h)
- 启用了Windows驱动 (`LV_USE_WINDOWS = 1`)
- 操作系统设置为Windows (`LV_USE_OS = LV_OS_WINDOWS`)
- 颜色深度设置为16位 (`LV_COLOR_DEPTH = 16`)
- 禁用了SDL2支持 (不需要外部依赖)

### 示例功能
当前示例包含：
- 一个简单的按钮
- 一个滑块控件
- 标题文本
- 原生Windows窗口支持
- 鼠标和键盘输入支持

## 故障排除

### 常见问题：

1. **CMake错误：找不到编译器**
   - 确保已安装Visual Studio或Visual Studio Build Tools
   - 或者安装MinGW-w64
   - 运行`.\build.bat`会自动检测可用的编译器

2. **链接错误**
   - 确保使用64位编译器 (推荐)
   - 检查Windows SDK是否已安装

3. **运行时错误**
   - 确保在Windows 10或更高版本运行
   - 检查是否有足够的内存

### 手动指定编译器：

如果自动检测失败，可以手动指定：

```cmd
# 使用特定的Visual Studio版本
cmake .. -G "Visual Studio 17 2022" -A x64

# 使用NMake (需要在开发者命令提示符中运行)
cmake .. -G "NMake Makefiles"

# 使用MinGW
cmake .. -G "MinGW Makefiles"
```

## 优势

相比SDL2版本的优势：
- **无外部依赖**: 只需要Windows系统库
- **原生Windows支持**: 完全集成Windows消息循环
- **更小的可执行文件**: 不需要链接SDL2
- **更好的Windows集成**: 支持DPI缩放、主题等

## 进一步开发

要扩展此演示：
1. 修改 `ui_init()` 函数添加更多LVGL组件
2. 在 `lv_conf.h` 中启用需要的LVGL功能
3. 添加自定义主题和样式
4. 集成图片和字体资源
5. 添加文件系统支持

更多LVGL文档请参考：https://docs.lvgl.io/