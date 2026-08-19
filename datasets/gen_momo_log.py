#!/usr/bin/env python3
"""
gen_momo_log.py — seeded synthetic MoMo (mobile-money) transaction log
generator for the momo_match kernel (Project 1: Roofline Reckoning).

Owner: Obed Ninson (Python/MATLAB + Test/Docs Lead)

Produces two parallel logs of `n` transactions each ("sent" and
"received"). A fraction `match_rate` of sent transactions have a truly
matching received transaction (same id, same amount); the rest are
deliberately unmatched (id present in `sent` but not in `received`, or
amount differs beyond tolerance) to give momo_match's hash-probe path a
realistic branch mix, not a trivial always-match case.

Output CSV columns (no header row consumed by gh_bench, but written for
human readability): sent_id,sent_amount,sent_ts,recv_id,recv_amount,recv_ts

Usage:
    python gen_momo_log.py --n 20000 --seed 30153 --match-rate 0.85 \
        --out ../datasets/momo_log.csv
"""
import argparse
import csv
import random


def generate(n: int, seed: int, match_rate: float):
    rng = random.Random(seed)
    rows = []
    next_id = 1000000
    for _ in range(n):
        sid = next_id
        next_id += 1
        samt = round(rng.uniform(1.0, 5000.0), 2)
        sts = rng.randint(1_700_000_000, 1_700_086_400)

        if rng.random() < match_rate:
            # True match: same id, same amount, received a few seconds later.
            rid = sid
            ramt = samt
            rts = sts + rng.randint(1, 120)
        else:
            # Deliberate mismatch: either a different id (never settled) or
            # the same id with a corrupted amount (partial settlement / fee
            # mismatch scenario), chosen 50/50.
            if rng.random() < 0.5:
                rid = next_id
                next_id += 1
                ramt = round(rng.uniform(1.0, 5000.0), 2)
            else:
                rid = sid
                ramt = round(samt + rng.uniform(0.5, 50.0), 2)
            rts = sts + rng.randint(1, 300)

        rows.append((sid, samt, sts, rid, ramt, rts))
    return rows


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--n", type=int, default=20000, help="transactions per log")
    ap.add_argument("--seed", type=int, required=True)
    ap.add_argument("--match-rate", type=float, default=0.85)
    ap.add_argument("--out", type=str, required=True)
    args = ap.parse_args()
    if not 0.0 <= args.match_rate <= 1.0:
        ap.error("--match-rate must be between 0.0 and 1.0")
    if args.n <= 0:
        ap.error("--n must be a positive integer")

    rows = generate(args.n, args.seed, args.match_rate)
    with open(args.out, "w", newline="") as f:
        w = csv.writer(f)
        w.writerow(["sent_id", "sent_amount", "sent_ts", "recv_id", "recv_amount", "recv_ts"])
        w.writerows(rows)
    print(f"wrote {len(rows)} transaction pairs to {args.out} (seed={args.seed})")


if __name__ == "__main__":
    main()
