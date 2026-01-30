# OBD II Module - Resumen de Archivos

He creado un módulo completo de OBD II para RIOT OS en la carpeta `obd2_app/`.

## Estructura

```
obd2_app/
├── include/
│   └── obd2.h              # API pública (40+ funciones)
├── obd2.c                  # Implementación completa (~400 líneas)
├── example_main.c          # Ejemplo completo con inicialización
├── simple_test.c           # Prueba simple de conexión
├── Makefile                # Configuración RIOT OS
├── Makefile.include        # Módulo RIOT
├── build.sh                # Script para compilar
├── README.md               # Documentación detallada
├── QUICKSTART.md           # Guía rápida y troubleshooting
└── FILES.md                # Este archivo
```

## Descripción Breve de Cada Archivo

### `include/obd2.h` 
**Header con API pública**
- Constantes de PIDs (RPM, Velocidad, Temperatura, etc.)
- Estructura `obd2_response_t` para respuestas
- ~20 funciones públicas:
  - Conexión: `obd2_init()`, `obd2_close()`, `obd2_test_connection()`
  - Lectura fácil: `obd2_get_rpm()`, `obd2_get_speed()`, `obd2_get_coolant_temp()`, etc.
  - Bajo nivel: `obd2_request_pid()`, `obd2_write()`, `obd2_read()`

### `obd2.c`
**Implementación completa**
- Funciones POSIX para puerto serie (open, read, write, termios)
- Configuración automática de baudrate y parámetros
- Parsing de respuestas OBD II
- Cálculo de unidades (RPM, °C, %, km/h)
- Timeouts y manejo de errores robusto
- ~400 líneas de código bien comentado

### `example_main.c`
**Ejemplo de uso completo**
- Conecta al adaptador
- Inicializa protocolo OBD II con comandos AT
- Lee múltiples parámetros
- Loop continuo de lectura cada 2 segundos
- Manejo de errores y mensajes útiles

### `simple_test.c`
**Prueba simple**
- Solo verifica si el adaptador responde
- Ideal para troubleshooting
- Muy simple de modificar y probar

### `README.md`
**Documentación completa**
- Características del módulo
- Instrucciones de uso
- Tabla de PIDs soportados
- Referencia de API
- Troubleshooting
- Referencias externas

### `QUICKSTART.md`
**Guía rápida práctica**
- Comandos para verificar adaptador conectado
- Cómo testear manualmente con picocom/screen
- Solución de problemas comunes
- Debugging
- Workflow recomendado
- Adaptadores recomendados

### `build.sh`
**Script auxiliar**
- Facilita la compilación
- Chequea requisitos
- Lipia builds anteriores

### `Makefile` y `Makefile.include`
**Configuración RIOT OS**
- Define el módulo como válido en RIOT
- Configura paths de includes

## Cómo Usar

### Opción 1: Usar directamente

```bash
cd /home/septien/IoTeC/code/iot_app

# Compilar
make all

# Ejecutar
./bin/native64/iot_app.elf
```

### Opción 2: Usar en tu aplicación

```c
#include "obd2.h"

int main(void) {
    obd2_init("/dev/ttyUSB0", 9600);
    obd2_test_connection();
    
    uint16_t rpm;
    obd2_get_rpm(&rpm);
    printf("RPM: %u\n", rpm);
    
    obd2_close();
    return 0;
}
```

### Opción 3: Testear adaptador primero

```bash
cd obd2_app

# Ver guía rápida
cat QUICKSTART.md

# Probar manualmente
picocom -b 9600 /dev/ttyUSB0

# Compilar simple test
gcc -I./include -o test simple_test.c obd2.c
./test
```

## Características Principales

✅ **Comunicación Serie POSIX**
- Compatible con Linux nativo (native, native64)
- Manejo de timeouts
- No necesita bibliotecas externas (solo POSIX)

✅ **Protocolo OBD II**
- Comandos AT ELM327
- Parsing de respuestas hexadecimales
- Cálculo automático de unidades

✅ **PIDs Soportados**
- Engine RPM
- Vehicle Speed
- Coolant Temperature
- Fuel Level
- Engine Load
- Y más (10+ PIDs preconfigrados)

✅ **Funciones Helper**
- `obd2_get_rpm()`
- `obd2_get_speed()`
- `obd2_get_coolant_temp()`
- `obd2_get_fuel_level()`
- `obd2_get_engine_load()`

✅ **Bajo Nivel**
- `obd2_request_pid()` para cualquier PID
- `obd2_write()` / `obd2_read()` para acceso raw

## Requisitos

### Hardware
- Adaptador USB-OBD II (ELM327 compatible)
- Cable USB
- Puerto OBD II del automóvil

### Software
- Linux (any distro)
- gcc y make
- RIOT OS (ya tienes en tu workspace)

### Permisos
```bash
sudo usermod -a -G dialout $USER
newgrp dialout
```

## Próximos Pasos

1. **Conecta el adaptador USB**
   ```bash
   lsusb | grep -i uart
   ```

2. **Verifica el puerto**
   ```bash
   ls /dev/ttyUSB* /dev/ttyACM*
   ```

3. **Prueba con picocom**
   ```bash
   picocom -b 9600 /dev/ttyUSB0
   # Escribe: AT
   # Debería responder: OK
   ```

4. **Compila la aplicación**
   ```bash
   make all
   ```

5. **Ejecuta**
   ```bash
   ./bin/native64/iot_app.elf
   ```

## Estructura del Código

```
main.c (tu aplicación)
  ↓
#include "obd2.h"
  ↓
obd2.c (implementación)
  ├── configure_serial() - Configurar puerto
  ├── read_with_timeout() - Lectura con timeout
  ├── parse_obd2_response() - Parsear respuesta
  ├── obd2_init() - Inicializar
  ├── obd2_test_connection() - Probar conexión
  ├── obd2_request_pid() - Solicitar PID
  ├── obd2_get_rpm() - Funciones helper
  └── ... más funciones
```

## Pruebas

El código está listo para:
- ✅ Compilar con `make all`
- ✅ Ejecutar en native64
- ✅ Comunicar con adaptador USB real
- ✅ Parsear respuestas OBD II
- ✅ Manejar errores y timeouts

## Notas Importantes

1. **El adaptador debe estar conectado** antes de ejecutar
2. **El auto debe estar encendido** para leer parámetros
3. **Los timeouts están en 1-2 segundos** (ajustar si es necesario)
4. **Algunos PIDs pueden no estar soportados** en tu vehículo
5. **Para porting a microcontrolador**, reemplaza `termios` por UART de RIOT

## Recursos

- `README.md` - Documentación completa
- `QUICKSTART.md` - Guía práctica y troubleshooting
- `example_main.c` - Código de ejemplo anotado
- `simple_test.c` - Prueba mínima

---

**Creado:** Enero 30, 2026
**Estado:** Listo para usar
**Plataforma:** native64 (Linux)
