#include <NikonLens.h>
#include <numeric.h>
#include "constants.h"

using namespace lain;

#define INVALID_CMD_TYPE 0
#define SEND_CMD_TYPE 1
#define RECV_CMD_TYPE 2
#define SENDRECV_CMD_TYPE 3
#define SPEC_CMD_TYPE 4
#define FOCUS_CMD_TYPE 5

#define MIN_FOCUS_DIRN -1
#define MAX_FOCUS_DIRN 1

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

  // get basic information
  result =
  NikonLens.sendCommand(CMD_GET_INFO_2, 44, in_buffer, 0, out_buffer);
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
      result =
        NikonLens.sendCommand(HexCmd, n_bytes_in, in_buffer, 0, out_buffer);

      // print results
      print_in_raw(n_bytes_in);
    }
    else if (parse_result == SENDRECV_CMD_TYPE)
    {
      result =
        NikonLens.sendCommand(HexCmd, n_bytes_in, in_buffer, n_bytes_out, out_buffer);
      // print results
      print_in_raw(n_bytes_in);

    }
    else if (parse_result == SPEC_CMD_TYPE)
    {
      if (HexCmd == 0x69) // 105 decimal is 69 hex
      {
        loop_main_cmds();
      }

      else if (HexCmd == 0x42)
      {
        aperture_sequence();
      }

      else if (HexCmd == 0x40)
      {
        high_cmds();
      }

      // FOCUS COMMAND
      else if (HexCmd == 0xFC)
      {
        //
        if (n_bytes_out == MIN_FOCUS_DIRN)
        {
          result =
  NikonLens.sendCommand(CMD_GET_INFO_2, 44, in_buffer, 0, out_buffer);
  
          Serial.print("Minimum Focus Direction\n");
          
          u8 focus_buffer[4] = {0x0E, 0x00, 0xC0, 0x01};
          result =
            NikonLens.sendCommand(CMD_FOCUS_INFO, 8, in_buffer, 0, out_buffer);
          // Serial.print(in_buffer);
          result =
            NikonLens.sendCommand(0xE0, 0, in_buffer, 4, focus_buffer);
          
        }
        else if (n_bytes_out == MAX_FOCUS_DIRN)
        {
          result =
  NikonLens.sendCommand(CMD_GET_INFO_2, 44, in_buffer, 0, out_buffer);
  
          u8 focus_buffer[4] = {0x0E, 0x00, 0xC4, 0x81};
          Serial.print("Infinity Focus Direction\n");
          
          result =
            NikonLens.sendCommand(CMD_FOCUS_INFO, 8, in_buffer, 0, out_buffer);
          // Serial.print(in_buffer);
          result =
            NikonLens.sendCommand(0xE0, 0, in_buffer, 4, focus_buffer);
        }
        else
        {
          // Serial.print("Unknown direction.");
        }

        // reset n_bytes_in to a more reasonable number
        n_bytes_in = 2;
        n_bytes_out = 0;
      }
    }
    else
    {
      Serial.print("Command type invalid. Not sending\n");
    }

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

String _dirn = "";

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
      sscanf(input_string.c_str() + offset + secondary_offset + (3 * sizeof(char) * i), "%2s", payload[i]);
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
  else if (_cmd == "SPEC")
  {
    
    if (*ext_cmd_byte == 0xFC)
    {
      char dirn[5];
      int n_steps_focus = 0;
      
      // reuses ext_nr_bytes_in for the number of steps to turn the focus ring
      cmd_type = SPEC_CMD_TYPE;

      // parse string and save nr of steps      
      sscanf(input_string.c_str() + offset, "%4s %d", dirn, &n_steps_focus);

      _dirn = dirn;

      if (n_steps_focus > 12000) n_steps_focus = 12000;
      if (n_steps_focus < 0)     n_steps_focus = 0;

      *ext_nr_bytes_in = n_steps_focus;
      
      // set direction
      if (_dirn == "MIN")
      {
        *ext_nr_bytes_out = MIN_FOCUS_DIRN;
      }
      else if (_dirn == "MAX")
      {
        *ext_nr_bytes_out = MAX_FOCUS_DIRN;
      }
      else
      {
        Serial.print("UNKNOWN DIRECTION. CANCELLING\n");
        *ext_nr_bytes_in = 0;
      }


    }
    else
    {
      Serial.print("Special cmd\n");
      *ext_cmd_byte = (int)strtol(cmd_byte, 0, 16);
      cmd_type = SPEC_CMD_TYPE;
    }
  }


  return cmd_type;
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



// OTHERS


void aperture_sequence()
{
  // c2 recv 4
  result =
    NikonLens.sendCommand(0xC2, 4, in_buffer, 0, out_buffer);
  // print_in_raw(4);

  // e7 recv(send?) 1 - 0x51
  out_buffer[0] = 0x51;
  result =
    NikonLens.sendCommand(0xE7, 0, in_buffer, 1, out_buffer);

  // ea recv(send?) 1 - 0x03
  out_buffer[0] = 0x03;
  result =
    NikonLens.sendCommand(0xEA, 0, in_buffer, 1, out_buffer);

  // 3 dc commands
  //  for (int i = 0; i < 3; i++)
  //  {
  //    result =
  //      NikonLens.sendCommand(0xDC, 2, in_buffer, 0, out_buffer);
  //  }

  // da send 0c1b
  out_buffer[0] = 0x54;
  out_buffer[1] = 0x1b;
  result =
    NikonLens.sendCommand(0xDA, 0, in_buffer, 2, out_buffer);

  Serial.print("Done Aperture:") ;

}


// SPEC 40 - turns focus ring in one way weirdly.
// recv b3 16
void high_cmds()
{
  Serial.println("IN1 IN2 OUT1 OUT2");
  for (u8 i = 178; i < 180; i++)
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
