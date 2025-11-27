cd software

make clean

make spaceinv F_CLK=50000000

make serial SERIAL_DEV=/dev/ttyUSB1


cat /dev/ttyUSB1

CPU reset

make load SERIAL_DEV=/dev/ttyUSB1
