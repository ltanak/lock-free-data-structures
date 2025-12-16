from latency_plots.LatencyPlot import LatencyPlot
from latency_plots.utils import *

def main():
    csv_path = csvDir(dir="latencies")
    most_recent = getMostRecentCsv(csv_path)

    lp = LatencyPlot(csv_path / most_recent)
    print(lp.summary())
    lp.histogram()


if __name__ == "__main__":
    main()
