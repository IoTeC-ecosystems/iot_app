#ifndef OBD2_H
#define OBD2_H

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief OBD II Module Interface
 * 
 * Provides functions to communicate with OBD II adapters via serial port
 * (USB-OBD II adapters or direct UART connection)
 */

/* OBD II PID Codes */
#define OBD2_PID_ENGINE_LOAD        0x04  /**< Engine Load (%) */
#define OBD2_PID_COOLANT_TEMP       0x05  /**< Coolant Temperature (°C) */
#define OBD2_PID_FUEL_PRESSURE      0x0A  /**< Fuel Pressure (kPa) */
#define OBD2_PID_ENGINE_RPM         0x0C  /**< Engine RPM */
#define OBD2_PID_VEHICLE_SPEED      0x0D  /**< Vehicle Speed (km/h) */
#define OBD2_PID_INTAKE_AIR_TEMP    0x0F  /**< Intake Air Temperature (°C) */
#define OBD2_PID_MAF_FLOW           0x10  /**< MAF Air Flow (g/s) */
#define OBD2_PID_THROTTLE_POS       0x11  /**< Throttle Position (%) */
#define OBD2_PID_FUEL_LEVEL         0x2F  /**< Fuel Level (%) */
#define OBD2_PID_DISTANCE           0x31  /**< Distance Since Codes Cleared (km) */

/* OBD II Response Structure */
typedef struct {
    uint8_t pid;           /**< PID requested */
    uint32_t value;        /**< Raw value from ECU */
    bool valid;            /**< True if response is valid */
    char unit[16];         /**< Unit of measurement */
} obd2_response_t;

/**
 * @brief Initialize OBD II adapter
 * 
 * @param port Serial port path (e.g., "/dev/ttyUSB0", "/dev/ttyACM0")
 * @param baudrate Baud rate (typically 9600 or 38400)
 * @return int 0 on success, negative on error
 */
int obd2_init(const char *port, uint32_t baudrate);

/**
 * @brief Close OBD II connection
 * 
 * @return int 0 on success, negative on error
 */
int obd2_close(void);

/**
 * @brief Test connection with AT commands
 * 
 * @return true if adapter responds correctly
 */
bool obd2_test_connection(void);

/**
 * @brief Request a PID from the ECU
 * 
 * @param pid PID code to request
 * @param response Pointer to response structure
 * @return int 0 on success, negative on error
 */
int obd2_request_pid(uint8_t pid, obd2_response_t *response);

/**
 * @brief Get Engine RPM
 * 
 * @param rpm Pointer to store RPM value
 * @return int 0 on success, negative on error
 */
int obd2_get_rpm(uint16_t *rpm);

/**
 * @brief Get Vehicle Speed
 * 
 * @param speed Pointer to store speed (km/h)
 * @return int 0 on success, negative on error
 */
int obd2_get_speed(uint8_t *speed);

/**
 * @brief Get Coolant Temperature
 * 
 * @param temp Pointer to store temperature (°C)
 * @return int 0 on success, negative on error
 */
int obd2_get_coolant_temp(int8_t *temp);

/**
 * @brief Get Fuel Level
 * 
 * @param level Pointer to store fuel level (%)
 * @return int 0 on success, negative on error
 */
int obd2_get_fuel_level(uint8_t *level);

/**
 * @brief Get Engine Load
 * 
 * @param load Pointer to store engine load (%)
 * @return int 0 on success, negative on error
 */
int obd2_get_engine_load(uint8_t *load);

/**
 * @brief Raw serial write
 * 
 * @param data Buffer with data to send
 * @param len Length of data
 * @return int Number of bytes written, negative on error
 */
int obd2_write(const uint8_t *data, size_t len);

/**
 * @brief Raw serial read
 * 
 * @param buffer Buffer to store received data
 * @param max_len Maximum buffer length
 * @return int Number of bytes read, negative on error
 */
int obd2_read(uint8_t *buffer, size_t max_len);

#endif // OBD2_H
