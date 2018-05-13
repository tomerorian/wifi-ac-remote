#undef HAS_AVR_INTERRUPT_H
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include "storage.h"

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

#define STORAGE_CODE_ADDRESS 0

class ACRemote {
private:
  String code = BASE_CODE;
  uint16_t pulse[PULSES];
  unsigned int nibbles[NIBBLES];
  boolean has_changed = false;
  IRsend* irsend;
  Storage* storage;;

  void load_code()
  {
    this->storage->begin();
    String loadedCode = this->storage->load_string(STORAGE_CODE_ADDRESS);
    this->storage->end();

    if (loadedCode.length() == this->code.length()) {
      this->code = loadedCode; 
    }
  }

  void save_code()
  {
    this->storage->begin();
    this->storage->save_string(STORAGE_CODE_ADDRESS, this->code.length() + 3, this->code);
    this->storage->end();
  }

  unsigned int* update_pulse()
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

  void update_nibbles()
  {
    for (int nibbleI = 0; nibbleI < NIBBLES; nibbleI++) {
      this->nibbles[nibbleI] = 0;
      
      for (int bitI = 1 + nibbleI*4 + 3; bitI >= 1 + nibbleI*4 ; bitI--) {
          this->nibbles[nibbleI] = this->nibbles[nibbleI] << 1;
          this->nibbles[nibbleI] += this->code[bitI] == '0' ? 0 : 1;
      }
    }
  }

  void update_code(unsigned int value, int start, int bits) 
  {
    for (int i = 0; i < bits; i++) {
      this->code[start + i] = (value & 0x1) == 0 ? '0' : '1';
      value = value >> 1;
    }

    this->has_changed = true;

    this->save_code();
  }

  unsigned int read_code(int start, int bits)
  {
    unsigned int value = 0x0;
    
    for (int i = 0; i < bits; i++) {
      value += (this->code[start + i] == '0' ? 0 : 1) << i;
    }

    return value;
  }

  unsigned int calc_crc()
  {
    unsigned int crc = 0;
    
    // Last 2 nibbles are the crc
    for (int i = 0; i < NIBBLES - 2; i++) {
      crc += this->nibbles[i];
      crc &= 0xFF;
    }
  
    return crc;
  }

  void update_crc() 
  {
     unsigned int crc = this->calc_crc();
     
     this->update_code(crc, CRC_START, CRC_BITS);
  }

  void send_code() 
  {
    Serial.println("Sending code");
    this->update_nibbles();
    this->update_crc();
    this->update_pulse();
  
    for (int i = 0; i < 1; i++) {
      this->irsend->sendRaw(this->pulse, sizeof(this->pulse) / sizeof(this->pulse[0]), 38);
      delay(100);
    }  
  }

public:
  ACRemote(int sendPin) {
    this->irsend = new IRsend(sendPin);
    this->irsend->begin();

    this->storage = new Storage();
    this->load_code();
  }
  
  void send_code_if_needed()
  {
    if (this->has_changed) {
      send_code();
      this->has_changed = false;
    }
  }

  void update_temp(unsigned int temp) 
  {
    this->update_code(temp, TEMP_START, TEMP_BITS);
  }

  int get_temp()
  {
    return read_code(TEMP_START, TEMP_BITS);
  }

  void update_on(boolean isOn) 
  {
    unsigned int onOff = isOn ? 0x3 : 0xC;
    
    this->update_code(onOff, ON_OFF_START, ON_OFF_BITS);
  }

  boolean get_on()
  {
    return read_code(ON_OFF_START, ON_OFF_BITS) == 0x3;
  }

  void update_fan_power(unsigned int fan_power)
  {
    this->update_code(fan_power, FAN_POWER_START, FAN_POWER_BITS);
  }

  int get_fan_power()
  {
    return read_code(FAN_POWER_START, FAN_POWER_BITS);
  }

  void update_mode(unsigned int ac_mode)
  {
    this->update_code(ac_mode, MODE_START, MODE_BITS);
  }

  int get_mode()
  {
    return read_code(MODE_START, MODE_BITS);
  }
};
