#include <NikonLens.h>
#include <numeric.h>
#include "constants.h"

using namespace lain;

#define HS_PIN_IN 3
#define HS_PIN_OUT 4

uint8_t in_buffer[55];
uint8_t out_buffer[8];

uint8_t serial_in[255];

tNikonLens::tResultCode result;

String inputString     = "";
boolean stringComplete = false;

void setup() {
  Serial.begin(9600);

  delay(2);

  NikonLens.begin(HS_PIN_IN, HS_PIN_OUT);

  delay(500);
  Serial.print("Enter command in format: [HEX CMD] [N_BYTES_TO_SEND]\n");
}

int i = 0;
void loop() {
  // put your main code here, to run repeatedly:
  serialEvent();
  // get camera info

  if (stringComplete)
  {
    //int HexInt = inputString.toInt();
    //    int HexInt = (int) strtol(inputString.c_str(), 0, 16);
    int HexInt, n_bytes = 0;
    char HexString[3];

    sscanf(inputString.c_str(), "%2s %d", &HexString, &n_bytes);
    HexInt = (int)strtol(HexString, 0, 16);

    // DEBUG
    Serial.print("Command:"); Serial.print(HexInt, HEX); Serial.print("; Bytes to receive: "); Serial.println(n_bytes);


    if (cmd_is_known(HexInt))
    {
      int nr_bytes_recv = 0;
      int nr_bytes_sent = 0;
      if (HexInt == 0x69)
      {
        loop_main_cmds();
      }
      else if (HexInt == 0xFC)
      {
        get_focus_posn();
      }
      else if (HexInt == 0xCD)
      {
        send_22_23();
      }
      else {

        if (n_bytes == 0) n_bytes = nr_bytes_recv;
        result =
          NikonLens.sendCommand((u8)HexInt, n_bytes, in_buffer, nr_bytes_sent, out_buffer);

        //        print_in_buffer(n_bytes);
        print_in_raw(n_bytes);

      }
    }
    else
    {
      Serial.print("Unknown Command: "); Serial.println(HexInt, HEX);
    }
    // clear input string
    inputString = "";
    stringComplete = false;
    Serial.print("\nEnter command in format: [HEX CMD] [N_BYTES_TO_SEND]\n");
  }
}

/**
   triggered by command 0x69

*/
void loop_main_cmds()
{
  Serial.println("IN1 IN2 OUT1 OUT2");
  for (u8 i = 0; i < 255; i++)
  {
    result =
      NikonLens.sendCommand(i, 8, in_buffer, 0, out_buffer);
    Serial.print("CMD: "); Serial.print(i, HEX); Serial.print("; ");
    for (u8 index = 0; index < 8; index++)
    {
      PrintHex8(in_buffer[index]);
    }
    Serial.print("\n");
    //    Serial.print(out_buffer[0], HEX); Serial.print(" ");
    //    Serial.print(out_buffer[1], HEX); Serial.print("\n");
  }
}

void send_22_23()
{

  result =
    NikonLens.sendCommand(34, 20, in_buffer, 0, out_buffer);
  Serial.print("CMD: "); Serial.print(34, HEX); Serial.print("; ");
  for (u8 index = 0; index < 20; index++)
  {
    PrintHex8(in_buffer[index]);
  }
  Serial.print("\n");

  result =
    NikonLens.sendCommand(195, 20, in_buffer, 0, out_buffer);
  Serial.print("CMD: "); Serial.print(195, HEX); Serial.print("; ");
  for (u8 index = 0; index < 20; index++)
  {
    PrintHex8(in_buffer[index]);
  }
  Serial.print("\n");

}

void get_focus_posn()
{
  Serial.print("FOCUS POSITION: ");
  for (u8 i = 0; i < 16; i++)
  {
    result =
      NikonLens.sendCommand(i, 1, in_buffer, 0, out_buffer);
    for (u8 index = 0; index < 1; index++)
    {
      PrintHex8(in_buffer[index]);
    }

    //    Serial.print(out_buffer[0], HEX); Serial.print(" ");
    //    Serial.print(out_buffer[1], HEX); Serial.print("\n");
  }
  Serial.print("\n");
}

void loop_thru_cmds()
{
  int i, j, k;

  Serial.print("\n*** TESTING AF MODE ***\n");
  Serial.print("\nbyte 0 \tbyte 1\tbyte 2\n");
  for (i = 0; i + 10 < 255; i++)
  {
    out_buffer[0] = i;
    for (j = 0; j + 10 < 255; j++)
    {
      out_buffer[1] = j;
      for (k = 0; k + 10 < 255; k++)
      {
        out_buffer[2] = k;
        // call send_command here
        result =
          NikonLens.sendCommand(0x32, 0, in_buffer, 1, out_buffer);


      }
      // print current command byte
      PrintHex8(out_buffer[0]); Serial.print("\t");
      PrintHex8(out_buffer[1]); Serial.print("\t");
      PrintHex8(out_buffer[2]); Serial.print("\n");

    }
  }
  Serial.print("\nbyte 0 \tbyte 1\tbyte 2\n");
}

bool cmd_is_known(byte in_cmd)
{
  //  return ((in_cmd == CMD_GET_INFO) ||
  //          (in_cmd == CMD_AF_START) ||
  //          (in_cmd == CMD_AF_STOP) ||
  //          (in_cmd == CMD_GET_INFO_2) ||
  //          (in_cmd == 0x22));

  return true;
}


void print_in_raw(int n_bytes)
{
  for (int i = 0; i < n_bytes; i++)
  {
    PrintHex8(in_buffer[i]);
    Serial.print(" ");
  }
  Serial.println("\nRepeated without spacing");

  for (int i = 0; i < n_bytes; i++)
  {
    PrintHex8(in_buffer[i]);
  }
  Serial.print("\nStatus: "); Serial.println(result == 0 ? "SUCCESS" : "FAIL"); Serial.println();
}

void serialEvent()
{
  while (Serial.available()) {
    char inChar = (char)Serial.read();
    if (inChar == '\n' or inChar == '\r') {
      stringComplete = true;
      if (inputString == "") inputString = "NULL";
      inputString.toUpperCase();
      continue;
    }
    inputString += inChar;
  }
}
