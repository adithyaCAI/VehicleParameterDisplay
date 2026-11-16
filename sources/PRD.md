[page 1]
CraftifAI | Vehicle Cabin Climate & Ride Dynamics Monitor | PRD Page 1  
 
Vehicle Cabin Climate and Ride Dynamics Monitor 
Product Requirements Document | STM32-NUCLEO-G070RB 
1. Objective 
Monitor interior cabin climate and vehicle ride dynamics using the STM32-NUCLEO-G070RB, and show the data 
on an OLED display for monitoring. The CraftifAI logo is shown on the OLED for the first 2 seconds after 
power-up. 
2. Hardware 
• MCU: STM32-NUCLEO-G070RB dev board 
• Climate sensor: DHT22 (temperature, humidity) 
• IMU: MPU6050 (3-axis accelerometer and gyroscope) 
• Display: 1.3" OLED (SH1106 driver), I2C 
• Power: 5 V via micro-USB (from a 12 V automotive adapter) 
2.1 Pin mapping 
 
Signal     STM32-NUCLEO-G070RB Pin 
DHT22 VCC     3.3V 
DHT22 GND     GND 
DHT22 OUT    PB4 
MPU6050 VCC    3.3V 
MPU6050 GND    GND 
MPU6050 SCL    PB8 (shared with OLED) 
 MPU6050 SDA    PB9 (shared with OLED) 
 OLED VDD     3.3V 
 OLED GND     GND 
 OLED SCL     PB8 (shared with MPU6050) 
 OLED SDA     PB9 (shared with MPU6050) 
 Inbuilt User LED (LD4)   PA 
 
3. Functional Requirements 
3.1 Startup logo (new) 
• FR-LOGO-1: On power-up or reset, the OLED shows the CraftifAI logo for 2 seconds, before any sensor data 
or heading is displayed. 
• FR-LOGO-2: The logo is stored in flash as a 1-bit monochrome bitmap array (logo.h), 128x64 px, SH1106 
page format (8 pages x 128 columns, LSB = top row). 
• FR-LOGO-3: The artwork (124x26 px) is centred on the display with a black border. 
• FR-LOGO-4: While the logo is shown, the firmware initialises the DHT22 and runs the MPU6050 zeroing (see 
3.3). This runs in the background and does not extend the splash time. The splash timing must be non-blocking 
(HAL_GetTick), not a blocking delay. 
• FR-LOGO-5: No alerts are shown and the LED does not blink during the splash. After 2 s the display switches 
to the Vehicle Parameters screen. 
• FR-LOGO-6: The logo display time is a configurable macro in config.h: LOGO_DISPLAY_MS = 2000. 
• FR-LOGO-7: The SH1106 has 132 columns of RAM but shows 128, so the driver applies a 2-column offset 
when writing the logo, as for all other screens. 


[page 2]
•  FR-LOGO-8: The logo is drawn white on a black background (inverted from the navy-on-white source artwork).

3.2 Climate
•  Read temperature (°C) and humidity (%) from the DHT22 every 2 seconds.
•  If temperature or humidity goes above its limit, show "!" next to the value.
3.3 Ride dynamics
•  Read X, Y, Z acceleration from the MPU6050 (±4 g range, 100 Hz) and convert to g.
•  At power-up, keep the board flat and still for 1 s so the readings are zeroed (X = 0 g, Y = 0 g, Z = 1 g). This
happens during the logo splash (FR-LOGO-4).
•  Axis mapping: X = lateral (cornering), Y = longitudinal (acceleration/braking), Z = vertical (bumps).

3.4 Event detection
"Suddenly" means the axis value changes by at least the threshold within 100 ms. The original 15 g threshold was
replaced because it is a crash-level force and would never trigger.
Event message   Condition

Unseen SpeedBreaker   Z changes by 0.30 g or more

Wide Turn   X changes by 0.25 g or more

Speeding   Y changes by +0.25 g or more (indicates harsh acceleration, not actual speed)

Sudden Braking   Y changes by -0.25 g or more (negative direction)
•  When an event occurs, the message appears immediately on the OLED while Temperature and Humidity stay
visible.
•  LD4 (PA5) blinks at 1 Hz (500 ms on, 500 ms off).
•  The message and blinking continue for 2 s after the last event.
•  Overlap priority: Sudden Braking, then Unseen SpeedBreaker, then Wide Turn, then Speeding.

3.5 Configurable thresholds (config.h)

Parameter   Demo value   Demo trigger   In-vehicle

Temperature high   32 °C   Hold sensor between warm fingers   45 °C

Humidity high   75 %   Breathe gently on the DHT22   80 %

Z (Unseen SpeedBreaker)   0.30 g   Quick tap or lift-and-drop   0.4-0.5 g

X (Wide Turn)   0.25 g   Quick flick left or right   0.4-0.5 g

+Y (Speeding)   +0.25 g   Quick push forward   0.4-0.5 g

-Y (Sudden Braking)   -0.25 g   Quick pull backward   0.4-0.5 g

Sudden window   100 ms

Event display time   2 s

Logo display time   2 s   Automatic at power-up

CraftifAI | Vehicle Cabin Climate & Ride Dynamics Monitor | PRD   Page 2

[page 3]
CraftifAI | Vehicle Cabin Climate & Ride Dynamics Monitor | PRD Page 3  
 
 
4. OLED Display Format 
Startup (0-2 s): full-screen CraftifAI logo. 
Normal screen (after 2 s): 
Vehicle Parameters 
    Display 
Temp: “value from DHT22” °C 
Humidity: “: “value from DHT22” % 
X:<x> 
Y:<y> 
Z:<z> g  
 
During an event, the event message is shown on the screen while Temp and Humidity remain visible. 
5. Expected Use 
Installed in a vehicle, streaming cabin comfort and ride data to a dashboard for driver behaviour review or ride 
comfort tracking, with immediate alerts on extreme cabin or dynamics events. The logo provides branding and a 
visual confirmation at boot that the display is working. 
6. Acceptance Criteria for the Logo 
1. The logo appears within 100 ms of power-up and stays for 2 s ± 100 ms. 
2. The logo is not distorted, inverted or offset (check the SH1106 column offset). 
3. The first sensor screen appears right after the splash, with valid zeroed X/Y/Z. 
4. A reset during operation replays the splash.