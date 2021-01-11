#!/usr/local/bin/python3
# Pouyan Keshavarzian
# EPFL STI IMT AQUA Dec 2020
#
# annotate matplotlib: 
# https://matplotlib.org/3.1.1/tutorials/text/text_intro.html
# gnuplot for python
# https://pypi.org/project/py-gnuplot/
# https://queirozf.com/entries/add-labels-and-text-to-matplotlib-plots-annotation-examples

import matplotlib.pyplot as plt
import csv
plt.rcParams.update({
    "text.usetex": True,
    "font.family": "serif",
    "font.serif": ["Computer Modern Roman"],
    "axes.linewidth" : 1.0, 
})
x = []
y = []

with open('dataIn/csv/spadPulse.csv','r') as csvfile:
    next(csvfile)
    plots = csv.reader(csvfile, delimiter=',')
    for row in plots:
        x.append(float(row[0]))
        y.append(float(row[1]))

fig = plt.figure()
fig.suptitle('figure suptitle', fontsize=14,)
pulse = fig.add_subplot(211)
pulse.plot(x,y, label='Loaded from file!')
pulse.text(1e-7, 0.5, r'an equation: $E=mc^2$', fontsize=15)
pulse.set_xlabel("$\mu$")
pulse.set_ylabel('y')
pulse.set_title('Interesting Graph\nCheck it out')
pulse.legend()
# output settings
ax = fig.add_subplot(212)
fig.subplots_adjust(top=0.85)
ax.set_title('axes title')
ax.set_xlabel('xlabel')
ax.set_ylabel('ylabel')
ax.text(3, 8, 'boxed italics text in data coords', style='italic',
        bbox={'facecolor': 'red', 'alpha': 0.5, 'pad': 10})
ax.text(2, 6, r'an equation: $E=mc^2$', fontsize=15)
ax.text(3, 2, 'unicode: Institut für Festkörperphysik')
ax.text(0.95, 0.01, 'colored text in axes coords',
        verticalalignment='bottom', horizontalalignment='right',
        transform=ax.transAxes,
        color='green', fontsize=15)

ax.plot([2], [1], 'o')
ax.annotate('annotate', xy=(2, 1), xytext=(3, 4),
            arrowprops=dict(facecolor='black', shrink=0.05))

ax.axis([0, 10, 0, 10])

fig.savefig('plotting/output/png/spadPulsefFromCadenceCSV.png', format='png', dpi=200,
        facecolor='w', edgecolor='black',
        orientation='portrait')
#plt.savefig('outputs/spadPulseCadence.eps', format='eps')

#plt.show()

import tikzplotlib

tikzplotlib.save("plotting/output/tex/tikzplotlibtest.tex")
