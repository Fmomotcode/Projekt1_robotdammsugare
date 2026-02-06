import cv2 as cv
import numpy as np
import serial 
import time

arduino = serial.Serial('/dev/ttyUSB0', 9600, timeout=1)
time.sleep(2)

cap = cv.VideoCapture(0)
cap.set(cv.CAP_PROP_FRAME_WIDTH, 320)
cap.set(cv.CAP_PROP_FRAME_HEIGHT, 240)

kernel = np.ones((3,3), np.uint8) #stärker kanter och maskar

last_cmd = ""

def send_cmd(cmd):
    global last_cmd
    if cmd != last_cmd:
        arduino.write((cmd + '\n').encode())
        last_cmd = cmd

while True:
    ret, frame = cap.read()
    frame = cv.flip(frame, 0)
    if not ret:
        break


    h, w, _ = frame.shape
    roi = frame[int(h*0.6):h, :] #ROI stands for Region Of Interest, in this case, only the floor is analyzed, which helps separate objects from the floor.
    gray = cv.cvtColor(roi, cv.COLOR_BGR2GRAY)
    blur = cv.GaussianBlur(gray, (5,5), 0)

    kant = cv.Canny(blur, 60, 120)
    hsv = cv.cvtColor(roi, cv.COLOR_BGR2HSV) # HSV is used for color detection, LAB could also be used, but it works slightly differently.
    color_mask = cv.inRange(hsv, np.array([0, 40, 40]), np.array([180, 255, 255]))

    combined = cv.bitwise_and(kant, color_mask)
    kernel = np.ones((3,3), np.uint8)
    combined = cv.dilate(combined, kernel, iterations=1)

    contours, _ = cv.findContours(combined, cv.RETR_EXTERNAL, cv.CHAIN_APPROX_SIMPLE) # List of detected objects
    
    objekt = [] # Empty list that stores all detected objects
    for cnt in contours:
        area = cv.contourArea(cnt)
        if 50 < area < 1000:
            objekt.append(cnt)
            x, y, w, h = cv.boundingRect(cnt)
            cv.rectangle(roi, (x,y), (x+w,y+h), (0,0,255), 2)
            cv.putText(roi, "SKRÄP", (x,y-5), cv.FONT_HERSHEY_SIMPLEX, 0.4,(0,0,255), 1)

    #Select the first object in the list as the "target", so the robot does not try to move toward multiple objects at once
    if objekt:
        target = objekt[0]
        x, y, w, h = cv.boundingRect(target)
        cx = x + w // 2

        img_center = roi.shape[1] // 2
        # Simple steering logic:
        # Turn toward the trash: forward, right, or left
        if cx < img_center - 20:
            send_cmd("VÄNSTER")
        elif cx > img_center + 20:
            send_cmd("HÖGER")
        else:
            send_cmd("FRAM")
    else:
        # No trash ahead
        send_cmd("FRAM")

    cv.imshow('mask', combined)
    cv.imshow('video', gray)
    cv.imshow('Kanter', kant)
    cv.imshow('detektera', frame)

    if cv.waitKey(1) & 0xFF == ord('q'):
        break
cap.release()
cv.destroyAllWindows()