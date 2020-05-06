//***************************************************************************************************************************************
/* Librería para el uso de la pantalla ILI9341 en modo 8 bits
 * Basado en el código de martinayotte - https://www.stm32duino.com/viewtopic.php?t=637
 * Adaptación, migración y creación de nuevas funciones: Pablo Mazariegos y José Morales
 * Con ayuda de: José Guerra
 * IE3027: Electrónica Digital 2 - 2019
 * Proyecto 2 IE3027
 * Josue Asturias y Miguel Garcia
 * Videojuego Dragon Ball Z en pantalla 320x240
 */
//***************************************************************************************************************************************
#include <stdint.h>
#include <stdbool.h>
#include <TM4C123GH6PM.h>
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/debug.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/rom_map.h"
#include "driverlib/rom.h"
#include "driverlib/sysctl.h"
#include "driverlib/timer.h"
#include <SPI.h>
#include <SD.h>
//#include <Wire.h>


//#include "bitmaps.h"
#include "font.h"
#include "lcd_registers.h"

#define SDA  PA_7
#define SCL  PA_6

/* Botones para el Jugador 1*/
#define PUSH_UP PE_2
#define PUSH_LEFT PE_3
#define PUSH_RIGHT PF_4
#define PUSH_DOWN PF_0
#define PUSH_A PC_4
#define PUSH_B PC_5

/* Botones para el Jugador 2 */
#define PUSH2_UP PC_7
#define PUSH2_LEFT PF_3
#define PUSH2_RIGHT PF_1
#define PUSH2_DOWN PC_6
#define PUSH2_A PE_4
#define PUSH2_B PE_5  

// Pines que van a la LCD
#define LCD_RST PD_6 //PD_0 PB6
#define LCD_CS PD_7  //PD_1 PB7
#define LCD_RS PD_2
#define LCD_WR PD_3
#define LCD_RD PE_1
int DPINS[] = {PB_0, PB_1, PB_2, PB_3, PB_4, PB_5, PB_6, PB_7};  

extern uint8_t cricosoNuevo[];
extern uint8_t suelo1[];
extern char * dir_ataqueA[];
extern char * dir_ataqueB1[];
extern char * dir_ataqueB2[];
extern uint8_t KO[];

struct jugador{
  uint8_t seleccion;
  uint8_t py;
  uint16_t px;
  uint8_t pos_spr;
  uint8_t orientacion;
  uint8_t vida;
  uint8_t ki;
  uint8_t listo;
};

struct poder{
  uint8_t tecnica;
  uint8_t py;
  uint16_t px;
  uint8_t spr;
  uint8_t damage;
  uint8_t gasto;
  unsigned long zeit;
  
};

unsigned char player1[12540] = {};
unsigned char player2[12540] = {};


jugador P1 = {1,159,100,0,0,100,100,0};
jugador P2 = {4,159,180,0,1,100,100,0};

poder AtaqueP1 = {0,100,100,0,0,0,0};
poder AtaqueP2 = {0,100,100,0,0,0,0};

//***************************************************************************************************************************************
// Functions Prototypes
//***************************************************************************************************************************************
void LCD_Init(void);
void LCD_CMD(uint8_t cmd);
void LCD_DATA(uint8_t data);
void SetWindows(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2);
void LCD_Clear(unsigned int c);
void H_line(unsigned int x, unsigned int y, unsigned int l, unsigned int c);
void V_line(unsigned int x, unsigned int y, unsigned int l, unsigned int c);
void Rect(unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int c);
void FillRect(unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int c);
void LCD_Print(String text, int x, int y, int fontSize, int color, int background);

void LCD_Bitmap(unsigned int x, unsigned int y, unsigned int width, unsigned int height, unsigned char bitmap[]);
void LCD_Sprite(int x, int y, int width, int height, unsigned char bitmap[],int columns, int index, char flip, char offset);

//void Anim_golpe(unsigned char personaje_golpe[], uint16_t pos_x, uint16_t pos_y,bool orientacion);
void LCD_SD_Bitmap(unsigned int x, unsigned int y, unsigned int width, unsigned int height, char * direccion);
void LCD_SD_Sprite(int x, int y, int width, int height,int columns, int index, char flip, char offset,char * direccion);
void SD_loadP1_Sprite(char * direccion);
void SD_loadP2_Sprite(char * direccion);
void Per_avatar(uint8_t player_select,uint16_t x, uint8_t y, uint8_t ancho, uint8_t alto);
void reset_Estados(void);
unsigned long ATK_limpiar(uint8_t orientacion, uint16_t x, uint16_t ATKx, uint8_t ATKy,uint8_t dimX, uint8_t dimY);

//***************************************************************************************************************************************
// Inicialización
//***************************************************************************************************************************************
uint8_t pantallas = 0;
void setup() {
  pinMode(PUSH_A,INPUT_PULLUP);
  pinMode(PUSH_B,INPUT_PULLUP);
  pinMode(PUSH_UP, INPUT_PULLUP);
  pinMode(PUSH_LEFT, INPUT_PULLUP);
  pinMode(PUSH_RIGHT, INPUT_PULLUP);
  pinMode(PUSH_DOWN, INPUT_PULLUP);

  pinMode(PUSH2_A,INPUT_PULLUP);
  pinMode(PUSH2_B,INPUT_PULLUP);
  pinMode(PUSH2_UP, INPUT_PULLUP);
  pinMode(PUSH2_LEFT, INPUT_PULLUP);
  pinMode(PUSH2_RIGHT, INPUT_PULLUP);
  pinMode(PUSH2_DOWN, INPUT_PULLUP);

  pinMode(SDA, OUTPUT);
  pinMode(SCL, OUTPUT);
  
  SysCtlClockSet(SYSCTL_SYSDIV_2_5|SYSCTL_USE_PLL|SYSCTL_OSC_MAIN|SYSCTL_XTAL_16MHZ);
  Serial.begin(9600);
  //Wire.begin();
  SPI.setModule(0);
  GPIOPadConfigSet(GPIO_PORTB_BASE, 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7, GPIO_STRENGTH_8MA, GPIO_PIN_TYPE_STD_WPU);
  LCD_Init();
  LCD_Clear(0x00);
  LCD_Clear(0x865e);

  if (!SD.begin(32)) {
    //Serial.println("initialization failed!");
    return;
  }
  //Serial.println("initialization done.");

  digitalWrite(SCL, HIGH);
  digitalWrite(SDA, LOW);
  
  LCD_Bitmap(0,0,320,240,cricosoNuevo);
  delay(3000);

  
}
//***************************************************************************************************************************************
// Loop Infinito       Loop Infinito    Loop Infinito      Loop Infinito      Loop Infinito     Loop Infinito     Loop Infinito   Loop Infinito                                                          
//***************************************************************************************************************************************
void loop() {
  if (pantallas == 0){
    digitalWrite(SCL, HIGH);
    digitalWrite(SDA, LOW);
    LCD_Clear(0xAA23);
    Per_avatar(P1.seleccion,70,100,40,56);
    String jug1 = " P1 ";
    LCD_Print(jug1,57,78,2,0xFFFF, 0xA8A3);
    LCD_SD_Sprite(50,120,18,22,2,0,0,0,"flechas.txt");
    LCD_SD_Sprite(112,120,18,22,2,0,1,0,"flechas.txt");
    Per_avatar(P2.seleccion,200,100,40,56);
    String jug2 = " P2 ";
    LCD_Print(jug2,187,78,2,0xFFFF, 0x1A0B);
    LCD_SD_Sprite(180,120,18,22,2,0,0,0,"flechas.txt");
    LCD_SD_Sprite(242,120,18,22,2,0,1,0,"flechas.txt");
  }
  
  while(pantallas == 0){

    //seleccion jugador 1:------------------------------------------------------------------------------------------------------------------
    if(digitalRead(PUSH_RIGHT) == 0 && P1.listo == 0){
      LCD_SD_Sprite(112,120,18,22,2,1,1,0,"flechas.txt");
      P1.seleccion ++;
      if (P1.seleccion >= 6){
        P1.seleccion = 1;
      }
      Per_avatar(P1.seleccion,70,100,40,56);
    }
    else{
      LCD_SD_Sprite(112,120,18,22,2,0,1,0,"flechas.txt");
     // LCD_SD_Sprite(int x, int y, int width, int height,int columns, int index, char flip, char offset,char * direccion){
    }

    if(digitalRead(PUSH_LEFT) == 0 && P1.listo == 0){
      LCD_SD_Sprite(50,120,18,22,2,1,0,0,"flechas.txt");
      P1.seleccion --;
      if (P1.seleccion < 1){
        P1.seleccion = 5;
      }
      Per_avatar(P1.seleccion,70,100,40,56);
    }
    else{
      LCD_SD_Sprite(50,120,18,22,2,0,0,0,"flechas.txt");
     // LCD_SD_Sprite(int x, int y, int width, int height,int columns, int index, char flip, char offset,char * direccion){
    }

    if(digitalRead(PUSH_A) == 0 && P1.listo == 0){
      P1.listo = 1;
      //void LCD_Print(String text, int x, int y, int fontSize, int color, int background) 
      String letrero = "Listo!";
      LCD_Print(letrero,43,170,2,0xFFFF,0xA8A3);
    }
    
    if(digitalRead(PUSH_B) == 0 && P1.listo == 1){
      P1.listo = 0;
      FillRect(43, 170, 95, 16, 0xAA23);
    }

    //seleccion jugador 2:------------------------------------------------------------------------------------------------------------
    if(digitalRead(PUSH2_RIGHT) == 0 && P2.listo == 0){
      LCD_SD_Sprite(242,120,18,22,2,1,1,0,"flechas.txt");
      P2.seleccion ++;
      if (P2.seleccion >= 6){
        P2.seleccion = 1;
      }
      Per_avatar(P2.seleccion,200,100,40,56);
    }
    else{
      LCD_SD_Sprite(242,120,18,22,2,0,1,0,"flechas.txt");
     // LCD_SD_Sprite(int x, int y, int width, int height,int columns, int index, char flip, char offset,char * direccion){
    }

    if(digitalRead(PUSH2_LEFT) == 0 && P2.listo == 0){
      LCD_SD_Sprite(180,120,18,22,2,1,0,0,"flechas.txt");
      P2.seleccion --;
      if (P2.seleccion < 1){
        P2.seleccion = 5;
      }
      Per_avatar(P2.seleccion,200,100,40,56);
    }
    else{
      LCD_SD_Sprite(180,120,18,22,2,0,0,0,"flechas.txt");
     // LCD_SD_Sprite(int x, int y, int width, int height,int columns, int index, char flip, char offset,char * direccion){
    }

    if(digitalRead(PUSH2_A) == 0 && P2.listo == 0){
      P2.listo = 1;
      String letrero = "Listo!";
      LCD_Print(letrero,173,170,2,0xFFFF,0xA8A3);
    }
    
    if(digitalRead(PUSH2_B) == 0 && P2.listo == 1){
      P2.listo = 0;
      FillRect(173, 170, 95, 16, 0xAA23);
    }
 
    if(P1.listo && P2.listo){  
      pantallas = 1;
      LCD_Clear(0x763e);
      for(uint8_t n = 0; n<16; n++){
        LCD_Bitmap(n*20,219,20,20,suelo1);
      }
      Per_avatar(P1.seleccion,2,2,40,56);
      Per_avatar(P2.seleccion,277,2,40,56);
      char * sprite_dir[] = {"xxSpr.txt", "GoSpr.txt","FreSpr.txt","VegSpr.txt", "CeSpr.txt", "BuSpr.txt"};
      SD_loadP1_Sprite(sprite_dir[P1.seleccion]);
      SD_loadP2_Sprite(sprite_dir[P2.seleccion]);

      reset_Estados();
      FillRect(45,10,100,20,0x4bc4);
      FillRect(172,10,100,20,0x4bc4); // x = 277-105

      FillRect(45,32,P1.ki,10,0x1A0B);
      FillRect(172+P2.ki,32,P2.ki,10,0x1A0B);

      digitalWrite(SCL, LOW);
      digitalWrite(SDA, HIGH);
      
//      LCD_Bitmap(125,100,69,44,KO);
//      String gana1 = "P1 WINS!";
//      LCD_Print(gana1, 95,75,2,0xFFFF,0xA8A3);
    }
  }

  /* *****************************************************************************************
   *                                         Pelea
   *******************************************************************************************/
  
  unsigned long currentMillis = millis();
  if( (currentMillis-AtaqueP1.zeit) >= 500){
    AtaqueP1.zeit = 0;
  }
  if( (currentMillis-AtaqueP2.zeit) >= 500){
    AtaqueP2.zeit = 0;
  }
  
  LCD_Sprite(P1.px,P1.py,33,38,player1,5,P1.pos_spr,P1.orientacion,0);
  H_line(P1.px,P1.py-1,34,0x763e);
  H_line(P1.px,P1.py + 39, 34, 0x763e);
  V_line(P1.px-1,P1.py,39, 0x763e);
  V_line(P1.px+34,P1.py,39, 0x763e);

  LCD_Sprite(P2.px,P2.py,33,38,player2,5,P2.pos_spr,P2.orientacion,0);
  H_line(P2.px,P2.py-1,34,0x763e);
  H_line(P2.px,P2.py + 39, 34, 0x763e);
  V_line(P2.px-1,P2.py,39, 0x763e);
  V_line(P2.px+34,P2.py,39, 0x763e);

// ATAQUE JUGADOR 1
  if(AtaqueP1.tecnica != 0){
    switch(AtaqueP1.tecnica){
      case 1:
        if( !(((AtaqueP1.px+13) >= P2.px) && (AtaqueP1.px <= (P2.px+33)) && ((AtaqueP1.py+13)>=P2.py) && (AtaqueP1.py <= (P2.py+38)) ) ){
          FillRect(AtaqueP1.px, AtaqueP1.py, 17,13,0x763e);
          switch (P1.orientacion){
            case 0:
              AtaqueP1.px +=17;
              break;
            case 1:
              AtaqueP1.px -=17;
              break;
          }
          if( !(AtaqueP1.px > 320  || AtaqueP1.px<=20) ){
            LCD_SD_Sprite(AtaqueP1.px,AtaqueP1.py,17,13,2,AtaqueP1.spr,P1.orientacion,0,dir_ataqueA[P1.seleccion]);
          }
          else{
            AtaqueP1.tecnica = 0;
            AtaqueP1.zeit = ATK_limpiar(P1.orientacion, P1.px, 2, AtaqueP1.py, 17,13);
          }
        }
        else{
          AtaqueP1.tecnica = 0;
          AtaqueP1.zeit = ATK_limpiar(P1.orientacion, P1.px, AtaqueP1.px, AtaqueP1.py, 17,13);
          P2.vida -= AtaqueP1.damage;
          FillRect(172,10,(100-P2.vida),20,0x763e);
          if (P2.vida == 0){
            pantallas = 0;
            GAME_OVER(1);
          }
        }
        
        break;
      case 2:
        if( !(((AtaqueP1.px+52)>=P2.px) && (AtaqueP1.px <= (P2.px+33)) && ((AtaqueP1.py+30) >= P2.py) && (AtaqueP1.py <= (P2.py+38))) ){
          switch(P1.orientacion){
            case 0:
              AtaqueP1.px +=5;
              break;
            case 1:
              AtaqueP1.px -=5;
              break;
          }
          if( !(AtaqueP1.px > 267 ||  AtaqueP1.px <=15) ){ //agregar topa con ataque contrario
            LCD_SD_Sprite(AtaqueP1.px, AtaqueP1.py,52,30,1,0,P1.orientacion,0, dir_ataqueB2[P1.seleccion]);
          }
          else{
            AtaqueP1.zeit = millis();
            switch(P1.orientacion){
              case 0:
                FillRect(P1.px+33,P1.py,(AtaqueP1.px + 52)-(P1.px+33),38,0x763e);
                break;
              case 1:
                FillRect(AtaqueP1.px,P1.py,P1.px-AtaqueP1.px,38,0x763e);
                break;
            }
          }
        }
        else{
          AtaqueP1.tecnica = 0;
          if (P2.vida > AtaqueP1.damage){
            P2.vida -= AtaqueP1.damage;
            FillRect(172,10,(100-P2.vida),20,0x763e);
          }
          else{
            P2.vida = 0;
            FillRect(172,10,(100-P2.vida),20,0x763e);
          }
          AtaqueP1.zeit = millis();
          switch(P1.orientacion){
            case 0:
              FillRect(P1.px+33,P1.py,(AtaqueP1.px + 52)-(P1.px+33),38,0x763e);
              break;
            case 1:
              FillRect(AtaqueP1.px,P1.py,P1.px-AtaqueP1.px,38,0x763e);
              break;
          }
          if(!P2.vida){
            pantallas = 0;
            GAME_OVER(1);
          }
        }
    }
  }

 // ATAQUE JUGADOR 2

 if(AtaqueP2.tecnica != 0){
    switch(AtaqueP2.tecnica){
      case 1:
        if( !(((AtaqueP2.px+13) >= P1.px) && (AtaqueP2.px <= (P1.px+33)) && ((AtaqueP2.py+13)>=P1.py) && (AtaqueP2.py <= (P1.py+38)) ) ){
          FillRect(AtaqueP2.px, AtaqueP2.py, 17,13,0x763e);
          switch (P2.orientacion){
            case 0:
              AtaqueP2.px +=17;
              break;
            case 1:
              AtaqueP2.px -=17;
              break;
          }
          if( !(AtaqueP2.px > 320  || AtaqueP2.px<=20) ){
            LCD_SD_Sprite(AtaqueP2.px,AtaqueP2.py,17,13,2,AtaqueP2.spr,P2.orientacion,0,dir_ataqueA[P2.seleccion]);
          }
          else{
            AtaqueP2.tecnica = 0;
            AtaqueP2.zeit = ATK_limpiar(P2.orientacion, P2.px, 2, AtaqueP2.py, 17,13);
          }
        }
        else{
          AtaqueP2.tecnica = 0;
          AtaqueP2.zeit = ATK_limpiar(P2.orientacion, P2.px, AtaqueP2.px, AtaqueP2.py, 17,13);
          P1.vida -= AtaqueP2.damage;
          //FillRect(172,10,(100-P1.vida),20,0x763e); //****
          FillRect(45 + P1.vida,10,100-P1.vida,20,0x763e);
          if (P1.vida == 0){
            pantallas = 0;
            GAME_OVER(2);
          }
        }
        break;
      case 2:
        if( !(((AtaqueP2.px+52)>=P1.px) && (AtaqueP2.px <= (P1.px+33)) && ((AtaqueP2.py+30) >= P1.py) && (AtaqueP2.py <= (P1.py+38))) ){
          switch(P2.orientacion){
            case 0:
              AtaqueP2.px +=5;
              break;
            case 1:
              AtaqueP2.px -=5;
              break;
          }
          if( !(AtaqueP2.px > 267 ||  AtaqueP2.px <=15) ){ //agregar topa con ataque contrario
            LCD_SD_Sprite(AtaqueP2.px, AtaqueP2.py,52,30,1,0,P2.orientacion,0, dir_ataqueB2[P2.seleccion]);
          }
          else{
            AtaqueP2.zeit = millis();
            switch(P2.orientacion){
              case 0:
                FillRect(P2.px+33,P1.py,(AtaqueP2.px + 52)-(P2.px+33),38,0x763e);
                break;
              case 1:
                FillRect(AtaqueP2.px,P2.py,P2.px-AtaqueP2.px,38,0x763e);
                break;
            }
          }
        }
        else{
          AtaqueP2.tecnica = 0;
          if (P1.vida > AtaqueP2.damage){
            P1.vida -= AtaqueP2.damage;
            //FillRect(172,10,(100-P2.vida),20,0x763e);  //*****
            FillRect(45 + P1.vida,10,100-P1.vida,20,0x763e);
          }
          else{
            P1.vida = 0;
            FillRect(45 + P1.vida,10,100-P1.vida,20,0x763e);
          }
          AtaqueP2.zeit = millis();
          switch(P2.orientacion){
            case 0:
              FillRect(P2.px+33,P2.py,(AtaqueP2.px + 52)-(P2.px+33),38,0x763e);
              break;
            case 1:
              FillRect(AtaqueP2.px,P2.py,P2.px-AtaqueP2.px,38,0x763e);
              break;
          }
          if(!P1.vida){
            pantallas = 0;
            GAME_OVER(2);
          }
        }
    }
  }

  
 // Controles Jugador 1-------------------------------------------------------------------------- 
  if (digitalRead(PUSH_RIGHT) == 0 && !AtaqueP1.tecnica){
    P1.orientacion = 0;
    P1.pos_spr = 1;
    if( !(((P1.px+33)>=P2.px) && (P1.px<(P2.px+33)) && (P1.py<(P2.py+38)) && ((P1.py+38)>P2.py))){
      P1.px +=1;
      if (P1.px > 285){
        P1.px = 285;
      }
    }
  }

  if (digitalRead(PUSH_LEFT) == 0 && !AtaqueP1.tecnica){
    P1.orientacion = 1;
    P1.pos_spr = 1;
    if( !((P1.px<=(P2.px+33)) && (P1.px>=P2.px) && (P1.py<(P2.py+38)) && ((P1.py+38)>P2.py) ) ){
      P1.px --;
      if(P1.px < 2){
        P1.px = 2;
      }
    }
  }
  
   if (digitalRead(PUSH_DOWN) == 0 && digitalRead(PUSH_B) && !AtaqueP1.tecnica ){
    if (P1.py != 179){
      P1.pos_spr = 1;
    }
    else{
      P1.pos_spr = 0;
    }
    if( !(((P1.py+38)>=P2.py) && (P1.py<(P2.py)) && ((P1.px+33)>P2.px) && (P1.px<(P2.px+33)) )){
      P1.py ++;
      if (P1.py > 179){
        P1.py = 179;
      }
    }
  }
  
  if (digitalRead(PUSH_UP) == 0 && !AtaqueP1.tecnica) {
    P1.pos_spr = 1;
    if( !((P1.py<=(P2.py+38)) && (P1.py>P2.py) && ((P1.px+33)>P2.px) && (P1.px<(P2.px+33))) ){
      P1.py --;
      if (P1.py < 65){
        P1.py = 65;
      }
    }
  }
  
  if (digitalRead(PUSH_A) == 0){
    P1.pos_spr = 3;
    if (P1.ki >= 10 && !AtaqueP1.zeit ){
      if(AtaqueP1.tecnica == 0){
         // ATK_limpiar(P1.orientacion, P1.px, AtaqueP1.px, AtaqueP1.py, 17,13);
          AtaqueP1.py = P1.py + 13;
          AtaqueP1.damage = 5;
          AtaqueP1.spr = 0;
          AtaqueP1.gasto = 10;
          P1.ki -= AtaqueP1.gasto;
          FillRect(46+P1.ki,32,100-P1.ki,10,0x763e);
          switch(P1.orientacion){
            case 0:
              AtaqueP1.px = P1.px + 35;
              LCD_SD_Sprite(AtaqueP1.px,AtaqueP1.py,17,13,2,AtaqueP1.spr,P1.orientacion,0,dir_ataqueA[P1.seleccion]);
              AtaqueP1.px +=17;
              AtaqueP1.tecnica = 1;
              AtaqueP1.spr = 1;
              break;
            case 1:
              if(P1.px<40){
                AtaqueP1.tecnica = 0;
                AtaqueP1.zeit = millis();
              }
              else{
                AtaqueP1.px = P1.px - 17;
                LCD_SD_Sprite(AtaqueP1.px,AtaqueP1.py,17,13,2,AtaqueP1.spr,P1.orientacion,0,dir_ataqueA[P1.seleccion]);
                AtaqueP1.px -=17;
                AtaqueP1.tecnica = 1;
                AtaqueP1.spr = 1;
              }
              
              break;
          }
      }
     }
   }
   else{
    if(AtaqueP1.spr == 1 && AtaqueP1.tecnica == 1){
      AtaqueP1.zeit = ATK_limpiar(P1.orientacion, P1.px, AtaqueP1.px, AtaqueP1.py, 17,13);
      AtaqueP1.tecnica = 0;
    }
   }

   if (digitalRead(PUSH_B) == 0 && digitalRead(PUSH_DOWN)){
    P1.pos_spr = 4;
    if (P1.ki >= 80 && !AtaqueP1.zeit ){
      if(!AtaqueP1.tecnica){
        //limpiar
        AtaqueP1.py = P1.py;
        AtaqueP1.damage = 30;
        AtaqueP1.gasto = 80;
        P1.ki -= AtaqueP1.gasto;  
        FillRect(46+P1.ki,32,100-P1.ki,10,0x763e);
        switch(P1.orientacion){
          case 0:
            AtaqueP1.px = P1.px + 35;
            LCD_SD_Sprite(AtaqueP1.px, AtaqueP1.py,12,38,1,0,P1.orientacion,0, dir_ataqueB1[P1.seleccion]);
            AtaqueP1.px +=12;
            AtaqueP1.py +=4;
            LCD_SD_Sprite(AtaqueP1.px, AtaqueP1.py,52,30,1,0,P1.orientacion,0, dir_ataqueB2[P1.seleccion]);
            AtaqueP1.tecnica = 2;
            break;
          case 1:
            if(P1.px<45){
              AtaqueP1.tecnica =0;
              AtaqueP1.zeit = millis();
            }
            else{
              AtaqueP1.px = P1.px - 12;
              LCD_SD_Sprite(AtaqueP1.px, AtaqueP1.py,12,38 ,1,0,P1.orientacion,0, dir_ataqueB1[P1.seleccion]);
              AtaqueP1.px -=52;
              AtaqueP1.py +=4;
              LCD_SD_Sprite(AtaqueP1.px, AtaqueP1.py,52,30,1,0,P1.orientacion,0, dir_ataqueB2[P1.seleccion]);
              AtaqueP1.tecnica = 2;
            }
            break;
        }
      }
    }
  }
  else{
    if(AtaqueP1.tecnica == 2){
      AtaqueP1.zeit = millis();
      AtaqueP1.tecnica = 0;
      switch(P1.orientacion){
        case 0:
          FillRect(P1.px+33,P1.py,(AtaqueP1.px + 52)-(P1.px+33),38,0x763e);
          break;
        case 1:
          FillRect(AtaqueP1.px,P1.py,P1.px-AtaqueP1.px,38,0x763e);
          break;
      }
    }
  }

  if( !digitalRead(PUSH_B) && !digitalRead(PUSH_DOWN)){
    AtaqueP1.zeit = millis();
    P1.pos_spr = 2;
    if(P1.ki <101){
      P1.ki ++;
      //FillRect(45,32,P1.ki,10,0x1A0B);
      V_line(45+P1.ki,32,9, 0x1A0B);
    }
    
    
  }

    if (digitalRead(PUSH_UP) == 1 &&
        digitalRead(PUSH_DOWN) == 1 &&
        digitalRead(PUSH_RIGHT) == 1 &&
        digitalRead(PUSH_LEFT) == 1 &&
        digitalRead(PUSH_A) == 1 &&
        digitalRead(PUSH_B) == 1){

     P1.pos_spr = 0;    
     AtaqueP1.tecnica = 0;
    }

// Controles JUGADOR 2

  if (digitalRead(PUSH2_RIGHT) == 0 && !AtaqueP2.tecnica){
    P2.orientacion = 0;
    P2.pos_spr = 1;
    if( !(((P2.px+33)>=P1.px) && (P2.px<(P1.px+33)) && (P2.py<(P1.py+38)) && ((P2.py+38)>P1.py))){
      P2.px +=1;
      if (P2.px > 285){
        P2.px = 285;
      }
    }
  }

  if (digitalRead(PUSH2_LEFT) == 0 && !AtaqueP2.tecnica){
    P2.orientacion = 1;
    P2.pos_spr = 1;
    if( !((P2.px<=(P1.px+33)) && (P2.px>=P1.px) && (P2.py<(P1.py+38)) && ((P2.py+38)>P1.py) ) ){
      P2.px --;
      if(P2.px < 2){
        P2.px = 2;
      }
    }
  }
  
   if (digitalRead(PUSH2_DOWN) == 0 && digitalRead(PUSH2_B) && !AtaqueP2.tecnica ){
    if (P2.py != 179){
      P2.pos_spr = 1;
    }
    else{
      P2.pos_spr = 0;
    }
    if( !(((P2.py+38)>=P1.py) && (P2.py<(P1.py)) && ((P2.px+33)>P1.px) && (P2.px<(P1.px+33)) )){
      P2.py ++;
      if (P2.py > 179){
        P2.py = 179;
      }
    }
  }
  
  if (digitalRead(PUSH2_UP) == 0 && !AtaqueP2.tecnica) {
    P2.pos_spr = 1;
    if( !((P2.py<=(P1.py+38)) && (P2.py>P1.py) && ((P2.px+33)>P1.px) && (P2.px<(P1.px+33))) ){
      P2.py --;
      if (P2.py < 65){
        P2.py = 65;
      }
    }
  }
  
  if (digitalRead(PUSH2_A) == 0){
    P2.pos_spr = 3;
    if (P2.ki >= 10 && !AtaqueP2.zeit ){
      if(AtaqueP2.tecnica == 0){
          //ATK_limpiar(P2.orientacion, P2.px, AtaqueP2.px, AtaqueP2.py, 17,13);
          AtaqueP2.py = P2.py + 13;
          AtaqueP2.damage = 5;
          AtaqueP2.spr = 0;
          AtaqueP2.gasto = 10;
          P2.ki -= AtaqueP2.gasto;
          FillRect(271-P2.ki-AtaqueP2.gasto,32,AtaqueP2.gasto,10,0x763e); //*******
          switch(P2.orientacion){
            case 0:
              AtaqueP2.px = P2.px + 35;
              LCD_SD_Sprite(AtaqueP2.px,AtaqueP2.py,17,13,2,AtaqueP2.spr,P2.orientacion,0,dir_ataqueA[P2.seleccion]);
              AtaqueP2.px +=17;
              AtaqueP2.tecnica = 1;
              AtaqueP2.spr = 1;
              break;
            case 1:
              if(P2.px<40){
                AtaqueP2.tecnica = 0;
                AtaqueP2.zeit = millis();
              }
              else{
                AtaqueP2.px = P2.px - 17;
                LCD_SD_Sprite(AtaqueP2.px,AtaqueP2.py,17,13,2,AtaqueP2.spr,P2.orientacion,0,dir_ataqueA[P2.seleccion]);
                AtaqueP2.px -=17;
                AtaqueP2.tecnica = 1;
                AtaqueP2.spr = 1;
              }
              
              break;
          }
      }
     }
   }
   else{
    if(AtaqueP2.spr == 1 && AtaqueP2.tecnica == 1){
      AtaqueP2.zeit = ATK_limpiar(P2.orientacion, P2.px, AtaqueP2.px, AtaqueP2.py, 17,13);
      AtaqueP2.tecnica = 0;
    }
   }

   if (digitalRead(PUSH2_B) == 0 && digitalRead(PUSH2_DOWN)){
    P2.pos_spr = 4;
    if (P2.ki >= 80 && !AtaqueP2.zeit ){
      if(!AtaqueP2.tecnica){
        //limpiar
        AtaqueP2.py = P2.py;
        AtaqueP2.damage = 30;
        AtaqueP2.gasto = 80;
        P2.ki -= AtaqueP2.gasto;  
        FillRect(271-P2.ki-AtaqueP2.gasto,32,AtaqueP2.gasto,10,0x763e);  //*****
        switch(P2.orientacion){
          case 0:
            AtaqueP2.px = P2.px + 35;
            LCD_SD_Sprite(AtaqueP2.px, AtaqueP2.py,12,38,1,0,P2.orientacion,0, dir_ataqueB1[P2.seleccion]);
            AtaqueP2.px +=12;
            AtaqueP2.py +=4;
            LCD_SD_Sprite(AtaqueP2.px, AtaqueP2.py,52,30,1,0,P2.orientacion,0, dir_ataqueB2[P2.seleccion]);
            AtaqueP2.tecnica = 2;
            break;
          case 1:
            if(P2.px<45){
              AtaqueP2.tecnica =0;
              AtaqueP2.zeit = millis();
            }
            else{
              AtaqueP2.px = P2.px - 12;
              LCD_SD_Sprite(AtaqueP2.px, AtaqueP2.py,12,38 ,1,0,P2.orientacion,0, dir_ataqueB1[P2.seleccion]);
              AtaqueP2.px -=52;
              AtaqueP2.py +=4;
              LCD_SD_Sprite(AtaqueP2.px, AtaqueP2.py,52,30,1,0,P2.orientacion,0, dir_ataqueB2[P2.seleccion]);
              AtaqueP2.tecnica = 2;
            }
            break;
        }
      }
    }
  }
  else{
    if(AtaqueP2.tecnica == 2){
      AtaqueP2.zeit = millis();
      AtaqueP2.tecnica = 0;
      switch(P2.orientacion){
        case 0:
          FillRect(P2.px+33,P2.py,(AtaqueP2.px + 52)-(P2.px+33),38,0x763e);
          break;
        case 1:
          FillRect(AtaqueP2.px,P2.py,P2.px-AtaqueP2.px,38,0x763e);
          break;
      }
    }
  }

  if( !digitalRead(PUSH2_B) && !digitalRead(PUSH2_DOWN)){
    AtaqueP2.zeit = millis();
    P2.pos_spr = 2;
    if(P2.ki <101){
      P2.ki ++;
      //FillRect(45,32,P1.ki,10,0x1A0B);
      V_line(272-P2.ki,32,9, 0x1A0B);  //**************
      //V_line(272,32,9,0xA8A3);
    }
    
    
  }

    if (digitalRead(PUSH2_UP) == 1 &&
        digitalRead(PUSH2_DOWN) == 1 &&
        digitalRead(PUSH2_RIGHT) == 1 &&
        digitalRead(PUSH2_LEFT) == 1 &&
        digitalRead(PUSH2_A) == 1 &&
        digitalRead(PUSH2_B) == 1){

     P2.pos_spr = 0;    
     AtaqueP2.tecnica = 0;
    }

  

  
}
//***************************************************************************************************************************************
// Función para inicializar LCD
//***************************************************************************************************************************************
void LCD_Init(void) {
  pinMode(LCD_RST, OUTPUT);
  pinMode(LCD_CS, OUTPUT);
  pinMode(LCD_RS, OUTPUT);
  pinMode(LCD_WR, OUTPUT);
  pinMode(LCD_RD, OUTPUT);
  for (uint8_t i = 0; i < 8; i++){
    pinMode(DPINS[i], OUTPUT);
  }
  //****************************************
  // Secuencia de Inicialización
  //****************************************
  digitalWrite(LCD_CS, HIGH);
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_WR, HIGH);
  digitalWrite(LCD_RD, HIGH);
  digitalWrite(LCD_RST, HIGH);
  delay(5);
  digitalWrite(LCD_RST, LOW);
  delay(20);
  digitalWrite(LCD_RST, HIGH);
  delay(150);
  digitalWrite(LCD_CS, LOW);
  //****************************************
  LCD_CMD(0xE9);  // SETPANELRELATED
  LCD_DATA(0x20);
  //****************************************
  LCD_CMD(0x11); // Exit Sleep SLEEP OUT (SLPOUT)
  delay(100);
  //****************************************
  LCD_CMD(0xD1);    // (SETVCOM)
  LCD_DATA(0x00);
  LCD_DATA(0x71);
  LCD_DATA(0x19);
  //****************************************
  LCD_CMD(0xD0);   // (SETPOWER) 
  LCD_DATA(0x07);
  LCD_DATA(0x01);
  LCD_DATA(0x08);
  //****************************************
  LCD_CMD(0x36);  // (MEMORYACCESS)
  LCD_DATA(0x40|0x80|0x20|0x08); // LCD_DATA(0x19);
  //****************************************
  LCD_CMD(0x3A); // Set_pixel_format (PIXELFORMAT)
  LCD_DATA(0x05); // color setings, 05h - 16bit pixel, 11h - 3bit pixel
  //****************************************
  LCD_CMD(0xC1);    // (POWERCONTROL2)
  LCD_DATA(0x10);
  LCD_DATA(0x10);
  LCD_DATA(0x02);
  LCD_DATA(0x02);
  //****************************************
  LCD_CMD(0xC0); // Set Default Gamma (POWERCONTROL1)
  LCD_DATA(0x00);
  LCD_DATA(0x35);
  LCD_DATA(0x00);
  LCD_DATA(0x00);
  LCD_DATA(0x01);
  LCD_DATA(0x02);
  //****************************************
  LCD_CMD(0xC5); // Set Frame Rate (VCOMCONTROL1)
  LCD_DATA(0x04); // 72Hz
  //****************************************
  LCD_CMD(0xD2); // Power Settings  (SETPWRNORMAL)
  LCD_DATA(0x01);
  LCD_DATA(0x44);
  //****************************************
  LCD_CMD(0xC8); //Set Gamma  (GAMMASET)
  LCD_DATA(0x04);
  LCD_DATA(0x67);
  LCD_DATA(0x35);
  LCD_DATA(0x04);
  LCD_DATA(0x08);
  LCD_DATA(0x06);
  LCD_DATA(0x24);
  LCD_DATA(0x01);
  LCD_DATA(0x37);
  LCD_DATA(0x40);
  LCD_DATA(0x03);
  LCD_DATA(0x10);
  LCD_DATA(0x08);
  LCD_DATA(0x80);
  LCD_DATA(0x00);
  //****************************************
  LCD_CMD(0x2A); // Set_column_address 320px (CASET)
  LCD_DATA(0x00);
  LCD_DATA(0x00);
  LCD_DATA(0x01);
  LCD_DATA(0x3F);
  //****************************************
  LCD_CMD(0x2B); // Set_page_address 480px (PASET)
  LCD_DATA(0x00);
  LCD_DATA(0x00);
  LCD_DATA(0x01);
  LCD_DATA(0xE0);
//  LCD_DATA(0x8F);
  LCD_CMD(0x29); //display on 
  LCD_CMD(0x2C); //display on

  LCD_CMD(ILI9341_INVOFF); //Invert Off
  delay(120);
  LCD_CMD(ILI9341_SLPOUT);    //Exit Sleep
  delay(120);
  LCD_CMD(ILI9341_DISPON);    //Display on
  digitalWrite(LCD_CS, HIGH);
}
//***************************************************************************************************************************************
// Función para enviar comandos a la LCD - parámetro (comando)
//***************************************************************************************************************************************
void LCD_CMD(uint8_t cmd) {
  digitalWrite(LCD_RS, LOW);
  digitalWrite(LCD_WR, LOW);
  GPIO_PORTB_DATA_R = cmd;
  digitalWrite(LCD_WR, HIGH);
}
//***************************************************************************************************************************************
// Función para enviar datos a la LCD - parámetro (dato)
//***************************************************************************************************************************************
void LCD_DATA(uint8_t data) {
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_WR, LOW);
  GPIO_PORTB_DATA_R = data;
  digitalWrite(LCD_WR, HIGH);
}
//***************************************************************************************************************************************
// Función para definir rango de direcciones de memoria con las cuales se trabajara (se define una ventana)
//***************************************************************************************************************************************
void SetWindows(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2) {
  LCD_CMD(0x2a); // Set_column_address 4 parameters
  LCD_DATA(x1 >> 8);
  LCD_DATA(x1);   
  LCD_DATA(x2 >> 8);
  LCD_DATA(x2);   
  LCD_CMD(0x2b); // Set_page_address 4 parameters
  LCD_DATA(y1 >> 8);
  LCD_DATA(y1);   
  LCD_DATA(y2 >> 8);
  LCD_DATA(y2);   
  LCD_CMD(0x2c); // Write_memory_start
}
//***************************************************************************************************************************************
// Función para borrar la pantalla - parámetros (color)
//***************************************************************************************************************************************
void LCD_Clear(unsigned int c){  
  unsigned int x, y;
  LCD_CMD(0x02c); // write_memory_start
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_CS, LOW);   
  SetWindows(0, 0, 319, 239); // 479, 319);
  for (x = 0; x < 320; x++)
    for (y = 0; y < 240; y++) {
      LCD_DATA(c >> 8); 
      LCD_DATA(c); 
    }
  digitalWrite(LCD_CS, HIGH);
} 
//***************************************************************************************************************************************
// Función para dibujar una línea horizontal - parámetros ( coordenada x, cordenada y, longitud, color)
//*************************************************************************************************************************************** 
void H_line(unsigned int x, unsigned int y, unsigned int l, unsigned int c) {  
  unsigned int i, j;
  LCD_CMD(0x02c); //write_memory_start
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_CS, LOW);
  l = l + x;
  SetWindows(x, y, l, y);
  j = l;// * 2;
  for (i = 0; i < l; i++) {
      LCD_DATA(c >> 8); 
      LCD_DATA(c); 
  }
  digitalWrite(LCD_CS, HIGH);
}
//***************************************************************************************************************************************
// Función para dibujar una línea vertical - parámetros ( coordenada x, cordenada y, longitud, color)
//*************************************************************************************************************************************** 
void V_line(unsigned int x, unsigned int y, unsigned int l, unsigned int c) {  
  unsigned int i,j;
  LCD_CMD(0x02c); //write_memory_start
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_CS, LOW);
  l = l + y;
  SetWindows(x, y, x, l);
  j = l; //* 2;
  for (i = 1; i <= j; i++) {
    LCD_DATA(c >> 8); 
    LCD_DATA(c);
  }
  digitalWrite(LCD_CS, HIGH);  
}
//***************************************************************************************************************************************
// Función para dibujar un rectángulo - parámetros ( coordenada x, cordenada y, ancho, alto, color)
//***************************************************************************************************************************************
void Rect(unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int c) {
  H_line(x  , y  , w, c);
  H_line(x  , y+h, w, c);
  V_line(x  , y  , h, c);
  V_line(x+w, y  , h, c);
}
//***************************************************************************************************************************************
// Función para dibujar un rectángulo relleno - parámetros ( coordenada x, cordenada y, ancho, alto, color)
//***************************************************************************************************************************************
void FillRect(unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int c) {
  unsigned int i;
  for (i = 0; i < h; i++) {
    H_line(x  , y  , w, c);
    H_line(x  , y+i, w, c);
  }
}
//***************************************************************************************************************************************
// Función para dibujar texto - parámetros ( texto, coordenada x, cordenada y, color, background) 
//***************************************************************************************************************************************
void LCD_Print(String text, int x, int y, int fontSize, int color, int background) {
  int fontXSize ;
  int fontYSize ;
  
  if(fontSize == 1){
    fontXSize = fontXSizeSmal ;
    fontYSize = fontYSizeSmal ;
  }
  if(fontSize == 2){
    fontXSize = fontXSizeBig ;
    fontYSize = fontYSizeBig ;
  }
  
  char charInput ;
  int cLength = text.length();
  Serial.println(cLength,DEC);
  int charDec ;
  int c ;
  int charHex ;
  char char_array[cLength+1];
  text.toCharArray(char_array, cLength+1) ;
  for (int i = 0; i < cLength ; i++) {
    charInput = char_array[i];
    Serial.println(char_array[i]);
    charDec = int(charInput);
    digitalWrite(LCD_CS, LOW);
    SetWindows(x + (i * fontXSize), y, x + (i * fontXSize) + fontXSize - 1, y + fontYSize );
    long charHex1 ;
    for ( int n = 0 ; n < fontYSize ; n++ ) {
      if (fontSize == 1){
        charHex1 = pgm_read_word_near(smallFont + ((charDec - 32) * fontYSize) + n);
      }
      if (fontSize == 2){
        charHex1 = pgm_read_word_near(bigFont + ((charDec - 32) * fontYSize) + n);
      }
      for (int t = 1; t < fontXSize + 1 ; t++) {
        if (( charHex1 & (1 << (fontXSize - t))) > 0 ) {
          c = color ;
        } else {
          c = background ;
        }
        LCD_DATA(c >> 8);
        LCD_DATA(c);
      }
    }
    digitalWrite(LCD_CS, HIGH);
  }
}
//***************************************************************************************************************************************
// Función para dibujar una imagen a partir de un arreglo de colores (Bitmap) Formato (Color 16bit R 5bits G 6bits B 5bits)
//***************************************************************************************************************************************
void LCD_Bitmap(unsigned int x, unsigned int y, unsigned int width, unsigned int height, unsigned char bitmap[]){  
  LCD_CMD(0x02c); // write_memory_start
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_CS, LOW); 
  
  unsigned int x2, y2;
  x2 = x+width;
  y2 = y+height;
  SetWindows(x, y, x2-1, y2-1);
  unsigned int k = 0;
  unsigned int i, j;

  for (int i = 0; i < width; i++) {
    for (int j = 0; j < height; j++) {
      LCD_DATA(bitmap[k]);
      LCD_DATA(bitmap[k+1]);
      //LCD_DATA(bitmap[k]);    
      k = k + 2;
     } 
  }
  digitalWrite(LCD_CS, HIGH);
}
//***************************************************************************************************************************************
// Función para dibujar una imagen sprite - los parámetros columns = número de imagenes en el sprite, index = cual desplegar, flip = darle vuelta
//***************************************************************************************************************************************
void LCD_Sprite(int x, int y, int width, int height, unsigned char bitmap[],int columns, int index, char flip, char offset){
  LCD_CMD(0x02c); // write_memory_start
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_CS, LOW); 

  unsigned int x2, y2;
  x2 =   x+width;
  y2=    y+height;
  SetWindows(x, y, x2-1, y2-1);
  int k = 0;
  int ancho = ((width*columns));
  if(flip){
  for (int j = 0; j < height; j++){
      k = (j*(ancho) + index*width -1 - offset)*2;
      k = k+width*2;
     for (int i = 0; i < width; i++){
      LCD_DATA(bitmap[k]);
      LCD_DATA(bitmap[k+1]);
      k = k - 2;
     } 
  }
  }else{
     for (int j = 0; j < height; j++){
      k = (j*(ancho) + index*width + 1 + offset)*2;
     for (int i = 0; i < width; i++){
      LCD_DATA(bitmap[k]);
      LCD_DATA(bitmap[k+1]);
      k = k + 2;
     } 
  }
    
    
    }
  digitalWrite(LCD_CS, HIGH);
}

//************************************************************************************************
//  animacion de golpe 
//************************************************************************************************
void Anim_golpe(unsigned char personaje_golpe[], uint16_t pos_x, uint16_t pos_y,bool orientacion){
  for(uint8_t n = 0; n<6;n++){
    LCD_Sprite(pos_x,pos_y,36,47,personaje_golpe,5,n,orientacion,0);
    //LCD_SD_Sprite(pos_x,pos_y,36,47,5, 2, 1, 0,"buSpr1.txt");
    delay(60);
  }
}

//************************************************************************************************
//  convertir caracter hex a su numero en unsigned int
//************************************************************************************************
unsigned char Char_to_uChar(char letra){
  unsigned char num;
  if(letra>=48 && letra <=57){
    num = letra - 48;
  }
  else if (letra >= 97 && letra <=102){
    num = letra -87;
  }
  return num;
}

//***************************************************************************************************************************************
// Función para dibujar una imagen a partir de un arreglo de colores (Bitmap) en la SD Formato (Color 16bit R 5bits G 6bits B 5bits)
//*************************************************************************************************************************************
//void LCD_Bitmap(unsigned int x, unsigned int y, unsigned int width, unsigned int height, unsigned char bitmap[])
void LCD_SD_Bitmap(unsigned int x, unsigned int y, unsigned int width, unsigned int height, char * direccion){
  File myFile = SD.open(direccion);
  uint16_t n = 0;
  uint16_t dimension = width*height*2;
  unsigned char vegueta[dimension] = {};
  if (myFile) {
    // read from the file until there's nothing else in it:
    while (myFile.available()) {
      //Serial.write(myFile.read());
      //delay(500);
      unsigned char numero = 0;
      for(uint8_t m = 0; m < 2; m++){
        char caracter = myFile.read();
        unsigned char digito = Char_to_uChar(caracter);
        if (m == 0){
          numero = digito*16;
        }
        else if (m == 1){
          numero = numero + digito;
        }
      }
      vegueta[n] = numero;
      n ++;
    }
    // close the file:
    myFile.close();
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening ");
  }
   LCD_Bitmap(x,y,width,height,vegueta);
}

//***************************************************************************************************************************************
// Función para dibujar una imagen sprite desde la SD - los parámetros columns = número de imagenes en el sprite, index = cual desplegar, flip = darle vuelta
//***************************************************************************************************************************************
void LCD_SD_Sprite(int x, int y, int width, int height,int columns, int index, char flip, char offset,char * direccion){
  File myFile = SD.open(direccion);
  uint16_t n = 0;
  uint16_t dimension = width*columns*height*2;
  unsigned char vegueta[dimension] = {};
  if (myFile) {
    // read from the file until there's nothing else in it:
    while (myFile.available()) {
      //Serial.write(myFile.read());
      //delay(500);
      unsigned char numero = 0;
      for(uint8_t m = 0; m < 2; m++){
        char caracter = myFile.read();
        unsigned char digito = Char_to_uChar(caracter);
        if (m == 0){
          numero = digito*16;
        }
        else if (m == 1){
          numero = numero + digito;
        }
      }
      vegueta[n] = numero;
      n ++;
    }
    // close the file:
    myFile.close();
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening ");
  }
   LCD_Sprite(x,y,width,height,vegueta,columns,index,flip,offset);
}

//***************************************************************************************************************************************
// Función para cargar sprite Player 1
//***************************************************************************************************************************************
void SD_loadP1_Sprite(char * direccion){
  File myFile = SD.open(direccion);
  uint16_t n = 0;
  //uint16_t dimension = width*columns*height*2;
  //unsigned char vegueta[dimension] = {};
  if (myFile) {
    // read from the file until there's nothing else in it:
    while (myFile.available()) {
      //Serial.write(myFile.read());
      //delay(500);
      unsigned char numero = 0;
      for(uint8_t m = 0; m < 2; m++){
        char caracter = myFile.read();
        unsigned char digito = Char_to_uChar(caracter);
        if (m == 0){
          numero = digito*16;
        }
        else if (m == 1){
          numero = numero + digito;
        }
      }
      player1[n] = numero;
      n ++;
    }
    // close the file:
    myFile.close();
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening ");
  }
}
//***************************************************************************************************************************************
// Función para cargar sprite Player 2
//***************************************************************************************************************************************
void SD_loadP2_Sprite(char * direccion){
  File myFile = SD.open(direccion);
  uint16_t n = 0;
  //uint16_t dimension = width*columns*height*2;
  //unsigned char vegueta[dimension] = {};
  if (myFile) {
    // read from the file until there's nothing else in it:
    while (myFile.available()) {
      //Serial.write(myFile.read());
      //delay(500);
      unsigned char numero = 0;
      for(uint8_t m = 0; m < 2; m++){
        char caracter = myFile.read();
        unsigned char digito = Char_to_uChar(caracter);
        if (m == 0){
          numero = digito*16;
        }
        else if (m == 1){
          numero = numero + digito;
        }
      }
      player2[n] = numero;
      n ++;
    }
    // close the file:
    myFile.close();
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening ");
  }
}

//***************************************************************************************************************************************
// Función para cargar cuadro con avatar de cada personaje para cada jugador
//***************************************************************************************************************************************
void Per_avatar(uint8_t player_select,uint16_t x, uint8_t y, uint8_t ancho, uint8_t alto){
  char * dir[] = {"xxPe.txt", "GoPe.txt","FrePe.txt","VegPe.txt", "CePe.txt", "BuPe.txt"};
  LCD_SD_Bitmap(x,y,ancho,alto,dir[player_select]);
}

//***************************************************************************************************************************************
// Función para cargar estado iniciales al terminar partida
//***************************************************************************************************************************************
void reset_Estados(void){
  P1.py = 179;
  P1.px = 20;
  P1.pos_spr = 0;
  P1.orientacion = 0;
  P1.vida = 100;
  P1.ki = 50;
  P1.listo = 0;
  AtaqueP1.zeit = 0;
  AtaqueP1.py = 100;
  AtaqueP1.px = 100;
  AtaqueP1.spr = 0;
  AtaqueP1.tecnica = 0;

  P2.py = 179;
  P2.px = 266;
  P2.pos_spr = 0;
  P2.orientacion = 1;
  P2.vida = 100;
  P2.ki = 50;
  P2.listo = 0;
  AtaqueP2.zeit = 0;
  AtaqueP2.py = 100;
  AtaqueP2.px = 100;
  AtaqueP2.spr = 0;
  AtaqueP2.tecnica = 0;

}

//***************************************************************************************************************************************
// Función para limpiar rastro de ataque
//***************************************************************************************************************************************
unsigned long ATK_limpiar(uint8_t orientacion, uint16_t x, uint16_t ATKx, uint8_t ATKy,uint8_t dimX, uint8_t dimY){
  switch(orientacion){
    case 0:
      FillRect(x+35,ATKy,dimX, dimY, 0x763e);
      FillRect(ATKx, ATKy, dimX,dimY,0x763e);
      break;
    case 1:
      FillRect(x - dimX,ATKy,dimX, dimY, 0x763e);
      FillRect(ATKx, ATKy, dimX,dimY,0x763e);
      break;
  }
  unsigned long tiempo = millis(); 
  return(tiempo); 
}

//***************************************************************************************************************************************
// Función mostrar fina de pelea
//***************************************************************************************************************************************
void GAME_OVER(uint8_t ganador){
  LCD_Bitmap(125,100,69,44,KO);
  switch(ganador){
    case 1:
    {
      String gana1 = "P1 GANA!";
      LCD_Print(gana1, 95 ,75,2,0xFFFF,0xA8A3);
    }
      break;
    case 2:
    {
      String gana2 = "P2 GANA!";
      LCD_Print(gana2, 95,75,2,0xFFFF,0x1A0B);
    }
      break;
  }
  delay(4000);
}
