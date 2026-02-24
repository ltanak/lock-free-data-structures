from pathlib import Path
import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
from benchmarking.python.plotting.utils import *

class HardwarePlot:

    def __init__(self, csv_path: Path):
        self.csv_path = csv_path
        
    def report_summary(self):
        summary_rows = [

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
