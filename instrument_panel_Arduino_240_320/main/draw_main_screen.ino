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
  spr_meters.setTextColor(WHITE, WHITE);
  int x_start = 0;
  int y_start = 0;


  
    // for (int j = 0; j < 3; j++){
    //   spr_meters.fillSmoothRoundRect(x_start,j*81+y_start, 78, 78, 6, GRAY, TFT_BLACK);
    // } 
  //spr_meters.fillSmoothRoundRect(x_start,y_start, 63, 48, 6, GRAY, TFT_BLACK);
  //spr_meters.fillSmoothRoundRect(x_start,51+y_start, 63, 128, 6, GRAY, TFT_BLACK); 
  spr_meters.fillSmoothRoundRect(x_start,y_start,85,180,0,TFT_BLACK, TFT_BLACK); 
  // spr_meters.fillCircle(30, 1*51+y_start+55+30, 15, TFT_DARKGREEN);  // 外圈底色
  // spr_meters.fillCircle(30, 1*51+y_start+55+30, 15-3, TFT_GREEN);    // 内圈高光  
  //起落架状态条
  spr_meters.loadFont(Final_Frontier_15);
  spr_meters.drawRect(x_start+10, 1*51+y_start+55+10 , 65, 15, GREEN);
  spr_meters.fillRect(x_start+11, 1*51+y_start+55+11, 63, 13,GRAY);
  spr_meters.setCursor(x_start+38,1*51+y_start+55+13);spr_meters.print("U");
  spr_meters.fillRect(x_start+10, 1*51+y_start+55+10, (chan11_raw-1000)*65/1000, 15, GREEN);
  //???状态条
  spr_meters.drawRect(x_start+10, 1*51+y_start+55+10+25 , 65, 15, TFT_YELLOW);
  spr_meters.fillRect(x_start+11, 1*51+y_start+55+11+25, 63, 13,GRAY);
  spr_meters.setCursor(x_start+38,1*51+y_start+55+13+25);spr_meters.print("A");
  spr_meters.fillRect(x_start+10, 1*51+y_start+55+10+25, (chan1_raw-1000)*65/1000, 15, TFT_YELLOW);

  spr_meters.unloadFont();
  draw_wifi(); // 绘制无线信号图标
  
  spr_meters.loadFont(Final_Frontier_15);
  distance=distance<=1000?1000:distance;
  spr_meters.setCursor(x_start,35);spr_meters.print("X:");spr_meters.print(distance,1);spr_meters.print("m");
  spr_meters.setCursor(x_start, 25+55); spr_meters.print(groundspeed*3.6, 1);spr_meters.print("km/h");
  // spr_meters.unloadFont();
  // spr_meters.loadFont(Final_Frontier_15);
  // spr_meters.setCursor(x_start+15, 25+70);spr_meters.print("km/h");//空速换算成km/h

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
  // spr_meters.pushSprite(0, 0);
  spr_meters.pushSprite(-80, 0);
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
  spr_meters_2.fillSmoothRoundRect(x_start,y_start,85,180,0,TFT_BLACK, TFT_BLACK);
  // spr_meters_2.fillCircle(30, 1*51+y_start+55+30, 15, TFT_DARKGREEN);  // 外圈底色
  // spr_meters_2.fillCircle(30, 1*51+y_start+55+30, 15-3, TFT_GREEN);    // 内圈高光  
  //起落架状态条
  spr_meters_2.loadFont(Final_Frontier_15);
  spr_meters_2.drawRect(x_start+10, 1*51+y_start+55+10 , 65, 15, TFT_RED);
  spr_meters_2.fillRect(x_start+11, 1*51+y_start+55+11, 63, 13,GRAY);
  spr_meters_2.setCursor(x_start+38,1*51+y_start+55+13);spr_meters_2.print("L");
  spr_meters_2.fillRect(x_start+10, 1*51+y_start+55+10, (chan10_raw-1000)*65/1000, 15, TFT_RED);
  //???状态条
  spr_meters_2.drawRect(x_start+10, 1*51+y_start+55+10+25 , 65, 15, TFT_BLUE);
  spr_meters_2.fillRect(x_start+11, 1*51+y_start+55+11+25, 63, 13,GRAY);
  spr_meters_2.setCursor(x_start+38,1*51+y_start+55+13+25);spr_meters_2.print("B");
  spr_meters_2.fillRect(x_start+10, 1*51+y_start+55+10+25, (chan2_raw-1000)*65/1000, 15, TFT_BLUE);
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
      spr_meters_2.pushImage(60, 0, 25, 25, salt_25_25); 
    }
  } else {
    // 卫星数量大于5，稳定显示
    spr_meters_2.pushImage(60, 0, 25, 25, salt_25_25); 
  }

  // 显示卫星信号强度参数
  /*spr_meters_2.loadFont(Final_Frontier_20);
  spr_meters_2.setTextColor(RED,RED);
  spr_meters_2.setCursor(20, 20); 
  spr_meters_2.print(satellites_visible);
  spr_meters_2.print("g"); // 添加单位
  spr_meters_2.unloadFont();*/
  
  spr_meters_2.loadFont(Final_Frontier_15);
  // spr_meters_2.setCursor(x_start+10, 0*81+y_start+30); spr_meters_2.print(double(current_battery)/100, 1); 
  // spr_meters_2.drawSmoothArc(x_start+38, 0*81+y_start+40, 36, 33, 60, 300, BLACK, BLACK, true);
  // spr_meters_2.drawSmoothArc(x_start+38, 0*81+y_start+40, 36, 33, 60, int32_t(60+current_battery*240/5000+1), YELLOW, BLACK, true); //最高电流按5000cA预估
  spr_meters_2.setTextColor(WHITE,WHITE);
  total_distance=total_distance<=1000?1000:total_distance;
  spr_meters_2.setCursor(x_start,35);spr_meters_2.print("S:");spr_meters_2.print(total_distance,1);spr_meters_2.print("m");
  if (alt > 100) {alt = 100;} 
  spr_meters_2.setCursor(x_start+15, 25+55);spr_meters_2.print(alt);spr_meters_2.print("m");
  // spr_meters_2.unloadFont();  
  // spr_meters_2.loadFont(Final_Frontier_15);
  // spr_meters_2.setCursor(x_start+30, 25+70);spr_meters_2.print("m"); 
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
  // spr_meters_2.pushSprite(235, 0);
  spr_meters_2.pushSprite(155, 0);
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
    spr_under.setCursor(x_start + 30, y_start + 20);  // 参数右移 40 像素
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

  // // 绘制动态指针
  //   float pointer_rad = radians(constrain(yaw/PI*180, -180, 180));
  //   int tip_x =  x_start + area_width / 2 + (25-10) * sin(pointer_rad);
  //   int tip_y = y_start + 30 - (25-10) * cos(pointer_rad);
  //   spr_under.drawWedgeLine(x_start + area_width / 2, y_start + 30, tip_x, tip_y, 2, 1, TFT_RED); // 渐变线宽
  //   spr_under.fillCircle(x_start + area_width / 2,y_start + 30,3,TFT_RED);
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
    // spr_under.pushSprite(20, 180);
    spr_under.pushSprite(-60, 180);
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
  