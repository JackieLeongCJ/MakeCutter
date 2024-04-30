#include <Arduino.h>
#include <Streaming.h>
#include <Vector.h>

#include <LiquidCrystal_I2C.h>
#include <key_pad.h>

#include <power_stepper.h>
#include <Servo.h>

#define NullChar 0
#define LastChar 'A'
#define NextChar 'B'
#define CreateChar 'C'
#define DeleteChar 'D'
#define EditChar 'E'
#define EnterChar 'F'
#define CancelChar 'C'

LiquidCrystal_I2C lcd(0x27, 20, 4);

KeyPad myKeyPad(300, 4, 4);

String mode = "list";

struct Task {
  int len;
  int num;
};

const int ELEMENT_COUNT_MAX = 100;
Task storage_array[ELEMENT_COUNT_MAX];
Vector<Task> tasks;
int showTaskIndex = 0;

PowerStepper myStepper(5, 4, 10000);

Servo myservo_cutter;
Servo myservo_director;
int RESET_STEP = 10;
double STEPS_PER_CM = 1000/50.8;

void Direct_cut() {   //去剪線的位置
  myservo_director.write(31);
  delay(2000);
}

void Direct_run() {   //去出線的位置
  myservo_director.write(23);
  delay(2000);
}

void Direct_reset() {   //回到原來的位置
  myservo_director.write(0);
  delay(2000);
}

void Cut_pull() {   // 往下剪線
  myservo_cutter.write(160);
  delay(2000);
}

void Cut_release() {  // 剪線鬆開
  myservo_cutter.write(10);
  delay(2000);
}

void Cut() {    // 剪線
  myservo_cutter.write(160);
  delay(2000);
  myservo_cutter.write(10);
  delay(2000);
}

void Move_one_step(int step) {
  myStepper.moveSteps(step);
  while(!myStepper.taskComplete())
  {
    myStepper.doTasks();
  }
}

void Move(int step) {    // 拉線
  Serial.println("Move");
  Move_one_step(-RESET_STEP);
  Move_one_step(RESET_STEP);
  Move_one_step(step);
}

bool IsDigit(char c) {
  return c >= '0' && c <= '9';
}

int ConvertStringToInt(String str, int index) {
  int num = 0;
  int idx = 0;
  int count = 0;
  while (idx < str.length() && !IsDigit(str[idx])) idx++;
  while (idx < str.length()) {
    if (IsDigit(str[idx])) {
      num *= 10;
      num += (int)str[idx] - '0';
    }
    else {
      count += 1;
      if (count > index) return num;
      num = 0;
    }
    idx++;
  }
  return num;
}

char GetKeypadKey() {
  while(true) {
    myKeyPad.doRoutine();
    char keypressed = myKeyPad.getKey();
    if (keypressed != NullChar) {
      // Serial.println(keypressed);
      return keypressed;
    }
  }
}

String LCDInput(int col, int row, String init_str) {
  String tmp = init_str;
  int mod = 3;
  int count = 0;

  char keypressed = GetKeypadKey();
  while (keypressed != EnterChar && keypressed != CancelChar) {
    if (IsDigit(keypressed)) {
      tmp[count] = keypressed;
      count = (count+1) % mod;
      lcd.setCursor(col, row);
      lcd.print(tmp);
    }
    keypressed = GetKeypadKey();
  }
  if (keypressed == EnterChar) return tmp;
  return "   ";
}

String PadRight(String str, int len) {
  for(int i=str.length();i<len;i++) str += " ";
  return str;
}

void AddTaskPage() {
  mode = "list";
  lcd.setCursor(0, 0);
  lcd.print("Add New Task:       ");
  lcd.setCursor(0, 1);
  lcd.print("length:     cm      ");
  lcd.setCursor(0, 2);
  lcd.print("number:             ");
  lcd.setCursor(0, 3);
  lcd.print("                    ");
  String len = LCDInput(8, 1, "   ");
  if (len.equals("   ")) return;
  String num = LCDInput(8, 2, "   ");
  if (num.equals("   ")) return;
  Task task;
  task.len = ConvertStringToInt(len, 0);
  task.num = ConvertStringToInt(num, 0);
  tasks.push_back(task);
}

void setup()
{
  Serial.begin(115200);
  Serial.println("Serial Start");

  // keypad setting
  char **key = new char*[4];
  key[0] = new char[4]{'0', '1', '2', '3'};
  key[1] = new char[4]{'4', '5', '6', '7'};
  key[2] = new char[4]{'8', '9', 'A', 'B'};
  key[3] = new char[4]{'C', 'D', 'E', 'F'};
  int *row_pin = new int[4]{22, 24, 26, 28}, *col_pin = new int[4]{23, 25, 27, 29};
  myKeyPad.Begin(col_pin, row_pin, key);
  
  for(int i=0;i<4;i++)
  {
    delete[] key[i];
  }
  delete[] key;
  delete[] col_pin;
  delete[] row_pin;

  // lcd setting
  lcd.init(); 
  lcd.backlight();

  // tasks setting
  tasks.setStorage(storage_array);

  // servo setting
  myservo_cutter.attach(9);
  myservo_cutter.write(20);

  myservo_director.attach(10);
  myservo_director.write(0);   // 設為reset的位置
}

void loop()
{
  myStepper.doTasks(); 
  if (myStepper.taskComplete() && !tasks.empty()) {
    if (tasks[0].num > 0){
      tasks[0].num--;
      int len = tasks[0].len;
      int num = tasks[0].num;
      int step = round(len * STEPS_PER_CM);
      Serial << "doTask => len: " << len << ", num: " << num << ", step: " << step << endl;

      Direct_cut();
      Cut();
      Direct_reset();
      Cut_pull();
      Move_one_step(-70);
      Cut_release();
      Move_one_step(70);
      Direct_run();

      Move(step);

      myservo_director.write(2);
      delay(2000);
      Cut();
      Direct_run();
      Move_one_step(8);
      Direct_cut();
      Cut();

      Serial.println("done");
    }
    else{
      Direct_reset();
      tasks.remove(0);
    } 
  }

  if (tasks.empty()) {
    AddTaskPage();
  }
}