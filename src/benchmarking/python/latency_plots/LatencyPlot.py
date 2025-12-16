import pathlib
import pandas as pd
import numpy as np
import matplotlib.pyplot as plt

class LatencyPlot:

    def __init__(self, csv_path: pathlib, metric: str = "latencies"):
        self.csv_path: pathlib = csv_path
        self.data = pd.read_csv(csv_path)[metric].astype(float).values

    def plot_normal_dist(self, metric, label: str = "", ):
        if metric not in self.map:
            raise KeyError(f"Metric {metric} does not exist")
        data = self.map[metric]
        # plt.figure()
        # plt.plot(data)
        # plt.xlabel

    """
    Plots distribution of latencies on a logarithmic graph
    """
    def histogram(self, bins=200, log_scale=True, title="Latency Histogram"):
        plt.figure()
        plt.hist(self.data, bins=bins)
        if log_scale:
            plt.yscale("log")

        plt.xlabel("Latency (ns)")
        plt.ylabel("Count")
        plt.title(title)
        plt.grid(True)
        plt.show()
    
    """
    Percentile curve
    """
    def cdf(self, title="Latency CDF"):
        data_sorted = np.sort(self.data)
        percentiles = np.linspace(0, 100, len(data_sorted))

        plt.figure()
        plt.plot(percentiles, data_sorted)
        plt.xlabel("Percentile")
        plt.ylabel("Latency (ns)")
        plt.title(title)
        plt.grid(True)
        plt.show()

    """
    Percentile summaries
    """
    def percentiles(self, ps=(50, 90, 99, 99.9), title="Latency Percentiles"):
        values = np.percentile(self.data, ps)

        plt.figure()
        plt.bar([f"p{p}" for p in ps], values)
        plt.ylabel("Latency (ns)")
        plt.title(title)
        plt.grid(axis="y")
        plt.show()

    def summary(self):
        return {
            "count": len(self.data),
            "mean_ns": np.mean(self.data),
            "median_ns": np.median(self.data),
            "p90_ns": np.percentile(self.data, 90),
            "p99_ns": np.percentile(self.data, 99),
            "p99.9_ns": np.percentile(self.data, 99.9),
            "max_ns": np.max(self.data),
        }