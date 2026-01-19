import sys
from pathlib import Path

# repo src directory is on sys.path
PROJECT_SRC = Path(__file__).resolve().parents[3]
if str(PROJECT_SRC) not in sys.path:
    sys.path.insert(0, str(PROJECT_SRC))

from benchmarking.python.plotting.LatencyPlot import LatencyPlot
from benchmarking.python.plotting.OrderingPlot import OrderingPlot
from benchmarking.python.plotting.utils import *
from benchmarking.python.plotting.ExchangePlot import ExchangePlot

# first arguments are positional / optional, final are keyword-only
def plot_graphs(file: str | None = None, must_match: bool = False, *, active: dict[str, bool]):
    files = {}
    if not file:
        files = get_latest_csv(must_match=must_match)
    else:
        files = get_csv_all_dirs_name(name=file)

    # latency plots
    if active["latencies"]:
        latency_csv_path = files["latencies"]
        lp = LatencyPlot(latency_csv_path)
        print("=== LATENCY SUMMARY ===")
        print(lp.summary())
        lp.plot_all()

    # Ordering plots
    if active["ordering"]:
        ordering_csv_path = files["ordering"]
        op = OrderingPlot(ordering_csv_path)
        print("\n=== ORDERING SUMMARY ===")
        print(op.mismatch_summary())
        op.plot_all(id_range=(0, 1000))

    # Exchange Plotting
    if active["exchange"]:
        exchange_csv_path = files["exchange"]
        ep = ExchangePlot(exchange_csv_path)
        print("\n=== EXCHANGE SUMMARY ===")
        # ep.plot_all()
        ep.plot_all(cycle_range=(200, 1000))


    # CSV = "GOOD_matching_15_01_2026_10_32_38.csv"
    # plotter = ExchangePlot(getExchangeCsv(CSV))
    

if __name__ == "__main__":

    file = "GOOD_matching_15_01_2026_10_32_38.csv"
    active = {"exchange": True, "ordering": True, "latencies": True}
    if file:
        active = get_csv_all_dirs(name= file)

    plot_graphs(
        file = file,
        must_match= False,
        active= active
    )

