//int OutPin = A0;
//int OutValue = 128;
//
//void setup() {
//Serial.begin(115200);
//pinMode(D5, INPUT_PULLUP);
//pinMode(D6, INPUT_PULLUP);
//analogWrite(OutPin, OutValue);
//}
//
//void loop() {
//
//if (!(digitalRead(D5)))
//{
//if(OutValue<255) OutValue +=1;
//}
//if (!(digitalRead(D6)))
//{
//if(OutValue>0) OutValue -=1;
//}
//
//analogWrite(OutPin, OutValue);
//delay(1);
//Serial.println(OutValue);
//
//}


int OutPin = A0;
int OutValue = 128;

//объявляем переменные с номерами пинов
int redPin = D0;
int greenPin = D1;
int bluePin = D2;


void setup() {
  // Объявляем работу с последоватлеьным портом в самом начале
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  // put your setup code here, to run once:

  // устанваливаем пин в режим выхода
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);
  
  pinMode(D5, INPUT_PULLUP); //INPUT_PULLUP
  pinMode(D6, INPUT_PULLUP);
  analogWrite(OutPin, OutValue);
//  digitalWrite(OutPin, OutValue);
  delay(10);

}

void loop() {      
  // put your main code here, to run repeatedly:
  if (!(digitalRead(D5)))
  {
    if (OutValue < 255) OutValue += 1;
  }
  if (!(digitalRead(D6)))
  {
    if (OutValue > 0) OutValue -= 1;
  }

  if (OutValue == 128) {
//    setColor(255, 0, 255);
    digitalWrite(redPin, HIGH);
    digitalWrite(greenPin, LOW);
    digitalWrite(bluePin, HIGH);
    
    }
  if (OutValue > 128) {
//    setColor(255, 255, 0);
    digitalWrite(redPin, HIGH);
    digitalWrite(greenPin, HIGH);
    digitalWrite(bluePin, LOW);
    }
  if (OutValue < 128) {
//    setColor(0, 255, 255);
    digitalWrite(redPin, LOW);
    digitalWrite(greenPin, HIGH);
    digitalWrite(bluePin, HIGH);
    }

    
  analogWrite(OutPin, OutValue);
//  digitalWrite(OutPin, OutValue);
  Serial.print("OutValue: ");
  Serial.println(OutValue);
  delay(500);
}

void setColor(int red, int green, int blue)

{

#ifdef COMMON_ANODE

red = 255 - red;

green = 255 - green;

blue = 255 - blue;

#endif

analogWrite(redPin, red);

analogWrite(greenPin, green);

analogWrite(bluePin, blue);

}

//int OutPin = A0;
//int OutValue = 128;
//void setup() {
//Serial.begin(115200);
//Serial.setDebugOutput(true);
////TCCR1A = TCCR1A & 0xe0 | 1;
////TCCR1B = TCCR1B & 0xe0 | 0x09;
//pinMode(D5, INPUT_PULLUP);
//pinMode(D6, INPUT_PULLUP);
//analogWrite(OutPin, OutValue);
//}
//void loop() {
//if (!(digitalRead(D5)))
//{if(OutValue<255) OutValue +=1;delay(2);}
//if (!(digitalRead(D6)))
//{if(OutValue>0) OutValue -=1;delay(2);}
//analogWrite(OutPin, OutValue);
//  Serial.print("OutValue: ");
//  Serial.println(OutValue);
//  delay(500);
//}
