#pragma once

///////////////////////////////////////////////////////////////////////////////
// ESP32 (ESP32-WROOM-32E-N4)
#if CONFIG_IDF_TARGET_ESP32
#define PIN_BOOT GPIO_NUM_0       // swBoot
#define PIN_RX0 GPIO_NUM_1        // RX0 (Arduino)
#define PIN_LED_RED GPIO_NUM_2    // ledRed
#define PIN_TX0 GPIO_NUM_3        // TX0 (Arduino)
#define PIN_RF_DIO1 GPIO_NUM_4    // rfDIO1
#define PIN_RF_RST GPIO_NUM_5     // rfRST
#define PIN_HMISO GPIO_NUM_12     // hMISO/ethMISO
#define PIN_HMOSI GPIO_NUM_13     // hMOSI/ethMOSI
#define PIN_HSCK GPIO_NUM_14      // hSCK/ethSCK
#define PIN_LED_GREEN GPIO_NUM_15 // ledGreen
#define PIN_RX2 GPIO_NUM_16       // RX2 (debugLog)
#define PIN_TX2 GPIO_NUM_17       // TX2 (debugLog)
#define PIN_VSCK GPIO_NUM_18      // vSCK/rfSCK
#define PIN_VMISO GPIO_NUM_19     // vMISO/rfMISO
#define PIN_RF_BUSY GPIO_NUM_21   // rfBUSY
#define PIN_RF_NSS GPIO_NUM_22    // rfNSS
#define PIN_VMOSI GPIO_NUM_23     // vMOSI/rfMOSI
#define PIN_CHRG_OUT GPIO_NUM_25  // chrgOut
#define PIN_ETH_CS GPIO_NUM_26    // ethCS
#define PIN_LED_BLUE GPIO_NUM_27  // ledBlue
#define PIN_ETH_RST GPIO_NUM_32   // ethRST
#define PIN_ETH_INT GPIO_NUM_33   // ethINT

#define PIN_SW_USER GPIO_NUM_39  // swUser
#define PIN_CHRG_IN GPIO_NUM_36  // chrgIn
#define PIN_VBUS_DET GPIO_NUM_34 // vbusDetect
#define PIN_LOW_BATT GPIO_NUM_35 // lowBatt

///////////////////////////////////////////////////////////////////////////////
// ESP32S3 (ESP32S3-WROOM-N4R2)
#elif CONFIG_IDF_TARGET_ESP32S3
#define PIN_BOOT GPIO_NUM_0       // swBoot
#define PIN_SW_USR1 GPIO_NUM_1    // swUsr1
#define PIN_SW_USR2 GPIO_NUM_2    // swUsr2
#define PIN_LED_GREEN GPIO_NUM_3  // ledBlue (ledRf)
#define PIN_CHRG_IN GPIO_NUM_4    // vbusDetect
#define PIN_CHRG_OUT GPIO_NUM_5   // chrgOut
#define PIN_VBUS_DET GPIO_NUM_6   // chrg (chrgIn)
#define PIN_LOW_BATT GPIO_NUM_7   // lowBatt
#define PIN_ETH_RST GPIO_NUM_8    // ethRST
#define PIN_ETH_INT GPIO_NUM_9    // ethINT
#define PIN_ETH_CS GPIO_NUM_10    // ethCS
#define PIN_HMOSI GPIO_NUM_11     // hMOSI/ethMOSI
#define PIN_HSCK GPIO_NUM_12      // hSCK/ethSCK
#define PIN_HMISO GPIO_NUM_13     // hMISO/ethMISO
#define PIN_RF_LDO_EN GPIO_NUM_14 // rfLdoEN
#define PIN_X32K_P GPIO_NUM_15    // x32k_p
#define PIN_X32K_N GPIO_NUM_16    // x32k_n
#define PIN_TX1 GPIO_NUM_17       // TX1 (debugLog)
#define PIN_RX1 GPIO_NUM_18       // RX1 (debugLog)
#define PIN_USB_DN GPIO_NUM_19    // usbD-
#define PIN_USB_DP GPIO_NUM_20    // usbD+
#define PIN_RF_DIO1 GPIO_NUM_21   // rfDIO1
#define PIN_VMOSI GPIO_NUM_35     // vMOSI/rfMOSI
#define PIN_VSCK GPIO_NUM_36      // vSCK/rfSCK
#define PIN_VMISO GPIO_NUM_37     // vMISO/rfMISO
#define PIN_RF_BUSY GPIO_NUM_38   // rfRST
// #define PIN_ GPIO_NUM_39  //
// #define PIN_ GPIO_NUM_40  //
// #define PIN_ GPIO_NUM_41  //
#define PIN_USB_ID GPIO_NUM_42    // usbID
#define PIN_LED_BLUE GPIO_NUM_45  // ledGreen (ledEth)
#define PIN_LED_RED GPIO_NUM_46   // ledRed
#define PIN_RF_RST GPIO_NUM_47    // rfBUSY
#define PIN_RF_NSS GPIO_NUM_48    // rfNSS

///////////////////////////////////////////////////////////////////////////////
// ESP32C3 (ESP32-C3-WROOM-02-N4)
#elif CONFIG_IDF_TARGET_ESP32C3
// ledRed?
#define PIN_LOW_BATT GPIO_NUM_0  // lowBatt
#define PIN_LED_GREEN GPIO_NUM_1 // ledGreen
#define PIN_RF_RST GPIO_NUM_2    // rfRST (strapping, pull-up)
#define PIN_RF_DIO1 GPIO_NUM_3   // rfDIO1
#define PIN_RF_BUSY GPIO_NUM_4   // rfBUSY
#define PIN_RF_NSS GPIO_NUM_5    // rfNSS
#define PIN_SW_USER GPIO_NUM_6   // swUser
#define PIN_MOSI GPIO_NUM_7      // vMOSI/rfMOSI
#define PIN_MISO GPIO_NUM_8      // vMISO/rfMISO (strapping)
#define PIN_BOOT GPIO_NUM_9      // swBoot (strapping, pull-up)
#define PIN_SCK GPIO_NUM_10      // vSCK/rfSCK
#define PIN_USB_DN GPIO_NUM_18   // usbD-
#define PIN_USB_DP GPIO_NUM_19   // usbD+
#define PIN_CHRG_IN GPIO_NUM_20  // chrgIn (RX)
#define PIN_CHRG_OUT GPIO_NUM_21 // chrgOut (TX)

#endif
