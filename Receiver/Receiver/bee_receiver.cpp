/**
 * Bee Hive Monitor Receiver using Raspberry Pi
 * Listen for signals on mrf24j40 reciver, parse and write to DB
 */

#include "mrf24j.h"
#include "credentials.h"

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <unistd.h>
#include <mysql/mysql.h>
#include <wiringPi.h>

#define NETWORK	0xcafe
#define NAME	0x4201

#define PIN_RESET	6
#define PIN_CS		7
#define PIN_INTERRUPT	2
#define SPI_CHANNEL	0

Mrf24j mrf(PIN_RESET, PIN_CS, SPI_CHANNEL);
MYSQL *mysql;

// Write readings to the DB
void store_data(uint16_t source, byte LQI, byte RSSI, byte battery, double temp1, double humidity1, double temp2)
{
    char query[100];
    char szBattery[10];

    if(battery > -1)
        snprintf(szBattery, sizeof(szBattery), "%u", battery);
    else
        snprintf(szBattery, sizeof(szBattery), "NULL");

    char szTemp1[10];
    if(temp1 < 999)
        snprintf(szTemp1, sizeof(szTemp1), "%f", temp1);
    else
        snprintf(szTemp1, sizeof(szTemp1), "NULL");

    char szHumidity1[10];
    if(humidity1 < 999)
        snprintf(szHumidity1, sizeof(szHumidity1), "%f", humidity1);
    else
        snprintf(szHumidity1, sizeof(szHumidity1), "NULL");

    char szTemp2[10];
    if(temp2 < 999)
        snprintf(szTemp2, sizeof(szTemp2), "%f", temp2);
    else
        snprintf(szTemp2, sizeof(szTemp2), "NULL");

    snprintf(query, sizeof(query), "CALL StoreData('%u', %u, %u, %s, %s, %s, %s)", source, LQI, RSSI, szBattery, szTemp1, szHumidity1, szTemp2);
    mysql_query(mysql, query);
}

void handle_rx() {
    printf("received a packet %u bytes long\r\n", mrf.get_rxinfo()->frame_length);

    if(mrf.get_bufferPHY()){
        printf("Packet data (PHY Payload):\r\n");
        for (int i = 0; i < mrf.get_rxinfo()->frame_length; i++) {
            printf("%u", mrf.get_rxbuf()[i]);
        }
    }

    printf("\r\nASCII data (relevant data):\r\n");
    for (int i = 0; i < mrf.rx_datalength(); i++) {
        printf("%c", mrf.get_rxinfo()->rx_data[i]);
    }

    // sometimes we get data length without data so double check
    if(mrf.rx_datalength() > 0) {
        char data[mrf.rx_datalength() + 1];
        snprintf(data, mrf.rx_datalength() + 1, "%s", mrf.get_rxinfo()->rx_data);
        if(strlen(data) > 0) {
            // parse message
            char *token;
            double temp1 = 999;
            double humidity1 = 999;
            double temp2 = 999;
            char battery = -1;

            char *datap = strdupa(data);
            char szTemp[10];
            while ((token = strsep(&datap, ";"))) {
                if(strncmp(token, "T1:", 3) == 0) {
                    strncpy(szTemp, token + 3, sizeof(szTemp));
                    temp1 = atof(szTemp);
                }
                else if(strncmp(token, "H1:", 3) == 0) {
                    strncpy(szTemp, token + 3, sizeof(szTemp));
                    humidity1 = atof(szTemp);
                }
                else if(strncmp(token, "T2:", 3) == 0) {
                    strncpy(szTemp, token + 3, sizeof(szTemp));
                    temp2 = atof(szTemp);
                }
                else if(strncmp(token, "BP:", 3) == 0) {
                    strncpy(szTemp, token + 3, sizeof(szTemp));
                    battery = atof(szTemp);
                }
            }
            store_data(mrf.get_rxinfo()->source, mrf.get_rxinfo()->lqi, mrf.get_rxinfo()->rssi, battery, temp1, humidity1, temp2);
        }
    }
    //printf("\r\nLQI/RSSI=%u/%u\r\n", mrf.get_rxinfo()->lqi, mrf.get_rxinfo()->rssi);
}

void handle_tx() {
}

void interrupt_routine() {
    mrf.interrupt_handler(); // mrf24 object interrupt routine
    mrf.check_flags(&handle_rx, &handle_tx);
}

int setup() {
    mrf.reset();
    mrf.init();
    mrf.set_pan(NETWORK);
    // This is _our_ address
    mrf.address16_write(NAME);

    // Open DB connection
    mysql = mysql_init(NULL);
    if(mysql == NULL) {
        printf("Failed to initialize DB connection\r\n");
        return -1;
    }
    if(mysql_real_connect(mysql, DATABASE_IP, DATABASE_USERNAME, DATABASE_PASSWORD, DATABASE_NAME, 0, NULL, 0) == NULL) {
        printf("Failed to connect to DB\r\n");
        return -1;
    }


    // Register for interrupts
    wiringPiISR(PIN_INTERRUPT, INT_EDGE_BOTH, interrupt_routine);

    return 0;
}

int main() {
    if(setup() == -1) {
      return -1;
    }

    // Wait until interrupt
    while(true)
        sleep(UINT_MAX);

    return 0;
}
