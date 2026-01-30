# Guía Rápida OBD II

## 1. Verificar Adaptador Conectado

```bash
# Listar dispositivos USB
lsusb | grep -i uart

# Ver puertos serie disponibles
ls -la /dev/ttyUSB* /dev/ttyACM*

# Ver dónde se conectó
dmesg | tail -20

# Ver en tiempo real
watch -n 1 'ls -la /dev/ttyUSB* /dev/ttyACM* 2>/dev/null || echo "No devices"'
```

## 2. Probar Adaptador Manualmente

```bash
# Instalar herramientas
sudo apt-get install screen minicom picocom

# Opción 1: Screen (simple)
screen /dev/ttyUSB0 9600

# Opción 2: Picocom (mejor)
picocom -b 9600 /dev/ttyUSB0

# Opción 3: Minicom (avanzado)
minicom -D /dev/ttyUSB0 -b 9600
```

### Comandos AT para Testear

Una vez conectado, prueba estos comandos:

```
AT          → Debería responder OK
ATZ         → Reset, debería responder OK
ATE0        → Echo off, debería responder OK
ATH0        → Headers off, debería responder OK
ATSP0       → Show protocols, debería listar protocolos
ATDP        → Describe protocol, debería mostrar protocolo actual
```

Para salir: Ctrl+A, luego X (screen) o Ctrl+A, Q (minicom)

## 3. Compilar OBD II Module

```bash
cd /home/septien/IoTeC/code/iot_app

# Compile sin usar Docker
make all

# O con Docker
BUILD_IN_DOCKER=1 make all

# Limpiar
make clean
```

## 4. Ejecutar

```bash
# Run de la aplicación
./bin/native64/iot_app.elf

# Con salida de debug
./bin/native64/iot_app.elf 2>&1 | tee output.log

# En background
./bin/native64/iot_app.elf > obd.log 2>&1 &
```

## 5. Solucionar Problemas

### Error: "Error opening port: Permission denied"

```bash
# Opción 1: Cambiar permisos
sudo chmod 666 /dev/ttyUSB0

# Opción 2: Agregar usuario a grupo dialout (mejor)
sudo usermod -a -G dialout $USER
newgrp dialout

# Verificar
groups $USER  # debería incluir "dialout"
```

### Error: "No such file or directory"

```bash
# El puerto no existe
# Verificar:
lsusb
ls /dev/ttyUSB* /dev/ttyACM*

# Puede ser /dev/ttyACM0 en lugar de /dev/ttyUSB0
# Cambiar en el código: obd2_init("/dev/ttyACM0", 9600)
```

### Adaptador No Responde

1. ¿Está alimentado? (LED rojo/verde encendido?)
2. ¿Está conectado al auto?
3. ¿Auto encendido?
4. ¿Baudrate correcto? (Probar 9600, 19200, 38400)
5. ¿Adaptador ELM327 compatible?

```bash
# Prueba de baudrates
for baud in 9600 19200 38400 57600 115200; do
    echo "Testing $baud..."
    timeout 1 picocom -b $baud /dev/ttyUSB0 -q
done
```

### Linux no detecta adaptador

```bash
# Ver si es FTDI o CH340
lsusb -v | grep -A 10 "UART\|Serial"

# Si no aparece, driver issue:
# Para CH340: sudo apt-get install ch340-dkms
# Para FTDI: debería funcionar automáticamente
```

## 6. Debugging en Código

Modificar `obd2.c` para más información:

```c
// Al inicio de funciones
printf("[OBD2_DEBUG] Entering obd2_init\n");

// Para ver bytes enviados
for (int i = 0; i < len; i++) {
    printf("0x%02X ", data[i]);
}
printf("\n");

// Para ver respuestas
printf("[OBD2] Raw response: ");
for (int i = 0; i < n; i++) {
    printf("%02X ", buffer[i]);
}
printf("\n");
```

## 7. Workflow Recomendado

1. **Probar adaptador primero con screen/picocom**
   ```bash
   picocom -b 9600 /dev/ttyUSB0
   ```

2. **Usar simple_test.c para verificar conexión**
   ```c
   // Compilar simple_test.c en lugar de example_main.c
   gcc -I./obd2_app/include -o test obd2_app/simple_test.c obd2_app/obd2.c
   ./test
   ```

3. **Si simple funciona, usar example_main.c completo**
   ```bash
   make all
   ./bin/native64/iot_app.elf
   ```

## 8. Adaptadores Recomendados

- **ELM327 V1.5** (clásico, $15-30)
  - Estándar de facto
  - Compatible con Linux
  - Soporta todos los protocolos

- **KONNWEI KW903** (moderno, $20-40)
  - Mejora en V1.5
  - Bluetooth + USB
  - Respuestas más rápidas

- **Vgate iCar2** (premium, $40-60)
  - Bluetooth inteligente
  - App móvil

## 9. Protocolos Soportados

Los adaptadores ELM327 soportan:

- SAE J1850 PWM (10.4 kbaud)
- SAE J1850 VPW (41.6 kbaud)
- ISO 9141-2 (5 baud init, 10.4 kbaud)
- ISO 14230-4 (5 baud init, 10.4 kbaud)
- ISO 15765-4 (CAN 11-bit, 29-bit)

Seleccionar con: `ATSP6` (Protocol 6 = ISO 15765-4 CAN 11-bit)

## 10. Lectura Adicional

- [ELM327 Reference](https://www.elmelectronics.com/DM41D.pdf)
- [OBD II Modes](https://en.wikipedia.org/wiki/OBD-II_PID#Modes)
- [RIOT OS Serial](https://doc.riot-os.org/)

---

**Última actualización:** Enero 2026
