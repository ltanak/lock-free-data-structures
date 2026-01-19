from pathlib import Path
import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import mplfinance as mpf
from benchmarking.python.plotting.utils import *


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
    def _process_csv(self, cycles: int = 10, cycle_range: tuple[int, int] | None = None, source: str = "expected") -> list[list[float]]:
        """
        agg trades into candlesticks over `cycles` rows.

        Uses the last value of prices in a row as the trade price; volume is
        the sum of quantities for that row. Empty rows (no trades) advance the
        cycle window but do not change OHLCV unless a trade occurs in the window.
        Returns a list of [Open, Close, High, Low, Volume].
        
        Args:
            cycles: Number of rows per candlestick window.
            cycle_range: Optional (min, max) tuple to filter rows by cycle number.
            source: Either "expected" (exp_prices/exp_qtities) or "actual" (acc_prices/acc_qtities).
        """

        candles: list[list[float]] = []

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

        self.df[col] = self.df[col].apply(_to_float_list)

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
        """
        Generate and save exchange candlestick plots for both expected and actual data.
        
        Args:
            out_dir: Output directory. If None, uses default exchange graphs directory.
            cycle_range: Optional (min, max) tuple to filter data by cycle range.
        """
        if out_dir is None:
            out_dir = get_graph_dir("exchange")
        
        base_name = self.csv_path.stem
        
        # Plot expected data
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
        
        # Plot actual data
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