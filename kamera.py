import cv2 as cv
import numpy as np
import serial 
import time

arduino = serial.Serial('/dev/ttyUSB0', 9600, timeout=1)
time.sleep(2)

cap = cv.VideoCapture(0)
cap.set(cv.CAP_PROP_FRAME_WIDTH, 320)
cap.set(cv.CAP_PROP_FRAME_HEIGHT, 240)

kernel = np.ones((3,3), np.uint8) #stäker kanter och maskar

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
    roi = frame[int(h*0.6):h, :] #roi står för region of interest, endast golv i det här fallet
    gray = cv.cvtColor(roi, cv.COLOR_BGR2GRAY)
    blur = cv.GaussianBlur(gray, (5,5), 0)

    kant = cv.Canny(blur, 60, 120)
    hsv = cv.cvtColor(roi, cv.COLOR_BGR2HSV) #fråga Huke om man ska använda HSV eller om LAB är bättre
    color_mask = cv.inRange(hsv, np.array([0, 40, 40]), np.array([180, 255, 255]))

    combined = cv.bitwise_and(kant, color_mask)
    kernel = np.ones((3,3), np.uint8)
    combined = cv.dilate(combined, kernel, iterations=1)

    contours, _ = cv.findContours(combined, cv.RETR_EXTERNAL, cv.CHAIN_APPROX_SIMPLE) #lista med hittade objekt
    
    objekt = [] #här är våran lista med alla objekt som dekteras 
    for cnt in contours:
        area = cv.contourArea(cnt)
        if 50 < area < 1000:
            objekt.append(cnt)
            x, y, w, h = cv.boundingRect(cnt)
            cv.rectangle(roi, (x,y), (x+w,y+h), (0,0,255), 2)
            cv.putText(roi, "SKRÄP", (x,y-5), cv.FONT_HERSHEY_SIMPLEX, 0.4,(0,0,255), 1)

    # Här väljer vi första objektet i listan som "mål" så att den inte försöker ta sig till alla föremål samtidigt
    if objekt:
        target = objekt[0]
        x, y, w, h = cv.boundingRect(target)
        cx = x + w // 2

        img_center = roi.shape[1] // 2
        # Enkel styrlogik: sväng mot skräpet
        if cx < img_center - 20:
            send_cmd("VÄNSTER")
        elif cx > img_center + 20:
            send_cmd("HÖGER")
        else:
            send_cmd("FRAM")
    else:
        # Inget skräp syns
        send_cmd("FRAM")

    cv.imshow('mask', combined)
    cv.imshow('video', gray)
    cv.imshow('Kanter', kant)
    cv.imshow('detektera', frame)

    if cv.waitKey(1) & 0xFF == ord('q'):
        break
cap.release()
cv.destroyAllWindows()


#Det du kan göra är att den väljer ett objekt/skräp i taget så att den åker först till ena skräpet