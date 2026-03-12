/*
    RP2040 USB Dynamic Lighting Device
    Using USB LampArray to control 8x11 matrix with Microsoft Dynamic Lighting
    Restart Windows everytime attributes is changed to remove the cache of previous attributes in Windows.
    The matrix is wired in serpentine way using WS2812B leds
*/


#include <Arduino.h>
#include <Adafruit_TinyUSB.h>
#include <FastLED.h>

// Structs
typedef struct __attribute__((packed)) {
    uint16_t lamp_count;
    uint32_t BoundingBoxWidthInMicrometers;
    uint32_t BoundingBoxHeightInMicrometers;
    uint32_t BoundingBoxDepthInMicrometers;
    uint32_t LampArrayKind;
    uint32_t MinUpdateIntervalInMicroseconds;
} lamp_array_attributes_report_t;

typedef struct __attribute__((packed)) {
    uint16_t LampID;
    uint32_t PositionXInMicrometers;
    uint32_t PositionYInMicrometers;
    uint32_t PositionZInMicrometers;
    uint32_t LampPurposes;
    uint32_t UpdateLatencyInMicroseconds;
    uint8_t RedLevelCount;
    uint8_t GreenLevelCount;
    uint8_t BlueLevelCount;
    uint8_t IntensityLevelCount;
    uint8_t IsProgrammable;
    uint8_t InputBinding;
} lamp_attributes_report_t;

typedef struct {
    uint8_t RedLevel;
    uint8_t GreenLevel;
    uint8_t BlueLevel;
    uint8_t IntensityLevel;
} lamp_rgb_t;

// WS2812B
#define WS2812B_PIN 12
#define WS2812B_UPDATE_MS 4 * 1000

// MATRIX
#define LAMP_COLS 8
#define LAMP_ROWS 11
#define TOTAL_LEDS (LAMP_COLS * LAMP_ROWS)
#define X_OFFSET 13000 // Offset from X origin in Micrometers
#define Y_OFFSET 13000 // Offset from Y origin in Micrometers
#define X_STEP   25000 // Distance between each led in X direction in Micrometers
#define Y_STEP   25000 // Distance between each led in Y direction in Micrometers
#define Z_POS    10000 // Z position for all LEDs in Micrometers
#define BOX_WIDTH_IN_MM 201
#define BOX_HEIGHT_IN_MM 301
#define BOX_DEPTH_IN_MM 10
#define LampArrayType 0x0A // Set as Artwork type
#define UPDATE_INTERVAL_MS 10 // Minimum time host should wait before sending next update after receiving an update in milliseconds
const static lamp_attributes_report_t LampIDAttributes[TOTAL_LEDS] = {

#define LAMP_ENTRY(i, x, y) \
{ i, x, y, 1000, WS2812B_UPDATE_MS, 0x20, 0xFF,0xFF,0xFF, 0x01, 0x01, 0x00 }

// Row 0: left → right
LAMP_ENTRY(0, X_OFFSET + 0*X_STEP, Y_OFFSET + 0*Y_STEP),
LAMP_ENTRY(1, X_OFFSET + 1*X_STEP, Y_OFFSET + 0*Y_STEP),
LAMP_ENTRY(2, X_OFFSET + 2*X_STEP, Y_OFFSET + 0*Y_STEP),
LAMP_ENTRY(3, X_OFFSET + 3*X_STEP, Y_OFFSET + 0*Y_STEP),
LAMP_ENTRY(4, X_OFFSET + 4*X_STEP, Y_OFFSET + 0*Y_STEP),
LAMP_ENTRY(5, X_OFFSET + 5*X_STEP, Y_OFFSET + 0*Y_STEP),
LAMP_ENTRY(6, X_OFFSET + 6*X_STEP, Y_OFFSET + 0*Y_STEP),
LAMP_ENTRY(7, X_OFFSET + 7*X_STEP, Y_OFFSET + 0*Y_STEP),

// Row 1: right → left
LAMP_ENTRY(8,  X_OFFSET + 7*X_STEP, Y_OFFSET + 1*Y_STEP),
LAMP_ENTRY(9,  X_OFFSET + 6*X_STEP, Y_OFFSET + 1*Y_STEP),
LAMP_ENTRY(10, X_OFFSET + 5*X_STEP, Y_OFFSET + 1*Y_STEP),
LAMP_ENTRY(11, X_OFFSET + 4*X_STEP, Y_OFFSET + 1*Y_STEP),
LAMP_ENTRY(12, X_OFFSET + 3*X_STEP, Y_OFFSET + 1*Y_STEP),
LAMP_ENTRY(13, X_OFFSET + 2*X_STEP, Y_OFFSET + 1*Y_STEP),
LAMP_ENTRY(14, X_OFFSET + 1*X_STEP, Y_OFFSET + 1*Y_STEP),
LAMP_ENTRY(15, X_OFFSET + 0*X_STEP, Y_OFFSET + 1*Y_STEP),

// Row 2: left → right
LAMP_ENTRY(16, X_OFFSET + 0*X_STEP, Y_OFFSET + 2*Y_STEP),
LAMP_ENTRY(17, X_OFFSET + 1*X_STEP, Y_OFFSET + 2*Y_STEP),
LAMP_ENTRY(18, X_OFFSET + 2*X_STEP, Y_OFFSET + 2*Y_STEP),
LAMP_ENTRY(19, X_OFFSET + 3*X_STEP, Y_OFFSET + 2*Y_STEP),
LAMP_ENTRY(20, X_OFFSET + 4*X_STEP, Y_OFFSET + 2*Y_STEP),
LAMP_ENTRY(21, X_OFFSET + 5*X_STEP, Y_OFFSET + 2*Y_STEP),
LAMP_ENTRY(22, X_OFFSET + 6*X_STEP, Y_OFFSET + 2*Y_STEP),
LAMP_ENTRY(23, X_OFFSET + 7*X_STEP, Y_OFFSET + 2*Y_STEP),

// Row 3: right → left
LAMP_ENTRY(24, X_OFFSET + 7*X_STEP, Y_OFFSET + 3*Y_STEP),
LAMP_ENTRY(25, X_OFFSET + 6*X_STEP, Y_OFFSET + 3*Y_STEP),
LAMP_ENTRY(26, X_OFFSET + 5*X_STEP, Y_OFFSET + 3*Y_STEP),
LAMP_ENTRY(27, X_OFFSET + 4*X_STEP, Y_OFFSET + 3*Y_STEP),
LAMP_ENTRY(28, X_OFFSET + 3*X_STEP, Y_OFFSET + 3*Y_STEP),
LAMP_ENTRY(29, X_OFFSET + 2*X_STEP, Y_OFFSET + 3*Y_STEP),
LAMP_ENTRY(30, X_OFFSET + 1*X_STEP, Y_OFFSET + 3*Y_STEP),
LAMP_ENTRY(31, X_OFFSET + 0*X_STEP, Y_OFFSET + 3*Y_STEP),

// Row 4: left → right
LAMP_ENTRY(32, X_OFFSET + 0*X_STEP, Y_OFFSET + 4*Y_STEP),
LAMP_ENTRY(33, X_OFFSET + 1*X_STEP, Y_OFFSET + 4*Y_STEP),
LAMP_ENTRY(34, X_OFFSET + 2*X_STEP, Y_OFFSET + 4*Y_STEP),
LAMP_ENTRY(35, X_OFFSET + 3*X_STEP, Y_OFFSET + 4*Y_STEP),
LAMP_ENTRY(36, X_OFFSET + 4*X_STEP, Y_OFFSET + 4*Y_STEP),
LAMP_ENTRY(37, X_OFFSET + 5*X_STEP, Y_OFFSET + 4*Y_STEP),
LAMP_ENTRY(38, X_OFFSET + 6*X_STEP, Y_OFFSET + 4*Y_STEP),
LAMP_ENTRY(39, X_OFFSET + 7*X_STEP, Y_OFFSET + 4*Y_STEP),

// Row 5: right → left
LAMP_ENTRY(40, X_OFFSET + 7*X_STEP, Y_OFFSET + 5*Y_STEP),
LAMP_ENTRY(41, X_OFFSET + 6*X_STEP, Y_OFFSET + 5*Y_STEP),
LAMP_ENTRY(42, X_OFFSET + 5*X_STEP, Y_OFFSET + 5*Y_STEP),
LAMP_ENTRY(43, X_OFFSET + 4*X_STEP, Y_OFFSET + 5*Y_STEP),
LAMP_ENTRY(44, X_OFFSET + 3*X_STEP, Y_OFFSET + 5*Y_STEP),
LAMP_ENTRY(45, X_OFFSET + 2*X_STEP, Y_OFFSET + 5*Y_STEP),
LAMP_ENTRY(46, X_OFFSET + 1*X_STEP, Y_OFFSET + 5*Y_STEP),
LAMP_ENTRY(47, X_OFFSET + 0*X_STEP, Y_OFFSET + 5*Y_STEP),

// Row 6: left → right
LAMP_ENTRY(48, X_OFFSET + 0*X_STEP, Y_OFFSET + 6*Y_STEP),
LAMP_ENTRY(49, X_OFFSET + 1*X_STEP, Y_OFFSET + 6*Y_STEP),
LAMP_ENTRY(50, X_OFFSET + 2*X_STEP, Y_OFFSET + 6*Y_STEP),
LAMP_ENTRY(51, X_OFFSET + 3*X_STEP, Y_OFFSET + 6*Y_STEP),
LAMP_ENTRY(52, X_OFFSET + 4*X_STEP, Y_OFFSET + 6*Y_STEP),
LAMP_ENTRY(53, X_OFFSET + 5*X_STEP, Y_OFFSET + 6*Y_STEP),
LAMP_ENTRY(54, X_OFFSET + 6*X_STEP, Y_OFFSET + 6*Y_STEP),
LAMP_ENTRY(55, X_OFFSET + 7*X_STEP, Y_OFFSET + 6*Y_STEP),

// Row 7: right → left
LAMP_ENTRY(56, X_OFFSET + 7*X_STEP, Y_OFFSET + 7*Y_STEP),
LAMP_ENTRY(57, X_OFFSET + 6*X_STEP, Y_OFFSET + 7*Y_STEP),
LAMP_ENTRY(58, X_OFFSET + 5*X_STEP, Y_OFFSET + 7*Y_STEP),
LAMP_ENTRY(59, X_OFFSET + 4*X_STEP, Y_OFFSET + 7*Y_STEP),
LAMP_ENTRY(60, X_OFFSET + 3*X_STEP, Y_OFFSET + 7*Y_STEP),
LAMP_ENTRY(61, X_OFFSET + 2*X_STEP, Y_OFFSET + 7*Y_STEP),
LAMP_ENTRY(62, X_OFFSET + 1*X_STEP, Y_OFFSET + 7*Y_STEP),
LAMP_ENTRY(63, X_OFFSET + 0*X_STEP, Y_OFFSET + 7*Y_STEP),

// Row 8: left → right
LAMP_ENTRY(64, X_OFFSET + 0*X_STEP, Y_OFFSET + 8*Y_STEP),
LAMP_ENTRY(65, X_OFFSET + 1*X_STEP, Y_OFFSET + 8*Y_STEP),
LAMP_ENTRY(66, X_OFFSET + 2*X_STEP, Y_OFFSET + 8*Y_STEP),
LAMP_ENTRY(67, X_OFFSET + 3*X_STEP, Y_OFFSET + 8*Y_STEP),
LAMP_ENTRY(68, X_OFFSET + 4*X_STEP, Y_OFFSET + 8*Y_STEP),
LAMP_ENTRY(69, X_OFFSET + 5*X_STEP, Y_OFFSET + 8*Y_STEP),
LAMP_ENTRY(70, X_OFFSET + 6*X_STEP, Y_OFFSET + 8*Y_STEP),
LAMP_ENTRY(71, X_OFFSET + 7*X_STEP, Y_OFFSET + 8*Y_STEP),

// Row 9: right → left
LAMP_ENTRY(72, X_OFFSET + 7*X_STEP, Y_OFFSET + 9*Y_STEP),
LAMP_ENTRY(73, X_OFFSET + 6*X_STEP, Y_OFFSET + 9*Y_STEP),
LAMP_ENTRY(74, X_OFFSET + 5*X_STEP, Y_OFFSET + 9*Y_STEP),
LAMP_ENTRY(75, X_OFFSET + 4*X_STEP, Y_OFFSET + 9*Y_STEP),
LAMP_ENTRY(76, X_OFFSET + 3*X_STEP, Y_OFFSET + 9*Y_STEP),
LAMP_ENTRY(77, X_OFFSET + 2*X_STEP, Y_OFFSET + 9*Y_STEP),
LAMP_ENTRY(78, X_OFFSET + 1*X_STEP, Y_OFFSET + 9*Y_STEP),
LAMP_ENTRY(79, X_OFFSET + 0*X_STEP, Y_OFFSET + 9*Y_STEP),

// Row 10: left → right
LAMP_ENTRY(80, X_OFFSET + 0*X_STEP, Y_OFFSET + 10*Y_STEP),
LAMP_ENTRY(81, X_OFFSET + 1*X_STEP, Y_OFFSET + 10*Y_STEP),
LAMP_ENTRY(82, X_OFFSET + 2*X_STEP, Y_OFFSET + 10*Y_STEP),
LAMP_ENTRY(83, X_OFFSET + 3*X_STEP, Y_OFFSET + 10*Y_STEP),
LAMP_ENTRY(84, X_OFFSET + 4*X_STEP, Y_OFFSET + 10*Y_STEP),
LAMP_ENTRY(85, X_OFFSET + 5*X_STEP, Y_OFFSET + 10*Y_STEP),
LAMP_ENTRY(86, X_OFFSET + 6*X_STEP, Y_OFFSET + 10*Y_STEP),
LAMP_ENTRY(87, X_OFFSET + 7*X_STEP, Y_OFFSET + 10*Y_STEP),
};

// USB
#define LAMPARRAY_ID 1
uint8_t const desc_hid_report[] = {
    TUD_HID_REPORT_DESC_LIGHTING(LAMPARRAY_ID)
};
Adafruit_USBD_HID usb_hid;

// Conway Game of Life Animation
uint8_t generation = 0;
uint8_t percentAlive = 32; // Initial percentage of alive cells in randomFillWorld
class Cell {
public:
    byte alive =  1;
    byte prev =  1;
    byte hue = 6;  
    byte brightness;
};
Cell world[LAMP_COLS][LAMP_ROWS];
CRGBPalette16 currentPalette = PartyColors_p; // default palette

// FASTLED
CRGB leds_matrix[TOTAL_LEDS];
CRGB leds_matrix_new[TOTAL_LEDS];

// Global Variable
uint16_t requested_LampID;
uint8_t AutonomousMode = true;
uint8_t LampUpdateComplete = true;
lamp_rgb_t LampColors[TOTAL_LEDS] = {0};
lamp_rgb_t LampColorsBuffer[TOTAL_LEDS] = {0};


// Functions Prototypes
uint16_t hid_get_report_cb(uint8_t report_id, hid_report_type_t report_type, uint8_t *buffer, uint16_t reqlen);
void hid_set_report_cb(uint8_t report_id, hid_report_type_t report_type, uint8_t const* buffer, uint16_t bufsize);
void applyBufferedColors();
void stepWorld();
void updateLampColors();
void randomFillWorld();
void chooseNewPalette();
uint8_t neighbors(uint8_t x, uint8_t y);
uint16_t XY(uint8_t x, uint8_t y);


void setup() {
    if (!TinyUSBDevice.isInitialized()) {
        TinyUSBDevice.begin(0);
    }
    usb_hid.setBootProtocol(HID_ITF_PROTOCOL_NONE);
    usb_hid.setReportDescriptor(desc_hid_report, sizeof(desc_hid_report));
    usb_hid.setStringDescriptor("RP2040 RGB MATRIX");
    USBDevice.setID(0x1209, 0x0010); // pid.codes for VID PID usage
    USBDevice.setConfigurationMaxPower(510);
    usb_hid.setReportCallback(hid_get_report_cb, hid_set_report_cb);
    usb_hid.begin();

    if (TinyUSBDevice.mounted()) {
    TinyUSBDevice.detach();
    delay(10);
    TinyUSBDevice.attach();
    }

    FastLED.addLeds<WS2812B, WS2812B_PIN, GRB>(leds_matrix, TOTAL_LEDS);
    FastLED.setBrightness(80);
}

void loop() {
    bool showUpdate = false;

    if(AutonomousMode){
        //Play Game of Life animation
        if(generation == 0){
            randomFillWorld();
            chooseNewPalette();
        }
        updateLampColors();
        stepWorld();
        delay(350);
        generation++;
        if(generation >= 128){
            generation = 0;
        }
        showUpdate = true;

    }
    else{
    // Update LEDs
        for (uint16_t i = 0; i < TOTAL_LEDS; i++) {
            leds_matrix_new[i].r = LampColors[i].RedLevel;
            leds_matrix_new[i].g = LampColors[i].GreenLevel;
            leds_matrix_new[i].b = LampColors[i].BlueLevel;

            if (leds_matrix_new[i].r != leds_matrix[i].r || leds_matrix_new[i].g != leds_matrix[i].g || leds_matrix_new[i].b != leds_matrix[i].b) {
                leds_matrix[i].r = leds_matrix_new[i].r;
                leds_matrix[i].g = leds_matrix_new[i].g;
                leds_matrix[i].b = leds_matrix_new[i].b;
                showUpdate = true;
            }
        }
    }
    if (showUpdate){
        FastLED.show();
    }
}

// ------------------------ HID Callbacks ------------------------
uint16_t hid_get_report_cb(uint8_t report_id, hid_report_type_t report_type, uint8_t *buffer, uint16_t reqlen) {
    if (report_type == HID_REPORT_TYPE_FEATURE) { // Host request attributes to device using feature type
        if (report_id == LAMPARRAY_ID) { // Report ID 1: Lamp Array Attributes
            const static lamp_array_attributes_report_t report = {
                .lamp_count = TOTAL_LEDS,
                .BoundingBoxWidthInMicrometers = BOX_WIDTH_IN_MM*1000,
                .BoundingBoxHeightInMicrometers = BOX_HEIGHT_IN_MM*1000,
                .BoundingBoxDepthInMicrometers = BOX_DEPTH_IN_MM*1000,
                .LampArrayKind = LampArrayType,
                .MinUpdateIntervalInMicroseconds = UPDATE_INTERVAL_MS*1000
            };
            memcpy(buffer, &report, sizeof(report));
            return sizeof(report);
        }
        if (report_id == (LAMPARRAY_ID + 2)) { // Report ID 3: Lamp Attributes for requested LampID
            lamp_attributes_report_t report = LampIDAttributes[requested_LampID];
            memcpy(buffer, &report, sizeof(report));
            requested_LampID++; 
            if (requested_LampID >= TOTAL_LEDS){
                requested_LampID = 0;
            } 
            return sizeof(report);
        }
    }
    return 0; // Invalid Request
}

void hid_set_report_cb(uint8_t report_id, hid_report_type_t report_type, uint8_t const* buffer, uint16_t bufsize) {

    if (report_type == HID_REPORT_TYPE_OUTPUT || report_type == HID_REPORT_TYPE_FEATURE) {
        if (report_id == (LAMPARRAY_ID + 1)) { // Report ID 2: Set requested LampID
            requested_LampID = buffer[0];
            return;
        }
        if (report_id == (LAMPARRAY_ID + 3)) { // Report ID 4: Lamp Multiupdate report
            uint8_t numUpdates = buffer[0];
            uint16_t lampId;
            uint16_t lampIdStart = 2; //
            uint16_t rgbStart = lampIdStart + 16;   // the RGBI tuplex start after 16 LampIDs based on descriptor
            bool LampUpdateComplete = buffer[1] & 0x01;

            // TODO: Channel Level check and identical LampID check

            // Store updates only
            for (uint8_t i = 0; i < numUpdates; i++) {
                lampId = buffer[lampIdStart + i*2] | (buffer[lampIdStart + i*2 + 1] << 8); 
                if (lampId >= TOTAL_LEDS) break;
                LampColorsBuffer[lampId].RedLevel       = buffer[rgbStart + i*4 + 0];
                LampColorsBuffer[lampId].GreenLevel     = buffer[rgbStart + i*4 + 1];
                LampColorsBuffer[lampId].BlueLevel      = buffer[rgbStart + i*4 + 2];
                LampColorsBuffer[lampId].IntensityLevel = buffer[rgbStart + i*4 + 3];
            }
            if (LampUpdateComplete && !(lampId >= TOTAL_LEDS)) { 
                applyBufferedColors();
            }
            return;
        }
        if (report_id == (LAMPARRAY_ID + 4)) { // Report ID 5: Lamp Range update report
            bool LampUpdateComplete = buffer[0] & 0x01;
            uint16_t startLampId = buffer[1] | (buffer[2] << 8);
            uint16_t endLampId   = buffer[3] | (buffer[4] << 8);
            bool report_valid = true;
            // Error Check
            if (startLampId > endLampId) return;
            if (startLampId >= TOTAL_LEDS || endLampId >= TOTAL_LEDS) return;
            // TODO: Channel Level check

            // Store updates only
            for (uint16_t i = startLampId; i <= endLampId; i++) {
                LampColorsBuffer[i].RedLevel       = buffer[5];
                LampColorsBuffer[i].GreenLevel     = buffer[6];
                LampColorsBuffer[i].BlueLevel      = buffer[7];
                LampColorsBuffer[i].IntensityLevel = buffer[8];
            }
            if (LampUpdateComplete) { // Update when final update arrives
                applyBufferedColors();
            }
            return;
        }
    }
    if (report_type == HID_REPORT_TYPE_FEATURE) {
        if (report_id == (LAMPARRAY_ID + 5)) { // Report ID 6: Set Autonomous Mode
            AutonomousMode = buffer[0] & 0x01;
            return;
        }
    }   
}

void applyBufferedColors() { 
    memcpy(LampColors, LampColorsBuffer, sizeof(LampColors));
    return;
}

// Conway's Game of life Animation
uint16_t XY(uint8_t x, uint8_t y) {
    if (y % 2 == 0) {
        // Even row (0,2,4...): left-to-right
        return y * LAMP_COLS + x;
    } else {
        // Odd row (1,3,5...): right-to-left
        return y * LAMP_COLS + (LAMP_COLS - 1 - x);
    }
}

void randomFillWorld() {
    for (uint8_t x = 0; x < LAMP_COLS; x++) {
        for (uint8_t y = 0; y < LAMP_ROWS; y++) {
            if (random(100) < percentAlive) { // 50% chance alive
                world[x][y].alive = true;
                world[x][y].brightness = 255;
            } else {
                world[x][y].alive = false;
                world[x][y].brightness = 0;
            }
            world[x][y].prev = world[x][y].alive;
        }
    }
}

uint8_t neighbors(uint8_t x, uint8_t y) {
    int count = 0;
    for (int dx = -1; dx <= 1; dx++) {
        for (int dy = -1; dy <= 1; dy++) {
            if (dx == 0 && dy == 0) continue; // skip the cell itself

            uint8_t nx = x + dx;
            uint8_t ny = y + dy;

            // check bounds
            if (nx >= 0 && nx < LAMP_COLS && ny >= 0 && ny < LAMP_ROWS) {
                if (world[nx][ny].prev) count++;
            }
        }
    }
    return count;
}

void updateLampColors() {
    for (uint8_t x = 0; x < LAMP_COLS; x++) {
        for (uint8_t y = 0; y < LAMP_ROWS; y++) {
            uint16_t lampId = XY(x, y);  // Map to lamp ID
            if (world[x][y].alive) {
                leds_matrix[lampId] = ColorFromPalette(currentPalette, world[x][y].hue * 2, world[x][y].brightness);
            } else {
                leds_matrix[lampId] = CRGB::Black;
            }
        }
    }
}

void stepWorld() {
    bool anyAlive = false;
    bool stable = true;  // assume stable first

    // Step 1: compute next generation
    for (uint8_t x = 0; x < LAMP_COLS; x++) {
        for (uint8_t y = 0; y < LAMP_ROWS; y++) {
            uint8_t n = neighbors(x, y);

            if (world[x][y].prev) {
                world[x][y].alive = (n == 2 || n == 3);
            } else {
                world[x][y].alive = (n == 3);
                if (world[x][y].alive) world[x][y].brightness = 255;
                world[x][y].hue += 2;
            }

            if (world[x][y].alive) anyAlive = true;

            // detect change
            if (world[x][y].alive != world[x][y].prev) {
                stable = false;
            }
        }
    }

    // Step 2: reset if dead or stable
    if (!anyAlive || stable) {
        randomFillWorld();
        chooseNewPalette();
        generation = 0;
    }

    // Step 3: copy next generation
    for (uint8_t x = 0; x < LAMP_COLS; x++)
        for (uint8_t y = 0; y < LAMP_ROWS; y++)
            world[x][y].prev = world[x][y].alive;
}

void chooseNewPalette() {
    switch(random(0, 8)) {
        case 0: currentPalette = CloudColors_p; break;
        case 1: currentPalette = ForestColors_p; break;
        case 2: currentPalette = HeatColors_p; break;
        case 3: currentPalette = LavaColors_p; break;
        case 4: currentPalette = OceanColors_p; break;
        case 5: currentPalette = PartyColors_p; break;
        case 6: currentPalette = RainbowColors_p; break;
        case 7:
        default: currentPalette = RainbowStripeColors_p; break;
    }
}
