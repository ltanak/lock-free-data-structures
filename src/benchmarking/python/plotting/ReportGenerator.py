import base64
import io
import tempfile
from datetime import datetime
from pathlib import Path
import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
import inspect

from benchmarking.python.plotting.ExchangePlot import ExchangePlot
from benchmarking.python.plotting.LatencyPlot import LatencyPlot
from benchmarking.python.plotting.OrderingPlot import OrderingPlot
from benchmarking.python.plotting.HardwarePlot import HardwarePlot
from benchmarking.python.plotting.utils import *
from benchmarking.python.plotting.consts import _CSS

# ---------------------------------------------------------------------------
# ReportGenerator
# ---------------------------------------------------------------------------

class ReportGenerator:

    def __init__(self,
        files: dict[str, Path],
        run_id: str | None = None,
        active: dict[str, bool] | None = None,
        id_range: tuple[int, int] | None = None,
        out_dir: Path | None = None,
    ):
        self.files = files
        self.run_id = run_id or "unknown"
        self.active = active or {"latencies": True, "ordering": True, "exchange": True}
        self.id_range = id_range
        self.out_dir = out_dir or get_report_dir()

        # sections stores the html strings for each of the generated sections and joins them up after
        self.sections: list[str] = []

    
    def generate_report(self) -> Path:
        self.sections.clear()
        mismatch_window = None

        if self.active.get("latencies") and "latencies" in self.files:
            latency_strs = self._latency_section()
            self.sections.append(latency_strs)

        if self.active.get("ordering") and "ordering" in self.files:
            ordering_strs, mismatch_window = self._ordering_section()
            self.sections.append(ordering_strs)

        if self.active.get("exchange") and "exchange" in self.files:
            exchange_strs = self._exchange_section(mismatch_window=mismatch_window)
            self.sections.append(exchange_strs)

        html = self._wrap_html("\n".join(self.sections))
        out_path = self.out_dir / f"report_{self.run_id}.html"
        out_path.write_text(html, encoding="utf-8")
        return out_path

    # ------------------------------------------------------------------
    # latency
    # ------------------------------------------------------------------

    def _latency_section(self):
        csv_path = self.files["latencies"]
        lp = LatencyPlot(csv_path)

        html_strs = ['<h2>Latency Benchmarking</h2>'] # may have to be csv_path.name
        html_strs.append(f'<p class="meta">CSV: <code>{csv_path.stem}</code></p>')

        for metric in ["enqueue", "dequeue"]:
            html_strs.append(f"<h3>{metric.capitalize()} Latency</h3>")

            # summary table of statistics
            report_summary_rows = lp.report_summary(metric=metric)
            html_strs.append(create_html_table(["Metric", "Value"], report_summary_rows))

            # latency plot figures
            latency_figs = self._latency_graph_figs(lp, id_range=None, metric=metric)

            # grid figures
            html_strs.append('<div class="graph-grid">')
            for name, layout, fig in latency_figs:
                if layout != "grid":
                    continue
                html_strs.append(get_html_img_tag(fig_encode(fig), f"{metric} {name}"))
            html_strs.append("</div>")

            # full figures
            html_strs.append('<div class="graph-full">')
            for name, layout, fig in latency_figs:
                if layout != "full":
                    continue

                html_strs.append(get_html_img_tag(fig_encode(fig), f"{metric} {name}"))
            html_strs.append("</div>")

        return "\n".join(html_strs)
    
    def _latency_graph_figs(self, lp: LatencyPlot, id_range: tuple[int, int] | None = None, metric: str = "enqueue") -> list[tuple[str, plt.Figure]]:
        if not id_range:
            id_range = self.id_range

        tags = []
        for _, method in inspect.getmembers(lp, predicate=callable):
            meta = getattr(method, "_report_meta", None)
            if meta is None and hasattr(method, "__func__"):
                meta = getattr(method.__func__, "_report_figure_meta", None)
            if meta:
                tags.append((meta, method))

        tags.sort(key=lambda x: x[0].get("order", 1000))

        figs: list[tuple[str, str, plt.Figure]] = []
        for meta, method in tags:
            fig = method(metric=metric, return_fig=True, id_range=id_range)
            figs.append((meta["name"], meta.get("layout", "grid"), fig))

        return figs

    # ------------------------------------------------------------------
    # ordering
    # ------------------------------------------------------------------

    def _ordering_section(self):
        csv_path = self.files["ordering"]
        op = OrderingPlot(csv_path)

        html_strs = ['<h2>Order Preservation Testing</h2>']
        html_strs.append(f'<p class="meta">CSV: <code>{csv_path.stem}</code></p>')

        preserved = op.order_preserved()

        status_cls = "ok" if preserved else "warn"
        status_txt = "Fully preserved" if preserved else "Mismatches detected"

        html_strs.append(f'<div class="summary-box"><strong class="{status_cls}">{status_txt}</strong></div>')
        rows = op.report_summary()
        html_strs.append(create_html_table(["Metric", "Value"], rows))

        # graphs - full range
        html_strs.append("<h3>Graphs - Full Range</h3>")
        graph_figs = self._ordering_graph_figs(op, id_range=self.id_range)
        
        html_strs.append('<div class="graph-grid">')
        for name, layout, fig in graph_figs:
            if layout != "grid":
                continue
            html_strs.append(get_html_img_tag(fig_encode(fig), name))
        html_strs.append("</div>")

        html_strs.append('<div class="graph-full">')
        for name, layout, fig in graph_figs:
            if layout != "full":
                continue
            html_strs.append(get_html_img_tag(fig_encode(fig), name))
        html_strs.append("</div>")

        # graphs - mismatch window (zoomed detail)
        mismatch_window = op.window_most_mismatches()
        if mismatch_window:
            html_strs.append("<h3>Graphs - Mismatch Detail (20% window with most mismatches)</h3>")
            html_strs.append(f'<p class="meta">Index range: {mismatch_window[0]} - {mismatch_window[1]}</p>')
            
            mismatch_figs = self._ordering_graph_figs(op, id_range=mismatch_window)
            
            html_strs.append('<div class="graph-grid">')
            for name, layout, fig in mismatch_figs:
                if layout != "grid":
                    continue
                html_strs.append(get_html_img_tag(fig_encode(fig), f"{name} (mismatch window)"))
            html_strs.append("</div>")
            
            html_strs.append('<div class="graph-full">')
            for name, layout, fig in mismatch_figs:
                if layout != "full":
                    continue
                html_strs.append(get_html_img_tag(fig_encode(fig), f"{name} (mismatch window)"))
            html_strs.append("</div>")

        return "\n".join(html_strs), mismatch_window

    def _ordering_graph_figs(self, op: OrderingPlot, id_range: tuple[int, int] | None = None) -> list[tuple[str, plt.Figure]]:
        if not id_range:
            id_range = self.id_range

        # return [
        #     ("Expected vs Actual", op.plot_out_of_order_pairs(id_range=id_range, return_fig=True)),
        #     ("Offset", op.plot_offset(id_range=id_range, return_fig=True)),
        # ]
        tags = []
        for _, method in inspect.getmembers(op, predicate=callable):
            meta = getattr(method, "_report_meta", None)
            if meta is None and hasattr(method, "__func__"):
                meta = getattr(method.__func__, "_report_figure_meta", None)
            if meta:
                tags.append((meta, method))

        tags.sort(key=lambda x: x[0].get("order", 1000))

        figs: list[tuple[str, str, plt.Figure]] = []
        for meta, method in tags:
            fig = method(return_fig=True, id_range=id_range)
            figs.append((meta["name"], meta.get("layout", "grid"), fig))

        return figs

    # ------------------------------------------------------------------
    # exchange
    # ------------------------------------------------------------------

    def _exchange_section(self, mismatch_window: tuple[int, int] | None = None):
        csv_path = self.files["exchange"]
        ep = ExchangePlot(csv_path)

        html_strs = ['<h2>Exchange / Matching Engine</h2>']
        html_strs.append(f'<p class="meta">CSV: <code>{csv_path.name}</code></p>')

        # summary stats from ExchangePlot
        summary_rows, diff_cycles = ep.report_summary()
        html_strs.append(create_html_table(["Metric", "Value"], summary_rows))

        # candlestick charts + overlay
        with tempfile.TemporaryDirectory() as tmpdir:
            imgs = ep.plot_report_candles(Path(tmpdir), cycle_range=self.id_range, mismatch_window=mismatch_window)

            html_strs.append("<h3>Candlestick Charts</h3>")
            html_strs.append('<div class="graph-grid">')

            if imgs["expected"] and imgs["actual"]:
                html_strs.append(get_html_img_tag(fig_encode_path(imgs["expected"]), "Expected candlestick"))
                html_strs.append(get_html_img_tag(fig_encode_path(imgs["actual"]), "Actual candlestick"))
            else:
                html_strs.append("<p>Error generating expected and actual trades: Images not found")
            html_strs.append("</div>")

            if imgs["overlay"]:
                html_strs.append("<h3>Overlay (Expected vs Actual)</h3>")
                html_strs.append('<div class="graph-full">')
                html_strs.append(get_html_img_tag(fig_encode_path(imgs["overlay"]), "Overlay"))
                html_strs.append("</div>")

            # zoomed overlay for mismatch window
            if imgs["overlay_zoom"] and mismatch_window:
                html_strs.append("<h3>Overlay - Mismatch Window Detail</h3>")
                html_strs.append(f'<p class="meta">Focused on order index range: {mismatch_window[0]} - {mismatch_window[1]}</p>')
                html_strs.append(f'<div class="summary-box"><strong>Note: The cycle size on the focused overlay is more granular compared to the original graph.</strong></div>')
                html_strs.append('<div class="graph-full">')
                html_strs.append(get_html_img_tag(fig_encode_path(imgs["overlay_zoom"]), "Overlay (zoomed to mismatch window)"))
                html_strs.append("</div>")

        return "\n".join(html_strs)

    # ------------------------------------------------------------------
    # hardware benchmarking (WIP WIP WIP)
    # ------------------------------------------------------------------

    def _hardware_section(
        self,
        hw_latency_csv: Path | None = None,
        hw_ordering_csv: Path | None = None,
    ):
        """
        Need to implement hardware logging / metrics before able to implement
        """
        if hw_latency_csv:
            hw = HardwarePlot(hw_latency_csv)
            title = "Latency"
        else:
            hw = HardwarePlot(hw_ordering_csv)
            title = "Ordering"

        html_strs = ['<h2>Hardware Counters</h2>']
        html_strs.append(f"<h3>Latency Hardware Metrics - {title}</h3>")
        rows = hw.report_summary()
        html_strs.append(create_html_table(["Metric", "Value"], rows))

        self.sections.append("\n".join(html_strs))

    # ------------------------------------------------------------------
    # HTML wrapper
    # ------------------------------------------------------------------

    def _wrap_html(self, body: str) -> str:
        now = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
        return f"""
                <!DOCTYPE html>
                <html lang="en">
                <head>
                <meta charset="utf-8">
                <meta name="viewport" content="width=device-width, initial-scale=1">
                <title>Benchmark Report — {self.run_id}</title>
                {_CSS}
                </head>
                <body>
                <h1>Benchmark Report</h1>
                <div class="meta">
                <strong>Run ID:</strong> {self.run_id}<br>
                <strong>Generated:</strong> {now}
                </div>
                {body}
                </body>
                </html>
            """
