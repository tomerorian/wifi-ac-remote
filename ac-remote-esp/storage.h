#include <EEPROM.h>

class Storage {
public:
  void begin() 
  {
    EEPROM.begin(512);
  }

  void end() 
  {
    EEPROM.end();
  }

  /** 
   *  Saves a string to the EEPROM
   *  
   *  @param start Starting location (first 2 bytes are used by the system)
   *  @param buffer_size The size to use (min 3 for empty string)
   *  @param string The string to save
   *  
   *  Note: First 2 bytes are used by the method to indicate a string, last byte must be '\0'
   */
  void save_string(int start, int buffer_size, String string)
  {
    int str_max_len = buffer_size - 3;
    
    if (str_max_len < 0) {
      return;
    }
  
    // String indicator
    EEPROM.write(start, 123);
    EEPROM.write(start + 1, 234);
  
    start += 2;
    int last = start;
    for (int i = start; i - start < string.length() && i - start < str_max_len; i++) {
      EEPROM.write(i, string[i-start] - '\0');
      last = i;
    }
  
    EEPROM.write(last+1, '\0');
  }
  
  String load_string(int start) {
    if (EEPROM.read(start) != 123 || EEPROM.read(start + 1) != 234) {
      return "";
    }
  
    String ret = "";
    int i = start + 2;
    char value = char(EEPROM.read(i));
    
    while (value != '\0') {
      ret += value;
      i += 1;
      value = char(EEPROM.read(i));
    }
  
    return ret;
  }
};
