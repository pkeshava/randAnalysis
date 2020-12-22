#!/usr/local/opt/python@3.8/bin/python3
# Pouyan Keshavarzian
# EPFL STI IMT AQUA Dec 2020

import pandas
df = pandas.read_csv('dataIn/csv/SPAD_tester.csv')
#print(df)
df=df.loc[:, "SPAD_tester.Anode"]
print(df)

# References
# panda parse : https://realpython.com/python-csv/#parsing-csv-files-with-the-pandas-library
# indexing : https://pythonhow.com/accessing-dataframe-columns-rows-and-cells/
