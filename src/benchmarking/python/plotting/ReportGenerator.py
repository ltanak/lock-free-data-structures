import base64
import io
import tempfile
from datetime import datetime
from pathlib import Path
import matplotlib.pyplot as plt
import numpy as np
import pandas as pd

from benchmarking.python.plotting.ExchangePlot import ExchangePlot
from benchmarking.python.plotting.LatencyPlot import LatencyPlot
from benchmarking.python.plotting.OrderingPlot import OrderingPlot
from benchmarking.python.plotting.utils import *



def _table_html(headers: list[str], rows: list[list[str]]) -> str:
    """Build a simple HTML table."""
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


# ---------------------------------------------------------------------------
# CSS
# ---------------------------------------------------------------------------

_CSS = """
<style>
  :root { --bg: #fdfdfd; --fg: #222; --accent: #3366cc; --border: #ddd; }
  * { box-sizing: border-box; }
  body {
    font-family: 'Segoe UI', system-ui, sans-serif;
    max-width: 1100px; margin: 0 auto; padding: 2rem;
    background: var(--bg); color: var(--fg);
    line-height: 1.6;
  }
  h1 { border-bottom: 3px solid var(--accent); padding-bottom: .4em; }
  h2 { color: var(--accent); margin-top: 2.5rem; }
  h3 { margin-top: 1.5rem; }
  table {
    border-collapse: collapse; width: 100%; margin: 1rem 0;
    font-size: 0.95rem;
  }
  th, td {
    border: 1px solid var(--border); padding: .45rem .7rem; text-align: left;
  }
  th { background: #f0f4fa; }
  tr:nth-child(even) { background: #fafafa; }
  .graph-grid {
    display: grid; grid-template-columns: 1fr 1fr; gap: 1.2rem;
    margin: 1rem 0;
  }
  .graph-grid img { width: 100%; border: 1px solid var(--border); border-radius: 4px; }
  .graph-full { margin: 1rem 0; }
  .graph-full img { width: 100%; border: 1px solid var(--border); border-radius: 4px; }
  .meta { color: #666; font-size: 0.9rem; margin-bottom: 1.5rem; }
  .summary-box {
    background: #f7f9fc; border-left: 4px solid var(--accent);
    padding: 1rem 1.2rem; margin: 1rem 0; border-radius: 0 4px 4px 0;
  }
  .warn { color: #b94a00; }
  .ok   { color: #1a7f37; }
</style>
"""


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

        if self.active.get("latencies") and "latencies" in self.files:
            latency_strs = self._latency_section()
            self.sections.append(latency_strs)

        if self.active.get("ordering") and "ordering" in self.files:
            ordering_strs = self._ordering_section()
            self.sections.append(ordering_strs)

        if self.active.get("exchange") and "exchange" in self.files:
            exchange_strs = self._exchange_section()
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
            html_strs.append(_table_html(["Metric", "Value"], report_summary_rows))

            # --------------------
            # LATENCY PLOT FIGURES
            # --------------------

            fig_hist = lp.histogram(metric=metric, return_fig=True)
            fig_cdf = lp.cdf(metric=metric, return_fig=True)

            html_strs.append('<div class="graph-grid">')
            html_strs.append(get_html_img_tag(fig_encode(fig_hist), f"{metric} histogram"))
            html_strs.append(get_html_img_tag(fig_encode(fig_cdf), f"{metric} CDF"))
            html_strs.append("</div>")

            # percentile graph
            fig_pct = lp.percentile_chart(metric=metric, return_fig=True)
            html_strs.append('<div class="graph-full">')
            html_strs.append(get_html_img_tag(fig_encode(fig_pct), f"{metric} percentile"))
            html_strs.append("</div>")

        return "\n".join(html_strs)

    # ------------------------------------------------------------------
    # ordering
    # ------------------------------------------------------------------

    def _ordering_section(self):
        csv_path = self.files["ordering"]
        op = OrderingPlot(csv_path)

        html_strs = ['<h2>Order Preservation Testing</h2>']
        html_strs.append(f'<p class="meta">CSV: <code>{csv_path.stem}</code></p>')

        preserved = op.order_preserved()

        # longest contiguous mismatch chain
        # longest_chain = self._longest_mismatch_chain(mismatch_idx)

        status_cls = "ok" if preserved else "warn"
        status_txt = "Fully preserved" if preserved else "Mismatches detected"

        html_strs.append(f'<div class="summary-box"><strong class="{status_cls}">{status_txt}</strong></div>')
        rows = op.report_summary()
        html_strs.append(_table_html(["Metric", "Value"], rows))

        # graphs
        html_strs.append("<h3>Graphs</h3>")
        graph_figs = self._ordering_graph_figs(op)
        html_strs.append('<div class="graph-grid">')
        for label, fig in graph_figs:
            html_strs.append(get_html_img_tag(fig_encode(fig), label))
        html_strs.append("</div>")

        return "\n".join(html_strs)

    def _ordering_graph_figs(self, op: OrderingPlot) -> list[tuple[str, plt.Figure]]:
        id_range = self.id_range
        return [
            ("Expected vs Actual",              op.plot_out_of_order_pairs(id_range=id_range, return_fig=True)),
            ("Offset",                          op.plot_offset(id_range=id_range, return_fig=True)),
            ("Displacement Heatmap",            op.plot_displacement_heatmap(id_range=id_range, return_fig=True)),
            ("Colored by Displacement",         op.plot_expected_vs_actual_colored(id_range=id_range, return_fig=True)),
        ]

    # ------------------------------------------------------------------
    # exchange
    # ------------------------------------------------------------------

    def _exchange_section(self):
        csv_path = self.files["exchange"]
        ep = ExchangePlot(csv_path)

        s = ['<h2>Exchange / Matching Engine</h2>']
        s.append(f'<p class="meta">CSV: <code>{csv_path.name}</code></p>')

        # summary stats from ExchangePlot
        summary_rows, diff_cycles = ep.report_summary()
        s.append(_table_html(["Metric", "Value"], summary_rows))

        # candlestick charts + overlay
        with tempfile.TemporaryDirectory() as tmpdir:
            imgs = ep.plot_report_candles(Path(tmpdir), cycle_range=self.id_range)

            s.append("<h3>Candlestick Charts</h3>")
            s.append('<div class="graph-grid">')

            if imgs["expected"]:
                s.append(get_html_img_tag(fig_encode_path(imgs["expected"]), "Expected candlestick"))
            else:
                s.append("<p>No expected trades found.</p>")

            if imgs["actual"]:
                s.append(get_html_img_tag(fig_encode_path(imgs["actual"]), "Actual candlestick"))
            else:
                s.append("<p>No actual trades found.</p>")

            s.append("</div>")

            if imgs["overlay"]:
                s.append("<h3>Overlay (Expected vs Actual)</h3>")
                s.append('<div class="graph-full">')
                s.append(get_html_img_tag(fig_encode_path(imgs["overlay"]), "Overlay"))
                s.append("</div>")

        return "\n".join(s)

    # ------------------------------------------------------------------
    # hardware benchmarking (stub for future use)
    # ------------------------------------------------------------------

    def build_hardware_section(
        self,
        hw_latency_csv: Path | None = None,
        hw_ordering_csv: Path | None = None,
    ):
        """
        Read hardware-counter CSVs and append a summary section.

        Call this *before* generate() so the section is included.
        Expected CSV columns (adjust once format is finalised):
          - hw latency:  metric, value  (e.g. cache-misses, branch-misses …)
          - hw ordering: metric, value

        This is a stub — expand the body once the CSV schema is known.
        """
        s = ['<h2>Hardware Counters</h2>']

        if hw_latency_csv and hw_latency_csv.exists():
            s.append("<h3>Latency Hardware Metrics</h3>")
            s.append(self._hw_csv_summary(hw_latency_csv))
        else:
            s.append("<p><em>No hardware-latency CSV provided.</em></p>")

        if hw_ordering_csv and hw_ordering_csv.exists():
            s.append("<h3>Ordering Hardware Metrics</h3>")
            s.append(self._hw_csv_summary(hw_ordering_csv))
        else:
            s.append("<p><em>No hardware-ordering CSV provided.</em></p>")

        self.sections.append("\n".join(s))

    @staticmethod
    def _hw_csv_summary(csv_path: Path) -> str:
        """
        Generic hardware CSV → HTML table.

        Tries to auto-detect the format:
          • Two-column (metric, value) → key/value table.
          • Multi-column → full table with all columns.
        """
        df = pd.read_csv(csv_path)
        headers = list(df.columns)
        rows = [list(str(v) for v in row) for _, row in df.iterrows()]
        return _table_html(headers, rows)

    # ------------------------------------------------------------------
    # HTML wrapper
    # ------------------------------------------------------------------

    def _wrap_html(self, body: str) -> str:
        now = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
        return f"""<!DOCTYPE html>
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
