#ifndef buffer_h
#define buffer_h

#include <Arduino.h>

class Buffer {
private:
    const static int size = 100;
    String buffer[size];
    int head = 0;
    int tail = 0;
    bool is_empty = true;
public:
    Buffer();
    void push(String message);
    String pop();
    int get(int offset);
    void clear();
    void increment(int& i) const;
    void decrement(int& i) const;
};

#endif 



    
