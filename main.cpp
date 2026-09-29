#include <Arduino.h>
#include <math.h>
#include <Adafruit_BNO08x.h>
#include <AceButton.h>
using namespace ace_button;

#define BNO08X_RESET -1
int pinD1 = 1;
AceButton button(pinD1);

Adafruit_BNO08x bno08x(BNO08X_RESET);
sh2_SensorValue_t sensorValue;

long flipTime1 = 0; // switch LED 1 state
long flipTime2 = 0;
long now = millis();
byte flag1 = 0;
byte flag2 = 0;

enum AxisMode
{
  Mode_Both,
  Mode_x,
  Mode_y,
  Mode_count
};

AxisMode currentMode = Mode_Both;

void ChangeMode(AceButton *button, uint8_t eventType, uint8_t buttonState)
{
  if (eventType == (uint8_t)AceButton::kEventDoubleClicked)
  {
    currentMode = (AxisMode)((currentMode + 1) % AxisMode::Mode_count);
  }
}

void setReports();

void setup(void)
{
  Serial.begin(115200);
  while (!Serial)
    delay(10); // will pause Zero, Leonardo, etc until serial console opens

  Serial.println("Adafruit BNO08x test!");

  pinMode(pinD1, INPUT_PULLDOWN);
  button.init(pinD1, LOW);

  ButtonConfig *buttonConfig = button.getButtonConfig();
  buttonConfig->setEventHandler(ChangeMode);
  buttonConfig->setFeature(ButtonConfig::kFeatureClick);
  buttonConfig->setFeature(ButtonConfig::kFeatureDoubleClick);

  // Try to initialize!
  if (!bno08x.begin_I2C())
  {
    // if (!bno08x.begin_UART(&Serial1)) {  // Requires a device with > 300 byte
    // UART buffer! if (!bno08x.begin_SPI(BNO08X_CS, BNO08X_INT)) {
    Serial.println("Failed to find BNO08x chip");
    while (1)
    {
      delay(10);
    }
  }
  Serial.println("BNO08x Found!");
  setReports();
}

void loop()
{
  now = millis();

  button.check();

  if (bno08x.wasReset())
  {
    Serial.println("sensor was reset ");
    setReports();
  }

  if (!bno08x.getSensorEvent(&sensorValue))
  {
    return;
  }

  float x = sensorValue.un.accelerometer.x;
  float y = sensorValue.un.accelerometer.y;
  float z = sensorValue.un.accelerometer.z;

  float alpha = degrees(atan2(x, sqrt(y * y + z * z)));
  float beta = degrees(atan2(y, x));

  /*Serial.print("Accelerometer - x: ");
  Serial.print(x);
  Serial.print(" y: ");
  Serial.print(y);
  Serial.print(" z: ");
  Serial.print(z);

  Serial.print(" Alpha: ");
  Serial.print(alpha);
  Serial.print(" Beta: ");
  Serial.println(beta); */

  if (currentMode == AxisMode::Mode_Both)
  {
    Serial.print("X Angle: ");
    Serial.print(alpha);
    Serial.print("   Y Angle: ");
    Serial.println(beta);
  }
  else if (currentMode == AxisMode::Mode_x)
  {
    Serial.print("X Angle: ");
    Serial.println(alpha);
  }
  else if (currentMode == AxisMode::Mode_y)
  {
    Serial.print("Y Angle: ");
    Serial.println(beta);
  }

  if (now > flipTime1)
  {
    flipTime1 = now + 5 * abs(alpha);
    if (flag1)
    {
      flag1 = 0;
      analogWrite(9, 0);
    }
    else
    {
      flag1 = 1;
      analogWrite(9, 255);
    }
  }

  if (now > flipTime2)
  {
    flipTime2 = now + 5 * abs(beta);
    if (flag2)
    {
      flag2 = 0;
      analogWrite(10, 0);
    }
    else
    {
      flag2 = 1;
      analogWrite(10, 255);
    }
  }
}

void setReports(void)
{
  Serial.println("Setting desired reports");
  if (!bno08x.enableReport(SH2_ACCELEROMETER))
  {
    Serial.println("Could not enable accelerometer");
  }
  else
  {
    Serial.println("Success!");
  }
}