#include <Bluepad32.h>
#include <uni.h>


// ESP32 GPIO Pinout
#define in1 0  // Front left (Wheel 1)
#define in2 4
#define in3 15  // Front right (Wheel 2)
#define in4 2
#define in5 18  // Rear left (Wheel 3)
#define in6 5
#define in7 17  // Rear right (Wheel 4)
#define in8 16

// Controller Address - Change based on your controller
// static const char* controller_addr_string = "EC:83:50:7F:CE:8C"; // Maxx's XboxOne 1708
static const char* controller_addr_string = "44:16:22:AF:11:EF";  // Maxx's XboxOne 1709
// Address for my controller, replace with your own. You can get the address
// by checking serial coms at 115200 b with Arduino IDE Serial Monitor

// Robot Variables
float direction = 0;
int speed = 0;
int rotation = 0;

// Controller Variables
float LX = 0;
float LY = 0;
float RightX = 0;
int Throttle = 0;
int Brake = 0;
float deadspace = 80;

// Motor power variables
int MaxPWM = 254;       // Maximum safe motor power PWM value
int PWMThresh = 140;    // Free spinning stall PWM of motor
int CruiseSpeed = 197;  // Base movement speed of robot as PWM value
int LFPower = 0;        // Tracking variables for PWM at motor leads
int RFPower = 0;
int LRPower = 0;
int RRPower = 0;

// Objects
ControllerPtr myControllers[BP32_MAX_GAMEPADS];

// This callback gets called any time a new gamepad is connected.
// Up to 4 gamepads can be connected at the same time.
void onConnectedController(ControllerPtr ctl) {
  bool foundEmptySlot = false;
  for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
    if (myControllers[i] == nullptr) {
      Serial.printf("CALLBACK: Controller is connected, index=%d\n", i);
      // Additionally, you can get certain gamepad properties like:
      // Model, VID, PID, BTAddr, flags, etc.
      ControllerProperties properties = ctl->getProperties();
      Serial.printf("Controller model: %s, VID=0x%04x, PID=0x%04x\n", ctl->getModelName().c_str(), properties.vendor_id,
                    properties.product_id);
      myControllers[i] = ctl;
      foundEmptySlot = true;
      break;
    }
  }
  if (!foundEmptySlot) {
    Serial.println("CALLBACK: Controller connected, but could not found empty slot");
  }
}

void onDisconnectedController(ControllerPtr ctl) {
  bool foundController = false;

  speed = 0;  //
  rotation = 0;
  direction = 0;
  LFPower = 0;
  RRPower = 0;
  LRPower = 0;
  RFPower = 0;

  analogWrite(in1, 0);  // Tuan all motors off too
  analogWrite(in2, 0);
  analogWrite(in3, 0);
  analogWrite(in4, 0);
  analogWrite(in5, 0);
  analogWrite(in6, 0);
  analogWrite(in7, 0);
  analogWrite(in8, 0);

  for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
    if (myControllers[i] == ctl) {
      Serial.printf("CALLBACK: Controller disconnected from index=%d\n", i);
      myControllers[i] = nullptr;
      foundController = true;
      break;
    }
  }

  if (!foundController) {
    Serial.println("CALLBACK: Controller disconnected, but not found in myControllers");
  }
}

void dumpGamepad(ControllerPtr ctl) {
  Serial.printf(
    "idx=%d, dpad: 0x%02x, buttons: 0x%04x, axis L: %4d, %4d, axis R: %4d, %4d, brake: %4d, throttle: %4d, "
    "misc: 0x%02x\n",
    ctl->index(),       // Controller Index
    ctl->dpad(),        // D-pad
    ctl->buttons(),     // bitmask of pressed buttons
    ctl->axisX(),       // (-511 - 512) left X Axis
    ctl->axisY(),       // (-511 - 512) left Y axis
    ctl->axisRX(),      // (-511 - 512) right X axis
    ctl->axisRY(),      // (-511 - 512) right Y axis
    ctl->brake(),       // (0 - 1023): brake trigger
    ctl->throttle(),    // (0 - 1023): throttle trigger
    ctl->miscButtons()  // bitmask of pressed "misc" buttons
  );
}

void processGamepad(ControllerPtr ctl) {
  // Planning movement...
  LX = ctl->axisX();  // Measure state of gamepad
  LY = ctl->axisY();
  RightX = ctl->axisRX();
  Throttle = int(ctl->throttle());
  Brake = int(ctl->brake());

  // Get speed from user by checking throttle & brake triggers
  speed = CruiseSpeed + int((MaxPWM - CruiseSpeed) * Throttle / (1023))
          - int((CruiseSpeed - PWMThresh) * Brake / (1023));

  if (abs(speed) > MaxPWM) {  // Scale speed to a safe value
    speed = speed * MaxPWM / abs(speed);
  }

  if (abs(RightX) < 1.5 * deadspace) { 
    rotation = 0;
  } else {
    rotation = int(speed * RightX / abs(RightX));
  }

  if (abs(LX) < deadspace) {  // Enforce controller deadzones
    LX = 0;
  }
  if (abs(LY) < deadspace) {
    LY = 0;
  }
  if (LX == 0 && LY == 0) {
    speed = 0;
  }

  if (abs(LX) > 1 || abs(LY) > 1 || abs(rotation) > 1) {  // Plan movement
    direction = atan2(-LY, LX);                           // [rad] float

    // Plan movement
    // The variable direction is an angle in radians, with the direction
    // "Straight forward" located at pi/2 radians = 1.5708 rad

    // This can be the most intense and annoying section to debug ever, but
    // It is often the source of trouble on 1st time setup FIXME
    if (0.393 <= direction && direction < 1.178) {  // move forward and right
      RFPower = 0 - rotation;
      LFPower = speed + rotation;
      LRPower = 0 - rotation;
      RRPower = speed - rotation;
    } else if (1.178 <= direction && direction < 1.963) {  // move forward
      RFPower = speed - rotation;
      LFPower = speed + rotation;
      LRPower = -speed - rotation;
      RRPower = speed - rotation;
    } else if (1.963 <= direction && direction < 2.749) {  // move forward and left
      RFPower = speed - rotation;
      LFPower = 0 + rotation;
      LRPower = -speed - rotation;
      RRPower = 0 - rotation;
    } else if (2.749 <= direction || direction < -2.749) {  // move left
      RFPower = speed - rotation;
      LFPower = -speed + rotation;
      LRPower = -speed - rotation;
      RRPower = -speed - rotation;
    } else if (-2.749 <= direction && direction < -1.963) {  //move backwards and left
      RFPower = 0 - rotation;
      LFPower = -speed + rotation;
      LRPower = 0 - rotation;
      RRPower = -speed - rotation;
    } else if (-1.963 <= direction && direction < -1.178) {  // move backwards
      RFPower = -speed - rotation;
      LFPower = -speed + rotation;
      LRPower = speed - rotation;
      RRPower = -speed - rotation;
    } else if (-1.178 <= direction && direction < -0.393) {  //move backwards and right}
      RFPower = -speed - rotation;
      LFPower = 0 + rotation;
      LRPower = speed - rotation;
      RRPower = 0 - rotation;
    } else {                                                  // move right
      RFPower = -speed - rotation;
      LFPower = speed + rotation;
      LRPower = speed - rotation;
      RRPower = speed - rotation;
    }
  } else {  // stay still if no movement commands issued
    speed = 0;
    rotation = 0;
    direction = 0;
    LFPower = 0;
    RRPower = 0;
    LRPower = 0;
    RFPower = 0;
  }

  // Constrain all pwm values to acceptable values
  if (0 < abs(LFPower) && abs(LFPower) < PWMThresh) {
    LFPower = 0;
  } else if (LFPower > MaxPWM) {
    LFPower = MaxPWM;
  } else if (LFPower < -MaxPWM) {
    LFPower = -MaxPWM;
  }

  if (0 < abs(RFPower) && abs(RFPower) < PWMThresh) {
    RFPower = 0;
  } else if (RFPower > MaxPWM) {
    RFPower = MaxPWM;
  } else if (RFPower < -MaxPWM) {
    RFPower = -MaxPWM;
  }

  if (0 < abs(LRPower) && abs(LRPower) < PWMThresh) {
    LRPower = 0;
  } else if (LRPower > MaxPWM) {
    LRPower = MaxPWM;
  } else if (LRPower < -MaxPWM) {
    LRPower = -MaxPWM;
  }

  if (0 < abs(RRPower) && abs(RRPower) < PWMThresh) {
    RRPower = 0;
  } else if (RRPower > MaxPWM) {
    RRPower = MaxPWM;
  } else if (RRPower < -MaxPWM) {
    RRPower = -MaxPWM;
  }

  // Debugging statements below. Outputs robot variables and controller variables
  Serial.printf("Dir: %4f, Spd: %3i, Rot: %3i\n", direction, speed, rotation);
  // Serial.printf("Spd: %f, rot: %f, dir: %4f, LF: %3i, RF: %3i, LR: %3i, RR: %3i\n", speed, rotation, direction, LFPower, RFPower, LRPower, RRPower);
  // dumpGamepad(ctl);

  // time since last controller update - xx seconds
}


void processControllers() {
  for (auto myController : myControllers) {
    if (myController && myController->isConnected() && myController->hasData()) {
      if (myController->isGamepad()) {
        processGamepad(myController);  // Plan movement...
      } else {
        Serial.println("Unsupported controller");
      }
    }
  }
}


void debugShuffle() {
  // Good tool for understanding which motors are connected to which ports. 
  // This is a critical part of the initial setup process for programmers. 
  // This function is called in line 408 @ void setup()

  Serial.println("Motors spinning in + direction");
  // Spin all motors in the `positive` direction
  analogWrite(in1, 0);
  analogWrite(in2, 255);
  analogWrite(in3, 0);
  analogWrite(in4, 255);
  analogWrite(in5, 0);
  analogWrite(in6, 255);
  analogWrite(in7, 0);
  analogWrite(in8, 255);

  delay(2000); // 2 seconds of powered motion
  
  Serial.println("Motors OFF");
  // Turn every thing off 
  analogWrite(in1, 0);
  analogWrite(in2, 0);
  analogWrite(in3, 0);
  analogWrite(in4, 0);
  analogWrite(in5, 0);
  analogWrite(in6, 0);
  analogWrite(in7, 0); 
  analogWrite(in8, 0); 

  delay(2000); // 2 seconds of no motion

  Serial.println("Motors spinning in the - direction");
  // Spin everything in the `negative` direction
  analogWrite(in1, 255);
  analogWrite(in2, 0);
  analogWrite(in3, 255);
  analogWrite(in4, 0);
  analogWrite(in5, 255);
  analogWrite(in6, 0);
  analogWrite(in7, 255);
  analogWrite(in8, 0);

  delay(2000); // 2 seconds of powered motion

  Serial.println("Motor 1 ONLY spinning in the + direction OFF");
  // Turn everything but motor 1 off
  analogWrite(in1, 0);
  analogWrite(in2, 255);
  analogWrite(in3, 0);
  analogWrite(in4, 0);
  analogWrite(in5, 0);
  analogWrite(in6, 0);
  analogWrite(in7, 0); 
  analogWrite(in8, 0); 

  delay(2000); // 2 seconds of powered motion

  Serial.println("Motor 2 ONLY spinning in the + direction OFF");
  // Turn everything but motor 1 off
  analogWrite(in1, 0);
  analogWrite(in2, 0);
  analogWrite(in3, 0);
  analogWrite(in4, 255);
  analogWrite(in5, 0);
  analogWrite(in6, 0);
  analogWrite(in7, 0); 
  analogWrite(in8, 0); 

  delay(2000); // 2 seconds of power motion

  Serial.println("Motor 3 ONLY spinning in the + direction OFF");
  // Turn everything but motor 3 off
  analogWrite(in1, 0);
  analogWrite(in2, 0);
  analogWrite(in3, 0);
  analogWrite(in4, 0);
  analogWrite(in5, 0);
  analogWrite(in6, 255);
  analogWrite(in7, 0); 
  analogWrite(in8, 0); 

  delay(2000); // 2 seconds of power motion

  Serial.println("Motor 4 ONLY spinning in the + direction OFF");
  // Turn everything but motor 4 off
  analogWrite(in1, 0);
  analogWrite(in2, 0);
  analogWrite(in3, 0);
  analogWrite(in4, 0);
  analogWrite(in5, 0);
  analogWrite(in6, 0);
  analogWrite(in7, 0); 
  analogWrite(in8, 255); 

  delay(2000);  // 2 seconds of powered motion

  // stop all powered motion
  analogWrite(in1, 0);
  analogWrite(in2, 0);
  analogWrite(in3, 0);
  analogWrite(in4, 0);
  analogWrite(in5, 0);
  analogWrite(in6, 0);
  analogWrite(in7, 0);
  analogWrite(in8, 0);

  delay(150); // Brief 150 ms delay 
}

// Arduino setup function. Runs in CPU 1
void setup() {
  Serial.begin(115200);  // Begin serial communication for debugging.
  Serial.printf("Firmware: %s\n", BP32.firmwareVersion());

  // This line prints the address of the connected controller.
  const uint8_t* addr = BP32.localBdAddress();
  Serial.printf("BD Addr: %2X:%2X:%2X:%2X:%2X:%2X\n", addr[0], addr[1],
                addr[2], addr[3], addr[4], addr[5]);

  // Only connect to the specified controller in controller_addr.
  Serial.println("Checking allowlist...");
  bd_addr_t controller_addr;
  sscanf_bd_addr(controller_addr_string, controller_addr);
  uni_bt_allowlist_add_addr(controller_addr);
  Serial.printf("Added to allowlist: %s\n", controller_addr_string);
  uni_bt_allowlist_set_enabled(true);
  Serial.printf("Allowlist enabled: %d\n", uni_bt_allowlist_is_enabled());

  // Setup the Bluepad32 callbacks
  BP32.setup(&onConnectedController, &onDisconnectedController);
  BP32.forgetBluetoothKeys();
  BP32.enableVirtualDevice(false);

  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
  pinMode(in3, OUTPUT);
  pinMode(in4, OUTPUT);
  pinMode(in5, OUTPUT);
  pinMode(in6, OUTPUT);
  pinMode(in7, OUTPUT);
  pinMode(in8, OUTPUT);

  debugShuffle();
}


// Arduino loop function. Runs in CPU 1.
void loop() {
  // Perform safety checks here, validate crucial assumptions

  // Accept Input from the user's connected gamepad
  bool dataUpdated = BP32.update();  // fetch controller data FIXME

  if (dataUpdated) {                 // Check for user input via gamepad
    processControllers();            // Robot motion programmed here

    // Execute planned movement...
    if (RFPower > PWMThresh) {  // Front right
      analogWrite(in1, abs(RFPower));
      analogWrite(in2, 0);
    } else if (RFPower < -PWMThresh) {
      analogWrite(in1, 0);
      analogWrite(in2, abs(RFPower));
    } else {
      analogWrite(in1, 0);
      analogWrite(in2, 0);
    }

    if (LFPower > PWMThresh) {  // Front Left
      analogWrite(in3, abs(LFPower));
      analogWrite(in4, 0);
    } else if (LFPower < -PWMThresh) {
      analogWrite(in3, 0);
      analogWrite(in4, abs(LFPower));
    } else {
      analogWrite(in3, 0);
      analogWrite(in4, 0);
    }

    if (LRPower > PWMThresh) {  // Rear left
      analogWrite(in5, abs(LRPower));
      analogWrite(in6, 0);
    } else if (LRPower < -PWMThresh) {
      analogWrite(in5, 0);
      analogWrite(in6, abs(LRPower));
    } else {
      analogWrite(in5, 0);
      analogWrite(in6, 0);
    }

    if (RRPower > PWMThresh) {  //Rear right
      analogWrite(in7, abs(RRPower));
      analogWrite(in8, 0);
    } else if (RRPower < -PWMThresh) {
      analogWrite(in7, 0);
      analogWrite(in8, abs(RRPower));
    } else {
      analogWrite(in7, 0);
      analogWrite(in8, 0);
    }


  } /*else {  // If controller isn't updated and connected, stop moving FIXME
    speed = 0;
    rotation = 0;
    direction = 0;
    LFPower = 0;
    RRPower = 0;
    LRPower = 0;
    RFPower = 0;

    // Tuan all motors off too
    analogWrite(in1, 0);
    analogWrite(in2, 0);
    analogWrite(in3, 0);
    analogWrite(in4, 0);
    analogWrite(in5, 0);
    analogWrite(in6, 0);
    analogWrite(in7, 0);
    analogWrite(in8, 0);
  }*/

  // If your robot does other things you can put them here!

  vTaskDelay(1);
  delay(15);
}
