#include <NewPing.h> // library for ultrasonic distance sensor
#include <Servo.h> // library for servo motor

// instantiate ultrasonic distance sensors to digital pin 2, 3, and 10
NewPing ultra_sensor ( 3 , 2 , 10 );    

//instantiate servo motor
Servo servo_9;

// instantiate speed (digital pin 5) and direction of rotation (digital pin 4) of left-side tyres
int spd_A = 5;
int dir_A = 4;

// instantiate speed (digital pin 6) and direction of rotation (digital pin 7) of left-side tyres
int spd_B = 6;
int dir_B = 7;

// instantiate buzzer to digital pin 8
int buzzer = 8;

// instantiate left, middle, and right IR sensors (analog pin A3, A4, and A5) and variables to store the IR sensor value
int left_IRsensor = A4;
int left_sensor_value = 0;
int right_IRsensor = A3;
int right_sensor_value = 0;
int mid_IRsensor = A5;
int mid_sensor_value = 0;

// instantiate character variable to receive incoming bluetooth data
char data ;

// instantiate mode of operation (mode 0 = autonomous, mode 1 = remote control)
int mode = 0 ;

// instantiate type of motion including forward, backward, left, right and stop
int motion = 0 ;

int loc_count = 0; // counter to record number of stop points passed through
int loc_user = 0; // record user-desired stop point input

void setup() {
  //setting up all pinmodes as output
  pinMode(spd_A , OUTPUT);
  pinMode(dir_A , OUTPUT);
  pinMode(spd_B , OUTPUT);
  pinMode(buzzer, OUTPUT);
  pinMode(dir_B , OUTPUT);

  // attaching servo motor to digital pin 9
  servo_9.attach(9);

  // begin serial communication of Arduino UNO board
  Serial.begin(9600);
}

// function to move forward in autonomous mode
void forward() {
  digitalWrite(dir_A , 1);
  analogWrite (spd_A , 90);
  digitalWrite (dir_B , 0);
  analogWrite (spd_B , 90 );
}

// function to move backwards in autonomous mode
void back() {
  digitalWrite(dir_A , 0);
  analogWrite (spd_A , 90);
  digitalWrite (dir_B , 1);
  analogWrite (spd_B , 90 );
}

// function to turn left in autonomous mode
void left() {
  digitalWrite(dir_A , 0);
  analogWrite (spd_A , 0);
  digitalWrite (dir_B , 0);
  analogWrite (spd_B , 90 );
}

// function to turn right in autonomous mode
void right() {
  digitalWrite(dir_A , 1);
  analogWrite (spd_A , 90);
  digitalWrite (dir_B , 0);
  analogWrite (spd_B , 0 );
}

// function to move forward in remote control mode
void forward1() {
  digitalWrite(dir_A , 1);
  analogWrite (spd_A , 200);
  digitalWrite (dir_B , 0);
  analogWrite (spd_B , 200 );
}

// function to move backward in remote control mode
void back1() {
  digitalWrite(dir_A , 0);
  analogWrite (spd_A , 200);
  digitalWrite (dir_B , 1);
  analogWrite (spd_B , 200 );
}

// function to turn left in remote control mode
void left1() {
  digitalWrite(dir_A , 0);
  analogWrite (spd_A , 150);
  digitalWrite (dir_B , 0);
  analogWrite (spd_B , 150 );
}

// function to turn right in remote control mode
void right1() {
  digitalWrite(dir_A , 1);
  analogWrite (spd_A , 150);
  digitalWrite (dir_B , 1);
  analogWrite (spd_B , 150 );
}

// function to stop in remote control mode
void Stop() {
  digitalWrite(dir_A , 0);
  analogWrite (spd_A , 0);
  digitalWrite (dir_B , 0);
  analogWrite (spd_B , 0 );
}

void loop() {
  // instantiating distance variable to record distance in front of ultrasonic distance sensor
  // ultrasonic distance sensor is positioned in the front position of the car
  int distance = ultra_sensor.ping_cm();

  // reading analog values from IR sensors and storing into variables
  left_sensor_value = analogRead(left_IRsensor);
  right_sensor_value = analogRead(right_IRsensor);
  mid_sensor_value = analogRead(mid_IRsensor);

  // delay 0.2 seconds to make entire loop run slower, thus not overloading the physical components
  delay(200);

  if ( Serial.available()) { 
    // read incoming data through bluetooth connection
    data = Serial.read();

    // choose between remote control (M) or autonomous (A) mode
    if (data == 'M') {
      mode = 1 ;
    }
    if (data == 'A') {
      mode = 0;
    }

    // location for load drop-off point for autonomous mode
    if (data == 'X') {
      loc_user = 1;
    }
    if (data == 'Y') {
      loc_user = 2;
    }

    // instruction input for remote control mode
    if (data == 'F') {
      motion = 1;
    }
    if (data == 'B') {
      motion = 2;
    }
    if (data == 'R') {
      motion = 3 ;
    }
    if (data == 'L') {
      motion = 4 ;
    }
    if (data == 'S') {
      motion = 0 ;
    }
  }

  // AUTONOMOUS CONTROL CODE BLOCK //

  if ( mode == 0 && loc_user != 0 ) // car will only move if autonomous mode and drop-off location is chosen
  {
    if (distance == 0) { // if there is no object in front of the ultrasonic distance sensor
      digitalWrite(buzzer , 0); // the buzzer will be deactivated and motion is continued

      // IR sensor
      // value > 900 = white region
      // value < 900 = black region

      // for when IR detects all black region (stop point)
      if (right_sensor_value < 900 && mid_sensor_value < 900 && left_sensor_value < 900) {
        loc_count++; //location counter increase by 1
        
        // when reach desired point, car stops moving
        // object is dropped off by activating servo motor
        // then the car continues to move
        // the car will not stop if location counter does not match user desired drop-off point
        if (loc_count == loc_user) { 
          delay(500);
          Stop();
          servo_9.write(0);
          delay(2000);
          servo_9.write(90);
          forward();
        }

        // when counter reach 3, it indicates that car has returned to initial position
        // counter and user location is reset to zero
        if (loc_count == 3) { 
          forward();
          delay(1000);
          Stop();
          loc_count = 0;
          loc_user = 0;
        }

        // delay of 0.5s is introduced to prevent overlapping increment of location counter at stop point
        delay(500);        
      }

      // if only detect black line, go forward
      else if (mid_sensor_value < 900) {
        forward();
      }

      // if detect black line on the right, turn right
      else if (right_sensor_value < 900  && left_sensor_value > 900) {
        right();
        delay(10);

      }

      // if detect black line on the left, turn left
      else if (right_sensor_value > 900 && left_sensor_value < 900) {
        left();
        delay(10);
      }

      // if no black line is detected at all, meaning car is off track
      // since the car trajectory is clockwise, the car only has to keep turning right until it detects the track again
      else if (right_sensor_value > 900 && mid_sensor_value > 900 && left_sensor_value > 900) {
        right();
        delay(5);
      }
    }

    // if there is an obstacle, the car stops
    // the buzzer is activated
    // the buzzer will only deactivate and car continue motion if the obstacle is removed
    else if (distance != 0) {
      Stop();
      digitalWrite(buzzer , 1);
    }
  }

  // REMOTE CONTROL CODE BLOCK //

  if (mode == 1) { // mode 1 is remote control mode
    // data input from bluetooth above is executed in this block
    if (motion == 0) {
      Stop();
    }
    else if (motion == 1) {
      forward1();
    }
    else if (motion == 2) {
      back1();
    }
    else if (motion == 3) {
      right1();
    }
    else if (motion == 4) {
      left1();
    }
  }
}
