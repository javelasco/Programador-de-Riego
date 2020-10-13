#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <MCUFRIEND_kbv.h>   
#include <RTClib.h>
#include <EEPROM.h>

MCUFRIEND_kbv tft;
RTC_DS3231 rtc;                 // SDA a pin A4 y SCL a pin A5

#include <FreeDefaultFonts.h>

#define pulsador    10
#define valvulas    11

#define BLACK   0x0000
#define RED     0xF800
#define GREEN   0x07E0
#define WHITE   0xFFFF
#define LCD_CS A3 // Chip Select goes to Analog 3
#define LCD_CD A2 // Command/Data goes to Analog 2
#define LCD_WR A1 // LCD Write goes to Analog 1
#define LCD_RD A0 // LCD Read goes to Analog 0
//#define LCD_RESET A4 // Can alternately just connect to Arduino's reset pin

unsigned int JoystickX, JoystickY;
byte IDSubSetting=0, i=0;
byte countJoystickV=1, countJoystickH=1;
byte alarmaHoras=0, alarmaMinutos=0;
byte diaSemana, diaSemanaRiego, flagConfigDate=0;
int dia, mes, anio, hora, minuto, segundo=0;
byte diaSemanaAlarma[] = {LOW,LOW,LOW,LOW,LOW,LOW,LOW};
boolean flagDashboard=true, flagSettings=false, flagAlarma=false, modoProgSecuencia=false, JoystickSW, flagSubDashboard=false, flagSettings2=false;

//Variables en EEPROM
boolean flagModo=false, estadoDispo=0;
byte horaInicio=0, minutoInicio=0, duracionHoras=0, duracionMinutos=0;
byte diasRiego[] = {LOW,LOW,LOW,LOW,LOW,LOW,LOW};
char lunes='_', martes='_', miercoles='_', jueves='_', viernes='_', sabado='_', domingo='_';
char marcaDiasRiego[] = {' ', ' ', ' ', ' ', ' ', ' ', ' '};

void setup() {
  pinMode(pulsador, INPUT);
  pinMode(valvulas, OUTPUT);  
  digitalWrite(valvulas,HIGH);
  
  Serial.begin(9600);


  // Ejecutar solo la primera vez que se graba, luego comentar la y volver a grabar el programa.
/*EEPROM.write(0, flagModo);                                                                 
  EEPROM.write(1, horaInicio); EEPROM.write(2, minutoInicio); EEPROM.write(3, duracionHoras); EEPROM.write(4, duracionMinutos); EEPROM.write(5, estadoDispo);
  EEPROM.write(6, diasRiego[0]); EEPROM.write(7, diasRiego[1]); EEPROM.write(8, diasRiego[2]); EEPROM.write(9, diasRiego[3]); EEPROM.write(10, diasRiego[4]);       
  EEPROM.write(11,diasRiego[5]); EEPROM.write(12,diasRiego[6]);                                                      
  EEPROM.write(13, lunes); EEPROM.write(14, martes); EEPROM.write(15, miercoles); EEPROM.write(16, jueves); EEPROM.write(17, viernes);   
  EEPROM.write(18, sabado); EEPROM.write(19, domingo);                                                      
  EEPROM.write(20, marcaDiasRiego[0]); EEPROM.write(21, marcaDiasRiego[1]); EEPROM.write(22, marcaDiasRiego[2]); EEPROM.write(23, marcaDiasRiego[3]); EEPROM.write(24, marcaDiasRiego[4]);   
  EEPROM.write(25, marcaDiasRiego[5]); EEPROM.write(26, marcaDiasRiego[6]);           */                 


  // Lee las posiciones de la EEPROM donde se almacena las variables principales del programa.
  flagModo=EEPROM.read(0);                                                                                                                                                          // LEER VARIABLE flagModo
  horaInicio=EEPROM.read(1); minutoInicio=EEPROM.read(2); duracionHoras=EEPROM.read(3); duracionMinutos=EEPROM.read(4); estadoDispo=EEPROM.read(5);                                 // LEER VARIABLE horaInicio, minutoInicio, duracionHoras, duracionMinutos y estadoDispo
  diasRiego[0]=EEPROM.read(6); diasRiego[1]=EEPROM.read(7); diasRiego[2]=EEPROM.read(8); diasRiego[3]=EEPROM.read(9); diasRiego[4]=EEPROM.read(10);                                 // LEER VARIABLE diasRiego[0], diasRiego[1], diasRiego[2], diasRiego[3], diasRiego[4]
  diasRiego[5]=EEPROM.read(11); diasRiego[6]=EEPROM.read(12);                                                                                                                       // LEER VARIABLE diasRiego[5] y diasRiego[6]
  lunes=EEPROM.read(13); martes=EEPROM.read(14); miercoles=EEPROM.read(15); jueves=EEPROM.read(16); viernes=EEPROM.read(17);                                                        // LEER VARIABLE lunes, martes, miercoles, jueves, viernes
  sabado=EEPROM.read(18); domingo=EEPROM.read(19);                                                                                                                                  // LEER VARIABLE sabado y domingo 
  marcaDiasRiego[0]=EEPROM.read(20); marcaDiasRiego[1]=EEPROM.read(21); marcaDiasRiego[2]=EEPROM.read(22); marcaDiasRiego[3]=EEPROM.read(23); marcaDiasRiego[4]=EEPROM.read(24);    // LEER VARIABLE marcaDiasRiego[0], marcaDiasRiego[1], marcaDiasRiego[2], marcaDiasRiego[3], marcaDiasRiego[4]
  marcaDiasRiego[5]=EEPROM.read(25); marcaDiasRiego[6]=EEPROM.read(26);                                                                                                             // LEER VARIABLE marcaDiasRiego[5] y marcaDiasRiego[6] 

  // Iniciar RTC
  if(!rtc.begin()){
    Serial.println("¡Modulo RTC no encontrado!");
    for(;;); //bucle 
    }

  // Iniciar TFT
   tft.begin(0x9486);
   tft.setRotation(1);
   tft.fillScreen(WHITE);
   tft.setTextColor(BLACK,WHITE); 
}

// Valida la hora y minuto que durará el Riego Automático
void checkTiempoAlarma(){
  byte carryH=0;
  byte carryM=0;
  DateTime fechaNow = rtc.now();   
  if ((fechaNow.minute()+duracionMinutos) >= 60){
    carryH=1;
    if (duracionMinutos > 1) {
      carryM=(duracionMinutos-(60-fechaNow.minute()));
    }
    else {carryM=0;}
  }
  
  alarmaHoras=(fechaNow.hour()+duracionHoras+carryH);
  if (alarmaHoras >= 24) alarmaHoras=alarmaHoras-24;
  if ((fechaNow.minute()+duracionMinutos) >= 60) {alarmaMinutos=carryM;}
  else {
    alarmaMinutos=(fechaNow.minute()+duracionMinutos);
  }
  Serial.println(alarmaHoras);
  Serial.println(alarmaMinutos);  
}    


//Valida si el día corresponde a los días que se deben regar
void validarDiaAlarma(){
  DateTime fechaNow = rtc.now();   
  switch (fechaNow.dayOfTheWeek()){
    case 1: diaSemanaAlarma[0]=HIGH; diaSemanaAlarma[1]=LOW; diaSemanaAlarma[2]=LOW; diaSemanaAlarma[3]=LOW; diaSemanaAlarma[4]=LOW; diaSemanaAlarma[5]=LOW; diaSemanaAlarma[6]=LOW; break;
    case 2: diaSemanaAlarma[0]=LOW; diaSemanaAlarma[1]=HIGH; diaSemanaAlarma[2]=LOW; diaSemanaAlarma[3]=LOW; diaSemanaAlarma[4]=LOW; diaSemanaAlarma[5]=LOW; diaSemanaAlarma[6]=LOW; break;
    case 3: diaSemanaAlarma[0]=LOW; diaSemanaAlarma[1]=LOW; diaSemanaAlarma[2]=HIGH; diaSemanaAlarma[3]=LOW; diaSemanaAlarma[4]=LOW; diaSemanaAlarma[5]=LOW; diaSemanaAlarma[6]=LOW; break;
    case 4: diaSemanaAlarma[0]=LOW; diaSemanaAlarma[1]=LOW; diaSemanaAlarma[2]=LOW; diaSemanaAlarma[3]=HIGH; diaSemanaAlarma[4]=LOW; diaSemanaAlarma[5]=LOW; diaSemanaAlarma[6]=LOW; break;
    case 5: diaSemanaAlarma[0]=LOW; diaSemanaAlarma[1]=LOW; diaSemanaAlarma[2]=LOW; diaSemanaAlarma[3]=LOW; diaSemanaAlarma[4]=HIGH; diaSemanaAlarma[5]=LOW; diaSemanaAlarma[6]=LOW; break;
    case 6: diaSemanaAlarma[0]=LOW; diaSemanaAlarma[1]=LOW; diaSemanaAlarma[2]=LOW; diaSemanaAlarma[3]=LOW; diaSemanaAlarma[4]=LOW; diaSemanaAlarma[5]=HIGH; diaSemanaAlarma[6]=LOW; break;
    case 0: diaSemanaAlarma[0]=LOW; diaSemanaAlarma[1]=LOW; diaSemanaAlarma[2]=LOW; diaSemanaAlarma[3]=LOW; diaSemanaAlarma[4]=LOW; diaSemanaAlarma[5]=LOW; diaSemanaAlarma[6]=HIGH; break;
    } 
}

void loop() {    
while (flagDashboard){

//################### VALIDAR ALARMA PARA RIEGO ###################

  DateTime fechaNow = rtc.now(); 
  if (fechaNow.hour()==horaInicio && fechaNow.minute()==minutoInicio){
    validarDiaAlarma();
    if (((diaSemanaAlarma[0] && diasRiego[0])==HIGH || (diaSemanaAlarma[1] && diasRiego[1])==HIGH || (diaSemanaAlarma[2] && diasRiego[2])==HIGH ||
    (diaSemanaAlarma[3] && diasRiego[3])==HIGH || (diaSemanaAlarma[4] && diasRiego[4])==HIGH || (diaSemanaAlarma[5] && diasRiego[5])==HIGH ||
    (diaSemanaAlarma[6] && diasRiego[6])==HIGH) && !flagAlarma && flagModo && (duracionHoras+duracionMinutos) != 0){
        checkTiempoAlarma();
        flagAlarma=true;
        digitalWrite(valvulas,LOW);
    }
  }  
  
  if (fechaNow.hour()==alarmaHoras && fechaNow.minute()==alarmaMinutos && flagAlarma){
      DateTime fechaNow = rtc.now(); 
      digitalWrite(valvulas,HIGH);
      flagAlarma=false;
      diaSemanaAlarma[0]=LOW; diaSemanaAlarma[1]=LOW; diaSemanaAlarma[2]=LOW; 
      diaSemanaAlarma[3]=LOW; diaSemanaAlarma[4]=LOW; diaSemanaAlarma[5]=LOW;
      diaSemanaAlarma[6]=LOW;
  }

//###################################################################
  
  dashboard();   
  JoystickX = analogRead(A6);
  JoystickY = analogRead(A7);

  //MOVIMIENTOS HORIZONTALES
  if (JoystickX >=0 && JoystickX <400) {        //¿SE MUEVE HACIA LA IZQUIERDA?
    delay(250);
    flagDashboard=false;
    flagSettings=true;
    flagSubDashboard=false;
    tft.fillScreen(WHITE);
    }
 
  if (JoystickX >600 && JoystickX <=1023) {     //¿SE MUEVE HACIA LA DERECHA?
    delay(250);
    flagDashboard=false;
    flagSettings=true;
    flagSubDashboard=false;    
    tft.fillScreen(WHITE);
  }
}

while (flagSettings){

//################### VALIDAR ALARMA PARA RIEGO ###################  
  DateTime fechaNow = rtc.now(); 
  if (fechaNow.hour()==horaInicio && fechaNow.minute()==minutoInicio){
    validarDiaAlarma();
    if (((diaSemanaAlarma[0] && diasRiego[0])==HIGH || (diaSemanaAlarma[1] && diasRiego[1])==HIGH || (diaSemanaAlarma[2] && diasRiego[2])==HIGH ||
    (diaSemanaAlarma[3] && diasRiego[3])==HIGH || (diaSemanaAlarma[4] && diasRiego[4])==HIGH || (diaSemanaAlarma[5] && diasRiego[5])==HIGH ||
    (diaSemanaAlarma[6] && diasRiego[6])==HIGH) && !flagAlarma && flagModo && (duracionHoras+duracionMinutos) != 0){
        checkTiempoAlarma();
        flagAlarma=true;
        digitalWrite(valvulas,LOW);
    }
  }  
  
  if (fechaNow.hour()==alarmaHoras && fechaNow.minute()==alarmaMinutos && flagAlarma){
      DateTime fechaNow = rtc.now(); 
      digitalWrite(valvulas,HIGH);
      flagAlarma=false;
      diaSemanaAlarma[0]=LOW; diaSemanaAlarma[1]=LOW; diaSemanaAlarma[2]=LOW; 
      diaSemanaAlarma[3]=LOW; diaSemanaAlarma[4]=LOW; diaSemanaAlarma[5]=LOW;
      diaSemanaAlarma[6]=LOW;
  }

//###################################################################
  
  settings();
  JoystickX = analogRead(A6);
  JoystickY = analogRead(A7);
  
  //MOVIMIENTOS VERTICALES
  if (JoystickY > 530 && JoystickY <= 1023) {        //¿SE MUEVE HACIA ABAJO? 
    delay(100);
    selectorSettingsDown();
    countJoystickV++;
    }
    if(countJoystickV > 4) countJoystickV=1;
 
  if (JoystickY >= 0 && JoystickY < 470) {     //¿SE MUEVE HACIA LA ARRIBA?
    delay(100);
    selectorSettingsUp();
    if (countJoystickV <= 0){
      countJoystickV=5;
      }
    countJoystickV--;
    }
    if(countJoystickV < 1) countJoystickV=4;
  
  //MOVIMIENTOS HORIZONTALES
  if (JoystickX >=0 && JoystickX <400) {        //¿SE MUEVE HACIA LA IZQUIERDA?
    delay(250);
    flagDashboard=true;
    flagSettings=false;
    flagSettings2=false;    
    tft.fillScreen(WHITE);
    }
 
  if (JoystickX >600 && JoystickX <=1023) {     //¿SE MUEVE HACIA LA DERECHA?
    delay(250);
    flagDashboard=true;
    flagSettings=false;
    flagSettings2=false;        
    tft.fillScreen(WHITE);
  }

  
  //SWITCH SELECTOR
  if (!digitalRead(pulsador)){
    while(!digitalRead(pulsador));
    flagSettings2=false;  
    subSettings();
    }
}
}

void selectorSettingsDown(){
  switch(countJoystickV){
  case 1: limpiarSelector(); textSinClear(18, 80, ">", 2, NULL); IDSubSetting=1; break;
  case 2: limpiarSelector(); textSinClear(18, 120, ">", 2, NULL); IDSubSetting=2; break;
  case 3: limpiarSelector(); textSinClear(18, 160, ">", 2, NULL); IDSubSetting=3; break;
  default: limpiarSelector(); textSinClear(18, 200, ">", 2, NULL); IDSubSetting=4;
  }
}

void selectorSettingsUp(){
  switch(countJoystickV){
  case 1: limpiarSelector(); textSinClear(18, 200, ">", 2, NULL); IDSubSetting=4; break;
  case 2: limpiarSelector(); textSinClear(18, 80, ">", 2, NULL); IDSubSetting=1; break;
  case 3: limpiarSelector(); textSinClear(18, 120, ">", 2, NULL); IDSubSetting=2; break;
  default: limpiarSelector(); textSinClear(18, 160, ">", 2, NULL); IDSubSetting=3;
  }
}

void subSettings(){
  switch(IDSubSetting){
  case 1: modoProgSecuencia=false; fechaHora(); tft.fillScreen(WHITE);  break;                                  // CONFIGURAR FECHA Y HORA
  case 2: modoProgSecuencia=true; programaSecuencia(); modoProgSecuencia=false; tft.fillScreen(WHITE); break;   // CONFIGURAR SECUENCIA
  case 3: riegoManual(); tft.fillScreen(WHITE); break;                                                          // ACTIVAR/DESACTIVAR RIEGO MANUAL
  default: desactivar(); tft.fillScreen(WHITE);                                                                 // ACTIVAR/DESACTIVAR SISTEMA
  }
}

void limpiarSelector(){
  tft.setTextColor(WHITE,WHITE);
  textSinClear(18, 80, ">", 2, NULL);
  textSinClear(18, 120, ">", 2, NULL);  
  textSinClear(18, 160, ">", 2, NULL);
  textSinClear(18, 200, ">", 2, NULL);  
  textSinClear(18, 240, ">", 2, NULL);   
  tft.setTextColor(BLACK,WHITE);
  }

void textSinClear(unsigned int x_pos, unsigned int y_pos, const char *text, byte sizeText, const GFXfont *f) {
  tft.setFont(f);  
  tft.setTextSize(sizeText);             // Normal 1:1 pixel scale  
  tft.setCursor(x_pos,y_pos);             // Start at top-left corner
  tft.print(text);
}
  
void settings(){
   if (flagSettings2==false){
    flagSettings2=true;    
    tft.fillRect(0,0,480,48,BLACK);
    tft.setTextColor(WHITE);     
    textSinClear(84, 12, "CONFIGURAR SISTEMA", 3, NULL);  
   }
   tft.setTextColor(BLACK,WHITE);        
   textSinClear(45, 80, "FECHA Y HORA", 2, NULL);
   textSinClear(45, 120, "PROGRAMAR SECUENCIA", 2, NULL);
   textSinClear(45, 160, "RIEGO MANUAL", 2, NULL);
   switch(estadoDispo){
    case 0: textSinClear(45, 200, "ACTIVAR", 2, NULL); break;
    default: textSinClear(45, 200, "DESACTIVAR", 2, NULL);
    }
}

void dashboard(){
   DateTime fechaNow = rtc.now(); 
   diaSemana = fechaNow.dayOfTheWeek(); dia=fechaNow.day(); mes=fechaNow.month(); anio=fechaNow.year()-2000; hora=fechaNow.hour(); minuto=fechaNow.minute();
   tft.setTextColor(BLACK,WHITE);  
   display_day(164,96);
   tft.setTextSize(5);     
   tft.setCursor(90, 140);                   
   printDigits(dia);
   tft.print('-');
   printDigits(mes);
   tft.print("-20"); 
   tft.print(anio);    
   tft.setCursor(126, 210);     
   //if (hora==24) hora=0;            
   printDigits(hora);
   tft.print(':');      
   printDigits(minuto);
   tft.print(':');      
   printDigits(fechaNow.second());  
   if (digitalRead(valvulas)==LOW) {
      tft.setTextColor(BLACK,WHITE); 
      textSinClear(178, 272, "REGANDO!", 3, NULL);
   }
   else {
    tft.setTextColor(WHITE,WHITE); 
    textSinClear(178, 272, "REGANDO!", 3, NULL);
   }   
   subDashboard();
}

void subDashboard(){
  if (flagSubDashboard==false){
    flagSubDashboard=true;
    tft.fillRect(0,0,480,48,BLACK);
    tft.setTextColor(WHITE);    
    textSinClear(31, 15, "D:", 2, NULL);    
    tft.print(lunes);
    tft.print(martes);
    tft.print(miercoles);
    tft.print(jueves);
    tft.print(viernes);
    tft.print(sabado);
    tft.print(domingo);
    tft.setCursor(168,15);
    tft.print("H:");  
    if (horaInicio==24) printDigits(horaInicio-24);
    else {printDigits(horaInicio);}
    tft.print(':');
    printDigits(minutoInicio);
 
    tft.setCursor(276,15);
    tft.print("T:");  
    if (duracionHoras==24) printDigits(duracionHoras-24);
    else {printDigits(duracionHoras);}
    tft.print(':');
    printDigits(duracionMinutos);
  
    tft.setCursor(384,15);
    if (estadoDispo==0) tft.print("E:OFF");
    else {tft.print("E:ON");}  
  }
}

void display_day(unsigned int x_pos, unsigned int y_pos){
  switch(diaSemana){
    case 1: textSinClear(x_pos, y_pos, "  LUNES  ", 3, NULL); break;
    case 2: textSinClear(x_pos, y_pos, " MARTES  ", 3, NULL); break;
    case 3: textSinClear(x_pos, y_pos, "MIERCOLES", 3, NULL); break;
    case 4: textSinClear(x_pos, y_pos, " JUEVES  ", 3, NULL); break;
    case 5: textSinClear(x_pos, y_pos, " VIERNES ", 3, NULL); break;
    case 6: textSinClear(x_pos, y_pos, " SABADO  ", 3, NULL); break;
    default: textSinClear(x_pos, y_pos, " DOMINGO", 3, NULL);
  }
}

void printDigits(unsigned int digits){
  if(digits < 10){
    tft.print('0');    
    }
  else if (digits == 24 && i==4){
    digits=0;
    tft.print('0');    
    }
  else if (digits == 60 && i==5){
    digits=0;
    tft.print('0');    
    }    
  tft.print(digits);
}

void oscilarText(){
  byte y=0;
  while (y<10 && JoystickY >=470 && JoystickY <540 && digitalRead(pulsador)){
    y++;
    JoystickX = analogRead(A6);
    JoystickY = analogRead(A7);
    delay(25);
  }
}

void fechaHora2(){
   if (flagConfigDate==0){
   DateTime fechaNow = rtc.now(); dia=fechaNow.day(); mes=fechaNow.month(); anio=fechaNow.year()-2000; hora=fechaNow.hour(); minuto=fechaNow.minute();
   }
   tft.fillRect(0,0,480,48,BLACK);
   tft.setTextColor(WHITE,BLACK);     
   textSinClear(138, 12, "FECHA Y HORA", 3, NULL);   
   tft.setTextColor(BLACK);
   
   tft.setFont(&FreeSevenSegNumFont);
   tft.setCursor(80, 153);
   tft.setTextSize(1);      
   printDigits(dia);
   
   textSinClear(147, 110, "-", 5, NULL);      
    
   tft.setFont(&FreeSevenSegNumFont);
   tft.setCursor(176, 153);
   tft.setTextSize(1);         
   printDigits(mes);
   
   textSinClear(243, 110, "-", 5, NULL);  
   textSinClear(272, 153, "20", 1, &FreeSevenSegNumFont);          

   tft.setFont(&FreeSevenSegNumFont);
   tft.setCursor(336, 153);
   tft.setTextSize(1);         
   tft.print(anio);

   tft.setFont(&FreeSevenSegNumFont);
   tft.setCursor(161, 227);
   tft.setTextSize(1);         
   //if (hora==24) hora=hora-24;        
   printDigits(hora);
   
   textSinClear(227, 181, ":", 6,  NULL);    

   tft.setFont(&FreeSevenSegNumFont);
   tft.setCursor(257, 227);
   tft.setTextSize(1);      
   if (minuto==60) minuto=minuto-60;  
   printDigits(minuto);
   
   textSinClear(312, 272, "GUARDAR", 3, NULL);     
}

void fechaHora(){
  flagConfigDate=1;
  tft.fillScreen(WHITE);
  tft.setTextColor(BLACK,WHITE);
  fechaHora2();  
  while(flagConfigDate){
  JoystickX = analogRead(A6);
  JoystickY = analogRead(A7);
  i=1;   
  dia = edit(80, 153, 1, dia, &FreeSevenSegNumFont);                      // Edit date
  mes = edit(176, 153, 1, mes, &FreeSevenSegNumFont);                    // Edit month
  anio = edit(336, 153, 1, anio, &FreeSevenSegNumFont);                    // Edit year
  hora = edit(161, 227, 1, hora, &FreeSevenSegNumFont);                     // Edit hours
  minuto = edit(257, 227, 1, minuto, &FreeSevenSegNumFont);                   // Edit minutes
  segundo = 0;
    while(true){
      JoystickX = analogRead(A6);
      //JoystickY = analogRead(A7);
      tft.setTextColor(WHITE,WHITE); 
      textSinClear(312, 272, "GUARDAR", 3, NULL);
      oscilarText();
      tft.setTextColor(BLACK,WHITE); 
      textSinClear(312, 272, "GUARDAR", 3, NULL);
      oscilarText();
      if(JoystickX >= 0 && JoystickX < 400){
        delay(300);
        break;
      }
      if(!digitalRead(pulsador)){
        while(!digitalRead(pulsador));
        rtc.adjust(DateTime(anio, mes, dia, hora, minuto, segundo));
        flagConfigDate=0;       
        digitalWrite(valvulas, HIGH);    
        flagModo=true; EEPROM.write(0, flagModo);
        flagAlarma=false;
        break;
      }
    }
  }    
}

byte edit(unsigned int x_pos, unsigned int y_pos, byte sizeText, int parametro, const GFXfont *f){
  while(true){
    tft.setFont(f);      
    tft.setCursor(x_pos,y_pos);
    tft.setTextSize(sizeText);           
    tft.setTextColor(BLACK,WHITE); 
    printDigits(parametro);
    oscilarText();
    //JoystickX = analogRead(A6);
    JoystickY = analogRead(A7);

    if (JoystickY >530 && JoystickY <=1023) {        //¿SE MUEVE HACIA ABAJO?
      switch(i){
        case 1: 
          if(parametro < 1) parametro = 31; 
          tft.setCursor(x_pos,y_pos); 
          tft.setTextColor(WHITE,WHITE); 
          printDigits(parametro);
          parametro--;
          if(parametro < 1) parametro = 31;
          delay(100); 
          break;
        case 2:
          if(parametro < 1) parametro = 12; 
          tft.setCursor(x_pos,y_pos); 
          tft.setTextColor(WHITE,WHITE); 
          printDigits(parametro);
          parametro--;
          if(parametro < 1) parametro = 12;
          delay(100); 
          break;
        case 3:
          if(parametro < 20) parametro = 30; 
          tft.setCursor(x_pos,y_pos); 
          tft.setTextColor(WHITE,WHITE); 
          printDigits(parametro);
          parametro--;
          if(parametro < 20) parametro = 30;
          delay(100); 
          break;
        case 4:
          if(parametro == 0) parametro = 24; 
          tft.setCursor(x_pos,y_pos); 
          tft.setTextColor(WHITE,WHITE); 
          printDigits(parametro);
          parametro--;
          if(parametro < 0) parametro = 23;
          delay(100);
          break;     
        default:
          if(parametro < 1) parametro = 60; 
          tft.setCursor(x_pos,y_pos); 
          tft.setTextColor(WHITE,WHITE); 
          printDigits(parametro);
          parametro--;
          if(parametro < 1) parametro = 60;
          delay(100);  
      }
    }
    if (JoystickY >= 0 && JoystickY < 470) {      //¿SE MUEVE HACIA ARRIBA?
      switch(i){
        case 1:
          tft.setCursor(x_pos,y_pos);
          tft.setTextColor(WHITE,WHITE); 
          printDigits(parametro);
          parametro++;
          if(parametro > 31) parametro = 1;
          delay(100);
          break; 
        case 2:
          tft.setCursor(x_pos,y_pos);
          tft.setTextColor(WHITE,WHITE); 
          printDigits(parametro);
          parametro++;
          if(parametro > 12) parametro = 1;
          delay(100); 
          break; 
        case 3:
          tft.setCursor(x_pos,y_pos);
          tft.setTextColor(WHITE,WHITE); 
          printDigits(parametro);
          parametro++;
          if(parametro > 30) parametro = 20;
          delay(100); 
          break; 
        case 4:
          tft.setCursor(x_pos,y_pos);
          tft.setTextColor(WHITE,WHITE); 
          printDigits(parametro);
          parametro++;
          if(parametro > 23) parametro = 0;
          delay(100);
          break; 
        default:
          tft.setCursor(x_pos,y_pos);
          tft.setTextColor(WHITE,WHITE); 
          printDigits(parametro);
          parametro++;
          if(parametro > 59) parametro = 0;
          delay(100); 
        } 
    }
    tft.setCursor(x_pos,y_pos);  
    tft.setTextColor(WHITE,WHITE); 
    printDigits(parametro);
    oscilarText();
    if(!digitalRead(pulsador)){
      while(!digitalRead(pulsador));
      tft.setCursor(x_pos,y_pos);  
      tft.setTextColor(BLACK,WHITE); 
      printDigits(parametro);    
      i++;
      if (i > 5) i=0;
      return parametro;
    }
  }
}  

void riegoManual(){
  countJoystickV=1;
  tft.fillScreen(WHITE);
  tft.fillRect(0,0,480,48,BLACK);
  tft.setTextColor(WHITE);     
  textSinClear(138, 12, "RIEGO MANUAL", 3, NULL);
  tft.setTextColor(BLACK,WHITE);         
  textSinClear(45, 80, "ACTIVAR", 2, NULL);
  textSinClear(45, 120, "DESACTIVAR", 2, NULL);
  while(!flagDashboard){
    JoystickY = analogRead(A7);
       //MOVIMIENTOS VERTICALES
    if (JoystickY >530 && JoystickY <=1023) {        //¿SE MUEVE HACIA ABAJO?
      delay(150);
      selectorRiegoManualDown();
      countJoystickV++;
      if(countJoystickV > 2) countJoystickV=1;
      }
    if (JoystickY >=0 && JoystickY <470) {     //¿SE MUEVE HACIA LA ARRIBA?
      delay(150);
      selectorRiegoManualUp();
      if (countJoystickV <=0) countJoystickV=3;
      countJoystickV--;
      if(countJoystickV < 1) countJoystickV=2;
      }
    if (!digitalRead(pulsador)){
      while(!digitalRead(pulsador));
      switch(IDSubSetting){
        case 1:   tft.fillScreen(WHITE); textSinClear(144, 128, "ACTIVADO", 4, NULL); digitalWrite(valvulas, LOW); delay(1500); estadoDispo = 0; EEPROM.write(5, estadoDispo); flagModo=false; EEPROM.write(0, flagModo); flagDashboard=true; flagSettings=false; flagAlarma=false; break;
        default:  tft.fillScreen(WHITE); textSinClear(118, 128, "DESACTIVADO", 4, NULL); digitalWrite(valvulas, HIGH); delay(1500); estadoDispo = 1; EEPROM.write(5, estadoDispo); flagModo=true; EEPROM.write(0, flagModo); flagDashboard=true; flagSettings=false; flagAlarma=false;  
      }
    }
  }
} 

void selectorRiegoManualDown(){
  switch(countJoystickV){
  case 1: limpiarSelector(); textSinClear(18, 80, ">", 2, NULL); IDSubSetting=1; break;
  default: limpiarSelector(); textSinClear(18, 120, ">", 2, NULL); IDSubSetting=2;
  }
}

void selectorRiegoManualUp(){
  switch(countJoystickV){
  case 1: limpiarSelector(); textSinClear(18, 120, ">", 2, NULL); IDSubSetting=2; break;
  default: limpiarSelector(); textSinClear(18, 80, ">", 2, NULL); IDSubSetting=1;
  }
}

void desactivar(){
  if (flagModo == false){
   digitalWrite(valvulas, HIGH);    
   tft.fillScreen(WHITE);
   textSinClear(144, 128, "ACTIVADO", 4, NULL);
   delay(1500);
   estadoDispo = 1; EEPROM.write(5, estadoDispo);
   flagModo=true; EEPROM.write(0, flagModo);
   flagDashboard=true;
   flagSettings=false;
   flagAlarma=false;  
   }
   else {
   digitalWrite(valvulas, HIGH);     
   tft.fillScreen(WHITE);
   textSinClear(118, 128, "DESACTIVADO", 4, NULL);
   delay(1500);
   estadoDispo = 0;  EEPROM.write(5, estadoDispo);
   flagModo=false; EEPROM.write(0, flagModo);
   flagDashboard=true;
   flagSettings=false;
   flagAlarma=false;     
   }
}

void programaSecuencia(){
  countJoystickV=1;
  tft.fillScreen(WHITE);
  tft.fillRect(0,0,480,48,BLACK);
  tft.setTextColor(WHITE);     
  textSinClear(72, 12, "PROGRAMAR SECUENCIA", 3, NULL);    
  tft.setTextColor(BLACK,WHITE);        
  textSinClear(45, 80, "HORA DE INICIO", 2, NULL);
  textSinClear(45, 120, "DURACION", 2, NULL);
  textSinClear(45, 160, "DIAS DE RIEGO", 2, NULL);
  textSinClear(45, 200, "BORRAR SECUENCIA", 2, NULL);  
  textSinClear(45, 240, "SALIR", 2, NULL);    
  
  while(modoProgSecuencia){
    //JoystickX = analogRead(A6);
    JoystickY = analogRead(A7);
       //MOVIMIENTOS VERTICALES
    if (JoystickY >530 && JoystickY <=1023) {        //¿SE MUEVE HACIA ABAJO?
      delay(150);
      selectorProgSecuenciaDown();
      countJoystickV++;
      if(countJoystickV > 5) countJoystickV=1;
      }
    if (JoystickY >=0 && JoystickY <470) {     //¿SE MUEVE HACIA LA ARRIBA?
      delay(150);
      selectorProgSecuenciaUp();
      if (countJoystickV <=0) countJoystickV=6;
      countJoystickV--;
      if(countJoystickV < 1) countJoystickV=5;
      }
    if (!digitalRead(pulsador)){
      while(!digitalRead(pulsador));
      switch(IDSubSetting){
        case 1: agregarHoraInicio(); modoProgSecuencia=false; break;
        case 2: agregarDuracion(); modoProgSecuencia=false; break;
        case 3: agregarDiasRiego(); modoProgSecuencia=false; break;
        case 4: borrarSecuencia(); modoProgSecuencia=false; break;    
        default: modoProgSecuencia=false;
      }
    }
  }
}

void selectorProgSecuenciaDown(){
  switch(countJoystickV){
  case 1: limpiarSelector(); textSinClear(18, 80, ">", 2, NULL); IDSubSetting=1; break;
  case 2: limpiarSelector(); textSinClear(18, 120, ">", 2, NULL); IDSubSetting=2; break;
  case 3: limpiarSelector(); textSinClear(18, 160, ">", 2, NULL); IDSubSetting=3; break;
  case 4: limpiarSelector(); textSinClear(18, 200, ">", 2, NULL); IDSubSetting=4; break;
  default: limpiarSelector(); textSinClear(18, 240, ">", 2, NULL); IDSubSetting=5;   
  }
}

void selectorProgSecuenciaUp(){
  switch(countJoystickV){
  case 1: limpiarSelector(); textSinClear(18, 240, ">", 2, NULL); IDSubSetting=5; break;
  case 2: limpiarSelector(); textSinClear(18, 80, ">", 2, NULL); IDSubSetting=1; break;
  case 3: limpiarSelector(); textSinClear(18, 120, ">", 2, NULL); IDSubSetting=2; break;
  case 4: limpiarSelector(); textSinClear(18, 160, ">", 2, NULL); IDSubSetting=3; break;
  default: limpiarSelector(); textSinClear(18, 200, ">", 2, NULL); IDSubSetting=4;      
  }
}

void agregarHoraInicio(){
    tft.fillScreen(WHITE);
    tft.fillRect(0,0,480,48,BLACK);
    tft.setTextColor(WHITE);     
    textSinClear(114, 12, "HORA DE INICIO", 3, NULL);    
    tft.setTextColor(BLACK,WHITE);   
  while(true){
    horarioSecuencia(horaInicio, minutoInicio);
    i=4; 
    horaInicio = edit(75, 225, 2, horaInicio, &FreeSevenSegNumFont);
    if(horaInicio==24) {horaInicio=0; EEPROM.write(1, horaInicio);}
    else {EEPROM.write(1, horaInicio);}
    i=5;
    minutoInicio = edit(275, 225, 2, minutoInicio, &FreeSevenSegNumFont);                
    if(minutoInicio==60) {minutoInicio=0; EEPROM.write(2, minutoInicio);}
    else {EEPROM.write(2, minutoInicio);}    
    while(true){
      JoystickX = analogRead(A6);
      //JoystickY = analogRead(A7);
      tft.setTextColor(WHITE, WHITE); 
      textSinClear(312, 272, "GUARDAR", 3, NULL);   
      oscilarText();
      tft.setTextColor(BLACK, WHITE); 
      textSinClear(312, 272, "GUARDAR", 3, NULL); 
      oscilarText();
      if(JoystickX >= 0 && JoystickX < 400){        // SE MUEVE HACIA LA IZQUIERDA
        break;
      }
      if(!digitalRead(pulsador)){
        while(!digitalRead(pulsador));
        return;
      }
    }
  }
}

void horarioSecuencia(byte horaInicio, byte minutoInicio){   
   textSinClear(110, 85, "HORA", 3, NULL);
   textSinClear(285, 85, "MINUTO", 3, NULL);    
   tft.setFont(&FreeSevenSegNumFont);
   tft.setTextSize(2);     
   tft.setCursor(75, 225);        
   if(horaInicio==24) printDigits(horaInicio-24);    
   else {printDigits(horaInicio);}   
   textSinClear(214, 140, ":", 10, NULL);
   tft.setFont(&FreeSevenSegNumFont);   
   tft.setTextSize(2);   
   tft.setCursor(275, 225);
   if(minutoInicio==60) printDigits(minutoInicio-60);    
   else {printDigits(minutoInicio);}         
   textSinClear(312, 272, "GUARDAR", 3, NULL); 
}

void agregarDuracion(){
    tft.fillScreen(WHITE);
    tft.fillRect(0,0,480,48,BLACK);
    tft.setTextColor(WHITE);     
    textSinClear(168, 12, "DURACION", 3, NULL);    
    tft.setTextColor(BLACK,WHITE);  
  while(true){
    duracionSecuencia(duracionHoras, duracionMinutos);
    i=4;
    duracionHoras=edit(75, 225, 2, duracionHoras, &FreeSevenSegNumFont);
    if(duracionHoras==24) {duracionHoras=0; EEPROM.write(3, duracionHoras);}
    else {EEPROM.write(3, duracionHoras);}                  
    i=5;
    duracionMinutos=edit(275, 225, 2, duracionMinutos, &FreeSevenSegNumFont);
    if(duracionMinutos==60) {duracionMinutos=0; EEPROM.write(4, duracionMinutos);}  
    else {EEPROM.write(4, duracionMinutos);}                       
    while(true){
      JoystickX = analogRead(A6);
      //JoystickY = analogRead(A7);
      tft.setTextColor(WHITE, WHITE); 
      textSinClear(312, 272, "GUARDAR", 3, NULL);
      oscilarText();
      tft.setTextColor(BLACK, WHITE); 
      textSinClear(312, 272, "GUARDAR", 3, NULL);
      oscilarText();
      if(JoystickX >= 0 && JoystickX < 400){
        break;
      }
      if(!digitalRead(pulsador)){
        while(!digitalRead(pulsador));
        return;
      }
    }
  }  
}

void duracionSecuencia(byte duracionHoras, byte duracionMinutos){
   textSinClear(110, 85, "HORA", 3, NULL);
   textSinClear(285, 85, "MINUTO", 3, NULL);    
   tft.setFont(&FreeSevenSegNumFont);
   tft.setTextSize(2);     
   tft.setCursor(75, 225);        
   if(duracionHoras==24) printDigits(duracionHoras-24);    
   else {printDigits(duracionHoras);}   
   textSinClear(214, 140, ":", 10, NULL);
   tft.setFont(&FreeSevenSegNumFont);   
   tft.setTextSize(2);   
   tft.setCursor(275, 225); 
   if(duracionMinutos==60) printDigits(duracionMinutos-60);    
   else {printDigits(duracionMinutos);}         
   textSinClear(312, 272, "GUARDAR", 3, NULL);    
}   

void agregarDiasRiego(){  
  countJoystickV=1;
  mostrarDiasRiego();
  diaSemanaRiego=1;
  while(true){
     //JoystickX = analogRead(A6);
     JoystickY = analogRead(A7);
     tft.setTextColor(WHITE, WHITE); 
     mostrarDiasRiego2();
     oscilarText();
     if (JoystickY >= 0 && JoystickY < 470) {         //¿SE MUEVE HACIA ARRIBA?
       if(diaSemanaRiego < 1) diaSemanaRiego = 9;   
       tft.setTextColor(BLACK, WHITE); 
       mostrarDiasRiego2();
       diaSemanaRiego--;
       if(diaSemanaRiego < 1) diaSemanaRiego = 8 ;
       delay(100);   
     }
     if (JoystickY >530 && JoystickY <= 1023) {      //¿SE MUEVE HACIA ABAJO?
       tft.setTextColor(BLACK, WHITE); 
       mostrarDiasRiego2();
       diaSemanaRiego++;
       if(diaSemanaRiego > 8) diaSemanaRiego = 1;
       delay(100);      
     }
       tft.setTextColor(BLACK, WHITE); 
       mostrarDiasRiego2();
       oscilarText();
     if(!digitalRead(pulsador)){
       while(!digitalRead(pulsador));
       tft.setTextColor(BLACK, WHITE); 
       mostrarDiasRiego2();
       switch(diaSemanaRiego){
         case 1:
         diasRiego[0]=!diasRiego[0]; EEPROM.write(6, diasRiego[0]);
         selectorDiasRiego();
         break;         

         case 2:
         diasRiego[1]=!diasRiego[1]; EEPROM.write(7, diasRiego[1]);
         selectorDiasRiego();
         break;  
          
         case 3:
         diasRiego[2]=!diasRiego[2]; EEPROM.write(8, diasRiego[2]);
         selectorDiasRiego();
         break;       

         case 4:
         diasRiego[3]=!diasRiego[3]; EEPROM.write(9, diasRiego[3]); 
         selectorDiasRiego();
         break;          

         case 5:
         diasRiego[4]=!diasRiego[4]; EEPROM.write(10, diasRiego[4]);
         selectorDiasRiego();
         break;

         case 6:
         diasRiego[5]=!diasRiego[5]; EEPROM.write(11, diasRiego[5]);
         selectorDiasRiego();
         break;
          
         case 7:
         diasRiego[6]=!diasRiego[6]; EEPROM.write(12, diasRiego[6]);
         selectorDiasRiego();
         break;

         default:
         delay(250);
         return;
       }
     }
   }
}

void selectorDiasRiego(){
  switch(diaSemanaRiego){
    case 1:
    if(diasRiego[0]==HIGH) {
      marcaDiasRiego[0]='X'; EEPROM.write(20, marcaDiasRiego[0]);
      lunes='L'; EEPROM.write(13, lunes);
      tft.setTextColor(BLACK, WHITE);  
      tft.setTextSize(2);     
      tft.setCursor(64, 80);
      tft.print(marcaDiasRiego[0]);
      break;
    }
    else {
      tft.setTextColor(WHITE);
      tft.setTextSize(2);     
      tft.setCursor(64, 80);
      tft.print(marcaDiasRiego[0]);     
      marcaDiasRiego[0]=' '; EEPROM.write(20, marcaDiasRiego[0]);
      lunes='_'; EEPROM.write(13, lunes);        
      tft.setTextColor(BLACK, WHITE);          
      break;      
    }
    
    case 2:
    if(diasRiego[1]==HIGH) {
      marcaDiasRiego[1]='X'; EEPROM.write(21, marcaDiasRiego[1]);
      martes='M'; EEPROM.write(14, martes);
      tft.setTextColor(BLACK, WHITE);             
      tft.setTextSize(2);     
      tft.setCursor(64, 120);
      tft.print(marcaDiasRiego[1]);
      break;      
    }
    else {
      tft.setTextColor(WHITE);
      tft.setTextSize(2);     
      tft.setCursor(64, 120);
      tft.print(marcaDiasRiego[1]);     
      marcaDiasRiego[1]=' '; EEPROM.write(21, marcaDiasRiego[1]);
      martes='_'; EEPROM.write(14, martes);          
      tft.setTextColor(BLACK, WHITE);           
      break;        
    }
        
    case 3:
    if(diasRiego[2]==HIGH) {
      marcaDiasRiego[2]='X'; EEPROM.write(22, marcaDiasRiego[2]);
      miercoles='M'; EEPROM.write(15, miercoles); 
      tft.setTextColor(BLACK, WHITE);               
      tft.setTextSize(2);     
      tft.setCursor(64, 160);
      tft.print(marcaDiasRiego[2]);
      break;        
    }
    else {
      tft.setTextColor(WHITE);
      tft.setTextSize(2);     
      tft.setCursor(64, 160);
      tft.print(marcaDiasRiego[2]);     
      marcaDiasRiego[2]=' '; EEPROM.write(22, marcaDiasRiego[2]);  
      miercoles='_'; EEPROM.write(15, miercoles);  
      tft.setTextColor(BLACK, WHITE);          
      break;       
    }

    case 4:
    if(diasRiego[3]==HIGH) {
      marcaDiasRiego[3]='X'; EEPROM.write(23, marcaDiasRiego[3]);
      jueves='J'; EEPROM.write(16, jueves); 
      tft.setTextColor(BLACK, WHITE);          
      tft.setTextSize(2);     
      tft.setCursor(64, 200);
      tft.print(marcaDiasRiego[3]);
      break; 
    }
    else {
      tft.setTextColor(WHITE);
      tft.setTextSize(2);     
      tft.setCursor(64, 200);
      tft.print(marcaDiasRiego[3]);     
      marcaDiasRiego[3]=' '; EEPROM.write(23, marcaDiasRiego[3]);
      jueves='_'; EEPROM.write(16, jueves);           
      tft.setTextColor(BLACK, WHITE);          
      break; 
    }

    case 5:
    if(diasRiego[4]==HIGH) {
      marcaDiasRiego[4]='X'; EEPROM.write(24, marcaDiasRiego[4]);
      viernes='V'; EEPROM.write(17, viernes);  
      tft.setTextColor(BLACK, WHITE);                 
      tft.setTextSize(2);     
      tft.setCursor(64, 240);
      tft.print(marcaDiasRiego[4]);
      break; 
    }
    else {
      tft.setTextColor(WHITE);
      tft.setTextSize(2);     
      tft.setCursor(64, 240);
      tft.print(marcaDiasRiego[4]);     
      marcaDiasRiego[4]=' '; EEPROM.write(24, marcaDiasRiego[4]); 
      viernes='_'; EEPROM.write(17, viernes);          
      tft.setTextColor(BLACK, WHITE);          
      break; 
    }

    case 6:
    if(diasRiego[5]==HIGH) {
      marcaDiasRiego[5]='X'; EEPROM.write(25, marcaDiasRiego[5]);
      sabado='S'; EEPROM.write(18, sabado);  
      tft.setTextColor(BLACK, WHITE);               
      tft.setTextSize(2);     
      tft.setCursor(306, 80);
      tft.print(marcaDiasRiego[5]);
      break;
    }
    else {
      tft.setTextColor(WHITE);
      tft.setTextSize(2);     
      tft.setCursor(306, 80);
      tft.print(marcaDiasRiego[5]);     
      marcaDiasRiego[5]=' '; EEPROM.write(25, marcaDiasRiego[5]);  
      sabado='_'; EEPROM.write(18, sabado);         
      tft.setTextColor(BLACK, WHITE);          
      break;    
    }

    case 7:
    if(diasRiego[6]==HIGH) {
      marcaDiasRiego[6]='X'; EEPROM.write(26, marcaDiasRiego[6]);
      domingo='D'; EEPROM.write(19, domingo);  
      tft.setTextColor(BLACK, WHITE);                 
      tft.setTextSize(2);     
      tft.setCursor(306, 120);
      tft.print(marcaDiasRiego[6]);
      break;
    }
    else {
      tft.setTextColor(WHITE);
      tft.setTextSize(2);     
      tft.setCursor(306, 120);
      tft.print(marcaDiasRiego[6]);     
      marcaDiasRiego[6]=' '; EEPROM.write(26, marcaDiasRiego[6]); 
      domingo='_'; EEPROM.write(19, domingo);         
      tft.setTextColor(BLACK, WHITE);          
      break;  
    }
    
    default:
    delay(200);
  }
}   

void mostrarDiasRiego(){
  tft.fillScreen(WHITE);
  tft.fillRect(0,0,480,48,BLACK);
  tft.setTextColor(WHITE);     
  textSinClear(132, 12, "DIAS DE RIEGO", 3, NULL);    
  tft.setTextColor(BLACK,WHITE); 
  textSinClear(45, 80, "(  ) LUNES", 2, NULL); 
  tft.setCursor(64, 80);
  tft.print(marcaDiasRiego[0]);
  textSinClear(45, 120, "(  ) MARTES", 2, NULL);
  tft.setCursor(64, 120);
  tft.print(marcaDiasRiego[1]);       
  textSinClear(45, 160, "(  ) MIERCOLES", 2, NULL);  
  tft.setCursor(64, 160);
  tft.print(marcaDiasRiego[2]);           
  textSinClear(45, 200, "(  ) JUEVES", 2, NULL);  
  tft.setCursor(64, 200);
  tft.print(marcaDiasRiego[3]);    
  textSinClear(45, 240, "(  ) VIERNES", 2, NULL);
  tft.setCursor(64, 240);
  tft.print(marcaDiasRiego[4]);     
  textSinClear(288, 80, "(  ) SABADO", 2, NULL);  
  tft.setCursor(306, 80);
  tft.print(marcaDiasRiego[5]);        
  textSinClear(288, 120, "(  ) DOMINGO", 2, NULL); 
  tft.setCursor(306, 120);
  tft.print(marcaDiasRiego[6]);          
  textSinClear(312, 272, "GUARDAR", 3, NULL);    
}

void mostrarDiasRiego2(){
  switch(diaSemanaRiego){
  case 1: textSinClear(105, 80, "LUNES", 2, NULL); break;
  case 2: textSinClear(105, 120, "MARTES", 2, NULL); break;
  case 3: textSinClear(105, 160, "MIERCOLES", 2, NULL); break;
  case 4: textSinClear(105, 200, "JUEVES", 2, NULL); break;
  case 5: textSinClear(105, 240, "VIERNES", 2, NULL); break;
  case 6: textSinClear(348, 80, "SABADO", 2, NULL); break;
  case 7: textSinClear(348, 120, "DOMINGO", 2, NULL); break;
  default: textSinClear(312, 272, "GUARDAR", 3, NULL);              
  }
}  

void borrarSecuencia(){
   
  horaInicio=0; EEPROM.write(1, horaInicio);
  minutoInicio=0; EEPROM.write(2, minutoInicio);
  duracionHoras=0; EEPROM.write(3, duracionHoras);
  duracionMinutos=0; EEPROM.write(4, duracionMinutos);     
  
  diasRiego[0]=LOW; EEPROM.write(6, diasRiego[0]);
  diasRiego[1]=LOW; EEPROM.write(7, diasRiego[1]);
  diasRiego[2]=LOW; EEPROM.write(8, diasRiego[2]);
  diasRiego[3]=LOW; EEPROM.write(9, diasRiego[3]);
  diasRiego[4]=LOW; EEPROM.write(10, diasRiego[4]);
  diasRiego[5]=LOW; EEPROM.write(11, diasRiego[5]);
  diasRiego[6]=LOW; EEPROM.write(12, diasRiego[6]);

  marcaDiasRiego[0]=' '; EEPROM.write(20, marcaDiasRiego[0]);
  marcaDiasRiego[1]=' '; EEPROM.write(21, marcaDiasRiego[1]);
  marcaDiasRiego[2]=' '; EEPROM.write(22, marcaDiasRiego[2]);
  marcaDiasRiego[3]=' '; EEPROM.write(23, marcaDiasRiego[3]);
  marcaDiasRiego[4]=' '; EEPROM.write(24, marcaDiasRiego[4]);
  marcaDiasRiego[5]=' '; EEPROM.write(25, marcaDiasRiego[5]);
  marcaDiasRiego[6]=' '; EEPROM.write(26, marcaDiasRiego[6]);

  estadoDispo = 0;  EEPROM.write(5, estadoDispo);
  flagModo=false; EEPROM.write(0, flagModo);
  flagDashboard=true;
  flagSettings=false;
  flagAlarma=false;  

  lunes='_'; EEPROM.write(13, lunes);
  martes='_'; EEPROM.write(14, martes);
  miercoles='_'; EEPROM.write(15, miercoles);
  jueves='_'; EEPROM.write(16, jueves);
  viernes='_'; EEPROM.write(17, viernes); 
  sabado='_'; EEPROM.write(18, sabado); 
  domingo='_'; EEPROM.write(19, domingo);
  
  tft.fillScreen(WHITE);
  tft.setTextColor(BLACK, WHITE);   
  textSinClear(132, 150, "SECUENCIA BORRADA", 2, NULL);
  delay(1500);  
}
