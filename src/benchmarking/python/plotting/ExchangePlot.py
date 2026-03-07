from pathlib import Path
import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import mplfinance as mpf
from benchmarking.python.plotting.utils import *
from benchmarking.python.image_editing.image import ImageEditor
import math

class ExchangePlot:

    def __init__(self, csv_path: Path):
        self.csv_path = csv_path
        self.df = pd.read_csv(csv_path)

        self._split_column("exp_prices")
        self._split_column("exp_qtities")
        self._split_column("acc_prices")
        self._split_column("acc_qtities")

        # cycle,exp_prices,exp_qtities,cycle2,acc_prices,acc_qtities
        self.data = {
            "cycle": self.df["cycle"].astype(int).values,
            "exp_prices": self.df["exp_prices"].to_numpy(),
            "exp_qtities": self.df["exp_qtities"].to_numpy(),
            "cycle2": self.df["cycle2"].astype(int).values,
            "acc_prices": self.df["acc_prices"].to_numpy(),
            "acc_qtities": self.df["acc_qtities"].to_numpy()
        }

    # itterates over the dataframe and yields row, converted to python data type
    def iter_rows(self):
        cols = ["cycle", "exp_prices", "exp_qtities", "cycle2", "acc_prices", "acc_qtities"]

        for _, row in self.df.iterrows():
            yield {
                "cycle": int(row["cycle"]) if "cycle" in self.df else None,
                "exp_prices": list(row["exp_prices"]) if "exp_prices" in self.df else [],
                "exp_qtities": list(row["exp_qtities"]) if "exp_qtities" in self.df else [],
                "cycle2": int(row["cycle2"]) if "cycle2" in self.df else None,
                "acc_prices": list(row["acc_prices"]) if "acc_prices" in self.df else [],
                "acc_qtities": list(row["acc_qtities"]) if "acc_qtities" in self.df else [],
            }

    # 2D array of processed data frame into  [["Open", "Close", "High", "Low", "Volume"], ...]
    # aggregates trades into candlesticks over 'cycles' instead of timestamp
    # last value in array is the trade price
    def _process_csv(self, cycles: int = 10, cycle_range: tuple[int, int] | None = None, source: str = "expected", fixed_candles: bool = True) -> list[list[float]]:
        candles: list[list[float]] = []
        
        num_orders = len(self.df)
        # candles = orders / cyccles
        if fixed_candles:
            cycles = math.ceil(num_orders / 80) 

        open_price = None
        close_price = None
        high_price = None
        low_price = None
        volume = 0.0
        seen_trade = False
        
        # Select data source
        price_key = "exp_prices" if source == "expected" else "acc_prices"
        qty_key = "exp_qtities" if source == "expected" else "acc_qtities"

        for idx, row in enumerate(self.iter_rows()):
            cycle_num = row.get("cycle")
            
            # Filter by cycle_range if provided
            if cycle_range is not None:
                if cycle_num is None or cycle_num < cycle_range[0] or cycle_num > cycle_range[1]:
                    continue
            
            prices = row.get(price_key, []) or []
            qtys = row.get(qty_key, []) or []

            if prices:
                trade_price = float(prices[-1])
                trade_volume = float(sum(qtys)) if qtys else 0.0

                if not seen_trade:
                    open_price = trade_price
                    high_price = trade_price
                    low_price = trade_price
                else:
                    high_price = max(high_price, trade_price)
                    low_price = min(low_price, trade_price)

                close_price = trade_price
                volume += trade_volume
                seen_trade = True

            # window boundary reached
            if cycles is not None and (idx + 1) % cycles == 0:
                if seen_trade:
                    candles.append([open_price, close_price, high_price, low_price, volume])
                # reset for next window
                open_price = close_price = high_price = low_price = None
                volume = 0.0
                seen_trade = False

        # tail window
        if seen_trade:
            candles.append([open_price, close_price, high_price, low_price, volume])

        return candles

    def _split_column(self, col: str):
        if col not in self.df:
            return
        
        def _to_float_list(value):
            if pd.isna(value):
                return []
            if isinstance(value, str):
                return [float(v) for v in value.split("|") if v != ""]
            try:
                iter(value)
                return [float(v) for v in value]
            except TypeError:
                return [float(value)]

        self.df[col] = self.df[col].apply(lambda x: _to_float_list(x))
        # self.df[col] = self.df[col].apply(_to_float_list) is equivalent, e.g. x -> f -> f x
        
    # plots the candlestick chart
    def plot_candles(self, data, start="2026-01-01", freq="T", title="Stock Price", out: Path | None = None):
        df = pd.DataFrame(
            data,
            columns=["Open", "Close", "High", "Low", "Volume"]
        )
        df = df[["Open", "High", "Low", "Close", "Volume"]]
        df.index = pd.date_range(start=start, periods=len(df), freq=freq)

        savefig = None
        if out is not None:
            out.parent.mkdir(parents=True, exist_ok=True)
            savefig = dict(fname=str(out), dpi=150, bbox_inches="tight")

        mpf.plot(
            df,
            type="candle",
            volume=True,
            style="charles",
            title=title,
            ylabel="Price",
            ylabel_lower="Volume",
            savefig=savefig,
        )

    def plot_all(self, out_dir: Path | None = None, cycle_range: tuple[int, int] | None = None):
        if out_dir is None:
            out_dir = get_graph_dir("exchange")
        
        base_name = self.csv_path.stem
        
        # process csv into useable form
        candles_expected = self._process_csv(cycle_range=cycle_range, source="expected")
        if candles_expected:
            self.plot_candles(
                candles_expected,
                start="2026-01-01",
                freq="T",
                title="Expected Matching",
                out=out_dir / f"expected_matching_{base_name}.png",
            )
        else:
            print("No expected trades found")
        
        # plot actual data
        candles_actual = self._process_csv(cycle_range=cycle_range, source="actual")
        if candles_actual:
            self.plot_candles(
                candles_actual,
                start="2026-01-01",
                freq="T",
                title="Actual Matching",
                out=out_dir / f"actual_matching_{base_name}.png",
            )
        else:
            print("No actual trades found")

    def report_summary(self) -> tuple[list[list[str]], list[int]]:
        """Return (summary_rows, differing_cycles) for the report."""
        total_cycles = len(self.data["cycle"])
        cycles_with_exp = 0
        cycles_with_act = 0
        cycles_differ = 0
        total_exp_trades = 0
        total_act_trades = 0
        differing_cycles: list[int] = []

        for row in self.iter_rows():
            exp_p = row.get("exp_prices", [])
            act_p = row.get("acc_prices", [])
            exp_q = row.get("exp_qtities", [])
            act_q = row.get("acc_qtities", [])

            has_exp = len(exp_p) > 0
            has_act = len(act_p) > 0

            if has_exp:
                cycles_with_exp += 1
                total_exp_trades += len(exp_p)
            if has_act:
                cycles_with_act += 1
                total_act_trades += len(act_p)

            if exp_p != act_p or exp_q != act_q:
                if has_exp or has_act:
                    cycles_differ += 1
                    differing_cycles.append(row.get("cycle", -1))

        comparable = max(cycles_with_exp, cycles_with_act, 1)
        match_rate = 100.0 * (1 - cycles_differ / comparable) if comparable else 0.0

        summary_rows = [
            ["Total cycles", f"{total_cycles:,}"],
            ["Cycles with expected trades", f"{cycles_with_exp:,}"],
            ["Cycles with actual trades", f"{cycles_with_act:,}"],
            ["Cycles where sequences differ", f"{cycles_differ:,}"],
            ["Match rate", f"{match_rate:.2f}%"],
            ["Total expected trades", f"{total_exp_trades:,}"],
            ["Total actual trades", f"{total_act_trades:,}"],
        ]
        return summary_rows, differing_cycles

    def plot_report_candles(self, out_dir: Path, cycle_range: tuple[int, int] | None = None, mismatch_window: tuple[int, int] | None = None) -> dict[str, Path | None]:
        out_dir.mkdir(parents=True, exist_ok=True)
        result: dict[str, Path | None] = {"expected": None, "actual": None, "overlay": None, "overlay_zoom": None}
        try:
            candles_exp = self._process_csv(cycle_range=cycle_range, source="expected")
            candles_act = self._process_csv(cycle_range=cycle_range, source="actual")

            if candles_exp:
                exp_path = out_dir / "expected.png"
                self.plot_candles(candles_exp, title="Expected Matching", out=exp_path)
                result["expected"] = exp_path

            if candles_act:
                act_path = out_dir / "actual.png"
                self.plot_candles(candles_act, title="Actual Matching", out=act_path)
                result["actual"] = act_path

            if result["expected"] and result["actual"]:
                editor = ImageEditor(result["expected"], result["actual"])
                overlay_path = str(out_dir / "overlay.png")
                editor.transparency(out=overlay_path)
                result["overlay"] = Path(overlay_path)

                if mismatch_window:
                    window_len = max(10, mismatch_window[1] - mismatch_window[0] + 1)
                    zoom_cycles = max(10, math.ceil(window_len / 200))

                    candles_exp_zoom = self._process_csv(
                        cycles=zoom_cycles,
                        cycle_range=mismatch_window,
                        source="expected",
                        fixed_candles=False,
                    )
                    candles_act_zoom = self._process_csv(
                        cycles=zoom_cycles,
                        cycle_range=mismatch_window,
                        source="actual",
                        fixed_candles=False,
                    )
                    
                    if candles_exp_zoom:
                        exp_zoom_path = out_dir / "expected_zoom.png"
                        self.plot_candles(candles_exp_zoom, title="Expected Matching (mismatch window)", out=exp_zoom_path)
                    
                    if candles_act_zoom:
                        act_zoom_path = out_dir / "actual_zoom.png"
                        self.plot_candles(candles_act_zoom, title="Actual Matching (mismatch window)", out=act_zoom_path)
                    
                    if exp_zoom_path.exists() and act_zoom_path.exists():
                        editor_zoom = ImageEditor(str(exp_zoom_path), str(act_zoom_path))
                        overlay_zoom_path = str(out_dir / "overlay_zoom.png")
                        editor_zoom.transparency(out=overlay_zoom_path)
                        result["overlay_zoom"] = Path(overlay_zoom_path)
                        
        except Exception:
            pass

        return result