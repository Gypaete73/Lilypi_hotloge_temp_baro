// => Hardware select
// #define LILYGO_WATCH_2019_WITH_TOUCH         // To use T-Watch2019 with touchscreen, please uncomment this line
// #define LILYGO_WATCH_2019_NO_TOUCH           // To use T-Watch2019 Not touchscreen , please uncomment this line
// #define LILYGO_WATCH_2020_V1                 // To use T-Watch2020 V1, please uncomment this line
// #define LILYGO_WATCH_2020_V2                 // To use T-Watch2020 V2, please uncomment this line
// #define LILYGO_WATCH_2020_V3                 // To use T-Watch2020 V3, please uncomment this line

 #define LILYGO_LILYPI_V1                     //LILYPI / TBLOCK requires an external display module
// #define LILYGO_WATCH_BLOCK                   //LILYPI / TBLOCK requires an external display module



#if defined(LILYGO_LILYPI_V1) || defined(LILYGO_WATCH_BLOCK)
 #define LILYGO_BLOCK_ST7796S_MODULE          //Use ST7796S
// #define LILYGO_BLOCK_ILI9481_MODULE          //Use ILI9841
// #define LILYGO_GC9A01A_MODULE                   //Use GC9A01A
#define LILYGO_TOUCH_DRIVER_FTXXX
#define LILYGO_WATCH_HAS_PCF8563
//#define LILYGO_WATCH_HAS_SDCARD

#endif

#include <LilyGoWatch.h>



//**************************************************************************************
//**                         Section 5: Font datum enumeration
//**************************************************************************************
////These enumerate the text plotting alignment (reference datum point)
//#define TL_DATUM 0 // Top left (default)
//#define TC_DATUM 1 // Top centre
//#define TR_DATUM 2 // Top right
//#define ML_DATUM 3 // Middle left
//#define CL_DATUM 3 // Centre left, same as above
//#define MC_DATUM 4 // Middle centre
//#define CC_DATUM 4 // Centre centre, same as above
//#define MR_DATUM 5 // Middle right
//#define CR_DATUM 5 // Centre right, same as above
//#define BL_DATUM 6 // Bottom left
//#define BC_DATUM 7 // Bottom centre
//#define BR_DATUM 8 // Bottom right
//#define L_BASELINE  9 // Left character baseline (Line the 'A' character would sit on)
//#define C_BASELINE 10 // Centre character baseline
//#define R_BASELINE 11 // Right character baseline
//
///***************************************************************************************
//**                         Section 6: Colour enumeration
//***************************************************************************************/
//// Default color definitions
//#define TFT_BLACK       0x0000      /*   0,   0,   0 */
//#define TFT_NAVY        0x000F      /*   0,   0, 128 */
//#define TFT_DARKGREEN   0x03E0      /*   0, 128,   0 */
//#define TFT_DARKCYAN    0x03EF      /*   0, 128, 128 */
//#define TFT_MAROON      0x7800      /* 128,   0,   0 */
//#define TFT_PURPLE      0x780F      /* 128,   0, 128 */
//#define TFT_OLIVE       0x7BE0      /* 128, 128,   0 */
//#define TFT_LIGHTGREY   0xD69A      /* 211, 211, 211 */
//#define TFT_DARKGREY    0x7BEF      /* 128, 128, 128 */
//#define TFT_BLUE        0x001F      /*   0,   0, 255 */
//#define TFT_GREEN       0x07E0      /*   0, 255,   0 */
//#define TFT_CYAN        0x07FF      /*   0, 255, 255 */
//#define TFT_RED         0xF800      /* 255,   0,   0 */
//#define TFT_MAGENTA     0xF81F      /* 255,   0, 255 */
//#define TFT_YELLOW      0xFFE0      /* 255, 255,   0 */
//#define TFT_WHITE       0xFFFF      /* 255, 255, 255 */
//#define TFT_ORANGE      0xFDA0      /* 255, 180,   0 */
//#define TFT_GREENYELLOW 0xB7E0      /* 180, 255,   0 */
//#define TFT_PINK        0xFE19      /* 255, 192, 203 */ //Lighter pink, was 0xFC9F      
//#define TFT_BROWN       0x9A60      /* 150,  75,   0 */
//#define TFT_GOLD        0xFEA0      /* 255, 215,   0 */
//#define TFT_SILVER      0xC618      /* 192, 192, 192 */
//#define TFT_SKYBLUE     0x867D      /* 135, 206, 235 */
//#define TFT_VIOLET      0x915C      /* 180,  46, 226 */
//
//// Next is a special 16 bit colour value that encodes to 8 bits
//// and will then decode back to the same 16 bit value.
//// Convenient for 8 bit and 16 bit transparent sprites.
//#define TFT_TRANSPARENT 0x0120 // This is actually a dark green
//
//// Default palette for 4 bit colour sprites
//static const uint16_t default_4bit_palette[] PROGMEM = {
//    TFT_BLACK,    //  0  ^
//    TFT_BROWN,    //  1  |
//    TFT_RED,      //  2  |
//    TFT_ORANGE,   //  3  |
//    TFT_YELLOW,   //  4  Colours 0-9 follow the resistor colour code!
//    TFT_GREEN,    //  5  |
//    TFT_BLUE,     //  6  |
//    TFT_PURPLE,   //  7  |
//    TFT_DARKGREY, //  8  |
//    TFT_WHITE,    //  9  v
//    TFT_CYAN,     // 10  Blue+green mix
//    TFT_MAGENTA,  // 11  Blue+red mix
//    TFT_MAROON,   // 12  Darker red colour
//    TFT_DARKGREEN,// 13  Darker green colour
//    TFT_NAVY,     // 14  Darker blue colour
//    TFT_PINK      // 15
//};
//*/
