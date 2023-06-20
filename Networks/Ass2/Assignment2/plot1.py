import numpy as np
import matplotlib.pyplot as plt
i=1	
x=[]
y=[]			
for line in open('output.txt', 'r'):
    lines = [i for i in line.split()]
    x.append(i)
    y.append(float(lines[0]))
    i=i+1
					      
plt.title("Variation in CW")
plt.xlabel('Update number')
plt.ylabel('CW value')
plt.ylim(0, max(max(y)+10,100))
plt.plot(x, y, c = 'g')
					  
plt.show()