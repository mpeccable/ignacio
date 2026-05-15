#include <Bluepad32.h>
#include <uni.h>

// ESP32 GPIO Pinout
#define in3 4  // Front left (Wheel 1)
#define in4 0
#define in1 2  // Front right (Wheel 2)
#define in2 15
#define in5 18  // Rear left (Wheel 3)
#define in6 5
#define in7 17  // Rear right (Wheel 4)
#define in8 16

// Controller Address - Change based on your controller
static const char* controller_addr_string = "44:16:22:AF:11:EF"; 
// Address for my controller, replace with your own. You can get the address
// by enabling the debugging in line 276 & checking serial coms at 115200 b

// Robot Variables
float speed = 0;
float direction = 0; 
float rotation = 0;

// Controller Variables
float LX = 0;
float LY = 0;
float RightX = 0;
float deadspace = 50;

// Motor power variables
int PWMThresh = 128;
int LFPower = 0;
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
    ctl->brake(),       // (0 - 1023): brake button
    ctl->throttle(),    // (0 - 1023): throttle (AKA gas) button
    ctl->miscButtons()  // bitmask of pressed "misc" buttons
  );
}

void processGamepad(ControllerPtr ctl) {
  // Planning movement...
  LX = ctl->axisX();  // Measure state of gamepad
  LY = ctl->axisY();
  RightX = ctl->axisRX();

  if (abs(LX) < deadspace) {    // Enforce controller deadzones
    LX = 0;
  }
  if (abs(LY) < deadspace) {
    LY = 0;
  }
  if (abs(RightX) < 2 * deadspace) {
    RightX = 0;
    rotation = 0;
  } else {
    rotation = (RightX / abs(RightX)) * 255;
  }

  if (LX != 0 || LY != 0 || RightX != 0) {
    speed = PWMThresh + (255 - PWMThresh) * ((sqrt(LX * LX + LY * LY) - deadspace)/ 590);  // Plan movement
    direction = atan2(-LY, LX);

    // Sort direction into 8 bins and move in direction at full speed based on input
    if((-PI/8 <= direction) && (direction < PI/8)) {              // Strafe right
        LFPower = 255;
        RFPower = -255;
        LRPower = 255;
        RRPower = 255;
    } else if((-3*PI/8 <= direction) && (direction < -PI/8)) {    // Strafe back and right
        LFPower = 255;
        RFPower = 255;
        LRPower = 255;
        RRPower = 255;
    } else if((-5*PI/8 <= direction) && (direction < -3*PI/8)) {  // Reverse
        LFPower = 255;
        RFPower = 255;
        LRPower = 255;
        RRPower = 255;
    } else if ((-7*PI/8 <= direction) && (direction < -5*PI/8)) { // Strafe back and left
        LFPower = 255;
        RFPower = 255;
        LRPower = 255;
        RRPower = 255;
    } else if ((direction < -7*PI/8) || (direction > 7*PI/8)) {   // Strafe left
        LFPower = 255;
        RFPower = 255;
        LRPower = 255;
        RRPower = 255;
    } else if ((5*PI/8 <= direction) && (direction < 7*PI/8)) {   // Strafe left and forward
        LFPower = 255;
        RFPower = 255;
        LRPower = 255;
        RRPower = 255;
    } else if ((3*PI/8 <= direction) && (direction < 5*PI/8)) {   // Forward
        LFPower = 255;
        RFPower = 255;
        LRPower = 255;
        RRPower = 255;
    } else {                                                      // Forward and right
        LFPower = 255;
        RFPower = 255;
        LRPower = 255;
        RRPower = 255;
    }

  } else {
    speed = 0;
    rotation = 0;
    direction= 0;
    LFPower = 0;
    RRPower = 0;
    LRPower = 0;
    RFPower = 0;
  }
  
  // Constrain all pwm values to acceptable values
  if ((0 < LFPower && LFPower < PWMThresh) || (-PWMThresh < LFPower && LFPower < 0)) {
    LFPower = 0;
  } else if (LFPower > 255) {
    LFPower = 255;
  } else if (LFPower < -254) {
    LFPower = -254;
  }

  if ((0 < RFPower && RFPower < PWMThresh) || (-PWMThresh < RFPower && RFPower < 0)) {
    RFPower = 0;
  } else if (RFPower > 255) {
    RFPower = 255;
  } else if (RFPower < -254) {
    RFPower = -254;
  }

  if ((0 < LRPower && LRPower < PWMThresh) || (-PWMThresh < LRPower && LRPower < 0)) {
    LRPower = 0;
  } else if (LRPower > 255) {
    LRPower = 255;
  } else if (LRPower < -254) {
    LRPower = -254;
  }

  if ((0 < RRPower && RRPower < PWMThresh) || (-PWMThresh < RRPower && RRPower < 0)) {
    RRPower = 0;
  } else if (RRPower > 255) {
    RRPower = 255;
  } else if (RRPower < -254) {
    RRPower = -254;
  }

  // Debugging statements below. Outputs robot variables and controller variables
  Serial.printf("Dir: %4f\n", direction);
  // Serial.printf("Spd: %f, rot: %f, dir: %4f, LF: %3i, RF: %3i, LR: %3i, RR: %3i\n", speed, rotation, direction, LFPower, RFPower, LRPower, RRPower);
  // dumpGamepad(ctl);

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
  } else if (LFPower < -PWMThresh){
    analogWrite(in3, 0);
    analogWrite(in4, abs(LFPower));
  } else {
    analogWrite(in3, 0);
    analogWrite(in4, 0);
  }

  if (LRPower > PWMThresh) {  // Rear left
    analogWrite(in5, abs(LRPower));
    analogWrite(in6, 0);
  } else if (LRPower < -PWMThresh){
    analogWrite(in5, 0);
    analogWrite(in6, abs(LRPower));
  } else {
    analogWrite(in5, 0);
    analogWrite(in6, 0);
  }
      
  if (RRPower > PWMThresh) {  //Rear right
    analogWrite(in7, abs(RRPower));
    analogWrite(in8, 0);
  } else if (RRPower < -PWMThresh){
    analogWrite(in7, 0);
    analogWrite(in8, abs(RRPower));
  } else {
    analogWrite(in7, 0);
    analogWrite(in8, 0);
  }
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


// Arduino setup function. Runs in CPU 1
void setup() {
  Serial.begin(115200);        // Begin serial communication for debugging. 
  Serial.printf("Firmware: %s\n", BP32.firmwareVersion());

  // This line prints the address of the connected controller. Good for 1st time setup
  const uint8_t* addr = BP32.localBdAddress();
  Serial.printf("BD Addr: %2X:%2X:%2X:%2X:%2X:%2X\n", addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);
  // If you are setting up your own controller, make sure to copy the address it lists for the controller
  // And paste the result into line 15, following the format 


  // Only connect to the specified controller in controller_addr. Uncomment this section 
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
}


// Arduino loop function. Runs in CPU 1.
void loop() {
  bool dataUpdated = BP32.update();     // fetch controller data
  if (dataUpdated) {
    processControllers();    // Robot motion programmed here
  }
  
  // If your robot does other things you can put them here!

  vTaskDelay(1);
  delay(15);
}
