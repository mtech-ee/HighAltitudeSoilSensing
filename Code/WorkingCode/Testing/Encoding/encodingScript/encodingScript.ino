void setup() {
  delay(500);
  Serial.begin(115200);
  char testString[26] = {"3341348254535638107957898514"};
  char finalString[10];
  int i;
  int strCounter = 0;
  char strToChange[2];
  Serial.begin(115200);
  for(strCounter = 0;strCounter < 14; strCounter++){
    for(i=0 + strCounter; i < 2 + (2*strCounter);i++){
      
    }
     
  }
  //int intToChange = atoi(strToChange);
  int intToChange = 93;
  //char asciiChar = intToChange + 33;
  char asciiChar = 'α';
  Serial.println(176,BYTE);
  

}

void loop() {
  // put your main code here, to run repeatedly:
  
}

char encodeString(char inChar){
  char ch;
  inChar = (int) inChar;
  switch(inChar){
    case 0:ch= "00";break; case 1:ch= "01";break;
    case 2:ch= "02";break; case 3:ch= "03";break;
    case 4:ch= "04";break; case 5:ch= "05";break;
    case 6:ch= "06";break; case 7:ch= "07";break;
    case 8:ch= "08";break; case 9:ch= "09";break;
    case 10:ch= "0a";break; case 11:ch= "0b";break;
    case 12:ch= "0c";break; case 13:ch= "0d";break;
    case 14:ch= "0e";break; case 15:ch= "0f";break;
    case 16:ch= "0g";break; case 17:ch= "0h";break;
    case 18:ch= "0i";break; case 19:ch= "0j";break;
    case 20:ch= "0k";break; case 21:ch= "0l";break;
    case 22:ch= "0m";break; case 23:ch= "0n";break;
    case 24:ch= "0o";break; case 25:ch= "0p";break;
    case 26:ch= "0q";break; case 27:ch= "0r";break;
    case 28:ch= "0s";break; case 29:ch= "0t";break;
    case 30:ch= "0u";break; case 31:ch= "0v";break;
    case 32:ch= "0w";break; case 33:ch= "0x";break;
    case 34:ch= "0y";break; case 35:ch= "0z";break;
    case 36:ch= "0A";break; case 37:ch= "0B";break;
    case 38:ch= "0C";break; case 39:ch= "0D";break;
    case 40:ch= "0E";break; case 41:ch= "0F";break;
    case 42:ch= "0G";break; case 43:ch= "0H";break;
    case 44:ch= "0I";break; case 45:ch= "0J";break;
    case 46:ch= "0K";break; case 47:ch= "0L";break;
    case 48:ch= "0M";break; case 49:ch= "0N";break;
    case 50:ch= "0O";break; case 51:ch= "0P";break;
    case 52:ch= "0Q";break; case 53:ch= "0R";break;
    case 54:ch= "0S";break; case 55:ch= "0T";break;
    case 56:ch= "0U";break; case 57:ch= "0V";break;
    case 58:ch= "0W";break; case 59:ch= "0X";break;
    case 60:ch= "0Y";break; case 61:ch= "0Z";break;
    
    case 62:ch= "10";break; case 63:ch= "11";break;
    case 64:ch= "12";break; case 65:ch= "13";break;
    case 66:ch= "14";break; case 67:ch= "15";break;
    case 68:ch= "16";break; case 69:ch= "17";break;
    case 70:ch= "18";break; case 71:ch= "19";break;
    case 72:ch= "1a";break; case 73:ch= "1b";break;
    case 74:ch= "1c";break; case 75:ch= "1d";break;
    case 76:ch= "1e";break; case 77:ch= "1f";break;
    case 78:ch= "1g";break; case 79:ch= "1h";break;
    case 80:ch= "1i";break; case 81:ch= "1j";break;
    case 82:ch= "1k";break; case 83:ch= "1l";break;
    case 84:ch= "1m";break; case 85:ch= "1n";break;
    case 86:ch= "1o";break; case 87:ch= "1p";break;
    case 88:ch= "1q";break; case 89:ch= "1r";break;
    case 90:ch= "1s";break; case 91:ch= "1t";break;
    case 92:ch= "1u";break; case 93:ch= "1v";break;
    case 94:ch= "1w";break; case 95:ch= "1x";break;
    case 96:ch= "1y";break; case 97:ch= "1z";break;
    case 98:ch= "1A";break; case 99:ch= "1B";break;
    case 100:ch= "1C";break; case 101:ch= "1D";break;
    case 102:ch= "1E";break; case 103:ch= "1F";break;
    case 104:ch= "1G";break; case 105:ch= "1H";break;
    case 106:ch= "1I";break; case 107:ch= "1J";break;
    case 108:ch= "1K";break; case 109:ch= "1L";break;
    case 110:ch= "1M";break; case 111:ch= "1N";break;
    case 112:ch= "1O";break; case 113:ch= "1P";break;
    case 114:ch= "1Q";break; case 115:ch= "1R";break;
    case 116:ch= "1S";break; case 117:ch= "1T";break;
    case 118:ch= "1U";break; case 119:ch= "1V";break;
    case 120:ch= "1W";break; case 121:ch= "1X";break;
    case 122:ch= "1Y";break; case 123:ch= "1Z";break;
    
    
  }
}
