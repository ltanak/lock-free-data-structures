# Python Pipeline

The Python side exists to turn the CSVs that the C++ benchmark drops into `results/` into something a human can actually read. In practice that means three things: standalone matplotlib plots for quick inspection, a single aggregated HTML report per run ID, and a separate cross-run comparison helper for overlaying latency curves from several runs onto one image.

## Layout

```
src/benchmarking/python/
  pixi.toml                     Environment + task definitions
  scripts/
    main.py                     CLI entry point
  plotting/
    LatencyPlot.py              Latency histograms, CDFs, percentile bars
    OrderingPlot.py             Order-preservation plots
    ExchangePlot.py             Matching-engine candle plots
    HardwarePlot.py              PAPI counter summaries
    ReportGenerator.py          Aggregates plots into one HTML file
    utils.py                    CSV discovery and helpers
    consts.py                   Shared constants
  image_editing/
    image.py                    Two-image alpha compositor
    compare_runs.py             Multi-run latency overlay
```

## Environment

Everything runs through [pixi](https://pixi.sh). From `src/benchmarking/python` the four useful invocations are:

```bash
pixi run report                     # report for the latest run
pixi run report --run-id=<RUN_ID>   # report for a specific run
pixi run main --help                # full flag listing
pixi run compare --run-ids 1 2 3    # overlay latency plots across runs
```

`pixi.toml` declares the dependencies (`numpy`, `pandas`, `matplotlib`, `mplfinance`, `pillow`) and the four shortcut tasks.

## scripts/main.py

`main.py` is the single entry point for plotting and report generation. It figures out which CSV files to load — by `--run-id`, by `--file`, or by "most recent" — then either generates an HTML report through `ReportGenerator` (the default behaviour of the `pixi run report` task), or produces standalone plots from one or more of `LatencyPlot`, `OrderingPlot` and `ExchangePlot` depending on which of `--latencies / --ordering / --exchange / --all` were passed.

The flags worth knowing about:

| Flag | Description |
| --- | --- |
| `--run-id <id>` | Load every CSV category for the given run ID |
| `--file <name>` | Load a CSV by filename across every category |
| `--id-range START END` | Restrict plots to a specific sequence range |
| `--ls` | List every run ID discoverable on disk |
| `--must-match` | Fail if loaded CSVs disagree on run ID / timestamp |
| `--latencies / --ordering / --exchange / --all` | Pick which plots to produce |
| `--report` | Produce the aggregated HTML report instead of standalone plots |

## plotting/utils.py

`utils.py` is the glue everything else relies on. It resolves paths into `results/` (`get_root`, `get_csv_dir`, `get_graph_dir`, `get_report_dir`); finds the most recent CSV in each category (`get_latest_csv`, optionally requiring a shared run ID); collects all CSVs for a given run ID (`get_csv_by_run_id`); and lists every run ID present on disk (`get_all_run_ids`). It also parses the `<category>_<run_id>_<MM_DD_YYYY_HH_MM_SS>.csv` filename format via `extract_run_id` / `extract_timestamp`.

The helpers in this file that the report generator leans on are `fig_encode` and `fig_encode_path` for embedding figures as base64 images, `get_html_img_tag` for the surrounding `<img>` markup, and `create_html_table` for rendering summary tables.

The `@report` decorator at the bottom of the file is what plumbs individual plotting methods into the HTML report. Decorating a method with `@report(name="...", layout="grid"|"full", rank=N)` attaches metadata that `ReportGenerator` later finds via `inspect`. `layout` controls whether the figure gets tiled in a grid or takes the full page width; `rank` controls ordering within a section.

## Plot classes

All four plot classes follow the same shape. The constructor reads the relevant CSV with pandas; individual plotting methods are decorated with `@report(...)` so they show up in the HTML report; and a `plot_all(...)` method produces standalone PNGs for the `pixi run main` workflow.

`LatencyPlot` reads a latency CSV (`enqueue_latency_ns`, `dequeue_latency_ns`) and produces histograms (linear or log-log depending on the spread of the data), CDFs, and a percentile bar chart for the usual P50 / P75 / P90 / P95 / P99 / P99.5 / P99.9 set. It also exposes `summary` and `report_summary` for the textual numbers that go alongside the plots in the HTML report. Outlier removal uses a 1.5x IQR cutoff and is on by default.

`OrderingPlot` reads the ordering CSV produced by the order scenario (which records expected vs. actual sequence numbers per item) and produces mismatch plots and the corresponding summary statistics.

`ExchangePlot` reads the exchange CSV and renders the matching engine output as candlestick-style plots via `mplfinance`. For order test runs it can produce expected-versus-actual comparisons too.

`HardwarePlot` reads the PAPI hardware counter CSV and produces per-thread summary tables and bar charts of cache misses, branch mispredictions, instructions retired, and the rest of the counter set.

## ReportGenerator

`ReportGenerator` is the thing that ties the plot classes together into a single HTML file. It is constructed with the `{category: csv_path}` dict that `main.py` resolved, plus the run ID and the dict of which sections are active. For each active category it instantiates the matching plot class, walks its `@report`-decorated methods via `inspect`, renders each figure in-memory, base64-encodes it, and splices it into the page next to the relevant summary table. The output is written to:

```
results/reports/report_<run_id>.html
```

Open it in any browser. There is no JavaScript and no external assets — every image is embedded as a data URL — so the report file is fully self-contained and easy to ship around.

## image_editing/

`image_editing/` is for things that operate on already-rendered PNGs rather than on the underlying CSVs.

`image.py` is the two-image overlay helper. Given two PNGs of the same plot — for example the expected vs. actual matching outputs from a single run — `ImageEditor` produces a semi-transparent alpha composite and saves it back into `image_editing/`. It is mostly useful for spotting per-cycle divergence visually.

`compare_runs.py` is the cross-run latency overlay. It takes a list of run IDs and produces three side-by-side comparison images: a CDF overlay, a step-style histogram overlay (step histograms scale much better than filled bars when there are 3+ runs on a single axis), and a grouped percentile bar chart. Each run gets its own colour from matplotlib's `tab10` cycle, plus a legend entry. Output filenames encode the metric and the participating run IDs so successive comparisons don't trample each other:

```
compare_cdf_<metric>_<runA>_<runB>_<runC>.png
compare_hist_<metric>_<runA>_<runB>_<runC>.png
compare_percentiles_<metric>_<runA>_<runB>_<runC>.png
```

Quickest invocation:

```bash
pixi run compare --run-ids 1234567 7654321 1122334
```

The supported flags are `--metric enqueue|dequeue` (defaults to enqueue, only one metric per invocation so the filenames stay unambiguous), `--labels "RegularQueue" "Rigtorp" "Wilt"` for nicer legend entries than raw run IDs, `--no-outlier-removal` to skip the 1.5x IQR filter, and `--out-dir <path>` to redirect the outputs somewhere other than `image_editing/`.

## Adding a plot to the report

The pattern for adding a new plotting method that participates in the HTML report is:

1. Add the method to the relevant `*Plot` class.
2. Decorate it with `@report(name="...", layout="grid"|"full", rank=N)`.
3. Make it accept at least `return_fig=False` and `id_range=None` as keyword arguments — the report generator calls every method uniformly.
4. Return a matplotlib figure when `return_fig=True`; otherwise save to `out` if it was provided, or fall back to `plt.show()`.

The next report generation will pick the new method up automatically through introspection — there is no central registry to update.