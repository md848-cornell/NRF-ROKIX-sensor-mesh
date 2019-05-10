import matplotlib.pyplot as plt
import numpy as np
from mpl_toolkits.mplot3d import Axes3D
import math


G1_M = 16000
BOUND = 10
VAR = 5


class ShapePlot:

	def __init__(self):
		self.plt3d = plt.figure().gca(projection='3d')

	def plot(self,vecs,points):
		if len(vecs) != len(points):
			print('error - plot_surfaces - points and vecs not equal length')
			return
		zs = []

		# create x,y
		xx, yy = np.meshgrid(range(-1*BOUND,BOUND), range(-1*BOUND,BOUND))

		for i in range(len(vecs)):
			vec = vecs[i]
			center_point = points[i]

			vec = normalize(vec)
			point  = np.array(center_point)
			normal = np.array(vec)

			d = -point.dot(normal)

			# gaussian distribution
			g = 2 / (math.pi*(VAR**2)) * \
			np.exp(-1.0*((xx-point[0])**2)/(2*(VAR**2))\
			+ -1.0*((yy-point[1])**2)/(2*(VAR**2))\
			)

			# calculate corresponding z
			zi = (-normal[0] * xx - normal[1] * yy - d) * 1. / normal[2]


			zs += [np.multiply(g, zi)]

		z = (1.0*sum(zs))/len(zs)
		# plot the surface
		self.plt3d.cla()
		self.plt3d.plot_surface(xx, yy, z)
		plt.show(block=False)

def plot_surfaces(vecs, points, plotblock=True):
	if len(vecs) != len(points):
		print('error - plot_surfaces - points and vecs not equal length')
		return
	zs = []

	# create x,y
	xx, yy = np.meshgrid(range(-1*BOUND,BOUND), range(-1*BOUND,BOUND))

	for i in range(len(vecs)):
		vec = vecs[i]
		center_point = points[i]

		vec = normalize(vec)
		point  = np.array(center_point)
		normal = np.array(vec)

		d = -point.dot(normal)

		# gaussian distribution
		g = 2 / (math.pi*(VAR**2)) * \
		np.exp(-1.0*((xx-point[0])**2)/(2*(VAR**2))\
		+ -1.0*((yy-point[1])**2)/(2*(VAR**2))\
		)

		# calculate corresponding z
		zi = (-normal[0] * xx - normal[1] * yy - d) * 1. / normal[2]


		zs += [np.multiply(g, zi)]

	z = (1.0*sum(zs))/len(zs)
	# plot the surface
	plt3d = plt.figure().gca(projection='3d')
	plt3d.plot_surface(xx, yy, z)
	plt.show(block=plotblock)

def plot_surface(vec, center_point=[0,0,0], plotblock=True):
	'''
	https://stackoverflow.com/questions/3461869/plot-a-plane-based-on-a-normal-vector-and-a-point-in-matlab-or-matplotlib
	'''
	vec = normalize(vec)
	point  = np.array(center_point)
	normal = np.array(vec)

	d = -point.dot(normal)

	# create x,y
	xx, yy = np.meshgrid(range(BOUND), range(BOUND))

	# calculate corresponding z
	print('vec:', vec)
	print('normal:', normal)
	z = (-normal[0] * xx - normal[1] * yy - d) * 1. / normal[2]

	# plot the surface
	plt3d = plt.figure().gca(projection='3d')
	plt3d.plot_surface(xx, yy, z)
	plt.show(block=plotblock)


def normalize(vec):
		vec = [(x*1.0) / G1_M for x in vec]
		return vec



def test():
	import csv
	sp = ShapePlot()
	f = open('data.csv', 'r')
        read = csv.reader(f)
        data = []
        for row in read:
                data += [row]
        f.close()
	pos = [0,0,0]
	i = 0
	while i < len(data):
		row = data[i]
		vec = [row[4], row[6], row[8]]
		vec = [float(x) for x in vec]
		plot_surface(vec, center_point=pos)
		#plot_surfaces([vec], [pos])
		raw_input('')
		i += 100


if __name__ == '__main__':
	test()
