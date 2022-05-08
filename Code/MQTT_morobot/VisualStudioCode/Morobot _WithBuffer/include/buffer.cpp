#include "buffer.h"

Buffer::Buffer()
{

}

void Buffer::push(String message)
{
    buffer[tail] = message;
    increment(tail);
    
    is_empty = false;
}

String Buffer::pop()
{
    String val = buffer[tail];
    decrement(tail);
    
    if (tail == head)
        is_empty = true;
    
    return val;
}

//NOT IMPLEMENTED
int Buffer::get(int offset)
{

}

void Buffer::clear()
{
    head = 0;
    tail = 0;
}

void Buffer::increment(int& i) const
{
    if(i + 1 < size)
    {
        i++;
    }
    else
    {
        i = 0;
    }
}

void Buffer::decrement(int& i) const
{
    if(i - 1 >= 0)
    {
        i--;
    }
    else
    {
        i = size - 1;
    }
}

