#include <NikonLens.h>
#include <numeric.h>
#include "constants.h"

using namespace lain;

#define INVALID_CMD_TYPE 0
#define SEND_CMD_TYPE 1
#define RECV_CMD_TYPE 2
#define SENDRECV_CMD_TYPE 3


#define HS_PIN_IN 3
#define HS_PIN_OUT 4

uint8_t in_buffer[55];
uint8_t out_buffer[10];

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
    int HexCmd, n_bytes_out = 0, n_bytes_in = 0;

    // DEBUG
    //    Serial.print("Command:"); Serial.print(HexInt, HEX); Serial.print("; Bytes to receive: "); Serial.println(n_bytes);


    int parse_result = parse_input_string(inputString, &HexCmd, &n_bytes_out, &n_bytes_in);

    // output buffer was updated
    if (parse_result == SEND_CMD_TYPE)
    {
      Serial.print(" *** SEND PAYLOAD *** \n");
      result =
        NikonLens.sendCommand((u8)HexCmd, n_bytes_in, in_buffer, n_bytes_out, out_buffer);
      Serial.print("Sent "); PrintHex8((u8)HexCmd); Serial.print(" with above payload.\n");
    }
    else if (parse_result == RECV_CMD_TYPE)
    {
      Serial.print("CMD: "); PrintHex8(HexCmd); Serial.print("\n");
      result =
        NikonLens.sendCommand(HexCmd, n_bytes_in, in_buffer, 0, out_buffer);

      // print results
      print_in_raw(n_bytes_in);
    }
    else if (parse_result == SENDRECV_CMD_TYPE)
    {
      
    }
    else
    {
      Serial.print("Command type invalid. Not sending\n");
    }

    //
    //    if (cmd_is_known(HexInt))
    //    {
    //      int nr_bytes_recv = 0;
    //      int nr_bytes_sent = 0;
    //      if (HexInt == 0x69)
    //      {
    //        loop_main_cmds();
    //      }
    //      else if (HexInt == 0xFC)
    //      {
    //        get_focus_posn();
    //      }
    //      else if (HexInt == 0xCD)
    //      {
    //        send_22_23();
    //      }
    //      else {
    //
    //        if (n_bytes == 0) n_bytes = nr_bytes_recv;
    //        result =
    //          NikonLens.sendCommand((u8)HexInt, n_bytes, in_buffer, nr_bytes_sent, out_buffer);
    //
    //        //        print_in_buffer(n_bytes);
    //        print_in_raw(n_bytes);
    //
    //      }
    //    }
    //    else
    //    {
    //      Serial.print("Unknown Command: "); Serial.println(HexInt, HEX);
    //    }
    // clear input string
    inputString = "";
    stringComplete = false;

    //    Serial.print("\nEnter command in format: [HEX CMD] [N_BYTES_TO_SEND]\n");
  }
}


/** *** PROCESSING OF INPUT STRING *** **/
/*
   Input string format is:

   send cmd_byte n_bytes_to_send Byte1 Byte2 Byte3 ... ByteN (Bytes are in HEX)
   recv cmd_byte n_bytes_to_receive
   srcv cmd_byte n_bytes_to_receive n_bytes_to_send Byte1 Byte2 Byte3 ... ByteN (Bytes are in HEX)

   returns command type: SEND_CMD_TYPE or RECV_CMD_TYPE, SENDRECV_CMD_TYPE or INVALID_CMD_TYPE

   also updates output_buffer with the send command payload bytes
*/

int parse_input_string(String input_string, int *ext_cmd_byte, int *ext_nr_bytes_out, int *ext_nr_bytes_in)
{
  char cmd[5];
  char cmd_byte[3];
  char payload[10][3];
  int nr_bytes_out, nr_bytes_in;
  int offset;

  // return value - command type
  int cmd_type = INVALID_CMD_TYPE;

  // process the input string
  Serial.println(input_string);
  if (sscanf(input_string.c_str(), "%4s %2s %n", cmd, cmd_byte, &offset) != 2)
  {
    Serial.println("ERROR: Invalid input string.");
    return INVALID_CMD_TYPE;
  }

  // create c++ string type to compare
  String _cmd = cmd;
  *ext_cmd_byte = (int)strtol(cmd_byte, 0, 16);

  // if send data commmand is found, get payload bytes ready
  if (_cmd == "SEND")
  {
    cmd_type = SEND_CMD_TYPE;
    Serial.print("SEND CMD. PAYLOAD: ");

    int secondary_offset;
    sscanf(input_string.c_str() + offset, "%d %n", &nr_bytes_out, &secondary_offset);

    // check nr of payload bytes
    if (nr_bytes_out > 10) nr_bytes_out = 10;
    if (nr_bytes_out < 0) nr_bytes_out = 0;

    *ext_nr_bytes_out = nr_bytes_out;
    *ext_nr_bytes_in = 0;

    for (int i = 0; i < nr_bytes_out; i++)
    {
      sscanf(input_string.c_str() + offset + secondary_offset + (2 * sizeof(char) * i), "%2s", payload[i]);
      out_buffer[i] = (int)strtol(payload[i], 0, 16);
      PrintHex8(out_buffer[i]); Serial.print(" ");
    }
    Serial.println();
  }
  // receive bytes from camera
  else if (_cmd == "RECV")
  {
    Serial.print("RECEIVING BYTES\n");
    cmd_type = RECV_CMD_TYPE;

    sscanf(input_string.c_str() + offset, "%d", &nr_bytes_in);
    *ext_nr_bytes_out = 0;

    if (nr_bytes_in > 50) nr_bytes_in = 50;
    if (nr_bytes_in < 0) nr_bytes_in = 0;
    
    *ext_nr_bytes_in = nr_bytes_in;
  }
  else if (_cmd == "SRCV")
  {
    Serial.print("Sending then receiving.\n");
    cmd_type = SENDRECV_CMD_TYPE;

    int secondary_offset;
    sscanf(input_string.c_str() + offset, "%d %d %n", &nr_bytes_out, &nr_bytes_in, &secondary_offset);

    // check nr of payload bytes
    if (nr_bytes_out > 10) nr_bytes_out = 10;
    if (nr_bytes_out < 0) nr_bytes_out = 0;

    if (nr_bytes_in > 50) nr_bytes_in = 50;
    if (nr_bytes_in < 0) nr_bytes_in = 0;

    *ext_nr_bytes_out = nr_bytes_out;
    *ext_nr_bytes_in = nr_bytes_in;

    Serial.print("Payload: ");
    for (int i = 0; i < nr_bytes_out; i++)
    {
      sscanf(input_string.c_str() + offset + secondary_offset + (2 * sizeof(char) * i), "%2s", payload[i]);
      out_buffer[i] = (int)strtol(payload[i], 0, 16);
      PrintHex8(out_buffer[i]); Serial.print(" ");
    }
    
    Serial.println();
  }


  return cmd_type;
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

/** Serial Event
   Important
*/
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
