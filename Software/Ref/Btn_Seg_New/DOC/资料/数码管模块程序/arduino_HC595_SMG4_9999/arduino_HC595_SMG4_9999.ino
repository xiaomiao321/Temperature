int latchPin=12; //ST CP of 74HC595//load
int clockPin=13;//SH CP of 74HC595//sclk
int dataPin=11; //DS of 74HC595//sdi
unsigned char smgduan[] = {  //共阳
  B11000000, // 0
  B11111001, // 1
  B10100100, // 2
  B10110000, // 3
  B10011001, // 3
  B10010010, // 8
  B10000010, // 6
  B11111000, // 7
  B10000000, // 8
  B10010000, // 9
  };//显示0~9的值共阴
//unsigned char duanMa[] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80}; //选择1-8哪个数码管段
void setup() {
      pinMode(latchPin, OUTPUT);
      pinMode(clockPin, OUTPUT);
      pinMode(dataPin, OUTPUT); 
}
void loop()
{
   for(int i=0; i<9999; i++)
   {
    unsigned char gewei = (i%100)%10;
    unsigned char shiwei = (i%100)/10;
    unsigned char baiwei = (i%1000)/100; 
    unsigned char qianwei = i/1000; 
      digitalWrite(latchPin,LOW); //低电位表示启动
      shiftOut(dataPin,clockPin,MSBFIRST,smgduan[gewei]);
      shiftOut(dataPin,clockPin,MSBFIRST,smgduan[shiwei]);
      shiftOut(dataPin,clockPin,MSBFIRST,smgduan[baiwei]);
      shiftOut(dataPin,clockPin,MSBFIRST,smgduan[qianwei]);
      digitalWrite(latchPin,HIGH); //高电位表示停止
      delayMicroseconds(2);
      digitalWrite(latchPin, HIGH);//ST_CP
      delay(100);
   }

}
