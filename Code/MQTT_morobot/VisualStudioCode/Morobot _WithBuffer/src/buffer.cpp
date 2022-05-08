#include <buffer.h>

Buffer::Buffer()
{
    head = 0;
    tail = 0;
    hasElement = false;
}

bool Buffer::push(String message)
{
    buffer[head] = message;
    increment(head);
    
    hasElement = true;
    Serial.println("Pushed to buffer");
    return true;
}

String Buffer::pop()
{
    String val = buffer[tail];
    increment(tail);
    
    if (tail == head)
    {
        hasElement = false;
    }
    
    return val;
}

String Buffer::get(int offset)
{
    return buffer[offset];
}

void Buffer::clear()
{
    head = 0;
    tail = 0;
}

bool Buffer::IsEmpty()
{
    return !hasElement;
}

void Buffer::increment(int& i) const
{
    if(i + 1 <= size)
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
        i = size;
    }
}

