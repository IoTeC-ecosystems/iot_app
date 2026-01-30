#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/select.h>
#include <errno.h>

#include "obd2.h"

/* Static variables for serial connection */
static int serial_fd = -1;
static struct termios old_tty;

/**
 * @brief Configure serial port settings
 * 
 * @param fd File descriptor
 * @param baudrate Baud rate to set
 * @return int 0 on success, -1 on error
 */
static int configure_serial(int fd, uint32_t baudrate)
{
    struct termios tty;
    
    if (tcgetattr(fd, &tty) != 0) {
        perror("tcgetattr");
        return -1;
    }
    
    /* Save old settings for restoration */
    memcpy(&old_tty, &tty, sizeof(struct termios));
    
    /* Set baud rate */
    speed_t baud;
    switch (baudrate) {
        case 9600:   baud = B9600;   break;
        case 19200:  baud = B19200;  break;
        case 38400:  baud = B38400;  break;
        case 57600:  baud = B57600;  break;
        case 115200: baud = B115200; break;
        default:     baud = B9600;   break;
    }
    
    cfsetispeed(&tty, baud);
    cfsetospeed(&tty, baud);
    
    /* Set 8N1 (8 bits, no parity, 1 stop bit) */
    tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;  /* 8-bit chars */
    tty.c_cflag &= ~PARENB;                       /* no parity bit */
    tty.c_cflag &= ~CSTOPB;                       /* only need 1 stop bit */
    tty.c_cflag &= ~CRTSCTS;                      /* no hardware flowcontrol */
    
    tty.c_cflag |= CLOCAL | CREAD;                /* ignore modem controls, enable reading */
    tty.c_iflag &= ~(IXON | IXOFF | IXANY);       /* shut off xon/xoff ctrl */
    tty.c_iflag &= ~(ICANON | ECHO | ECHOE | ISIG); /* make raw */
    tty.c_oflag = 0;                               /* make raw */
    
    tty.c_cc[VMIN] = 0;  /* non-blocking */
    tty.c_cc[VTIME] = 5; /* 0.5 seconds timeout */
    
    if (tcsetattr(fd, TCSANOW, &tty) != 0) {
        perror("tcsetattr");
        return -1;
    }
    
    return 0;
}

/**
 * @brief Read response with timeout
 * 
 * @param buffer Buffer to store response
 * @param max_len Maximum length
 * @param timeout_ms Timeout in milliseconds
 * @return Number of bytes read, -1 on error, 0 on timeout
 */
static ssize_t read_with_timeout(uint8_t *buffer, size_t max_len, int timeout_ms)
{
    if (serial_fd < 0) {
        return -1;
    }
    
    struct timeval tv;
    fd_set readfds;
    
    FD_ZERO(&readfds);
    FD_SET(serial_fd, &readfds);
    
    tv.tv_sec = timeout_ms / 1000;
    tv.tv_usec = (timeout_ms % 1000) * 1000;
    
    int ret = select(serial_fd + 1, &readfds, NULL, NULL, &tv);
    
    if (ret <= 0) {
        return ret;  /* 0 = timeout, -1 = error */
    }
    
    ssize_t n = read(serial_fd, buffer, max_len);
    return (n < 0) ? -1 : n;
}

/**
 * @brief Parse OBD II response
 * 
 * @param response_str String response from adapter
 * @param pid PID requested
 * @param response Pointer to store parsed response
 * @return 0 on success, -1 on parse error
 */
static int parse_obd2_response(const char *response_str, uint8_t pid, obd2_response_t *response)
{
    response->pid = pid;
    response->valid = false;
    
    /* Expected format: 41 0C AABB (for RPM) */
    /* Skip the "41" (positive response to mode 01) */
    if (strlen(response_str) < 5) {
        return -1;
    }
    
    uint32_t data_a, data_b;
    
    if (sscanf(response_str + 3, "%02X%02X", &data_a, &data_b) != 2) {
        return -1;
    }
    
    response->value = (data_a << 8) | data_b;
    response->valid = true;
    
    /* Set units based on PID */
    switch (pid) {
        case OBD2_PID_ENGINE_RPM:
            response->value = (response->value / 4);
            strcpy(response->unit, "RPM");
            break;
        case OBD2_PID_VEHICLE_SPEED:
            strcpy(response->unit, "km/h");
            break;
        case OBD2_PID_COOLANT_TEMP:
            response->value = response->value - 40;
            strcpy(response->unit, "°C");
            break;
        case OBD2_PID_ENGINE_LOAD:
            response->value = (response->value * 100) / 255;
            strcpy(response->unit, "%");
            break;
        case OBD2_PID_FUEL_LEVEL:
            response->value = (response->value * 100) / 255;
            strcpy(response->unit, "%");
            break;
        default:
            strcpy(response->unit, "raw");
            break;
    }
    
    return 0;
}

/* Public API Implementation */

int obd2_init(const char *port, uint32_t baudrate)
{
    if (serial_fd >= 0) {
        obd2_close();
    }
    
    /* Open serial port */
    serial_fd = open(port, O_RDWR | O_NOCTTY | O_SYNC);
    if (serial_fd < 0) {
        printf("[OBD2] Error opening port %s: %s\n", port, strerror(errno));
        return -1;
    }
    
    /* Configure serial port */
    if (configure_serial(serial_fd, baudrate) != 0) {
        close(serial_fd);
        serial_fd = -1;
        return -1;
    }
    
    printf("[OBD2] Connected to %s at %u baud\n", port, baudrate);
    usleep(100000);  /* Wait 100ms for adapter to stabilize */
    
    return 0;
}

int obd2_close(void)
{
    if (serial_fd >= 0) {
        tcsetattr(serial_fd, TCSANOW, &old_tty);
        close(serial_fd);
        serial_fd = -1;
        printf("[OBD2] Connection closed\n");
    }
    return 0;
}

bool obd2_test_connection(void)
{
    if (serial_fd < 0) {
        printf("[OBD2] Not connected\n");
        return false;
    }
    
    /* Send AT command and wait for OK response */
    const char *cmd = "AT\r";
    if (write(serial_fd, cmd, strlen(cmd)) < 0) {
        printf("[OBD2] Error sending AT command\n");
        return false;
    }
    
    uint8_t buffer[256];
    ssize_t n = read_with_timeout(buffer, sizeof(buffer) - 1, 1000);
    
    if (n > 0) {
        buffer[n] = '\0';
        if (strstr((char *)buffer, "OK") != NULL) {
            printf("[OBD2] Connection test successful: %s", (char *)buffer);
            return true;
        }
    }
    
    printf("[OBD2] Connection test failed\n");
    return false;
}

int obd2_request_pid(uint8_t pid, obd2_response_t *response)
{
    if (serial_fd < 0) {
        printf("[OBD2] Not connected\n");
        return -1;
    }
    
    if (response == NULL) {
        return -1;
    }
    
    /* Build command: "01 PID\r" in hex format */
    char cmd[32];
    snprintf(cmd, sizeof(cmd), "0100%02X\r", pid);
    
    /* Send command */
    if (write(serial_fd, cmd, strlen(cmd)) < 0) {
        printf("[OBD2] Error sending PID request\n");
        return -1;
    }
    
    /* Read response */
    uint8_t buffer[256];
    ssize_t n = read_with_timeout(buffer, sizeof(buffer) - 1, 2000);
    
    if (n <= 0) {
        printf("[OBD2] No response for PID 0x%02X\n", pid);
        response->valid = false;
        return -1;
    }
    
    buffer[n] = '\0';
    printf("[OBD2] Response: %s\n", (char *)buffer);
    
    /* Parse response */
    if (parse_obd2_response((char *)buffer, pid, response) != 0) {
        printf("[OBD2] Failed to parse response\n");
        return -1;
    }
    
    return 0;
}

int obd2_get_rpm(uint16_t *rpm)
{
    if (rpm == NULL) return -1;
    
    obd2_response_t response;
    if (obd2_request_pid(OBD2_PID_ENGINE_RPM, &response) != 0) {
        return -1;
    }
    
    if (!response.valid) {
        return -1;
    }
    
    *rpm = (uint16_t)response.value;
    return 0;
}

int obd2_get_speed(uint8_t *speed)
{
    if (speed == NULL) return -1;
    
    obd2_response_t response;
    if (obd2_request_pid(OBD2_PID_VEHICLE_SPEED, &response) != 0) {
        return -1;
    }
    
    if (!response.valid) {
        return -1;
    }
    
    *speed = (uint8_t)response.value;
    return 0;
}

int obd2_get_coolant_temp(int8_t *temp)
{
    if (temp == NULL) return -1;
    
    obd2_response_t response;
    if (obd2_request_pid(OBD2_PID_COOLANT_TEMP, &response) != 0) {
        return -1;
    }
    
    if (!response.valid) {
        return -1;
    }
    
    *temp = (int8_t)response.value;
    return 0;
}

int obd2_get_fuel_level(uint8_t *level)
{
    if (level == NULL) return -1;
    
    obd2_response_t response;
    if (obd2_request_pid(OBD2_PID_FUEL_LEVEL, &response) != 0) {
        return -1;
    }
    
    if (!response.valid) {
        return -1;
    }
    
    *level = (uint8_t)response.value;
    return 0;
}

int obd2_get_engine_load(uint8_t *load)
{
    if (load == NULL) return -1;
    
    obd2_response_t response;
    if (obd2_request_pid(OBD2_PID_ENGINE_LOAD, &response) != 0) {
        return -1;
    }
    
    if (!response.valid) {
        return -1;
    }
    
    *load = (uint8_t)response.value;
    return 0;
}

int obd2_write(const uint8_t *data, size_t len)
{
    if (serial_fd < 0 || data == NULL) {
        return -1;
    }
    
    ssize_t n = write(serial_fd, data, len);
    return (n < 0) ? -1 : (int)n;
}

int obd2_read(uint8_t *buffer, size_t max_len)
{
    if (serial_fd < 0 || buffer == NULL) {
        return -1;
    }
    
    ssize_t n = read_with_timeout(buffer, max_len, 1000);
    return (n < 0) ? -1 : (int)n;
}
