//#define __ASSERT_USE_STDERR
//#include <assert.h>

#include <NikonLens.h>
#include "command.h"
#include <numeric.h>


String inputString     = "";
boolean stringComplete = false;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);

  delay(500);

  lens::NikonLens.begin();

}

void loop() {
  // put your main code here, to run repeatedly:
  serialEvent();

  Command cmd;
  if (stringComplete)
  {
    parse_input_string(inputString, &cmd);

    //    Serial.print(" " + String(cmd.getCommandType()) + " " + String(cmd.changeValue) + "\n");

    if (cmd.getCommandType() == APERTURE)
    {
      Serial.println("Drive aperture.");
      lens::NikonLens.setAperture(cmd.aperture);
    }
    else if (cmd.getCommandType() == FOCUS)
    {
      Serial.println("Drive Focus " + String(cmd.focusSteps) + " steps towards "
                     + (cmd.focusSteps < 0 ? "min focus" : "infinity focus."));
      lens::NikonLens.driveFocus(cmd.focusSteps);
    }
    inputString = "";
    stringComplete = false;
  }
}

// input format: FC step
void parse_input_string(String input_string, Command *cmd)
{
  long int change_val = 0;
  char _cmd_str[3];
  CmdType cmd_type = INVALID;

  if (sscanf(input_string.c_str(), "%2s %ld", &_cmd_str, &change_val) != 2)
  {
    Serial.println("INVALID COMMAND");
    return Command(INVALID, 0);
  }

  String cmd_str = _cmd_str;
  if (cmd_str == "FC")
  {
    Serial.println("FOCUS COMMAND");
    cmd_type = FOCUS;
  }
  else if (cmd_str == "AP")
  {
    Serial.println("APERTURE COMMAND");
    cmd_type = APERTURE;
  }
  else if (cmd_str == "IF")
  {
    // Serial.println("GET FOCUS POSN");
    lens::NikonLens.initLens();
  }
  else
  {
    Serial.println("UNKNOWN COMMAND TYPE: " + cmd_str);
  }

  *cmd = Command(cmd_type, change_val);

  //  assert(cmd->changeValue >= -12000);
  //  assert(cmd->changeValue <= 12000);
}

/** Serial Event */
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


//// handle diagnostic informations given by assertion and abort program execution:
//void __assert(const char *__func, const char *__file, int __lineno, const char *__sexp) {
//    // transmit diagnostic informations through serial link.
//    Serial.println(__func);
//    Serial.println(__file);
//    Serial.println(__lineno, DEC);
//    Serial.println(__sexp);
//    Serial.flush();
//    // abort program execution.
//    abort();
//}
