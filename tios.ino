#include <TFT_eSPI.h>
#include "NotoSansBold15.h"
#include <EEPROM.h>

TFT_eSPI tft = TFT_eSPI();
TFT_eSprite sprite = TFT_eSprite(&tft);
#define EEPROM_SIZE 48

#define gra 0x7BCF
#define red TFT_RED
#define blk TFT_BLACK
#define nc 0x0415

#define background TFT_WHITE

byte fromTop=52;
byte fromLeft=58;
byte w=24;
byte h=17;

unsigned short typeColor[5]={TFT_ORANGE,0x04FA,0x2E66,TFT_PURPLE,TFT_MAGENTA};
unsigned short leftPinsColor[24]={blk,blk,gra,gra,gra,gra,gra,gra,nc,blk,blk,red,red,gra,gra,gra,gra,gra,gra,gra,nc,nc,blk,red};
unsigned short stateColor[2]={TFT_BLACK,TFT_RED};

int pins[24]={100,100,43,44,18,17,21,16,100,100,100,100,100,1,2,3,10,11,12,13,100,100,100,100};
String leftPinsStr[28]={"G","G","43","44","18","17","21","16","NC","G","G","3V","3V","1","2","3","10","11","12","13","NC","NC","G","5V","0","14","T1","T2"};
String leftPinsStr2[28]={"G","G","43","44","18","17","21","16","NC","G","G","3V","3V","1","2","3","10","11","12","13","NC","NC","G","5V","PB1","PB2","T1","T2"};
byte leftPinsType[28]={0,0,0,0,2,4,0,3,0,0,0,0,0,0,0,0,3,0,3,5,0,0,0,0,1,2,6,6}; // 0=not set, 1=inp, 2=pullup,3=out,4=ana,5=pwm,6
int leftPinsState[28]={0};
byte leftPinsSource[28]={100,100,100,100,100,100,100,124,100,100,100,100,100,100,100,100,25,100,26,5,100,100,100,100,100,100};
int deb[28]={0};
bool presed[28]={0};
bool fase=0;
int debounce=0;
int ldb,rdb=0;

String typeStr[5]={"INP","SW","OUT","ANA","PWM"};

int x,y,l,l2,c,s,a;
unsigned long currentTime[2]={0,0};
unsigned long times[2][2]={{1000,500},{300,300}};
byte ts[4]={250,250,150,150}; 
byte tm[4]={10,10,10,10};
byte tSource[4]={0};
bool t1;
bool t2;

int menu=0;
int item=0;
int chosenOne=0;

String menuTitle[10]={"MENU","SELECT PIN","SELECT TYPE","SET SOURCE","PWM","SET TIMERS","","","MULTIPLIER","BRIGHTNESS"};

String firstMenu[10][28]={{"Exit","Reset All","Set Pin","Set Timer","Brightness","","","","","","","","","","","","","","","","","","","","","","",""},
                        {"BACK","43","44","18","17","21","16","1","2","3","10","11","12","13","PB1","PB2","T1","T2","","","","","","","","","",""},
                        {"BACK","NOT SET","INP_PULLUP","ON/OFF SW","OUTPUT","ANALOG ","PWM","","","","","","","","","","","","","","","","","","","","",""},
                        {"HIGH","LOW","T1","!T1","T2","!T2","PB1","!PB1","PB2","!PB","9","9","9","9","9","9","9","10","","","","","","","","","",""},
                        {"50","100","150"},
                        {"BACK","SET T1","SET T2"},
                        {"BACK","ON TIME","OFF TIME","MULTIPLIER"},
                        {"1","50","100","150","250"},
                        {"1","10","100","200","250"},
                        {"50","100","150","200","250"}};

int MenuPins[18]={50,2,3,4,5,6,7,13,14,15,16,17,18,19,24,25,26,27};
int MenuPins2[18]={0};
int MenuPins3[18]={0};
int MenuPins4[18]={0};
int menuItems[10]={5,14,7,7,3,3,4,5,5,5};


void setup() {  ///////////////////////////////////////////////////////////////////////////
  pinMode(0,INPUT_PULLUP);
  pinMode(14,INPUT_PULLUP);
  tft.init();
  sprite.createSprite(170,320);
  EEPROM.begin(EEPROM_SIZE);
     ledcSetup(0, 10000, 8);
     ledcAttachPin(38, 0);
     ledcWrite(0, 160);  
   readEprom();
   setupPins();
}

void readEprom()
{
for(int i=0;i<24;i++){
leftPinsType[i]=EEPROM.read(i);
if(leftPinsType[i]>5)
leftPinsType[i]=0;
}

for(int j=0;j<24;j++)
leftPinsSource[j]=EEPROM.read(j+24);
}

void writeEprom()
{
for(int i=0;i<24;i++){
    EEPROM.write(i, leftPinsType[i]);
    EEPROM.commit();
}


for(int j=0;j<24;j++){
    EEPROM.write(j+24, leftPinsSource[j]);
    EEPROM.commit();}
}

void detach(int pin)
{
  for(int i=0;i<24;i++)
  if(leftPinsSource[i]==pin)
  {leftPinsType[i]=0; leftPinsSource[i]=100;
  pinMode(pin,OUTPUT); digitalWrite(pin,0);}
}

void setupPins()  /////////////////////////////////////////////////////////////////////////
{
int pwmCha=1;
for(int i=0;i<24;i++)
{
  if(leftPinsType[i]==1 || leftPinsType[i]==2)
  pinMode(leftPinsStr[i].toInt(),INPUT_PULLUP);

  if(leftPinsType[i]==3)
  pinMode(leftPinsStr[i].toInt(),OUTPUT);

  if(leftPinsType[i]==5)
  {
    ledcSetup(pwmCha, 5000, 8);
    ledcAttachPin(leftPinsStr[i].toInt(), pwmCha); 
    pwmCha++;
  }
}
}
int cha=0;
void readPins()
{

for(int i=0;i<4;i++)
{
  if(tSource[i]!=0)
  ts[i]=leftPinsState[tSource[i]];
}

times[0][0]=ts[0]*tm[0]; 
times[0][1]=ts[1]*tm[1]; 
times[1][0]=ts[2]*tm[2]; 
times[1][1]=ts[3]*tm[3]; 


cha=1;
if(digitalRead(0)==0 && digitalRead(14)==0)
{if(debounce==0)
debounce=1; fase=!fase; delay(200);
}else debounce=0;

for(int i=0;i<26;i++)
{
  
  if(leftPinsType[i]==1)     // if pin is input pullup
  leftPinsState[i]=digitalRead(leftPinsStr[i].toInt());
  

  if(leftPinsType[i]==2)   // if pis is switch
  {
  if(digitalRead(leftPinsStr[i].toInt())==0)
  {if(deb[i]==0)
  {deb[i]=1; presed[i]=!presed[i]; leftPinsState[i]=presed[i]; }
  }else deb[i]=0;
  }

  if(leftPinsType[i]==3 && leftPinsSource[i]!=100){
  if(leftPinsSource[i]>100 && leftPinsSource[i]<200)
  leftPinsState[i]=!leftPinsState[leftPinsSource[i]-100];
  if(leftPinsSource[i]<100)
  leftPinsState[i]=leftPinsState[leftPinsSource[i]];
  if(leftPinsSource[i]>=200)
  leftPinsState[i]=leftPinsSource[i]-200;
  digitalWrite(leftPinsStr[i].toInt(),leftPinsState[i]);
  }

  if(leftPinsType[i]==4)
  leftPinsState[i]=map(analogRead(leftPinsStr[i].toInt()),0,4095,0,255);

     if(leftPinsType[i]==5){
     if(leftPinsSource[i]>100)
     leftPinsState[i]=leftPinsSource[i]-100;
     else
     leftPinsState[i]=leftPinsState[leftPinsSource[i]];

     ledcWrite(cha, leftPinsState[i]);
     cha++;
    }
}}

void draw()
  {
    sprite.fillSprite(background); 
    sprite.fillSmoothRoundRect(4,20,78,28,4,0x2516,background);
    sprite.fillSmoothRoundRect(4,30,50,38,4,0x2516,background);
    sprite.fillSmoothRoundRect(5,21,76,26,4,background,0x2516);
    sprite.fillSmoothRoundRect(5,30,48,37,4,background,0x2516);
    sprite.fillSmoothCircle(40,55,9,0x2516);
    sprite.setTextDatum(4);
    
    
    sprite.setTextDatum(0);
    sprite.setTextColor(0x2516,background);
    
    sprite.drawString("ON "+String(times[0][0]),8,25);
    sprite.drawString("OFF "+String(times[0][1]),8,35);

    sprite.fillSmoothRoundRect(90,20,76,28,4,0x2516,background);
    sprite.fillSmoothRoundRect(118,30,48,38,4,0x2516,background);
    sprite.fillSmoothRoundRect(91,21,74,26,4,background,0x2516);
    sprite.fillSmoothRoundRect(119,30,46,37,4,background,0x2516);
    sprite.fillSmoothCircle(132,55,10,0x2516);
    sprite.setTextDatum(4);
    sprite.setTextColor(background,0x2516);

    sprite.fillSmoothRoundRect(4,212,50,57,4,TFT_PURPLE,background);
    sprite.setTextDatum(0);
    
    

    sprite.setTextDatum(0);
    sprite.setTextColor(0x2516,background);
    
    sprite.drawString("ON "+String(times[1][0]),94,25);
    sprite.drawString("OFF "+String(times[1][1]),94,35);

    sprite.setTextDatum(4);
    
    for(int i=0;i<5;i++)
    {
     sprite.fillSmoothRoundRect(4+i*32,4,30,12,2,typeColor[i],background);
     sprite.setTextColor(TFT_WHITE,typeColor[i]);
     sprite.drawString(typeStr[i],4+i*32+30/2,4+6); 
    }

    for(int i=0;i<24;i++){

     if(i<12)
     {x=fromLeft;y=fromTop+i*20;l=4;l2=x; c=46;s=30; a=s-8;}
     else
     {x=fromLeft+32;y=fromTop+(i-12)*20;l=154,l2=x+w;c=x+w+10; s=c+18; a=s-20;}
  
    sprite.fillSmoothRoundRect(x,y,w,h,2,leftPinsColor[i],background);
    sprite.setTextColor(TFT_WHITE,leftPinsColor[i]);
    sprite.drawString(leftPinsStr[i],x+w/2,y+h/2,2);

    
    if(leftPinsType[i]!=0)
    {
      sprite.drawLine(l,y+h/2,l2,y+h/2,stateColor[leftPinsState[i]]);
      sprite.drawLine(l,y+h/2+1,l2,y+h/2+1,stateColor[leftPinsState[i]]);
      sprite.fillSmoothRoundRect(l, y+2, w-12, h-4, 2, typeColor[leftPinsType[i]-1]);
      sprite.setTextColor(TFT_WHITE,typeColor[leftPinsType[i]-1]);
      sprite.drawString(typeStr[leftPinsType[i]-1].substring(0,1),l+6, y+3+((h-4)/2));

      if(leftPinsType[i]==4){ // if analog pin
      sprite.fillSmoothRoundRect(a, y+2, w+3, h-4, 4, typeColor[leftPinsType[i]-1]);
      sprite.setTextColor(TFT_WHITE,typeColor[leftPinsType[i]-1]);
      sprite.drawString(String(leftPinsState[i]),a+14, 1+y+h/2);
      }

      if(leftPinsType[i]==5){ // if PWM pin
      sprite.fillSmoothRoundRect(a, y+2, w+3, h-4, 4, typeColor[leftPinsType[i]-1]);
      sprite.setTextColor(TFT_WHITE,typeColor[leftPinsType[i]-1]);
      sprite.drawString(String(leftPinsState[i]),a+14, 1+y+h/2);
      }

      if(leftPinsType[i]==3){
      sprite.setTextColor(gra,background);
      if(leftPinsSource[i]>100)
      sprite.drawString("!"+String(leftPinsStr2[leftPinsSource[i]-100]),s,y+4);
      else
      sprite.drawString(String(leftPinsStr2[leftPinsSource[i]]),s,y+4);
    
      }
   
       if(leftPinsType[i]<4)
          {
            sprite.fillSmoothCircle(c, y+h/2,5, stateColor[leftPinsState[i]],background);
            sprite.setTextColor(TFT_WHITE,stateColor[leftPinsState[i]]);
            sprite.drawString(String(leftPinsState[i]),c+1, 1+y+h/2);
          }
     }
}

    sprite.fillSmoothRoundRect(4,294,46,22,4,typeColor[leftPinsType[24]-1]);
    sprite.fillSmoothRoundRect(6,296,24,18,2,gra);
    
    

        sprite.fillSmoothRoundRect(120,294,46,22,4,typeColor[leftPinsType[25]-1]);
        sprite.fillSmoothRoundRect(122,296,24,18,2,gra);
        sprite.setTextColor(TFT_WHITE,gra);
          
 
    sprite.setTextColor(TFT_BLACK,background);
    sprite.drawString("Press both",85,300);
    sprite.drawString("for MENU",85,310);

    sprite.loadFont(NotoSansBold15);
    sprite.setTextColor(0x02AE,background);
    sprite.drawString("PB1",18,288);
    sprite.drawString("PB2",152,288);
    sprite.drawString("T1",18,57);
    sprite.drawString("T2",155,57);
    sprite.setTextColor(background,0x2516);
    sprite.drawString(String(leftPinsState[26]),40,57,2);
    sprite.drawString(String(leftPinsState[27]),132,57,2);

    sprite.setTextColor(TFT_WHITE,gra);
    sprite.drawString(leftPinsStr[24],17,306);
    sprite.drawString(leftPinsStr[25],134,306);
    sprite.setTextColor(0x02AE,TFT_ORANGE);
    sprite.drawString(String(leftPinsState[24]),38,306);
    sprite.setTextColor(TFT_WHITE,typeColor[leftPinsType[25]-1]);
    sprite.drawString(String(leftPinsState[25]),155,306);

    sprite.setTextDatum(0);
    sprite.setTextColor(background,TFT_PURPLE);
    sprite.drawString("TIOS",8,218);
    sprite.unloadFont();
    
    sprite.drawString("made by",8,236);
    sprite.drawString("VOLOS",8,248,2);

    sprite.unloadFont();
    sprite.pushSprite(0,0);
  }

  void clearPins()///////////////////////////////////////////////////////////////////
  {
    for(int i=0;i<24;i++)
    {leftPinsType[i]=0;
    leftPinsSource[i]=100;
     if(pins[i]!=100)
     {
       pinMode(pins[i],OUTPUT);
       digitalWrite(pins[i],0);
     }
    }
  }

  void findInputs() //////////////////////////////////////////////////////////////////
  {
  int n=10;
  for(int i=0;i<24;i++)
  {
    if(leftPinsType[i]==1 || leftPinsType[i]==2)
    {
      MenuPins2[n]=i;
      firstMenu[3][n]=leftPinsStr[i];
      n++;
      MenuPins2[n]=i;
      firstMenu[3][n]="!"+leftPinsStr[i];
      n++;
    }
  } menuItems[3]=n;
  }

  void findAnalogs() ////////////////////////////////////////////////////////////////////////////
  {
    int m=3;
    menuItems[4]=3;
    for(int i=0;i<24;i++)
    if(leftPinsType[i]==4)
       {
       firstMenu[4][m]="PIN "+leftPinsStr[i];
       MenuPins3[m]=i;
       m++;
       }
    menuItems[4]=m;   
  }

    void findAnalogs2() ////////////////////////////////////////////////////////////////////////////
  {
    int m=5;
    menuItems[7]=m;
    for(int i=0;i<24;i++)
    if(leftPinsType[i]==4)
       {
       firstMenu[7][m]="PIN "+leftPinsStr[i];
       MenuPins4[m]=i;
       m++;
       }
    menuItems[7]=m;   
  }
 
 
  bool finded=0;
  int mx,my,ix,iy;
  int sTimer=3;
  int onOrOff=3;
  
  void setPins()  ///////////////////////////////////////////////////////////////////////////////
  {
     
     finded=0;
     sprite.fillSprite(TFT_BLACK);
     sprite.fillSmoothRoundRect(99, 24, 62, 244, 6, 0xEF5D,TFT_BLACK);
     sprite.fillSmoothRoundRect(4, 296, 40, 20, 4, nc,TFT_BLACK);
     sprite.fillSmoothRoundRect(126, 296, 40, 20, 4, nc,TFT_BLACK);
     sprite.drawLine(4,20,92,20,gra);
     sprite.fillRect(4,24,86,244,TFT_PURPLE);
     sprite.drawLine(4,270,92,270,gra);
     sprite.drawLine(92,20,92,270,gra);
     sprite.setTextDatum(4);

      for(int i=0;i<5;i++)
    {
     sprite.fillSmoothRoundRect(4+i*32,278,30,12,2,typeColor[i],TFT_BLACK);
     sprite.setTextColor(TFT_WHITE,typeColor[i]);
     sprite.drawString(typeStr[i],4+i*32+30/2,278+7); 
    }
     for(int i=0;i<24;i++){

     if(i<12)
     {x=102;y=28+i*20;l=4;l2=x; c=46;s=30; a=s-8;}
     else
     {x=102+30;y=28+(i-12)*20;l=154,l2=x+w;c=x+w+10; s=c+18; a=s-10;}
  
    if(leftPinsType[i]!=0)
    {
      sprite.fillSmoothRoundRect(x,y,w,h,2,typeColor[leftPinsType[i]-1],0xEF5D);
      sprite.setTextColor(TFT_WHITE,typeColor[leftPinsType[i]-1]);
      sprite.drawString(leftPinsStr[i],x+w/2,y+h/2,2);
    }
    else
    {
      sprite.fillSmoothRoundRect(x,y,w,h,2,leftPinsColor[i],0xEF5D);
      sprite.setTextColor(TFT_WHITE,leftPinsColor[i]);
      sprite.drawString(leftPinsStr[i],x+w/2,y+h/2,2);
    }
    if(leftPinsStr[i]==firstMenu[1][chosenOne]){
    sprite.drawRoundRect(x,y,w,h,2,TFT_RED);
    sprite.drawRoundRect(x-1,y-1,w+2,h+2,2,TFT_RED);
    }
    }


     

     sprite.setTextDatum(0);
     sprite.loadFont(NotoSansBold15);
     sprite.setTextColor(TFT_WHITE,TFT_BLACK);
     sprite.drawString(menuTitle[menu],4,4,2);
     sprite.drawString("SET",12,299,2);
     sprite.drawString("OK",135,299,2);
     sprite.unloadFont();
     
     sprite.setTextDatum(4);
     sprite.setTextColor(TFT_WHITE,TFT_BLACK);
     sprite.drawString("MADE BY",85,302);
     sprite.drawString("VOLOS",85,312);

       sprite.setTextDatum(0);
       sprite.setTextColor(0x7E1E,TFT_PURPLE);
       
       
       for(int i=0;i<menuItems[menu];i++)
       {
         if(i<16) {mx=14; my=24;} else {mx=62; my=24-240;}
         sprite.drawString(firstMenu[menu][i],mx,my+(i*15),2);

       }
        if(item<16) {ix=6; iy=32;} else {ix=54; iy=32-240;}
       sprite.fillCircle(ix,iy+(item*15),3,TFT_ORANGE);
     

     if(digitalRead(0)==0)
     {
     if(ldb==0)
     {
       ldb=1; item++; 
       if(item==menuItems[menu])
       item=0;
       if(menu==1 && item>0)
       chosenOne=item;
     }
     }else ldb=0;

    if(digitalRead(14)==0)
     {
     if(rdb==0)
     {
       rdb=1;
       if(menu==0 && item==0) //EXIT 
       { fase=0; writeEprom(); setupPins();}

       if(menu==0 && item==1) //clear all pins
       clearPins();
      
       if(menu==0 && item==2 && finded==0) //NEXT MENU
       {menu=1; item=0; finded=1;}

        if(menu==0 && item==4 && finded==0) //NEXT MENU
       {menu=9; item=0; finded=1;}
        
       if(menu==1 && item==0 && finded==0) //GO BACK
       {menu=0; item=0; finded=1;}

       if(menu==1 && item!=0 && finded==0)
       {menu=2; item=0; finded=1;}

       if(menu==2 && item==0 && finded==0) //GO BACK
       {menu=1; item=0; finded=1;}

         if(menu==2 && item==1 && finded==0) //dont use pin
       {detach(MenuPins[chosenOne]);menu=0; item=0; finded=1; leftPinsType[MenuPins[chosenOne]]=0; leftPinsSource[MenuPins[chosenOne]]=100; }

        if(menu==2 && item==2 && finded==0) //input pullup
       {detach(MenuPins[chosenOne]); menu=0; item=0; finded=1; leftPinsType[MenuPins[chosenOne]]=1; leftPinsSource[MenuPins[chosenOne]]=100; 
       pinMode(firstMenu[1][chosenOne].toInt(),INPUT_PULLUP);}

        if(menu==2 && item==3 && finded==0) //sw
       { detach(MenuPins[chosenOne]); leftPinsState[MenuPins[chosenOne]]=0; menu=0; item=0; finded=1; leftPinsType[MenuPins[chosenOne]]=2; leftPinsSource[MenuPins[chosenOne]]=100;
       pinMode(firstMenu[1][chosenOne].toInt(),INPUT_PULLUP); }

         if(menu==2 && item==5 && finded==0) //analog
       {detach(MenuPins[chosenOne]);menu=0; item=0; finded=1; leftPinsType[MenuPins[chosenOne]]=4; leftPinsSource[MenuPins[chosenOne]]=100; }

        if(menu==2 && item==4 && finded==0) //output
       {detach(MenuPins[chosenOne]);  findInputs(); menu=3; item=0; finded=1; leftPinsType[MenuPins[chosenOne]]=3; leftPinsSource[MenuPins[chosenOne]]=100;
       pinMode(firstMenu[1][chosenOne].toInt(),OUTPUT);  }

     

        // output menu items
       if(menu==3 && item==0 && finded==0) //GO BACK
       { menu=0; item=0; finded=1; leftPinsSource[MenuPins[chosenOne]]=201; }

       if(menu==3 && item==1 && finded==0) //GO BACK
       { menu=0; item=0; finded=1; leftPinsSource[MenuPins[chosenOne]]=200;  }

       if(menu==3 && item==2 && finded==0) //GO BACK
       { menu=0; item=0; finded=1; leftPinsSource[MenuPins[chosenOne]]=26;  }

       if(menu==3 && item==3 && finded==0) //GO BACK
       { menu=0; item=0; finded=1; leftPinsSource[MenuPins[chosenOne]]=126;  }

        if(menu==3 && item==4 && finded==0) //GO BACK
       { menu=0; item=0; finded=1; leftPinsSource[MenuPins[chosenOne]]=27;  }

        if(menu==3 && item==5 && finded==0) //GO BACK
       { menu=0; item=0; finded=1; leftPinsSource[MenuPins[chosenOne]]=127;  }

        if(menu==3 && item==6 && finded==0) //GO BACK
       { menu=0; item=0; finded=1; leftPinsSource[MenuPins[chosenOne]]=24;  }
       if(menu==3 && item==7 && finded==0) //GO BACK
       { menu=0; item=0; finded=1; leftPinsSource[MenuPins[chosenOne]]=124;  }

         if(menu==3 && item==8 && finded==0) //GO BACK
       { menu=0; item=0; finded=1; leftPinsSource[MenuPins[chosenOne]]=25;  }

        if(menu==3 && item==9 && finded==0) //GO BACK
       { menu=0; item=0; finded=1; leftPinsSource[MenuPins[chosenOne]]=125;  }

       
       //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
        if(menu==3 && item>9 && finded==0) //GO BACK
       { menu=0;  finded=1; 
       if(firstMenu[3][item].charAt(0)=='!')
       {leftPinsSource[MenuPins[chosenOne]]=MenuPins2[item]+100; item=0;}
       else
       {leftPinsSource[MenuPins[chosenOne]]=MenuPins2[item]; item=0;} }

      //:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
       //PWM items

          if(menu==2 && item==6 && finded==0) //output
       {detach(MenuPins[chosenOne]); findAnalogs(); menu=4; item=0; finded=1; leftPinsType[MenuPins[chosenOne]]=5; leftPinsSource[MenuPins[chosenOne]]=100;
       }

         if(menu==4 && item==0 && finded==0) //GO BACK
       { menu=0; item=0; finded=1; leftPinsSource[MenuPins[chosenOne]]=150;  }

         if(menu==4 && item==1 && finded==0) //GO BACK
       { menu=0; item=0; finded=1; leftPinsSource[MenuPins[chosenOne]]=200;  }

          if(menu==4 && item==2 && finded==0) //GO BACK
       { menu=0; item=0; finded=1; leftPinsSource[MenuPins[chosenOne]]=250;  }

        if(menu==4 && item>2 && finded==0) //GO BACK
       { menu=0;  finded=1; leftPinsSource[MenuPins[chosenOne]]=MenuPins3[item]; item=0;  }





      //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::.

        if(menu==0 && item==3 && finded==0) //SET TIMERS
        {menu=5; item=0; finded=1;}

         if(menu==5 && item==0 && finded==0) //GO BACK
         {menu=0; item=0; finded=1;}

          if(menu==5 && item==1 && finded==0) //GO BACK
         {menu=6; sTimer=0; menuTitle[6]="Set T1"; item=0; finded=1;}

            if(menu==5 && item==2 && finded==0) //GO BACK
         {menu=6; sTimer=1; menuTitle[6]="Set T2"; item=0; finded=1;}

         if(menu==6 && item==0 && finded==0) //GO BACK
         {menu=0; item=0; finded=1;}

         if(menu==6 && item==1 && finded==0) //GO BACK
         {findAnalogs2(); menu=7; menuTitle[7]="OFF TIME"; item=0; finded=1; onOrOff=0;} 
         
         if(menu==6 && item==2 && finded==0) //GO BACK
         {findAnalogs2(); menu=7; menuTitle[7]="OFF TIME"; item=0; finded=1; onOrOff=1;} 
         
         if(menu==6 && item==3 && finded==0) //GO BACK
         {menu=8; item=0; finded=1;} 

          if(menu==7 && finded==0 && item<5) //GO BACK
         {ts[(sTimer*2)+onOrOff]=firstMenu[7][item].toInt(); item=0; finded=1; menu=6;
         tSource[(sTimer*2)+onOrOff]=0;} 

         if(menu==7 && finded==0 && item>4) //GO BACK
         {tSource[(sTimer*2)+onOrOff]=MenuPins4[item]; item=0; finded=1; menu=6;} 

          if(menu==8 && finded==0) 
         {tm[sTimer*2]=firstMenu[8][item].toInt();
          tm[sTimer*2+1]=firstMenu[8][item].toInt(); 
          item=0; finded=1; menu=6;} 

          if(menu==9 && finded==0) 
          {ledcWrite(0, firstMenu[9][item].toInt()); item=0; menu=0;}
          

         //byte ts[4]={250,250,150,150}; 
  
        delay(250);

     }
     }else rdb=0;

     sprite.pushSprite(0,0);
  }

void loop() {

    if(leftPinsState[26]==0)
  if(millis()>currentTime[0]+times[0][1])
  {leftPinsState[26]=1;
  currentTime[0]=millis();} 

    if(leftPinsState[26]==1)
  if(millis()>currentTime[0]+times[0][0])
  {leftPinsState[26]=0;
  currentTime[0]=millis();} 

      if(leftPinsState[27]==0)
  if(millis()>currentTime[1]+times[1][1])
  {leftPinsState[27]=1;
  currentTime[1]=millis();} 

    if(leftPinsState[27]==1)
  if(millis()>currentTime[1]+times[1][0])
  {leftPinsState[27]=0;
  currentTime[1]=millis();} 


 
  if(fase==0)
  {draw(); readPins(); }
  if(fase==1)
  setPins();

}
