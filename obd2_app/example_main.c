/**
 * @file
 * @brief Example OBD II Application
 * 
 * This example demonstrates how to use the OBD II module to:
 * 1. Connect to an OBD II adapter via USB
 * 2. Test connection with AT commands
 * 3. Request vehicle parameters (RPM, Speed, Temperature, Fuel Level)
 * 
 * Usage:
 * - Connect USB-OBD II adapter to computer
 * - Verify it appears as /dev/ttyUSB0 or /dev/ttyACM0
 * - Run: make -C obd2_app
 * - Run application: ./bin/native64/iot_app.elf
 * 
 * @author Your Name
 * @date January 2026
 */

#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "obd2.h"

/**
 * @brief Main application loop
 */
int main(void)
{
    printf("\n=== OBD II Serial Communication Example ===\n");
    printf("This example shows how to connect to an OBD II adapter\n");
    printf("via USB serial port on native64 platform.\n\n");
    
    /* Define the serial port - change based on your system */
    const char *serial_port = "/dev/ttyUSB0";  /* Typical USB adapter */
    uint32_t baudrate = 9600;                   /* Standard OBD II baudrate */
    
    printf("[MAIN] Attempting to connect to OBD II adapter at %s\n", serial_port);
    
    /* Initialize OBD II connection */
    if (obd2_init(serial_port, baudrate) != 0) {
        printf("[MAIN] Failed to initialize OBD II adapter\n");
        printf("[MAIN] Try:\n");
        printf("       - Check if adapter is connected: lsusb\n");
        printf("       - Check port: ls -la /dev/ttyUSB* /dev/ttyACM*\n");
        printf("       - Check permissions: sudo usermod -a -G dialout $USER\n");
        return 1;
    }
    
    /* Test connection */
    printf("\n[MAIN] Testing connection...\n");
    if (!obd2_test_connection()) {
        printf("[MAIN] Connection test failed!\n");
        printf("[MAIN] Possible issues:\n");
        printf("       - Wrong serial port\n");
        printf("       - OBD II adapter not responding\n");
        printf("       - Wrong baudrate\n");
        obd2_close();
        return 1;
    }
    
    printf("[MAIN] Connection successful!\n\n");
    
    /* Initialize OBD II (send setup commands) */
    printf("[MAIN] Initializing OBD II protocol...\n");
    obd2_write((uint8_t *)"ATZ\r", 4);     /* Reset adapter */
    usleep(500000);
    
    obd2_write((uint8_t *)"ATE0\r", 5);    /* Echo off */
    usleep(200000);
    
    obd2_write((uint8_t *)"ATH1\r", 5);    /* Headers on */
    usleep(200000);
    
    obd2_write((uint8_t *)"ATL0\r", 5);    /* Line feeds off */
    usleep(200000);
    
    obd2_write((uint8_t *)"ATSP6\r", 6);   /* Select protocol 6 (ISO 15765-4, CAN 11-bit) */
    usleep(200000);
    
    printf("[MAIN] OBD II ready!\n\n");
    
    /* Request vehicle parameters */
    printf("=== Vehicle Parameters ===\n");
    
    /* Engine RPM */
    uint16_t rpm = 0;
    if (obd2_get_rpm(&rpm) == 0) {
        printf("Engine RPM: %u\n", rpm);
    } else {
        printf("Failed to get Engine RPM\n");
    }
    
    /* Vehicle Speed */
    uint8_t speed = 0;
    if (obd2_get_speed(&speed) == 0) {
        printf("Vehicle Speed: %u km/h\n", speed);
    } else {
        printf("Failed to get Vehicle Speed\n");
    }
    
    /* Coolant Temperature */
    int8_t temp = 0;
    if (obd2_get_coolant_temp(&temp) == 0) {
        printf("Coolant Temperature: %d °C\n", temp);
    } else {
        printf("Failed to get Coolant Temperature\n");
    }
    
    /* Fuel Level */
    uint8_t fuel = 0;
    if (obd2_get_fuel_level(&fuel) == 0) {
        printf("Fuel Level: %u %%\n", fuel);
    } else {
        printf("Failed to get Fuel Level\n");
    }
    
    /* Engine Load */
    uint8_t load = 0;
    if (obd2_get_engine_load(&load) == 0) {
        printf("Engine Load: %u %%\n", load);
    } else {
        printf("Failed to get Engine Load\n");
    }
    
    printf("\n=== Continuous Reading Loop ===\n");
    printf("Press Ctrl+C to exit\n\n");
    
    /* Continuous loop - read data every 2 seconds */
    int iteration = 1;
    while (1) {
        printf("--- Iteration %d ---\n", iteration);
        
        if (obd2_get_rpm(&rpm) == 0) {
            printf("RPM: %u\n", rpm);
        }
        
        if (obd2_get_speed(&speed) == 0) {
            printf("Speed: %u km/h\n", speed);
        }
        
        printf("\n");
        sleep(2);  /* Wait 2 seconds before next read */
        iteration++;
    }
    
    /* Cleanup (unreachable in infinite loop, but good practice) */
    obd2_close();
    
    return 0;
}
