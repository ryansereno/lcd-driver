
import serial
import time
import getch

import ollama

stream = ollama.chat(
    model="llama3:latest",
    messages=[{"role": "user", "content": "Why is the sky blue?"}],
    stream=True,
)

for chunk in stream:
    print(chunk["message"]["content"], end="", flush=True)


ser = serial.Serial("/dev/tty.usbmodem101", 9600, timeout=1)
time.sleep(2)

print("Start typing (press 'Esc' to quit):")

while True:
    char = getch.getch()

    if ord(char) == 27:
        break

    ser.write(char.encode())
    print(char, end="", flush=True)

    received = ser.read().decode()
    if received:
        print(f"\nEchoed back: {received}")

ser.close()
print("\nConnection closed.")
