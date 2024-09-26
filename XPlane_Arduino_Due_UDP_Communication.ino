
#include <Ethernet.h>
#include <EthernetUdp.h>

#include <SPI.h>

#ifdef ARDUINO_SAMD_VARIANT_COMPLIANCE
  #define SERIAL SerialUSB
  #define SYS_VOL   3.3
#else
  #define SERIAL Serial
  #define SYS_VOL   5
#endif

//=======================================================
float parameter_to_send_1;
float parameter_to_send_2;

char message_trimup[509];

unsigned long time_duration_of_packet;
unsigned long tic;

unsigned long time_without_packets;
unsigned long time_since_last_packet;
int time_threshold = 1000000; //after 1 second (1,000,000 microseconds) without incoming UDP packets there would be a standby state

//=======================UNION============================
 union {
        float f;
        unsigned char c_data[4];
        } char2float;
//====================ETHERNET SHIELD=====================
// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};
IPAddress ip(192, 168, 1, 177);

unsigned int localPort = 49004;// local port to listen on

// buffers for receiving and sending data
char packetBuffer[500];  // buffer to hold incoming packet, 5+n*36, where "n" - number of parameters from: General Data Output
char ReplyBuffer[] = "acknowledged";

// An EthernetUDP instance to let us send and receive packets over UDP
EthernetUDP Udp;

int number_of_received_parameters;
int k;
int message_size = 509;// constant message size that is required by X-Plane for sending datarefs
float trim_ratio = 0; //example parameter to send to X-Plane

//##############################VOID SETUP####################################
void setup() {

//arm ethernet shield
pinMode(10, OUTPUT);
digitalWrite(10,LOW);
  /*
  // You can use Ethernet.init(pin) to configure the CS pin
  //Ethernet.init(10);  // Most Arduino shields
  //Ethernet.init(5);   // MKR ETH shield
  //Ethernet.init(0);   // Teensy 2.0
  //Ethernet.init(20);  // Teensy++ 2.0
  //Ethernet.init(15);  // ESP8266 with Adafruit Featherwing Ethernet
  //Ethernet.init(33);  // ESP32 with Adafruit Featherwing Ethernet

  // start the Ethernet
  // Ethernet.init(10);
 
*/
 Ethernet.begin(mac, ip);
  // Open serial communications and wait for port to open:
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  // Check for Ethernet hardware present
  if (Ethernet.hardwareStatus() == EthernetNoHardware) {
    Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
    while (true) {
      delay(1); // do nothing, no point running without Ethernet hardware
    }
  }
  if (Ethernet.linkStatus() == LinkOFF) {
    Serial.println("Ethernet cable is not connected.");
  }
  // start UDP
  Udp.begin(localPort);
  

//==================other=============================
analogReadResolution(12); //set 10 bit for backwards compability
  
time_without_packets = micros(); //begin counting time without incoming packets
}

//###########################VOID LOOP################################
void loop() {
  tic = micros();

//============RECEIVING A PACKET=======================
  // if there's data available, read a packet
  int packetSize = Udp.parsePacket();
  if (packetSize) {
  //  Serial.print("Received packet of size ");
  //  Serial.println(packetSize);
  //  Serial.print("From ");
    IPAddress remote = Udp.remoteIP();
    for (int i=0; i < 4; i++) {
     // Serial.print(remote[i], DEC);
      if (i < 3) {
       // Serial.print(".");
      }
    }
   // Serial.print(", port ");
   // Serial.println(Udp.remotePort());

    // read the packet into packetBufffer
    Udp.read(packetBuffer, packetSize);
    //Serial.println("Contents:");

//===========EXTRACTING DATA FROM PACKET=================
 for (int i=0; i < packetSize; i++) {
      //Serial.println(packetBuffer[i], HEX);
    }
   // Serial.println(packetBuffer);

 number_of_received_parameters = (packetSize - 5)/36;  // packet size - 5 byte header and then divided by 36 bytes for every parameter (4 byte ID + 32 bytes of data)
 // Serial.print("number of received parameters: ");
 //Serial.println(number_of_received_parameters);

double data_from_xplane[number_of_received_parameters][8]; //8 SEGMENTS in every row of matrix

int id[number_of_received_parameters];
    for(int j = 0 ; j < number_of_received_parameters; j++){

    
     id[j] = ( ( (packetBuffer[8+36*j]) * 256 + packetBuffer[7+36*j]) * 256 + packetBuffer[6+36*j]) * 256 + packetBuffer[5+36*j]; //ID of parameter (first 4 bytes i.e.:0x08, 0x00, 0x00, 0x00 which in this case is 8)
    //Serial.print("Decoded ID of parameter:");
    //Serial.println(id[j]);
    }
    for(int j = 0 ; j < number_of_received_parameters; j++){ //extracting parameters from each ID
      //Serial.print("index: ");
      //Serial.println(id[j]);
      k = 0; //index of a SEGMENT
            for(int i = 9+36*j ; i < 5+36+36*j ; i+=4){ //extracting data from each segment
                          char2float.c_data[0] = packetBuffer[i];
                          char2float.c_data[1] = packetBuffer[i+1];
                          char2float.c_data[2] = packetBuffer[i+2];
                          char2float.c_data[3] = packetBuffer[i+3];
                          data_from_xplane[j][k] = char2float.f;  //matrix of extracted data
                          //Serial.print("data_from_xplane ");
                          //Serial.println(data_from_xplane[j][k], 4);
                          k++;
            }
    }
//==========================CREATING A MESSAGE TO X-PLANE===============================
  // SEGMENT 1 of PARAMETER 1
parameter_to_send_1 = variable_1;
byte *segment = (byte *)&parameter_to_send_1;                                                                                                                                 
  // SEGMENT 2 of PARAMETER 1
parameter_to_send_2 = variable_2;
byte *segment2 = (byte *)&parameter_to_send_2;

byte message_to_xplane[] = {0x44, 0x41, 0x54, 0x41, 0x2a, 0x08, 0x00, 0x00, 0x00, segment[0],segment[1],segment[2],segment[3],segment2[0],segment2[1],segment2[2],segment2[3], 0x00, 0xc0, 0x79, 0xc4, 0x00, 0xc0, 0x79, 0xc4, 0x00, 0xc0, 0x79, 0xc4, 0x00, 0xc0, 0x79, 0xc4, 0x00, 0xc0, 0x79, 0xc4, 0x00, 0xc0, 0x79, 0xc4};
// "DATA*[ID][segment][segment2][-999][-999][-999][-999][-999][-999]"
// 0x44, 0x41, 0x54, 0x41, 0x2a   is 5 byte header "DATA*" in HEX (last byte is just a NULL)
// 0x08, 0x00, 0x00, 0x00         is 4 byte ID number (8 in this case) of parameter that you want to over-ride
// then you have 8 segments that are 4 byte each (32 bytes in total)
//(put -999 for blank parameter or to leave default value if you dont want to over-ride it)
//==============SENDING A MESSAGE TO X-PLANE=================================
    Udp.beginPacket(Udp.remoteIP(), Udp.remotePort()-1); //sending messageto X-Plane on port that is smaller by one than the port number wereceive from (49001-1=49000)
    Udp.write(message_to_xplane,41); // 41 bytes = 4 byte header + 1 byte NULL + 4 byte ID + 32 byte message (8 segments, 4 byte each)
    Udp.endPacket();

 //==============================SENDING DATAREFS=============================
/*
//====================================CMND====================================
   if(TRIMbuttonup == HIGH){
//CMND0sim/flight_controls/pitch_trim_up0
    message_trimup = {0x43, 0x4d, 0x4e, 0x44, 0x00, 0x73, 0x69, 0x6d, 0x2f, 0x63, 0x6f, 0x63, 0x6b, 0x70, 0x69, 0x74, 0x32, 0x2f, 0x63, 0x6f, 0x6e, 0x74, 0x72, 0x6f, 0x6c, 0x73, 0x2f, 0x65, 0x6c, 0x65, 0x76, 0x61, 0x74, 0x6f, 0x72, 0x5f, 0x74, 0x72, 0x69, 0x6d, 0x00};
    
    Udp.beginPacket(Udp.remoteIP(), Udp.remotePort()-1); //sending messageto X-Plane on port that is smaller by one than the port number wereceive from (49001-1=49000)
    Udp.write(message_trimup,message_size);
    Udp.endPacket();
    }
*/
//=====================================DREF====================================
    byte *b_trim = (byte *)&trim_ratio;
//DREF+ byte byte byte byte sim/cockpit2/controls/elevator_trim
char message_trimup = {0x44, 0x52, 0x45, 0x46, 0x2b, b_trim[0], b_trim[1], b_trim[2], b_trim[3], 0x73, 0x69, 0x6d, 0x2f, 0x63, 0x6f, 0x63, 0x6b, 0x70, 0x69, 0x74, 0x32, 0x2f, 0x63, 0x6f, 0x6e, 0x74, 0x72, 0x6f, 0x6c, 0x73, 0x2f, 0x65, 0x6c, 0x65, 0x76, 0x61, 0x74, 0x6f, 0x72, 0x5f, 0x74, 0x72, 0x69, 0x6d, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    
    Udp.beginPacket(Udp.remoteIP(), Udp.remotePort()-1); //sending messageto X-Plane on port that is smaller by one than the port number wereceive from (49001-1=49000)
    Udp.write(message_trimup,message_size);
    Udp.endPacket();
    }

//==============================================================================
  //time duration of a single loop
  time_duration_of_packet = micros() - tic;
  //Serial.print("time duration of packet: ");
  //Serial.println(time_duration_of_packet);
  
  time_since_last_packet = 0; //RESET of time since last received packet
  time_without_packets = micros(); //UPDATE of time reference without incoming packets
  
  }//end of if(packetSize)...
  else //====================== NO CONNECTION WITH X-PLANE====================
  {
  time_since_last_packet = micros() - time_without_packets; //calculate time since last received packet
      if(time_since_last_packet >= time_threshold){
      //action to do if there is too much time since last received packet (for example turn off some motors)




      }
      //Serial.print("time since last packet: ");
      //Serial.println(time_since_last_packet);

          delay(10);
  }
}
