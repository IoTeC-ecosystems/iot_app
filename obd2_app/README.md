# OBD II Application Module

Este módulo proporciona funcionalidad completa para comunicarse con adaptadores OBD II via puerto serie USB.

## Características

- ✅ Inicialización y configuración de puerto serie
- ✅ Comandos AT para adaptadores ELM327
- ✅ Lectura de PIDs específicos (RPM, Velocidad, Temperatura, Combustible, etc.)
- ✅ Parsing automático de respuestas OBD II
- ✅ Timeout y manejo de errores
- ✅ Funciones helper para parámetros comunes

## Archivos

```
obd2_app/
├── include/
│   └── obd2.h           # API pública
├── obd2.c               # Implementación
├── example_main.c       # Ejemplo de uso
├── Makefile             # Configuración RIOT
├── Makefile.include     # Módulo RIOT
└── README.md            # Este archivo
```

## Uso

### 1. Incluir el módulo en tu Makefile

```makefile
APPLICATION = iot_app
BOARD ?= native
RIOTBASE ?= $(CURDIR)/RIOT

USEMODULE += obd2_app
EXTERNAL_MODULE_DIRS += $(CURDIR)

include $(RIOTBASE)/Makefile.include
```

### 2. Usar en tu código

```c
#include "obd2.h"

int main(void) {
    // Conectar al adaptador OBD II
    if (obd2_init("/dev/ttyUSB0", 9600) != 0) {
        printf("Error conectando\n");
        return 1;
    }
    
    // Probar conexión
    if (!obd2_test_connection()) {
        printf("Adaptador no responde\n");
        return 1;
    }
    
    // Leer RPM
    uint16_t rpm;
    if (obd2_get_rpm(&rpm) == 0) {
        printf("RPM: %u\n", rpm);
    }
    
    // Cerrar conexión
    obd2_close();
    return 0;
}
```

## Configuración del Adaptador

### Requisitos Hardware
- Adaptador USB-OBD II (ELM327 compatible)
- Puerto OBD II del vehículo
- Cable USB

### Requisitos Software
- Linux (nativo)
- Permisos en puerto serie:
  ```bash
  sudo usermod -a -G dialout $USER
  newgrp dialout
  ```

### Detectar Adaptador
```bash
# Ver dispositivos USB
lsusb | grep UART

# Ver puertos serie
ls -la /dev/ttyUSB* /dev/ttyACM*

# Obtener información
dmesg | tail -20
```

## PIDs Soportados

| Código | Nombre                    | Unidad |
|--------|---------------------------|--------|
| 0x04   | Engine Load               | %      |
| 0x05   | Coolant Temperature       | °C     |
| 0x0A   | Fuel Pressure             | kPa    |
| 0x0C   | Engine RPM                | RPM    |
| 0x0D   | Vehicle Speed             | km/h   |
| 0x0F   | Intake Air Temperature    | °C     |
| 0x10   | MAF Air Flow              | g/s    |
| 0x11   | Throttle Position         | %      |
| 0x2F   | Fuel Level                | %      |
| 0x31   | Distance Since Codes      | km     |

## API Pública

### Conexión
- `obd2_init(port, baudrate)` - Inicializar conexión
- `obd2_close()` - Cerrar conexión
- `obd2_test_connection()` - Probar conexión

### Lectura de Parámetros
- `obd2_get_rpm(rpm)` - Obtener RPM
- `obd2_get_speed(speed)` - Obtener velocidad
- `obd2_get_coolant_temp(temp)` - Obtener temperatura
- `obd2_get_fuel_level(level)` - Obtener nivel combustible
- `obd2_get_engine_load(load)` - Obtener carga motor

### Bajo Nivel
- `obd2_request_pid(pid, response)` - Solicitar PID específico
- `obd2_write(data, len)` - Escritura serie raw
- `obd2_read(buffer, len)` - Lectura serie raw

## Ejemplo Completo

Ver `example_main.c` para un programa de ejemplo completo que:
1. Se conecta al adaptador
2. Configura el protocolo OBD II
3. Lee parámetros del vehículo
4. Muestra datos continuamente

## Troubleshooting

### "Error opening port: No such file or directory"
- Verifica que el adaptador esté conectado: `lsusb`
- Comprueba el puerto correcto: `ls /dev/ttyUSB*`

### "Connection test failed"
- El adaptador no responde a AT
- Intenta cambiar baudrate (19200, 38400)
- Verifica que el adaptador sea ELM327 compatible

### "No response for PID"
- El vehículo puede no estar encendido
- El PID puede no ser soportado por este vehículo
- Problema de comunicación CAN del vehículo

### Permisos denegados
```bash
sudo chmod 666 /dev/ttyUSB0
# O mejor:
sudo usermod -a -G dialout $USER
```

## Referencias

- [OBD II PIDs](https://en.wikipedia.org/wiki/OBD-II_PID)
- [ELM327 Commands](https://www.elmelectronics.com/DM41D.pdf)
- [RIOT OS Serial](https://doc.riot-os.org/group__sys__stdio.html)

## Notas

- Este módulo está diseñado para la plataforma `native` (Linux)
- Para portarlo a microcontroladores, reemplaza las llamadas POSIX por RIOT OS APIs
- Los timeouts están configurados para comunicación reliably, pueden ajustarse según necesidad

## Licencia

MIT License - Libre para usar en tus proyectos
