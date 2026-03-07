from pathlib import Path
import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
from benchmarking.python.plotting.utils import *

class HardwarePlot:

    def __init__(self, csv_path: Path, test: str | None = "stress"):
        self.csv_path = csv_path
        self.test_type = test
        df = pd.read_csv(csv_path)

        # all hardware metrics are average across all the threads
        # maybe should add separation between producer and consumer threads?

        self.data = {
            "cpu_cycles": df["cpu_cycles"].astype(float).values,
            "cpu_insts": df["cpu_insts"].astype(float).values,
            "cache_refs": df["cache_refs"].astype(float).values,
            "cache_misses": df["cache_misses"].astype(float).values,
            "branch_insts": df["branch_insts"].astype(float).values,
            "branch_misses": df["branch_misses"].astype(float).values,
            "num_threads": df["num_threads"].astype(int).values,
        }
        
    def report_summary(self):
        cpu_cycles = self.data["cpu_cycles"][0]
        cpu_insts = self.data["cpu_insts"][0]
        cache_refs = self.data["cache_refs"][0]
        cache_misses = self.data["cache_misses"][0]
        branch_insts = self.data["branch_insts"][0]
        branch_misses = self.data["branch_misses"][0]
        num_threads = int(self.data["num_threads"][0])
        
        # calculated metrics
        ipc = cpu_insts / cpu_cycles if cpu_cycles > 0 else 0
        cache_miss_rate = (cache_misses / cache_refs * 100) if cache_refs > 0 else 0
        branch_miss_rate = (branch_misses / branch_insts * 100) if branch_insts > 0 else 0
        cache_mpki = (cache_misses / cpu_insts * 1000) if cpu_insts > 0 else 0
        branch_mpki = (branch_misses / cpu_insts * 1000) if cpu_insts > 0 else 0
        cycles_per_cache_miss = cpu_cycles / cache_misses if cache_misses > 0 else 0
        
        threads_summary: str = (
            "Threads (producer + consumer)" if self.test_type == "stress"
            else "Threads (consumers)"
        )

        summary_rows = [
            [threads_summary, f"{num_threads}"],
            ["", ""],
            ["CPU Cycles (avg/thread)", f"{cpu_cycles:,.0f}"],
            ["CPU Instructions (avg/thread)", f"{cpu_insts:,.0f}"],
            ["IPC (Instructions/Cycle)", f"{ipc:.3f}"],
            ["", ""],
            ["Cache References (avg/thread)", f"{cache_refs:,.0f}"],
            ["Cache Misses (avg/thread)", f"{cache_misses:,.0f}"],
            ["Cache Miss Rate", f"{cache_miss_rate:.2f}%"],
            ["Cache MPKI (misses/1K insts)", f"{cache_mpki:.2f}"],
            ["Cycles per Cache Miss", f"{cycles_per_cache_miss:.1f}"],
            ["", ""],
            ["Branch Instructions (avg/thread)", f"{branch_insts:,.0f}"],
            ["Branch Misses (avg/thread)", f"{branch_misses:,.0f}"],
            ["Branch Miss Rate", f"{branch_miss_rate:.2f}%"],
            ["Branch MPKI (misses/1K insts)", f"{branch_mpki:.2f}"],
        ]
        return summary_rows
    
    def example_chart(self, metric: str, title: str, out: Path | None = None, return_fig=False):
        # do figure stuff here
        ARRAY = np.array([])
        OTHER_STUFF = np.linespace(0, 10, 1)

        fig = plt.figure()
        plt.plot(ARRAY, OTHER_STUFF)
        plt.xlabel("CHANGE ME")
        plt.ylabel("CHANGE ME")
        plt.title(f"{title} - {metric}")
        plt.grid(True)

        if return_fig:
            return fig
        
        if out:
            out.parent.mkdir(parents=True, exist_ok=True)
            plt.savefig(out, dpi=150, bbox_inches="tight")
        else:
            plt.show()
        
        plt.close()
