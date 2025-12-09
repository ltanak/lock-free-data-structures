"""
Eventually this will have matplotlib code to create graphs on the outputs from the benchmarking framework

Design to be thought about, but currently will write data to CSV which python will read.

This part doesn't have to be quick at all (as this is processed after the benchmarks), hence done in Python as good libraries

"""

import pathlib
import pandas as pd
import numpy as np
import matplotlib.pyplot as plt

class LatencyPlot:

    def __init__(self, csv_path: pathlib):
        self.csv_path: pathlib = csv_path
        self.df = pd.read_csv(csv_path)
        self.map = {}
        for col in self.df.columns:
            self.map[col] = self.df[col].astype(float).values

    def plot_normal_dist(self, metric, label: str = "", ):
        if metric not in self.map:
            raise KeyError(f"Metric {metric} does not exist")
        data = self.map[metric]
        # plt.figure()
        # plt.plot(data)
        # plt.xlabel

    def plot_histogram(self, metric, label: str = ""):
        return
    
    def plot_percentiles(self, metric, lable: str = ""):
        return