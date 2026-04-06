#include <Arduino.h>

#include <Adafruit_GFX.h>

#include <MAVLink.h>
#include "TFT_eSPI.h"
#include <math.h>
#include <U8g2lib.h>

#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif

#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>  //hardware I2C need this library
#endif


#include "cockpit_var.h"

#include "tftespi_cn_fonts.h"
#include "tftespi_en_fonts.h"
#include "j20_109_182.h" //歼20图标
#include "salt_45_43.h" //GPS卫星图标
#include "salt_25_25.h"

#include "flight_35_9.h"
#include "flight_64_9.h"
#include "flight_70_9.h"
// 定义箭头参数
#define ARROW_LENGTH 10    // 箭头分支长度
#define ARROW_ANGLE  45    // 箭头分支与主线的夹角（单位：度）



// 双核运行所需handle，用于追踪执行情况
TaskHandle_t Task1;
TaskHandle_t Task2;


TFT_eSPI tft = TFT_eSPI();
TFT_eSprite spr = TFT_eSprite(&tft);  // Declare Sprite object "spr" with pointer to "tft" object
TFT_eSprite spr_meters = TFT_eSprite(&tft);
TFT_eSprite spr_meters_2 = TFT_eSprite(&tft);
TFT_eSprite spr_jet = TFT_eSprite(&tft);
TFT_eSprite spr_status = TFT_eSprite(&tft);
TFT_eSprite spr_left_bar = TFT_eSprite(&tft);
TFT_eSprite spr_left_rec = TFT_eSprite(&tft);
TFT_eSprite spr_right_bar = TFT_eSprite(&tft);
TFT_eSprite spr_right_rec = TFT_eSprite(&tft);
TFT_eSprite spr_mid = TFT_eSprite(&tft);
TFT_eSprite spr_under = TFT_eSprite(&tft);
TFT_eSprite spr_compass = TFT_eSprite(&tft);

//定义下方副显示屏（2号屏）的虚拟对象，起名u8g2。无法同时开启两个硬件I2C协议，用软件I2C。FPS较低。
U8G2_SSD1306_72X40_ER_F_SW_I2C u8g2(U8G2_R0, /* clock=*/ 41, /* data=*/ 40, /* reset=*/ U8X8_PIN_NONE);  


//定义最上方副显示屏（1号屏）的虚拟对象，起名u8g1。1号屏用硬件I2C协议，FPS高，用于显示电流数据。
U8G2_SSD1306_96X16_ER_F_HW_I2C u8g1(U8G2_R0, /* reset=*/ U8X8_PIN_NONE, /* clock=*/ 1, /* data=*/ 2);


bool change_flag=false;


void cal_att_cord(){
  // 计算pitch后r0长度
  r0 = pitch*pitch_to_pixel;
  alpha = HALF_PI + roll;
  beta = HALF_PI + alpha;
  cosa = cos(alpha);
  sina = sin(alpha);
  cosb = cos(beta);
  sinb = sin(beta);
    
  x0 = r0*cosa + x_center; // 0度姿态线中点横坐标
  y_0 = r0*sina + y_center; // 0度姿态线中点纵坐标
  vx = level_pixel*cosa;
  vy = level_pixel*sina;

  center_degree = (pitch)/PI*180; // 把幅度pitch换算成角度
  base_vect = round(center_degree/level_angle); // x0,y_0到姿态仪中心点有几条姿态线
  
  // 0度姿态线坐标
  xa1 = x0+cosb*r1;
  ya1 = y_0+sinb*r1;
  xa2 = x0+cosb*r2;
  ya2 = y_0+sinb*r2;  
  xa3 = x0-cosb*r3;
  ya3 = y_0-sinb*r3;
  xa4 = x0-cosb*r4;
  ya4 = y_0-sinb*r4;

  // 计算姿态仪中心姿态线坐标，也就是第五行坐标：att_points[4]
  att_points[4][0] = xa1-vx*base_vect;
  att_points[4][1] = ya1-vy*base_vect;
  att_points[4][2] = xa2-vx*base_vect;
  att_points[4][3] = ya2-vy*base_vect;
  att_points[4][4] = xa3-vx*base_vect;
  att_points[4][5] = ya3-vy*base_vect;
  att_points[4][6] = xa4-vx*base_vect;
  att_points[4][7] = ya4-vy*base_vect;
  
  // 根据姿态仪中心姿态线坐标，计算其他几条姿态线坐标
  for (int i = 0; i < num_att_lines; i++) {      
    int j = i - 4; // j用于标记姿态线距离姿态仪中心姿态线的距离
    att_points[i][0] = att_points[4][0] - j * vx;
    att_points[i][1] = att_points[4][1] - j * vy;
    att_points[i][2] = att_points[4][2] - j * vx;
    att_points[i][3] = att_points[4][3] - j * vy;
    att_points[i][4] = att_points[4][4] - j * vx;
    att_points[i][5] = att_points[4][5] - j * vy;
    att_points[i][6] = att_points[4][6] - j * vx;
    att_points[i][7] = att_points[4][7] - j * vy;
  }  
  
  // 长的0度地平线坐标
  xb1 = x0+cosb*l1;
  yb1 = y_0+sinb*l1;
  xb2 = x0+cosb*l2;
  yb2 = y_0+sinb*l2;  
  xb3 = x0-cosb*l3;
  yb3 = y_0-sinb*l3;
  xb4 = x0-cosb*l4;  
  yb4 = y_0-sinb*l4;  
  
  // b5 b6 b7 b8 构成了天空框， b5->b7, b6->b8
  xb5 = x0+cosb*l0;
  yb5 = y_0+sinb*l0;
  xb6 = x0-cosb*l0;
  yb6 = y_0-sinb*l0;    
  xb7 = x0+cosb*l0 - 9 * 2*vx;
  yb7 = y_0+sinb*l0 - 9 * 2*vy;
  xb8 = x0-cosb*l0 - 9 * 2*vx;
  yb8 = y_0-sinb*l0 - 9 * 2*vy;  
}

void draw_wifi()
{
   // 绘制无线信号图标
   int signalX = 0;  // 信号图标起始 x 坐标
   int signalY = 0;  // 信号图标起始 y 坐标
   int barWidth = 3; // 每格信号宽度
   int barHeight = 25; // 最高格信号高度
   int barGap = 2;   // 格与格之间的间距
 
   int num_bars; // 信号格数
   // 根据 rssi 计算要绘制的信号格数
   
   rssi=rssi>255?255:rssi;
   int signal_pecentage = (int)((float)rssi * 100/ 255); // 将 rssi 转换为百分比
   num_bars = signal_pecentage /20; // 每 20% 绘制一格信号
   //if (num_bars > 3) num_bars = 3; // 最多绘制三格信号
   // 绘制信号格
    // 处理闪烁逻辑
    unsigned long currentMillis = millis();
    if (signal_pecentage < 40) {
      if (currentMillis - blinkMillisWifi >= blinkIntervalWifi) {
        blinkMillisWifi = currentMillis;
        blinkStateWifi = !blinkStateWifi;
      }
      if (!blinkStateWifi) {
        return; // 不绘制图标，实现闪烁效果
      }
    }
   for (int i = 0; i < num_bars; i++) {
        int currentHeight = (i + 1) * barHeight / 5;
       spr_meters.fillRect(signalX + i * (barWidth + barGap), signalY + barHeight - currentHeight, barWidth, currentHeight, TFT_WHITE);
   }
   
   for (int j = 0; j < 5; j++) {
     int currentHeight = (j + 1) * barHeight / 5;
     spr_meters.drawRect(signalX + j * (barWidth + barGap), signalY + barHeight - currentHeight, barWidth, currentHeight, GRAY);
   }
   // 显示无线信号强度参数
   // spr_meters.loadFont(Final_Frontier_15);
   // spr_meters.setCursor(20, 50); 
   // signal_pecentage = (int)((float)rssi / 255 * 100); // 将 rssi 转换为百分比
   // spr_meters.print(signal_pecentage);
   // spr_meters.print("%"); // 显示百分号
   
   spr_meters.unloadFont();
}

void draw_meters(){
  // spr_meters.drawRect(2,0,275,240,GREEN);
  spr_meters.setTextColor(WHITE, BLACK);
  int x_start = 0;
  int y_start = 0;


  
  // for (int j = 0; j < 3; j++){
  //   spr_meters.fillSmoothRoundRect(x_start,j*81+y_start, 78, 78, 6, GRAY, TFT_BLACK);
  // } 
  //spr_meters.fillSmoothRoundRect(x_start,y_start, 63, 48, 6, GRAY, TFT_BLACK);
  //spr_meters.fillSmoothRoundRect(x_start,51+y_start, 63, 128, 6, GRAY, TFT_BLACK); 
  spr_meters.fillSmoothRoundRect(x_start,y_start,65,180,0,TFT_BLACK, TFT_BLACK); 
  // spr_meters.fillCircle(30, 1*51+y_start+55+30, 15, TFT_DARKGREEN);  // 外圈底色
  // spr_meters.fillCircle(30, 1*51+y_start+55+30, 15-3, TFT_GREEN);    // 内圈高光  
  //起落架状态条
  // spr_meters.drawRect(30-7.5, 1*51+y_start+55+15 , 15, 30, GREEN);
  // spr_meters.fillRect(30-7.5,  1*51+y_start+55+30+15-(chan11_raw-1000)*30/1000, 15, chan10_raw*30/100, GREEN);
  spr_meters.loadFont(Final_Frontier_15);
  spr_meters.drawRect(x_start+5, 1*51+y_start+55+10 , 55, 15, GREEN);
  spr_meters.fillRect(x_start+6, 1*51+y_start+55+11, 53, 13,GRAY);
  spr_meters.setCursor(x_start+33-5,1*51+y_start+55+13);spr_meters.print("U");
  spr_meters.fillRect(x_start+5, 1*51+y_start+55+10, (chan11_raw-1000)*65/1000, 15, GREEN);
  //???状态条
  spr_meters.drawRect(x_start+5, 1*51+y_start+55+10+25 , 55, 15, TFT_YELLOW);
  spr_meters.fillRect(x_start+6, 1*51+y_start+55+11+25, 53, 13,GRAY);
  spr_meters.setCursor(x_start+33-5,1*51+y_start+55+13+25);spr_meters.print("A");
  spr_meters.fillRect(x_start+5, 1*51+y_start+55+10+25, (chan1_raw-1000)*65/1000, 15, TFT_YELLOW);

  spr_meters.unloadFont();
  draw_wifi(); // 绘制无线信号图标
//原版
  // spr_meters.loadFont(Final_Frontier_20);
  // spr_meters.setCursor(x_start,55);spr_meters.print(distance,1);
  // spr_meters.setCursor(x_start+10, 25+55); spr_meters.print(groundspeed*3.6, 1);
  // spr_meters.unloadFont();
  // spr_meters.loadFont(Final_Frontier_15);
  // spr_meters.setCursor(x_start+15, 25+70);spr_meters.print("km/h");//空速换算成km/h
//改后
  spr_meters.loadFont(Final_Frontier_15);
  distance=distance<=1000?1000:distance;
  spr_meters.setCursor(x_start,35);spr_meters.print(distance,1);spr_meters.print("m");
  spr_meters.setCursor(x_start, 25+55); spr_meters.print(groundspeed*3.6, 1);spr_meters.print("km/h");
  // spr_meters.setCursor(x_start, 25+85);spr_meters.print(chan6_raw); 
  //spr_meters.drawSmoothArc(x_start+25, 1*51+y_start+65, 23, 21, 60, 300, BLACK, BLACK, true);
  //spr_meters.drawSmoothArc(x_start+25, 1*51+y_start+65, 23, 21, 60, int32_t(60+airspeed*240/150+1), YELLOW, BLACK, true); //预计速度最高不超过150km/h
  // spr_meters.setCursor(x_start+10, 1*81+y_start+30); spr_meters.print(groundspeed*3.6, 1); //地速换算成km/h
  // spr_meters.drawSmoothArc(x_start+38, 1*81+y_start+40, 36, 33, 60, 300, BLACK, BLACK, true);
  // spr_meters.drawSmoothArc(x_start+38, 1*81+y_start+40, 36, 33, 60, int32_t(60+groundspeed*240/150+1), YELLOW, BLACK, true); //预计速度最高不超过150km/h

  // if (alt > 100) {alt = 100;}
  // spr_meters.setCursor(x_start+10, 2*81+y_start+30); spr_meters.print(alt);spr_meters.print("m"); 
  // spr_meters.drawSmoothArc(x_start+38, 2*81+y_start+40, 36, 33, 60, 300, BLACK, BLACK, true);
  // spr_meters.drawSmoothArc(x_start+38, 2*81+y_start+40, 36, 33, 60, int32_t(60+alt*240/100+1), YELLOW, BLACK, true); //高度显示最多100m

  // spr_meters.loadFont(cn_ht_26);     
  // spr_meters.drawString("空速",x_start+12, 0*81+y_start+55); //从左上角开始计算坐标
  // spr_meters.drawString("地速",x_start+12, 1*81+y_start+55); //从左上角开始计算坐标
  // spr_meters.drawString("高度",x_start+12, 2*81+y_start+55); //从左上角开始计算坐标
  spr_meters.unloadFont();
  spr_meters.pushSprite(0, 0);
}

void draw_meters_2(){
  // spr_meters.drawRect(2,0,275,240,GREEN);
  spr_meters_2.setTextColor(WHITE, BLACK);
  int x_start = 0;
  int y_start = 0;
    // for (int j = 0; j < 3; j++){
    //   spr_meters_2.fillSmoothRoundRect(x_start,j*81+y_start, 78, 78, 6, GRAY, TFT_BLACK);
    // }
  //spr_meters_2.fillSmoothRoundRect(x_start,y_start, 63, 48, 6, GRAY, TFT_BLACK);
  //spr_meters_2.fillSmoothRoundRect(x_start,51+y_start, 63, 128, 6, GRAY, TFT_BLACK);
  spr_meters_2.fillSmoothRoundRect(x_start,y_start,65,180,0,TFT_BLACK, TFT_BLACK);
  // spr_meters_2.fillCircle(30, 1*51+y_start+55+30, 15, TFT_DARKGREEN);  // 外圈底色
  // spr_meters_2.fillCircle(30, 1*51+y_start+55+30, 15-3, TFT_GREEN);    // 内圈高光  
  //起落架状态条
  // spr_meters_2.drawRect(30-7.5, 1*51+y_start+55+15 , 15, 30, TFT_RED);
  // spr_meters_2.fillRect(30-7.5,  1*51+y_start+55+30+15-(chan10_raw-1000)*30/1000, 15, chan10_raw*30/100, TFT_RED);
  spr_meters_2.loadFont(Final_Frontier_15);
  spr_meters_2.drawRect(x_start+5, 1*51+y_start+55+10 , 55, 15, TFT_RED);
  spr_meters_2.fillRect(x_start+6, 1*51+y_start+55+11, 53, 13,GRAY);
  spr_meters_2.setCursor(x_start+33-5,1*51+y_start+55+13);spr_meters_2.print("L");
  spr_meters_2.fillRect(x_start+5, 1*51+y_start+55+10, (chan10_raw-1000)*65/1000, 15, TFT_RED);
  //???状态条
  spr_meters_2.drawRect(x_start+5, 1*51+y_start+55+10+25 , 55, 15, TFT_BLUE);
  spr_meters_2.fillRect(x_start+6, 1*51+y_start+55+11+25, 53, 13,GRAY);
  spr_meters_2.setCursor(x_start+33-5,1*51+y_start+55+13+25);spr_meters_2.print("B");
  spr_meters_2.fillRect(x_start+5, 1*51+y_start+55+10+25, (chan2_raw-1000)*65/1000, 15, TFT_BLUE);
  spr_meters_2.unloadFont();  
    // 绘制卫星信号图标
  //spr_meters_2.pushImage(5, 0, 45, 43, salt_45_43); 
  // 处理闪烁逻辑
  unsigned long currentMillis = millis();
  if (satellites_visible <= 5) {
    if (currentMillis - blinkMillis >= blinkInterval) {
      blinkMillis = currentMillis;
      blinkState = !blinkState;
    }
    if (blinkState) {
      // 绘制卫星信号图标
      spr_meters_2.pushImage(40, 0, 25, 25, salt_25_25); 
    }
  } else {
    // 卫星数量大于5，稳定显示
    spr_meters_2.pushImage(40, 0, 25, 25, salt_25_25); 
  }

  // 显示卫星信号强度参数
  /*spr_meters_2.loadFont(Final_Frontier_20);
  spr_meters_2.setTextColor(RED,RED);
  spr_meters_2.setCursor(20, 20); 
  spr_meters_2.print(satellites_visible);
  spr_meters_2.print("g"); // 添加单位
  spr_meters_2.unloadFont();*/
//原版
  // spr_meters_2.loadFont(Final_Frontier_20);
  // // spr_meters_2.setCursor(x_start+10, 0*81+y_start+30); spr_meters_2.print(double(current_battery)/100, 1); 
  // // spr_meters_2.drawSmoothArc(x_start+38, 0*81+y_start+40, 36, 33, 60, 300, BLACK, BLACK, true);
  // // spr_meters_2.drawSmoothArc(x_start+38, 0*81+y_start+40, 36, 33, 60, int32_t(60+current_battery*240/5000+1), YELLOW, BLACK, true); //最高电流按5000cA预估
  // spr_meters_2.setTextColor(WHITE,WHITE);
  // if (alt > 100) {alt = 100;} 
  // spr_meters_2.setCursor(x_start+10, 25+55);spr_meters_2.print(alt);
  // spr_meters_2.unloadFont();  
  // spr_meters_2.loadFont(Final_Frontier_15);
  // spr_meters_2.setCursor(x_start+30, 25+70);spr_meters_2.print("m"); 
//改后
  spr_meters_2.loadFont(Final_Frontier_15);
  // spr_meters_2.setCursor(x_start+10, 0*81+y_start+30); spr_meters_2.print(double(current_battery)/100, 1); 
  // spr_meters_2.drawSmoothArc(x_start+38, 0*81+y_start+40, 36, 33, 60, 300, BLACK, BLACK, true);
  // spr_meters_2.drawSmoothArc(x_start+38, 0*81+y_start+40, 36, 33, 60, int32_t(60+current_battery*240/5000+1), YELLOW, BLACK, true); //最高电流按5000cA预估
  spr_meters_2.setTextColor(WHITE,WHITE);
  total_distance=total_distance<=1000?1000:total_distance;
  spr_meters_2.setCursor(x_start,35);spr_meters_2.print(total_distance,1);spr_meters_2.print("m");
  if (alt > 100) {alt = 100;} 
  spr_meters_2.setCursor(x_start, 25+55);spr_meters_2.print(alt);spr_meters_2.print("m");
  //spr_meters_2.drawSmoothArc(x_start+25, 1*51+y_start+65, 23, 21, 60, 300, BLACK, BLACK, true);
  //spr_meters_2.drawSmoothArc(x_start+25, 1*51+y_start+65, 23, 21,60, int32_t(60+alt*240/100+1), YELLOW, BLACK, true); //高度显示最多100m

  // if (voltage_battery < 19200) {voltage_battery = 19200;}
  // spr_meters_2.setCursor(x_start+10, 1*81+y_start+30); spr_meters_2.print(double(voltage_battery)/6000, 1); //6s电池，计算每节电池电压
  // spr_meters_2.drawSmoothArc(x_start+38, 1*81+y_start+40, 36, 33, 60, 300, BLACK, BLACK, true);
  // spr_meters_2.drawSmoothArc(x_start+38, 1*81+y_start+40, 36, 33, 60, int32_t(60+(voltage_battery-19200)*240/6000+1), YELLOW, BLACK, true);//最高电压按6s电池25.2V预估

  // spr_meters_2.setCursor(x_start+10, 2*81+y_start+30); spr_meters_2.print(brightness_new*100/255); 
  // spr_meters_2.drawSmoothArc(x_start+38, 2*81+y_start+40, 36, 33, 60, 300, BLACK, BLACK, true);
  // spr_meters_2.drawSmoothArc(x_start+38, 2*81+y_start+40, 36, 33, 60, int32_t(60+brightness_new*240/255+1), YELLOW, BLACK, true); //亮度信号最高255

  // spr_meters_2.loadFont(cn_ht_26);     
  // spr_meters_2.drawString("电流",x_start+12, 0*81+y_start+55); //从左上角开始计算坐标  
  // spr_meters_2.drawString("电压",x_start+12, 1*81+y_start+55); //从左上角开始计算坐标  
  // spr_meters_2.drawString("亮度",x_start+12, 2*81+y_start+55); //从左上角开始计算坐标  
  spr_meters_2.unloadFont();  
  spr_meters_2.pushSprite(215, 0);
}

void draw_mid(){
  spr_mid.fillSprite(BLACK); 
  // spr_mid.drawRect(0, 0, 200, 240,GREEN);
  spr_mid.setSwapBytes(true); //纠正pushimage颜色不对的问题
  spr_mid.pushImage(45, 0, 109, 182, j20_109_182); 
  spr_mid.pushImage(5, 0, 45, 43, salt_45_43); 

  spr_mid.loadFont(Final_Frontier_15);
  spr_mid.fillRect(55, 10, 35, 30, BLACK);
  spr_mid.setCursor(55, 10); spr_mid.print(satellites_visible);
  // spr_mid.fillRect(130, 10, 70, 30, BLACK);
  time_str = String(time_boot_ms/1000/60)+":"+String(time_boot_ms/1000%60);
  spr_mid.drawString(time_str, 130, 10);
  spr_mid.unloadFont();
//current_consumed 电量状态条，4000mah为100%
  spr_mid.drawRect(170, 55, 20, 120,GREEN);
  spr_mid.fillRect(170, 55+current_consumed*120/4000, 20, 120-current_consumed*120/4000, GREEN);
//油门状态条
  spr_mid.drawRect(13, 55, 20, 120, GREEN);
  spr_mid.fillRect(13, 55+120-throttle*120/100, 20, throttle*120/100, GREEN);


  spr_mid.setTextColor(WHITE, BLACK);
  spr_mid.loadFont(cn_ht_26);
  spr_mid.drawString("油",10,185); //从左上角开始计算坐标
  spr_mid.drawString("门",10,215); //从左上角开始计算坐标
  spr_mid.drawString("电",165,185); //从左上角开始计算坐标
  spr_mid.drawString("量",165,215); //从左上角开始计算坐标

  spr_mid.fillSmoothRoundRect(45,190, 109, 50, 6, BLUE, TFT_BLACK);
  spr_mid.drawString("系统状态", 50, 195); //从左上角开始计算坐标
  spr_mid.setCursor(50, 220); 
  spr_mid.print(system_status_str);
  // spr_mid.setCursor(50, 220); spr_mid.print("rssi:");spr_mid.print(rssi)
  // spr_mid.print("FPS:");spr_mid.print(fps0);
  spr_mid.unloadFont();
  // spr_mid.pushSprite(0, 0);
}

void draw_under() {
    
    spr_under.fillSprite(BLACK); // 设置背景为黑色
    spr_under.setTextColor(WHITE, BLACK);

    int area_width = (280 - 2 * 5) / 3;  // 每个区域的宽度，减去间隔距离
    int x_start = 0;  // 起始 x 坐标，留出左边间隔
    int y_start = 0;
    int gap = 5; // 相邻区域之间的间隔距离

    // 绘制油门信息
    // spr_under.fillSmoothRoundRect(x_start, y_start, area_width, 60, 6, GRAY, TFT_BLACK);
    spr_under.loadFont(Final_Frontier_15);
    spr_under.setCursor(x_start + 40, y_start + 20);  // 参数右移 40 像素
    throttle=throttle>100?0:throttle;
    spr_under.print(throttle);
    
    spr_under.print("%");
    spr_under.drawSmoothArc(x_start + area_width / 2, y_start + 30, 25, 22, 60, 300, GRAY, GRAY, true);
    spr_under.drawSmoothArc(x_start + area_width / 2, y_start + 30, 25, 22, 60, 60 + throttle * 240 / 100.0+1, YELLOW, BLACK, true);
    spr_under.unloadFont();

    // 绘制航向角信息
    // x_start += area_width + gap;
    // spr_under.fillSmoothRoundRect(x_start, y_start, area_width, 60, 6, GRAY, TFT_BLACK);
    // spr_under.loadFont(Final_Frontier_15);
    // spr_under.setCursor(x_start + 40, y_start + 20);  // 参数右移 40 像素
    // spr_under.print(heading);
    // spr_under.print("°");
    // spr_under.drawSmoothArc(x_start + area_width / 2, y_start + 30, 25, 22, 60, 300, BLACK, BLACK, true);
    // spr_under.drawSmoothArc(x_start + area_width / 2, y_start + 30, 25, 22, 60, int32_t(60 + heading * 240 / 360 + 1), YELLOW, BLACK, true);
    // spr_under.unloadFont();
    x_start += area_width + gap;
    // spr_under.fillSmoothRoundRect(x_start, y_start, area_width, 60, 6, GRAY, TFT_BLACK);
    
  // spr.drawSmoothArc(x_center, y_limit+30,60,59,120,240,WHITE,WHITE,false);
  // spr.drawSmoothArc(x_center,y_limit,25,24,90,270,WHITE,WHITE,false);
  // 绘制刻度线（每10°小刻度，每30°大刻度）
    spr_under.drawSmoothArc(x_start + area_width / 2, y_start + 30,18,17,0,360,WHITE,WHITE,true);
    spr_under.loadFont(Final_Frontier_15);
    for(int angle = 0; angle <= 360; angle += 45) {
      float rad = radians(angle);
      int x1 = x_start + area_width / 2 + (18 ) * sin(rad);
      int y1 = y_start + 30 - (18 ) * cos(rad);
      int x2 = x1 + (abs(angle%90)==0 ? 12 : 6) * sin(rad);
      int y2 = y1 - (abs(angle%90)==0 ? 12 : 6) * cos(rad);
      
      spr_under.drawLine(x2, y2, x1, y1, TFT_WHITE);
      
      // 添加主刻度标签
      // if(abs(angle%90) == 0) {
      //   int y_offset = 0;
      //   if(abs(angle)==90){
      //     y_offset = 10;
      //   }
      //   spr_under.setTextColor(TFT_WHITE, TFT_BLACK);
      //   spr_under.setTextDatum(MC_DATUM);
      //   spr_under.drawNumber(angle, 
      //     x_start + area_width / 2 + (25+15)*sin(rad),
      //     y_start + 30 - (25+15)*cos(rad)-y_offset);
      // }
    }

  // 绘制动态指针
    // float pointer_rad = radians(constrain(yaw/PI*180, -180, 180));
    // int tip_x =  x_start + area_width / 2 + (25-10) * sin(pointer_rad);
    // int tip_y = y_start + 30 - (25-10) * cos(pointer_rad);
    // spr_under.drawWedgeLine(x_start + area_width / 2, y_start + 30, tip_x, tip_y, 2, 1, TFT_RED); // 渐变线宽
    // spr_under.fillCircle(x_start + area_width / 2,y_start + 30,3,TFT_RED);
    // spr_under.unloadFont();
    // 绘制动态指针（等腰三角形）
    float pointer_rad = radians(constrain(yaw/PI*180, -180, 180));
    int x_center1 = x_start + area_width / 2;
    int y_center1 = y_start + 30;

    // 计算尖端坐标（与原指针长度一致）
    int tip_x = x_center1 + (25-10) * sin(pointer_rad);
    int tip_y = y_center1 - (25-10) * cos(pointer_rad);

    // 计算方向向量
    float dir_x = sin(pointer_rad);
    float dir_y = -cos(pointer_rad);

    // 定义三角形参数
    int base_width = 5;  // 底边宽度的一半（可调整）
    float perp_x = -dir_y;  // 垂直方向向量（顺时针旋转90度）
    float perp_y = dir_x;

    // 计算底边两个端点（基于中心点对称偏移）
    int left_x = x_center1 + perp_x * base_width;
    int left_y = y_center1 + perp_y * base_width;
    int right_x = x_center1 - perp_x * base_width;
    int right_y = y_center1 - perp_y * base_width;

    // 绘制填充三角形
    spr_under.fillTriangle(tip_x, tip_y, left_x, left_y, right_x, right_y, TFT_RED);
    // 保留中心红点
    spr_under.fillCircle(x_center1, y_center1, 3, TFT_RED);

    spr_under.unloadFont();
    // 绘制电压信息
    x_start += area_width + gap;
    // spr_under.fillSmoothRoundRect(x_start, y_start, area_width, 60, 6, GRAY, TFT_BLACK);
    spr_under.loadFont(Final_Frontier_15);
    // if (voltage_battery < 19200) {
    //     voltage_battery = 19200;
    // }
    spr_under.setCursor(x_start + 30, y_start + 20);  // 参数右移 30 像素
    spr_under.print(double(voltage_battery) / 6000, 1);
    spr_under.print("V");
    spr_under.drawSmoothArc(x_start + area_width / 2, y_start + 30, 25, 22, 60, 300, GRAY, GRAY, true);
    spr_under.drawSmoothArc(x_start + area_width / 2, y_start + 30, 25, 22, 60, int32_t(240*3.7/4.2+ (voltage_battery - 19200) * 240/36000  + 1), YELLOW, BLACK, true);
    spr_under.unloadFont();

    // 将 Sprite 推送到主屏幕上，假设姿态仪高度为 180
    spr_under.pushSprite(0, 180);
}
void draw_new_attitude(){

  //覆盖原姿态线
  spr.fillRect(1,1,x_limit-1,y_limit-1,TFT_BROWN);  
  spr.fillTriangle(xb5,yb5,xb6,yb6,xb7,yb7,BLUE);
  // spr.fillTriangle(xb5,yb5,xb6,yb6,0,0,BLUE);
  spr.fillTriangle(xb8,yb8,xb6,yb6,xb7,yb7,BLUE);
  // spr.fillRect(xb7,yb7,xb8-xb7,yb8-yb6,BLUE); 
  spr.loadFont(Final_Frontier_15);  

  spr.drawRect(0,0,x_limit,y_limit,YELLOW);//画出姿态线的框框
  // spr.fillTriangle(x_center, y_center,x_center-35,y_center+9,x_center+35,y_center+9, RED);//画出飞机标记
  // spr.pushImage(x_center-35,y_center,70,9,flight_70_9);
  spr.drawWideLine(x_center-35, y_center, x_center-5, y_center, 4, GREEN);
  spr.drawWideLine(x_center+35, y_center, x_center+5, y_center, 4, GREEN);
  spr.drawWideLine(x_center-5,y_center,x_center,y_center+5,4,GREEN);
  spr.drawWideLine(x_center+5,y_center,x_center,y_center+5,4,GREEN);
  //spr.drawSmoothArc(x_center,y_limit,25,24,90,270,WHITE,WHITE,false);
  spr.drawSmoothArc(x_center, y_center,arc_radius,arc_radius-1,120,240,GREEN,GREEN,false);
  spr.fillTriangle(x_center, y_center-arc_radius,x_center-5,0,x_center+5,0, RED);
  //绘制刻度线（每10°小刻度，每30°大刻度）
  for(int angle = -60; angle <= 60; angle += 5) {
    float rad = radians(angle);
    int x1 = x_center + (arc_radius ) * sin(rad);
    int y1 = y_center - (arc_radius ) * cos(rad);
    int x2,y2;
    if(angle>=-30&&angle<=30)
    {
      x2 = x1 + (abs(angle%30)==0 ? 12 : (abs(angle%10)==0 ? 6 : 0)) * sin(rad);
      y2 = y1 - (abs(angle%30)==0 ? 12 : (abs(angle%10)==0 ? 6 : 0)) * cos(rad);
      spr.drawLine(x1, y1, x2, y2, GREEN);
    }
    else
    {
      x2 = x1 + (abs(angle%30)==0 ? 12 : (abs(angle%15)==0 ? 6 : 0)) * sin(rad);
      y2 = y1 - (abs(angle%30)==0 ? 12 : (abs(angle%15)==0 ? 6 : 0)) * cos(rad);
      spr.drawLine(x1,y1,x2,y2,GREEN);
    }
    
    // // 添加主刻度标签
    // if(abs(angle%30) == 0) {
    //   int y_offset = 0;
    //   if(abs(angle)==90){
    //     y_offset = 10;
    //   }
    //   spr.setTextColor(TFT_WHITE, TFT_BLACK);
    //   spr.setTextDatum(MC_DATUM);
    //   spr.drawNumber(angle, 
    //                 x_center + (arc_radius+15)*sin(rad),
    //                 y_center - (arc_radius+15)*cos(rad)-y_offset);
    // }

  }


// 计算指针角度
float pointer_rad = radians(-constrain(roll/PI*180, -60, 60));

// 箭头尖端坐标（紧贴圆弧）
int tip_x = x_center + arc_radius * sin(pointer_rad);
int tip_y = y_center - arc_radius * cos(pointer_rad);

// 箭头两侧点坐标
float arrow_angle_offset = radians(5);
int left_x = x_center + (arc_radius - 8) * sin(pointer_rad - arrow_angle_offset);
int left_y = y_center - (arc_radius - 8) * cos(pointer_rad - arrow_angle_offset);
int right_x = x_center + (arc_radius - 8) * sin(pointer_rad + arrow_angle_offset);
int right_y = y_center - (arc_radius - 8) * cos(pointer_rad + arrow_angle_offset);

// 绘制箭头
spr.drawLine(tip_x, tip_y, left_x, left_y, TFT_YELLOW);
spr.drawLine(tip_x, tip_y, right_x, right_y, TFT_YELLOW);
spr.fillTriangle(tip_x, tip_y, left_x, left_y, right_x, right_y, TFT_YELLOW);




//0刻度线的绘制
double roll_angle=roll/PI*180;
  // 左半段箭头绘制

double left_angle = roll_angle + 180 - ARROW_ANGLE; // 左箭头分支角度（考虑滚转）
// int arrow_lx1 = xb2 - ARROW_LENGTH * cos(left_angle * DEG_TO_RAD);
// int arrow_ly1 = yb2 - ARROW_LENGTH * sin(left_angle * DEG_TO_RAD);
int arrow_lx2 = xb2 - ARROW_LENGTH * cos((left_angle + 2*ARROW_ANGLE) * DEG_TO_RAD);
int arrow_ly2 = yb2 - ARROW_LENGTH * sin((left_angle + 2*ARROW_ANGLE) * DEG_TO_RAD);
// spr.drawWideLine(xb2, yb2, arrow_lx1, arrow_ly1, 2, GREEN);
spr.drawWideLine(xb2, yb2, arrow_lx2, arrow_ly2, 2, WHITE);

// 右半段箭头绘制
double right_angle = roll_angle + ARROW_ANGLE; // 右箭头分支角度
// int arrow_rx1 = xb3 - ARROW_LENGTH * cos(right_angle * DEG_TO_RAD);
// int arrow_ry1 = yb3 - ARROW_LENGTH * sin(right_angle * DEG_TO_RAD);
int arrow_rx2 = xb3 - ARROW_LENGTH * cos((right_angle - 2*ARROW_ANGLE) * DEG_TO_RAD);
int arrow_ry2 = yb3 - ARROW_LENGTH * sin((right_angle - 2*ARROW_ANGLE) * DEG_TO_RAD);
// spr.drawWideLine(xb3, yb3, arrow_rx1, arrow_ry1, 2, GREEN);
spr.drawWideLine(xb3, yb3, arrow_rx2, arrow_ry2, 2, WHITE);


  for (int i = 0; i < num_att_lines; i++) {
    //base_vect*level_angle得到离姿态仪中心最近的姿态线角度
    draw_level_angle = base_vect*level_angle+(i - 4)*level_angle; 
    if (draw_level_angle < 0){
      color = BLACK;      
    } else{
      color = WHITE;
    }
    
    itoa(draw_level_angle, level_char, 10);        
    //判断长的0度地平线左边一半线段坐标是否在画布范围内
    if ((draw_level_angle == 0) && (xb1>1) && (xb1<x_limit) && (xb2>1) && (xb2<x_limit) && (yb1>1) && (yb1<y_limit) && (yb2>1)  && (yb2<y_limit)){
      spr.drawWideLine(xb1, yb1, xb2, yb2, 2, WHITE); //画一根长的地平线
      // double costriggle=((xb2+xb3)/2-xb2)/sqrt((((xb2+xb3)/2-xb2)*((xb2+xb3)/2-xb2))+64);
      // spr.drawWideLine(xb2,yb2,(xb2+xb3)/2,(yb2+8*costriggle),2,GREEN);
    } else {
      int start_x1 = att_points[i][0];
      int start_y1 = att_points[i][1];
      int end_x1 = att_points[i][2];
      int end_y1 = att_points[i][3];
      
      // 判断是否为 5 度或 15 度对应的线
      if (abs(draw_level_angle) / 5 % 2 ==1 ) {
        start_x1 = (start_x1 + att_points[i][2]) / 2;
        start_y1 = (start_y1 + att_points[i][3]) / 2;
      }
      
      if((start_x1>1) && (start_x1<x_limit) && (start_y1>1)  && (start_y1<y_limit) && (end_x1>1) && (end_x1<x_limit)  && (end_y1>1) && (end_y1<y_limit) ) {
        spr.drawWideLine(start_x1, start_y1, end_x1, end_y1, 2, color);
      }       
    } 

    //判断长的0度地平线右边一半线段坐标是否在画布范围内
    if ((draw_level_angle == 0) && (xb3>1) && (xb4>1) && (xb3<x_limit) && (xb4<x_limit) && (yb3>1) && (yb4>1) && (yb3<y_limit) && (yb4<y_limit) ){
      spr.drawWideLine(xb3, yb3, xb4, yb4, 2, WHITE); //画一根长的地平线
      // spr.drawWideLine(xb3,yb3,(xb2+xb3)/2+8*cosa,(yb3+8)*sina,2,GREEN);
    } else {
      int start_x2 = att_points[i][4];
      int start_y2 = att_points[i][5];
      int end_x2 = att_points[i][6];
      int end_y2 = att_points[i][7];
      
      // 判断是否为 5 度或 15 度对应的线
      if (abs(draw_level_angle) / 5 % 2 == 1) {
        end_x2 = (end_x2 + att_points[i][4]) / 2;
        end_y2 = (end_y2 + att_points[i][5]) / 2;
      }
      
      if((start_x2>1) && (start_x2<x_limit) && (start_y2>1)  && (start_y2<y_limit) && (end_x2>1) && (end_x2<x_limit)  && (end_y2>1) && (end_y2<y_limit) )  {          
        spr.drawWideLine(start_x2, start_y2, end_x2, end_y2, 2, color);
      }
    }  

    // 写上pitch度数
    if (draw_level_angle == 0 ){
      spr.setCursor((att_points[i][0]+att_points[i][6])/2 - 5, (att_points[i][1]+att_points[i][7])/2 - 10);
      spr.setTextColor(color);
      spr.println(level_char);    
    } else if (draw_level_angle > 0 ){
      int x_offset = 0;
      if(draw_level_angle == 5){  //pitch度数为5，向右偏移5像素
        x_offset = 5;
      }
      spr.setCursor((att_points[i][0]+att_points[i][6])/2 - 10+x_offset, (att_points[i][1]+att_points[i][7])/2 - 10);
      spr.setTextColor(color);
      spr.println(level_char);    
    } else {
      spr.setCursor((att_points[i][0]+att_points[i][6])/2 - 15, (att_points[i][1]+att_points[i][7])/2 - 10);
      spr.setTextColor(color);
      spr.println(level_char);    
    }
  } 

  spr.unloadFont();
  spr.pushSprite(65, 0);
}

void draw_compass(){
  int x_start=0,y_start=0;
  // 绘制动态表盘（刻度随yaw旋转）
  float current_rotation = heading; // 根据传感器方向调整符号
  for(int angle = 0; angle <= 360; angle += 30) {
    float rad = radians(angle + current_rotation); // 动态叠加旋转角度
    int x1 = x_center + (50 - 8) * sin(rad);
    int y1 = y_center - (50 - 8) * cos(rad);
    int x2 = x1 + (abs(angle%30)==0 ? 8 : 3) * sin(rad);
    int y2 = y1 - (abs(angle%30)==0 ? 8 : 3) * cos(rad);
    spr_compass.drawLine(x1, y1, x2, y2, TFT_WHITE);
  }

  // 绘制静态指针（固定方向）
  float pointer_rad = radians(0); // 固定为0度
  int tip_x = x_center + (50-10) * sin(pointer_rad);
  int tip_y = y_center - (50-10) * cos(pointer_rad);
  spr_compass.drawWedgeLine(x_center, y_center , tip_x, tip_y, 2, 1, TFT_RED);
  spr_compass.fillCircle(x_center, y_center, 3, TFT_RED);
  float rad=radians(heading);
  if (abs(heading%90) == 0) {
    spr_compass.setTextColor(TFT_WHITE, TFT_BLACK);
    spr_compass.setTextDatum(MC_DATUM);
    String dir_label;
    switch ((heading + int(current_rotation)) % 360) { // 动态计算标签方向
      case 0: dir_label = "N"; break;
      case 90: dir_label = "E"; break;
      case 180: dir_label = "S"; break;
      case 270: dir_label = "W"; break;
    }
    spr_compass.drawString(dir_label, 
          x_center + (25+15)*sin(rad),
          y_center - (25+15)*cos(rad));
  }
  spr_compass.pushSprite(0, 0);
}



float calculateModelDistance(const mavlink_global_position_int_t& current,
  const mavlink_global_position_int_t& home) {
// 单位转换：degE7 → 弧度
constexpr float DEG_E7_TO_RAD = 1e-7f * M_PI / 180.0f;
const float lat1 = static_cast<float>(home.lat) * DEG_E7_TO_RAD;
const float lon1 = static_cast<float>(home.lon) * DEG_E7_TO_RAD;
const float lat2 = static_cast<float>(current.lat) * DEG_E7_TO_RAD;
const float lon2 = static_cast<float>(current.lon) * DEG_E7_TO_RAD;

// 有效性检查（纬度范围±85度，避免极区计算误差）
if (std::abs(lat1) > 1.483f || std::abs(lat2) > 1.483f) { // 85° ≈ 1.483 radians
return NAN;
}

// Haversine公式实现
const float dlat = lat2 - lat1;
const float dlon = lon2 - lon1;
const float a = std::sin(dlat/2) * std::sin(dlat/2) +
std::cos(lat1) * std::cos(lat2) *
std::sin(dlon/2) * std::sin(dlon/2);
const float c = 2 * std::atan2(std::sqrt(a), std::sqrt(1-a));

// 地球平均半径（单位：米）
constexpr float EARTH_RADIUS = 6371000.0f; // 精确值可优化为WGS84椭球模型
return EARTH_RADIUS * c;
}

void update_distance(uint32_t current_time, float current_speed) {
  if (last_time_stamp != 0) {  // 跳过首次无效计算
      
          double dt = (current_time - last_time_stamp) / 1000.0;  // 转换为秒
          float avg_speed = (current_speed + last_groundspeed) / 2.0f;
          total_distance += avg_speed * dt;
      
  }
  last_time_stamp = current_time;
  last_groundspeed = current_speed;
}

// MAVLink消息处理函数
void handle_vfr_hud(const mavlink_message_t& msg) {
  mavlink_vfr_hud_t vfr_hud;
  mavlink_msg_vfr_hud_decode(&msg, &vfr_hud);
  update_distance(millis(), vfr_hud.groundspeed);  // 使用Arduino时间戳
}

void get_mavlink_data(){
  while (Serial.available() > 0) {
    // Serial.read前不能有任何干扰串口信息，比如往串口发送信息的Serial.print,否则会干扰串口数据。
    uint8_t result = Serial.read(); // 读取串口信息，假设此时串口已经开始发送信息///
    msgReceived = mavlink_parse_char(MAVLINK_COMM_1, result, &msg, &status);
    if (msgReceived) {
      // Serial.print("Msgid: ");  Serial.println(msg.msgid);
      switch (msg.msgid) {  
        case MAVLINK_MSG_ID_HEARTBEAT:  // #0
        {
          mavlink_heartbeat_t heart;
          mavlink_msg_heartbeat_decode(&msg, &heart);
          base_mode = heart.base_mode;
          custom_mode = heart.custom_mode;
          flight_mode = custom_mode + base_mode;
          system_status = heart.system_status;
          switch(system_status){
            case 3:
              system_status_str = "待命";
              break;
            case 4:
              system_status_str = "运行中";
              break;
            case 5:
              system_status_str = "锁定";
              break;
          }  
          switch (custom_mode){
            case 11:
              flight_mode_str = "返航";
              break;
            case 5:
              flight_mode_str = "巡航";
              break;
            case 0:
              flight_mode_str = "手动";
              break;
            case 4:
              flight_mode_str = "战斗";
              break;
            case 13:
              flight_mode_str = "弹射起飞";
              break;
          }

          break;           
        }

        // case MAVLINK_MSG_ID_AVAILABLE_MODES:
        // {
        //   mavlink_available_modes_t available_modes;
        //   mavlink_msg_heartbeat_decode(&msg, &available_modes);
        //   mode_index = available_modes.mode_index;
        // }

        case MAVLINK_MSG_ID_SYS_STATUS:  // #1
          {
            mavlink_sys_status_t sys_status;
            mavlink_msg_sys_status_decode(&msg, &sys_status);
            voltage_battery = sys_status.voltage_battery;
            current_battery = sys_status.current_battery;
            break;          
          }
        
        case MAVLINK_MSG_ID_DISTANCE_SENSOR:// 132
        {
          mavlink_distance_sensor_t rangefinder;
          mavlink_msg_distance_sensor_decode(&msg, &rangefinder);
          current_distance = rangefinder.current_distance;
          rangefinder_distance = current_distance/100;
          break; 
        }

        case MAVLINK_MSG_ID_BATTERY_STATUS:  // #147
          {
            mavlink_battery_status_t batt_status;
            mavlink_msg_battery_status_decode(&msg, &batt_status);
            current_consumed = batt_status.current_consumed;
            break;          
          }
        
        case MAVLINK_MSG_ID_GPS_RAW_INT: // #24  
          {
            mavlink_gps_raw_int_t  gps_raw_int;
            mavlink_msg_gps_raw_int_decode(&msg, &gps_raw_int);   
            satellites_visible = gps_raw_int.satellites_visible; 
            cog = gps_raw_int.cog;
            break;                  
          }        
              
        case MAVLINK_MSG_ID_ATTITUDE:  // #30
          {
            mavlink_attitude_t attitude;
            mavlink_msg_attitude_decode(&msg, &attitude);
            pitch = attitude.pitch;
            roll = -attitude.roll;
            yaw = attitude.yaw;   
            break;                     
          }
        
        
        case MAVLINK_MSG_ID_GLOBAL_POSITION_INT:  // #33
          {
            mavlink_global_position_int_t global_position_int;
            mavlink_msg_global_position_int_decode(&msg, &global_position_int);
            // 记录起飞点（通常在解锁时设置一次）
            if (first_arm) {
              home_lat = global_position_int.lat;
              home_lon = global_position_int.lon;
              first_arm=false;
            }
            // 计算实时距离
            mavlink_global_position_int_t current_pos = {
              .lat = global_position_int.lat,  
              .lon = global_position_int.lon  
            };
          
            mavlink_global_position_int_t home_pos = {
                .lat = home_lat,  
                .lon = home_lon  
            };
            
            distance = calculateModelDistance(current_pos, home_pos);
          
            time_boot_ms = global_position_int.time_boot_ms;
            relative_alt = global_position_int.relative_alt/1000;
            hdg = global_position_int.hdg; /*< [cdeg] Vehicle heading (yaw angle), 0.0..359.99 degrees. If unknown, set to: UINT16_MAX*/
            break; 
          }
          
        case MAVLINK_MSG_ID_REQUEST_DATA_STREAM: //66
          {
            mavlink_request_data_stream_t r_stream_t;
            mavlink_msg_request_data_stream_decode (&msg, &r_stream_t);
            req_message_rate =  r_stream_t.req_message_rate;
            break; 
          } 
                 
        case MAVLINK_MSG_ID_DATA_STREAM: // 67
          { 
            mavlink_data_stream_t stream_t;
            mavlink_msg_data_stream_decode (&msg, &stream_t);
            message_rate = stream_t.message_rate;
            stream_id = stream_t.stream_id;       
            on_off = stream_t.on_off;
            break; 
          }
        
        
        
        case MAVLINK_MSG_ID_VFR_HUD:  // #74
          {
            mavlink_vfr_hud_t vfr_hud;
            mavlink_msg_vfr_hud_decode(&msg, &vfr_hud);
            airspeed = vfr_hud.airspeed; /*< [m/s] Vehicle speed. */
            groundspeed = vfr_hud.groundspeed; /*< [m/s] Current ground speed.*/
            alt = vfr_hud.alt; /*< [m] Current altitude (MSL).*/            
            climb = vfr_hud.climb; /*< [m/s] Current climb rate.*/
            heading = vfr_hud.heading; //[deg] Current heading in compass units (0-360, 0=north).
            throttle = vfr_hud.throttle; /*< [%] Current throttle setting (0 to 100).*/
            handle_vfr_hud(msg); // 触发路程计算
            break; 
          }
        
        case MAVLINK_MSG_ID_RC_CHANNELS: //needs mavlink V2 to get this data
          {
            mavlink_rc_channels_t rc_channels;
            mavlink_msg_rc_channels_decode(&msg, &rc_channels);
            	// chan1_raw = rc_channels_raw.chan1_raw;
              // chan2_raw = rc_channels_raw.chan2_raw;
              // chan3_raw = rc_channels_raw.chan3_raw;
              // chan4_raw = rc_channels_raw.chan4_raw;
              // chan5_raw = rc_channels_raw.chan5_raw;
            chan6_raw = rc_channels.chan6_raw;
              // chan7_raw = rc_channels_raw.chan7_raw;
              // chan8_raw = rc_channels_raw.chan8_raw;
            chan10_raw = rc_channels.chan10_raw;
            chan11_raw = rc_channels.chan11_raw;
              // rc_channels_raw.port = port;
            // rssi = rc_channels_raw.rssi;
            break; 
          }
        // case MAVLINK_MSG_ID_RC_CHANNELS_RAW: //needs mavlink V1 to get this data
        // {
        //   mavlink_rc_channels_raw_t rc_channels_raw;
        //   mavlink_msg_rc_channels_raw_decode(&msg, &rc_channels_raw);
        //   // chan1_raw = rc_channels_raw.chan1_raw;
        //   // chan2_raw = rc_channels_raw.chan2_raw;
        //   // chan3_raw = rc_channels_raw.chan3_raw;
        //   // chan4_raw = rc_channels_raw.chan4_raw;
        //   // chan5_raw = rc_channels_raw.chan5_raw;
        //   chan6_raw = rc_channels_raw.chan6_raw;
        //   // chan7_raw = rc_channels_raw.chan7_raw;
        //   // chan8_raw = rc_channels_raw.chan8_raw;
        //   // rc_channels_raw.port = port;
        //   // rssi = rc_channels_raw.rssi;
        //   break; 
        // }
        case MAVLINK_MSG_ID_RADIO_STATUS:
          {
            mavlink_radio_status_t radio_status;
            mavlink_msg_radio_status_decode(&msg, &radio_status);
            rssi = radio_status.rssi;
            break; 
          }

                  
      }//switch (message.msgid)         
    }//if (msgReceived)    
  }//while (Serial.available() > 0)
}

void push_main_screen()
{
  get_mavlink_data();
  cal_att_cord();
  if(!change_flag)
  draw_new_attitude();
  // spr.pushImage(0, 0, 165, 280, (uint16_t*)spr.getPointer());//原版
  // spr.pushImage(0, 0, 80, 280, (uint16_t*)spr.getPointer());
  // st7789_push_colors(0, 0, 165, 280, (uint16_t*)spr.getPointer());
  now0 = millis();
    if (now0 - Millis0 >= 500){
      if(!change_flag)
      {
        draw_meters();
        draw_meters_2();
        draw_under();
      }
      else
      draw_compass();
      // spr_meters.pushImage(166+4, 0, 80, 280, (uint16_t*)spr_meters.getPointer());
      // spr_meters_2.pushImage(536-80, 0, 80, 280, (uint16_t*)spr_meters_2.getPointer());
      // st7789_push_colors(166+4, 0, 80, 280, (uint16_t*)spr_meters.getPointer());
      // st7789_push_colors(536-80, 0, 80, 280, (uint16_t*)spr_meters_2.getPointer());
    }      

    if (now0 - Millis0 >= 500){
      // draw_mid();
      // spr_mid.pushImage(166+4+80, 0, 200, 280, (uint16_t*)spr_mid.getPointer());  
      // st7789_push_colors(166+4+80, 0, 200, 280, (uint16_t*)spr_mid.getPointer());  
    }      
}
//绘制罗盘

// 上方小屏数据 96X16
void push_u8g1()
{
  u8g1.clearBuffer();
  u8g1.setFont(u8g2_font_wqy16_t_gb2312b);
  u8g1.setCursor(10, 14);   
  u8g1.print("动力: ");
  u8g1.print(double(current_battery)/100, 1);
  u8g1.sendBuffer();
}

//下方小屏数据 72X40
void push_u8g2()
{
  u8g2.clearBuffer();
  u8g2.setCursor(0, 16); 
  u8g2.print("飞行模式:");
  u8g2.setCursor(0, 36);
  u8g2.print(flight_mode_str);
  u8g2.sendBuffer();
}


void adjust_brightness_main(){
  
  brightness_new_main = (chan6_raw-1000)*255/1000; //only the integral part is stored and the fractional part of the number is lost
  if (abs(brightness_new_main - brightness_old_main) > 20){
    analogWrite(TFT_BL, brightness_new_main);
    brightness_old_main = brightness_new_main;
  }
}

void adjust_brightness_small(){
  brightness_new = (chan6_raw-1000)*255/1000; //only the integral part is stored and the fractional part of the number is lost
  if (abs(brightness_new - brightness_old) > 8){
    u8g1.setContrast(brightness_new);
    u8g2.setContrast(brightness_new/4);
    brightness_old = brightness_new;
  }
}
// task1code, 显示主显示屏信息
void Task1code( void * pvParameters ){
  //task 的时间不能太短，否则会触发task watchdog,导致芯片重启
  // Serial.print("Task1 begines on core ");
  // Serial.println(xPortGetCoreID());
  for(;;){
    vTaskDelay(1); //disable task watchdog
    // Serial.print("Task1 running on core ");
    // Serial.println(xPortGetCoreID());

    push_main_screen();
    // spr_meters.pushSprite(0, 0);    // 左侧仪表（底层）
    // // spr_meters_2.pushSprite(160, 0);// 右侧仪表（底层）（竖版）
    // spr_meters_2.pushSprite(200, 0);// 右侧仪表（底层）（横版）
  
    // 再推送中层主仪表
              // 覆盖部分两侧仪表
    adjust_brightness_main();

    // 计算主屏幕fps0
    frameCount0 ++;
    now0 = millis();
    if (now0 - Millis0 >= 2000){
      fps0 = frameCount0/2;
      frameCount0 = 0;   
      Millis0 = now0;      
    }      
  } 
}

// Task2code: 显示1、2屏幕信息
void Task2code( void * pvParameters ){

  for(;;){
    vTaskDelay(1);
    // Serial.print("Task2 running on core ");
    // Serial.println(xPortGetCoreID());
    push_u8g1();
    adjust_brightness_small();

    now1 = millis();
    frameCount1 ++;
    if (now1 - Millis1st >= 1000){
      fps1 = frameCount1;
      frameCount1 = 0;   
      Millis1st = now1;

      push_u8g2();
    }
  }
}

void setup() {

  tft.init();
  tft.setRotation(1);  // 调整屏幕旋转方向
  tft.invertDisplay(true);  // 部分ST7789屏幕需要反转显示
  tft.fillScreen(WHITE);
  // pinMode(TFT_BL, OUTPUT);
  // digitalWrite(TFT_BL, TFT_BACKLIGHT_ON);
  u8g1.begin();
  u8g2.begin();
  u8g1.enableUTF8Print();
  u8g2.enableUTF8Print();

  u8g1.setFont(u8g2_font_wqy16_t_gb2312b);
  u8g2.setFont(u8g2_font_wqy16_t_gb2312b);

  Serial.begin(57600);

  // spr.createSprite(165, 280);  //attitude indicator
  spr_meters.createSprite(65, 180);
  spr_meters_2.createSprite(65, 180); //原80→75
  // spr_jet.createSprite(109, 182);
  // spr_mid.createSprite(200, 280);
  // spr_status.createSprite(109, 53);
  // spr_right_bar.createSprite(50, 180);
  // spr_left_bar.createSprite(50, 180);
  // spr_right_rec.createSprite(80, 40);
  // spr_left_rec.createSprite(80, 40);
  // spr_compass.createSprite(280,240);
  //   // 初始化时调整尺寸
  // spr.createSprite(80, 280);  //attitude indicator（竖版）
  spr.createSprite(150, 180);  //attitude indicator（横版）
  spr_under.createSprite(280, 60);  // 创建一个 280x60 的 Sprite
  // spr_mid.createSprite(240, 280);
  // spr_meters.createSprite(80, 280);    // 原80→75
  // spr_meters_2.createSprite(80, 280);
  // spr_status.createSprite(75, 40);     // 原109→75


xTaskCreatePinnedToCore(
                  Task1code,   /* Task function. */
                  "Task1",     /* name of task. */
                  10000,       /* Stack size of task */
                  NULL,        /* parameter of the task */
                  1,           /* priority of the task */
                  &Task1,      /* Task handle to keep track of created task */
                  0);          /* pin task to core 0 */     


  //create a task that will be executed in the Task2code() function, with priority 1 and executed on core 1
xTaskCreatePinnedToCore(
                    Task2code,   /* Task function. */
                    "Task2",     /* name of task. */
                    10000,       /* Stack size of task */
                    NULL,        /* parameter of the task */
                    1,           /* priority of the task */
                    &Task2,      /* Task handle to keep track of created task */
                    1);          /* pin task to core 1 */

}



void loop(void) {  

} // void loop
