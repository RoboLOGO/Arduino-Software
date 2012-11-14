/*
 * Lehet, hogy nem jó irányba fog mozogni, mivel nem tudtam motorkkal tesztelni. Ha nem jó irányba forog a goforward,gobackward,goleft,goright,penup,pendown metódusokban lehet változtatni a LOW HIGH-ra való átírásával (ahol kell). Az if(how_much > 0) ágán a jó irány, az else ágán az ellentétes irány adható meg (egymás ellentétei mindig a két ágban).
 * 7szám1\n szám2\n utasítással (7=DIM) a kerék kerületét (szám1) és a két kerék távolságát (szám2) lehet megadni
 * 8szám\n utasítással (8=FULL) a kerék teljes átfordulásához szükséges lépésszámot lehet megadni. Alap: 200.
 * PORT 5: bal motor
 * PORT 6: jobb motor
 * PORT 7: toll motor
 * PORT 8: bal motor iránya
 * PORT 9: jobb motor iránya
 * PORT 10: toll motor iránya
 */
#include <Servo.h>

Servo myservo;

const char* commandName[] = { "stop", "forward", "backward", "left", "right", "penup", "pendown", "dim", "full" };

const int left_enable = 10; 
const int left_pin = 9;
const int left_dir = 8;

const int right_enable = 7;
const int right_pin = 6;
const int right_dir = 5;

const int pen_pin = 4;

enum Cmds {
  STOP, FORWARD, BACKWARD, LEFT, RIGHT, PENUP, PENDOWN, DIM, FULL
};
		
float perimeter = 207.3451;		// kerék kerület
float distance = 190.0;			// kerék távolság
int full_circle = 800;          // egy kerékátforduláshoz szükséges ismétlés
float inc_l, inc_a;
char valueBuffer[32];
byte b = 0;
Cmds command = STOP;
boolean commandset = false;
boolean value1set = false;
boolean value2set = false;
float value1 = 0;
float value2 = 0;
boolean pen_on = true;

void setup() {
  pinMode(left_enable, OUTPUT);
  pinMode(right_enable, OUTPUT);
  
  pinMode(left_dir, OUTPUT);
  pinMode(right_dir, OUTPUT);
  
  pinMode(left_pin, OUTPUT);
  pinMode(right_pin, OUTPUT);
  pinMode(pen_pin, OUTPUT);
  
  digitalWrite(left_enable, HIGH);
  digitalWrite(right_enable, HIGH);
  myservo.attach(pen_pin);
  
  for(int i = 0; i < sizeof(valueBuffer); i++) valueBuffer[i] = 0; 
  b = 0;
  command = STOP;
  commandset = false;
  value1set = false;
  value2set = false;
  inc_l = perimeter / full_circle;
  inc_a = 180 * inc_l / 3.141592 / distance * 2;
  pen(true);
  Serial.begin(9600);
}

void loop() {

}

void motor(int cycle) {
  if(cycle != 0) {
    digitalWrite(left_enable, LOW);
    digitalWrite(right_enable, LOW);
    delay(10);
    while(cycle > 0) {
      digitalWrite(left_pin, HIGH);
      digitalWrite(right_pin, HIGH);
      delay(1);
      digitalWrite(left_pin, LOW);
      digitalWrite(right_pin, LOW);
      delay(1);
      cycle--;
    }    
    delay(10);
    digitalWrite(left_enable, HIGH);
    digitalWrite(right_enable, HIGH);
  }
  write("ok;"); 
}

void goforward(float how_much) {
  //direction
  digitalWrite(left_dir, HIGH);
  digitalWrite(right_dir, LOW);
  //calc steps
  how_much = abs(how_much);
  int cycle = (int)(how_much / inc_l);
  //run motor
  motor(cycle);
}

void gobackward(float how_much) {
  //direction
  digitalWrite(left_dir, LOW);
  digitalWrite(right_dir, HIGH);
  //calc steps
  how_much = abs(how_much);
  int cycle = (int)(how_much / inc_l);
  //run motor
  motor(cycle);
}
	
void goleft(float how_much) {
  //direction
  digitalWrite(left_dir, HIGH);
  digitalWrite(right_dir, HIGH);
  //calc steps
  how_much = abs(how_much);
  int cycle = (int)(how_much / inc_a);
  //run motor
  motor(cycle);
}

void goright(float how_much) {
  //direction
  digitalWrite(left_dir, LOW);
  digitalWrite(right_dir, LOW);
  //calc steps
  how_much = abs(how_much);
  int cycle = (int)(how_much / inc_a);
  //run motor
  motor(cycle);
}

void pen(boolean on) {
  if(on && !pen_on) {
    pen_on = true;
    myservo.write(80);
  } else if(!on && pen_on) {
    pen_on = false;
    myservo.write(55);
  } else {
    write("error;id=206;description=\"Pen is in the desired position\"");
    return;
  }
  
  write("ok;");
}

void dim(float wheel, float dist) {
  perimeter = wheel;
  distance = dist;
  inc_l = perimeter / full_circle;
  inc_a = 180 * inc_l / 3.141592 / distance * 2;
  Serial.print("mm_step=");Serial.print(inc_l, DEC);Serial.print(";deg_step=");Serial.println(inc_a, DEC);
}

void full(float cycle) {
  full_circle = (int)cycle;
  Serial.print("full_circle=");Serial.println(full_circle, DEC);
  dim(perimeter, distance);
}

void runcmd() {
  boolean value1notseterror = false;
  boolean value2notseterror = false;
  if(commandset) {
    switch(command) {
    case STOP:
      write("ready;");
      break;
    case FORWARD:
      if(value1set) goforward(value1);
      else value1notseterror = true;
      break;
    case BACKWARD:
      if(value1set) gobackward(value1);
      else value1notseterror = true;
      break;
    case LEFT:
      if(value1set) goleft(value1);
      else value1notseterror = true;
      break;
    case RIGHT:
      if(value1set) goright(value1);
      else value1notseterror = true;
      break;
    case PENUP:
      pen(false);
      break;
    case PENDOWN:
      pen(true);
      break;
    case DIM:
      if(value1set && value2set) dim(value1, value2);
      else value2notseterror = true;
      break;
    case FULL:
      if(value1set) full(value1);
      else value1notseterror = true;
      break;
    }
    command = STOP;
    value1 = 0;
    value2 = 0;
    commandset = false;
    value1set = false;
    value2set = false;
  } else write("error;id=203;decription=\"No command set\"");
  if(value1notseterror) write("error;id=204;description=\"First value not set\"");
  else if(value2notseterror) write("error;id=205;description=\"Second value not set\"");
}

void serialEvent() {
  while (Serial.available() != 0) {
    char inputChar = (char) Serial.read(); 
    if(!commandset) {
      commandset = true;
      switch(inputChar) {
      case '0':
        command = STOP;
        runcmd();
	break;
      case '1':
        command = FORWARD;
	break;
      case '2':
	command = BACKWARD;
	break;
      case '3':
	command = LEFT;
	break;
      case '4':
	command = RIGHT;
	break;
      case '5':
	command = PENUP;
	runcmd();
	break;
      case '6':
	command = PENDOWN;
	runcmd();
	break;
      case '7':
        command = DIM;
        break;
      case '8':
        command = FULL;
        break;
      case '\n':
        commandset = false;
        break;
      default:
	commandset = false;
	write("error;id=201;description=\"Unknown command\"");
	break;
      }
    } else {
      if(inputChar == '\r' || inputChar == '\n' || b == 31) {
        valueBuffer[b] = 0;
        float v = atof(valueBuffer);
        if(v == NAN) write(strcat(strcat("error;id=202;decription=\"Invalid value for command `", commandName[(int)command]), "'\""));
        else {
          if(!value1set) {
	    value1 = v;
	    value1set = true;
	    switch(command) {
	    case FORWARD:
	    case BACKWARD:
  	    case LEFT:
  	    case RIGHT:
            case FULL:
              runcmd();
              break;
	    }
          } else if(!value2set) {
            value2 = v;
            value2set = true;
            if(command == DIM) runcmd();
          }
        }
        b = 0;
      } else {
        valueBuffer[b++] = inputChar;
      }					
    }			
  }
}

void write(String text) {
  Serial.println(text);
}



