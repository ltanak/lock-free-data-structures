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
    lp.histogram(metric="enqueue", out=histOut)
    lp.cdf(metric="enqueue", out=cdfOut)



if __name__ == "__main__":
    main()
