//
// Created by Tom Knötzsch on 16.05.24.
//

/*
 * Adafruit Feather M4 CAN Receiver Callback Example
 */
#include "Arduino.h"
#include "CAN.h"

#include <Adafruit_NeoPixel.h>

#define PIN 17 // For microBUS 1
// #define PIN 13 // For microBUS 2
#define NUM_PIXELS 100

Adafruit_NeoPixel LEDs(NUM_PIXELS, PIN, NEO_GRB + NEO_KHZ800);


// TODO: Die andere Richtung programmieren, die für spieler 1. Sonst ist kacke.
// TODO: Geile animation machen
// TODO: "fiesigkeit" einbauen, mit den winkeln die bälle in die ecken drängen


int a = 1;

int ball_position_y = 65;
int previous_x = 100;
int previous_y = 65;

int gamestand_1 = 0;
int gamestand_2 = 0;

void onReceive(int packageSize);

void int8ToBitArray(int8_t value, bool* bitArray) {
    for (int i = 0; i < 8; ++i) {
        bitArray[i] = (value >> i) & 1;
    }
}

void setup() {
    Serial.begin(9600);
    while (!Serial) delay(10);

    Serial.println("CAN Receiver Callback");
    a = 2;

    pinMode(PIN_CAN_STANDBY, OUTPUT);
    digitalWrite(PIN_CAN_STANDBY, false); // turn off STANDBY
    pinMode(PIN_CAN_BOOSTEN, OUTPUT);
    digitalWrite(PIN_CAN_BOOSTEN, true); // turn on booster

    // wenn ball in falsche richtung geht (zum gegner) gehe zur mitte um vorbereitet zu sein

    // start the CAN bus at 250 kbps
    if (!CAN.begin(250000)) {
        Serial.println("Starting CAN failed!");
        while (1) delay(10);
    }
    Serial.println("Starting CAN!");

    LEDs.begin();

    // Clear all LEDs
    LEDs.clear();

    // Set brightness of all LEDs. [0, 255]
    LEDs.setBrightness(40);

    // register the receive callback
    CAN.onReceive(onReceive);
}


void draw(int number, int offset_x, int offset_y)
{
    if(number == 0)
    {
        LEDs.setPixelColor(1 + offset_x + 10 * offset_y, LEDs.Color(255, 255, 255));
        LEDs.setPixelColor(2 + offset_x + 10 * offset_y, LEDs.Color(255, 255, 255));
        LEDs.setPixelColor(3 + offset_x + 10 * offset_y, LEDs.Color(255, 255, 255));
        LEDs.setPixelColor(5 + 10 + offset_x + 10 * offset_y, LEDs.Color(255, 255, 255));

        LEDs.setPixelColor(4 + 00 + offset_x + 10 * offset_y, LEDs.Color(255, 255, 255));

        LEDs.setPixelColor(4 + 20 + offset_x + 10 * offset_y, LEDs.Color(255, 255, 255));
        LEDs.setPixelColor(3 + 20 + offset_x + 10 * offset_y, LEDs.Color(255, 255, 255));
        LEDs.setPixelColor(2 + 20 + offset_x + 10 * offset_y, LEDs.Color(255, 255, 255));
        LEDs.setPixelColor(1 + 20 + offset_x + 10 * offset_y, LEDs.Color(255, 255, 255));
        LEDs.setPixelColor(0 + 10 + offset_x + 10 * offset_y, LEDs.Color(255, 255, 255));
        LEDs.show();
    } else if (number == 1)
    {
        LEDs.setPixelColor(0 + offset_x + 10 * offset_y, LEDs.Color(255, 255, 255));
        LEDs.setPixelColor(1 + offset_x + 10 * offset_y, LEDs.Color(255, 255, 255));
        LEDs.setPixelColor(2 + offset_x + 10 * offset_y, LEDs.Color(255, 255, 255));
        LEDs.setPixelColor(3 + offset_x + 10 * offset_y, LEDs.Color(255, 255, 255));
        LEDs.setPixelColor(4 + offset_x + 10 * offset_y, LEDs.Color(255, 255, 255));
        LEDs.setPixelColor(5 + offset_x + 10 * offset_y, LEDs.Color(255, 255, 255));
        LEDs.setPixelColor(4 + 10 + offset_x + 10 * offset_y, LEDs.Color(255, 255, 255));
        LEDs.show();
    } else if (number == 2)
    {
        LEDs.setPixelColor(0 + offset_x + 10 * offset_y, LEDs.Color(255, 255, 255));
        LEDs.setPixelColor(0 + 10 + offset_x + 10 * offset_y, LEDs.Color(255, 255, 255));
        LEDs.setPixelColor(0 + 20 + offset_x + 10 * offset_y, LEDs.Color(255, 255, 255));
        LEDs.setPixelColor(1 + 20 + offset_x + 10 * offset_y, LEDs.Color(255, 255, 255));
        LEDs.setPixelColor(2 + 10 + offset_x + 10 * offset_y, LEDs.Color(255, 255, 255));
        LEDs.setPixelColor(3 + 0 + offset_x + 10 * offset_y, LEDs.Color(255, 255, 255));
        LEDs.setPixelColor(4 + 0 + offset_x + 10 * offset_y, LEDs.Color(255, 255, 255));
        LEDs.setPixelColor(5 + 10 + offset_x + 10 * offset_y, LEDs.Color(255, 255, 255));
        LEDs.setPixelColor(4 + 20 + offset_x + 10 * offset_y, LEDs.Color(255, 255, 255));
        LEDs.show();
    } else if (number == 3)
    {
        LEDs.setPixelColor(0 + 20 + offset_x + 10 * offset_y, LEDs.Color(255, 255, 255));
        LEDs.setPixelColor(0 + 10 + offset_x + 10 * offset_y, LEDs.Color(255, 255, 255));
        LEDs.setPixelColor(1 + 00 + offset_x + 10 * offset_y, LEDs.Color(255, 255, 255));
        LEDs.setPixelColor(2 + 00 + offset_x + 10 * offset_y, LEDs.Color(255, 255, 255));
        LEDs.setPixelColor(3 + 10 + offset_x + 10 * offset_y, LEDs.Color(255, 255, 255));
        LEDs.setPixelColor(4 + 0 + offset_x + 10 * offset_y, LEDs.Color(255, 255, 255));
        LEDs.setPixelColor(5 + 10 + offset_x + 10 * offset_y, LEDs.Color(255, 255, 255));
        LEDs.setPixelColor(5 + 20 + offset_x + 10 * offset_y, LEDs.Color(255, 255, 255));
        LEDs.show();
    } else if(number == 4)
    {
        LEDs.setPixelColor(0 + 00 + offset_x + 10 * offset_y, LEDs.Color(255, 255, 255));
        LEDs.setPixelColor(1 + 00 + offset_x + 10 * offset_y, LEDs.Color(255, 255, 255));
        LEDs.setPixelColor(2 + 00 + offset_x + 10 * offset_y, LEDs.Color(255, 255, 255));
        LEDs.setPixelColor(3 + 00 + offset_x + 10 * offset_y, LEDs.Color(255, 255, 255));
        LEDs.setPixelColor(4 + 00 + offset_x + 10 * offset_y, LEDs.Color(255, 255, 255));
        LEDs.setPixelColor(5 + 0 + offset_x + 10 * offset_y, LEDs.Color(255, 255, 255));
        LEDs.setPixelColor(3 + 10 + offset_x + 10 * offset_y, LEDs.Color(255, 255, 255));
        LEDs.setPixelColor(3 + 20 + offset_x + 10 * offset_y, LEDs.Color(255, 255, 255));
        LEDs.setPixelColor(4 + 20 + offset_x + 10 * offset_y, LEDs.Color(255, 255, 255));
        LEDs.setPixelColor(5 + 20 + offset_x + 10 * offset_y, LEDs.Color(255, 255, 255));
        LEDs.show();
    } else if (number == 5)
    {
        LEDs.setPixelColor(0 + 00 + offset_x + 10 * offset_y, LEDs.Color(255, 255, 255));
        LEDs.setPixelColor(1 + 00 + offset_x + 10 * offset_y, LEDs.Color(255, 255, 255));
        LEDs.setPixelColor(2 + 00 + offset_x + 10 * offset_y, LEDs.Color(255, 255, 255));
        LEDs.setPixelColor(0 + 10 + offset_x + 10 * offset_y, LEDs.Color(255, 255, 255));
        LEDs.setPixelColor(0 + 20 + offset_x + 10 * offset_y, LEDs.Color(255, 255, 255));
        LEDs.setPixelColor(3 + 0 + offset_x + 10 * offset_y, LEDs.Color(255, 255, 255));
        LEDs.setPixelColor(5 + 0 + offset_x + 10 * offset_y, LEDs.Color(255, 255, 255));
        LEDs.setPixelColor(3 + 10 + offset_x + 10 * offset_y, LEDs.Color(255, 255, 255));
        LEDs.setPixelColor(3 + 20 + offset_x + 10 * offset_y, LEDs.Color(255, 255, 255));
        LEDs.setPixelColor(5 + 10 + offset_x + 10 * offset_y, LEDs.Color(255, 255, 255));
        LEDs.setPixelColor(5 + 20 + offset_x + 10 * offset_y, LEDs.Color(255, 255, 255));
        LEDs.setPixelColor(4 + 20 + offset_x + 10 * offset_y, LEDs.Color(255, 255, 255));
        LEDs.show();
    }
}


boolean switcher = true;

void loop() {
    draw(gamestand_1, 2, 7);
    draw(gamestand_2, 2, 0);
    if(switcher)
    {
        LEDs.setPixelColor(4 + 40, LEDs.Color(255, 255, 255));
        LEDs.setPixelColor(5 + 50, LEDs.Color(255, 255, 255));
        switcher = false;
    } else
    {
        LEDs.setPixelColor(4 + 40, LEDs.Color(0, 0, 0));
        LEDs.setPixelColor(5 + 50, LEDs.Color(0, 0, 0));
        switcher = true;
    }
    delay(500);
}

void onReceive(int packetSize) {
    // received a packet

    if (CAN.packetId() != 0x01) return;

    uint8_t content[] = {0, 0, 0};

    // only print packet data for non-RTR packets
    CAN.readBytes(content, packetSize);

    uint8_t x = content[0];
    uint8_t y = content[1];
    uint8_t ballstate = content[2];
    uint8_t final_y = 0;

    if(ballstate == 0)
    {
        ball_position_y = 65;
        gamestand_1 = 0;
        gamestand_2 = 0;
    }
    if(ballstate == 2)
    {
        gamestand_1++;
    }

    if(ballstate == 3)
    {
        gamestand_2++;
    }

    boolean turn = false;
    if ((x - previous_x) == 0)
    {
        final_y = y;
    } else
    {
        if(y < previous_y)
        {
            final_y = 149 - previous_y + ((255-previous_x) / (x - previous_x)) * (previous_y - y);
            turn = true;
        }
        else
        {
            final_y = previous_y + ((255-previous_x) / (x - previous_x)) * (y - previous_y);
        }
    }

    uint8_t amount_spaces = final_y / 149;
    uint8_t final_height = final_y % 149;

    if(((amount_spaces%2) != 0 && turn == false) || ((amount_spaces%2) == 0 && turn == true))
    {
        final_height = 149 - final_height;
    }


    int8_t updateValue = 0;
    if(previous_x > x)
    {
        if(ball_position_y > 65)
        {
            ball_position_y--;
            updateValue = -1;
        } else if (ball_position_y < 65)
        {
            ball_position_y++;
            updateValue = +1;
        } else
        {
            updateValue = 0;
        }
    } else
    {
        if(ball_position_y + 10 > final_height)
        {
            if (ball_position_y != 0)
            {
                ball_position_y--;
            }
            updateValue = -1;
        } else if (ball_position_y + 10 < final_height)
        {
            if(ball_position_y != 129)
            {
                ball_position_y++;
            }
            updateValue = 1;
        } else
        {
            updateValue = 0;
        }
    }



    Serial.println();
    Serial.printf("Position of x: %d\n", x);
    Serial.printf("Position of y: %d\n", y);
    Serial.printf("Ballstate %d\n", ballstate);
    Serial.printf("Final y: %d\n", final_y);
    Serial.printf("updateValue: %d\n", updateValue);
    Serial.println();


    CAN.beginPacket(0x03);
    CAN.write(updateValue);
    CAN.endPacket();

    previous_x = x;
    previous_y = y;
    Serial.println("Recieved and sent 1");
}