import numpy as np
import pandas as pd
import plotly.graph_objects as go
import serial

port = serial.Serial('/dev/ttyACM0')
if(port):
    print("Serial connected", port.name)
port.baudrate = 115200
sampleArr = np.zeros((0,))
bungus = True

fig = go.FigureWidget()
fig.add_scatter()
fig.update_layout(
    xaxis_title='Frequency (Hz)',
    yaxis_title='Amplitude (V)',
    width=1000,
    height=800,
    template='plotly_dark',
)

while bungus:
    # try:
        # portList = serial.tools.list_ports.comports()
        if(np.size(sampleArr) > 200000):
            fig = go.Line(sampleArr, x="Time", y="IR Amplitude")
            fig.show()
            bungus = False
        # np.append(sampleArr, port.readline().decode())
        sampleArr = np.append(sampleArr, int(port.readline().decode('ascii').strip('\r\n')))
        print(sampleArr)
        # if(np.size(sampleArr) > 0):
        #     print(sampleArr[-1])
        
    # except:
    #     print("interrupt")
    #     port.close()
    #     break