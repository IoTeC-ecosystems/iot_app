/**
 * @file
 * @brief Simple OBD II Test
 * 
 * This is a minimal example to test if OBD II communication works.
 * It only tests connection without reading actual parameters.
 * 
 * Usage: make -C obd2_app && ./bin/native64/iot_app.elf
 */

#include <stdio.h>
#include <unistd.h>

#include "obd2.h"

int main(void)
{
    printf("\n=== OBD II Simple Connection Test ===\n\n");
    
    const char *port = "/dev/ttyUSB0";
    
    printf("Connecting to %s...\n", port);
    if (obd2_init(port, 9600) != 0) {
        printf("FAILED: Could not open port\n");
        printf("Hint: Check if device is at /dev/ttyUSB0 or /dev/ttyACM0\n");
        printf("      lsusb\n");
        printf("      ls /dev/ttyUSB* /dev/ttyACM*\n");
        return 1;
    }
    
    printf("OK: Port opened\n");
    printf("Testing connection with AT command...\n");
    
    if (obd2_test_connection()) {
        printf("SUCCESS: OBD II adapter is responding!\n");
        printf("\nNext steps:\n");
        printf("1. Make sure car is turned on\n");
        printf("2. Modify code to request specific PIDs\n");
        printf("3. See example_main.c for full example\n");
    } else {
        printf("FAILED: Adapter not responding\n");
        printf("Check:\n");
        printf("- Adapter is powered (lights on?)\n");
        printf("- Car is turned on\n");
        printf("- Correct serial port and baudrate\n");
    }
    
    obd2_close();
    return 0;
}
