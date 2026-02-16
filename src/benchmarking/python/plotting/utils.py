from pathlib import Path
from datetime import datetime

"""
Code that contains all the utility functions for getting most recent graphs,
csvs, and directory paths.
"""

# ../benchmarking
def get_root() -> Path:
    return Path(__file__).resolve().parents[2]

# returns the path for the specific csv directory (exchange, ordering, latencies)
def get_csv_dir(dir: str) -> Path:
    return get_root() / "csvs" / dir

# eturns the path for the specific graph directory (exchange, ordering, latencies)
def get_graph_dir(dir: str) -> Path:
    path = get_root() / "graphs" / dir
    path.mkdir(parents=True, exist_ok=True)
    return path

# get the most recent CSV from all directories (exchange, ordering, latencies)
def get_latest_csv(must_match: bool = False) -> dict[str, Path]:
    dirs = ["exchange", "ordering", "latencies"]
    latest = {}
    run_ids = {}
    timestamps = {}
    
    for key in dirs:
        path = get_csv_dir(key)
        csvs = get_csvs_dir(path, reverse=True)
        if not csvs:
            raise FileNotFoundError(f"No CSVs found in {path}")
        
        latest[key] = csvs[0]
        run_ids[key] = extract_run_id(csvs[0])
        timestamps[key] = extract_timestamp(csvs[0])
    
    if must_match:
        # match by run_id first (preferred for grouped runs)
        unique_run_ids = set(r for r in run_ids.values() if r is not None)
        if len(unique_run_ids) > 1:
            raise ValueError(f"CSV run IDs do not match: {run_ids}. Set must_match=False to allow different run IDs.")
        elif len(unique_run_ids) == 0:
            # no run_ids, fall back to timestamp matching
            unique_timestamps = set(timestamps.values())
            if len(unique_timestamps) > 1:
                raise ValueError(f"CSV timestamps do not match: {timestamps}. Set must_match=False to allow different timestamps.")
    
    return latest

# CSVs with a specific run_id from all directories
def get_csv_by_run_id(run_id: str) -> dict[str, Path]:
    dirs = ["exchange", "ordering", "latencies"]
    result = {}
    
    for dir_name in dirs:
        path = get_csv_dir(dir_name)
        csvs = get_csvs_dir(path)
        
        for csv_file in csvs:
            file_run_id = extract_run_id(csv_file)
            if file_run_id == run_id:
                result[dir_name] = csv_file
                break
    
    if not result:
        raise FileNotFoundError(f"No CSVs found with run_id: {run_id}")
    
    return result

# Get all unique run_ids from CSV files
def get_all_run_ids() -> list[str]:
    dirs = ["exchange", "ordering", "latencies"]
    run_ids = set()
    
    for dir_name in dirs:
        path = get_csv_dir(dir_name)
        csvs = get_csvs_dir(path)
        
        for csv_file in csvs:
            run_id = extract_run_id(csv_file)
            if run_id:
                run_ids.add(run_id)
    
    return sorted(list(run_ids), reverse=True)  # Most recent first

# returns the most recent csv in the directory specified
def get_latest_csv_dir(dir: str) -> Path:
    return get_latest_csv()[dir]

# returns all the csvs in the directory specified
def get_csvs_dir(path: Path, reverse: bool = False):
    csvs = list(path.glob("*.csv"))
    if not csvs:
        return []

    def parse_timestamp(p: Path) -> datetime:
        parts = p.stem.split("_")
        if len(parts) >= 6:
            try:
                return datetime.strptime("_".join(parts[-6:]), "%m_%d_%Y_%H_%M_%S")
            except ValueError:
                pass
        return datetime.min

    return sorted(csvs, key=parse_timestamp, reverse=reverse)

# Extract run_id from filename if present
# Format: {type}_{run_id}_{timestamp}.csv or {type}_{timestamp}.csv
def extract_run_id(filename: Path) -> str | None:
    parts = filename.stem.split("_")
    # New format: type_runid_MM_DD_YYYY_HH_MM_SS (8 parts with single runid)
    # Old format with timestamp_pid: type_runid1_runid2_MM_DD_YYYY_HH_MM_SS (9+ parts)
    # Old format: type_MM_DD_YYYY_HH_MM_SS (7 parts without runid)
    if len(parts) == 8:
        # Simple run_id format: parts[1] is the run_id
        return parts[1]
    elif len(parts) >= 9:
        # Old timestamp_pid format: parts[1] and parts[2] make up the run_id
        return f"{parts[1]}_{parts[2]}"
    return None

# Extract timestamp from filename
def extract_timestamp(filename: Path) -> str:
    parts = filename.stem.split("_")
    if len(parts) >= 6:
        return "_".join(parts[-6:])
    return filename.stem

# returns True if the csv is found in the specified directory
def get_csv(name: str, dir: str) -> bool:
    path = get_csv_dir(dir)
    return (path / name).exists()

# returns a dictionary mapping directory names to whether the csv exists in them
def get_csv_all_dirs(name: str) -> dict[str, bool]:
    dirs = ["exchange", "ordering", "latencies"]
    return {dir_name: get_csv(name, dir_name) for dir_name in dirs}

# returns a dictionary of directories to Paths if the file name is in any of the directories
def get_csv_all_dirs_name(name: str) -> dict[str, Path]:
    dirs = ["exchange", "ordering", "latencies"]
    result = {}
    for dir_name in dirs:
        if get_csv(name, dir_name):
            result[dir_name] = get_csv_dir(dir_name) / name
    return result