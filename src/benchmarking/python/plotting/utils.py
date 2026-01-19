from pathlib import Path
"""
Code that contains all the utility functions for getting most recent graphs,
csvs, and directory paths.
"""

# ../benchmarking
def get_root() -> Path:
    return Path(__file__).resolve().parents[2]

def get_csv_dir(dir: str) -> Path:
    return get_root() / "csvs" / dir

def get_graph_dir(dir: str) -> Path:
    path = get_root() / "graphs" / dir
    path.mkdir(parents=True, exist_ok=True)
    return path

# get the most recent CSV from all directories (exchange, ordering, latencies).
def get_latest_csv(must_match: bool = False) -> dict[str, Path]:
    dirs = {"exchange": "exchange", "ordering": "ordering", "latencies": "latencies"}
    latest = {}
    timestamps = {}
    
    for key, dir_name in dirs.items():
        path = get_csv_dir(dir_name)
        csvs = get_csvs_dir(path, reverse=True)
        if not csvs:
            raise FileNotFoundError(f"No CSVs found in {path}")
        
        latest[key] = csvs[0]

        # timestamp format: prefix_DD_MM_YYYY_HH_MM_SS.csv
        parts = csvs[0].stem.split("_")
        if len(parts) >= 6:
            timestamps[key] = "_".join(parts[-6:])
        else:
            timestamps[key] = csvs[0].stem
    
    if must_match:
        unique_timestamps = set(timestamps.values())
        if len(unique_timestamps) > 1:
            raise ValueError(f"CSV timestamps do not match: {timestamps}. Set must_match=False to allow different timestamps.")
    
    return latest

def get_latest_csv_dir(dir: str) -> Path:
    return get_latest_csv()[dir]

def get_csvs_dir(path: Path, reverse: bool = False):
    return sorted(path.glob("*.csv"), reverse=reverse)

# returns True if the csv is found in the specified directory
def get_csv(name: str, dir: str) -> bool:
    path = get_csv_dir(dir)
    return (path / name).exists()

# returns a dictionary mapping directory names to whether the csv exists in them
def get_csv_all_dirs(name: str) -> dict[str, bool]:
    dirs = ["exchange", "ordering", "latencies"]
    return {dir_name: get_csv(name, dir_name) for dir_name in dirs}

def get_csv_all_dirs_name(name: str) -> dict[str, Path]:
    """
    Find a CSV by name across all directories and return full paths.
    
    Returns:
        Dictionary mapping dir names to full CSV paths where the file exists.
    """
    dirs = ["exchange", "ordering", "latencies"]
    result = {}
    for dir_name in dirs:
        if get_csv(name, dir_name):
            result[dir_name] = get_csv_dir(dir_name) / name
    return result