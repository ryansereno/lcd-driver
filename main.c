#include <stdint.h> // standard integer types
#include <util/delay.h>

// UART serial communication registers
#define UBRR0H (*((volatile uint8_t *)0xC5))
#define UBRR0L (*((volatile uint8_t *)0xC4))
#define UCSR0A (*((volatile uint8_t *)0xC0))
#define UCSR0B (*((volatile uint8_t *)0xC1))
#define UCSR0C (*((volatile uint8_t *)0xC2))
#define UDR0 (*((volatile uint8_t *)0xC6))

// UCSR0B bits
#define RXEN0 4
#define TXEN0 3

// UCSR0C bits
#define UCSZ00 1
#define UCSZ01 2

// UCSR0A bits
#define RXC0 7
#define UDRE0 5

// board registers
#define PORTB_REG (*((volatile uint8_t *)0x25))
#define PORTD_REG (*((volatile uint8_t *)0x2B))
#define DDRB_REG (*((volatile uint8_t *)0x24))
#define DDRD_REG (*((volatile uint8_t *)0x2A))

// LCD pin connections to Arduino pins
// These are the corresponding bits for the pins connected to the LCD
// PORTB pins
#define LCD_RS_BIT 4 // PB4 (pin 12)->LCD_RS
#define LCD_E_BIT 3  // PB3 (pin 11)->LCD_E

// PORTD pins
#define LCD_D4_BIT 5 // PD5 (pin 5)->LCD_D4
#define LCD_D5_BIT 4 // PD4 (pin 4)->LCD_D5
#define LCD_D6_BIT 3 // PD3 (pin 3)->LCD_D6
#define LCD_D7_BIT 2 // PD2 (pin 2)->LCD_D7

// serial communication functions
void uart_init(void) {
  // Set baud rate to 9600 for 16MHz clock
  UBRR0H = 0;
  UBRR0L = 103;
  // Enable receiver and transmitter
  UCSR0B = (1 << RXEN0) | (1 << TXEN0);
  // Set frame format: 8data, 1stop bit
  UCSR0C = (3 << UCSZ00);
}

void uart_transmit(unsigned char data) {
  // Wait for empty transmit buffer
  while (!(UCSR0A & (1 << UDRE0)))
    ;
  // Put data into buffer, sends the data
  UDR0 = data;
}

unsigned char uart_receive(void) {
  // Wait for data to be received
  while (!(UCSR0A & (1 << RXC0)))
    ;
  // Get and return received data from buffer
  return UDR0;
}

// LCD functions
void lcd_pulse_enable(void) {
  PORTB_REG |= (1 << LCD_E_BIT);  // set enable pin high
  _delay_us(1);                   // wait for 1 microsecond
  PORTB_REG &= ~(1 << LCD_E_BIT); // set Enable pin low
  _delay_ms(1);                   // wait for 1 millisecond
}

void lcd_send_nibble(uint8_t nibble) {
  // set each data pin based on the nibble bits
  if (nibble & 0x01)
    PORTD_REG |= (1 << LCD_D4_BIT);
  else
    PORTD_REG &= ~(1 << LCD_D4_BIT);
  if (nibble & 0x02)
    PORTD_REG |= (1 << LCD_D5_BIT);
  else
    PORTD_REG &= ~(1 << LCD_D5_BIT);
  if (nibble & 0x04)
    PORTD_REG |= (1 << LCD_D6_BIT);
  else
    PORTD_REG &= ~(1 << LCD_D6_BIT);
  if (nibble & 0x08)
    PORTD_REG |= (1 << LCD_D7_BIT);
  else
    PORTD_REG &= ~(1 << LCD_D7_BIT);
  lcd_pulse_enable(); // Pulse the enable pin to latch the data
}

void lcd_command(uint8_t cmd) {
  PORTB_REG &= ~(1 << LCD_RS_BIT); // Set RS low for command mode
  lcd_send_nibble(cmd >> 4);       // Send high nibble
  lcd_send_nibble(cmd);            // Send low nibble
  _delay_ms(2);                    // Wait for command to execute
}

void lcd_data(uint8_t data) {
  PORTB_REG |= (1 << LCD_RS_BIT); // Set RS high for data mode
  lcd_send_nibble(data >> 4);     // Send high nibble
  lcd_send_nibble(data);          // Send low nibble
  _delay_ms(1);                   // Wait for data to be processed
}

void lcd_init(void) {
  // Set the pins we're using as outputs
  DDRB_REG |= (1 << LCD_RS_BIT) | (1 << LCD_E_BIT);
  DDRD_REG |= (1 << LCD_D4_BIT) | (1 << LCD_D5_BIT) | (1 << LCD_D6_BIT) |
              (1 << LCD_D7_BIT);
  _delay_ms(50); // Wait for LCD to power up

  // 4-bit initialization sequence
  lcd_send_nibble(0x03);
  _delay_ms(5);
  lcd_send_nibble(0x03);
  _delay_ms(1);
  lcd_send_nibble(0x03);
  _delay_ms(1);
  lcd_send_nibble(0x02);
  _delay_ms(1);

  // Configure LCD: 4-bit mode, 2 lines, 5x8 font
  lcd_command(0x28);
  // Display on, cursor off, blink off
  lcd_command(0x0C);
  // Increment cursor, no display shift
  lcd_command(0x06);

  lcd_command(0x01); // Clear display
  _delay_ms(2);
}

void lcd_string(const char *str) {
  while (*str) {      // Loop until null terminator
    lcd_data(*str++); // Send each character and increment pointer
  }
}

int main(void) {
  lcd_init(); // Initialize the LCD
  uart_init();

  lcd_command(0x80); // Set cursor to beginning of first line
  lcd_string("Llama3.1 7B");

  lcd_command(0xC0); // Set cursor to beginning of second line
  lcd_string("Loading...");

  while (1) {
    if (UCSR0A & (1 << RXC0)) { // Check if data is available
      lcd_command(0x01);        // Clear display
      lcd_command(0x80);        // Move to first line
      char received = uart_receive();
      lcd_data(received);      // Display the received character
      uart_transmit(received); // Echo back the received character
    }
  }
  return 0; // Never reached
}
