# Python code for Multiple Color Detection
import numpy as np
import cv2
# if error:
# pip install opencv-python
# use python 3.7 in your configs
import cap

import paho.mqtt.client as mqtt
import time

mqttBroker = "mqtt.eclipseprojects.io"
client = mqtt.Client("Simon")
client.connect(mqttBroker)
print("Connected to broker")

def CurrentTime():
    return int(time.time())

keepAliveTimer = 30
timer =1
previousKeepAlive, previousGreen, previousRed = CurrentTime()

# Capturing video through webcam
# 0 .. live camera input
webcam = cv2.VideoCapture(0)

# colors the camera is looking out for :
# - red
# - green

# Start a while loop
while (1):
    currentTime = CurrentTime()

    if  currentTime - previousKeepAlive >= keepAliveTimer:
        client.publish("BIPDemo/Messages", "KeepAlive")
        previousKeepAlive = currentTime
        print("KeepAlive sent")
    

  
    # Reading the video from the
    # webcam in image frames
    _, imageFrame = webcam.read()

    # Convert the imageFrame in
    # BGR(RGB color space) to
    # HSV(hue-saturation-value)
    # color space
    hsvFrame = cv2.cvtColor(imageFrame, cv2.COLOR_BGR2HSV)

    ###############
    #     RED     #
    ###############

    # Set range for red color
    red_lower = np.array([136, 87, 111], np.uint8)
    red_upper = np.array([180, 255, 255], np.uint8)
    # create/set a red mask
    red_mask = cv2.inRange(hsvFrame, red_lower, red_upper)

    ###############
    #    GREEN    #
    ###############

    # we set a range as well
    green_lower = np.array([57, 64, 55], np.uint8)
    green_upper = np.array([80, 255, 255], np.uint8)
    # we set a green mask
    green_mask = cv2.inRange(hsvFrame, green_lower, green_upper)

    kernal = np.ones((5, 5), "uint8")

    ###############
    #     RED     #
    ###############

    red_mask = cv2.dilate(red_mask, kernal)
    res_red = cv2.bitwise_and(imageFrame, imageFrame,
                              mask=red_mask)

    ###############
    #    GREEN    #
    ###############

    green_mask = cv2.dilate(green_mask, kernal)
    res_green = cv2.bitwise_and(imageFrame, imageFrame,
                                mask = green_mask)

    ###############
    #     RED     #
    ###############

    # Creating contour to track red color
    contours, hierarchy = cv2.findContours(red_mask,
                                           cv2.RETR_TREE,
                                           cv2.CHAIN_APPROX_SIMPLE)

    for pic, contour in enumerate(contours):
        area = cv2.contourArea(contour)
        if (area > 300):
            x, y, w, h = cv2.boundingRect(contour)
            if(currentTime - previousRed >= timer1):
                previousKeepAlive, previousRed = currentTime
                client.publish("BIPDemo/Messages", '1')
                keepAliveTimer = 30

            # draw a square
            imageFrame = cv2.rectangle(imageFrame, (x, y),
                                       (x + w, y + h),
                                       (0, 0, 255), 2)

            # put text above the square (in our case not really needed since we only classify one class)
            cv2.putText(imageFrame, "Ripe", (x, y),
                        cv2.FONT_HERSHEY_SIMPLEX, 1.0,
                        (0, 0, 255))

    ###############
    #    GREEN    #
    ###############

    contours, hierarchy = cv2.findContours(green_mask,
                                           cv2.RETR_TREE,
                                           cv2.CHAIN_APPROX_SIMPLE)

    for pic, contour in enumerate(contours):
        area = cv2.contourArea(contour)
        if (area > 300):
            if(currentTime - previousGreen >= timer1):
                client.publish("BIPDemo/Messages", '0')
                previousKeepAlive, previousGreen = currentTime
            

            x, y, w, h = cv2.boundingRect(contour)
            imageFrame = cv2.rectangle(imageFrame, (x, y),
                                       (x + w, y + h),
                                       (0, 255, 0), 2)

            cv2.putText(imageFrame, "Not Ripe", (x, y),
                        cv2.FONT_HERSHEY_SIMPLEX,
                        1.0, (0, 255, 0))

    # Program Termination
    cv2.imshow("Ripe Detection", imageFrame)
    if cv2.waitKey(10) & 0xFF == ord('q'):
        cap.release()
        cv2.destroyAllWindows()
        break
