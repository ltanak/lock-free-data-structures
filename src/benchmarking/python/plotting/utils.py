from pathlib import Path
from datetime import datetime
import io
import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
import base64
import tempfile
from functools import wraps

"""
Code that contains all the utility functions for getting most recent graphs,
csvs, and directory paths.
"""

# ../benchmarking to ../.. (results)
def get_root() -> Path:
    return Path(__file__).resolve().parents[4] / "results"

# returns the path for the specific csv directory (exchange, ordering, latencies, hardware)
# latencies and hardware are in stress_testing, exchange and ordering are in order_testing
# for hardware, you specify "hardware_stress" or "hardware_order" for specific location
def get_csv_dir(dir: str) -> Path:
    if dir == "latencies":
        return get_root() / "stress_testing" / "latencies"
    elif dir in ["hardware", "hardware_stress"]:
        return get_root() / "stress_testing" / "hardware"
    elif dir == "hardware_order":
        return get_root() / "order_testing" / "hardware"
    elif dir == "ordering":
        return get_root() / "order_testing" / "ordering"
    elif dir == "exchange":
        return get_root() / "order_testing" / "exchange"
    else:
        raise ValueError(f"Unknown CSV directory: {dir}")

# returns the path for the specific graph directory (exchange, ordering, latencies, hardware)
# latencies and hardware graphs are in stress_testing, exchange and ordering graphs are in order_testing
# For hardware, you can specify "hardware_stress" or "hardware_order" for specific location
def get_graph_dir(dir: str) -> Path:
    if dir == "latencies":
        path = get_root() / "stress_testing" / "latencies"
    elif dir in ["hardware", "hardware_stress"]:
        path = get_root() / "stress_testing" / "hardware"
    elif dir == "hardware_order":
        path = get_root() / "order_testing" / "hardware"
    elif dir == "ordering":
        path = get_root() / "order_testing" / "ordering"
    elif dir == "exchange":
        path = get_root() / "order_testing" / "exchange"
    else:
        raise ValueError(f"Unknown graph directory: {dir}")
    path.mkdir(parents=True, exist_ok=True)
    return path

# returns the path for the generated reports
def get_report_dir() -> Path:
    path = get_root() / "reports"
    path.mkdir(parents=True, exist_ok=True)
    return path

# get the most recent CSV from all directories (exchange, ordering, latencies, hardware)
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
    
    # check hardware in both locations and pick the most recent
    hw_stress_path = get_csv_dir("hardware_stress")
    hw_order_path = get_csv_dir("hardware_order")
    hw_stress_csvs = get_csvs_dir(hw_stress_path, reverse=True)
    hw_order_csvs = get_csvs_dir(hw_order_path, reverse=True)
    
    if hw_stress_csvs or hw_order_csvs:
        # pick most recent hardware CSV from either location
        all_hw_csvs = hw_stress_csvs + hw_order_csvs
        all_hw_csvs.sort(key=lambda p: extract_timestamp(p), reverse=True)
        latest["hardware"] = all_hw_csvs[0]
        run_ids["hardware"] = extract_run_id(all_hw_csvs[0])
        timestamps["hardware"] = extract_timestamp(all_hw_csvs[0])
    
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
    
    # check hardware in both locations
    for hw_dir in ["hardware_stress", "hardware_order"]:
        path = get_csv_dir(hw_dir)
        csvs = get_csvs_dir(path)
        
        for csv_file in csvs:
            file_run_id = extract_run_id(csv_file)
            if file_run_id == run_id:
                # Store as "hardware" regardless of location
                result["hardware"] = csv_file
                break
        
        if "hardware" in result:
            break
    
    if not result:
        raise FileNotFoundError(f"No CSVs found with run_id: {run_id}")
    
    return result

# all unique run_ids from CSV files
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
    
    # check hardware in both locations
    for hw_dir in ["hardware_stress", "hardware_order"]:
        path = get_csv_dir(hw_dir)
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
    # current format: type_runid_MM_DD_YYYY_HH_MM_SS (8 parts with single runid)
    # old format with timestamp_pid: type_runid1_runid2_MM_DD_YYYY_HH_MM_SS (9+ parts)
    # old format: type_MM_DD_YYYY_HH_MM_SS (7 parts without runid)
    if len(parts) == 8:
        # Simple run_id format: parts[1] is the run_id
        return parts[1]
    elif len(parts) >= 9:
        # Old timestamp_pid format: parts[1] and parts[2] make up the run_id
        return f"{parts[1]}_{parts[2]}"
    return None

# extract timestamp from filename
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
    result = {dir_name: get_csv(name, dir_name) for dir_name in dirs}
    # Check hardware in both locations
    result["hardware"] = get_csv(name, "hardware_stress") or get_csv(name, "hardware_order")
    return result

# returns a dictionary of directories to Paths if the file name is in any of the directories
def get_csv_all_dirs_name(name: str) -> dict[str, Path]:
    dirs = ["exchange", "ordering", "latencies"]
    result = {}
    for dir_name in dirs:
        if get_csv(name, dir_name):
            result[dir_name] = get_csv_dir(dir_name) / name
    # Check hardware in both locations
    if get_csv(name, "hardware_stress"):
        result["hardware"] = get_csv_dir("hardware_stress") / name
    elif get_csv(name, "hardware_order"):
        result["hardware"] = get_csv_dir("hardware_order") / name
    return result

# given a matplotlib image, convert into into bytes
def fig_encode(fig) -> str:
    buf = io.BytesIO()
    fig.savefig(buf, format="png", dpi=150, bbox_inches="tight")
    plt.close(fig)
    buf.seek(0)
    return base64.b64encode(buf.read()).decode("utf-8")

# given a path, encode the image at the path
def fig_encode_path(path: Path) -> str:
    with open(path, "rb") as f:
        return base64.b64encode(f.read()).decode("utf-8")

# return html string for an encoded image
def get_html_img_tag(b64: str, alt: str = "", width: str = "100%") -> str:
    return f'<img src="data:image/png;base64,{b64}" alt="{alt}" style="max-width:{width};">'

# creates a html table given the headers and the rows
def create_html_table(headers: list[str], rows: list[list[str]]) -> str:

    parts = ["<table>", "<thead><tr>"]
    for h in headers:
        parts.append(f"<th>{h}</th>")

    parts.append("</tr></thead><tbody>")
    for row in rows:
        parts.append("<tr>")
        for cell in row:
            parts.append(f"<td>{cell}</td>")
        parts.append("</tr>")
    parts.append("</tbody></table>")
    return "\n".join(parts)

# ------------------------------------------------------------------
# decorator for automatic figure enlisting
# ------------------------------------------------------------------

def report(name: str, layout: str = "grid", rank: int = 0):

    def decorator(func):
        @wraps(func)
        def wrapper(*args, **kwargs):
            return func(*args, **kwargs)
        
        wrapper._report_meta = {
            "name": name.capitalize(), 
            "layout": layout.lower(),
            "rank": int(rank)
        }
        return wrapper
    
    return decorator