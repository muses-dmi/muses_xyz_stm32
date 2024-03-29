/**
 * 
 */
#pragma once

#define SENSEL_REG_MAP_PROTOCOL_VERSION 1
#define SENSEL_REG_MAP_MAJOR_VERSION 0
#define SENSEL_REG_MAP_MINOR_VERSION 7
#define SENSEL_REG_MAP_BUILD_VERSION 60

// Sensel Register Addresses:

#define SENSEL_REG_MAGIC                                    0x00
#define SENSEL_REG_FW_VERSION_PROTOCOL                      0x06
#define SENSEL_REG_FW_VERSION_MAJOR                         0x07
#define SENSEL_REG_FW_VERSION_MINOR                         0x08
#define SENSEL_REG_FW_VERSION_BUILD                         0x09
#define SENSEL_REG_FW_VERSION_RELEASE                       0x0B
#define SENSEL_REG_DEVICE_ID                                0x0C
#define SENSEL_REG_DEVICE_REVISION                          0x0E
#define SENSEL_REG_DEVICE_SERIAL_NUMBER                     0x0F
#define SENSEL_REG_SENSOR_NUM_COLS                          0x10
#define SENSEL_REG_SENSOR_NUM_ROWS                          0x12
#define SENSEL_REG_SENSOR_ACTIVE_AREA_WIDTH_UM              0x14
#define SENSEL_REG_SENSOR_ACTIVE_AREA_HEIGHT_UM             0x18
#define SENSEL_REG_COMPRESSION_METADATA                     0x1C
#define SENSEL_REG_SCAN_FRAME_RATE                          0x20
#define SENSEL_REG_SCAN_BUFFER_CONTROL                      0x22
#define SENSEL_REG_SCAN_DETAIL_CONTROL                      0x23
#define SENSEL_REG_FRAME_CONTENT_CONTROL                    0x24
#define SENSEL_REG_SCAN_ENABLED                             0x25
#define SENSEL_REG_SCAN_READ_FRAME                          0x26
#define SENSEL_REG_FRAME_CONTENT_SUPPORTED                  0x28
#define SENSEL_REG_CONTACTS_MAX_COUNT                       0x40
#define SENSEL_REG_CONTACTS_ENABLE_BLOB_MERGE               0x41
#define SENSEL_REG_CONTACTS_MIN_FORCE                       0x47
#define SENSEL_REG_CONTACTS_MASK                            0x4B
#define SENSEL_REG_BASELINE_ENABLED                         0x50
#define SENSEL_REG_BASELINE_INCREASE_RATE                   0x51
#define SENSEL_REG_BASELINE_DECREASE_RATE                   0x53
#define SENSEL_REG_BASELINE_DYNAMIC_ENABLED                 0x57
#define SENSEL_REG_ACCEL_X                                  0x60
#define SENSEL_REG_ACCEL_Y                                  0x62
#define SENSEL_REG_ACCEL_Z                                  0x64
#define SENSEL_REG_BATTERY_STATUS                           0x70
#define SENSEL_REG_BATTERY_PERCENTAGE                       0x71
#define SENSEL_REG_POWER_BUTTON_PRESSED                     0x72
#define SENSEL_REG_LED_BRIGHTNESS                           0x80
#define SENSEL_REG_LED_BRIGHTNESS_SIZE                      0x81
#define SENSEL_REG_LED_BRIGHTNESS_MAX                       0x82
#define SENSEL_REG_LED_COUNT                                0x84
#define SENSEL_REG_UNIT_SHIFT_DIMS                          0xA0
#define SENSEL_REG_UNIT_SHIFT_FORCE                         0xA1
#define SENSEL_REG_UNIT_SHIFT_AREA                          0xA2
#define SENSEL_REG_UNIT_SHIFT_ANGLE                         0xA3
#define SENSEL_REG_UNIT_SHIFT_TIME                          0xA4
#define SENSEL_REG_DEVICE_OPEN                              0xD0
#define SENSEL_REG_SOFT_RESET                               0xE0
#define SENSEL_REG_ERROR_CODE                               0xEC


// Sensel Register Sizes:

#define SENSEL_REG_SIZE_MAGIC                               6
#define SENSEL_REG_SIZE_FW_VERSION_PROTOCOL                 1
#define SENSEL_REG_SIZE_FW_VERSION_MAJOR                    1
#define SENSEL_REG_SIZE_FW_VERSION_MINOR                    1
#define SENSEL_REG_SIZE_FW_VERSION_BUILD                    2
#define SENSEL_REG_SIZE_FW_VERSION_RELEASE                  1
#define SENSEL_REG_SIZE_DEVICE_ID                           2
#define SENSEL_REG_SIZE_DEVICE_REVISION                     1
#define SENSEL_REG_SIZE_DEVICE_SERIAL_NUMBER                1
#define SENSEL_REG_SIZE_SENSOR_NUM_COLS                     2
#define SENSEL_REG_SIZE_SENSOR_NUM_ROWS                     2
#define SENSEL_REG_SIZE_SENSOR_ACTIVE_AREA_WIDTH_UM         4
#define SENSEL_REG_SIZE_SENSOR_ACTIVE_AREA_HEIGHT_UM        4
#define SENSEL_REG_SIZE_SCAN_FRAME_RATE                     2
#define SENSEL_REG_SIZE_SCAN_BUFFER_CONTROL                 1
#define SENSEL_REG_SIZE_SCAN_DETAIL_CONTROL                 1
#define SENSEL_REG_SIZE_FRAME_CONTENT_CONTROL               1
#define SENSEL_REG_SIZE_SCAN_ENABLED                        1
#define SENSEL_REG_SIZE_SCAN_READ_FRAME                     1
#define SENSEL_REG_SIZE_FRAME_CONTENT_SUPPORTED             1
#define SENSEL_REG_SIZE_CONTACTS_MAX_COUNT                  1
#define SENSEL_REG_SIZE_CONTACTS_ENABLE_BLOB_MERGE          1
#define SENSEL_REG_SIZE_CONTACTS_MIN_FORCE                  2
#define SENSEL_REG_SIZE_CONTACTS_MASK                       1
#define SENSEL_REG_SIZE_BASELINE_ENABLED                    1
#define SENSEL_REG_SIZE_BASELINE_INCREASE_RATE              2
#define SENSEL_REG_SIZE_BASELINE_DECREASE_RATE              2
#define SENSEL_REG_SIZE_BASELINE_DYNAMIC_ENABLED            1
#define SENSEL_REG_SIZE_ACCEL_X                             2
#define SENSEL_REG_SIZE_ACCEL_Y                             2
#define SENSEL_REG_SIZE_ACCEL_Z                             2
#define SENSEL_REG_SIZE_BATTERY_STATUS                      1
#define SENSEL_REG_SIZE_BATTERY_PERCENTAGE                  1
#define SENSEL_REG_SIZE_POWER_BUTTON_PRESSED                1
#define SENSEL_REG_SIZE_LED_BRIGHTNESS                      1
#define SENSEL_REG_SIZE_LED_BRIGHTNESS_SIZE                 1
#define SENSEL_REG_SIZE_LED_BRIGHTNESS_MAX                  2
#define SENSEL_REG_SIZE_LED_COUNT                           1
#define SENSEL_REG_SIZE_UNIT_SHIFT_DIMS                     1
#define SENSEL_REG_SIZE_UNIT_SHIFT_FORCE                    1
#define SENSEL_REG_SIZE_UNIT_SHIFT_AREA                     1
#define SENSEL_REG_SIZE_UNIT_SHIFT_ANGLE                    1
#define SENSEL_REG_SIZE_UNIT_SHIFT_TIME                     1
#define SENSEL_REG_SIZE_DEVICE_OPEN                         1
#define SENSEL_REG_SIZE_SOFT_RESET                          1
#define SENSEL_REG_SIZE_ERROR_CODE                          1