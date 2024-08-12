#ifndef NETWORKING_H
#define NETWORKING_H

#include <Arduino.h>


class Networking {
public:
    void writeDataToThingSpeak(String data);
    void setup(int rate);
    void updateRate(int rate);
};

#endif // NETWORKING_H