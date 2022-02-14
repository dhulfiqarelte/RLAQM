import glob
import matplotlib
import matplotlib.pyplot as pyplt
import glob
import pandas


baseFolder='log'


def setParams():
	matplotlib.rcParams['pdf.fonttype'] = 42
	matplotlib.rcParams['ps.fonttype'] = 42
	matplotlib.rcParams['agg.path.chunksize'] = 10000
	#	 matplotlib.rcParams['text.usetex'] = True
	params = {'legend.fontsize': 'medium',
			 'figure.figsize': (14, 12),
			'axes.labelsize': 'x-large',
			'axes.titlesize':'x-large',
			'xtick.labelsize':'x-large',
			'ytick.labelsize':'x-large'}
	pyplt.rcParams.update(params)

def plot(number="*"):
	#print ("The number of the best episode : %d <--" % number)
	
	global baseFolder
	setParams()

	for f in glob.glob(baseFolder+"/rewards-Best.csv"):
		newFilename = f[:-4].split("/")[-1]
		df = pandas.read_csv(f,sep=';')
		fig2 = pyplt.figure()
		ax2 = fig2.add_subplot(411)
		ax3 = fig2.add_subplot(412)
		ax4 = fig2.add_subplot(413)
		ax5 = fig2.add_subplot(414)

		#for lab in ['delayMs','Util','dropProb','reward']:
		df.plot('time','delayMs',label='delayMs', ax=ax2,alpha=0.5, color='red')
		df.plot('time','Util',label='Util', ax=ax3,alpha=0.5, color='green')
		df.plot('time','dropProb',label='dropProb', ax=ax4,alpha=0.5, color='blue')
		#df.plot('time','Dq',label='Dq', ax=ax5,alpha=0.5)
		df.plot('time','reward',label='reward', ax=ax5,alpha=0.5)
#		ax2.legend()
#		ax2.grid()
		pyplt.savefig(baseFolder+"/"+newFilename+".jpg",format='jpg', dpi=500,bbox_inches='tight')
		pyplt.close(fig2)

if __name__ == '__main__':
	plot("Base")