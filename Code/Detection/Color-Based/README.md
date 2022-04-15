# Color Based Ripeness Detection

For now our Project will be using the Colors of the incoming Item to deceide
wether or not the object is ripe or not. 

## First Steps

### Requirements

For this Implementation to work u will need the following: 
- Numpy
- OpenCV 
- Python 3.x version
(The Version that worked best for me was 3.7)

## Installation of OpenCV

Installing OpenCV in ur Project IDE works with the following command: 
> pip install opencv-python

Importing the Module into the project is the following line:
> import cv2

But basically the installation should be done by running the requirements.txt

## Code Explanation
(Explanation based on only one color - the second color is the same but values changed)

First step of detecting any kind of material is to get some type of input. Since we work with an active live webcam we have to capture live input.
To assure that we add the following line of code: 
```py

webcam = cv2.VideoCapture(0)

```
0 stands for live video capture through a current webcam. If you want to define a camera (IN case the program is not able to work or detect the Zed 2 Camera) please look into the following documentation of ways to define an input stream : https://docs.opencv.org/3.4/d8/dfe/classcv_1_1VideoCapture.html#ad890d4783ff81f53036380bd89dd31aa

Now we are starting a while-loop. The while loop is there to loop through the frames of the current input material. Why is that needed? To work with video material we have to capture and analyze each frame individually. Thats why after opening the while-loop we capture a frame and save it in a variable.

```py

_, imageFrame = webcam.read() # we are READING the video step by step => frame by frame, as we read a sentence word by word

```

The frame we now have saved will be converted to a different color spectrum. We get the frame in a RGB color space and turn it into hue-saturation value color space (short: HSV). But why do we convert from RGB to HSV ? The answer is simple. HSV is used in computer vision since it separates luma, or the image intensity, from chroma or the color information. In computer vision you often want to separate color components from intensity for various reasons, such as robustness to lighting changes, or removing shadows. You may ask *But arent there other color spaces that do the exact same thing as HSV? WHy not use those?*, Well because the function to convert RGB to HSV is widely known and much easier to implement (as far as my knowledge goes).

```py

 hsvFrame = cv2.cvtColor(imageFrame, cv2.COLOR_BGR2HSV)

```

Now we are taking care of the Colors. The first step is to *define* the colors we want to detect. Therefore we have to define the lower and upper border of the color we are looking for ( Color Borders are important since they define until which shade and intensity of a color we are going for ). Those borders are now set into a mask - the mask of a color is just a little variable that will check if the a frame has an some values that lie inbetween red_lower and red_upper.

```py

    red_lower = np.array([136, 87, 111], np.uint8)
    red_upper = np.array([180, 255, 255], np.uint8)
    red_mask = cv2.inRange(hsvFrame, red_lower, red_upper)

```


If you want to try out different colors to get used to the program or change the values: 
- Website to convert RGB to HSV : https://www.rapidtables.com/convert/color/rgb-to-hsv.html 
- And here is a little example of how the HSV color spectrum works (for better visualization and understanding)
![image](https://user-images.githubusercontent.com/71399699/163532085-ca495dae-5ee6-4c1c-b7f9-66ec64c08d20.png)

Now this *red_mask* is used to find contours of the objects that suit in the given spectrum of our mask.

```py

    contours, hierarchy = cv2.findContours(red_mask,
                                           cv2.RETR_TREE,
                                           cv2.CHAIN_APPROX_SIMPLE)

```

The two other Parameters in the function *cvs.findContours* are two different modes/methods:
- **cv2.RETR_TREE** : finds all the promising contour lines and reconstructs a full hierarchy of nested contours.
- **cv2.CHAIN_APPROX_SIMPLE** : returns only the endpoints that are necessary for drawing the contour line

The function *cv2.findContours()* returns a list of found contours in our captured frame. With a for loop we iterate through the given contours and draw rectangles around those.

```py

    for pic, contour in enumerate(contours):
        area = cv2.contourArea(contour)
        if (area > 300):
            x, y, w, h = cv2.boundingRect(contour)

            # draw a square
            imageFrame = cv2.rectangle(imageFrame, (x, y),
                                       (x + w, y + h),
                                       (0, 0, 255), 2)

            # put text above the square (in our case not really needed since we only classify one class)
            cv2.putText(imageFrame, "Ripe", (x, y),
                        cv2.FONT_HERSHEY_SIMPLEX, 1.0,
                        (0, 0, 255))

```

Since this program is an open while-loop ( meaning it can only be terminated by an error or a user input ) we have to make sure that the user can terminate the running program. Since I was working with my laptop I deicded to terminate it in the most classic way: 

```py

    if cv2.waitKey(10) & 0xFF == ord('q'):
        cap.release()
        cv2.destroyAllWindows()
        break

```

Sometimes this may not work: try to remove cap.release() then the issue might be gone. In our case we would hae to either change the input for the termination to work with our prototype, or keep a running prototype that terminates the program if the entire system is shut down. (No input)
