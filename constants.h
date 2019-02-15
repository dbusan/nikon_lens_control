#ifndef _CONSTANTS_H_
#define _CONSTANTS_H_


// known commands
#define CMD_GET_INFO 0x27
#define NR_GET_INFO_BYTES 40
#define CMD_GET_INFO_2 0x28

// test command
#define CMD_AF_START 0x32
#define CMD_AF_STOP 0x33
#define NR_BYTES_OUT_AF 3


#define CMD_LENS_CAPABILITIES 0x40
#define CMD_EXTRA_LENS_CAPABILITIES 0x41

/**
 * Function based on robert.bares' post on arduino forum
 * https://forum.arduino.cc/index.php?topic=38107.0
 * does not work.
 */
void PrintHex8_OLD(uint8_t *data, uint8_t length) // prints 8-bit data in hex
{
     char tmp[length*2+1];
     byte first;
     byte second;
     for (int i=0; i<length; i++) {
           first = (data[i] >> 4) & 0x0f;
           second = data[i] & 0x0f;
           // base for converting single digit numbers to ASCII is 48
           // base for 10-16 to become lower-case characters a-f is 87
           // note: difference is 39
           tmp[i*2] = first+48;
           tmp[i*2+1] = second+48;
           if (first > 9) tmp[i*2] += 39;
           if (second > 9) tmp[i*2+1] += 39;
     }
     tmp[length*2] = 0;
//     Serial.println("0x" + String(tmp));
     Serial.print(String(tmp));
}

void PrintHex8(uint8_t data)
{
  if (data < 16) {Serial.print("0");};
  Serial.print(data, HEX);
}
#endif //_CONSTANTS_H_
