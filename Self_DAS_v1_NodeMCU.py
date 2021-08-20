import gc
from machine import I2C, Pin
from struct import unpack
from time import sleep, ticks_ms
import network
import urequests

from bmp180 import BMP180
from esp8266_i2c_lcd import I2cLcd
gc.collect()
WiFi_ssid="Roscoe A30"
WiFi_pwd="N700MSG-BAFM"

ThingSpeak_Write_API_Key="MUQHY74J5VV7ILH9"

ardI2Caddress=0x39
lcdI2Caddress=0x38
bmpI2Caddress=0x77

ardData=[1,2,3,4,5,6,7,8,9]
bmpData=[1,2]
recData=[1,2,3,4,5,6,7,8,9,10,11]

tempData=bytearray(4)
humData=bytearray(4)
latData=bytearray(4)
lngData=bytearray(4)
dayData=bytearray(4)
monthData=bytearray(4)
yearData=bytearray(4)
hourData=bytearray(4)
minuteData=bytearray(4)
gc.collect()

toSend=bytearray(1)

count=1
currCount=1
currentMillis=ticks_ms()

i2c=I2C(scl=Pin(5), sda=Pin(4), freq=100000)

lcd = I2cLcd(i2c, lcdI2Caddress, 2, 16)

bmp180 = BMP180(i2c)
bmp180.oversample_sett = 2
bmp180.baseline = 101325
gc.collect()

def connectWiFi():
    gc.collect()
    ap_if=network.WLAN(network.AP_IF)
    ap_if.active(False)
    sta_if=network.WLAN(network.STA_IF)
    if not sta_if.isconnected():
        print("Connecting to Wifi...", end="")
        lcdClearAndPrintAtPos(0,1,"Connecting to:")
        lcdPrintAtPos(0,2,WiFi_ssid)
        sta_if.active(True)
        sta_if.connect(WiFi_ssid, WiFi_pwd)
        while not sta_if.isconnected():
            print(".", end="")
            sleep(1)
    if sta_if.isconnected():
        ok1=0
        while(ok1!=1):
            try:
                sendToArduino(2)
                ok1=1
            except:
                ok1=0
            
    print("\nNetwork config: ", sta_if.ifconfig())
       
    
def updateAllData():
    gc.collect()
    getArduinoData()
    getBMPData()
    
    recData[0]=ardData[0]
    recData[1]=ardData[1]
    recData[2]=ardData[2]
    recData[3]=ardData[3]
    recData[4]=ardData[4]
    recData[5]=ardData[5]
    recData[6]=ardData[6]
    recData[7]=ardData[7]
    recData[8]=ardData[8]
    recData[9]=bmpData[0]
    recData[10]=bmpData[1]
    
    

def getArduinoData():
    gc.collect()
    ok=0
    while(ok!=1):
        try:
            receivedData=i2c.readfrom(ardI2Caddress, 19)
            ok=1
            sendToArduino(3)
        except:
            ok=0
            
    print("Data Received")
    print(receivedData)
    tempData[0]=receivedData[0]
    tempData[1]=receivedData[1]
    tempData[2]=receivedData[2]
    tempData[3]=receivedData[3]
    humData[0]=receivedData[4]
    latData[0]=receivedData[5]
    latData[1]=receivedData[6]
    latData[2]=receivedData[7]
    latData[3]=receivedData[8]
    lngData[0]=receivedData[9]
    lngData[1]=receivedData[10]
    lngData[2]=receivedData[11]
    lngData[3]=receivedData[12]
    dayData[0]=receivedData[13]
    monthData[0]=receivedData[14]
    yearData[0]=receivedData[15]
    yearData[1]=receivedData[16]
    hourData[0]=receivedData[17]
    minuteData[0]=receivedData[18]
    gc.collect()
    
    t=str(unpack('<f',tempData)[0])
    h=str(unpack('<i', humData)[0])
    lat=str(unpack('<f', latData)[0])
    lng=str(unpack('<f', lngData)[0])
    day=str(unpack('<i', dayData)[0])
    month=str(unpack('<i', monthData)[0])
    year=str(unpack('<i', yearData)[0])
    hour=str(unpack('<i', hourData)[0])
    minute=str(unpack('<i', minuteData)[0])
        
    if(int(day)<10):
        day="0"+day
        
    if(int(month)<10):
        month="0"+month
        
    if(int(minute)<10):
        minute="0"+minute
        
    if(int(hour)<10):
        hour="0"+hour

    ardData[0]=t
    ardData[1]=h
    ardData[2]=lat
    ardData[3]=lng
    ardData[4]=day
    ardData[5]=month
    ardData[6]=year
    ardData[7]=hour
    ardData[8]=minute


def getBMPData():
    gc.collect()
    pres = bmp180.pressure
    altitude = bmp180.altitude
    
    bmpData[0]=pres
    bmpData[1]=altitude
    

def sendToArduino(a):
    toSend[0]=a
    i2c.writeto(ardI2Caddress, toSend)
    print("Sent data to Arduino")
    

def lcdClearAndPrintAtPos(q,w,e):
    gc.collect()
    lcd.clear()
    lcd.move_to(q,w)
    lcd.putstr(e)
    
def lcdPrintAtPos(q,w,e):
    lcd.move_to(q,w)
    lcd.putstr(e)

def displayOnLCD():
    global currCount
    gc.collect()
    if(count!=currCount):
        lcd.clear()
    if(count==1):
        lcd.move_to(0,0)
        lcd.putstr("Temperature:"+str(recData[0]))
        lcd.move_to(-4,2)
        lcd.putstr("Humidity   :"+str(recData[1])+" %")
        currCount=1
    if(count==2):
        lcd.move_to(0,0)
        lcd.putstr("Lat    : "+str(recData[2]))
        lcd.move_to(-4,2)
        lcd.putstr("Long   : "+str(recData[3]))
        currCount=2
    if(count==3):
        lcd.move_to(0,0)
        lcd.putstr("Pressure:"+str(recData[9]))
        lcd.move_to(-4,2)
        lcd.putstr("Altitude:"+str(recData[10]))
        currCount=3
    if(count==4):
        lcd.move_to(0,0)
        lcd.putstr("Date: "+str(recData[4])+"/"+str(recData[5])+"/"+str(recData[6]))
        lcd.move_to(-4,2)
        lcd.putstr("Time:      "+str(recData[7])+":"+str(recData[8]))
        currCount=4
  
def sendToTS():
    gc.collect()
    print('Invoking log webhook')
    print("ThingSpeak Write Key: ", ThingSpeak_Write_API_Key)
    url = "https://api.thingspeak.com/update?api_key="+ThingSpeak_Write_API_Key+"&field1="+str(recData[0])+"&field2="+str(recData[1])+"&field3="+str(recData[10])
    print("URL: ", url)
    response = urequests.get(url)
    if response.status_code < 400:
        print('Webhook invoked')
        sendToArduino(4)
    else:
        print('Webhook failed')

        
def run():
    connectWiFi()
    gc.collect()
    
    while True:
        gc.collect()
        print("\n\n", end="")
        updateAllData()
        displayOnLCD()
        sendToTS()
        print(recData[0])
        print(recData[1])
        print(recData[2])
        print(recData[3])
        print(recData[4], end="/")
        print(recData[5], end="/")
        print(recData[6])
        print(recData[7], end=":")
        print(recData[8])
        print(recData[9])
        print(recData[10])
        global count
        global currentMillis
        if(ticks_ms()-currentMillis>=4000):
            currentMillis=ticks_ms()
            if(count<=4):
                count=count+1
            else:
                count=1
        #sleep(1)
        
run()




