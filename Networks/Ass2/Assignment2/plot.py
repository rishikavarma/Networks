import numpy as np
import matplotlib.pyplot as plt
import sys, os, getopt
a_ki=[1,4]
a_km=[1,1.5]
a_kn=[0.5,1]
a_kf=[0.1,0.3]
a_ps=[0.01, 0.0001]
for ki in a_ki:
	for km in a_km:
		for kn in a_kn:
			for kf in a_kf:
				for ps in a_ps: 
					os.system('./cw -i '+str(ki)+' -m '+str(km)+' -n '+str(kn)+' -f '+str(kf)+' -s '+str(ps)+' -T 10000 -o output.txt')	
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
					txt="Ki= "+str(ki)+" Km= "+str(km)+" Kn= "+str(kn)+" Kf= "+str(kf)+" Ps= "+str(ps)
					plt.figtext(0.5, 0.005, txt, wrap=True, horizontalalignment='center', fontsize=12)  
					plt.show()