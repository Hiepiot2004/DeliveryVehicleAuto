// Khai báo chân cho Motor A
#include <NewPing.h>
#define TRIGGER_PIN  3
#define ECHO_PIN1    13
#define ECHO_PIN2    4
#define ECHO_PIN3    12

// Khoảng cách giới hạn (cm) để dừng lại nếu có vật cản
#define MAX_DISTANCE 15

// Tạo đối tượng NewPing cho từng cảm biến
NewPing sonar1(TRIGGER_PIN, ECHO_PIN1, MAX_DISTANCE);
NewPing sonar2(TRIGGER_PIN, ECHO_PIN2, MAX_DISTANCE);
NewPing sonar3(TRIGGER_PIN, ECHO_PIN3, MAX_DISTANCE);
const int ENA = 10;    // Chân PWM điều khiển tốc độ động cơ A
const int IN1 = 9;    // Chân điều khiển chiều quay
const int IN2 = 8;    // Chân điều khiển chiều quay

// Khai báo chân cho Motor B
const int ENB = 5;   // Chân PWM điều khiển tốc độ động cơ B
const int IN3 = 7;    // Chân điều khiển chiều quay
const int IN4 = 6;    // Chân điều khiển chiều quay
int base_speed = 80;   // Tốc độ trung bình
int max_speed = 160;   // Tốc độ cao nhất

int thresholds[8] = {500, 350, 400, 350, 300, 350, 350, 300};
int sensorState[8];
int sensorValues[8];

char data = 0;
int X = 0;  // điểm lấy hàng
int Y = 0; // điểm trả hàng

bool checkVatCan() {
    int distance1 = sonar1.ping_cm();
    int distance2 = sonar2.ping_cm();
    int distance3 = sonar3.ping_cm();

    // Nếu khoảng cách đo được < MAX_DISTANCE (10cm), có vật cản
    if ((distance1 > 0 && distance1 < MAX_DISTANCE) || 
        (distance2 > 0 && distance2 < MAX_DISTANCE) || 
        (distance3 > 0 && distance3 < MAX_DISTANCE)) {
        return true;  // Có vật cản
    }
    return false;  // Không có vật cản
}


void setup() {
  Serial.begin(9600); // Khởi động Serial Monitor
  pinMode(2, OUTPUT);
  digitalWrite(2, LOW);

  // Cấu hình các chân là đầu ra
  pinMode(ENA, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);

  pinMode(ENB, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  setMotorSpeed(0, 0);

}
void setMotorSpeed(int speedA, int speedB) {
    // Giới hạn tốc độ trong khoảng -255..255
    speedA = constrain(speedA, -255, 255);
    speedB = constrain(speedB, -255, 255);

    // Lấy giá trị tuyệt đối để đưa vào analogWrite (PWM)
    int pwmA = abs(speedA);
    int pwmB = abs(speedB);

    analogWrite(ENA, pwmA);
    analogWrite(ENB, pwmB);

    // Xác định chiều quay cho động cơ A
    if (speedA > 0) {
        // Tiến
        digitalWrite(IN1, HIGH);
        digitalWrite(IN2, LOW);
    } else if (speedA < 0) {
        // Lùi
        digitalWrite(IN1, LOW);
        digitalWrite(IN2, HIGH);
    } else {
        // Dừng
        digitalWrite(IN1, LOW);
        digitalWrite(IN2, LOW);
    }

    // Xác định chiều quay cho động cơ B
    if (speedB > 0) {
        // Tiến
        digitalWrite(IN3, HIGH);
        digitalWrite(IN4, LOW);
    } else if (speedB < 0) {
        // Lùi
        digitalWrite(IN3, LOW);
        digitalWrite(IN4, HIGH);
    } else {
        // Dừng
        digitalWrite(IN3, LOW);
        digitalWrite(IN4, LOW);
    }
}
void readSensor() {
  int sensorValues[8]; // Lưu giá trị analog từ cảm biến
    for (int i = 0; i < 8; i++) {
        sensorValues[i] = analogRead(A7 - i);
        sensorState[i] = (sensorValues[i] > thresholds[i]) ? 1 : 0;
    }
}
bool checkNgaTu(int sensorState[8]) {
    int countOnLine = 0;
    for (int i = 0; i < 8; i++) {
        if (sensorState[i] == 1) {
            countOnLine++;
        }
    }
    // Nếu có ít nhất 6 cảm biến thấy line => coi như ngã tư
    return (countOnLine >= 7);
}

// Hàm rẽ trái khi gặp ngã tư
void ReTrai() {
     if (checkVatCan()) {
        setMotorSpeed(-30,-30 ); // Dừng lại nếu có vật cản
        delay(200);
        digitalWrite(2, HIGH); // Bật đèn báo vật cản
        while (checkVatCan()) { // Chờ đến khi không còn vật cản
            delay(100);
        }
        digitalWrite(2, LOW); // Tắt đèn khi vật cản đã mất
    }
    // 1) Dừng ngắn để tránh vọt qua
    setMotorSpeed(0, 0);
    delay(200); // chờ 0.2 giây

    // 2) Rẽ trái (motor trái lùi, motor phải tiến)
    
    setMotorSpeed(-110, 110);
    delay(200);
    while(true){
      readSensor();
      if(sensorState[0] == 1 && sensorState[1] == 1){
        break;
      }
      delay(10);
    }
    // 3) Dừng hoặc quay lại dò line
    setMotorSpeed(0, 0);
    delay(200);
}
// Hàm rẽ phải khi gặp ngã tư
void RePhai() {
     if (checkVatCan()) {
        setMotorSpeed(-30, -30); // Dừng lại nếu có vật cản
        delay(200);
        digitalWrite(2, HIGH); // Bật đèn báo vật cản
        while (checkVatCan()) { // Chờ đến khi không còn vật cản
            delay(100);
        }
        digitalWrite(2, LOW); // Tắt đèn khi vật cản đã mất
    }
    // 1) Dừng ngắn để tránh vọt qua
    setMotorSpeed(0, 0);
    delay(200);

    // 2) Rẽ phải (motor trái tiến, motor phải lùi)

    setMotorSpeed(120, -120);
    delay(200);
    while(1){
      readSensor();
      if(sensorState[6] == 1 && sensorState[7] == 1){
        break;
      }
    }

    // 3) Dừng hoặc quay lại dò line
    setMotorSpeed(0, 0);
    delay(200);
}
void QuayDau() {
   if (checkVatCan()) {
        setMotorSpeed(-30, -30); // Dừng lại nếu có vật cản
        delay(200);
        digitalWrite(2, HIGH); // Bật đèn báo vật cản
        while (checkVatCan()) { // Chờ đến khi không còn vật cản
            delay(100);
        }
        digitalWrite(2, LOW); // Tắt đèn khi vật cản đã mất
    }
    // 1) Cho động cơ trái lùi, động cơ phải tiến => xe xoay tại chỗ
    setMotorSpeed(-200,200);
    delay(100);
    setMotorSpeed(-120, 120);
    delay(200);
    // Quay liên tục cho đến khi cảm biến 0 và 1 phát hiện line
    while (true) {
        readSensor();  // Cập nhật sensorState từ cảm biến line
        if (sensorState[0] == 1) {
            break;  // Nếu cảm biến 0 và 1 đều thấy line, thoát vòng lặp
        }
        delay(10);  // Nghỉ một chút để tránh làm quá tải CPU
    }
    // 3) Dừng xe
    setMotorSpeed(0, 0);
    delay(200);
}


int intersectionCount = 0; // Biến toàn cục, reset khi bắt đầu hành trình mới

int step = 0;
unsigned long stateTimer = 0; // Dùng để chờ 3 giây, chờ quay đầu, v.v.
bool quatrinh = false;

void dichuyen(int x,int y, bool &quatrinh){
  if(x==1 && y==4){
    switch (step) {

      case 0:
      if (checkNgaTu(sensorState)) {
        setMotorSpeed(0,0);
        delay(200);
        ReTrai(); // Tới node1 => Rẽ trái để vào điểm 1
        step = 1; // Chuyển sang bước tiếp theo: GO TO DIEM1
      } else {// Chưa tới node1 => tiếp tục dò line
        line_detect();
      }
      break;

      case 1:
      // GO TO DIEM1
      // Tiếp tục dò line đến khi gặp ngã tư => điểm 1
      if (checkNgaTu(sensorState)) {
        // Đã đến điểm 1 => dừng 3s
        setMotorSpeed(0, 0);
        stateTimer = millis();   // Ghi thời gian bắt đầu chờ
        step = 2;               // Sang bước WAIT_DIEM1
      } else {
        line_detect();
      }
      break;

      case 2:
      // WAIT_DIEM1 (3 giây)
      if (millis() - stateTimer >= 3000) {
        // Hết 3 giây => quay đầu (U-turn)
        QuayDau();
        // Sang bước tiếp theo: BACK_NODE1
        step = 3;
      }
      // Xe vẫn dừng
      break;

      case 3:
      // BACK_NODE1
      // Dò line quay lại node1
      // Khi gặp ngã tư => node1
      if (checkNgaTu(sensorState)) {
        setMotorSpeed(0,0);
        delay(200);
        ReTrai();
        step = 4;  // GO TO NODE2
      } else {
        line_detect();
      }
      break;

      case 4:
      // GO TO NODE2
      // Tiếp tục dò line đến khi gặp ngã tư => node2
      if (checkNgaTu(sensorState)) {
        setMotorSpeed(0,0);
        delay(200);
        // Ở node2 => rẽ phải để đi sang điểm 4
        RePhai();
        step = 5;  // GO TO DIEM4
      } else {
        line_detect();
      }
      break;

      case 5:
      // GO TO DIEM4
      // Dò line đến khi gặp ngã tư => điểm 4
      if (checkNgaTu(sensorState)) {
        // Đã đến điểm 4 => dừng 3s
        setMotorSpeed(0, 0);
        stateTimer = millis();
        step = 6;  // WAIT_DIEM4
      } else {
        line_detect();
      }
      break;

      case 6:
      // WAIT_DIEM4 (3 giây)
      if (millis() - stateTimer >= 3000) {
        // Hết 3 giây => quay đầu
        QuayDau();
        step = 7; // BACK_NODE2
      }
      // Xe vẫn dừng
      break;

      case 7:
      // BACK_NODE2
      // Dò line quay lại node2
      if (checkNgaTu(sensorState)) {
        setMotorSpeed(0,0);
        delay(200);
        // Tới node2 => rẽ trái để đi thẳng qua node1 về Start
        ReTrai();
        step = 8; // GO THRU NODE1 -> START
      } else {
        line_detect();
      }
      break;

      case 8:
      // GO THRU NODE1 -> START
      // Dò line, lần đầu gặp ngã tư => node1 (đi thẳng), 
      // lần sau gặp ngã tư => Start
      if (checkNgaTu(sensorState)) {
        // Có thể đếm 2 lần intersection 
        // hoặc đơn giản: assume node1 + Start là 2 intersections
        intersectionCount++;

        if (intersectionCount == 1) {
          // Vừa qua node1 => đi thẳng (không rẽ)
          // Giả sử ta "nhích" thẳng: 
          setMotorSpeed(80, 80);
          delay(300); // Chạy qua node1 chút
        } else if (intersectionCount == 2) {
          // Đã đến Start => dừng hẳn
          setMotorSpeed(0, 0);
          delay(200);
          step = 9; // DONE
          intersectionCount = 0;
        }
      } else {
        line_detect();
      }
      break;

      case 9:
        QuayDau();
        step = 10;
      break;
      case 10:
      quatrinh = false;
      break;
  }
  }
  if(x==1 && y==3){
    switch (step) {

      case 0:
      if (checkNgaTu(sensorState)) {
        setMotorSpeed(0,0);
        delay(200);
        ReTrai();
        step = 1;
      } else {
        line_detect();
      }
      break;

      case 1:
      if (checkNgaTu(sensorState)) {
        setMotorSpeed(0, 0);
        stateTimer = millis();   
        step = 2;              
      } else {
        line_detect();
      }
      break;

      case 2:
      if (millis() - stateTimer >= 3000) {
        QuayDau();
        step = 3;
      }
      break;
      case 3:
      if (checkNgaTu(sensorState)) {
        setMotorSpeed(0,0);
        delay(200);
        ReTrai();
        step = 4;  // GO TO NODE2
      } else {
        line_detect();
      }
      break;
      case 4:
      if (checkNgaTu(sensorState)) {
        setMotorSpeed(0,0);
        delay(200);
        ReTrai();
        step = 5;  // GO TO DIEM3
      } else {
        line_detect();
      }
      break;

      case 5:
      if (checkNgaTu(sensorState)) {
        setMotorSpeed(0, 0);
        stateTimer = millis();
        step = 6;  // WAIT_DIEM4
      } else {
        line_detect();
      }
      break;
      case 6:
      if (millis() - stateTimer >= 3000) {
        QuayDau();
        step = 7; // BACK_NODE2
      }
      break;

      case 7:
      // BACK_NODE2
      // Dò line quay lại node2
      if (checkNgaTu(sensorState)) {
        setMotorSpeed(0,0);
        delay(200);
        // Tới node2 => rẽ trái để đi thẳng qua node1 về Start
        RePhai();
        step = 8; // GO THRU NODE1 -> START
      } else {
        line_detect();
      }
      break;

      case 8:
      if (checkNgaTu(sensorState)) {
        intersectionCount++;

        if (intersectionCount == 1) {
          setMotorSpeed(80, 80);
          delay(300); // Chạy qua node1 chút
        } else if (intersectionCount == 2) {
          // Đã đến Start => dừng hẳn
          setMotorSpeed(0, 0);
          delay(200);
          step = 9; // DONE
          intersectionCount = 0;
        }
      } else {
        line_detect();
      }
      break;
      case 9:
        QuayDau();
        step = 10;
      break;
      case 10:
      quatrinh=false;
      break;
  }
  }
  if(x==1 && y==2){
    switch (step) {

      case 0:
      if (checkNgaTu(sensorState)) {
        setMotorSpeed(0,0);
        delay(200);
        ReTrai();
        step = 1;
      } else {
        line_detect();
      }
      break;

      case 1:
      if (checkNgaTu(sensorState)) {
        setMotorSpeed(0, 0);
        stateTimer = millis();   
        step = 2;              
      } else {
        line_detect();
      }
      break;

      case 2:
      if (millis() - stateTimer >= 3000) {
        QuayDau();
        step = 3;
      }
      break;
      case 3:
        if (checkNgaTu(sensorState)) {
        intersectionCount++;

        if (intersectionCount == 1) {
          setMotorSpeed(80, 80);
          delay(300); // Chạy qua node1 chút
        } else if (intersectionCount == 2) {
          // Đã đến Start => dừng hẳn
          setMotorSpeed(0, 0);
          stateTimer = millis();
          step = 4; // DONE
          intersectionCount = 0;
        }
      } else {
        line_detect();
      }
      break;
      case 4:
      if(millis() - stateTimer >= 3000) {
        QuayDau();
        step = 5;
      } 
      break;

      case 5:
       if (checkNgaTu(sensorState)) {
        setMotorSpeed(0,0);
        delay(200);
        // Tới node2 => rẽ trái để đi thẳng qua node1 về Start
        ReTrai();
        step = 6; // GO THRU NODE1 -> START
      } else {
        line_detect();
      }
      break;
      case 6:
       if (checkNgaTu(sensorState)) {
        setMotorSpeed(0,0);
        delay(300);
        QuayDau();
        step=7;
      }
      else {
        line_detect();
      }
      break;

      case 7:
      quatrinh = false;
      break;
  }
  }
  if(x==2 && y==4){
    switch (step) {

      case 0:
      if (checkNgaTu(sensorState)) {
         setMotorSpeed(0,0);
          delay(200);
        RePhai();
        step = 1;
      } else {
        line_detect();
      }
      break;

      case 1:
      if (checkNgaTu(sensorState)) {
        setMotorSpeed(0, 0);
        stateTimer = millis();   
        step = 2;              
      } else {
        line_detect();
      }
      break;

      case 2:
      if (millis() - stateTimer >= 3000) {
        QuayDau();
        step = 3;
      }
      break;
      case 3:
      if (checkNgaTu(sensorState)) {
         setMotorSpeed(0,0);
          delay(200);
        RePhai();
        step = 4;  // GO TO NODE2
      } else {
        line_detect();
      }
      break;
      case 4:
      if (checkNgaTu(sensorState)) {
         setMotorSpeed(0,0);
          delay(200);
        RePhai();
        step = 5;  // GO TO DIEM3
      } else {
        line_detect();
      }
      break;

      case 5:
      if (checkNgaTu(sensorState)) {
        setMotorSpeed(0, 0);
        stateTimer = millis();
        step = 6;  // WAIT_DIEM4
      } else {
        line_detect();
      }
      break;
      case 6:
      if (millis() - stateTimer >= 3000) {
        QuayDau();
        step = 7; // BACK_NODE2
      }
      break;

      case 7:
      // BACK_NODE2
      // Dò line quay lại node2
      if (checkNgaTu(sensorState)) {
         setMotorSpeed(0,0);
          delay(200);
        // Tới node2 => rẽ trái để đi thẳng qua node1 về Start
        ReTrai();
        step = 8; // GO THRU NODE1 -> START
      } else {
        line_detect();
      }
      break;

      case 8:
      if (checkNgaTu(sensorState)) {
        intersectionCount++;

        if (intersectionCount == 1) {
          setMotorSpeed(80, 80);
          delay(300); // Chạy qua node1 chút
        } else if (intersectionCount == 2) {
          // Đã đến Start => dừng hẳn
          setMotorSpeed(0, 0);
          delay(200);
          step = 9; // DONE
          intersectionCount = 0;
        }
      } else {
        line_detect();
      }
      break;
      case 9:
        QuayDau();
        step = 10;
      break;
      case 10:
      quatrinh = false;
      break;
  }
  }
  if(x==2 && y==3){
    switch (step) {

      case 0:
      if (checkNgaTu(sensorState)) {
         setMotorSpeed(0,0);
          delay(200);
        RePhai();
        step = 1;
      } else {
        line_detect();
      }
      break;

      case 1:
      if (checkNgaTu(sensorState)) {
        setMotorSpeed(0, 0);
        stateTimer = millis();   
        step = 2;              
      } else {
        line_detect();
      }
      break;

      case 2:
      if (millis() - stateTimer >= 3000) {
        QuayDau();
        step = 3;
      }
      break;
      case 3:
      if (checkNgaTu(sensorState)) {
         setMotorSpeed(0,0);
          delay(200);
        RePhai();
        step = 4;  // GO TO NODE2
      } else {
        line_detect();
      }
      break;
      case 4:
      if (checkNgaTu(sensorState)) {
         setMotorSpeed(0,0);
          delay(200);
        ReTrai();
        step = 5;  // GO TO DIEM3
      } else {
        line_detect();
      }
      break;

      case 5:
      if (checkNgaTu(sensorState)) {
        setMotorSpeed(0, 0);
        stateTimer = millis();
        step = 6;  // WAIT_DIEM4
      } else {
        line_detect();
      }
      break;
      case 6:
      if (millis() - stateTimer >= 3000) {
        QuayDau();
        step = 7; // BACK_NODE2
      }
      break;

      case 7:
      // BACK_NODE2
      // Dò line quay lại node2
      if (checkNgaTu(sensorState)) {
        // Tới node2 => rẽ trái để đi thẳng qua node1 về Start
        RePhai();
        step = 8; // GO THRU NODE1 -> START
      } else {
        line_detect();
      }
      break;

      case 8:
      if (checkNgaTu(sensorState)) {
        intersectionCount++;

        if (intersectionCount == 1) {
          setMotorSpeed(80, 80);
          delay(300); // Chạy qua node1 chút
        } else if (intersectionCount == 2) {
          // Đã đến Start => dừng hẳn
          setMotorSpeed(0, 0);
          delay(200);
          step = 9; // DONE
          intersectionCount = 0;
        }
      } else {
        line_detect();
      }
      break;
      case 9:
        QuayDau();
        step = 10;
      break;
      case 10:
      quatrinh = false;
      break;
  }
  }
  if(x==2 && y==1){
    switch (step) {

      case 0:
      if (checkNgaTu(sensorState)) {
         setMotorSpeed(0,0);
          delay(200);
        RePhai();
        step = 1;
      } else {
        line_detect();
      }
      break;

      case 1:
      if (checkNgaTu(sensorState)) {
        setMotorSpeed(0, 0);
        stateTimer = millis();   
        step = 2;              
      } else {
        line_detect();
      }
      break;

      case 2:
      if (millis() - stateTimer >= 3000) {
        QuayDau();
        step = 3;
      }
      break;
      case 3:
        if (checkNgaTu(sensorState)) {
        intersectionCount++;

        if (intersectionCount == 1) {
          setMotorSpeed(80, 80);
          delay(300); // Chạy qua node1 chút
        } else if (intersectionCount == 2) {
          // Đã đến Start => dừng hẳn
          setMotorSpeed(0, 0);
          stateTimer = millis();
          step = 4; // DONE
          intersectionCount = 0;
        }
      } else {
        line_detect();
      }
      break;
      case 4:
      if(millis() - stateTimer >= 3000) {
        QuayDau();
        step = 5;
      } 
      break;

      case 5:
       if (checkNgaTu(sensorState)) {
         setMotorSpeed(0,0);
          delay(200);
        // Tới node2 => rẽ trái để đi thẳng qua node1 về Start
        RePhai();
        step = 6; // GO THRU NODE1 -> START
      } else {
        line_detect();
      }
      break;
      case 6:
       if (checkNgaTu(sensorState)) {
        setMotorSpeed(0,0);
        delay(300);
        QuayDau();
        step=7;
      }
      else {
        line_detect();
      }
      break;

      case 7:
       quatrinh = false;
      break;
  }
  }
  if(x==3 && y==4){
    switch (step) {

      case 0:
      if (checkNgaTu(sensorState)) {
        intersectionCount++;

        if (intersectionCount == 1) {
          setMotorSpeed(80, 80);
          delay(300); // Chạy qua node1 chút
        } else if (intersectionCount == 2) {
          // Đã đến Start => dừng hẳn
          setMotorSpeed(0, 0);
          delay(200);
          ReTrai();
          step = 1; // DONE
          intersectionCount = 0;
        }
      } else {
        line_detect();
      }
      break;

      case 1:
      if (checkNgaTu(sensorState)) {
        setMotorSpeed(0, 0);
        stateTimer = millis();   
        step = 2;              
      } else {
        line_detect();
      }
      break;

      case 2:
      if (millis() - stateTimer >= 3000) {
        QuayDau();
        step = 3;
      }
      break;
      case 3:
        if (checkNgaTu(sensorState)) {
        intersectionCount++;

        if (intersectionCount == 1) {
          setMotorSpeed(80, 80);
          delay(300); // Chạy qua node1 chút
        } else if (intersectionCount == 2) {
          // Đã đến Start => dừng hẳn
          setMotorSpeed(0, 0);
          stateTimer = millis();
          step = 4; // DONE
          intersectionCount = 0;
        }
      } else {
        line_detect();
      }
      break;
      case 4:
      if(millis() - stateTimer >= 3000) {
        QuayDau();
        step = 5;
      } 
      break;

      case 5:
       if (checkNgaTu(sensorState)) {
         setMotorSpeed(0,0);
          delay(200);
        ReTrai();
        step = 6; // GO THRU NODE1 -> START
      } else {
        line_detect();
      }
      break;
      case 6:
       if (checkNgaTu(sensorState)) {
        intersectionCount++;

        if (intersectionCount == 1) {
          setMotorSpeed(80, 80);
          delay(300); // Chạy qua node1 chút
        } else if (intersectionCount == 2) {
          // Đã đến Start => dừng hẳn
          setMotorSpeed(0, 0);
          delay(200);
          step = 7; // DONE
          intersectionCount = 0;
        }
      } else {
        line_detect();
      }
      break;
      case 7:
        QuayDau();
        step = 8;
      break;
      case 8:
      quatrinh = false;
      break;
  }
  }
  if(x==3 && y==2){
     switch (step) {

      case 0:
      if (checkNgaTu(sensorState)) {
        intersectionCount++;

        if (intersectionCount == 1) {
          setMotorSpeed(80, 80);
          delay(300); // Chạy qua node1 chút
        } else if (intersectionCount == 2) {
          // Đã đến Start => dừng hẳn
          setMotorSpeed(0, 0);
          delay(200);
          ReTrai();
          step = 1; // DONE
          intersectionCount = 0;
        }
      } else {
        line_detect();
      }
      break;

      case 1:
      if (checkNgaTu(sensorState)) {
        setMotorSpeed(0, 0);
        stateTimer = millis();   
        step = 2;              
      } else {
        line_detect();
      }
      break;

      case 2:
      if (millis() - stateTimer >= 3000) {
        QuayDau();
        step = 3;
      }
      break;
      case 3:
        if (checkNgaTu(sensorState)) {
           setMotorSpeed(0,0);
          delay(200);
        RePhai();
        step = 4; // GO THRU NODE1 -> START
      } else {
        line_detect();
      }
      break;
      case 4:
        if (checkNgaTu(sensorState)) {
           setMotorSpeed(0,0);
          delay(200);
        ReTrai();
        step = 5; // GO THRU NODE1 -> START
      } else {
        line_detect();
      }
      break;
      case 5:
      if (checkNgaTu(sensorState)) {
        setMotorSpeed(0, 0);
        stateTimer = millis();   
        step = 6;              
      } else {
        line_detect();
      }
      break;
      case 6:
      if(millis() - stateTimer >= 3000) {
        QuayDau();
        step = 7;
      } 
      break;

      case 7:
       if (checkNgaTu(sensorState)) {
         setMotorSpeed(0,0);
          delay(200);
        ReTrai();
        step = 8; // GO THRU NODE1 -> START
      } else {
        line_detect();
      }
      break;
      case 8:
       if (checkNgaTu(sensorState)) {
          setMotorSpeed(0, 0);
          delay(200);
          step = 9; // DONE
      } else {
        line_detect();
      }
      break;
      case 9:
        QuayDau();
        step = 10;
      break;
      case 10:
      quatrinh = false;
      break;
  }
  }
  if(x==3 && y==1){
    switch (step) {

      case 0:
      if (checkNgaTu(sensorState)) {
        intersectionCount++;

        if (intersectionCount == 1) {
          setMotorSpeed(80, 80);
          delay(300); // Chạy qua node1 chút
        } else if (intersectionCount == 2) {
          // Đã đến Start => dừng hẳn
          setMotorSpeed(0, 0);
          delay(100);
          ReTrai();
          step = 1; // DONE
          intersectionCount = 0;
        }
      } else {
        line_detect();
      }
      break;

      case 1:
      if (checkNgaTu(sensorState)) {
        setMotorSpeed(0, 0);
        stateTimer = millis();   
        step = 2;              
      } else {
        line_detect();
      }
      break;

      case 2:
      if (millis() - stateTimer >= 3000) {
        QuayDau();
        step = 3;
      }
      break;
      case 3:
        if (checkNgaTu(sensorState)) {
           setMotorSpeed(0,0);
          delay(200);
        RePhai();
        step = 4; // GO THRU NODE1 -> START
      } else {
        line_detect();
      }
      break;
      case 4:
        if (checkNgaTu(sensorState)) {
           setMotorSpeed(0,0);
          delay(200);
        RePhai();
        step = 5; // GO THRU NODE1 -> START
      } else {
        line_detect();
      }
      break;
      case 5:
      if (checkNgaTu(sensorState)) {
        setMotorSpeed(0, 0);
        stateTimer = millis();   
        step = 6;              
      } else {
        line_detect();
      }
      break;
      case 6:
      if(millis() - stateTimer >= 3000) {
        QuayDau();
        step = 7;
      } 
      break;

      case 7:
       if (checkNgaTu(sensorState)) {
         setMotorSpeed(0,0);
          delay(200);
        RePhai();
        step = 8; // GO THRU NODE1 -> START
      } else {
        line_detect();
      }
      break;
      case 8:
       if (checkNgaTu(sensorState)) {
          setMotorSpeed(0, 0);
          delay(200);
          step = 9; // DONE
      } else {
        line_detect();
      }
      break;
      case 9:
        QuayDau();
        step = 10;
      break;
      case 10:
      quatrinh = false;
      break;
  }
  }
  if(x==4 && y==3){
     switch (step) {

      case 0:
      if (checkNgaTu(sensorState)) {
        intersectionCount++;

        if (intersectionCount == 1) {
          setMotorSpeed(80, 80);
          delay(300); // Chạy qua node1 chút
        } else if (intersectionCount == 2) {
          // Đã đến Start => dừng hẳn
          setMotorSpeed(0, 0);
          delay(200);
          RePhai();
          step = 1; // DONE
          intersectionCount = 0;
        }
      } else {
        line_detect();
      }
      break;

      case 1:
      if (checkNgaTu(sensorState)) {
        setMotorSpeed(0, 0);
        stateTimer = millis();   
        step = 2;              
      } else {
        line_detect();
      }
      break;

      case 2:
      if (millis() - stateTimer >= 3000) {
        QuayDau();
        step = 3;
      }
      break;
      case 3:
        if (checkNgaTu(sensorState)) {
        intersectionCount++;

        if (intersectionCount == 1) {
          setMotorSpeed(80, 80);
          delay(300); // Chạy qua node1 chút
        } else if (intersectionCount == 2) {
          // Đã đến Start => dừng hẳn
          setMotorSpeed(0, 0);
          stateTimer = millis();
          step = 4; // DONE
          intersectionCount = 0;
        }
      } else {
        line_detect();
      }
      break;
      case 4:
      if(millis() - stateTimer >= 3000) {
        QuayDau();
        step = 5;
      } 
      break;

      case 5:
       if (checkNgaTu(sensorState)) {
         setMotorSpeed(0,0);
          delay(200);
        RePhai();
        step = 6; // GO THRU NODE1 -> START
      } else {
        line_detect();
      }
      break;
      case 6:
       if (checkNgaTu(sensorState)) {
        intersectionCount++;

        if (intersectionCount == 1) {
          setMotorSpeed(80, 80);
          delay(300); // Chạy qua node1 chút
        } else if (intersectionCount == 2) {
          // Đã đến Start => dừng hẳn
          setMotorSpeed(0, 0);
          delay(200);
          step = 7; // DONE
          intersectionCount = 0;
        }
      } else {
        line_detect();
      }
      break;
      case 7:
        QuayDau();
        step = 8;
      break;
      case 8:
      quatrinh = false;
      break;
  }
  }
  if(x==4 && y==2){
    switch (step) {

      case 0:
      if (checkNgaTu(sensorState)) {
        intersectionCount++;

        if (intersectionCount == 1) {
          setMotorSpeed(80, 80);
          delay(300); // Chạy qua node1 chút
        } else if (intersectionCount == 2) {
          // Đã đến Start => dừng hẳn
          setMotorSpeed(0, 0);
          delay(200);
          RePhai();
          step = 1; // DONE
          intersectionCount = 0;
        }
      } else {
        line_detect();
      }
      break;

      case 1:
      if (checkNgaTu(sensorState)) {
        setMotorSpeed(0, 0);
        stateTimer = millis();   
        step = 2;              
      } else {
        line_detect();
      }
      break;

      case 2:
      if (millis() - stateTimer >= 3000) {
        QuayDau();
        step = 3;
      }
      break;
      case 3:
        if (checkNgaTu(sensorState)) {
           setMotorSpeed(0,0);
          delay(200);
        ReTrai();
        step = 4; // GO THRU NODE1 -> START
      } else {
        line_detect();
      }
      break;
      case 4:
        if (checkNgaTu(sensorState)) {
          setMotorSpeed(0,0);
          delay(200);
        ReTrai();
        step = 5; // GO THRU NODE1 -> START
      } else {
        line_detect();
      }
      break;
      case 5:
      if (checkNgaTu(sensorState)) {
        setMotorSpeed(0, 0);
        stateTimer = millis();   
        step = 6;              
      } else {
        line_detect();
      }
      break;
      case 6:
      if(millis() - stateTimer >= 3000) {
        QuayDau();
        step = 7;
      } 
      break;

      case 7:
       if (checkNgaTu(sensorState)) {
        ReTrai();
        step = 8; // GO THRU NODE1 -> START
      } else {
        line_detect();
      }
      break;
      case 8:
       if (checkNgaTu(sensorState)) {
          setMotorSpeed(0, 0);
          delay(200);
          step = 9; // DONE
      } else {
        line_detect();
      }
      break;
      case 9:
        QuayDau();
        step = 10;
      break;
      case 10:
      quatrinh = false;
      break;
  }
  }
  if(x==4 && y==1){
    switch (step) {

      case 0:
      if (checkNgaTu(sensorState)) {
        intersectionCount++;

        if (intersectionCount == 1) {
          setMotorSpeed(80, 80);
          delay(300); // Chạy qua node1 chút
        } else if (intersectionCount == 2) {
          // Đã đến Start => dừng hẳn
          setMotorSpeed(0, 0);
          delay(100);
          RePhai();
          step = 1; // DONE
          intersectionCount = 0;
        }
      } else {
        line_detect();
      }
      break;

      case 1:
      if (checkNgaTu(sensorState)) {
        setMotorSpeed(0, 0);
        stateTimer = millis();   
        step = 2;              
      } else {
        line_detect();
      }
      break;

      case 2:
      if (millis() - stateTimer >= 3000) {
        QuayDau();
        step = 3;
      }
      break;
      case 3:
        if (checkNgaTu(sensorState)) {
          setMotorSpeed(0,0);
          delay(200);
        ReTrai();
        step = 4; // GO THRU NODE1 -> START
      } else {
        line_detect();
      }
      break;
      case 4:
        if (checkNgaTu(sensorState)) {
        setMotorSpeed(0,0);
        delay(200);
        RePhai();
        step = 5; // GO THRU NODE1 -> START
      } else {
        line_detect();
      }
      break;
      case 5:
      if (checkNgaTu(sensorState)) {
        setMotorSpeed(0, 0);
        stateTimer = millis();   
        step = 6;              
      } else {
        line_detect();
      }
      break;
      case 6:
      if(millis() - stateTimer >= 3000) {
        QuayDau();
        step = 7;
      } 
      break;

      case 7:
       if (checkNgaTu(sensorState)) {
        RePhai();
        setMotorSpeed(0,0);
        delay(200);
        step = 8; // GO THRU NODE1 -> START
      } else {
        line_detect();
      }
      break;
      case 8:
       if (checkNgaTu(sensorState)) {
          setMotorSpeed(0, 0);
          delay(200);
          step = 9; // DONE
      } else {
        line_detect();
      }
      break;
      case 9:
        QuayDau();
        step = 10;
      break;
      case 10:
      quatrinh = false;
      break;
  }
  }
}

void bluetooth(){
  if (Serial.available() > 0) {  // Kiểm tra nếu có dữ liệu từ Serial
        data = Serial.read(); // Đọc dữ liệu từ Bluetooth
        Serial.println(char(data)); // In giá trị nhận được ra Serial Monitor

        // Điều khiển LED dựa vào dữ liệu nhận được
        if (data == 'a') { X = 1; Y = 2;      }
        else if (data == 'b') { X = 1; Y = 3; }
        else if (data == 'c') { X = 1; Y = 4; }
        else if (data == 'd') { X = 2; Y = 1; }
        else if (data == 'e') { X = 2; Y = 3; }
        else if (data == 'f') { X = 2; Y = 4; }
        else if (data == 'g') { X = 3; Y = 1; }
        else if (data == 'h') { X = 3; Y = 2; }
        else if (data == 'i') { X = 3; Y = 4; }
        else if (data == 'j') { X = 4; Y = 1; }
        else if (data == 'k') { X = 4; Y = 2; }
        else if (data == 'l') { X = 4; Y = 3; }
        else{ Serial.println("Invalid data!");}
        if(!quatrinh) {
           step = 0;  // Reset state machine cho lộ trình mới
           quatrinh = true;  // Bắt đầu lộ trình mới
        }
        
    }
    
}
int last_turn_direction = 0;
void line_detect() {
   if (checkVatCan()) {
        setMotorSpeed(-30, -30); // Dừng lại nếu có vật cản
        delay(200);
        digitalWrite(2, HIGH); // Bật đèn báo vật cản
        while (checkVatCan()) { // Chờ đến khi không còn vật cản
            delay(100);
        }
        digitalWrite(2, LOW); // Tắt đèn khi vật cản đã mất
    }
  if (sensorState[3] == 1 || sensorState[4] == 1) {
    last_turn_direction = 0; // Reset trạng thái rẽ
    if (sensorState[3] == 1 && sensorState[4] == 0) {
      // Nhích sang trái
      setMotorSpeed(base_speed - 5, base_speed + 5);
    } else if (sensorState[4] == 1 && sensorState[3] == 0) {
      // Nhích sang phải
      setMotorSpeed(base_speed + 5, base_speed - 5);
    } else if (sensorState[3] == 1 && sensorState[4] == 1) {
      // Đi thẳng
      setMotorSpeed(base_speed, base_speed);
    }
  } 
  else {
    int a = 0;  // Hệ số tăng tốc
    int b = 0;  // Hệ số giảm tốc
    // Nếu line nằm bên trái (cảm biến 0,1,2)
    if (sensorState[0] || sensorState[1] || sensorState[2]) {
      last_turn_direction = -1;
      if (sensorState[2] == 1) {
        a = max_speed / 2;
        b = base_speed / 2;
      } else if (sensorState[1] == 1) {
        a = (2 * max_speed) / 3;
        b = base_speed;
      } else if (sensorState[0] == 1) {
        a = (3 * max_speed) / 4;
        b = base_speed;
      }
      // Tăng tốc bánh phải, giảm bánh trái => quay trái
      setMotorSpeed(base_speed - b, base_speed + a);
    }
    // Nếu line nằm bên phải (cảm biến 5,6,7)
    else if (sensorState[5] || sensorState[6] || sensorState[7]) {
      last_turn_direction = 1;
      if (sensorState[5] == 1) {
        a = max_speed / 2;
        b = base_speed / 2;
      } else if (sensorState[6] == 1) {
        a = (2 * max_speed) / 3;
        b = base_speed;
      } else if (sensorState[7] == 1) {
        a = (3 * max_speed) / 4;
        b = base_speed;
      }
      // Tăng tốc bánh trái, giảm bánh phải => quay phải
      setMotorSpeed(base_speed + a, base_speed - b);
    }
    // Nếu không thấy line ở bất kỳ vùng nào
    else {
      // Tiếp tục rẽ theo trạng thái trước đó
      if (last_turn_direction == -1) {
        // Nếu lần trước rẽ trái, tiếp tục quay trái (hoặc dừng bên trái)
        setMotorSpeed(0, 50);
      } else if (last_turn_direction == 1) {
        setMotorSpeed(50, 0);
      } else {
        // Mặc định: chạy chậm
        setMotorSpeed(max_speed / 4, 0);
      }
    }
  }
}
// Vòng loop chính
void loop() {
  readSensor();
 if (X != 0 && Y != 0) {
    dichuyen(X, Y, quatrinh);
  }  
  bluetooth();

  delay(10); // Tránh lặp quá nhanh


  //  int distance1 = sonar1.ping_cm();
  //   int distance2 = sonar2.ping_cm();
  //   int distance3 = sonar3.ping_cm();

  //   // In khoảng cách đo được từ các cảm biến siêu âm
  //   Serial.print("Distance1: "); Serial.print(distance1); Serial.print(" cm, ");
  //   Serial.print("Distance2: "); Serial.print(distance2); Serial.print(" cm, ");
  //   Serial.print("Distance3: "); Serial.print(distance3); Serial.println(" cm");

  //   if (checkVatCan()) {
  //       digitalWrite(2, HIGH); // Bật đèn báo vật cản
  //       Serial.println("Vật cản phát hiện! (1)");
  //   } else {         
  //       digitalWrite(2, LOW); 
  //       Serial.println("Không có vật cản.");
  //   }

  //   delay(500); // Giảm tốc độ lặp để dễ theo dõi
}
