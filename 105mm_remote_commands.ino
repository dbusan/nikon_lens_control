
#include <NikonLens.h>
#include <numeric.h>
#include "constants.h"



using namespace lens;

#define INVALID_CMD_TYPE 0
#define SEND_CMD_TYPE 1
#define RECV_CMD_TYPE 2
#define SENDRECV_CMD_TYPE 3
#define SPEC_CMD_TYPE 4
#define FOCUS_CMD_TYPE 5
#define APERTURE_CMD_TYPE 5

#define MIN_FOCUS_DIRN -1
#define MAX_FOCUS_DIRN 1

#define HS_PIN_IN 3
#define HS_PIN_OUT 4

uint8_t in_buffer[55];
uint8_t out_buffer[10];

uint8_t serial_in[255];

NikonLens_Class::tResultCode result;

String inputString     = "";
boolean stringComplete = false;

void setup() {
  Serial.begin(9600);

  delay(1000);

  NikonLens.begin();

//  init_lens();
//
//  // get basic information
//  result =
    NikonLens.sendCommand(CMD_GET_INFO, 44, in_buffer, 0, out_buffer);
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
//        loop_main_cmds();
          NikonLens.initLens();
      }
      else if (HexCmd == 0xCC)
      {
        step_thru_focus();
      }

      else if (HexCmd == 0xAC)
      {
//        aperture_sequence();
        int aperture_value_index = n_bytes_in;

        NikonLens.sendCommand(CMD_SET_APERTURE, 0, in_buffer, 2, aperture_lookup[aperture_value_index]);
        Serial.print("Aperture Payload ");
        for (u8 i = 0; i < 2; i++)
        {
          PrintHex8(aperture_lookup[aperture_value_index][i]); Serial.print(" ");
        }
        Serial.println();

        NikonLens.sendCommand(CMD_GET_INFO, 44, in_buffer, 0, out_buffer);
      }

      else if (HexCmd == 0x40)
      {
        high_cmds();
      }

      // FOCUS COMMAND
      // n_bytes_in contains the number of steps to step the focus by
      // the rough calculation to convert number of steps to hex byte packet is:
      // for min focus direction: packets = nr_steps >> 1;
      // for infinity focus dirn: packets = nr_steps >> 1 then set BIT 1 to 1
      // then the payload is as follows:
      // 0x0E 0x00 LSB MSB where LSB and MSB are the corresponding bytes from the calculation
      //
      else if (HexCmd == 0xFC)
      {
        int n_steps = n_bytes_in;

        if (n_bytes_out == MIN_FOCUS_DIRN)
        {
          result =
            NikonLens.sendCommand(CMD_GET_INFO, 44, in_buffer, 0, out_buffer);

          Serial.print("Minimum Focus Direction. N Steps: "); Serial.print(n_steps); Serial.println();

          u8 focus_buffer[4] = {0x0E, 0x00, 0xC0, 0x01};

          // 0x01C0
          // compute step command byte
          int byte_step_cmd = n_steps >> 1;

          // update output buffer: set LSB then MSB

          // LSB
          focus_buffer[2]  = (byte_step_cmd & 0xFF);
          // MSB
          focus_buffer[3]  = (byte_step_cmd >> 8);

          Serial.print("CMD payload: ");

          for (u8 i = 0; i < 4; i++)
          {
            PrintHex8(focus_buffer[i]); Serial.print(" ");
          }
          Serial.println();

          result =
            NikonLens.sendCommand(CMD_FOCUS_INFO, 8, in_buffer, 0, out_buffer);
          // Serial.print(in_buffer);
          result =
            NikonLens.sendCommand(0xE0, 0, in_buffer, 4, focus_buffer);

        }
        else if (n_bytes_out == MAX_FOCUS_DIRN)
        {
          Serial.print("INF Focus Direction. N Steps: "); Serial.print(n_steps); Serial.println();

          result =
            NikonLens.sendCommand(CMD_GET_INFO, 44, in_buffer, 0, out_buffer);

          u8 focus_buffer[4] = {0x0E, 0x00, 0xC4, 0x81};

          // compute step command byte
          int byte_step_cmd = n_steps >> 1;

          // update output buffer: set LSB then MSB

          // LSB
          focus_buffer[2]  = (byte_step_cmd & 0xFF);
          // MSB
          focus_buffer[3]  = (byte_step_cmd >> 8);

          // FLIP BIT 1 of MSB to change direction to inf focus
          focus_buffer[3] |= (1 << 7);

          Serial.print("CMD payload: ");

          for (u8 i = 0; i < 4; i++)
          {
            PrintHex8(focus_buffer[i]); Serial.print(" ");
          }
          Serial.println();


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
    else if (*ext_cmd_byte == 0xAC)
    {
      // reuses ext_nr_bytes_in for the aperture value 
      cmd_type = SPEC_CMD_TYPE;

      // parse string and save nr of steps
      int aperture_integer, aperture_decimal = 0, n_read;
      n_read = sscanf(input_string.c_str() + (offset), "%d.%d", &aperture_integer, &aperture_decimal);
      
//      if (n_read == 1)
//      {
//        Serial.print(aperture_integer); Serial.print(" aperture\n");
//      }
//      else if (n_read == 2)
//      {
//        Serial.print(aperture_integer); Serial.print("."); Serial.println(aperture_decimal);
//      }

      // iterate the lookup table coz i'm lazy
      int index = -1;
      Serial.print(" Aperture Integer: "); Serial.println(aperture_integer);
      Serial.print(" Aperture Decimal: "); Serial.println(aperture_decimal);
      for (u8 i = 0; i < 22; i++)
      {
        
        if (aperture_integer == aperture_floats[i][0])
        {
          if (aperture_decimal == aperture_floats[i][1])
          {
            Serial.print("Found aperture:"); Serial.println(String(aperture_floats[i][0]) + "." + String(aperture_floats[i][1]));
            index = i;
            break;
          }
        }
      }
      
      if (index < 0)
      {
        Serial.print("Did not find match. Setting aperture to default 1.4\n");
        index = 0;
        
      }
      *ext_nr_bytes_in = index;

      // find aperture in table
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

void step_thru_focus()
{
  // go to max focus
  Serial.print("Stepping thru focus steps from min to max\n");
  NikonLens.driveFocus(12000);
  delay(1500);
  NikonLens.printInputBuffer();
  // then step from min to max in 600 step increments
  for (u8 i = 0; i < 15; i++)
  {
    NikonLens.driveFocus(-600);
    delay(500);
    NikonLens.printInputBuffer();
    delay(800);
    
  }
  
}

void init_lens()
{
  // cmd byte 0x40. receive 7 bytes send 2 bytes
  u8 _cmd_byte = 0x40;
  u8 comms_init[2] = {0x02, 0x21};
  // send byte
  NikonLens.sendCommand(_cmd_byte, 7, in_buffer, 2, comms_init);

  // print camera response
  Serial.print("1. 0x40 - ");
  print_in_raw(7);

  // recv 2 bytes from 0x41
  _cmd_byte = 0x41;
  NikonLens.sendCommand(_cmd_byte, 2, in_buffer, 0, out_buffer);

  // print response from lens
  Serial.print("2. 0x41 - ");
  print_in_raw(2);

  _cmd_byte = 0x40;
  NikonLens.sendCommand(_cmd_byte, 7, in_buffer, 2, comms_init);

  // print lens response once again.
  // should be at 153.6 khz now
  Serial.print("3. 0x40 - ");
  print_in_raw(7);

  // get full lens info now
  _cmd_byte = 0x28;
  NikonLens.sendCommand(_cmd_byte, 44, in_buffer, 0, out_buffer);
  // print result
  Serial.print("4. 0x28 - ");
  print_in_raw(44);
  delay(3);

  // get short info - heartbeat
  _cmd_byte = 0xE7;
  out_buffer[0] = 0x51;
  NikonLens.sendCommand(_cmd_byte, 0, in_buffer, 1, out_buffer);
  // print result
  Serial.print("5. 0xE7 sent 0x51\n");
  delay(3);
  //  print_in_raw(1);
  //
  //  // get short info - heartbeat
  _cmd_byte = 0xC2;
  NikonLens.sendCommand(_cmd_byte, 4, in_buffer, 0, out_buffer);
  // print result
  Serial.print("6. 0xC2 - ");
  print_in_raw(4);
  delay(3);
  //

  // SET APERTURE
  _cmd_byte = 0xDA;
  out_buffer[0] = 0xFF;
  out_buffer[1] = 0xFF;
  NikonLens.sendCommand(_cmd_byte, 0, in_buffer, 2, out_buffer);
  // print result
  Serial.print("7. 0xDA sent 0xFF 0xFF\n");
  delay(3);
  //  print_in_raw(1);

  for (u8 i = 0; i < 6; i++)
  {
    // 5th and 6th iteration the response should be different.
  }

  _cmd_byte = 0xC2;
  NikonLens.sendCommand(_cmd_byte, 4, in_buffer, 0, out_buffer);
  // print result
  Serial.print("8. 0xC2 - ");
  print_in_raw(4);

  // get short info - heartbeat
  _cmd_byte = 0xEA;
  out_buffer[0] = 0x03;
  NikonLens.sendCommand(_cmd_byte, 0, in_buffer, 1, out_buffer);
  // print result
  Serial.print("9. 0xEA sent 0x03\n");
  

}

void print_in_raw(int n_bytes)
{
  for (int i = 0; i < n_bytes; i++)
  {
    PrintHex8(in_buffer[i]);
    Serial.print(" ");
  }
  //  Serial.println("\nRepeated without spacing");
  //
    for (int i = 0; i < n_bytes; i++)
    {
      PrintHex8(in_buffer[i]);
    }
  // Serial.print("\nStatus: "); Serial.println(result == 0 ? "SUCCESS" : "FAIL");
  Serial.println();
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
  // dc 2
  result =
    NikonLens.sendCommand(0xDC, 2, in_buffer, 0, out_buffer);
  delay(5);
  // c2 recv 4
  result =
    NikonLens.sendCommand(0xC2, 4, in_buffer, 0, out_buffer);
  // print_in_raw(4);
  delay(5);
  
  // e7 recv(send?) 1 - 0x51
  out_buffer[0] = 0x51;
  result =
    NikonLens.sendCommand(0xE7, 0, in_buffer, 1, out_buffer);
  
  delay(5);
  // ea recv(send?) 1 - 0x03
  out_buffer[0] = 0x03;
  result =
    NikonLens.sendCommand(0xEA, 0, in_buffer, 1, out_buffer);
  delay(5);
  // 3 dc commands
  //  for (int i = 0; i < 3; i++)
  //  {
  //    result =
  //      NikonLens.sendCommand(0xDC, 2, in_buffer, 0, out_buffer);
  //  }

  // da send 0c1b
  out_buffer[0] = 0x24;
  out_buffer[1] = 0x21;
  result =
    NikonLens.sendCommand(0xDA, 0, in_buffer, 2, out_buffer);

  Serial.print("Done Aperture:");

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
  for (u8 i = 64; i < 66; i++)
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
