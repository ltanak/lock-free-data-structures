from pathlib import Path
import pandas as pd
import numpy as np
import matplotlib.pyplot as plt

class LatencyPlot:

    def __init__(self, csv_path: Path, metric: str = "latencies"):
        self.csv_path: Path = csv_path
        self.data = pd.read_csv(csv_path)[metric].astype(float).values

    def _filtered_data(self, max_perc: float | None):
        if max_perc is None:
            return self.data, None

        cutoff = np.percentile(self.data, max_perc)
        filtered = self.data[self.data <= cutoff]
        return filtered, cutoff
    
    def _latency_bins(self, data, linear_max, linear_bins=200, log_bins=100):
        lin_bins = np.linspace(data.min(), linear_max, linear_bins)

        log_bins = np.logspace(
            np.log10(linear_max),
            np.log10(data.max()),
            log_bins,
        )
        return np.unique(np.concatenate([lin_bins, log_bins]))


    """
    Plots distribution of latencies on a logarithmic graph
    """
    def histogram(self, title="Latency Histogram", out: Path | None = None, max_perc: float | None = None):
        if max_perc is not None:
            cutoff = np.percentile(self.data, max_perc)
            data = self.data[self.data <= cutoff]
        else:
            cutoff = None
            data = self.data

        data = data[data > 0]

        xmin = data.min()
        xmax = data.max()
        span = xmax - xmin

        plt.figure()

        # very tight distribution (fast path) ---
        if span <= 50:  # ns
            bins = np.arange(xmin, xmax + 2)  # 1ns resolution
            plt.hist(data, bins=bins, edgecolor="black")
            plt.xlabel("Latency (ns)")
            plt.ylabel("Count")

        # wider distribution (mixed paths) ---
        else:
            bins = np.logspace(
                np.log10(xmin),
                np.log10(xmax),
                200,
            )
            plt.hist(data, bins=bins)
            plt.xscale("log")
            plt.yscale("log")
            plt.xlabel("Latency (ns)")
            plt.ylabel("Count")

        if cutoff is not None:
            title += f" (≤ p{max_perc}, cutoff={cutoff:.1f} ns)"

        plt.title(title)
        plt.grid(True, which="both", linestyle="--", alpha=0.4)

        if out is not None:
            out = Path(out)
            out.parent.mkdir(parents=True, exist_ok=True)
            plt.savefig(out, dpi=150, bbox_inches="tight")
        else:
            plt.show()

        plt.close()


    
    """
    Percentile curve
    """
    def cdf(self, title="Latency CDF", out: Path | None = None, max_perc: float | None = None,):
        if max_perc is not None:
            cutoff = np.percentile(self.data, max_perc)
            data = self.data[self.data <= cutoff]
        else:
            cutoff = None
            data = self.data

        data = np.sort(data)
        percentiles = np.linspace(0, max_perc or 100, len(data))

        plt.figure()
        plt.plot(percentiles, data)

        if cutoff is not None:
            title += f" (≤ p{max_perc})"

        plt.xlabel("Percentile")
        plt.ylabel("Latency (ns)")
        plt.title(title)
        plt.grid(True)

        if out is not None:
            out = Path(out)
            out.parent.mkdir(parents=True, exist_ok=True)
            plt.savefig(out, dpi=150, bbox_inches="tight")
        else:
            plt.show()

        plt.close()

    """
    Percentile summaries
    """
    def percentiles(self, ps=(50, 90, 99, 99.9), title="Latency Percentiles", out: Path | None = None, max_perc: float | None = None):
        data, cutoff = self._filtered_data(max_perc)
        values = np.percentile(data, ps)

        plt.figure()
        plt.bar([f"p{p}" for p in ps], values)
        plt.ylabel("Latency (ns)")
        plt.title(title)
        plt.grid(axis="y")
        if out is not None:
            out = Path(out)
            out.parent.mkdir(parents=True, exist_ok=True)
            plt.savefig(out, dpi=150, bbox_inches="tight")
        else:
            plt.show()

        plt.close()

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