/**
 * Bee Hive Transmitter using Arduino
 * Read sensor values every 15 minutes and send them through mrf24j40
 */
#include <avr/power.h>
#include <LowPower.h>
#include <SPI.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_AM2320.h>
#include "mrf24j.h"
#include "voltage.h"

#define PIN_TEMPERATURE 5
#define PIN_RESET       6
#define PIN_CS          7
#define PIN_POWERSW     3
#define PIN_INTERRUPT   2

#define NETWORK 0xcafe
#define NAME    0x4202
#define BASE    0x4201

Mrf24j mrf(PIN_RESET, PIN_CS, PIN_INTERRUPT);
OneWire owTemperature(PIN_TEMPERATURE);
DallasTemperature sensorTemperature(&owTemperature);
Adafruit_AM2320 am2320 = Adafruit_AM2320();

char tx_msg[128];
int tx_max_retries = 10;
int tx_retries = 0;
bool tx_wait_confirm = false;

void setup() {
  pinMode(PIN_POWERSW, OUTPUT);
}

void interrupt_routine() {
  mrf.interrupt_handler(); // mrf24 object interrupt routine
}

void loop() {
  // Turn power on to sensors + wireless
  power_on();
  
  // Request temperature reading
  sensorTemperature.requestTemperatures();
  delay(100);

  // Read temperature reading 
  char temp1[10];
  dtostrf(am2320.readTemperature(), sizeof(temp1) - 1, 2, temp1);
  char *temp1fix = skip_whitespace(temp1);

  char hum1[10];
  dtostrf(am2320.readHumidity(), sizeof(hum1) - 1, 2, hum1);
  char *hum1fix = skip_whitespace(hum1);
  
  char temp2[10];
  dtostrf(sensorTemperature.getTempCByIndex(0), sizeof(temp2) - 1, 2, temp2);
  char *temp2fix = skip_whitespace(temp2);

  int batteryPC = getVoltagePC();
  
  snprintf(tx_msg, sizeof(tx_msg), "BP:%u;BV:%u;T1:%s;H1:%s;T2:%s", batteryPC, getVoltage(), temp1fix, hum1fix, temp2fix);
  do_tx();

  // Sleep for 15 minutes
  for(int i=0; i<112; i++) {
    LowPower.powerStandby(SLEEP_8S, ADC_OFF, BOD_OFF);
  }
}

// Skip leading whitespace
char *skip_whitespace(char *in)
{
  char *skipper = &in[0];
  while(isWhitespace(skipper[0]))
    skipper++;

  return skipper;
}

void handle_rx() {
}

void do_tx() {
  tx_wait_confirm = true;
  mrf.send16(BASE, tx_msg);
  
  // Loop here a few times while tx_wait_confirm
  for(int i=0; i<5 && tx_wait_confirm; i++) {
    delay(100);
    mrf.check_flags(&handle_rx, &handle_tx);
  }

  // If we didn't get confirmation or we did but it failed, send again
  if((tx_wait_confirm || !mrf.get_txinfo()->tx_ok) && tx_retries < tx_max_retries) {
    tx_retries++;
    do_tx();
  } else {
    tx_wait_confirm = false;
    tx_retries = 0;
    power_off(); 
  }
}

void handle_tx() {
  tx_wait_confirm = false;
}

void power_on() {
  digitalWrite(PIN_POWERSW, LOW);
  delay(100);

  power_twi_enable();

  pinMode(PIN_TEMPERATURE, INPUT_PULLUP);
  sensorTemperature.begin();

  am2320.begin();
  
  // Revert changes made on MOSI pin during power off
  pinMode(11, OUTPUT);
  digitalWrite(11, HIGH);
  
  mrf.reset();
  mrf.init();
  mrf.set_pan(NETWORK);
  mrf.address16_write(NAME);
  
  //mrf.set_palna(true);
  //mrf.write_short(MRF_TRISGPIO, 0b00111111);
  //mrf.write_short(MRF_GPIO, 0b00001111);
  
  attachInterrupt(digitalPinToInterrupt(PIN_INTERRUPT), interrupt_routine, CHANGE);
  interrupts();
}

void power_off() {
  // End SPI
  mrf.shutdown();

  // Force MOSI to input and low to reduce power usage
  pinMode(11, INPUT);
  digitalWrite(11, LOW);

  // Other pins to low
  digitalWrite(PIN_RESET, LOW);
  digitalWrite(PIN_CS, LOW);

  // Stop interrupts using power
  detachInterrupt(digitalPinToInterrupt(PIN_INTERRUPT));

  // I2C shutdown
  TWCR &= ~(_BV(TWEN));
  TWCR = 0;
  pinMode(SDA, INPUT);
  pinMode(SCL, INPUT);
  digitalWrite(SDA, LOW);
  digitalWrite(SCL, LOW);
  //power_twi_disable();
  
  digitalWrite(PIN_POWERSW, HIGH);
}

