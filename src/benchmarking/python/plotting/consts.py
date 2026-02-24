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