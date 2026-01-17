import sys
from pathlib import Path

# repo src directory is on sys.path
PROJECT_SRC = Path(__file__).resolve().parents[3]
if str(PROJECT_SRC) not in sys.path:
    sys.path.insert(0, str(PROJECT_SRC))

from latency_plots.LatencyPlot import LatencyPlot
from benchmarking.python.ordering_plots.OrderingPlot import OrderingPlot
from benchmarking.python.utils.utils import *

def main():
    # Latency plots
    csv_dir: Path = csvDir(dir="latencies")
    recent_csv:Path = getMostRecentCsv(csv_dir)
    recent_name: str = recent_csv.stem
    csv_path = csv_dir / recent_csv

    histOut = latencyPlotsDir() / f"{recent_name}_hist.png"
    cdfOut = latencyPlotsDir() / f"{recent_name}_cdf.png"
    percOut = latencyPlotsDir() / f"{recent_name}_perc.png"

    lp = LatencyPlot(csv_path)
    print("=== LATENCY SUMMARY ===")
    print(lp.summary())
    lp.histogram(metric="enqueue", out=histOut)
    lp.cdf(metric="enqueue", out=cdfOut)

    # Ordering plots
    ord_csv_dir: Path = csvDir(dir="ordering")
    ord_recent_csv: Path = getMostRecentCsv(ord_csv_dir)
    ord_recent_name: str = ord_recent_csv.stem
    ord_csv_path = ord_csv_dir / ord_recent_csv

    ordPairsOut = orderPlotsDir() / f"{ord_recent_name}_pairs.png"
    ordOffsetOut = orderPlotsDir() / f"{ord_recent_name}_offset.png"
    ordHeatmapOut = orderPlotsDir() / f"{ord_recent_name}_heatmap.png"
    ordColoredOut = orderPlotsDir() / f"{ord_recent_name}_colored.png"

    op = OrderingPlot(ord_csv_path)
    print("\n=== ORDERING SUMMARY ===")
    print(op.mismatch_summary())
    op.plot_out_of_order_pairs(out=ordPairsOut, id_range=(0, 1000))
    op.plot_offset(out=ordOffsetOut)
    op.plot_displacement_heatmap(out=ordHeatmapOut, id_range=(0, 1000))
    op.plot_expected_vs_actual_colored(out=ordColoredOut, id_range=(0, 1000))



if __name__ == "__main__":
    main()
