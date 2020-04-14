This example builds on project BluePillDemo_I2C_Pressure_BMP280. It uses the same code to
drive the same pressure sensor but this time it wraps the driving of the I2C device in a
FreeRTOS task. A second task communicates via a queue and receives pressure and
temperature readings.

The dricing of the I2C device is the same as project BluePillDemo_I2C_Pressure_BMP280. See
that project for details.

The task that drives the I2C pressure sensor firstly reads the calibration values from the
sensor. Then it signals the main task that it is ready. Periodically this task initiates a
sensor reading and then after a delay reads the results. Ther temperature and pressure are
calculated (same as BluePillDemo_I2C_Pressure_BMP280 project) and the values are then 
inserted in a queue with a queue length of 1.

The main task periodically reads from this queue. If there is a value to read the queue read
function returns ok and the queued item is removed.

Schematic provided.