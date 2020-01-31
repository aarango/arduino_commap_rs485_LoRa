#include <Arduino.h>
#include <ModbusMaster.h>
#include <SoftwareSerial.h>
#include <SPI.h>
#include <LoRa.h>

#define MAX485_DE 2
#define MAX485_RE_NEG 2
bool state = true;

const char *ID_GW = "00";
const char *ID_STR = "03";
const char *READ_CMD = "030";
const char *WRITE_CMD = "031";

String incoming = "";
uint8_t contador = 0;
uint16_t timmer = 0;
int test = 1;
String rState1 = "on";
int parseOn = 0;

// comando paa escribir ON getResponseBuffer
const uint16_t cmd_engine_on[] = {0x01FE, 0x0000, 0x0001};
const uint16_t cmd_engine_off[] = {0x02FD, 0x0000, 0x0001};

const uint16_t reg_engine = 0x18D6;
int sendReader = 0;
double bat;
double g1;
double g2;
double g3;
double g4;
double g5;
double g6;
double tempr;
double oi = 0;
double starstatus = 0;
double timert = 0;
double breakste = 0;
double rpmstatus = 0;
double rpmstatusuno = 0;
double rpmstatusdos = 0;
double nstart = 0;
double hrma = 0;
double run1 = 0;
double run2 = 0;

int timmer2 = 100;

SoftwareSerial mySerial(10, 11);
ModbusMaster node;

void preTransmission()
{
  //digitalWrite(MAX485_RE_NEG, 1);
  digitalWrite(MAX485_DE, 1);
}

void postTransmission()
{
  //digitalWrite(MAX485_RE_NEG, 0);
  digitalWrite(MAX485_DE, 0);
}

void setup()
{
  //pinMode(MAX485_RE_NEG, OUTPUT);
  pinMode(MAX485_DE, OUTPUT);
  // Init in receive mode
  //digitalWrite(MAX485_RE_NEG, 0);
  digitalWrite(MAX485_DE, 0);

  // Modbus communication runs at 115200 baud

  mySerial.begin(9600);

  // Modbus slave ID 1
  node.begin(1, mySerial);
  // Callbacks allow us to configure the RS485 transceiver correctly
  node.preTransmission(preTransmission);
  node.postTransmission(postTransmission);

  LoRa.setPins(1, 4, 7);
  if (!LoRa.begin(433E6))
  {
    ////Serial.println("Starting LoRa failed!");
    while (1)
      ;
  }
  //LoRa.onReceive(onReceive);
  //LoRa.receive();
}

void loop()
{
  if (test)
  {
    incoming = "03100";
    if (incoming == (String(WRITE_CMD) + "00"))
    {
      Serial.println(String(WRITE_CMD) + "01");
    }
    test = 0;
    delay(1000);
  }
  readLora();
  if (parseOn)
  {
    parseOn = 0;
    if (incoming == READ_CMD)
    {
      sendReader = 1;
      for (int i = 0; i < 17; i++)
      {
        readRegs(i);
        readLora();
        actionLora();
      }
    }
    if (sendReader == 1)
    { // TENGO LA DUDA SI ESTO VA ABAJO
      LoRa.beginPacket();
      LoRa.print("0003," + String(bat) + "," + String(g1) + "," + String(g2) + "," + String(g3) + "," + String(g4) + "," + String(g5) + "," + String(g6) + "," + String(tempr) + "," + String(oi) + "," + String(starstatus) + "," + String(timert) + "," + String(breakste) + "," + String(rpmstatus) + "," + String(nstart) + "," + String(hrma) + "," + String(run1) + "," + String(rState1));
      LoRa.endPacket();
      sendReader = 0;
    }
    actionLora();
  }
}

void readLora()
{
  int packetSize = LoRa.parsePacket();
  if (packetSize)
  {
    parseOn = 1;
    incoming = "";
    contador = 0;

    while (LoRa.available())
    {

      incoming += (char)LoRa.read();
      if (contador == 2)
      {
        if (incoming == READ_CMD)
          break;
      }
      if (contador == 4)
      {
        if (incoming == (String(WRITE_CMD) + "01"))
          break;
        if (incoming == (String(WRITE_CMD) + "00"))
          break;
      }
      contador++;
    }
    Serial.println(incoming);
  }
}

void actionLora()
{
  if (incoming == (String(WRITE_CMD) + "00"))
  {
    Apagar(); // Aca se llama el comando para  Apagar  estan en el final
    rState1 = "off";
  }
  if (incoming == (String(WRITE_CMD) + "01"))
  {
    Encender(); // // Aca se llama el comando para Encender
    rState1 = "on";
  }
}

void readRegs(int p)
{
  if (p == 0)
    baterias();
  if (p == 1)
    gen1();
  if (p == 2)
    gen2();
  if (p == 3)
    gen3();
  if (p == 4)
    gen4();
  if (p == 5)
    gen5();
  if (p == 6)
    gen6();
  if (p == 7)
    temp();
  if (p == 8)
    oil();
  if (p == 9)
    started();
  if (p == 10)
    timertext();
  if (p == 11)
    breakestate();
  if (p == 12)
    rpm();
  if (p == 13)
    numerostart();
  if (p == 14)
    hrmantenimiento();
  if (p == 15)
    runhours1(); //frq
  if (p == 16)
    runhours2();

  delay(timmer2);
}

void baterias()
{
  uint8_t result;
  uint16_t data[6];
  result = node.readHoldingRegisters(50, 1);

  if (result == node.ku8MBSuccess)
  {
    Serial.print("Vbatt: ");
    //mySerial.println(node.getResponseBuffer(0));
    Serial.println(node.getResponseBuffer(0) / 10.0);
    //Serial.println(mySerial.read());
    bat = (node.getResponseBuffer(0) / 10.0);
  }
}

void gen1()
{
  uint8_t result;
  uint16_t data[6];
  // Read 16 registers starting at 0x3100)
  //Serial.println("intento gen1");
  result = node.readHoldingRegisters(00, 1);

  if (result == node.ku8MBSuccess)
  {
    Serial.print("Gen1 V L1-N: ");
    //mySerial.println(node.getResponseBuffer(0));
    Serial.println(node.getResponseBuffer(0));
    //Serial.println(mySerial.read());
    g1 = (node.getResponseBuffer(0));
  }
}

void gen2()
{
  uint8_t result;
  uint16_t data[6];
  // Read 16 registers starting at 0x3100)
  //Serial.println("intento gen2");
  result = node.readHoldingRegisters(01, 1);

  if (result == node.ku8MBSuccess)
  {
    Serial.print("Gen2 V L2-N: ");
    //mySerial.println(node.getResponseBuffer(0) / 10.0);
    Serial.println(node.getResponseBuffer(0));
    //Serial.println(mySerial.read());
    g2 = (node.getResponseBuffer(0));
  }
}

void gen3()
{
  uint8_t result;
  uint16_t data[6];
  // Read 16 registers starting at 0x3100)
  //Serial.println("intento gen3");
  result = node.readHoldingRegisters(02, 1);

  if (result == node.ku8MBSuccess)
  {
    Serial.print("Gen3 V L3-N: ");
    //mySerial.println(node.getResponseBuffer(0) / 10.0);
    Serial.println(node.getResponseBuffer(0));
    // Serial.println(mySerial.read());
    g3 = (node.getResponseBuffer(0));
  }
}

void gen4()
{
  uint8_t result;
  uint16_t data[6];
  // Read 16 registers starting at 0x3100)
  // Serial.println("intento gen4");
  result = node.readHoldingRegisters(04, 1);

  if (result == node.ku8MBSuccess)
  {
    Serial.print("Gen4 V L1-L2: ");
    //mySerial.println(node.getResponseBuffer(0) / 10.0);
    Serial.println(node.getResponseBuffer(0));
    // Serial.println(mySerial.read());
    g4 = (node.getResponseBuffer(0));
  }
}

void gen5()
{
  uint8_t result;
  uint16_t data[6];
  // Read 16 registers starting at 0x3100)
  // Serial.println("intento gen5");
  result = node.readHoldingRegisters(05, 1);

  if (result == node.ku8MBSuccess)
  {
    Serial.print("Gen5 V L2-L3: ");
    //mySerial.println(node.getResponseBuffer(0) / 10.0);
    Serial.println(node.getResponseBuffer(0));
    // Serial.println(mySerial.read());
    g5 = (node.getResponseBuffer(0));
  }
}

void gen6()
{
  uint8_t result;
  uint16_t data[6];
  // Read 16 registers starting at 0x3100)
  //Serial.println("intento gen6");
  result = node.readHoldingRegisters(06, 1);

  if (result == node.ku8MBSuccess)
  {
    Serial.print("Gen5 V L3-L1: ");
    //mySerial.println(node.getResponseBuffer(0) / 10.0);
    Serial.println(node.getResponseBuffer(0));
    // Serial.println(mySerial.read());
    g6 = (node.getResponseBuffer(0));
  }
}

void oil()
{
  uint8_t result;
  uint16_t data[6];
  // Read 16 registers starting at 0x3100)
  // Serial.println("intento oil");
  result = node.readHoldingRegisters(53, 1);

  if (result == node.ku8MBSuccess)
  {
    Serial.print("oil: ");
    //mySerial.println(node.getResponseBuffer(0) / 10.0);
    Serial.println(node.getResponseBuffer(0 / 100));
    // Serial.println(mySerial.read());
    oi = (node.getResponseBuffer(0 / 100));
  }
}

void temp()
{
  uint8_t
      result1;
  uint16_t data[6];
  // Read 16 registers starting at 0x3100)
  //Serial.println("intento temp");
  result1 = node.readHoldingRegisters(54, 1);

  if (result1 == node.ku8MBSuccess)
  {
    Serial.print("temp: ");
    //mySerial.println(node.getResponseBuffer(0) / 10.0);
    Serial.println(node.getResponseBuffer(0) / 100.0);
    //Serial.println(mySerial.read());
    tempr = (node.getResponseBuffer(0) / 100.0);
  }
}

void started() //27 off 23 on
{
  uint8_t result1;
  uint16_t data[6];
  // Read 16 registers starting at 0x3100)
  //Serial.println("intento started");
  result1 = node.readHoldingRegisters(70, 1);

  if (result1 == node.ku8MBSuccess)
  {
    Serial.print("started: ");
    //mySerial.println(node.getResponseBuffer(0));
    Serial.println(node.getResponseBuffer(0));
    //Serial.println(mySerial.read());
    starstatus = (node.getResponseBuffer(0));
  }
}

void timertext() // 3 y pasa a 4
{
  uint8_t result1;
  uint16_t data[6];
  // Read 16 registers starting at 0x3100)
  //Serial.println("intento timertext");
  result1 = node.readHoldingRegisters(72, 1);

  if (result1 == node.ku8MBSuccess)
  {
    Serial.print("timertext: ");
    // mySerial.println(node.getResponseBuffer(0));
    Serial.println(node.getResponseBuffer(0));
    // Serial.println(mySerial.read());
    timert = (node.getResponseBuffer(0));
  }
}

void numerostart()
{
  uint8_t result1;
  uint16_t data[6];
  // Read 16 registers starting at 0x3100)
  // Serial.println("intento numerostart");
  result1 = node.readHoldingRegisters(3004, 1);

  if (result1 == node.ku8MBSuccess)
  {
    Serial.print("numerostart: ");
    Serial.println(node.getResponseBuffer(0));
    nstart = (node.getResponseBuffer(0));
  }
}

void breakestate() // 35
{
  uint8_t result1;
  uint16_t data[6];
  // Read 16 registers starting at 0x3100)
  //Serial.println("intento breakestate");
  result1 = node.readHoldingRegisters(71, 1);

  if (result1 == node.ku8MBSuccess)
  {
    Serial.print("breakestate: ");
    //mySerial.println(node.getResponseBuffer(0));
    Serial.println(node.getResponseBuffer(0));
    //Serial.println(mySerial.read());
    breakste = (node.getResponseBuffer(0));
  }
}

void rpm()
{
  uint8_t result1;
  uint16_t data[6];
  // Read 16 registers starting at 0x3100)
  // Serial.println("intento rpm");
  result1 = node.readHoldingRegisters(10, 1);

  if (result1 == node.ku8MBSuccess)
  {
    Serial.print("rpm: ");
    //mySerial.println(node.getResponseBuffer(0));
    Serial.println(node.getResponseBuffer(0));
    //Serial.println(mySerial.read());
    rpmstatus = (node.getResponseBuffer(0));
  }
}

void hrmantenimiento()
{
  uint8_t result1;
  uint16_t data[6];
  // Read 16 registers starting at 0x3100)
  //Serial.println("intento hrmantenimiento");
  result1 = node.readHoldingRegisters(3003, 1);

  if (result1 == node.ku8MBSuccess)
  {
    Serial.print("hrmantenimiento: ");
    Serial.println(node.getResponseBuffer(0));
    hrma = (node.getResponseBuffer(0));
  }
}

void runhours1() // frq
{
  uint8_t result1;
  uint16_t data[6];
  // Read 16 registers starting at 0x3100)
  // Serial.println("intento runhours1");
  result1 = node.readHoldingRegisters(3030, 1);

  if (result1 == node.ku8MBSuccess)
  {
    Serial.print("Freq: ");
    Serial.println(node.getResponseBuffer(0));
    run1 = (node.getResponseBuffer(0));
  }
}

void runhours2()
{
  uint8_t result1;
  uint16_t data[6];
  // Read 16 registers starting at 0x3100)
  //Serial.println("intento iomai3");
  result1 = node.readHoldingRegisters(3001, 4);

  if (result1 == node.ku8MBSuccess)
  {
    Serial.print("runhours2: ");
    Serial.println(node.getResponseBuffer(0));
    run2 = (node.getResponseBuffer(0));
  }
}

void Encender()
{
  uint8_t result;
  node.setTransmitBuffer(0, cmd_engine_on[0]);
  node.setTransmitBuffer(1, cmd_engine_on[1]);
  node.setTransmitBuffer(2, cmd_engine_on[2]);
  result = node.writeMultipleRegisters(reg_engine, 3);
  if (result == node.ku8MBSuccess)
  {
    Serial.print("Encendido !! ");
  }
  else
  {
    Serial.print("fallo al encender !! ");
  }
}

void Apagar()
{
  uint8_t result;
  node.setTransmitBuffer(0, cmd_engine_off[0]);
  node.setTransmitBuffer(1, cmd_engine_off[1]);
  node.setTransmitBuffer(2, cmd_engine_off[2]);
  result = node.writeMultipleRegisters(reg_engine, 3);
  if (result == node.ku8MBSuccess)
  {
    Serial.print("Apagado !! ");
  }
  else
  {
    Serial.print("fallo al apagar !! ");
  }
}

//void onReceive(int packetSize) {
//
//    int i = 0;
//    byte id_read [4];
//    String incoming = "";                 // payload of packet
//
//    while (LoRa.available()) {            // can't use readString() in callback, so
//      incoming += (char)LoRa.read();      // add bytes one by one
//    }
//    if( incoming == "030"){
//      //Serial.println("enviar");
//      sendReader = 1;
//    }
//
//}
