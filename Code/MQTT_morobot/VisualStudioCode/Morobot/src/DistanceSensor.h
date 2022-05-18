class DistanceSensor
{
    private:
    long duration;
    int distance;
    int echoPin;
    int trigPin;
    bool flag;

    public:
    DistanceSensor(int echopin, int trigpin);
    void setupDistanceSensor(int echopin, int trigpin);
    void watchForObjects();
    void setFlag();
    void disableFlag();
    bool getFlag();
    
};