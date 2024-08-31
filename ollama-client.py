import serial
import time
import ollama


def send_to_board(ser, text):
    for char in text:
        ser.write(char.encode())
        print(char, end="", flush=True)
        time.sleep(0.05) 
        received = ser.read().decode()
        if received:
            print(f"\nEchoed back: {received}")


ser = serial.Serial("/dev/tty.usbmodem101", 9600, timeout=1)
time.sleep(2)

print("Sending Ollama response to MCU...")

stream = ollama.chat(
    model="llama3:latest",
    messages=[{"role": "user", "content": "Why is the sky blue?"}],
    stream=True,
)

for chunk in stream:
    content = chunk["message"]["content"]
    send_to_board(ser, content)

print("\nTransmission complete.")
ser.close()
print("Connection closed.")
