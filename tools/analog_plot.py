#!/usr/bin/python

# ################################################################################
# # showdata.py
# #
# # Display analog data from Arduino using Python (matplotlib)
# # 
# # electronut.in
# #
# ################################################################################

# import sys, serial
# import numpy as np
# from time import sleep
# from collections import deque
# from matplotlib import pyplot as plt

# # class that holds analog data for N samples
# class AnalogData:
#   # constr
#   def __init__(self, maxLen):
#     self.ax = deque([0.0]*maxLen)
#     self.maxLen = maxLen

#   # ring buffer
#   def addToBuf(self, buf, val):
#     if len(buf) < self.maxLen:
#       buf.append(val)
#     else:
#       buf.pop()
#       buf.appendleft(val)

#   # add data
#   def add(self, data):
#     assert(len(data) == 1)
#     self.addToBuf(self.ax, data[0])
    
# # plot class
# class AnalogPlot:
#   # constr
#   def __init__(self, analogData):
#     # set plot to animated
#     print("init plot")
#     plt.ion() 
#     self.axline, = plt.plot(analogData.ax)
#     plt.ylim([0, 400])

#   # update plot
#   def update(self, analogData):
#     print("update plot")
#     self.axline.set_ydata(analogData.ax)
#     plt.draw()

# # main() function
# def main():
#   # expects 1 arg - serial port string
#   if(len(sys.argv) != 2):
#     print 'Example usage: python showdata.py "/dev/tty.usbmodem411"'
#     exit(1)

#  #strPort = '/dev/tty.usbserial-A7006Yqh'
#   strPort = sys.argv[1];

#   # plot parameters
#   analogData = AnalogData(100)
#   analogPlot = AnalogPlot(analogData)

#   print 'plotting data...'

#   # open serial port
#   ser = serial.Serial(strPort, 115200)
#   while True:
#     try:
#       line = ser.readline()
#       try:
#         data = [float(val) for val in line.split()]
#         # print data
#         if (len(data) == 1):
#           print("read: " + str(data))
#           analogData.add([1])
#           analogPlot.update(analogData)
#       except:
#         print("except")
#         # skip line in case serial data is corrupt
#         pass
#     except KeyboardInterrupt:
#       print 'exiting'
#       break
#   # close serial
#   ser.flush()
#   ser.close()

# # call main
# if __name__ == '__main__':
#   main()


from threading import Thread
import serial
import time
import collections
import matplotlib.pyplot as plt
import matplotlib.animation as animation
import struct
import pandas as pd


class serialPlot:
    def __init__(self, serialPort = '/dev/ttyUSB0', serialBaud = 38400, plotLength = 100, dataNumBytes = 2):
        self.port = serialPort
        self.baud = serialBaud
        self.plotMaxLength = plotLength
        self.dataNumBytes = dataNumBytes
        self.rawData = bytearray(dataNumBytes)
        self.data = collections.deque([0] * plotLength, maxlen=plotLength)
        self.isRun = True
        self.isReceiving = False
        self.thread = None
        self.plotTimer = 0
        self.previousTimer = 0
        # self.csvData = []

        print('Trying to connect to: ' + str(serialPort) + ' at ' + str(serialBaud) + ' BAUD.')
        try:
            self.serialConnection = serial.Serial(serialPort, serialBaud, timeout=4)
            print('Connected to ' + str(serialPort) + ' at ' + str(serialBaud) + ' BAUD.')
        except:
            print("Failed to connect with " + str(serialPort) + ' at ' + str(serialBaud) + ' BAUD.')

    def readSerialStart(self):
        if self.thread == None:
            self.thread = Thread(target=self.backgroundThread)
            self.thread.start()
            # Block till we start receiving values
            while self.isReceiving != True:
                time.sleep(0.1)

    def getSerialData(self, frame, lines, lineValueText, lineLabel, timeText):
        currentTimer = time.perf_counter()
        self.plotTimer = int((currentTimer - self.previousTimer) * 1000)     # the first reading will be erroneous
        self.previousTimer = currentTimer
        timeText.set_text('Plot Interval = ' + str(self.plotTimer) + 'ms')
        value,  = struct.unpack('f', self.rawData)    # use 'h' for a 2 byte integer
        self.data.append(value)    # we get the latest data point and append it to our array
        lines.set_data(range(self.plotMaxLength), self.data)
        lineValueText.set_text('[' + lineLabel + '] = ' + str(value))
        # self.csvData.append(self.data[-1])

    def backgroundThread(self):    # retrieve data
        time.sleep(1.0)  # give some buffer time for retrieving data
        self.serialConnection.reset_input_buffer()
        while (self.isRun):
            self.serialConnection.readinto(self.rawData)
            self.isReceiving = True
            #print(self.rawData)

    def close(self):
        self.isRun = False
        self.thread.join()
        self.serialConnection.close()
        print('Disconnected...')
        # df = pd.DataFrame(self.csvData)
        # df.to_csv('/home/rikisenia/Desktop/data.csv')


def main():
    # portName = 'COM5'     # for windows users
    portName = '/dev/ttyUSB0'
    portName = "/dev/cu.wchusbserial14110"
    baudRate = 38400
    baudRate = 115200
    maxPlotLength = 100
    dataNumBytes = 4        # number of bytes of 1 data point
    s = serialPlot(portName, baudRate, maxPlotLength, dataNumBytes)   # initializes all required variables
    s.readSerialStart()                                               # starts background thread

    # plotting starts below
    pltInterval = 50    # Period at which the plot animation updates [ms]
    xmin = 0
    xmax = maxPlotLength
    ymin = -(1)
    ymax = 1
    fig = plt.figure()
    ax = plt.axes(xlim=(xmin, xmax), ylim=(float(ymin - (ymax - ymin) / 10), float(ymax + (ymax - ymin) / 10)))
    ax.set_title('Arduino Analog Read')
    ax.set_xlabel("time")
    ax.set_ylabel("AnalogRead Value")

    lineLabel = 'Potentiometer Value'
    timeText = ax.text(0.50, 0.95, '', transform=ax.transAxes)
    lines = ax.plot([], [], label=lineLabel)[0]
    lineValueText = ax.text(0.50, 0.90, '', transform=ax.transAxes)
    anim = animation.FuncAnimation(fig, s.getSerialData, fargs=(lines, lineValueText, lineLabel, timeText), interval=pltInterval)    # fargs has to be a tuple

    plt.legend(loc="upper left")
    plt.show()

    s.close()


if __name__ == '__main__':
    main()