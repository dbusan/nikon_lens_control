#ifndef COMMAND_H
#define COMMAND_H

enum CmdType {
  FOCUS, APERTURE, INVALID = -1
};

class Command 
{
  public:
  int focusSteps;
  lens::ApertureValue aperture;
  
  Command()
  {
    focusSteps = 0;
    aperture = lens::F1_4;
    
    _cmd = INVALID;
  }
  
  Command(
    CmdType cmd,
    long int m_changeValue) 
  {
    _cmd = cmd;
    if (_cmd == FOCUS)
    {
      
      if (m_changeValue > 12000) m_changeValue = 12000;
      if (m_changeValue < -12000) m_changeValue = -12000;
      
      focusSteps = (int)m_changeValue;
      aperture = 0;
    }
    else if (_cmd == APERTURE)
    {
      if (m_changeValue > lens::F16_0) m_changeValue = lens::F16_0;
      if (m_changeValue < lens::F1_4) m_changeValue = lens::F1_4;
      
      aperture = m_changeValue;
      focusSteps = 0;
    }
    else
    {
      _cmd = INVALID;
      focusSteps = 0;
      aperture = lens::F1_4;
    }
  }

  CmdType getCommandType() {return _cmd; };
  
  private:
    CmdType _cmd;
};

#endif // include guard
