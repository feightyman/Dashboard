

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
  