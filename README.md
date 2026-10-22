# 🔍 LogLens

> Put a lens on your logs. A fast C++17 command-line tool that turns raw
> web server logs into clear, actionable insights.

LogLens streams Apache/Nginx-style access logs line by line, so it handles
large files without loading them into memory. It summarizes traffic,
highlights the busiest clients and endpoints, and flags suspicious behavior
such as bot or brute-force activity.

## ✨ Features

- **Traffic summary**: total requests, bytes served, status code breakdown
- **Top-N reports**: busiest IPs and most requested URLs, with share of traffic
- **Hourly traffic histogram**: spot peak hours at a glance
- **Anomaly detection**: a 60-second sliding window flags IPs with abnormal request rates
- **Filters**: by status code, IP address and date range
- **Report export**: save results as text or CSV
- **Fast**: hand-written `string_view` parser, benchmarked on 2 million lines
- **Tested**: GoogleTest unit tests for the parser, analyzer and anomaly detector

## 🚀 Build

Requires a C++17 compiler (GCC 9+, Clang, or MSVC) and CMake 3.14+.

```bash
git clone https://github.com/<your-username>/loglens.git
cd loglens
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

Generate sample data (needs Python 3):

```bash
python scripts/gen_logs.py 100000 sample_logs/access.log
```

## 🧰 Usage

```bash
# Basic summary
loglens sample_logs/access.log

# Top 5 only
loglens sample_logs/access.log --top 5

# Only 404 errors
loglens sample_logs/access.log --status 404

# One client's traffic
loglens sample_logs/access.log --ip 203.0.113.7

# Stricter anomaly threshold
loglens sample_logs/access.log --threshold 50

# Date range, then export
loglens sample_logs/access.log --from 2026-09-30 --to 2026-09-30 \
    --out reports/report.txt --csv reports/report.csv
```

| Option | Meaning |
|---|---|
| `--top N` | show the top N IPs and URLs (default 10) |
| `--status CODE` | only count requests with this status code |
| `--ip ADDRESS` | only count requests from this IP |
| `--from DATE` / `--to DATE` | date range, `YYYY-MM-DD` |
| `--threshold N` | anomaly threshold: requests per 60 s (default 100) |
| `--out FILE` | also save the text report |
| `--csv FILE` | also save a CSV report |
| `-h`, `--help` | show help |

## 📊 Example output

```
=== LogLens Report ===
Total lines : 100321
Parsed      : 100300
Malformed   : 21
Matched     : 100300  (after filters)

Top IPs
    #  IP               Requests   Share
  ------------------------------------------
    1  192.168.1.xx          ...    0.8%

Traffic by hour
  14:00 | ######################################## 7712

Anomaly detection (more than 100 requests in 60s)
  ! 203.0.113.7  peak 29x requests/60s, first flagged at 2026-09-30 14:30:xx (+0530)
```

## ⚡ Performance

Measured on 2,000,000 log lines (about 200 MB), Release build, GCC 16.2, Windows 11.

| Parser | Time | Throughput |
|---|---|---|
| `std::regex` (first version) | ___ s | ___ M lines/s |
| `string_view` + `std::from_chars` | ___ s | ___ M lines/s |

Speedup: **___x**. The regex version builds many temporary strings per line;
the hand-written parser slices the line with `string_view` and converts numbers
with `from_chars`, so it avoids those allocations.

## 🗂 Project structure

```
include/loglens/   public headers
src/               implementation
tests/             GoogleTest unit tests
scripts/           fake log generator
docs/              benchmark notes
```

| Component | Job |
|---|---|
| `Parser` | one log line to a `LogEntry` |
| `Timestamp` | date text to epoch seconds, no locale or `mktime` |
| `Reader` | streams the file and feeds entries to a callback |
| `Analyzer` | counts, top-N, hourly buckets |
| `AnomalyDetector` | per-IP sliding window |
| `Report` | text and CSV output to any `std::ostream` |
| `Options` | command-line parsing |

## 🧪 Tests

```bash
cmake -B build -G Ninja -DBUILD_TESTS=ON
cmake --build build
ctest --test-dir build --output-on-failure
```

## ⚠️ Limitations

- Expects entries in chronological order (needed for the sliding window)
- The log timezone is assumed to be `+0530` for `--from`/`--to` and report times
- Supports the Common Log Format variant produced by `scripts/gen_logs.py`

## 🛣 Roadmap

- [ ] Multi-threaded parsing
- [ ] Live "tail" mode
- [ ] JSON output
- [ ] Configurable timezone

