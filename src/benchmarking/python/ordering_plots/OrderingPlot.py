from pathlib import Path
import pandas as pd
import numpy as np
import matplotlib.pyplot as plt

class OrderingPlot:
    def __init__(self, csv_path: Path):
        self.csv_path = csv_path
        df = pd.read_csv(csv_path)

        self.data = {
            "expected_id": df["expected_id"].astype(float).values,
            "actual_id": df["actual_id"].astype(float).values,
        }

    def order_preserved(self) -> bool:
        return np.array_equal(self.data["expected_id"], self.data["actual_id"])

    def mismatch_indices(self) -> np.ndarray:
        return np.nonzero(self.data["expected_id"] != self.data["actual_id"])[0]

    def mismatch_summary(self):
        idx = self.mismatch_indices()
        total = len(self.data["expected_id"])
        return {
            "total": total,
            "mismatches": len(idx),
            "mismatch_pct": 100.0 * len(idx) / total if total else 0.0,
            "first_mismatch_index": int(idx[0]) if len(idx) else None,
        }

    def plot_out_of_order_pairs(self, title="Expected vs Actual", out: Path | None = None, id_range: tuple[int, int] | None = None):
        exp = self.data["expected_id"]
        act = self.data["actual_id"]
        
        # Filter by id_range if provided
        if id_range:
            mask = (exp >= id_range[0]) & (exp <= id_range[1])
            exp = exp[mask]
            act = act[mask]
        
        mism = exp != act

        plt.figure()
        plt.scatter(exp[~mism], act[~mism], s=1, c="tab:gray", label="match", alpha=0.6)
        if mism.any():
            plt.scatter(exp[mism], act[mism], s=1, c="tab:red", label="mismatch", alpha=0.9)

        plt.plot([exp.min(), exp.max()], [exp.min(), exp.max()], "k--", linewidth=1, label="ideal")
        plt.xlabel("expected_id")
        plt.ylabel("actual_id")
        plt.title(title)
        plt.legend()
        plt.grid(True, linestyle="--", alpha=0.4)

        if out:
            out.parent.mkdir(parents=True, exist_ok=True)
            plt.savefig(out, dpi=150, bbox_inches="tight")
        else:
            plt.show()
        plt.close()

    def plot_offset(self, title="Actual - Expected", out: Path | None = None, id_range: tuple[int, int] | None = None):
        exp = self.data["expected_id"]
        act = self.data["actual_id"]
        
        # Filter by id_range if provided
        if id_range:
            mask = (exp >= id_range[0]) & (exp <= id_range[1])
            exp = exp[mask]
            act = act[mask]
        
        delta = act - exp
        
        # Only plot points with non-zero offset
        offset_mask = delta != 0
        exp_offset = exp[offset_mask]
        delta_offset = delta[offset_mask]

        plt.figure()
        plt.plot(exp_offset, delta_offset, linestyle="none", marker="o", markersize=1, alpha=0.7, c="tab:red")
        plt.axhline(0, color="k", linestyle="--", linewidth=1)
        plt.xlabel("expected_id")
        plt.ylabel("offset (actual - expected)")
        plt.title(title)
        plt.grid(True, linestyle="--", alpha=0.4)

        if out:
            out.parent.mkdir(parents=True, exist_ok=True)
            plt.savefig(out, dpi=150, bbox_inches="tight")
        else:
            plt.show()
        plt.close()

    def plot_displacement_heatmap(self, title="Displacement Magnitude", out: Path | None = None, id_range: tuple[int, int] | None = None):
        exp = self.data["expected_id"]
        act = self.data["actual_id"]
        
        # Filter by id_range if provided
        if id_range:
            mask = (exp >= id_range[0]) & (exp <= id_range[1])
            exp = exp[mask]
            act = act[mask]
        
        delta = np.abs(act - exp)

        plt.figure(figsize=(12, 3))
        scatter = plt.scatter(exp, np.zeros_like(exp), c=delta, s=1, cmap="YlOrRd", alpha=0.8)
        plt.colorbar(scatter, label="displacement (|actual - expected|)")
        plt.xlabel("position (expected_id)")
        plt.ylabel("")
        plt.yticks([])
        plt.title(title)
        plt.grid(True, axis="x", linestyle="--", alpha=0.4)

        if out:
            out.parent.mkdir(parents=True, exist_ok=True)
            plt.savefig(out, dpi=150, bbox_inches="tight")
        else:
            plt.show()
        plt.close()

    def plot_expected_vs_actual_colored(self, title="Expected vs Actual (colored by displacement)", out: Path | None = None, id_range: tuple[int, int] | None = None):
        exp = self.data["expected_id"]
        act = self.data["actual_id"]
        
        # Filter by id_range if provided
        if id_range:
            mask = (exp >= id_range[0]) & (exp <= id_range[1])
            exp = exp[mask]
            act = act[mask]
        
        delta = np.abs(act - exp)

        plt.figure()
        scatter = plt.scatter(exp, act, c=delta, s=1, cmap="YlOrRd", alpha=0.7)
        plt.plot([exp.min(), exp.max()], [exp.min(), exp.max()], "k--", linewidth=1.5, label="ideal (no offset)")
        plt.colorbar(scatter, label="displacement")
        plt.xlabel("expected_id")
        plt.ylabel("actual_id")
        plt.title(title)
        plt.legend()
        plt.grid(True, linestyle="--", alpha=0.4)

        if out:
            out.parent.mkdir(parents=True, exist_ok=True)
            plt.savefig(out, dpi=150, bbox_inches="tight")
        else:
            plt.show()
        plt.close()

    