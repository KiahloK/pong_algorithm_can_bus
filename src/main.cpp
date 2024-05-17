//
// Created by Tom Knötzsch on 16.05.24.
//

/*
 * Implementation of a KI for Pong with CAN
 */

#include "Arduino.h"
#include "CAN.h"

#include <Adafruit_NeoPixel.h>

#define PIN 17 // For microBUS 1
// #define PIN 13 // For microBUS 2
#define NUM_PIXELS 100

Adafruit_NeoPixel LEDs(NUM_PIXELS, PIN, NEO_GRB + NEO_KHZ800);

// 1 for left side, 2 for right side
int player = 2;

// TODO: Geile animation machen
// TODO: "fiesigkeit" einbauen, mit den winkeln die bälle in die ecken drängen

int a = 1;

int ball_position_y = 65;
int16_t previous_x = 100;
int16_t previous_y = 65;

int gamestand_1 = 0;
int gamestand_2 = 0;

void onReceive(int packageSize);

void draw(int number, int offset_x, int offset_y);


void draw_lose(int offset_x, int offset_y);
void draw_victory(int offset_x, int offset_y);

void setup() {
    Serial.begin(9600);
    //while (!Serial) delay(10);

    Serial.println("CAN Receiver Callback");
    a = 2;

    pinMode(PIN_CAN_STANDBY, OUTPUT);
    digitalWrite(PIN_CAN_STANDBY, false); // turn off STANDBY
    pinMode(PIN_CAN_BOOSTEN, OUTPUT);
    digitalWrite(PIN_CAN_BOOSTEN, true); // turn on booster

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
    LEDs.setBrightness(20);

    // register the receive callback
    CAN.onReceive(onReceive);
}

boolean color_switcher = true;


int won = 0;

void loop() {
    delay(100);
    if(won == 0)
    {
        draw(gamestand_1, 2, 7);
        draw(gamestand_2, 2, 0);
        if(color_switcher)
        {
            LEDs.setPixelColor(4 + 40, LEDs.Color(255, 255, 255));
            LEDs.setPixelColor(5 + 50, LEDs.Color(255, 255, 255));
            color_switcher = false;
        } else
        {
            LEDs.setPixelColor(4 + 40, LEDs.Color(0, 0, 0));
            LEDs.setPixelColor(5 + 50, LEDs.Color(0, 0, 0));
            LEDs.clear();
            color_switcher = true;
        }
        delay(500);
    } else if (won == 1)
    {
        draw_victory(0,0);
        delay(500);
        LEDs.clear();
        delay(500);
    } else
    {
        draw_lose(0,0);
        delay(500);
        LEDs.clear();
        delay(500);
    }
}



void onReceive(int packetSize) {

    // received a packet
    if (CAN.packetId() != 0x01) return;

    uint8_t content[] = {0, 0, 0};

    // only print packet data for non-RTR packets
    CAN.readBytes(content, packetSize);

    // current x, y data
    int16_t x = content[0];
    int16_t y = content[1];

    uint8_t ballstate = content[2];
    uint8_t final_y;

    if(ballstate == 0 || ballstate == 7)
    {
        ball_position_y = 65;
        gamestand_1 = 0;
        gamestand_2 = 0;
        won = 0;
        LEDs.clear();
    }
    if(ballstate == 2)
    {
        gamestand_1++;
        LEDs.clear();
    }

    if(ballstate == 3 || ballstate == 6)
    {
        gamestand_2++;
        LEDs.clear();
    }

    if(ballstate == 5)
    {
        if(player == 1)
        {
            won = 1;
            LEDs.clear();
        } else
        {
            won = 2;
            LEDs.clear();
        }
    }

    if(ballstate == 6)
    {
        if(player == 1)
        {
            won = 2;
            LEDs.clear();
        } else
        {
            won = 1;
            LEDs.clear();
        }
    }
    boolean turn = false;

    // ball is going vertical -> just go where it's going
    if (x - previous_x == 0)
    {
        final_y = y;
    } else
    {
        // ball is going "down"
        if(y < previous_y)
        {
            // ball is going to the left
            if(previous_x > x)
            {
                final_y = 149 - previous_y + ((6-previous_x) / (x - previous_x)) * (previous_y - y);
            } // ball is going to the right
            else
            {
                final_y = 149 - previous_y + (249-previous_x) / (x - previous_x) * (previous_y - y);
            }
            turn = true;
        }
        // ball is going "up"
        else
        {
            // ball is going left
            if(previous_x > x)
            {
                final_y = previous_y + ((6-previous_x) / (x - previous_x)) * (y - previous_y);
            } // ball is going to the right
            else
            {
                final_y = previous_y + ((249-previous_x) / (x - previous_x)) * (y - previous_y);
            }

        }
    }


    uint8_t amount_spaces = final_y / 149;
    uint8_t final_height = final_y % 149;

    // we "turn" the space if it lands on a "uneven" space or if the turn shall be true
    if((amount_spaces%2 != 0 && turn == false) || (amount_spaces%2 == 0 && turn == true))
    {
        final_height = 149 - final_height;
    }


    int8_t updateValue = 0;

    // if it goes to the wrong direction, the player shall go to the middle
    if((previous_x > x && player == 2) || (previous_x < x && player == 1))
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
    } // "normal" case, the player goes to the ball
    else
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


    // output data
    Serial.println();
    Serial.printf("Position of x: %d\n", x);
    Serial.printf("Position of y: %d\n", y);
    Serial.printf("Ballstate %d\n", ballstate);
    Serial.printf("Final y: %d\n", final_y);
    Serial.printf("updateValue: %d\n", updateValue);
    Serial.println();

    // defines the type of packet that is sent corresponding to the player
    if(player == 1)
    {
        CAN.beginPacket(0x02);
    } else if (player == 2)
    {
        CAN.beginPacket(0x03);
    }

    // change to updateValue
    CAN.write(1);
    CAN.endPacket();

    previous_x = x;
    previous_y = y;
    Serial.println("Recieved and sent 1");
}


void draw(int number, int offset_x, int offset_y)
{
    auto color = LEDs.Color(255, 255, 255);
    if(number == 0)
    {
        LEDs.setPixelColor(1 + offset_x + 10 * offset_y, color);
        LEDs.setPixelColor(2 + offset_x + 10 * offset_y, color);
        LEDs.setPixelColor(3 + offset_x + 10 * offset_y, color);
        LEDs.setPixelColor(5 + 10 + offset_x + 10 * offset_y, color);

        LEDs.setPixelColor(4 + 00 + offset_x + 10 * offset_y, color);

        LEDs.setPixelColor(4 + 20 + offset_x + 10 * offset_y, color);
        LEDs.setPixelColor(3 + 20 + offset_x + 10 * offset_y, color);
        LEDs.setPixelColor(2 + 20 + offset_x + 10 * offset_y, color);
        LEDs.setPixelColor(1 + 20 + offset_x + 10 * offset_y, color);
        LEDs.setPixelColor(0 + 10 + offset_x + 10 * offset_y, color);
        LEDs.show();

    } else if (number == 1)
    {
        LEDs.setPixelColor(0 + offset_x + 10 * offset_y, color);
        LEDs.setPixelColor(1 + offset_x + 10 * offset_y, color);
        LEDs.setPixelColor(2 + offset_x + 10 * offset_y, color);
        LEDs.setPixelColor(3 + offset_x + 10 * offset_y, color);
        LEDs.setPixelColor(4 + offset_x + 10 * offset_y, color);
        LEDs.setPixelColor(5 + offset_x + 10 * offset_y, color);
        LEDs.setPixelColor(4 + 10 + offset_x + 10 * offset_y, color);
        LEDs.show();
    } else if (number == 2)
    {
        LEDs.setPixelColor(0 + offset_x + 10 * offset_y, color);
        LEDs.setPixelColor(0 + 10 + offset_x + 10 * offset_y, color);
        LEDs.setPixelColor(0 + 20 + offset_x + 10 * offset_y, color);
        LEDs.setPixelColor(1 + 20 + offset_x + 10 * offset_y, color);
        LEDs.setPixelColor(2 + 10 + offset_x + 10 * offset_y, color);
        LEDs.setPixelColor(3 + 0 + offset_x + 10 * offset_y, color);
        LEDs.setPixelColor(4 + 0 + offset_x + 10 * offset_y, color);
        LEDs.setPixelColor(5 + 10 + offset_x + 10 * offset_y, color);
        LEDs.setPixelColor(4 + 20 + offset_x + 10 * offset_y, color);
        LEDs.show();
    } else if (number == 3)
    {
        LEDs.setPixelColor(0 + 20 + offset_x + 10 * offset_y, color);
        LEDs.setPixelColor(0 + 10 + offset_x + 10 * offset_y, color);
        LEDs.setPixelColor(1 + 00 + offset_x + 10 * offset_y,  color);
        LEDs.setPixelColor(2 + 00 + offset_x + 10 * offset_y, color);
        LEDs.setPixelColor(3 + 10 + offset_x + 10 * offset_y, color);
        LEDs.setPixelColor(4 + 0 + offset_x + 10 * offset_y, color);
        LEDs.setPixelColor(5 + 10 + offset_x + 10 * offset_y, color);
        LEDs.setPixelColor(5 + 20 + offset_x + 10 * offset_y, color);
        LEDs.show();
    } else if(number == 4)
    {
        LEDs.setPixelColor(0 + 00 + offset_x + 10 * offset_y, color);
        LEDs.setPixelColor(1 + 00 + offset_x + 10 * offset_y, color);
        LEDs.setPixelColor(2 + 00 + offset_x + 10 * offset_y, color);
        LEDs.setPixelColor(3 + 00 + offset_x + 10 * offset_y, color);
        LEDs.setPixelColor(4 + 00 + offset_x + 10 * offset_y, color);
        LEDs.setPixelColor(5 + 0 + offset_x + 10 * offset_y, color);
        LEDs.setPixelColor(3 + 10 + offset_x + 10 * offset_y, color);
        LEDs.setPixelColor(3 + 20 + offset_x + 10 * offset_y, color);
        LEDs.setPixelColor(4 + 20 + offset_x + 10 * offset_y, color);
        LEDs.setPixelColor(5 + 20 + offset_x + 10 * offset_y, color);
        LEDs.show();
    } else if (number == 5)
    {
        LEDs.setPixelColor(0 + 00 + offset_x + 10 * offset_y, color);
        LEDs.setPixelColor(1 + 00 + offset_x + 10 * offset_y, color);
        LEDs.setPixelColor(2 + 00 + offset_x + 10 * offset_y, color);
        LEDs.setPixelColor(0 + 10 + offset_x + 10 * offset_y, color);
        LEDs.setPixelColor(0 + 20 + offset_x + 10 * offset_y, color);
        LEDs.setPixelColor(3 + 0 + offset_x + 10 * offset_y, color);
        LEDs.setPixelColor(5 + 0 + offset_x + 10 * offset_y, color);
        LEDs.setPixelColor(3 + 10 + offset_x + 10 * offset_y, color);
        LEDs.setPixelColor(3 + 20 + offset_x + 10 * offset_y, color);
        LEDs.setPixelColor(5 + 10 + offset_x + 10 * offset_y, color);
        LEDs.setPixelColor(5 + 20 + offset_x + 10 * offset_y, color);
        LEDs.setPixelColor(4 + 20 + offset_x + 10 * offset_y, color);
        LEDs.show();
    }
}

void draw_victory(int offset_x, int offset_y)
{
    auto color1 = LEDs.Color(224, 169, 18);
    auto color2 = LEDs.Color(255, 194, 14);
    auto color3 = LEDs.Color(255, 255, 255);
    auto color4 = LEDs.Color(255, 242, 0);

    LEDs.setPixelColor(7 + offset_x + 10 * offset_y, color1);
    LEDs.setPixelColor(8 + offset_x + 10 * offset_y, color1);
    LEDs.setPixelColor(16 + 00 + offset_x + 10 * offset_y, color1);
    LEDs.setPixelColor(18 + offset_x + 10 * offset_y, color1);
    LEDs.setPixelColor(20 + offset_x + 10 * offset_y, color1);
    LEDs.setPixelColor(25 + offset_x + 10 * offset_y, color1);
    LEDs.setPixelColor(26+ offset_x + 10 * offset_y, color2);
    LEDs.setPixelColor(27+ offset_x + 10 * offset_y, color2);
    LEDs.setPixelColor(28 + offset_x + 10 * offset_y, color2);
    LEDs.setPixelColor(29 + offset_x + 10 * offset_y, color1);
    LEDs.setPixelColor(30 + offset_x + 10 * offset_y, color2);
    LEDs.setPixelColor(31 + offset_x + 10 * offset_y, color1);
    LEDs.setPixelColor(34 + offset_x + 10 * offset_y, color1);
    LEDs.setPixelColor(35 + offset_x + 10 * offset_y, color2);
    LEDs.setPixelColor(36 + offset_x + 10 * offset_y, color2);
    LEDs.setPixelColor(37 + offset_x + 10 * offset_y, color2);
    LEDs.setPixelColor(38 + offset_x + 10 * offset_y, color2);
    LEDs.setPixelColor(39 + offset_x + 10 * offset_y, color2);
    LEDs.setPixelColor(40 + offset_x + 10 * offset_y, color2);
    LEDs.setPixelColor(41 + offset_x + 10 * offset_y, color2);
    LEDs.setPixelColor(42 + offset_x + 10 * offset_y, color2);
    LEDs.setPixelColor(43 + offset_x + 10 * offset_y, color2);
    LEDs.setPixelColor(44 + offset_x + 10 * offset_y, color2);
    LEDs.setPixelColor(45 + offset_x + 10 * offset_y, color3);
    LEDs.setPixelColor(46 + offset_x + 10 * offset_y, color3);
    LEDs.setPixelColor(47 + offset_x + 10 * offset_y, color3);
    LEDs.setPixelColor(48 + offset_x + 10 * offset_y, color3);
    LEDs.setPixelColor(49 + offset_x + 10 * offset_y, color2);
    LEDs.setPixelColor(50 + offset_x + 10 * offset_y, color2);
    LEDs.setPixelColor(51 + offset_x + 10 * offset_y, color2);
    LEDs.setPixelColor(52 + offset_x + 10 * offset_y, color2);
    LEDs.setPixelColor(53 + offset_x + 10 * offset_y, color2);
    LEDs.setPixelColor(54 + offset_x + 10 * offset_y, color2);
    LEDs.setPixelColor(55 + offset_x + 10 * offset_y, color2);
    LEDs.setPixelColor(56 + offset_x + 10 * offset_y, color2);
    LEDs.setPixelColor(57 + offset_x + 10 * offset_y, color3);
    LEDs.setPixelColor(58 + offset_x + 10 * offset_y, color2);
    LEDs.setPixelColor(59 + offset_x + 10 * offset_y, color2);
    LEDs.setPixelColor(60 + offset_x + 10 * offset_y, color2);
    LEDs.setPixelColor(61 + offset_x + 10 * offset_y, color4);
    LEDs.setPixelColor(64 + offset_x + 10 * offset_y, color4);
    LEDs.setPixelColor(65 + offset_x + 10 * offset_y, color2);
    LEDs.setPixelColor(66 + offset_x + 10 * offset_y, color2);
    LEDs.setPixelColor(67 + offset_x + 10 * offset_y, color2);
    LEDs.setPixelColor(68 + offset_x + 10 * offset_y, color2);
    LEDs.setPixelColor(69 + offset_x + 10 * offset_y, color4);
    LEDs.setPixelColor(70 + offset_x + 10 * offset_y, color4);
    LEDs.setPixelColor(75 + offset_x + 10 * offset_y, color4);
    LEDs.setPixelColor(76 + offset_x + 10 * offset_y, color2);
    LEDs.setPixelColor(77 + offset_x + 10 * offset_y, color2);
    LEDs.setPixelColor(78 + offset_x + 10 * offset_y, color2);
    LEDs.setPixelColor(79 + offset_x + 10 * offset_y, color4);
    LEDs.setPixelColor(86 + offset_x + 10 * offset_y, color4);
    LEDs.setPixelColor(88 + offset_x + 10 * offset_y, color4);
    LEDs.setPixelColor(97 + offset_x + 10 * offset_y, color4);
    LEDs.setPixelColor(98 + offset_x + 10 * offset_y, color4);
    LEDs.show();
}


void draw_lose(int offset_x, int offset_y)
{
    auto color1 = LEDs.Color(255, 255, 255);
    auto color2 = LEDs.Color(30, 0, 255);

    LEDs.setPixelColor(5 + offset_x + 10 * offset_y, color1);
    LEDs.setPixelColor(6 + offset_x + 10 * offset_y, color1);
    LEDs.setPixelColor(14 + 00 + offset_x + 10 * offset_y, color1);
    LEDs.setPixelColor(15 + offset_x + 10 * offset_y, color1);
    LEDs.setPixelColor(16 + offset_x + 10 * offset_y, color1);
    LEDs.setPixelColor(17 + offset_x + 10 * offset_y, color1);
    LEDs.setPixelColor(18+ offset_x + 10 * offset_y, color1);
    LEDs.setPixelColor(21+ offset_x + 10 * offset_y, color1);
    LEDs.setPixelColor(22 + offset_x + 10 * offset_y, color1);
    LEDs.setPixelColor(23 + offset_x + 10 * offset_y, color1);
    LEDs.setPixelColor(24 + offset_x + 10 * offset_y, color1);
    LEDs.setPixelColor(25 + offset_x + 10 * offset_y, color1);
    LEDs.setPixelColor(26 + offset_x + 10 * offset_y, color2);
    LEDs.setPixelColor(27 + offset_x + 10 * offset_y, color2);
    LEDs.setPixelColor(28 + offset_x + 10 * offset_y, color1);
    LEDs.setPixelColor(29 + offset_x + 10 * offset_y, color1);
    LEDs.setPixelColor(33 + offset_x + 10 * offset_y, color1);
    LEDs.setPixelColor(34 + offset_x + 10 * offset_y, color1);
    LEDs.setPixelColor(35 + offset_x + 10 * offset_y, color1);
    LEDs.setPixelColor(37 + offset_x + 10 * offset_y, color2);
    LEDs.setPixelColor(38 + offset_x + 10 * offset_y, color1);
    LEDs.setPixelColor(39 + offset_x + 10 * offset_y, color1);
    LEDs.setPixelColor(41 + offset_x + 10 * offset_y, color1);
    LEDs.setPixelColor(42 + offset_x + 10 * offset_y, color1);
    LEDs.setPixelColor(43 + offset_x + 10 * offset_y, color1);
    LEDs.setPixelColor(45 + offset_x + 10 * offset_y, color1);
    LEDs.setPixelColor(46 + offset_x + 10 * offset_y, color1);
    LEDs.setPixelColor(47 + offset_x + 10 * offset_y, color1);
    LEDs.setPixelColor(48 + offset_x + 10 * offset_y, color1);
    LEDs.setPixelColor(49 + offset_x + 10 * offset_y, color1);
    LEDs.setPixelColor(53 + offset_x + 10 * offset_y, color1);
    LEDs.setPixelColor(55 + offset_x + 10 * offset_y, color1);
    LEDs.setPixelColor(56 + offset_x + 10 * offset_y, color1);
    LEDs.setPixelColor(57 + offset_x + 10 * offset_y, color1);
    LEDs.setPixelColor(58 + offset_x + 10 * offset_y, color1);
    LEDs.setPixelColor(59 + offset_x + 10 * offset_y, color1);
    LEDs.setPixelColor(63 + offset_x + 10 * offset_y, color1);
    LEDs.setPixelColor(64 + offset_x + 10 * offset_y, color1);
    LEDs.setPixelColor(65 + offset_x + 10 * offset_y, color1);
    LEDs.setPixelColor(68 + offset_x + 10 * offset_y, color1);
    LEDs.setPixelColor(69 + offset_x + 10 * offset_y, color1);
    LEDs.setPixelColor(71 + offset_x + 10 * offset_y, color1);
    LEDs.setPixelColor(72 + offset_x + 10 * offset_y, color1);
    LEDs.setPixelColor(73 + offset_x + 10 * offset_y, color1);
    LEDs.setPixelColor(74 + offset_x + 10 * offset_y, color1);
    LEDs.setPixelColor(75 + offset_x + 10 * offset_y, color1);
    LEDs.setPixelColor(77 + offset_x + 10 * offset_y, color1);
    LEDs.setPixelColor(78 + offset_x + 10 * offset_y, color1);
    LEDs.setPixelColor(79 + offset_x + 10 * offset_y, color1);
    LEDs.setPixelColor(84 + offset_x + 10 * offset_y, color1);
    LEDs.setPixelColor(85 + offset_x + 10 * offset_y, color1);
    LEDs.setPixelColor(86 + offset_x + 10 * offset_y, color1);
    LEDs.setPixelColor(87 + offset_x + 10 * offset_y, color1);
    LEDs.setPixelColor(88 + offset_x + 10 * offset_y, color1);
    LEDs.setPixelColor(95 + offset_x + 10 * offset_y, color1);
    LEDs.setPixelColor(96 + offset_x + 10 * offset_y, color1);
    LEDs.show();
}