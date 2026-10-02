#!/usr/bin/env python3
"""Generate fake Apache-style access logs.
Usage: python scripts/gen_logs.py [num_lines] [output_file]
"""
import random
import sys
from datetime import datetime, timedelta

NUM_LINES = int(sys.argv[1]) if len(sys.argv) > 1 else 100_000
OUT_FILE = sys.argv[2] if len(sys.argv) > 2 else "sample_logs/access.log"

random.seed(42)  # same output every run

IPS = [f"192.168.1.{i}" for i in range(1, 60)] + \
      [f"10.0.0.{i}" for i in range(1, 30)] + \
      [f"172.16.{random.randint(0, 255)}.{random.randint(1, 254)}" for _ in range(40)]

URLS = ["/", "/index.html", "/about", "/login", "/api/users", "/api/orders",
        "/products", "/products/42", "/static/app.js", "/static/style.css",
        "/images/logo.png", "/contact", "/admin", "/search?q=cpp"]

METHODS = ["GET"] * 8 + ["POST"] * 2 + ["PUT", "DELETE"]

# mostly 200, some redirects, client errors, server errors
STATUSES = [200] * 85 + [301, 302, 304] * 2 + [404] * 6 + [403, 401] + [500] * 2 + [503]

ATTACKER = "203.0.113.7"  # used for anomaly detection on Day 6

start = datetime(2026, 9, 30, 0, 0, 0)
lines = []

for _ in range(NUM_LINES):
    t = start + timedelta(seconds=random.randint(0, 86_399))
    # more traffic during the daytime
    if random.random() < 0.6:
        t = t.replace(hour=random.randint(9, 18))
    lines.append((t, random.choice(IPS), random.choice(METHODS),
                  random.choice(URLS), random.choice(STATUSES),
                  random.randint(200, 50_000)))

# Inject an "attacker": 300 requests to /login within one minute
burst_start = start + timedelta(hours=14, minutes=30)
for i in range(300):
    t = burst_start + timedelta(seconds=random.randint(0, 59))
    lines.append((t, ATTACKER, "POST", "/login", random.choice([401, 401, 401, 200]), 512))

lines.sort(key=lambda x: x[0])

# Write the file, sprinkling in a few malformed lines to test the parser
with open(OUT_FILE, "w") as f:
    for idx, (t, ip, method, url, status, size) in enumerate(lines):
        ts = t.strftime("%d/%b/%Y:%H:%M:%S +0530")
        f.write(f'{ip} - - [{ts}] "{method} {url} HTTP/1.1" {status} {size}\n')
        if idx % 5000 == 0:
            f.write("this is a malformed line\n")

print(f"Wrote {len(lines)} log lines to {OUT_FILE}")