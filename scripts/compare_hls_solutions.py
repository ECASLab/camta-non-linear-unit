#!/usr/bin/env python3
"""Post-synthesis comparison for Vitis HLS solutions.

Scans solution folders, extracts timing/latency/resources from csynth RPT
reports, and writes comparison reports (CSV + Markdown + charts).
"""

from __future__ import annotations

import argparse
import csv
import datetime as dt
import re
import sys
from dataclasses import dataclass
from pathlib import Path
from typing import Dict, List, Optional

try:
    import matplotlib

    # Use a headless backend to avoid Tk/Tcl conflicts under Vitis HLS.
    matplotlib.use("Agg", force=True)
    import matplotlib.pyplot as plt
except Exception:
    plt = None

RESOURCE_KEYS = ["BRAM_18K", "DSP", "FF", "LUT", "URAM"]


@dataclass
class SolutionMetrics:
    solution_name: str
    datetime_tag: str
    report_path: Path
    target_clock_ns: Optional[float]
    estimated_clock_ns: Optional[float]
    best_latency_cycles: Optional[int]
    worst_latency_cycles: Optional[int]
    total_cycles: Optional[int]
    interval_min_cycles: Optional[int]
    interval_max_cycles: Optional[int]
    resources: Dict[str, Optional[int]]
    available_resources: Dict[str, Optional[int]]


def _to_float(value: Optional[str]) -> Optional[float]:
    if value is None:
        return None
    try:
        return float(value)
    except ValueError:
        return None


def _to_int(value: Optional[str]) -> Optional[int]:
    if value is None:
        return None
    try:
        return int(float(value))
    except ValueError:
        return None


def _extract_datetime_tag(solution_name: str) -> str:
    m = re.search(r"(\d{8}_\d{6})$", solution_name)
    if m:
        return m.group(1)
    return "unknown"


def _find_report_file(solution_dir: Path, top_name: Optional[str]) -> Optional[Path]:
    report_dir = solution_dir / "syn" / "report"
    generic_report = report_dir / "csynth.rpt"

    if top_name:
        top_report = report_dir / f"{top_name}_csynth.rpt"
        if top_report.exists():
            return top_report

    if generic_report.exists():
        return generic_report

    rpt_candidates = sorted(report_dir.glob("*_csynth.rpt"))
    if rpt_candidates:
        return rpt_candidates[0]
    return None


def _parse_ns_token(token: str) -> Optional[float]:
    cleaned = token.replace("ns", "").strip()
    return _to_float(cleaned)


def _parse_int_token(token: str) -> Optional[int]:
    cleaned = token.strip()
    if cleaned in {"", "-", "~0"}:
        return None
    return _to_int(cleaned)


def _split_row(line: str) -> List[str]:
    return [part.strip() for part in line.strip().strip("|").split("|")]


def _extract_timing_latency(
    lines: List[str],
) -> tuple[Optional[float], Optional[float], Optional[int], Optional[int], Optional[int], Optional[int]]:
    target_clock = None
    estimated_clock = None
    best_lat = None
    worst_lat = None
    interval_min = None
    interval_max = None

    in_performance = False
    in_timing = False
    in_latency = False

    for line in lines:
        stripped = line.strip()

        if stripped.startswith("== Performance Estimates"):
            in_performance = True
            continue
        if in_performance and stripped.startswith("== Utilization Estimates"):
            in_performance = False
            in_timing = False
            in_latency = False
            continue
        if not in_performance:
            continue

        if stripped.startswith("+ Timing:"):
            in_timing = True
            in_latency = False
            continue
        if stripped.startswith("+ Latency:"):
            in_latency = True
            in_timing = False
            continue

        if in_timing and stripped.startswith("|ap_clk"):
            cols = _split_row(stripped)
            if len(cols) >= 3:
                target_clock = _parse_ns_token(cols[1])
                estimated_clock = _parse_ns_token(cols[2])

        elif in_latency and re.match(r"^\|\s*\d+\s*\|\s*\d+\s*\|", stripped):
            cols = _split_row(stripped)
            if len(cols) >= 6 and best_lat is None:
                best_lat = _parse_int_token(cols[0])
                worst_lat = _parse_int_token(cols[1])
                interval_min = _parse_int_token(cols[4])
                interval_max = _parse_int_token(cols[5])

    return target_clock, estimated_clock, best_lat, worst_lat, interval_min, interval_max


def _extract_resource_tables(
    lines: List[str],
) -> tuple[Dict[str, Optional[int]], Dict[str, Optional[int]]]:
    resources = {key: None for key in RESOURCE_KEYS}
    available_resources = {key: None for key in RESOURCE_KEYS}

    in_util = False
    for line in lines:
        stripped = line.strip()

        if stripped.startswith("== Utilization Estimates"):
            in_util = True
            continue
        if in_util and stripped.startswith("+ Detail:"):
            break
        if not in_util:
            continue

        if stripped.startswith("|Total"):
            cols = _split_row(stripped)
            if len(cols) >= 6:
                resources["BRAM_18K"] = _parse_int_token(cols[1])
                resources["DSP"] = _parse_int_token(cols[2])
                resources["FF"] = _parse_int_token(cols[3])
                resources["LUT"] = _parse_int_token(cols[4])
                resources["URAM"] = _parse_int_token(cols[5])

        if stripped.startswith("|Available"):
            cols = _split_row(stripped)
            if len(cols) >= 6:
                available_resources["BRAM_18K"] = _parse_int_token(cols[1])
                available_resources["DSP"] = _parse_int_token(cols[2])
                available_resources["FF"] = _parse_int_token(cols[3])
                available_resources["LUT"] = _parse_int_token(cols[4])
                available_resources["URAM"] = _parse_int_token(cols[5])

    return resources, available_resources


def parse_solution(
    solution_dir: Path,
    top_name: Optional[str],
    report_file: Optional[Path] = None,
) -> Optional[SolutionMetrics]:
    if report_file is None:
        report_file = _find_report_file(solution_dir, top_name)
    if report_file is None:
        return None

    try:
        lines = report_file.read_text(encoding="utf-8", errors="ignore").splitlines()
    except OSError:
        return None

    (
        target_clock_ns,
        estimated_clock_ns,
        best_latency_cycles,
        worst_latency_cycles,
        interval_min_cycles,
        interval_max_cycles,
    ) = _extract_timing_latency(lines)
    resources, available_resources = _extract_resource_tables(lines)

    solution_name = solution_dir.name
    return SolutionMetrics(
        solution_name=solution_name,
        datetime_tag=_extract_datetime_tag(solution_name),
        report_path=report_file,
        target_clock_ns=target_clock_ns,
        estimated_clock_ns=estimated_clock_ns,
        best_latency_cycles=best_latency_cycles,
        worst_latency_cycles=worst_latency_cycles,
        total_cycles=worst_latency_cycles,
        interval_min_cycles=interval_min_cycles,
        interval_max_cycles=interval_max_cycles,
        resources=resources,
        available_resources=available_resources,
    )


def discover_solutions(project_dir: Path, top_name: Optional[str], report_glob: str) -> List[SolutionMetrics]:
    rows: List[SolutionMetrics] = []
    seen: set[Path] = set()

    for report in project_dir.glob(report_glob):
        if not report.is_file():
            continue
        try:
            solution_dir = report.parents[2]
        except IndexError:
            continue

        if not solution_dir.name.startswith("solution"):
            continue

        resolved = solution_dir.resolve()
        if resolved in seen:
            continue
        seen.add(resolved)

        parsed = parse_solution(solution_dir, top_name, report_file=report)
        if parsed is not None:
            rows.append(parsed)

    def sort_key(item: SolutionMetrics):
        try:
            return dt.datetime.strptime(item.datetime_tag, "%Y%m%d_%H%M%S")
        except ValueError:
            return dt.datetime.min

    rows.sort(key=sort_key)
    return rows


def _pct(used: Optional[int], available: Optional[int]) -> Optional[float]:
    if used is None or available in (None, 0):
        return None
    return (100.0 * used) / available


def _fmt_num(v: Optional[float], digits: int = 3) -> str:
    if v is None:
        return "NA"
    return f"{v:.{digits}f}"


def _fmt_int(v: Optional[int]) -> str:
    return "NA" if v is None else str(v)


def write_csv(rows: List[SolutionMetrics], out_csv: Path) -> None:
    headers = [
        "solution_name",
        "datetime_tag",
        "estimated_clock_ns",
        "target_clock_ns",
        "clock_slack_ns_target_minus_estimated",
        "total_cycles",
        "best_latency_cycles",
        "worst_latency_cycles",
        "interval_min_cycles",
        "interval_max_cycles",
    ]

    for key in RESOURCE_KEYS:
        headers.extend([f"{key}_used", f"{key}_avail", f"{key}_util_pct"])

    with out_csv.open("w", newline="", encoding="utf-8") as fp:
        writer = csv.DictWriter(fp, fieldnames=headers)
        writer.writeheader()

        for row in rows:
            record = {
                "solution_name": row.solution_name,
                "datetime_tag": row.datetime_tag,
                "estimated_clock_ns": row.estimated_clock_ns,
                "target_clock_ns": row.target_clock_ns,
                "clock_slack_ns_target_minus_estimated": (
                    None
                    if row.target_clock_ns is None or row.estimated_clock_ns is None
                    else row.target_clock_ns - row.estimated_clock_ns
                ),
                "total_cycles": row.total_cycles,
                "best_latency_cycles": row.best_latency_cycles,
                "worst_latency_cycles": row.worst_latency_cycles,
                "interval_min_cycles": row.interval_min_cycles,
                "interval_max_cycles": row.interval_max_cycles,
            }

            for key in RESOURCE_KEYS:
                used = row.resources.get(key)
                avail = row.available_resources.get(key)
                record[f"{key}_used"] = used
                record[f"{key}_avail"] = avail
                record[f"{key}_util_pct"] = _pct(used, avail)

            writer.writerow(record)


def write_markdown(rows: List[SolutionMetrics], out_md: Path, current_solution: Optional[str]) -> None:
    best_clock = min((r.estimated_clock_ns for r in rows if r.estimated_clock_ns is not None), default=None)
    best_lut = min((r.resources.get("LUT") for r in rows if r.resources.get("LUT") is not None), default=None)

    lines: List[str] = []
    lines.append("# HLS Solution Comparison")
    lines.append("")
    lines.append(f"Generated: {dt.datetime.now().strftime('%Y-%m-%d %H:%M:%S')}")
    lines.append(f"Solutions analyzed: {len(rows)}")
    if current_solution:
        lines.append(f"Current solution: `{current_solution}`")
    lines.append("")

    lines.append("## Timing and Latency")
    lines.append("")
    lines.append("| Solution | Est. Clock (ns) | Target (ns) | Slack (ns) | Total Cycles | Best Lat (cyc) | Worst Lat (cyc) |")
    lines.append("|---|---:|---:|---:|---:|---:|---:|")
    for r in rows:
        slack = None
        if r.target_clock_ns is not None and r.estimated_clock_ns is not None:
            slack = r.target_clock_ns - r.estimated_clock_ns
        mark = ""
        if best_clock is not None and r.estimated_clock_ns == best_clock:
            mark = " <- best"
        lines.append(
            "| "
            + f"`{r.solution_name}` | {_fmt_num(r.estimated_clock_ns)} | {_fmt_num(r.target_clock_ns)}"
            + f" | {_fmt_num(slack)} | {_fmt_int(r.total_cycles)} | {_fmt_int(r.best_latency_cycles)} | {_fmt_int(r.worst_latency_cycles)}{mark} |"
        )

    lines.append("")
    lines.append("## Resource Usage")
    lines.append("")
    lines.append("| Solution | BRAM_18K | DSP | FF | LUT | URAM |")
    lines.append("|---|---:|---:|---:|---:|---:|")
    for r in rows:
        lut_mark = ""
        if best_lut is not None and r.resources.get("LUT") == best_lut:
            lut_mark = " <- best LUT"
        lines.append(
            "| "
            + f"`{r.solution_name}`"
            + f" | {_fmt_int(r.resources.get('BRAM_18K'))}"
            + f" | {_fmt_int(r.resources.get('DSP'))}"
            + f" | {_fmt_int(r.resources.get('FF'))}"
            + f" | {_fmt_int(r.resources.get('LUT'))}{lut_mark}"
            + f" | {_fmt_int(r.resources.get('URAM'))} |"
        )

    lines.append("")
    lines.append("## Resource Utilization (%)")
    lines.append("")
    lines.append("| Solution | BRAM_18K % | DSP % | FF % | LUT % | URAM % |")
    lines.append("|---|---:|---:|---:|---:|---:|")
    for r in rows:
        lines.append(
            "| "
            + f"`{r.solution_name}`"
            + f" | {_fmt_num(_pct(r.resources.get('BRAM_18K'), r.available_resources.get('BRAM_18K')), 2)}"
            + f" | {_fmt_num(_pct(r.resources.get('DSP'), r.available_resources.get('DSP')), 2)}"
            + f" | {_fmt_num(_pct(r.resources.get('FF'), r.available_resources.get('FF')), 2)}"
            + f" | {_fmt_num(_pct(r.resources.get('LUT'), r.available_resources.get('LUT')), 2)}"
            + f" | {_fmt_num(_pct(r.resources.get('URAM'), r.available_resources.get('URAM')), 2)} |"
        )

    out_md.write_text("\n".join(lines) + "\n", encoding="utf-8")


def print_console_summary(rows: List[SolutionMetrics]) -> None:
    print("\n[HLS] Solution comparison summary")
    print("Solution                                      EstClk(ns)  TotalCycles  LUT")
    print("-" * 78)
    for r in rows:
        print(
            f"{r.solution_name:<44}  {_fmt_num(r.estimated_clock_ns):>10}"
            f"  {_fmt_int(r.total_cycles):>11}  {_fmt_int(r.resources.get('LUT')):>5}"
        )


def _labels(rows: List[SolutionMetrics]) -> List[str]:
    return [r.datetime_tag if r.datetime_tag != "unknown" else r.solution_name for r in rows]


def _bar_values_int(rows: List[SolutionMetrics], getter) -> List[int]:
    values: List[int] = []
    for r in rows:
        value = getter(r)
        values.append(0 if value is None else int(value))
    return values


def _bar_values_float(rows: List[SolutionMetrics], getter) -> List[float]:
    values: List[float] = []
    for r in rows:
        value = getter(r)
        values.append(0.0 if value is None else float(value))
    return values


def write_charts(rows: List[SolutionMetrics], chart_dir: Path) -> List[Path]:
    if plt is None:
        print("[HLS] matplotlib is not installed. Skipping chart generation.")
        return []

    chart_dir.mkdir(parents=True, exist_ok=True)
    labels = _labels(rows)
    output_files: List[Path] = []

    fig, ax = plt.subplots(figsize=(10, 5))
    ax.bar(labels, _bar_values_float(rows, lambda r: r.estimated_clock_ns), color="#1f77b4")
    ax.set_title("Estimated Clock Comparison")
    ax.set_ylabel("ns")
    ax.grid(axis="y", linestyle="--", alpha=0.4)
    plt.xticks(rotation=30, ha="right")
    plt.tight_layout()
    p = chart_dir / "timing_estimated_clock.png"
    fig.savefig(p, dpi=140)
    plt.close(fig)
    output_files.append(p)

    fig, ax = plt.subplots(figsize=(10, 5))
    ax.bar(labels, _bar_values_int(rows, lambda r: r.total_cycles), color="#ff7f0e")
    ax.set_title("Total Cycles Comparison")
    ax.set_ylabel("cycles")
    ax.grid(axis="y", linestyle="--", alpha=0.4)
    plt.xticks(rotation=30, ha="right")
    plt.tight_layout()
    p = chart_dir / "latency_total_cycles.png"
    fig.savefig(p, dpi=140)
    plt.close(fig)
    output_files.append(p)

    x = list(range(len(labels)))
    width = 0.16
    fig, ax = plt.subplots(figsize=(12, 6))
    ax.bar([i - 2 * width for i in x], _bar_values_int(rows, lambda r: r.resources.get("BRAM_18K")), width=width, label="BRAM_18K")
    ax.bar([i - width for i in x], _bar_values_int(rows, lambda r: r.resources.get("DSP")), width=width, label="DSP")
    ax.bar(x, _bar_values_int(rows, lambda r: r.resources.get("FF")), width=width, label="FF")
    ax.bar([i + width for i in x], _bar_values_int(rows, lambda r: r.resources.get("LUT")), width=width, label="LUT")
    ax.bar([i + 2 * width for i in x], _bar_values_int(rows, lambda r: r.resources.get("URAM")), width=width, label="URAM")
    ax.set_title("Resource Totals Comparison")
    ax.set_ylabel("count")
    ax.set_xticks(x)
    ax.set_xticklabels(labels, rotation=30, ha="right")
    ax.grid(axis="y", linestyle="--", alpha=0.4)
    ax.legend()
    plt.tight_layout()
    p = chart_dir / "resources_totals.png"
    fig.savefig(p, dpi=140)
    plt.close(fig)
    output_files.append(p)

    fig, ax = plt.subplots(figsize=(10, 5))
    lut_util = _bar_values_float(rows, lambda r: _pct(r.resources.get("LUT"), r.available_resources.get("LUT")))
    ff_util = _bar_values_float(rows, lambda r: _pct(r.resources.get("FF"), r.available_resources.get("FF")))
    ax.plot(labels, lut_util, marker="o", label="LUT %")
    ax.plot(labels, ff_util, marker="o", label="FF %")
    ax.set_title("Utilization Comparison")
    ax.set_ylabel("percent")
    ax.grid(axis="y", linestyle="--", alpha=0.4)
    ax.legend()
    plt.xticks(rotation=30, ha="right")
    plt.tight_layout()
    p = chart_dir / "utilization_percent.png"
    fig.savefig(p, dpi=140)
    plt.close(fig)
    output_files.append(p)

    return output_files


def main() -> int:
    parser = argparse.ArgumentParser(description="Compare Vitis HLS solution reports")
    parser.add_argument("--project-dir", default=".", help="Workspace/project root directory")
    parser.add_argument("--top", default=None, help="Top function name (optional, used for file naming)")
    parser.add_argument(
        "--report-glob",
        default="horner_core*/solution*/syn/report/*_csynth.rpt",
        help="Glob pattern relative to --project-dir to discover csynth reports",
    )
    parser.add_argument("--current-solution", default=None, help="Current solution name")
    parser.add_argument(
        "--output-dir",
        default="comparison_reports",
        help="Directory where CSV/Markdown/charts are written",
    )
    args = parser.parse_args()

    project_dir = Path(args.project_dir).resolve()
    output_dir = project_dir / args.output_dir
    output_dir.mkdir(parents=True, exist_ok=True)

    rows = discover_solutions(project_dir, args.top, args.report_glob)
    if not rows:
        print(f"[HLS] No valid solution reports were found for pattern: {args.report_glob}")
        return 1

    report_tag = args.top if args.top else "all"
    csv_path = output_dir / f"solution_comparison_{report_tag}.csv"
    md_path = output_dir / f"solution_comparison_{report_tag}.md"
    chart_dir = output_dir / "charts"

    write_csv(rows, csv_path)
    write_markdown(rows, md_path, args.current_solution)

    try:
        chart_files = write_charts(rows, chart_dir)
    except Exception as exc:
        chart_files = []
        print(f"[HLS] Chart generation failed, continuing without charts: {exc}")

    print_console_summary(rows)
    print(f"\n[HLS] CSV report: {csv_path}")
    print(f"[HLS] Markdown report: {md_path}")
    if chart_files:
        print("[HLS] Charts generated:")
        for chart in chart_files:
            print(f"  - {chart}")

    return 0


if __name__ == "__main__":
    sys.exit(main())
