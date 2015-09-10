'''
Created on 29.11.2014

@author: Sebastian
'''

import numpy as np
import matplotlib
matplotlib.use('TkAgg')
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D
from mpl_toolkits.mplot3d.art3d import Poly3DCollection

font = {'family' : 'serif',
        'weight' : 'normal',
        'size'   : 14}

matplotlib.rc('font', **font)


fig = plt.figure()
ax = fig.gca(projection='3d')
z=40/16.0*10/2
r=[[-20,-z,-50],[20,-z,-50],[20,z,-50],[-20,z,-50]]
verts=[]
xs=[0,0,0]
for i in xrange(len(r)):
    ax.text(r[i][0]-0.5,r[i][1]+0.5,r[i][2],'(%.1f,%.1f,%.1f)'%(r[i][0],r[i][1],r[i][2]))
    plt.plot([r[i][0],xs[0]], [r[i][1],xs[1]], zs=[r[i][2],xs[2]],color='b')
    a=np.array([r[i],r[(i+1)%4]]).transpose()
    plt.plot(a[0],a[1],a[2],color='r')
    verts = [[r[i],r[(i+1)%4],xs]]
    ax.add_collection3d(Poly3DCollection(verts,facecolors='w', linewidths=1, alpha=0.5))


d=2*np.array(r)    
for i in xrange(len(d)):
    ax.text(d[i][0]-0.5,d[i][1]+0.5,d[i][2],'(%.1f,%.1f,%.1f)'%(d[i][0],d[i][1],d[i][2]))
    plt.plot([r[i][0],d[i][0]], [r[i][1],d[i][1]], zs=[r[i][2],d[i][2]],color='r')
    a=np.array([d[i],d[(i+1)%4]]).transpose()
    plt.plot(a[0],a[1],a[2],color='r')

ax.text(-2,2,0,'(%.1f,%.1f,%.1f)'%(0,0,0))

plt.xlabel('x in cm')
plt.ylabel('y in cm')
ax.set_zlabel('z in cm')
    
print(verts)
plt.show()
