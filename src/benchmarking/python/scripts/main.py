from latency_plots.LatencyPlot import LatencyPlot
from latency_plots.utils import *

def main():
    csv_dir: Path = csvDir(dir="latencies")
    recent_csv:Path = getMostRecentCsv(csv_dir)
    recent_name: str = recent_csv.stem
    csv_path = csv_dir / recent_csv

    histOut = latencyPlotsDir() / f"{recent_name}_hist.png"
    cdfOut = latencyPlotsDir() / f"{recent_name}_cdf.png"
    percOut = latencyPlotsDir() / f"{recent_name}_perc.png"

    lp = LatencyPlot(csv_path)
    print(lp.summary())
    lp.histogram(
        out=histOut,
        max_perc=99
    )
    lp.cdf(
        out=cdfOut,
        max_perc=50
    )
    lp.percentiles(
        out=percOut,
        max_perc=50
    )


if __name__ == "__main__":
    main()
