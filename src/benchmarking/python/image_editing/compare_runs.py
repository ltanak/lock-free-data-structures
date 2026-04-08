import sys
import argparse
from pathlib import Path
import numpy as np
import pandas as pd
import matplotlib.pyplot as plt

PROJECT_SRC = Path(__file__).resolve().parents[3]
if str(PROJECT_SRC) not in sys.path:
    sys.path.insert(0, str(PROJECT_SRC))

from benchmarking.python.plotting.utils import get_csv_by_run_id

"""
overlay latency data from multiple benchmark runs onto shared plots.
- given list of run IDs, loads the matching latency CSV for each run 
- produces comparison images (CDF, histogram, percentile bars)
- saves results in directory
"""
class RunComparator:

    def __init__(self,
        run_ids: list[str],
        labels: list[str] | None = None,
        metric: str = "enqueue",
        remove_outliers: bool = True,
        out_dir: Path | None = None,
    ):
        if not run_ids:
            raise ValueError("At least one run_id is required")
        if labels is not None and len(labels) != len(run_ids):
            raise ValueError("labels length must match run_ids length")

        self.run_ids = run_ids
        self.labels = labels or list(run_ids)
        self.metric = metric
        self.remove_outliers = remove_outliers
        self.out_dir = out_dir or Path(__file__).resolve().parent

        col = f"{metric}_latency_ns"
        self.series: list[np.ndarray] = []
        for rid in run_ids:
            files = get_csv_by_run_id(rid)
            if "latencies" not in files:
                raise FileNotFoundError(f"No latencies CSV found for run_id {rid}")
            df = pd.read_csv(files["latencies"])
            if col not in df.columns:
                raise ValueError(f"Column {col} missing from {files['latencies']}")
            data = df[col].astype(float).values
            data = data[data > 0]
            if self.remove_outliers:
                data = self._iqr_filter(data)
            self.series.append(data)

        cmap = plt.get_cmap("tab10")
        self.colours = [cmap(i % 10) for i in range(len(run_ids))]

    @staticmethod
    def _iqr_filter(data: np.ndarray, k: float = 1.5) -> np.ndarray:
        q1 = np.percentile(data, 25)
        q3 = np.percentile(data, 75)
        iqr = q3 - q1
        lo, hi = q1 - k * iqr, q3 + k * iqr
        return data[(data >= lo) & (data <= hi)]

    def _slug(self) -> str:
        return "_".join(self.run_ids)

    # ------------------------------------------------------------------ CDF
    def plot_cdf(self) -> Path:
        fig, ax = plt.subplots(figsize=(9, 5))
        for data, label, colour in zip(self.series, self.labels, self.colours):
            sorted_data = np.sort(data)
            percentiles = np.linspace(0, 100, len(sorted_data))
            ax.plot(percentiles, sorted_data, label=label, color=colour, linewidth=1.8)

        ax.set_xlabel("Percentile")
        ax.set_ylabel("Latency (ns)")
        ax.set_title(f"Latency CDF overlay - {self.metric}")
        ax.grid(True, linestyle="--", alpha=0.4)
        ax.legend(title="Run")

        out = self.out_dir / f"compare_cdf_{self.metric}_{self._slug()}.png"
        out.parent.mkdir(parents=True, exist_ok=True)
        fig.savefig(out, dpi=150, bbox_inches="tight")
        plt.close(fig)
        return out

    def plot_histogram(self, bins: int = 120) -> Path:
        fig, ax = plt.subplots(figsize=(9, 5))

        combined_min = min(d.min() for d in self.series)
        combined_max = max(d.max() for d in self.series)
        span = combined_max - combined_min

        if span > 50 and combined_min > 0:
            bin_edges = np.logspace(np.log10(combined_min), np.log10(combined_max), bins)
            log_x = True
        else:
            bin_edges = np.linspace(combined_min, combined_max, bins)
            log_x = False

        for data, label, colour in zip(self.series, self.labels, self.colours):
            ax.hist(
                data,
                bins=bin_edges,
                histtype="step",
                linewidth=1.8,
                color=colour,
                label=label,
            )

        if log_x:
            ax.set_xscale("log")
            ax.set_yscale("log")
        ax.set_xlabel("Latency (ns)")
        ax.set_ylabel("Count")
        ax.set_title(f"Latency histogram overlay - {self.metric}")
        ax.grid(True, which="both", linestyle="--", alpha=0.4)
        ax.legend(title="Run")

        out = self.out_dir / f"compare_hist_{self.metric}_{self._slug()}.png"
        fig.savefig(out, dpi=150, bbox_inches="tight")
        plt.close(fig)
        return out

    def plot_percentiles(self) -> Path:
        percentiles = [50, 75, 90, 95, 99, 99.5, 99.9]
        labels_x = [f"P{p}" for p in percentiles]
        n_runs = len(self.series)
        width = 0.8 / n_runs
        x = np.arange(len(percentiles))

        fig, ax = plt.subplots(figsize=(10, 5))
        for i, (data, label, colour) in enumerate(zip(self.series, self.labels, self.colours)):
            values = [np.percentile(data, p) for p in percentiles]
            offset = (i - (n_runs - 1) / 2) * width
            ax.bar(x + offset, values, width=width, label=label, color=colour, alpha=0.9)

        ax.set_xticks(x)
        ax.set_xticklabels(labels_x)
        ax.set_ylabel("Latency (ns)")
        ax.set_title(f"Tail-latency percentiles - {self.metric}")
        ax.grid(True, axis="y", linestyle="--", alpha=0.4)
        ax.legend(title="Run")

        out = self.out_dir / f"compare_percentiles_{self.metric}_{self._slug()}.png"
        fig.savefig(out, dpi=150, bbox_inches="tight")
        plt.close(fig)
        return out

    def plot_all(self) -> list[Path]:
        outputs = [
            self.plot_cdf(),
            self.plot_histogram(),
            self.plot_percentiles(),
        ]
        for p in outputs:
            print(f"Saved: {p}")
        return outputs


def main():
    parser = argparse.ArgumentParser(
        description="Overlay latency graphs from multiple benchmark runs onto single images.",
    )
    parser.add_argument(
        "--run-ids",
        type=str,
        nargs="+",
        required=True,
        help="List of run IDs to overlay (e.g. --run-ids 1234567 7654321 1122334)",
    )
    parser.add_argument(
        "--labels",
        type=str,
        nargs="+",
        default=None,
        help="Optional legend labels, one per run ID (defaults to the run IDs themselves)",
    )
    parser.add_argument(
        "--metric",
        type=str,
        choices=["enqueue", "dequeue"],
        default="enqueue",
        help="Which latency column to overlay (default: enqueue)",
    )
    parser.add_argument(
        "--no-outlier-removal",
        action="store_true",
        help="Disable the 1.5x IQR outlier filter",
    )
    parser.add_argument(
        "--out-dir",
        type=str,
        default=None,
        help="Optional override for the output directory (defaults to image_editing/)",
    )
    args = parser.parse_args()

    comparator = RunComparator(
        run_ids=args.run_ids,
        labels=args.labels,
        metric=args.metric,
        remove_outliers=not args.no_outlier_removal,
        out_dir=Path(args.out_dir) if args.out_dir else None,
    )
    comparator.plot_all()


if __name__ == "__main__":
    main()