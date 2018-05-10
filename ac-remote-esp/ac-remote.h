#undef HAS_AVR_INTERRUPT_H
#include <IRremoteESP8266.h>
#include <IRsend.h>

#define PULSES 131
#define ENCASING_PULSES 3
#define BITS ((PULSES - ENCASING_PULSES) / 2)
#define NIBBLES (BITS / 4)

#define MODE_START 9
#define MODE_BITS 3
#define FAN_POWER_START 13
#define FAN_POWER_BITS 3
#define TEMP_START 18
#define TEMP_BITS 6
#define ON_OFF_START 45
#define ON_OFF_BITS 4
#define SWEEPING_START 55
#define SWEEPING_BITS 2
#define CRC_START 57
#define CRC_BITS 8

#define BASE_CODE "710000000100011000111110000000000000000000000110000000000100110006"

class ACRemote {
private:
  String code = BASE_CODE;
  uint16_t pulse[PULSES];
  unsigned int nibbles[NIBBLES];
  boolean hasChanged = false;
  IRsend* irsend;

  unsigned int* updatePulse()
  {
    int pulseI = 0;
    for (int i = 0; i < this->code.length(); i++) {
      switch (this->code[i]) {
        case '7':
          this->pulse[pulseI] = 7900;
          this->pulse[pulseI+1] = 4300;
          pulseI += 2;
          break;
        case '6':
          this->pulse[pulseI] = 1600;
          pulseI += 1;
          break;
        case '1':
          this->pulse[pulseI] = 1750;
          this->pulse[pulseI+1] = 600;
          pulseI += 2;
          break;
        case '0':
          this->pulse[pulseI] = 600;
          this->pulse[pulseI+1] = 1750;
          pulseI += 2;
          break;
      }
    }
  }

  void updateNibbles()
  {
    for (int nibbleI = 0; nibbleI < NIBBLES; nibbleI++) {
      this->nibbles[nibbleI] = 0;
      
      for (int bitI = 1 + nibbleI*4 + 3; bitI >= 1 + nibbleI*4 ; bitI--) {
          this->nibbles[nibbleI] = this->nibbles[nibbleI] << 1;
          this->nibbles[nibbleI] += this->code[bitI] == '0' ? 0 : 1;
      }
    }
  }

  void updateCode(unsigned int value, int start, int bits) 
  {
    for (int i = 0; i < bits; i++) {
      this->code[start + i] = (value & 0x1) == 0 ? '0' : '1';
      value = value >> 1;
    }

    this->hasChanged = true;
  }

  unsigned int calcCrc()
  {
    unsigned int crc = 0;
    
    // Last 2 nibbles are the crc
    for (int i = 0; i < NIBBLES - 2; i++) {
      crc += this->nibbles[i];
      crc &= 0xFF;
    }
  
    return crc;
  }

  void updateCrc() 
  {
     unsigned int crc = this->calcCrc();
     
     this->updateCode(crc, CRC_START, CRC_BITS);
  }

  void sendCode() 
  {
    Serial.println("Sending code");
    this->updateNibbles();
    this->updateCrc();
    this->updatePulse();
  
    for (int i = 0; i < 4; i++) {
      irsend->sendRaw(this->pulse, sizeof(this->pulse) / sizeof(this->pulse[0]), 38);
      delay(40);
    }  
  }

public:
  ACRemote(int sendPin) {
    irsend = new IRsend(sendPin);
    irsend->begin();
  }
  
  void sendCodeIfNeeded()
  {
    if (this->hasChanged) {
      sendCode();
      this->hasChanged = false;
    }
  }

  void updateOnOff(boolean isOn) 
  {
    unsigned int onOff = isOn ? 0x3 : 0xC;
    
    this->updateCode(onOff, ON_OFF_START, ON_OFF_BITS);
  }

  void updateTemp(unsigned int temp) 
  {
    temp = temp & 0x3F;
  
    this->updateCode(temp, TEMP_START, TEMP_BITS);
  }
};
