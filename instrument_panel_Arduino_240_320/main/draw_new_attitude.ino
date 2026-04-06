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
    // spr.pushSprite(85, 0);
    spr.pushSprite(5, 0);
  }