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
#define valvula_1    12

#define BLACK   0x0000
#define RED     0xF800
#define GREEN   0x07E0
#define WHITE   0xFFFF
#define LCD_CS A3 // Chip Select goes to Analog 3
#define LCD_CD A2 // Command/Data goes to Analog 2 (RS)
#define LCD_WR A1 // LCD Write goes to Analog 1
#define LCD_RD A0 // LCD Read goes to Analog 0
//#define LCD_RESET A4 // Can alternately just connect to Arduino's reset pin

#define EEPROM_hora_Inicio  1

#define EEPROM_minuto_Inicio  5

#define EEPROM_duracion_Horas  9

#define EEPROM_duracion_Minutos  13

#define EEPROM_dias_Riego_0  17

#define EEPROM_dias_Riego_1  21

#define EEPROM_dias_Riego_2  25

#define EEPROM_dias_Riego_3  29

#define EEPROM_dias_Riego_4  33

#define EEPROM_dias_Riego_5  37

#define EEPROM_dias_Riego_6  41

#define EEPROM_marca_Dias_Riego_0  45

#define EEPROM_marca_Dias_Riego_1  49

#define EEPROM_marca_Dias_Riego_2  53

#define EEPROM_marca_Dias_Riego_3  57

#define EEPROM_marca_Dias_Riego_4  61

#define EEPROM_marca_Dias_Riego_5  65

#define EEPROM_marca_Dias_Riego_6  69

#define EEPROM_lunes  74

#define EEPROM_martes  78

#define EEPROM_miercoles  82

#define EEPROM_jueves  86

#define EEPROM_viernes  90

#define EEPROM_sabado  94

#define EEPROM_domingo  98

#define EEPROM_flagModo1  102

#define EEPROM_sector1  106

#define EEPROM_sector_Marca1  110

unsigned int JoystickX, JoystickY;
byte IDSubSetting=0, i=0;
byte countJoystickV=1, countJoystickH=1;
byte alarmaHoras=0, alarmaMinutos=0;
byte diaDelaSemana, diaSemanaRiego, contadorSector, flagConfigDate=0;
int dia, mes, anio, hora, minuto;
byte diaSemana, segundo=0;
byte diaSemanaAlarma[] = {LOW,LOW,LOW,LOW,LOW,LOW,LOW};
boolean flagDashboard=true, flagSettings=false, flagAlarma=false, JoystickSW, flagSubDashboard=false, flagSettings2=false;

//Variables en EEPROM
boolean flagModo=false; // Boolean que indica si el modo Automático está activo: True:Activo / False: Desactivado

byte hora_Inicio=0, minuto_Inicio=0, duracion_Horas=0, duracion_Minutos=0;

byte dias_Riego[] = {LOW,LOW,LOW,LOW,LOW,LOW,LOW};

byte sector[] = {LOW,LOW,LOW,LOW};
char sector_Marca[] = {' ', ' ', ' ', ' '};

//Variables usadas para el selector de días
char marca_Dias_Riego[] = {' ', ' ', ' ', ' ', ' ', ' ', ' '};

//Variables usadas para mostrar en el Dashboard
char dia_Semana[]={'_', '_', '_', '_', '_', '_', '_'};


//Limites de X
#define X_LowerLimit 380
#define X_UpperLimit 620

//Limites de Y
#define Y_LowerLimit 380
#define Y_UpperLimit 620

void setup() {
  pinMode(pulsador, INPUT_PULLUP);
  pinMode(valvula_1, OUTPUT);     
  digitalWrite(valvula_1,HIGH);
  
  Serial.begin(9600);      

//Inicializar por primera vez la eeprom del ATmega328p
  if (EEPROM.read(EEPROM_hora_Inicio) != 255 || EEPROM.read(EEPROM_lunes) != 255)
    readEEPROM();
  else
    writeEEPROM();

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

void checkTiempoAlarma(byte *alarma_H, byte *alarma_M, byte *duracion_H, byte *duracion_M){
  byte carryH = 0;
  byte carryM = 0;
  DateTime fechaNow = rtc.now();   
  if ((fechaNow.minute() + *duracion_M) >= 60){
    carryH = 1;
    if (*duracion_M > 1) {
      carryM = (*duracion_M - (60-fechaNow.minute()));
    }
    else {carryM = 0;}
  }
  
  *alarma_H = (fechaNow.hour() + *duracion_H + carryH);
  if (*alarma_H >= 24) *alarma_H = *alarma_H - 24;
  if ((fechaNow.minute() + *duracion_M) >= 60) {*alarma_M = carryM;}
  else {
    *alarma_M = (fechaNow.minute() + *duracion_M);
  }
  //Serial.println(*alarma_H);
  //Serial.println(*alarma_M);  
}    

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

void validarAlarmasRiego(){
//################### VALIDAR ALARMA PARA RIEGO ###################  
  DateTime fechaNow = rtc.now(); 
  if (fechaNow.hour()==hora_Inicio && fechaNow.minute()==minuto_Inicio && !flagAlarma){
    validarDiaAlarma();
    if (((diaSemanaAlarma[0] && dias_Riego[0])==HIGH || (diaSemanaAlarma[1] && dias_Riego[1])==HIGH || (diaSemanaAlarma[2] && dias_Riego[2])==HIGH ||
    (diaSemanaAlarma[3] && dias_Riego[3])==HIGH || (diaSemanaAlarma[4] && dias_Riego[4])==HIGH || (diaSemanaAlarma[5] && dias_Riego[5])==HIGH ||
    (diaSemanaAlarma[6] && dias_Riego[6])==HIGH) && !flagAlarma && flagModo && (duracion_Horas+duracion_Minutos) != 0){
        checkTiempoAlarma(&alarmaHoras, &alarmaMinutos, &duracion_Horas, &duracion_Minutos);
        flagAlarma=true;
        digitalWrite(valvula_1,LOW);
    }
  }

  if (flagAlarma){
    validarAlarmasRiego2(alarmaHoras, alarmaMinutos, &flagAlarma, valvula_1);
  }
}

void validarAlarmasRiego2(byte Alarm_H, byte Alarm_M, boolean *FlagAlarm, int Valvulas){
  DateTime fechaNow = rtc.now();
  if (fechaNow.hour()==Alarm_H && fechaNow.minute()==Alarm_M && *FlagAlarm){
      DateTime fechaNow = rtc.now(); 
      digitalWrite(Valvulas,HIGH);
      *FlagAlarm=false;
  }
}

void loop() {
  //Serial.println("Empieza Programa");   
  while (flagDashboard){
  
    validarAlarmasRiego();
   
    dashboard();   
    JoystickX = analogRead(A6);
    //JoystickY = analogRead(A7);
  
    //MOVIMIENTOS HORIZONTALES
    if (JoystickX >=0 && JoystickX <X_LowerLimit) {        //¿SE MUEVE HACIA LA IZQUIERDA?
      delay(250);
      flagDashboard=false;
      flagSettings=true;
      flagSubDashboard=false;
      tft.fillScreen(WHITE);
      }
   
    if (JoystickX >X_UpperLimit && JoystickX <=1023) {     //¿SE MUEVE HACIA LA DERECHA?
      delay(250);
      flagDashboard=false;
      flagSettings=true;
      flagSubDashboard=false;    
      tft.fillScreen(WHITE);
    }
  }

  while (flagSettings){
  
    validarAlarmasRiego();
    
    if (!flagSettings2){
      flagSettings2=true;  
      settings();
      selectorSettings();
    }
    
    JoystickX = analogRead(A6);
    JoystickY = analogRead(A7);
  
    //MOVIMIENTOS VERTICALES
    if (JoystickY >Y_UpperLimit && JoystickY <=1023) {        //¿SE MUEVE HACIA ARRIBA? 
      delay(200);
      if(countJoystickV >= 4) countJoystickV=0;
      countJoystickV++;
      selectorSettings();
    }
   
    if (JoystickY >=0 && JoystickY <Y_LowerLimit) {     //¿SE MUEVE HACIA ABAJO?
      delay(200);
      if (countJoystickV <= 1) countJoystickV=5;
      countJoystickV--;
      selectorSettings();    
    }
    
    //MOVIMIENTOS HORIZONTALES
    if (JoystickX >=0 && JoystickX <X_LowerLimit) {        //¿SE MUEVE HACIA LA IZQUIERDA?
      delay(250);
      flagDashboard=true;
      flagSettings=false;
      flagSettings2=false;    
      tft.fillScreen(WHITE);
      }
   
    if (JoystickX >X_UpperLimit && JoystickX <=1023) {     //¿SE MUEVE HACIA LA DERECHA?
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

void selectorSettings(){
  switch(countJoystickV){
  case 1: limpiarSelector(); textSinClear(18, 80, ">", 2, NULL); IDSubSetting=1; break;
  case 2: limpiarSelector(); textSinClear(18, 120, ">", 2, NULL); IDSubSetting=2; break;
  case 3: limpiarSelector(); textSinClear(18, 160, ">", 2, NULL); IDSubSetting=3; break;
  case 4: limpiarSelector(); textSinClear(18, 200, ">", 2, NULL); IDSubSetting=4; break;
  }
}

void subSettings(){
  switch(IDSubSetting){
  case 1: fechaHora(); tft.fillScreen(WHITE); countJoystickV=1; break;           // CONFIGURAR FECHA Y HORA
  case 2: programaSecuencia(); tft.fillScreen(WHITE); countJoystickV=1; break;   // CONFIGURAR SECUENCIA
  case 3: riegoManual(); tft.fillScreen(WHITE); countJoystickV=1; break;         // ACTIVAR/DESACTIVAR RIEGO MANUAL
  default: desactivar(); tft.fillScreen(WHITE); countJoystickV=1;                // ACTIVAR/DESACTIVAR SISTEMA
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
    tft.fillRect(0,0,480,48,BLACK);
    tft.setTextColor(WHITE);     
    textSinClear(84, 12, "CONFIGURAR SISTEMA", 3, NULL);  
    tft.setTextColor(BLACK,WHITE);        
    textSinClear(45, 80, "FECHA Y HORA", 2, NULL);
    textSinClear(45, 120, "PROGRAMAR SECUENCIA", 2, NULL);
    textSinClear(45, 160, "RIEGO MANUAL", 2, NULL);
    textSinClear(45, 200, "ACTIVAR SECTORES", 2, NULL);      
}

void dashboard(){
   DateTime fechaNow = rtc.now(); 
   diaSemana = fechaNow.dayOfTheWeek(); dia=fechaNow.day(); mes=fechaNow.month(); anio=fechaNow.year()-2000; hora=fechaNow.hour(); minuto=fechaNow.minute();
   tft.setTextColor(BLACK,WHITE);  
   display_day(164,130);
   tft.setTextSize(4);     
   tft.setCursor(128, 180);                   
   printDigits(dia);
   tft.print('-');
   printDigits(mes);
   tft.print("-20"); 
   tft.print(anio);    
   tft.setCursor(156, 220);     
   //if (hora==24) hora=0;            
   printDigits(hora);
   tft.print(':');      
   printDigits(minuto);
   tft.print(':');      
   printDigits(fechaNow.second());  
   if (digitalRead(valvula_1)==LOW) {
      tft.setTextColor(BLACK,WHITE); 
      textSinClear(180, 272, "REGANDO!", 3, NULL);
   }
   else {
    tft.setTextColor(WHITE,WHITE); 
    textSinClear(180, 272, "REGANDO!", 3, NULL);
   }
   subDashboard();
}

void subDashboard2(char diaSemanaArray[], byte pos_Y, byte HoraInicio, byte MinutoInicio, byte DuracionHora, byte DuracionMinuto, boolean fModo){
  tft.setTextColor(WHITE);   
  textSinClear(31, pos_Y, "D:", 2, NULL);
  tft.print(diaSemanaArray[0]); //Lunes
  tft.print(diaSemanaArray[1]); //Martes
  tft.print(diaSemanaArray[2]); //Miercoles
  tft.print(diaSemanaArray[3]); //Jueves
  tft.print(diaSemanaArray[4]); //Viernes
  tft.print(diaSemanaArray[5]); //Sabado
  tft.print(diaSemanaArray[6]); //Domingo

  tft.setCursor(168,pos_Y);
  tft.print("H:");  
  if (HoraInicio==24) printDigits(HoraInicio-24);
  else {printDigits(HoraInicio);}
  tft.print(':');
  printDigits(MinutoInicio);
 
  tft.setCursor(276,pos_Y);
  tft.print("T:");  
  if (DuracionHora==24) printDigits(DuracionHora-24);
  else {printDigits(DuracionHora);}
  tft.print(':');
  printDigits(DuracionMinuto);
  
  tft.setCursor(384,pos_Y);
  if (!fModo) tft.print("E:OFF");
  else {tft.print("E:ON");}  
}

void subDashboard(){
  if (flagSubDashboard==false){
  
    flagSubDashboard=true;
    tft.fillRect(0,0,480,48,BLACK);
    subDashboard2(dia_Semana, 15, hora_Inicio, minuto_Inicio, duracion_Horas, duracion_Minutos, flagModo);  
  }
}

void display_day(unsigned int x_pos, unsigned int y_pos){
  switch(diaSemana){
    case 1: textSinClear(x_pos, y_pos, "  LUNES    ", 3, NULL); break;
    case 2: textSinClear(x_pos, y_pos, "  MARTES   ", 3, NULL); break;
    case 3: textSinClear(x_pos, y_pos, "MIERCOLES", 3, NULL); break;
    case 4: textSinClear(x_pos, y_pos, "  JUEVES   ", 3, NULL); break;
    case 5: textSinClear(x_pos, y_pos, " VIERNES   ", 3, NULL); break;
    case 6: textSinClear(x_pos, y_pos, "  SABADO   ", 3, NULL); break;
    default: textSinClear(x_pos, y_pos, " DOMINGO  ", 3, NULL);
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
  while (y<10 && JoystickY >=Y_LowerLimit && JoystickY <Y_UpperLimit && digitalRead(pulsador)){
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
      if(JoystickX >= 0 && JoystickX < X_LowerLimit){
        delay(300);
        break;
      }
      if(!digitalRead(pulsador)){
        while(!digitalRead(pulsador));
        rtc.adjust(DateTime(anio, mes, dia, hora, minuto, segundo));
        flagConfigDate=0;       
        resetFlags(NULL, false, HIGH);
        break;
      }
    }
  }    
}

void resetFlags(boolean Flag_Modo, boolean Flag_Alarma, boolean Estado_Valvulas){
  if (Flag_Modo != NULL){
    flagModo=Flag_Modo;
  }
 
  if (Flag_Alarma != NULL){
    flagAlarma=Flag_Alarma; 
  }
  
  digitalWrite(valvula_1, Estado_Valvulas);
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

    if (JoystickY >Y_UpperLimit && JoystickY <=1023) {        //¿SE MUEVE HACIA ABAJO?
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
    if (JoystickY >=0 && JoystickY <Y_LowerLimit) {      //¿SE MUEVE HACIA ARRIBA?
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
  selectorRiegoManual();
  
  while(!flagDashboard){
    JoystickY = analogRead(A7);
       //MOVIMIENTOS VERTICALES
    if (JoystickY >Y_UpperLimit && JoystickY <= 1023) {        //¿SE MUEVE HACIA ABAJO?
      delay(200);
      if(countJoystickV >= 2) countJoystickV=0;
      countJoystickV++;
      selectorRiegoManual();      
    }
    if (JoystickY >=0 && JoystickY < Y_LowerLimit) {     //¿SE MUEVE HACIA LA ARRIBA?
      delay(200);
      if (countJoystickV <=1) countJoystickV=3;
      countJoystickV--;
      selectorRiegoManual();      
    }
    if (!digitalRead(pulsador)){
      while(!digitalRead(pulsador));
      switch(IDSubSetting){
        case 1:
        tft.fillScreen(WHITE); 
        textSinClear(144, 128, "ACTIVADO", 4, NULL); 
        delay(1500);
        resetFlags(false, false, LOW);                     
        flagDashboard=true; 
        flagSettings=false; 
        break;
        
        default:  
        tft.fillScreen(WHITE); 
        textSinClear(118, 128, "DESACTIVADO", 4, NULL); 
        delay(1500); 
        resetFlags(false, NULL, HIGH);  
        flagModo=EEPROM.read(EEPROM_flagModo1);                      
        flagDashboard=true; 
        flagSettings=false; 
      }
    }
  }
} 

void selectorRiegoManual(){
  switch(countJoystickV){
  case 1: limpiarSelector(); textSinClear(18, 80, ">", 2, NULL); IDSubSetting=1; break;
  case 2: limpiarSelector(); textSinClear(18, 120, ">", 2, NULL); IDSubSetting=2; break;
  }
}

void desactivar (){ 
  digitalWrite(valvula_1, HIGH);
  flagDashboard=true;
  flagSettings=false;
  flagAlarma=false;
   
  countJoystickV=1;
  mostrarSectoresActivar();
  contadorSector=1;
  while(true){
     JoystickY = analogRead(A7);
     tft.setTextColor(WHITE, WHITE); 
     mostrarSectores();
     oscilarText();
     if (JoystickY >=0 && JoystickY <Y_LowerLimit) {         //¿SE MUEVE HACIA ARRIBA?
       if(contadorSector < 1) contadorSector = 3;   
       tft.setTextColor(BLACK, WHITE); 
       mostrarSectores();
       contadorSector--;
       if(contadorSector < 1) contadorSector = 2 ;
       delay(200);   
     }
     if (JoystickY >Y_UpperLimit && JoystickY <= 1023) {      //¿SE MUEVE HACIA ABAJO?
       tft.setTextColor(BLACK, WHITE); 
       mostrarSectores();
       contadorSector++;
       if(contadorSector > 2) contadorSector = 1;
       delay(200);      
     }
       tft.setTextColor(BLACK, WHITE); 
       mostrarSectores();
       oscilarText();
     
     if(!digitalRead(pulsador)){
       while(!digitalRead(pulsador));
       tft.setTextColor(BLACK, WHITE); 
       mostrarSectores();
       
       switch(contadorSector){
         case 1:
         sector[0]=!sector[0];
         EEPROM.write(EEPROM_sector1, sector[0]); 
         selectorSectores();    
         break;             

         default:
         delay(250);
         return;
       }
     }
   }
}

void selectorSectores(){
  switch(contadorSector){
    case 1:
    if(sector[0]==HIGH) {
      sector_Marca[0]='X';
      EEPROM.write(EEPROM_sector_Marca1, sector_Marca[0]); 
      tft.setTextColor(BLACK, WHITE);
      tft.setTextSize(2);
      tft.setCursor(64, 80);
      tft.print(sector_Marca[0]);
      flagModo=true;
      EEPROM.write(EEPROM_flagModo1, flagModo);
      break;
    }
    else {
      tft.setTextColor(WHITE);
      tft.setTextSize(2);     
      tft.setCursor(64, 80);
      tft.print(sector_Marca[0]);     
      sector_Marca[0]=' ';
      EEPROM.write(EEPROM_sector_Marca1, sector_Marca[0]);
      tft.setTextColor(BLACK, WHITE);
      flagModo=false;
      EEPROM.write(EEPROM_flagModo1, flagModo);            
      break;        
    }
   
    default:
    delay(200);
  }
}   

void mostrarSectoresActivar(){
  tft.fillScreen(WHITE);
  tft.fillRect(0,0,480,48,BLACK);
  tft.setTextColor(WHITE);     
  textSinClear(116, 12, "ACTIVAR SECTORES", 3, NULL);
  tft.setTextColor(BLACK,WHITE); 
  textSinClear(45, 80, "(  ) SECTOR #1", 2, NULL); 
  tft.setCursor(64, 80);
  tft.print(sector_Marca[0]);

  textSinClear(312, 272, "GUARDAR", 3, NULL);    
}

void mostrarSectores(){
  switch(contadorSector){
  case 1: textSinClear(105, 80, "SECTOR #1", 2, NULL); break;
  default: textSinClear(312, 272, "GUARDAR", 3, NULL);              
  }
}  

void selectorProgSecuencia(){
  switch(countJoystickV){
  case 1: limpiarSelector(); textSinClear(18, 80, ">", 2, NULL); IDSubSetting=1; break;
  case 2: limpiarSelector(); textSinClear(18, 240, ">", 2, NULL); IDSubSetting=2;   
  }
}

void menuProgramarSecuencia(){
  tft.fillScreen(WHITE);
  tft.fillRect(0,0,480,48,BLACK);
  tft.setTextColor(WHITE);     
  textSinClear(72, 12, "PROGRAMAR SECUENCIA", 3, NULL);    
  tft.setTextColor(BLACK,WHITE);        
  textSinClear(45, 80, "SECTOR #1", 2, NULL);
  textSinClear(45, 240, "SALIR", 2, NULL);    

  countJoystickV=1;
  selectorProgSecuencia();  
}

void programaSecuencia(){
  menuProgramarSecuencia();

  boolean modoProgSecuencia=true;
  while(modoProgSecuencia){
    //JoystickX = analogRead(A6);
    JoystickY = analogRead(A7);
       //MOVIMIENTOS VERTICALES
    if (JoystickY >Y_UpperLimit && JoystickY <=1023) {        //¿SE MUEVE HACIA ABAJO?
      delay(200);
      if(countJoystickV >= 3) countJoystickV=0;      
      countJoystickV++;      
      selectorProgSecuencia();
    }
    if (JoystickY >=0 && JoystickY <Y_LowerLimit) {     //¿SE MUEVE HACIA LA ARRIBA?
      delay(200);
      if (countJoystickV <=1) countJoystickV=3;     
      countJoystickV--; 
      selectorProgSecuencia();
    }
    if (!digitalRead(pulsador)){
      while(!digitalRead(pulsador));
      switch(IDSubSetting){
        case 1: programaSecuencia2(&hora_Inicio, &minuto_Inicio, &duracion_Horas, &duracion_Minutos, dias_Riego, marca_Dias_Riego, dia_Semana, 1); menuProgramarSecuencia(); break;
        default: modoProgSecuencia=false; 
      }
    }
  }
}

void menuProgramarSecuencia2(){
  tft.fillScreen(WHITE);
  tft.fillRect(0,0,480,48,BLACK);
  tft.setTextColor(WHITE);     
  textSinClear(72, 12, "PROGRAMAR SECUENCIA", 3, NULL);    
  tft.setTextColor(BLACK,WHITE);        
  textSinClear(45, 80, "HORA DE INICIO", 2, NULL);
  textSinClear(45, 120, "DURACION", 2, NULL);
  textSinClear(45, 160, "DIAS DE RIEGO", 2, NULL);
  textSinClear(45, 200, "BORRAR SECUENCIA", 2, NULL);  
  textSinClear(45, 240, "ATRAS", 2, NULL);
  countJoystickV=1;
  selectorProgSecuencia2();     
}

void programaSecuencia2(byte *horaInicio, byte *minutoInicio, byte *duracionHoras, byte *duracionMinutos, byte diasRiego[], char marcaDiasRiego[], char diaSemana[], byte numPrograma){
  menuProgramarSecuencia2();  
  
  boolean modoProgSecuencia2=true;
  while(modoProgSecuencia2){
    //JoystickX = analogRead(A6);
    JoystickY = analogRead(A7);
       //MOVIMIENTOS VERTICALES
    if (JoystickY >Y_UpperLimit && JoystickY <=1023) {        //¿SE MUEVE HACIA ABAJO?
      delay(200);
      if(countJoystickV >= 5) countJoystickV=0;      
      countJoystickV++;      
      selectorProgSecuencia2();
    }
    if (JoystickY >=0 && JoystickY <Y_LowerLimit) {     //¿SE MUEVE HACIA LA ARRIBA?
      delay(200);
      if (countJoystickV <=1) countJoystickV=6;     
      countJoystickV--; 
      selectorProgSecuencia2();
    }
    if (!digitalRead(pulsador)){
      while(!digitalRead(pulsador));
      switch(IDSubSetting){
        case 1: agregarhoraInicio(*horaInicio, *minutoInicio, numPrograma); menuProgramarSecuencia2(); break;
        case 2: agregarDuracion(*duracionHoras, *duracionMinutos, numPrograma); menuProgramarSecuencia2(); break;
        case 3: agregarDiasRiego(diasRiego, marcaDiasRiego, diaSemana, numPrograma); menuProgramarSecuencia2(); break;
        case 4: borrarSecuencia(numPrograma); menuProgramarSecuencia2(); break;    
        default: modoProgSecuencia2=false;
      }
    }
  }
}

void selectorProgSecuencia2(){
  switch(countJoystickV){
  case 1: limpiarSelector(); textSinClear(18, 80, ">", 2, NULL); IDSubSetting=1; break;
  case 2: limpiarSelector(); textSinClear(18, 120, ">", 2, NULL); IDSubSetting=2; break;
  case 3: limpiarSelector(); textSinClear(18, 160, ">", 2, NULL); IDSubSetting=3; break;
  case 4: limpiarSelector(); textSinClear(18, 200, ">", 2, NULL); IDSubSetting=4; break;
  default: limpiarSelector(); textSinClear(18, 240, ">", 2, NULL); IDSubSetting=5;   
  }
}

void WR_EEPROM (int pos_memory_eeprom, byte dato_W, boolean WR, byte *dato_R){
  EEPROM.write(pos_memory_eeprom, dato_W);
  delay(4);
  if(WR){
    *dato_R=EEPROM.read(pos_memory_eeprom);
    delay(1);
  }  
}

void agregarhoraInicio(byte horaInicio, byte minutoInicio, byte numPrograma){
    tft.fillScreen(WHITE);
    tft.fillRect(0,0,480,48,BLACK);
    tft.setTextColor(WHITE);     
    textSinClear(114, 12, "HORA DE INICIO", 3, NULL);    
    tft.setTextColor(BLACK,WHITE);   
  while(true){
    horarioSecuencia(horaInicio, minutoInicio);
    i=4; 
    horaInicio = edit(75, 225, 2, horaInicio, &FreeSevenSegNumFont);
    if(horaInicio==24) {
      horaInicio=0;
      WR_EEPROM(EEPROM_hora_Inicio, horaInicio, true, &hora_Inicio); 
    }
    else {
      WR_EEPROM(EEPROM_hora_Inicio, horaInicio, true, &hora_Inicio);   
    }   
    i=5;
    minutoInicio = edit(275, 225, 2, minutoInicio, &FreeSevenSegNumFont);                
    if(minutoInicio==60) {
      minutoInicio=0;
      WR_EEPROM(EEPROM_minuto_Inicio, minutoInicio, true, &minuto_Inicio);      
    }
    else {
      WR_EEPROM(EEPROM_minuto_Inicio, minutoInicio, true, &minuto_Inicio);
    }  
    
    while(true){
      JoystickX = analogRead(A6);
      //JoystickY = analogRead(A7);
      tft.setTextColor(WHITE, WHITE); 
      textSinClear(312, 272, "GUARDAR", 3, NULL);   
      oscilarText();
      tft.setTextColor(BLACK, WHITE); 
      textSinClear(312, 272, "GUARDAR", 3, NULL); 
      oscilarText();
      if(JoystickX >= 0 && JoystickX < X_LowerLimit){        // SE MUEVE HACIA LA IZQUIERDA
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

void agregarDuracion(byte duracionHoras, byte duracionMinutos, byte numPrograma){
    tft.fillScreen(WHITE);
    tft.fillRect(0,0,480,48,BLACK);
    tft.setTextColor(WHITE);     
    textSinClear(168, 12, "DURACION", 3, NULL);    
    tft.setTextColor(BLACK,WHITE);  
  while(true){
    duracionSecuencia(duracionHoras, duracionMinutos);
    i=4;
    duracionHoras=edit(75, 225, 2, duracionHoras, &FreeSevenSegNumFont);
    if(duracionHoras==24) {
      duracionHoras=0;
      WR_EEPROM(EEPROM_duracion_Horas, duracionHoras, true, &duracion_Horas);      
    }
    else {
      WR_EEPROM(EEPROM_duracion_Horas, duracionHoras, true, &duracion_Horas);
    }                  
    i=5;
    duracionMinutos=edit(275, 225, 2, duracionMinutos, &FreeSevenSegNumFont);
    if(duracionMinutos==60) {
      duracionMinutos=0; 
      WR_EEPROM(EEPROM_duracion_Minutos, duracionMinutos, true, &duracion_Minutos);     
    }  
    else {
      WR_EEPROM(EEPROM_duracion_Minutos, duracionMinutos, true, &duracion_Minutos);
    }                       
    while(true){
      JoystickX = analogRead(A6);
      //JoystickY = analogRead(A7);
      tft.setTextColor(WHITE, WHITE); 
      textSinClear(312, 272, "GUARDAR", 3, NULL);
      oscilarText();
      tft.setTextColor(BLACK, WHITE); 
      textSinClear(312, 272, "GUARDAR", 3, NULL);
      oscilarText();
      if(JoystickX >= 0 && JoystickX < X_LowerLimit){
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

void agregarDiasRiego(byte diasRiego[], char marcaDiasRiego[], char diaSemana[], byte numPrograma){  
  countJoystickV=1;
  mostrarDiasRiego(marcaDiasRiego);
  diaSemanaRiego=1;
  while(true){
     //JoystickX = analogRead(A6);
     JoystickY = analogRead(A7);
     tft.setTextColor(WHITE, WHITE); 
     mostrarDiasRiego2();
     oscilarText();
     if (JoystickY >=0 && JoystickY <Y_LowerLimit) {         //¿SE MUEVE HACIA ARRIBA?
       if(diaSemanaRiego < 1) diaSemanaRiego = 9;   
       tft.setTextColor(BLACK, WHITE); 
       mostrarDiasRiego2();
       diaSemanaRiego--;
       if(diaSemanaRiego < 1) diaSemanaRiego = 8 ;
       delay(200);   
     }
     if (JoystickY >Y_UpperLimit && JoystickY <= 1023) {      //¿SE MUEVE HACIA ABAJO?
       tft.setTextColor(BLACK, WHITE); 
       mostrarDiasRiego2();
       diaSemanaRiego++;
       if(diaSemanaRiego > 8) diaSemanaRiego = 1;
       delay(200);      
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
         diasRiego[0]=!diasRiego[0];
         WR_EEPROM(EEPROM_dias_Riego_0, diasRiego[0], false, NULL); 
         selectorDiasRiego(diasRiego, marcaDiasRiego, diaSemana);   
         break; 

         case 2:
         diasRiego[1]=!diasRiego[1];
         WR_EEPROM(EEPROM_dias_Riego_1, diasRiego[1], false, NULL); 
         selectorDiasRiego(diasRiego, marcaDiasRiego, diaSemana);    
         break;    
          
         case 3:
         diasRiego[2]=!diasRiego[2];
         WR_EEPROM(EEPROM_dias_Riego_2, diasRiego[2], false, NULL); 
         selectorDiasRiego(diasRiego, marcaDiasRiego, diaSemana);   
         break;    

         case 4:
         diasRiego[3]=!diasRiego[3];
         WR_EEPROM(EEPROM_dias_Riego_3, diasRiego[3], false, NULL); 
         selectorDiasRiego(diasRiego, marcaDiasRiego, diaSemana);   
         break;          

         case 5:
         diasRiego[4]=!diasRiego[4];
         WR_EEPROM(EEPROM_dias_Riego_4, diasRiego[4], false, NULL); 
         selectorDiasRiego(diasRiego, marcaDiasRiego, diaSemana);
         break;         

         case 6:
         diasRiego[5]=!diasRiego[5];
         WR_EEPROM(EEPROM_dias_Riego_5, diasRiego[5], false, NULL); 
         selectorDiasRiego(diasRiego, marcaDiasRiego, diaSemana);
         break;         
          
         case 7:
         diasRiego[6]=!diasRiego[6];
         WR_EEPROM(EEPROM_dias_Riego_6, diasRiego[6], false, NULL); 
         selectorDiasRiego(diasRiego, marcaDiasRiego, diaSemana);
         break;         

         default:
         delay(250);
         return;
       }
     }
   }
}

void selectorDiasRiego(byte diasRiego[], char marcaDiasRiego[], char diaSemana[]){
  switch(diaSemanaRiego){
    case 1:
    if(diasRiego[0]==HIGH) {
      marcaDiasRiego[0]='X'; diaSemana[0]='L';
      WR_EEPROM(EEPROM_marca_Dias_Riego_0, marcaDiasRiego[0], false, NULL);
      WR_EEPROM(EEPROM_lunes, diaSemana[0], false, NULL); 
      tft.setTextColor(BLACK, WHITE); 
      tft.setTextSize(2); 
      tft.setCursor(64, 80); 
      tft.print(marcaDiasRiego[0]);
    }
    else {
      tft.setTextColor(WHITE);
      tft.setTextSize(2);     
      tft.setCursor(64, 80);
      tft.print(marcaDiasRiego[0]);     
      marcaDiasRiego[0]=' '; diaSemana[0]='_';
      WR_EEPROM(EEPROM_marca_Dias_Riego_0, marcaDiasRiego[0], false, NULL);
      WR_EEPROM(EEPROM_lunes, diaSemana[0], false, NULL); 
      tft.setTextColor(BLACK, WHITE);       
    }
    break;
    
    case 2:
    if(diasRiego[1]==HIGH) {
      marcaDiasRiego[1]='X'; diaSemana[1]='M';
      WR_EEPROM(EEPROM_marca_Dias_Riego_1, marcaDiasRiego[1], false, NULL);
      WR_EEPROM(EEPROM_martes, diaSemana[1], false, NULL);       
      tft.setTextColor(BLACK, WHITE); 
      tft.setTextSize(2); 
      tft.setCursor(64, 120); 
      tft.print(marcaDiasRiego[1]);             
    }
    else {
      tft.setTextColor(WHITE);
      tft.setTextSize(2);     
      tft.setCursor(64, 120);
      tft.print(marcaDiasRiego[1]);     
      marcaDiasRiego[1]=' '; diaSemana[1]='_';
      WR_EEPROM(EEPROM_marca_Dias_Riego_1, marcaDiasRiego[1], false, NULL);
      WR_EEPROM(EEPROM_martes, diaSemana[1], false, NULL);        
      tft.setTextColor(BLACK, WHITE);              
    }
    break;
        
    case 3:
    if(diasRiego[2]==HIGH) {
      marcaDiasRiego[2]='X'; diaSemana[2]='M';
      WR_EEPROM(EEPROM_marca_Dias_Riego_2, marcaDiasRiego[2], false, NULL);
      WR_EEPROM(EEPROM_miercoles, diaSemana[2], false, NULL);       
      tft.setTextColor(BLACK, WHITE); 
      tft.setTextSize(2); 
      tft.setCursor(64, 160); 
      tft.print(marcaDiasRiego[2]);                         
    }
    else {
      tft.setTextColor(WHITE);
      tft.setTextSize(2);     
      tft.setCursor(64, 160);
      tft.print(marcaDiasRiego[2]);     
      marcaDiasRiego[2]=' '; diaSemana[2]='_';
      WR_EEPROM(EEPROM_marca_Dias_Riego_2, marcaDiasRiego[2], false, NULL);
      WR_EEPROM(EEPROM_miercoles, diaSemana[2], false, NULL);         
      tft.setTextColor(BLACK, WHITE);         
    }
    break;

    case 4:
    if(diasRiego[3]==HIGH) {
      marcaDiasRiego[3]='X'; diaSemana[3]='J';
      WR_EEPROM(EEPROM_marca_Dias_Riego_3, marcaDiasRiego[3], false, NULL);
      WR_EEPROM(EEPROM_jueves, diaSemana[3], false, NULL);         
      tft.setTextColor(BLACK, WHITE); 
      tft.setTextSize(2); 
      tft.setCursor(64, 200); 
      tft.print(marcaDiasRiego[3]);       
    }
    else {
      tft.setTextColor(WHITE);
      tft.setTextSize(2);     
      tft.setCursor(64, 200);
      tft.print(marcaDiasRiego[3]);     
      marcaDiasRiego[3]=' '; diaSemana[3]='_';
      WR_EEPROM(EEPROM_marca_Dias_Riego_3, marcaDiasRiego[3], false, NULL);
      WR_EEPROM(EEPROM_jueves, diaSemana[3], false, NULL);          
      tft.setTextColor(BLACK, WHITE);            
    }
    break;

    case 5:
    if(diasRiego[4]==HIGH) {
      marcaDiasRiego[4]='X'; diaSemana[4]='V';
      WR_EEPROM(EEPROM_marca_Dias_Riego_4, marcaDiasRiego[4], false, NULL);
      WR_EEPROM(EEPROM_viernes, diaSemana[4], false, NULL);         
      tft.setTextColor(BLACK, WHITE); 
      tft.setTextSize(2); 
      tft.setCursor(64, 240); 
      tft.print(marcaDiasRiego[4]);     
    }
    else {
      tft.setTextColor(WHITE);
      tft.setTextSize(2);     
      tft.setCursor(64, 240);
      tft.print(marcaDiasRiego[4]);     
      marcaDiasRiego[4]=' '; diaSemana[4]='_'; 
      WR_EEPROM(EEPROM_marca_Dias_Riego_4, marcaDiasRiego[4], false, NULL);
      WR_EEPROM(EEPROM_viernes, diaSemana[4], false, NULL);        
      tft.setTextColor(BLACK, WHITE);             
    }
    break;

    case 6:
    if(diasRiego[5]==HIGH) {
      marcaDiasRiego[5]='X'; diaSemana[5]='S';
      WR_EEPROM(EEPROM_marca_Dias_Riego_5, marcaDiasRiego[5], false, NULL);
      WR_EEPROM(EEPROM_sabado, diaSemana[5], false, NULL);        
      tft.setTextColor(BLACK, WHITE);
      tft.setTextSize(2); 
      tft.setCursor(306, 80); 
      tft.print(marcaDiasRiego[5]);      
    }
    else {
      tft.setTextColor(WHITE);
      tft.setTextSize(2);     
      tft.setCursor(306, 80);
      tft.print(marcaDiasRiego[5]);     
      marcaDiasRiego[5]=' '; diaSemana[5]='_';
      WR_EEPROM(EEPROM_marca_Dias_Riego_5, marcaDiasRiego[5], false, NULL);
      WR_EEPROM(EEPROM_sabado, diaSemana[5], false, NULL);        
      tft.setTextColor(BLACK, WHITE);            
    }
    break;

    case 7:
    if(diasRiego[6]==HIGH) {
      marcaDiasRiego[6]='X'; diaSemana[6]='D';
      WR_EEPROM(EEPROM_marca_Dias_Riego_6, marcaDiasRiego[6], false, NULL);
      WR_EEPROM(EEPROM_domingo, diaSemana[6], false, NULL);        
      tft.setTextColor(BLACK, WHITE); 
      tft.setTextSize(2); 
      tft.setCursor(306, 120); 
      tft.print(marcaDiasRiego[6]);        
    }
    else {
      tft.setTextColor(WHITE);
      tft.setTextSize(2);     
      tft.setCursor(306, 120);
      tft.print(marcaDiasRiego[6]);   
      marcaDiasRiego[6]=' '; diaSemana[6]='_';
      WR_EEPROM(EEPROM_marca_Dias_Riego_6, marcaDiasRiego[6], false, NULL);
      WR_EEPROM(EEPROM_domingo, diaSemana[6], false, NULL);        
      tft.setTextColor(BLACK, WHITE);            
    }
    break;
    
    default:
    delay(200);
  }
}   

void mostrarDiasRiego(char marcaDiasRiego[]){
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

void borrarSecuencia(byte numPrograma){
  switch(numPrograma){
  case 1:
    hora_Inicio=0; EEPROM.write(EEPROM_hora_Inicio, hora_Inicio);
    minuto_Inicio=0; EEPROM.write(EEPROM_minuto_Inicio, minuto_Inicio);
    duracion_Horas=0; EEPROM.write(EEPROM_duracion_Horas, duracion_Horas);
    duracion_Minutos=0; EEPROM.write(EEPROM_duracion_Minutos, duracion_Minutos);

    dias_Riego[0]=LOW; EEPROM.write(EEPROM_dias_Riego_0, dias_Riego[0]);
    dias_Riego[1]=LOW; EEPROM.write(EEPROM_dias_Riego_1, dias_Riego[1]);
    dias_Riego[2]=LOW; EEPROM.write(EEPROM_dias_Riego_2, dias_Riego[2]);
    dias_Riego[3]=LOW; EEPROM.write(EEPROM_dias_Riego_3, dias_Riego[3]);
    dias_Riego[4]=LOW; EEPROM.write(EEPROM_dias_Riego_4, dias_Riego[4]);
    dias_Riego[5]=LOW; EEPROM.write(EEPROM_dias_Riego_5, dias_Riego[5]);
    dias_Riego[6]=LOW; EEPROM.write(EEPROM_dias_Riego_6, dias_Riego[6]);     

    marca_Dias_Riego[0]=' '; EEPROM.write(EEPROM_marca_Dias_Riego_0, marca_Dias_Riego[0]);
    marca_Dias_Riego[1]=' '; EEPROM.write(EEPROM_marca_Dias_Riego_1, marca_Dias_Riego[1]);
    marca_Dias_Riego[2]=' '; EEPROM.write(EEPROM_marca_Dias_Riego_2, marca_Dias_Riego[2]);
    marca_Dias_Riego[3]=' '; EEPROM.write(EEPROM_marca_Dias_Riego_3, marca_Dias_Riego[3]);
    marca_Dias_Riego[4]=' '; EEPROM.write(EEPROM_marca_Dias_Riego_4, marca_Dias_Riego[4]);
    marca_Dias_Riego[5]=' '; EEPROM.write(EEPROM_marca_Dias_Riego_5, marca_Dias_Riego[5]);
    marca_Dias_Riego[6]=' '; EEPROM.write(EEPROM_marca_Dias_Riego_6, marca_Dias_Riego[6]);

    dia_Semana[0]='_'; EEPROM.write(EEPROM_lunes, dia_Semana[0]);
    dia_Semana[1]='_'; EEPROM.write(EEPROM_martes, dia_Semana[1]);
    dia_Semana[2]='_'; EEPROM.write(EEPROM_miercoles, dia_Semana[2]);
    dia_Semana[3]='_'; EEPROM.write(EEPROM_jueves, dia_Semana[3]);
    dia_Semana[4]='_'; EEPROM.write(EEPROM_viernes, dia_Semana[4]); 
    dia_Semana[5]='_'; EEPROM.write(EEPROM_sabado, dia_Semana[5]); 
    dia_Semana[6]='_'; EEPROM.write(EEPROM_domingo, dia_Semana[6]);

    sector[0]=LOW, EEPROM.write(EEPROM_sector1, sector[0]);
    sector_Marca[0]=' '; EEPROM.write(EEPROM_sector_Marca1, sector_Marca[0]);
    flagModo=false; EEPROM.write(EEPROM_flagModo1, flagModo);
    digitalWrite(valvula_1, HIGH);
    flagAlarma=false; 
    break;       
  }
       
  flagDashboard=true;
  flagSettings=false; 

  tft.fillScreen(WHITE);
  tft.setTextColor(BLACK, WHITE);   
  textSinClear(132, 150, "SECUENCIA BORRADA", 2, NULL);
  delay(1500);  
}

void readEEPROM(){
  flagModo=EEPROM.read(EEPROM_flagModo1);

  sector[0]=EEPROM.read(EEPROM_sector1);

  sector_Marca[0]=EEPROM.read(EEPROM_sector_Marca1);
  
  hora_Inicio=EEPROM.read(EEPROM_hora_Inicio);
  
  minuto_Inicio=EEPROM.read(EEPROM_minuto_Inicio);
  
  duracion_Horas=EEPROM.read(EEPROM_duracion_Horas); 
  
  duracion_Minutos=EEPROM.read(EEPROM_duracion_Minutos); 
   
  dias_Riego[0]=EEPROM.read(EEPROM_dias_Riego_0); 
  dias_Riego[1]=EEPROM.read(EEPROM_dias_Riego_1); 
  dias_Riego[2]=EEPROM.read(EEPROM_dias_Riego_2); 
  dias_Riego[3]=EEPROM.read(EEPROM_dias_Riego_3); 
  dias_Riego[4]=EEPROM.read(EEPROM_dias_Riego_4);                                 
  dias_Riego[5]=EEPROM.read(EEPROM_dias_Riego_5); 
  dias_Riego[6]=EEPROM.read(EEPROM_dias_Riego_6);    

  dia_Semana[0]=EEPROM.read(EEPROM_lunes);

  dia_Semana[1]=EEPROM.read(EEPROM_martes);   

  dia_Semana[2]=EEPROM.read(EEPROM_miercoles); 
        
  dia_Semana[3]=EEPROM.read(EEPROM_jueves);  
        
  dia_Semana[4]=EEPROM.read(EEPROM_viernes);
        
  dia_Semana[5]=EEPROM.read(EEPROM_sabado); 
        
  dia_Semana[6]=EEPROM.read(EEPROM_domingo);    
  
  marca_Dias_Riego[0]=EEPROM.read(EEPROM_marca_Dias_Riego_0); 
  marca_Dias_Riego[1]=EEPROM.read(EEPROM_marca_Dias_Riego_1); 
  marca_Dias_Riego[2]=EEPROM.read(EEPROM_marca_Dias_Riego_2); 
  marca_Dias_Riego[3]=EEPROM.read(EEPROM_marca_Dias_Riego_3); 
  marca_Dias_Riego[4]=EEPROM.read(EEPROM_marca_Dias_Riego_4);
  marca_Dias_Riego[5]=EEPROM.read(EEPROM_marca_Dias_Riego_5); 
  marca_Dias_Riego[6]=EEPROM.read(EEPROM_marca_Dias_Riego_6);  
  
}

void writeEEPROM(){
  EEPROM.write(EEPROM_flagModo1, false);

  EEPROM.write(EEPROM_sector1, ' ');

  EEPROM.write(EEPROM_sector_Marca1, ' ');
  
  EEPROM.write(EEPROM_hora_Inicio, 0);
  
  EEPROM.write(EEPROM_minuto_Inicio, 0);
  
  EEPROM.write(EEPROM_duracion_Horas, 0); 
  
  EEPROM.write(EEPROM_duracion_Minutos, 0); 
   
  EEPROM.write(EEPROM_dias_Riego_0, LOW); 
  EEPROM.write(EEPROM_dias_Riego_1, LOW); 
  EEPROM.write(EEPROM_dias_Riego_2, LOW); 
  EEPROM.write(EEPROM_dias_Riego_3, LOW); 
  EEPROM.write(EEPROM_dias_Riego_4, LOW);                                 
  EEPROM.write(EEPROM_dias_Riego_5, LOW); 
  EEPROM.write(EEPROM_dias_Riego_6, LOW);                                                                                                                           
  
  EEPROM.write(EEPROM_lunes, '_');

  EEPROM.write(EEPROM_martes, '_');   
        
  EEPROM.write(EEPROM_miercoles, '_'); 
        
  EEPROM.write(EEPROM_jueves, '_'); 
        
  EEPROM.write(EEPROM_viernes, '_');
        
  EEPROM.write(EEPROM_sabado, '_');  
        
  EEPROM.write(EEPROM_domingo, '_');    
  
  EEPROM.write(EEPROM_marca_Dias_Riego_0, ' '); 
  EEPROM.write(EEPROM_marca_Dias_Riego_1, ' '); 
  EEPROM.write(EEPROM_marca_Dias_Riego_2, ' '); 
  EEPROM.write(EEPROM_marca_Dias_Riego_3, ' '); 
  EEPROM.write(EEPROM_marca_Dias_Riego_4, ' ');
  EEPROM.write(EEPROM_marca_Dias_Riego_5, ' '); 
  EEPROM.write(EEPROM_marca_Dias_Riego_6, ' ');    
}
