#!/usr/bin/env python3
"""Regenerate the Branch_Predictor README charts from the simulator's own output files.

The per-run output files are the source of truth for the rates (the checked-in
CSVs originally had their GCC and PERL columns swapped), so this script
parses the misprediction rates straight out of the per-run
output files under graph/graph1/ and graph/graph2/.

Usage (from a checkout of the repo, or point --repo at one):
    python3 make_charts.py [--repo /path/to/Branch_Predictor] [--out docs]

Outputs (into --out, default "docs"):
    graph1-bimodal.svg      bimodal misprediction rate vs. m (7..16), 3 traces
    graph2-gshare-n.svg     gshare (GCC) misprediction rate vs. n, m = 8, 12, 16
    graph3-gshare-best.svg  gshare best-over-n misprediction rate vs. m, 3 traces
    graph1-data.csv         corrected bimodal table (from the output files)
    graph2-data.csv         corrected gshare table (from the output files)
"""
import argparse
import csv
import os
import re
import sys

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt

RATE_RE = re.compile(r"misprediction rate:\s+([\d.]+)%")
TRACES = ["gcc", "jpeg", "perl"]
COLORS = {"gcc": "#4285F4", "jpeg": "#34A853", "perl": "#EA4335"}
LABELS = {"gcc": "GCC", "jpeg": "JPEG", "perl": "Perl"}


def rate_from_file(path):
    with open(path) as f:
        for line in f:
            m = RATE_RE.search(line)
            if m:
                return float(m.group(1))
    raise ValueError(f"no misprediction rate in {path}")


def load_graph1(repo):
    """Bimodal: {m: {trace: rate}} for m = 7..16."""
    data = {}
    for m in range(7, 17):
        data[m] = {}
        for t in TRACES:
            data[m][t] = rate_from_file(
                os.path.join(repo, "graph", "graph1", f"output{t}_m{m}.txt"))
    return data


def load_graph2(repo):
    """Gshare: {(m, n): {trace: rate}} for m = 7..16, n = 2..m."""
    data = {}
    for m in range(7, 17):
        for n in range(2, m + 1):
            data[(m, n)] = {}
            for t in TRACES:
                data[(m, n)][t] = rate_from_file(
                    os.path.join(repo, "graph", "graph2", f"output{t}_m{m}{n}.txt"))
    return data


def style(ax, title, xlabel, ylabel):
    ax.set_title(title, fontsize=13, fontweight="bold")
    ax.set_xlabel(xlabel, fontsize=11)
    ax.set_ylabel(ylabel, fontsize=11)
    ax.grid(True, linestyle="--", alpha=0.4)
    ax.legend(fontsize=10, loc="best")
    for s in ("top", "right"):
        ax.spines[s].set_visible(False)


def chart_bimodal(g1, out):
    ms = sorted(g1)
    fig, ax = plt.subplots(figsize=(9, 5))
    for t in TRACES:
        ax.plot(ms, [g1[m][t] for m in ms], marker="o", markersize=5,
                color=COLORS[t], label=LABELS[t])
        ax.annotate(f"{g1[16][t]:.2f}%", (16, g1[16][t]), textcoords="offset points",
                    xytext=(6, 2), fontsize=9, color=COLORS[t])
    ax.set_xticks(ms)
    style(ax, "Bimodal: misprediction rate vs. table index bits (m)",
          "m — PC index bits (table has 2^m entries)", "Misprediction rate (%)")
    fig.tight_layout()
    fig.savefig(out, format="svg")
    plt.close(fig)


def chart_gshare_n(g2, out):
    fig, ax = plt.subplots(figsize=(9, 5))
    for m, color in ((8, "#FBBC05"), (12, "#4285F4"), (16, "#EA4335")):
        ns = [n for n in range(2, m + 1)]
        ax.plot(ns, [g2[(m, n)]["gcc"] for n in ns], marker="o", markersize=5,
                color=color, label=f"m = {m}")
        best_n = min(ns, key=lambda n: g2[(m, n)]["gcc"])
        ax.annotate(f"best n={best_n} ({g2[(m, best_n)]['gcc']:.2f}%)",
                    (best_n, g2[(m, best_n)]["gcc"]), textcoords="offset points",
                    xytext=(4, 8), fontsize=9, color=color)
    ax.set_xticks(range(2, 17))
    style(ax, "Gshare (GCC): misprediction rate vs. history bits (n)",
          "n — global branch history bits (n ≤ m)", "Misprediction rate (%)")
    fig.tight_layout()
    fig.savefig(out, format="svg")
    plt.close(fig)


def chart_gshare_best(g2, out):
    ms = sorted({m for (m, n) in g2})
    fig, ax = plt.subplots(figsize=(9, 5))
    for t in TRACES:
        best = [min(g2[(m, n)][t] for n in range(2, m + 1)) for m in ms]
        ax.plot(ms, best, marker="o", markersize=5, color=COLORS[t], label=LABELS[t])
    ax.axvline(15, color="#666666", linestyle=":", linewidth=1.2)
    ax.annotate("report design point\nm = 15, n = 9", (15, 8.6), textcoords="offset points",
                xytext=(-8, 6), fontsize=9, color="#555555")
    ax.set_xticks(ms)
    style(ax, "Gshare: best misprediction rate per m (min over n)",
          "m — PC index bits", "Best misprediction rate over n (%)")
    fig.tight_layout()
    fig.savefig(out, format="svg")
    plt.close(fig)


def write_csvs(g1, g2, outdir):
    with open(os.path.join(outdir, "graph1-data.csv"), "w", newline="") as f:
        w = csv.writer(f)
        w.writerow(["m", "misprediction_rate_gcc", "misprediction_rate_jpeg",
                    "misprediction_rate_perl"])
        for m in sorted(g1):
            w.writerow([m, f"{g1[m]['gcc']:.2f}%", f"{g1[m]['jpeg']:.2f}%",
                        f"{g1[m]['perl']:.2f}%"])
    with open(os.path.join(outdir, "graph2-data.csv"), "w", newline="") as f:
        w = csv.writer(f)
        w.writerow(["m", "n", "misprediction_rate_gcc", "misprediction_rate_jpeg",
                    "misprediction_rate_perl"])
        for (m, n) in sorted(g2):
            w.writerow([m, n, f"{g2[(m, n)]['gcc']:.2f}%",
                        f"{g2[(m, n)]['jpeg']:.2f}%", f"{g2[(m, n)]['perl']:.2f}%"])


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--repo", default=".", help="repo checkout root")
    ap.add_argument("--out", default="docs", help="output directory for SVGs/CSVs")
    args = ap.parse_args()
    os.makedirs(args.out, exist_ok=True)
    g1 = load_graph1(args.repo)
    g2 = load_graph2(args.repo)
    chart_bimodal(g1, os.path.join(args.out, "graph1-bimodal.svg"))
    chart_gshare_n(g2, os.path.join(args.out, "graph2-gshare-n.svg"))
    chart_gshare_best(g2, os.path.join(args.out, "graph3-gshare-best.svg"))
    write_csvs(g1, g2, args.out)
    print(f"wrote 3 charts + 2 corrected CSVs to {args.out}/")
    # sanity: report design points
    print(f"bimodal m=14: gcc {g1[14]['gcc']:.2f}% jpeg {g1[14]['jpeg']:.2f}% "
          f"perl {g1[14]['perl']:.2f}%")
    print(f"gshare m=15 n=9: gcc {g2[(15, 9)]['gcc']:.2f}% "
          f"jpeg {g2[(15, 9)]['jpeg']:.2f}% perl {g2[(15, 9)]['perl']:.2f}%")


if __name__ == "__main__":
    sys.exit(main())
