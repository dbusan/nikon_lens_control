#ifndef _CONSTANTS_H_
#define _CONSTANTS_H_

// known commands
#define CMD_GET_INFO      0x28
#define CMD_FOCUS_INFO    0x26
#define CMD_START_FOCUS   0xE0
#define CMD_SET_APERTURE  0xDA

// test command
#define CMD_AF_START    0x32
#define CMD_AF_STOP     0x33
#define NR_BYTES_OUT_AF 3


#define CMD_LENS_CAPABILITIES       0x40
#define CMD_EXTRA_LENS_CAPABILITIES 0x41


// lookup table for aperture values
// 105mm f/1.4 has 22 aperture steps
// lowest aperture is f/1.4 highest is f/16
u8 aperture_floats[22][2] = {
  {1 , 4},
  {1 , 6},
  {1 , 8},
  {2 , 0},
  {2 , 2},
  {2 , 5},
  {2 , 8},
  {3 , 2},
  {3 , 5},
  {4 , 0},
  {4 , 5},
  {5 , 0},
  {5 , 6},
  {6 , 3},
  {7 , 1},
  {8 , 0},
  {9 , 0},
  {10, 0},
  {11, 0},
  {13, 0},
  {14, 0},
  {16, 0}
};

u8 aperture_lookup[22][2] = {
  {0x00, 0x15},
  {0x04, 0x1B},
  {0x08, 0x1B},
  {0x0C, 0x1B},
  {0x10, 0x1B},
  {0x14, 0x1B},
  {0x18, 0x1B},
  {0x1C, 0x1B},
  {0x20, 0x1B},
  {0x24, 0x1B},
  {0x28, 0x1B},
  {0x2C, 0x1B},
  {0x30, 0x1B},
  {0x34, 0x1B},
  {0x38, 0x1B},
  {0x3C, 0x1B},
  {0x40, 0x1B},
  {0x44, 0x1B},
  {0x48, 0x1B},
  {0x4C, 0x1B},
  {0x50, 0x1B},
  {0x54, 0x1B}
};


void PrintHex8(uint8_t data)
{
  if (data < 16) {
    Serial.print("0");
  };
  Serial.print(data, HEX);
}

class Cmd
{
  public:

    int cmd_byte;
    int nr_bytes_out;
    int nr_bytes_in;

    Cmd(int _bytes_out, int _bytes_in)
    {
      nr_bytes_out = _bytes_out;
      nr_bytes_in = _bytes_in;
    }

};

#endif //_CONSTANTS_H_
