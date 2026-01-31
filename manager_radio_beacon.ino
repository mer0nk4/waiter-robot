#include <RCSwitch.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>

#define TRANSMIT_PIN 12
#define BUZZER_PIN 11
RCSwitch Transmitter = RCSwitch();
LiquidCrystal_I2C lcd(0x27, 16, 2);

RCSwitch Receiver = RCSwitch();


const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte rowPins[ROWS] = {10,9,8,7};
byte colPins[COLS] = {6,5,4,3};
Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );

String currentTask = "None";
bool inputMode = false;
byte tableInput = 0;

void setup() {
  lcd.init();
  lcd.backlight();

  Transmitter.enableTransmit(TRANSMIT_PIN);
  Transmitter.setProtocol(1);
  Transmitter.setPulseLength(350);
  Transmitter.setRepeatTransmit(10);
  Receiver.enableReceive(digitalPinToInterrupt(2));

  pinMode(BUZZER_PIN, OUTPUT);

  showCurrentTask();
  Serial.begin(9600);
}

void showCurrentTask() {
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Current Task:");
  lcd.setCursor(0,1);
  lcd.print(currentTask);
}

void receiveTasks() {
  if (Receiver.available()) {
    unsigned long packet = Receiver.getReceivedValue();

    byte typeId = (packet >> 8) & 0b11;
    if (typeId == 0b10) {
      byte tableId = (packet >> 2) & 0b111111;
      byte taskId  = packet & 0b11;
      Serial.println(packet);
      currentTask = String("T") + String(taskId) + " -> Table " + String(tableId);
      showCurrentTask();
    }
    else if (packet == 0b11000000) {
      currentTask = "Robot stuck!";
      digitalWrite(BUZZER_PIN, 1);
      delay(1000);
      digitalWrite(BUZZER_PIN, 0);
      showCurrentTask();
    }

    Receiver.resetAvailable();
  }
}


void loop() {
  receiveTasks();

  char key = keypad.getKey();

  if (!inputMode) { // обычный режим — кнопка A для начала ввода
    
    if (key == 'A') {
      inputMode = true;
      tableInput = 0;
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Enter TableID:");
      lcd.setCursor(0,1);
      lcd.print(tableInput);
    }
  }
  else { // режим ввода номера стола

    if (key >= '0' && key <= '9') {
      byte digit = key - '0';
      byte newValue = tableInput * 10 + digit;
      if (newValue <= 63) {
        tableInput = newValue;
        lcd.setCursor(0,1);
        lcd.print("                ");
        lcd.setCursor(0,1);
        lcd.print(tableInput);
      }
    }
    else if (key == '#') { // подтверждение
      byte taskId = 1;
      unsigned long packet = ((unsigned long)0b01 << 8) | ((unsigned long)tableInput << 2) | taskId;
      Transmitter.send(packet, 10);
      digitalWrite(BUZZER_PIN, 1);
      delay(500);
      digitalWrite(BUZZER_PIN, 0);
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Task sent!");
      delay(1000);
      inputMode = false;
      showCurrentTask();
    }
    else if (key == '*') { // сброс ввода
      tableInput = 0;
      lcd.setCursor(0,1);
      lcd.print("                ");
      lcd.setCursor(0,1);
      lcd.print(tableInput);
    }
    else if (key == 'B') { // возврат в режим показа статуса
      tableInput = 0;
      inputMode = false;
      showCurrentTask();
    }
  }

  delay(50);
}
