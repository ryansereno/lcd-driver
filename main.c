#include <stdint.h>     // standard integer types
//#include <util/delay.h> // delay functions

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
#define _delay_us(x) 
#define _delay_ms(x)

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
  // Clear display
  lcd_command(0x01);
  _delay_ms(2);
}

void lcd_string(const char *str) {
  while (*str) {      // Loop until null terminator
    lcd_data(*str++); // Send each character and increment pointer
  }
}

int main(void) {
  lcd_init(); // Initialize the LCD

  lcd_command(0x80); // Set cursor to beginning of first line
  lcd_string("Llama3.1 70B");

  lcd_command(0xC0); // Set cursor to beginning of second line
  lcd_string("Loading...");

  while (1) {
    // Main loop - do nothing
  }
  return 0; // Never reached
}
