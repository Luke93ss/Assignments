import pandas as pd
import numpy as np
import yfinance as yf
import datetime as dt
from pandas_datareader import data as pdr
import time as clock
import matplotlib.pyplot as plt

yf.pdr_override()



startyear = 2022
startmonth = 9
startday = 12

start=dt.datetime(startyear, startmonth, startday)
now = dt.datetime.now()

realestate = ['GMG.AX','SCG.AX','MGR.AX','CHC.AX','CLW.AX','NSR.AX',
              'CNI.AX','HMC.AX','ARF.AX','CQR.AX',
              'LIC.AX','DXS.AX']



pos=0
num=0
percentchange = []
moola = 20000
total_fees = 0
data = []
companies = len(realestate)
positive = 0

for i in realestate:

    df = pdr.get_data_yahoo(i,start, now, interval='1d')
    df["Ema2min"] = round(df.iloc[:,4].ewm(span=2, adjust=False).mean(),3)
    df["Ema5min"] = round(df.iloc[:,4].ewm(span=5, adjust=False).mean(),3)
    df = df[5:]
    data.append(df)
  



for i in df.index:

    try:
        close = data[3]["Adj Close"][i]
    except:
        close = 0
    for j in data:
        if j["Ema2min"][i] > j["Ema5min"][i]:
            positive+=1
        
        

    if positive > (companies//2 + 2):
        if pos==0:
            bp=close
            pos=1
            stk_purc = moola//bp
            moola -= (stk_purc*bp + 10)
            total_fees+=10
            print("**Buying Now at " + str(bp)+"**")
            print("Money: " + str(moola))
            plt.plot(i,bp,'g^',markersize=12)
    elif positive < (companies//2 - 2):
        if pos==1:
            pos=0
            sp=close
            print("**Selling now at " + str(sp)+"**")
            moola += (stk_purc*sp - 10)
            print("Money: " + str(moola))
            total_fees+=10
            pc = (sp/bp-1)*100
            percentchange.append(pc)
            plt.plot(i,sp,'rv',markersize=12)
            
    if (num==df["Adj Close"].count()-1 and pos==1):
        pos=0
        sp=close
        print("Selling now at "+str(sp))
        pc=(sp/bp-1)*100
        percentchange.append(pc)
        total_fees+=10
        moola += (stk_purc*sp - 10)
        plt.plot(i,sp,'rv',markersize=12)

    positive = 0
    num+=1

      
for i in df.index:
    

    ema200 = df["Ema200"][i]
   
    close = df["Adj Close"][i]
    
    print("EMA200: " + str(ema200))
    print("Close: " + str(close))
    print("Date: " + str(i))

    clock.sleep(2)


    if df["Adj Close"][i] > ema200:
        
        if pos==0:
            bp=close
            pos=1
            stk_purc = moola//bp
            moola -= (stk_purc*bp + 10)
            total_fees+=10
            print("**Buying Now at " + str(bp)+"**")
            print("Money: " + str(moola))
            plt.plot(i,bp,'g^',markersize=12)
            
        
        
    elif df["Adj Close"][i] < ema200:
        
        if pos==1:
            pos=0
            sp=close
            print("**Selling now at " + str(sp)+"**")
            moola += (stk_purc*sp - 10)
            print("Money: " + str(moola))
            total_fees+=10
            pc = (sp/bp-1)*100
            percentchange.append(pc)
            plt.plot(i,sp,'rv',markersize=12)
            
    
    
    if (num==df["Adj Close"].count()-1 and pos==1):
        pos=0
        sp=close
        print("Selling now at "+str(sp))
        pc=(sp/bp-1)*100
        percentchange.append(pc)
        total_fees+=10
        moola += (stk_purc*sp - 10)
        plt.plot(i,sp,'rv',markersize=12)

    
    num+=1
    count+=1
    

print(percentchange)

gains=0
ng=0
nl=0
losses=0
totalR=1


for i in percentchange:
    if i > 0:
        gains += i
        ng+=1
    else:
        losses += i
        nl+=1
    totalR = totalR*((i/100)+1)

totalR = round((totalR-1)*100,2)

if(ng>0):
    avgGain=gains/ng
    maxR=str(max(percentchange))
else:
    avgGain=0
    maxR="undefined"

if(nl>0):
    avgLoss=losses/nl
    maxL=str(min(percentchange))
    ratio=str(-avgGain/avgLoss)
else:
    avgLoss=0
    maxL="undefined"
    ratio="inf"

if (ng>0 or nl > 0):
    battingAvg=ng/(ng+nl)
else:
    battingAvg = 0

print()
print("Results for "+ stock + " going back to "+str(df.index[0])+", Sample size: "+str(ng+nl)+" trades")
print("Batting Avg: "+str(battingAvg))
print("Gain/loss ratio: "+ ratio)
print("Average Gain: "+str(avgGain))
print("Average Loss: "+str(avgLoss))
print("Max Return: "+maxR)
print("Max Loss: "+maxL)
print("Total return over "+str(ng+nl)+ " trades: "+str(totalR) + "%")
print("Total cash: " + str(moola))
print("Total_fees: " +str(total_fees))
plt.plot(df.index,df["Adj Close"])
plt.plot(df.index,df["Ema200"],'r')
plt.show()

