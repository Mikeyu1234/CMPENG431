import os
import matplotlib.pyplot as plt

f = open("result.txt","r")
txt=f.readlines()
result = []
bestEDP=[]
bestTime=[]
x =[]
i=0
while len(txt) > i:
    
    
    if txt[i][:3]=="Ite":
        
        result.append(txt[i+1].strip().replace("proposedGeoEDP=","").replace(" bestEDP=","").replace(" proposedGeoTime=","").replace(" bestTime=","").split(","))
    i+=1
#result = proposed geoEDP bset EDP proposed GEO time bestTime

for i in result:
    bestTime.append(i[3])
    bestEDP.append(i[1])
for i in range(len(bestEDP)):
    x.append(i)
plt.title("bestEDP over iteration")
plt.xlabel("iteration")
plt.ylabel("bestEDP")
plt.scatter(x,bestEDP)
plt.plot(x,bestEDP)
plt.show()

#plot best time
plt.title("best time over iteration")
plt.xlabel("iteration")
plt.ylabel("bestTime")
plt.scatter(x,bestTime)
plt.plot(x,bestTime)
plt.show()

