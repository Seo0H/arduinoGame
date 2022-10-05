#include <MsTimer2.h>
#include <LedControl.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

//--전역변수 부분--

//매트릭스 연결
const int DIN = 12;
const int CS = 11;
const int CLK = 10;
LedControl lc = LedControl(DIN, CLK, CS, 1);

LiquidCrystal_I2C lcd(0x27, 16, 2);

//조이스틱 
const int varXPin = A3;
const int pushPin = 3;
int ReadLx = 0;

//내 초기 위치(마지막 행의 열 위치)
int startY = 3;

//시작상태인지 체크
bool isStart = false;

//life 초깃값
int life = 3;

//게임 시간 측정
int gameTime_s = 0;

//시간 측정 변수
unsigned long cur_time;
unsigned long old_time = millis()/1000;

void setup() {
  Serial.begin(9600); //시리얼 통신 세팅
  
  lc.shutdown(0,false); //lc 초기화
  lc.setIntensity(0,8);
  lc.clearDisplay(0);

  lcd.init();  // lcd 초기화
  lcd.backlight();  // lcd 백라이트

  // 조이스틱 선언부분
  pinMode(varXPin, INPUT); 
  pinMode(pushPin, INPUT_PULLUP);

  //타이머 설정
  MsTimer2::set(100, move); //1초 주기로 move() 실행
  MsTimer2::start ();
  
}

void loop() {

 //게임 스타트 부분
  int push = digitalRead(pushPin);
  cur_time = millis()%60;


  if (push == 0){ // 3초 누르면 게임시작
    if(fabs(cur_time - old_time)>=3 && (isStart ==false)){
      isStart=true;
      Serial.println("게임 시작");
      }
    }


  //게임 시작 부분
  if(isStart == true) { 
    
    lightInit();
    gameTime();
    ledDown();

    //죽었을 경우 
    if(life == 0){
      isStart = false;
      life = 3;
      gameTime_s = 0;
      lcd.setCursor(0,0);
      lcd.print("  Game Over..  ");
      lcd.setCursor(0,1);
      lcd.print("   :(   "); 
      delay(5000); //5초 딜레이 후 밑의 else 코드 진행 (* 표시한 부분)
    }
  }
  else {
      lcd.setCursor(0,0);
      lcd.print("  Push Joystic  ");
      lcd.setCursor(0,1);
      lcd.print("   game start!  ");       
    } 
}


void lightInit(){ //lcd 백라이트 초기화 함수
  lcd.init();  // I2C LCD를 초기화 합니다..
  lcd.backlight();  // I2C LCD의 백라이트를 켜줍니다.
  }


void gameTime() { //게임 플레이시간 측정 + lcd에 뿌리는 함수
  cur_time = millis();
  if (fabs(old_time-cur_time)>=1000){
    gameTime_s += 1;
  }
  int gameTime_m = 0;
  Serial.println (gameTime_s);

  if (gameTime_s >= 60 ) {
    gameTime_s = 0;
    gameTime_m += 1;
  }
      
  
  lcd.setCursor(0,0);
  lcd.print(":");
  lcd.print(gameTime_m);
  lcd.print(":");
  lcd.print(gameTime_s);
  lcd.setCursor(0,1);
  lcd.print("Life:");
  lcd.print(life);
}

void ledDown() {
  
  int col = random(8); //0부터 7까지의 값 중 하나를 랜덤하게 열 값으로 받는다. 
  int row = 0;
  for(row = 0; row<8; row++){
    if(row == 0) {
      lc.setLed(0,row,col,true); //첫번째 행 켜기
    }
    else if(row == 1){
      lc.setLed(0,row,col,true); //첫번째 행 켜기
    }
    else if (row == 6){ //마지막줄에서 못피했는지 체크
      lc.setLed(0,row-2,col,false); //첫번째 행부터 끄기
      lc.setLed(0,row,col,true); //입력받은 열만 킴

      if(col == startY) //6번째 행의 col위치와 7번째 행의 내 위치가 같으면 => 마주쳤으면
      {
        life -= 1;
        lc.setLed(0,row-1,col,false); //5행 끄기
        lc.setLed(0,row,col,false); //6행 끄기  
        Serial.print("life : ");
        Serial.println(life);
        
        if(life ==0 ){
          row = 10; //row 증가시켜서 for문 끝내기
        }
      }
    }
    else {
      lc.setLed(0,row-2,col,false); //첫번째 행부터 끄기
      lc.setLed(0,row,col,true); //입력받은 열만 킴
    }
    
    // 내려가는 시간에 적당한 delay주기
    delay(100);
  }
  
  if(row == 8){ //마주치지 않고 끝까지 내려왔다면
    lc.setLed(0,6,col,false);//첫번째 행부터 끄기
    //delay(500);
    // ****** 이 부분에서 뒤의 7행의 도트와 부딪히는 경우 고려
    if(col == startY) //7번째 행의 col위치와 7번째 행의 내 위치가 같으면 => 마주쳤으면
    {
      life -= 1;
    }
    // ********  
    lc.setLed(0,7,col,false);
  } 

}

 
void move() {
    //움직이는 기능 -> timer를 통해 1초에 한번씩 움직임을 확인하고 그 시간동안은 ledDown이 잠시 멈추도록 구현??
    ReadLx = analogRead(varXPin); //x축으로 이동한 값 읽기
    //Serial.println(ReadLx);
    int mapLx = map(ReadLx, 0, 1023, -512, 512); //범위조정
    
    //오른쪽으로 한 칸 이동
    if(mapLx < -300) //이동한 정도가 너무 예민하지 않도록 조절하기
    {
      if(startY < 7){ //양쪽 모서리가 아닌지 확인
        lc.setLed(0,7,startY,false); //첫번째 행부터 끄기
        startY += 1;
        lc.setLed(0,7,startY,true); //입력받은 열만 킴
      }
    }
     
    //왼쪽으로 한 칸 이동
    if(mapLx > 300)
    {
      if(startY > 0){ //양쪽 모서리가 아닌지 확인 
        lc.setLed(0,7,startY,false); //첫번째 행부터 끄기
        startY -= 1;
        lc.setLed(0,7,startY,true); //입력받은 열만 킴
      }
    }
}
