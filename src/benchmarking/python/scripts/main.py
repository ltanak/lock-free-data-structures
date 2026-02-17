import sys
import argparse
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
def plot_graphs(
        file: str | None = None, 
        run_id: str | None = None, 
        must_match: bool = False, 
        id_range: tuple[int, int] | None = None, 
        *, active: dict[str, bool]
    ):
    files = {}
    
    if run_id:
        try:
            files = get_csv_by_run_id(run_id)
            print(f"Using run_id: {run_id}")
        except FileNotFoundError as e:
            print(f"Error: {e}")
            return
    elif file:
        files = get_csv_all_dirs_name(name=file)
    else:
        # gest most recent CSVs
        files = get_latest_csv(must_match=must_match)
    
    if len(files) > 0:
        keys = files.keys()
        for key in keys:
            path = files[key].stem.split("/")[-1]
            print(f"{key} file: {path}")
    else:
        print("No CSV files found.")
        return
    

    # latency plots
    if active.get("latencies") and "latencies" in files:
        latency_csv_path = files["latencies"]
        lp = LatencyPlot(latency_csv_path)
        print("=== LATENCY SUMMARY ===")
        print(lp.summary())
        lp.plot_all()

    # Ordering plots
    if active.get("ordering") and "ordering" in files:
        ordering_csv_path = files["ordering"]
        op = OrderingPlot(ordering_csv_path)
        print("\n=== ORDERING SUMMARY ===")
        print(op.mismatch_summary())
        op.plot_all(id_range=id_range if id_range else None)

    # Exchange Plotting
    if active.get("exchange") and "exchange" in files:
        exchange_csv_path = files["exchange"]
        ep = ExchangePlot(exchange_csv_path)
        print("\n=== EXCHANGE SUMMARY ===")
        # ep.plot_all()
        ep.plot_all(cycle_range=id_range if id_range else None)


    # CSV = "GOOD_matching_15_01_2026_10_32_38.csv"
    # plotter = ExchangePlot(getExchangeCsv(CSV))
    

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Plot benchmarking results from CSV files")
    parser.add_argument("--file", type=str, help="CSV filename to plot")
    parser.add_argument("--run-id", type=str, help="ID to plot (e.g. 1771246990)")
    parser.add_argument("--id-range", type=int, nargs=2, metavar=('START', 'END'), help="The range of IDs that are to be plotted (e.g. --id-range 0 1000)")
    parser.add_argument("--ls", action="store_true", help="List all available run IDs")
    parser.add_argument("--must-match", action="store_true", help="Require all CSVs to have matching timestamps/run_ids")
    parser.add_argument("--latencies", action="store_true", help="Plot latency graphs")
    parser.add_argument("--ordering", action="store_true", help="Plot ordering graphs")
    parser.add_argument("--exchange", action="store_true", help="Plot exchange graphs")
    parser.add_argument("--all", action="store_true", help="Plot all graph types")
    
    args = parser.parse_args()
    
    # List available run IDs if requested
    if args.ls:
        run_ids = get_all_run_ids()
        if run_ids:
            print("Available run IDs:")
            for rid in run_ids:
                print(f"  {rid}")
        else:
            print("No run IDs found.")
        sys.exit(0)
    
    # Determine which plots to generate
    if args.all:
        active = {"exchange": True, "ordering": True, "latencies": True}
    else:
        active = {
            "latencies": args.latencies,
            "ordering": args.ordering,
            "exchange": args.exchange
        }
    
    # Convert id_range list to tuple if provided
    id_range = tuple(args.id_range) if args.id_range else None
    
    plot_graphs(
        file=args.file,
        run_id=args.run_id,
        must_match=args.must_match,
        id_range=id_range,
        active=active
    )

