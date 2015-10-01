/***********************************************************/
// Demo for the Serial MP3 Player Catalex (YX5300 chip)
// Hardware: Serial MP3 Player *1
// Board:  Arduino
// http://www.dx.com/p/uart-control-serial-mp3-music-player-module-for-arduino-avr-arm-pic-blue-silver-342439#.VfHyobPh5z0 
//
// 
//


// Uncomment SoftwareSerial for Arduino Uno or Nano.  

#include <SoftwareSerial.h>

#define ARDUINO_RX 5  //should connect to TX of the Serial MP3 Player module
#define ARDUINO_TX 6  //connect to RX of the module

SoftwareSerial mp3(ARDUINO_RX, ARDUINO_TX);
//#define mp3 Serial3    // Connect the MP3 Serial Player to the Arduino MEGA Serial3 (14 TX3 -> RX, 15 RX3 -> TX)

static int8_t Send_buf[8] = {0}; // Buffer for Send commands.  // BETTER LOCALLY
static uint8_t ansbuf[10] = {0}; // Buffer for the answers.    // BETTER LOCALLY

static int8_t pre_vol, volume = 0x0f; // Volume. 0-30 DEC values. 0x0f = 15. 

String mp3Answer;           // Answer from the MP3.   

boolean playing = false;    // Sending 'p' the module switch to Play to Pause or viceversa.   

/************ Command byte **************************/
#define CMD_NEXT_SONG     0X01  // Play next song.
#define CMD_PREV_SONG     0X02  // Play previous song.
#define CMD_PLAY_W_INDEX  0X03
#define CMD_VOLUME_UP     0X04
#define CMD_VOLUME_DOWN   0X05
#define CMD_SET_VOLUME    0X06

#define CMD_SNG_CYCL_PLAY 0X08  // Single Cycle Play.
#define CMD_SEL_DEV       0X09
#define CMD_SLEEP_MODE    0X0A
#define CMD_WAKE_UP       0X0B
#define CMD_RESET         0X0C
#define CMD_PLAY          0X0D
#define CMD_PAUSE         0X0E
#define CMD_PLAY_FOLDER_FILE 0X0F

#define CMD_STOP_PLAY     0X16
#define CMD_FOLDER_CYCLE  0X17
#define CMD_SHUFFLE_PLAY  0x18 //
#define CMD_SET_SNGL_CYCL 0X19 // Set single cycle.

#define CMD_SET_DAC 0X1A
  #define DAC_ON  0X00
  #define DAC_OFF 0X01
  
#define CMD_PLAY_W_VOL    0X22
#define CMD_PLAYING_N     0x4C

/************ Opitons **************************/  
#define DEV_TF            0X02  
#define SINGLE_CYCLE_ON   0X00
#define SINGLE_CYCLE_OFF  0X01


/*********************************************************************/

void setup() 
{
  Serial.begin(9600);
  mp3.begin(9600);
  delay(500);
                                          
        sendCommand(CMD_SEL_DEV, DEV_TF);  
  delay(200);
//  sendCommand(CMD_PLAY_W_VOL, 0X0F28); // Playing a 15 (0x0F) Vol the song num 40 (0x28). 
}


void loop() 
{
 char c=' ';
  
  // If there a char on Serial call sendMP3Command to sendCommand
   if( Serial.available() )
    {
    c = Serial.read();
    sendMP3Command(c);
    }
    
  // Check for the answer.
  if (mp3.available())
  {
    Serial.println(decodeMP3Answer());
  }
  delay(100);
}

 
/********************************************************************************/
/*Function sendMP3Command: seek for a 'c' command and send it to MP3  */
/*Parameter: c. Code for the MP3 Command, 'h' for help.                                                                                                         */
/*Return:  void                                                                */

void sendMP3Command(char c){
    switch (c) {
    case '?':
    case 'h':
          Serial.println("HELP  ");
          Serial.println(" p > Play / Pause ");
          Serial.println(" n > Next");          
          Serial.println(" b > Previous");
          Serial.println(" u > Volume UP");
          Serial.println(" d > Volume DOWN");
      break;
                 
     
      case 'p':
          if(!playing){
            Serial.println("Play ");
            //sendCommand(CMD_PLAY_W_VOL, 0X0F01);//play the first song with volume 15 class
            sendCommand(CMD_PLAY, 0);
            playing = true;
          }else{
            Serial.println("Pause");
            sendCommand(CMD_PAUSE, 0);
             playing = false;           
          }
      break;

      
      case 'n':
          Serial.println("Next");
          sendCommand(CMD_NEXT_SONG, 0);
          sendCommand(CMD_PLAYING_N, 0x0000); // ask for the number of file is playing
      break;
      
      
      case 'b':
          Serial.println("Previous");
          sendCommand(CMD_PREV_SONG, 0);
          sendCommand(CMD_PLAYING_N, 0x0000); // ask for the number of file is playing
      break;
     
      case 'u':
          Serial.println("Volume Up");
          sendCommand(CMD_VOLUME_UP, 0);
      break;

      case 'd':
          Serial.println("Volume Down");
          sendCommand(CMD_VOLUME_DOWN, 0);
      break;
      
    }
}


 
/********************************************************************************/
/*Function decodeMP3Answer: Decode MP3 answer.                                  */
/*Parameter:-void                                                               */
/*Return: The                                                  */

String decodeMP3Answer(){
  String decodedMP3Answer="";
  
      decodedMP3Answer+=sanswer(); 
      
    //  if (ansbuf[3] == 0x4C) // currently planying
    //  {
    //    decodedMP3Answer+=" -> Playing: "+String(ansbuf[6],DEC);
    //  }
      
     switch (ansbuf[3]) {
      case 0x3A:
         decodedMP3Answer+=" -> Memory card inserted.";
         break; 
         
      case 0x3D:
         decodedMP3Answer+=" -> Completed play num "+String(ansbuf[6],DEC);
         break; 
         
      case 0x4C:
         decodedMP3Answer+=" -> Playing: "+String(ansbuf[6],DEC);
         break;
      
      case 0x41:
         decodedMP3Answer+=" -> Data recived correctly. ";
         break;     
     } 
      
   return decodedMP3Answer;
 }  


 
 


/********************************************************************************/
/*Function: Send command to the MP3                                         */
/*Parameter:-int8_t command                                                     */
/*Parameter:-int16_ dat  parameter for the command                              */

void sendCommand(int8_t command, int16_t dat)
{
  delay(20);
  Send_buf[0] = 0x7e;   //
  Send_buf[1] = 0xff;   //
  Send_buf[2] = 0x06;   // Len 
  Send_buf[3] = command;//
  Send_buf[4] = 0x01;   // 0x00 NO, 0x01 feedback
  Send_buf[5] = (int8_t)(dat >> 8);  //datah
  Send_buf[6] = (int8_t)(dat);       //datal
  Send_buf[7] = 0xef;   //
  for(uint8_t i=0; i<8; i++)
  {
    mp3.write(Send_buf[i]) ;
  }
  
}



/********************************************************************************/
/*Function: sbyte2hex. Returns a byte data in HEX format.                 */
/*Parameter:- uint8_t b. Byte to convert to HEX.                                */
/*Return: String                                                                */


String sbyte2hex(uint8_t b)
{
  String shex;
  
  //Serial.print("0x");
  shex="0X";
  
  //if (b < 16) Serial.print("0");
  if (b < 16) shex+="0";
  //Serial.print(b, HEX);
  shex+=String(b,HEX);
  //Serial.print(" ");
  shex+=" ";
  return shex;
}




/********************************************************************************/
/*Function: sanswer. Returns a String answer from mp3 UART module.          */
/*Parameter:- uint8_t b. void.                                                  */
/*Return: String. If the answer is well formated answer.                        */

 String sanswer(void)
{
  uint8_t i = 0;
  String mp3answer="";
  
 // Get only 10 Bytes
 while(mp3.available() && (i < 10))
  {
    uint8_t b = mp3.read();
    ansbuf[i] = b;
    i++;
   
    //Serial.print(sbyte2hex(b));
    mp3answer+=sbyte2hex(b);
  }
 
  //Serial.println();
 
  // if the answer format is correct.
  if ((ansbuf[0] == 0x7E) && (ansbuf[9] == 0xEF))
  {
    //return true;
    return mp3answer;
  }
 
  //return false;
  return "???: "+mp3answer;
}
