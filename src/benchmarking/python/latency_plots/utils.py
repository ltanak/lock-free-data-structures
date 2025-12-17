from pathlib import Path


def getBenchmarkingRoot() -> Path:
    """
    Returns:
        .../benchmarking
    """
    return Path(__file__).resolve().parents[2]


def csvDir(dir: str) -> Path:
    return getBenchmarkingRoot() / "csvs" / dir

def all_csvs(path: Path, reverse: bool = False):
    return sorted(path.glob("*.csv"), reverse=reverse)

def getMostRecentCsv(path: Path) -> Path:
    return all_csvs(path, True)[0]

def latencyPlotsDir():
    """
    Returns:
        benchmarking/python/latency_plots/graphs
    """
    path = getBenchmarkingRoot() / "python" / "latency_plots" / "graphs"
    path.mkdir(parents=True, exist_ok=True)
    return path