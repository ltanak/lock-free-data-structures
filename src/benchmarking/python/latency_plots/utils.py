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

def getMostRecentCsv(path: Path) -> str:
    return all_csvs(path, True)[0]