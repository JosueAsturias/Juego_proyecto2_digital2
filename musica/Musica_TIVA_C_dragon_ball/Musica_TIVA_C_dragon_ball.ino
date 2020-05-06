/*
 * Miguel García 17560
 * Josué Asturias 17262
 * Electrónica Digital 2
 * Sección:20
 * 
 * Proyecto 2 - música Dragon Ball Z
 */

//#include <Wire.h>

  // put your setup code here, to run once:
int SDA = PA_7;   
int SCL = PA_6;  
int btn3 = PD_7;
int pinSound = PC_4;   
int altaveu = PB_3;
int notaActual = 0;

int  DO=523.25, //definimos las frecuencias de las notas
        DOS=554.37,
        RE=587.33,
        RES=622.25,
        MI=659.26,
        FA=698.46,
        FAS=739.99,
        SOL=783.99,
        SOLS=830.61,
        LA=880,
        LAS=932.33,
        SI=987.77,
        RE2=1174.66,
        FAS2=1479.98,
        DO2 =1046.5,
        SI2 = 19975.53,
        MI2=1318.51,
        PAU=30000; //pausa

int d1=900,d=850, e=650, m=450, c=200, o=100; //definimos los tiempos de doble, entera, media y cuarta

int melodia[] ={
698,698,622,523,698,698,698,622,523,698,
466,466,466,415,466,466,466,415,349,698,
698,622,523,698,698,698,622,523,698,466,
466,466,415,466,466,466,415,523,523,622,
698,698,698,622,523,622,523,622,622,698,
523,622,698,830,783,698,622,830,783,698
};

int duracionNota[] ={
141,132,132,132,523,132,132,132,132,526,
132,132,132,132,132,132,132,132,1053,132,
132,132,132,526,132,132,132,132,526,132,
132,132,132,132,132,132,132,1053,132,132,
263,263,263,132,132,132,132,132,132,526,
132,132,526,132,132,132,263,132,132,263
};

long unsigned int duracion2[] = {
232,232,232,232,232,232,232,232,232,232,
232,232,232,232,232,232,232,232,829,232,
232,232,232,232,232,232,232,232,232,232,
232,232,232,232,232,232,232,232,232,232,
395,395,395,232,232,232,232,79,232,232,
232,232,829,232,232,232,395,232,232,395
};

int melodia3[]= {
SI2,FAS,MI,SI,MI,FAS,SI2,FAS,MI,SI,MI,FAS,SI2,FAS,SI,PAU,LA,PAU,SOLS,MI,PAU,LA,PAU,SOLS,PAU,SOLS,PAU,SOLS, //28
FAS,FAS,FAS,PAU,FAS,FAS,MI,MI,MI,          //9
PAU,LA,SOLS,MI,SI2,LA,LA,SOLS,LA,SI,SI,LA,LA,MI2,MI,FAS,SOL,   //17
LA,SI,DOS,RE,RE,DOS,DO,SI,LA,SOLS,SI,   //11
PAU,LA,SOLS,MI,MI,SI2,SI2,LA,LA,SOLS,LA,SI2,SI2,LA,MI2,MI,FAS,SOLS,      //18
LA,SI2,DO2,RE2,DO,SI2,SI2,SI2,PAU,   //9
DOS,PAU,SOLS,SOLS,DOS,SI2,LA,SOLS,FAS,LA,SOLS,SOLS     //12
};

int duracionNota3[]={
o,o,o,o,o,o,o,o,o,o,o,o,o,o,c,m,c,c,c,c,c,c,c,c,m,c,c,m,
o,c,d1,c,c,c,o,c,d1,
c,c,o,c,m,c,c,c,m,c,c,c,c,e,c,c,e,
e,m,e,c,m,c,c,c,c,c,e,
c,m,c,m,c,c,c,c,c,c,c,m,c,m,d,c,c,e,
m,c,c,c,o,c,c,d,c,
m,o,o,o,c,c,c,c,c,m,c,e
};

int melodia2[] = {
1432,1275,1432,1136,1432,1014,1432,1275,1432,1432,
1136,1275,1432,1519,1432,1275,1136,1014,1136,1136,
956,1014,1136,1275,1136,1014,956,892,836,836,
956,1519,1432,956,1014,1136,1275,1136,1014,956,
1014,1014,956,892,956
};

int duracionNota2[] = {
125,125,125,125,125,125,125,125,250,250,
125,125,125,125,250,250,250,250,250,250,
125,125,125,125,250,250,250,250,125,125,
250,250,500,250,125,125,250,250,250,125,
125,500,250,250,500
};


//int x;    //variable de recibir en i2c

void setup() {
  //Wire.begin(20);                // join i2c bus with address #20
  //Wire.onReceive(receiveEvent); // register event
  Serial.begin(9600);           // start serial for output
  pinMode (pinSound,OUTPUT); // PIN DEL BUZZER O ALTOPARLANTE
  pinMode (SDA, INPUT); // BOTON PARA INICIAR LAS CANCIONES 
  pinMode (SCL, INPUT);
  pinMode (btn3, INPUT);
  pinMode (altaveu,OUTPUT); // PIN DEL BUZZER O ALTOPARLANTE
}

void loop() {
  // put your main code here, to run repeatedly:
  notaActual = 0;
  while((digitalRead(SCL)==0) && (digitalRead(SDA)==1))
   { 
    tone(pinSound, melodia[notaActual]); //da el tono a la frecuencia de la nota en ese momento
    delay(duracionNota[notaActual]);//se mantiene con la nota el tiempo definido para esa nota
    noTone(pinSound); //finaliza la melodía
    notaActual++;
    if (notaActual ==60){
      notaActual = 0;
    }
   }
    notaActual = 0;
    while((digitalRead(SCL)==1) && (digitalRead(SDA)==1 ))
   { 
    tone(pinSound, melodia2[notaActual]); //da el tono a la frecuencia de la nota en ese momento
    delay(duracionNota2[notaActual]);//se mantiene con la nota el tiempo definido para esa nota
    noTone(pinSound); //finaliza la melodía
    notaActual++;
    if (notaActual ==44){
      notaActual = 0;
    }
   }

   
  notaActual = 0;
  while((digitalRead(SCL)==1) && (digitalRead(SDA)==0))
   {    
    tone(pinSound, melodia3[notaActual]); //da el tono a la frecuencia de la nota en ese momento
    delay(duracionNota3[notaActual]);//se mantiene con la nota el tiempo definido para esa nota
    noTone(pinSound); //finaliza la melodía
    notaActual++;
    if (notaActual ==84){
      notaActual = 0;
    }
    }
}


/*void receiveEvent(int howMany) {
  while (1 < Wire.available()) { // loop through all but the last
    char c = Wire.read(); // receive byte as a character
    Serial.print(c);         // print the character
  }
  x = Wire.read();    // receive byte as an integer
  Serial.println(x);         // print the integer
}*/
