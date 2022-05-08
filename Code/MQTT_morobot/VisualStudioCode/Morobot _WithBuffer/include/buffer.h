#ifndef BUFFER_H
#define BUFFER_H

#include <Arduino.h>

class Buffer 
{
    private:
        const static int size = 5;
        String buffer[size];
        int head;
        int tail;
        bool hasElement;
    public:
        Buffer();
        bool push(String message);
        String pop();
        String get(int offset);
        void clear();
        bool IsEmpty();
        void increment(int& i) const;
        void decrement(int& i) const;
};

#endif 



    
