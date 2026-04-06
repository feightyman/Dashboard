# Flight Dashboard (飞行仪表盘 / 姿态仪)

## 项目简介

本项目是一个基于 Arduino 和 240x320 TFT 彩色屏幕的飞行仪表盘（Cockpit Dashboard）程序。它通过 MAVLink 协议与飞行控制器（如 Pixhawk 等）进行通信，接收实时的飞行数据，并在屏幕上动态绘制姿态仪（Artificial Horizon，人工地平线）、飞行参数和仪表 UI。

该项目非常适合用于无人机地面站、模拟飞行器、或者遥控模型的高级车载仪表演示。

## 硬件需求

* **微控制器**: 推荐使用 **ESP32** 或性能较强的 ARM 架构开发板（如 STM32 等）。因为实时渲染仪表盘和处理高频的 MAVLink 数据流对算力和内存有较高要求。
* **显示屏**: 分辨率为 **240x320** 的 SPI TFT 彩色液晶屏（如基于 ILI9341、ST7789、ST7796 等驱动芯片的屏幕）。

## 软件依赖

项目已经内置了所需的第三方驱动库（位于 `libraries/` 目录下），主要包括：

1.  **TFT_eSPI** (Bodmer): 用于高效且快速地驱动 TFT 屏幕并进行硬件 SPI 渲染。
2.  **MAVLink**: 用于解析和封装 MAVLink (Micro Air Vehicle Link) 无人机通信协议。
3.  **Adafruit_GFX & Adafruit_BusIO**: 提供基础的图形和几何绘制接口。
4.  **U8g2**: 提供单色显示处理和字体支持。

## 核心代码结构 (`main/` 目录)

项目的核心业务逻辑拆分在多个 `.ino` 和头文件中：

* `main.ino`: 主程序入口，包含硬件的 `setup()` 初始化和 `loop()` 主循环。
* `cockpit_var.h`: 全局配置文件，定义了仪表盘所需的结构体、宏定义以及核心飞行变量。
* `mavlink_data.ino`: 通信模块，负责监听串口，读取并解析从飞控传来的 MAVLink 消息包。
* `draw_main_screen.ino`: 负责绘制仪表盘的静态背景、UI 框架和刻度线。
* `draw_new_attitude.ino`: 核心渲染模块，根据当前接收到的俯仰角（Pitch）和横滚角（Roll）动态更新姿态仪地平线。
* `cal_att_cord.ino`: 数学运算模块，处理姿态旋转、三角函数和屏幕坐标系转换的计算。
* `push_oled.ino`: 显示刷新控制，负责将内存中的图像数据推送到 TFT 屏幕上。

### 资源文件
* **图标与贴图**: `j20_109_182.h`、`flight_35_9.h`、`salt_*.h` 等文件存储了转换为 C 语言数组的图像/图标数据（例如飞机模型图标）。
* **字体**: `tftespi_cn_fonts.h` 和 `tftespi_en_fonts.h` 包含了界面所需的中英文字库数据。

## 安装与配置说明

1.  **安装库文件**:
    请将项目根目录 `libraries/` 文件夹下的所有子文件夹（`TFT_eSPI`, `MAVLink`, `U8g2`, 等）复制到你的 Arduino 本地库目录中（通常为 `文档/Arduino/libraries/`）。
2.  **配置 TFT_eSPI (关键)**:
    打开 `libraries/TFT_eSPI/User_Setup.h` 文件，根据你实际使用的屏幕驱动芯片和连接的引脚，取消对应的注释并修改引脚定义（如 MOSI, SCLK, CS, DC, RST 等）。
3.  **编译与烧录**:
    使用 Arduino IDE 打开 `main/main.ino`，选择对应的开发板型号和串口，点击编译并上传。

## 数据通信

* 系统默认通过硬件串口（Serial）接收数据。
* 将飞行控制器的 **Telemetry (数传)** 端口的 `TX` 引脚连接到微控制器的 `RX` 引脚。
* 请确保飞控端和微控制器端的波特率（Baudrate）设置一致（通常 MAVLink 默认使用 57600 或 115200）。

## 许可证

本项目所依赖的库（TFT_eSPI, U8g2, MAVLink 等）均遵循其原有的开源许可证。详细信息请参阅各自目录下的 `LICENSE` 文件。
