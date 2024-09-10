import serial
import serial.tools
import serial.tools.list_ports

print(serial.tools.list_ports)

# Open port at “9600,8,N,1”, no timeout:

# >>> import serial
# >>> ser = serial.Serial('/dev/ttyUSB0')  # open serial port
# >>> print(ser.name)         # check which port was really used
# >>> ser.write(b'hello')     # write a string
# >>> ser.close()             # close port

# Open named port at “19200,8,N,1”, 1s timeout:

# >>> with serial.Serial('/dev/ttyS1', 19200, timeout=1) as ser:
# ...     x = ser.read()          # read one byte
# ...     s = ser.read(10)        # read up to ten bytes (timeout)
# ...     line = ser.readline()   # read a '\n' terminated line

