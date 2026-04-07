# Flight Dashboard (飞行仪表盘 / 姿态仪) ✈️
<img width="4096" height="1520" alt="8bc24adaf6523fa68de40099ae4c386b" src="https://github.com/user-attachments/assets/25fe8abb-0aa5-4681-92a3-e4b7b42c4295" />

## 📖 项目简介

本项目是一个基于微控制器（如 ESP32、STM32 等）和 240x320 TFT 彩色屏幕的**高性能飞行仪表盘（Cockpit Dashboard / 姿态仪）程序。
系统通过直接解析 MAVLink 协议，与常见飞行控制器（如 Pixhawk、APM 等）进行实时双向通信，高效获取飞行器的姿态（Pitch, Roll, Yaw）、高度、速度等关键数据，并在 TFT 屏幕上以 60FPS 级别的流畅度动态渲染出逼真的人工地平线 (Artificial Horizon)** 及飞行参数 UI。

非常适合应用于：**无人机地面站 (GCS) 拓展显示**、**模拟飞行器外设**、或**高阶遥控模型 (RC) 的车载/机载仪表演示**。

-----

## 🌿 多版本分支说明 (Branch & Versions)

为了适配不同的硬件性能和应用场景，本项目目前维护了 **三个核心分支**，请根据您的实际需求 `checkout` 对应的分支：

1.  **`v1` (2.4寸屏版)**
      * **特点**: 功能最全，渲染最流畅，包含所有最新的 UI 特性和动画效果。
      * **适用硬件**: 推荐使用 **ESP32**（利用双核优势和高主频）或带硬件浮点运算的 **STM32F4/F7** 系列。
      * **屏幕适配**: 240x320 彩色 TFT 屏幕（SPI 高速驱动）。
2.  **`v2` (2.4寸屏单文件版)**
      * **特点**: 裁剪了部分复杂的渐变和高清位图贴图，优化了内存占用（RAM）和 CPU 开销。
      * **适用硬件**: 推荐使用 **ESP32**（利用双核优势和高主频）或带硬件浮点运算的 **STM32F4/F7** 系列。
      * **屏幕适配**: 240x320 彩色 TFT 屏幕（SPI 高速驱动）。
3.  **`v3` (1.69寸屏platformio版)**
      * **特点**: 项目早期的稳定版本，保留了最基础的地平线逻辑和单色/简单色彩 UI 风格，代码结构最简单。
      * **适用硬件**: 推荐使用 **ESP32**（利用双核优势和高主频）或带硬件浮点运算的 **STM32F4/F7** 系列。
      * **屏幕适配**: 240x280 彩色 TFT 屏幕（SPI 高速驱动）。

-----
![2ae45a44c11238e04cae7ef38fb884ec](https://github.com/user-attachments/assets/5e14dd4e-feb6-4c98-8ea4-dee050257e35)
![1d5a1c842bcbf543f0f4d62fefa5ecee](https://github.com/user-attachments/assets/9bd1b51a-1482-426b-9fe5-a49b7eb158bd)
![09a5ebdd4a727978722f5ba995b00a17](https://github.com/user-attachments/assets/bcf776fc-fca4-4732-9f02-6aca183fe7fd)
![9e0f775d01c6461abc960d8cc254dbb6](https://github.com/user-attachments/assets/bc00e90b-122c-403b-a98b-8daf31dfa95f)
<img width="4094" height="2302" alt="9a2172571f3c6bb6a2c156c7f1abf3e3" src="https://github.com/user-attachments/assets/8a8a6244-ef5a-4c16-9d29-b8dcb1b3e8f2" />

## 🛠️ 技术实现与核心架构 (深度解析)

本项目的难点在于**在资源受限的微控制器上实现高频的协议解析与无闪烁的实时图形渲染**。核心逻辑被模块化拆分为多个 `.ino` 和 `.h` 文件。

### 1\. 数据通信与协议解析 (`mavlink_data.ino`)

  * **异步串口监听**: 程序通过硬件串口（HardwareSerial）以非阻塞方式监听飞控的 Telemetry 端口（默认波特率 57600 或 115200）。
  * **MAVLink 数据包解包**: 实时捕获并解析 MAVLink 心跳包 (Heartbeat) 及核心消息帧。主要提取 `MSG_ID_ATTITUDE`（俯仰角 Pitch、横滚角 Roll、偏航角 Yaw）和 `MSG_ID_VFR_HUD`（空速、地速、高度、油门等）。
  * **数据过滤与平滑**: 为了防止屏幕 UI 剧烈抖动，对接收到的原生角度数据进行了低通滤波处理，确保仪表盘指针移动顺滑。

### 2\. 姿态数学建模与坐标系转换 (`cal_att_cord.ino`)

这是项目的**数学核心**。飞控传回的欧拉角是 3D 空间数据，而屏幕是 2D 的。

  * **横滚计算 (Roll Rotation)**: 使用三角函数 (`sin`, `cos`) 建立二维旋转矩阵。根据当前的 Roll 角度，实时计算地平线（天际线）在屏幕左右边缘的交点坐标。
  * **俯仰计算 (Pitch Translation)**: 俯仰角决定了地平线在屏幕 Y 轴上的平移量。程序内部建立了一套映射比例（如：1度 Pitch 对应屏幕上的 N 个像素），将角度线性映射为 Y 轴偏移。
  * **边界裁剪算法**: 为了防止绘制地平线时越过仪表盘的圆形/方形边界，采用了快速多边形裁剪逻辑，确保“蓝天”和“土地”色块被严格限制在仪表盘 UI 内部。

### 3\. 高性能图形渲染引擎 (`draw_main_screen.ino` & `draw_new_attitude.ino`)

为了避免传统的逐像素重绘导致的“屏幕撕裂”或“严重闪烁”，本项目在图形栈上做了深度优化：

  * **局部刷新与 Sprite 缓存区 (推屏技术)**: 借助 `TFT_eSPI` 库，在 MCU 的 RAM 中开辟一块显存缓冲区（Sprite）。所有动态的地平线更新、刻度线旋转，首先在内存中的 Sprite 上完成绘制，然后通过 `push_oled.ino` 将整块内存数据通过高速 SPI 甚至 DMA 一次性推送到屏幕，实现无缝刷新。
  * **静态与动态层分离**:
      * `draw_main_screen.ino` 负责渲染无需频繁变化的**静态遮罩**（如仪表外边框、固定的文字标签、十字准星）。
      * `draw_new_attitude.ino` 专门负责高频更新的**动态图层**（姿态球背景色填充、动态刻度、数值文本更新）。

### 4\. 资源与内存管理 (`cockpit_var.h` & 资源头文件)

  * **全局状态机**: `cockpit_var.h` 中定义了共享的全局结构体和枚举，管理当前连接状态、上一帧的姿态数据（用于差异比对，仅在数值改变时触发重绘，极大节省 CPU）。
  * **PROGMEM 贴图直读**: 所有的位图 UI（如飞机模型 `j20_109_182.h`、特殊图标 `flight_35_9.h`）均已被转换为 C 语言 `const unsigned char` 数组，并使用 `PROGMEM` 关键字存储在 Flash 中。这免去了外挂 SD 卡的 IO 瓶颈，实现极速贴图。
  * **多语言字库**: 包含中英文字库 `tftespi_cn_fonts.h` 与 `tftespi_en_fonts.h`，支持无乱码的本地化数据显示。

-----

## 💻 硬件需求与连线

  * **核心板**: ESP32 (推荐 NodeMCU / WROOM) 或 STM32 开发板。
  * **显示屏**: 240x320 SPI TFT 液晶屏（支持 ILI9341, ST7789, ST7796 等主流驱动 IC）。
  * **接线方式**:

| 飞控 (Pixhawk/APM) | 微控制器 (MCU) | 备注 |
| :--- | :--- | :--- |
| Telemetry TX | MCU RX (如 GPIO 16) | 接收飞控数据 |
| Telemetry RX | MCU TX (如 GPIO 17) | *(可选)* 发送请求或心跳 |
| GND | GND | **必须共地** |

-----

## ⚙️ 安装与编译指南

1.  **安装依赖库**:
    将本项目根目录 `libraries/` 文件夹下的所有第三方库（`TFT_eSPI`, `MAVLink`, `U8g2`, `Adafruit_GFX` 等）复制到您的 Arduino 本地库目录中。

      * Windows: `C:\Users\<用户名>\Documents\Arduino\libraries\`
      * Mac: `~/Documents/Arduino/libraries/`

2.  **配置屏幕驱动 (TFT\_eSPI)**:

      * **关键步骤**：打开您刚才复制的 `TFT_eSPI/User_Setup.h` 文件。
      * 根据您使用的屏幕芯片（如 ILI9341）取消相应的宏注释。
      * 修改文件中针对 SPI 引脚的定义（如 MOSI, SCLK, CS, DC, RST），使其与您开发板的实际物理连线一致。

3.  **编译与烧录**:

      * 打开 Arduino IDE，载入 `main/main.ino`。
      * 选择目标开发板型号（如 `ESP32 Dev Module`）。
      * 选择对应的串口。
      * 点击**上传**。

-----

## 📄 许可证 (License)

本项目代码本体基于 MIT 协议开源。
本项目内置依赖的第三方库（TFT\_eSPI, U8g2, MAVLink, Adafruit\_GFX 等）均遵循其原作者的开源许可证（如 GPL、BSD 等）。详细信息请查阅 `libraries/` 各子目录下的 `LICENSE` 文件。

-----
