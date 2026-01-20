from pathlib import Path
import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
from benchmarking.python.plotting.utils import get_graph_dir

class LatencyPlot:
    def __init__(self, csv_path: Path):
        self.csv_path = csv_path
        df = pd.read_csv(csv_path)

        self.data = {
            "enqueue": df["enqueue_latency_ns"].astype(float).values,
            "dequeue": df["dequeue_latency_ns"].astype(float).values,
        }

    # outlier removal (IQR)
    def _remove_outliers_iqr(self, data, k=1.5):
        q1 = np.percentile(data, 25)
        q3 = np.percentile(data, 75)
        iqr = q3 - q1
        lo = q1 - k * iqr
        hi = q3 + k * iqr
        return data[(data >= lo) & (data <= hi)], (lo, hi)

    def histogram(self, metric="enqueue", title="Latency Histogram", out: Path | None = None, remove_outliers=True):
        data = self.data[metric]
        data = data[data > 0]

        cutoff_info = ""
        if remove_outliers:
            data, (lo, hi) = self._remove_outliers_iqr(data)
            cutoff_info = f" (IQR [{lo:.1f}, {hi:.1f}])"

        xmin, xmax = data.min(), data.max()
        span = xmax - xmin

        plt.figure()

        # tight fast-path
        if span <= 50:
            bin_width = 0.1  # ns resolution
            bins = np.arange(xmin, xmax + bin_width, bin_width)

            plt.hist(data, bins=bins, edgecolor="black")

            ax = plt.gca()
            ax.xaxis.set_major_locator(plt.MaxNLocator(nbins=15))
            ax.xaxis.set_major_formatter(plt.FormatStrFormatter('%.1f'))

            plt.xlabel("Latency (ns)")
            plt.ylabel("Count")

        # wide / tail-heavy
        else:
            bins = np.logspace(np.log10(xmin), np.log10(xmax), 200)
            plt.hist(data, bins=bins)
            plt.xscale("log")
            plt.yscale("log")
            plt.xlabel("Latency (ns)")
            plt.ylabel("Count")

        plt.title(f"{title} – {metric}{cutoff_info}")
        plt.grid(True, which="both", linestyle="--", alpha=0.4)

        if out:
            out.parent.mkdir(parents=True, exist_ok=True)
            plt.savefig(out, dpi=150, bbox_inches="tight")
        else:
            plt.show()

        plt.close()

    # CDF
    def cdf(self, metric="enqueue", title="Latency CDF", out: Path | None = None, remove_outliers=True):
        data = self.data[metric]

        if remove_outliers:
            data, _ = self._remove_outliers_iqr(data)

        data = np.sort(data)
        percentiles = np.linspace(0, 100, len(data))

        plt.figure()
        plt.plot(percentiles, data)
        plt.xlabel("Percentile")
        plt.ylabel("Latency (ns)")
        plt.title(f"{title} – {metric}")
        plt.grid(True)

        if out:
            out.parent.mkdir(parents=True, exist_ok=True)
            plt.savefig(out, dpi=150, bbox_inches="tight")
        else:
            plt.show()

        plt.close()

    # plot summary of factors
    def summary(self, metric="enqueue", remove_outliers=True):
        data = self.data[metric]
        if remove_outliers:
            data, _ = self._remove_outliers_iqr(data)

        return {
            "count": len(data),
            "mean_ns": np.mean(data),
            "median_ns": np.median(data),
            "p90_ns": np.percentile(data, 90),
            "p99_ns": np.percentile(data, 99),
            "max_ns": np.max(data),
        }

    # Plot all latency graphs
    def plot_all(self, out_dir: Path | None = None, remove_outliers=True):
        """
        Generate and save all latency plots (histogram and CDF) to the specified directory.
        
        Args:
            out_dir: Output directory. If None, uses default latency graphs directory.
            remove_outliers: Whether to remove outliers using IQR method.
        """
        if out_dir is None:
            out_dir = get_graph_dir("latencies")
        
        base_name = self.csv_path.stem
        self.histogram(metric="enqueue", out=out_dir / f"{base_name}_hist.png", remove_outliers=remove_outliers)
        self.cdf(metric="enqueue", out=out_dir / f"{base_name}_cdf.png", remove_outliers=remove_outliers)
