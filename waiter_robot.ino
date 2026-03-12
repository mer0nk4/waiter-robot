#include <RCSwitch.h> 
#include <Ultrasonic.h>

#define rf 2 // right forward
#define rb 4 // right back
#define lf 6 // left forward
#define lb 7 // left back
#define ENA 5
#define ENB 3
#define trigPin 11
#define echoPin 10
#define lineSensorPin 8
#define RledPin 1
#define GledPin 12
#define BledPin 13

RCSwitch Receiver;
RCSwitch Transmitter;
Ultrasonic ultrasonic(trigPin, echoPin);

// -------------------------------МАРШРУТЫ-------------------------------
String routeToTable1[] = {"f211", "l90", "f91"};
const int route1Length = sizeof(routeToTable1)/sizeof(routeToTable1[0]);
String routeBackFromTable1[route1Length + 1];

String routeToTable2[] = {"f211", "r90", "f91"};
const int route2Length = sizeof(routeToTable2)/sizeof(routeToTable2[0]);
String routeBackFromTable2[route2Length + 1];

String routeToTable3[] = {"r90"};
const int route3Length = sizeof(routeToTable3)/sizeof(routeToTable3[0]);
String routeBackFromTable3[route3Length + 1];

String routeToTable4[] = {"f200"};
const int route4Length = sizeof(routeToTable4)/sizeof(routeToTable4[0]);
String routeBackFromTable4[route4Length + 1];

String routeToTable5[] = {"r180"};
const int route5Length = sizeof(routeToTable5)/sizeof(routeToTable5[0]);
String routeBackFromTable5[route5Length + 1];

String routeToTable6[] = {"r90"};
const int route6Length = sizeof(routeToTable6)/sizeof(routeToTable6[0]);
String routeBackFromTable6[route6Length + 1];
// -------------------------------МАРШРУТЫ-------------------------------

#define queueSize 128
unsigned int taskQueue[queueSize];
int pointer_completed = 0;
int pointer_received = 0;
unsigned int currentTask = 0;
unsigned long statusPacket = 0;
unsigned int lastCompletedTask = 0;
static unsigned long lastTime = 0;

void setup() {
  pinMode(lf, OUTPUT);
  pinMode(lb, OUTPUT);
  pinMode(rf, OUTPUT);
  pinMode(rb, OUTPUT);
  pinMode(ENA, OUTPUT);
  pinMode(ENB, OUTPUT);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(lineSensorPin, INPUT);
  pinMode(RledPin, OUTPUT);
  pinMode(GledPin, OUTPUT);
  pinMode(BledPin, OUTPUT);
  Serial.begin(9600);

  Receiver.enableReceive(2);
  Transmitter.enableTransmit(9);
  Transmitter.setProtocol(1);
  Transmitter.setPulseLength(350);
  Transmitter.setRepeatTransmit(5);

  toBase(routeToTable1, routeBackFromTable1, route1Length);
  toBase(routeToTable2, routeBackFromTable2, route2Length);
  toBase(routeToTable3, routeBackFromTable3, route3Length);
  toBase(routeToTable4, routeBackFromTable4, route4Length);
  toBase(routeToTable5, routeBackFromTable5, route5Length);
  toBase(routeToTable6, routeBackFromTable6, route6Length);
}

void loop() {
  receiveTasks();

  if (!queueEmpty()) {
    currentTask = getNextTask();

    if (currentTask != lastCompletedTask) {

      byte typeId  = (currentTask >> 8) & 0b11;       // первые 2 бита = тип пакета
      byte tableId = (currentTask >> 2) & 0b111111;   // 6 бит = номер стола
      byte taskId  = currentTask & 0b11;             // последние 2 бита = задача

      if (typeId == 0b01) { // команда

        if (tableId == 1 && taskId == 1) {
          unsigned long statusPacket = (0b10 << 8) | (tableId << 2) | taskId; // отправка статуса менеджеру
          delay(150);
          Transmitter.send(statusPacket, 10);
          table1_task1();
          lastCompletedTask = currentTask;
          Transmitter.send(1000000000, 10);
        }

        if (tableId == 1 && taskId == 2) {
          unsigned long statusPacket = (0b10 << 8) | (tableId << 2) | taskId; // отправка статуса менеджеру
          delay(150);
          Transmitter.send(statusPacket, 10);
          table1_task2();
          lastCompletedTask = currentTask;
          Transmitter.send(1000000000, 10);
        }

        if (tableId == 2 && taskId == 1) {
          unsigned long statusPacket = (0b10 << 8) | (tableId << 2) | taskId;
          delay(150);
          Transmitter.send(statusPacket, 10);
          table2_task1();
          lastCompletedTask = currentTask;
          Transmitter.send(1000000000, 10);
        }

        if (tableId == 2 && taskId == 2) {
          unsigned long statusPacket = (0b10 << 8) | (tableId << 2) | taskId; // отправка статуса менеджеру
          delay(150);
          Transmitter.send(statusPacket, 10);
          table2_task2();
          lastCompletedTask = currentTask;
          Transmitter.send(1000000000, 10);
        }

        if (tableId == 3 && taskId == 1) {
          unsigned long statusPacket = (0b10 << 8) | (tableId << 2) | taskId;
          delay(150);
          Transmitter.send(statusPacket, 10);
          table3_task1();
          lastCompletedTask = currentTask;
          Transmitter.send(1000000000, 10);
        }

        if (tableId == 4 && taskId == 1) {
          unsigned long statusPacket = (0b10 << 8) | (tableId << 2) | taskId;
          delay(150);
          Transmitter.send(statusPacket, 10);
          table4_task1();
          lastCompletedTask = currentTask;
          Transmitter.send(1000000000, 10);
        }

        if (tableId == 5 && taskId == 1) {
          unsigned long statusPacket = (0b10 << 8) | (tableId << 2) | taskId;
          delay(150);
          Transmitter.send(statusPacket, 10);
          table5_task1();
          lastCompletedTask = currentTask;
          Transmitter.send(1000000000, 10);
        }

        if (tableId == 6 && taskId == 1) {
          unsigned long statusPacket = (0b10 << 8) | (tableId << 2) | taskId;
          delay(150);
          Transmitter.send(statusPacket, 10);
          table6_task1();
          lastCompletedTask = currentTask;
          Transmitter.send(1000000000, 10);
        }

        // и так далее по шаблону
      }
    }
  }
  else {
    digitalWrite(RledPin, LOW);
    digitalWrite(GledPin, HIGH);
    digitalWrite(BledPin, LOW);
  }
}

void receiveTasks() {
  if (Receiver.available()) {
    unsigned long now = millis();

    if (now - lastTime > 300) {
      unsigned long packet = Receiver.getReceivedValue();
      
      byte typeId = (packet >> 8) & 0b11; // фильтруем только команды
      if (typeId == 0b01) {
        addTask(packet);
        Serial.println(packet);
      }

      lastTime = now;
    }
    Receiver.resetAvailable();
  }
}

// HIGH - поднос есть, LOW - отсутствие подноса
void waitForTrayState(int state) {
  digitalWrite(RledPin, HIGH);
  digitalWrite(GledPin, HIGH);
  digitalWrite(BledPin, HIGH);
  while (digitalRead(lineSensorPin) == state) {
    receiveTasks();
    delay(100);
  }

  unsigned long start = millis();
  digitalWrite(RledPin, LOW);
  digitalWrite(GledPin, LOW);
  digitalWrite(BledPin, HIGH);
  while (millis() - start < 5000) {
    receiveTasks();
    delay(100);
  }
}

void forward(int targetDistance) {
  int distanceComplete = 0;
  start();

  while (distanceComplete < targetDistance) {
    receiveTasks();
    long dist = ultrasonic.read();

    if (dist > 0 && dist < 30) {
      stop();

      unsigned long waitStart = millis();
      bool signalSent = false;

      while (ultrasonic.read() < 30) {
        receiveTasks();
        delay(100);

        if (!signalSent && millis() - waitStart >= 7000) {
          digitalWrite(RledPin, HIGH);
          digitalWrite(GledPin, LOW);
          digitalWrite(BledPin, LOW);
          Transmitter.send((0b11 << 8), 10);  // первые 2 бита "11" = препятствие
          signalSent = true;
        }
      }
      if (signalSent) {
        Transmitter.send(statusPacket, 10);
        digitalWrite(RledPin, LOW);
        digitalWrite(GledPin, LOW);
        digitalWrite(BledPin, HIGH);
      }
      start();
    }

    analogWrite(ENA, 255);
    analogWrite(ENB, 255);
    distanceComplete += 4.4851; // один шаг
    delay(50);
  }

  stop();
}

void rotate_right(int angle) {
  digitalWrite(rf, 0); digitalWrite(lf, 1);
  digitalWrite(rb, 1); digitalWrite(lb, 0);
  analogWrite(ENA, 255); analogWrite(ENB, 255);
  if (angle < 180) delay(8.2 * angle);
  else delay(7.02 * angle);
  digitalWrite(rf, 1); digitalWrite(lf, 1);
  digitalWrite(rb, 1); digitalWrite(lb, 1);
}

void rotate_left(int angle) {
  digitalWrite(rf, 1); digitalWrite(lf, 0);
  digitalWrite(rb, 0); digitalWrite(lb, 1);
  analogWrite(ENA, 255); analogWrite(ENB, 255);
  if (angle < 180) delay(8.2 * angle);
  else delay(7.02 * angle);
  digitalWrite(rf, 1); digitalWrite(lf, 1);
  digitalWrite(rb, 1); digitalWrite(lb, 1);
}

void start() {
  digitalWrite(rf, 1); digitalWrite(lf, 1);
  digitalWrite(rb, 0); digitalWrite(lb, 0);

  for (int currentSpeed = 0; currentSpeed <= 255; currentSpeed += 5) {
    receiveTasks();
    analogWrite(ENA, currentSpeed);
    analogWrite(ENB, currentSpeed);
    delay(10);
  }
}

void stop() {
  digitalWrite(rf, 1); digitalWrite(lf, 1);
  digitalWrite(rb, 0); digitalWrite(lb, 0);

  for (int currentSpeed = 255; currentSpeed >= 0; currentSpeed -= 5) {
    receiveTasks();
    analogWrite(ENA, currentSpeed);
    analogWrite(ENB, currentSpeed);
    delay(10);
  }
}

void table1_task1() {
  waitForTrayState(LOW);
  followRoute(routeToTable1, route1Length);
  waitForTrayState(HIGH);
  followRoute(routeBackFromTable1, route1Length + 2);
}

void table1_task2() {
  followRoute(routeToTable1, route1Length);
  waitForTrayState(HIGH);
  followRoute(routeBackFromTable1, route1Length + 2);
}

void table2_task1() {
  waitForTrayState(LOW);
  followRoute(routeToTable2, route2Length);
  waitForTrayState(HIGH);
  followRoute(routeBackFromTable2, route2Length + 2);
}

void table2_task2() {
  followRoute(routeToTable2, route2Length);
  waitForTrayState(HIGH);
  followRoute(routeBackFromTable2, route2Length + 2);
}

void table3_task1() {
  waitForTrayState(LOW);
  followRoute(routeToTable3, route3Length);
  waitForTrayState(HIGH);
  followRoute(routeBackFromTable3, route3Length + 2);
}

void table4_task1() {
  waitForTrayState(LOW);
  followRoute(routeToTable4, route4Length);
  waitForTrayState(HIGH);
  followRoute(routeBackFromTable4, route4Length + 2);
}

void table5_task1() {
  waitForTrayState(LOW);
  followRoute(routeToTable5, route5Length);
  waitForTrayState(HIGH);
  followRoute(routeBackFromTable5, route5Length + 2);
}

void table6_task1() {
  waitForTrayState(LOW);
  followRoute(routeToTable6, route6Length);
  waitForTrayState(HIGH);
  followRoute(routeBackFromTable6, route6Length + 2);
}

void followRoute(String route[], int length) {
  for (int i = 0; i < length; i++) {
    receiveTasks();
    char a = route[i][0];
    int v = route[i].substring(1).toInt();
    if (a == 'f') forward(v);
    if (a == 'r') rotate_right(v);
    if (a == 'l') rotate_left(v);
    delay(200);
  }
}

// формируем маршрут обратно
void toBase(String original[], String reversed[], int length) {
  reversed[0] = "r180";
  reversed[length - 1] = "r180";
  for (int i = 0; i < length; i++) {
    String cmd = original[length - 1 - i];
    char a = cmd[0];
    int v = cmd.substring(1).toInt();
    if (a == 'f') reversed[i + 1] = "f" + String(v);
    if (a == 'r') reversed[i + 1] = "l" + String(v);
    if (a == 'l') reversed[i + 1] = "r" + String(v);
  }
}

void addTask(unsigned int packet) {
  taskQueue[pointer_received] = packet;
  pointer_received = (pointer_received + 1) % queueSize;
}

bool queueEmpty() {
  return pointer_completed == pointer_received;
}

unsigned int getNextTask() {
  unsigned int t = taskQueue[pointer_completed];
  pointer_completed = (pointer_completed + 1) % queueSize;
  return t;
}
