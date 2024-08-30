import serial
import time

ser = serial.Serial("/dev/tty.usbmodem101", 9600, timeout=1)
time.sleep(2)

while True:
    char = input("Enter a character to send (or 'q' to quit): ")
    if char == "q":
        break
    ser.write(char.encode())
    received = ser.read().decode()
    print(f"Echoed back: {received}")

ser.close()
