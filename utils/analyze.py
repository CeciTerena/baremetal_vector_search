import sys
import math
import statistics
from pathlib import Path
from compare import BEGIN_RE, PARAM_RE

LOG_FOLDER = Path(__file__).parent.parent / "logs"
TAG_INTERRUPTS_BEFORE = "INTERRUPTS_BEFORE "
TAG_INTERRUPTS_AFTER = "INTERRUPTS_AFTER "

def resolve_path(arg):
    p = Path(arg)
    return p if p.exists() else LOG_FOLDER / arg

def is_run_log(path):
    return path.suffix == ".log" and not path.name.endswith(".boot.log")

def logs_in_folder(folder):
    return sorted(p for p in folder.glob("*.log") if is_run_log(p))

def collect_logs(args):
    paths = []
    for arg in args:
        p = resolve_path(arg)
        paths += logs_in_folder(p) if p.is_dir() else [p]
    return paths

def platform_of(begin_match):
    params = dict(PARAM_RE.findall(begin_match.group("params")))
    return params.get("platform", "unknown")

def info_pair(line):
    key, value = line[2:].strip().split("=", 1)
    return key, value

def batch_ticks(line):
    return int(line.split()[2])

def query_ticks(line):
    return int(line.split()[3])

def empty_log(path):
    return {"name": path.name, "platform": "unknown", "info": {},
            "batch": [], "query": [], "interrupts_before": [], "interrupts_after": []}

def parse_log(path):
    log = empty_log(path)
    with open(path) as f:
        for line in f:
            if m := BEGIN_RE.match(line):
                log["platform"] = platform_of(m)
            elif line.startswith("I ") and "=" in line:
                key, value = info_pair(line)
                log["info"][key] = value
            elif line.startswith("B "):
                log["batch"].append(batch_ticks(line))
            elif line.startswith("T "):
                log["query"].append(query_ticks(line))
            elif line.startswith(TAG_INTERRUPTS_BEFORE):
                log["interrupts_before"].append(line.removeprefix(TAG_INTERRUPTS_BEFORE))
            elif line.startswith(TAG_INTERRUPTS_AFTER):
                log["interrupts_after"].append(line.removeprefix(TAG_INTERRUPTS_AFTER))
    return log

def log_time(name):
    date, time = name.removesuffix(".log").split("_")[-2:]
    if len(date) == 8:
        date, time = f"{date[4:6]}-{date[6:]}", f"{time[:2]}:{time[2:4]}:{time[4:]}"
    return f"{date}_{time}"

def has_results(log):
    return bool(log["batch"]) and bool(log["query"])

def timer_hz(log):
    return int(log["info"].get("timer_freq", 54_000_000))

def to_ms(ticks, hz):
    return ticks / hz * 1e3

def to_us(ticks, hz):
    return ticks / hz * 1e6

def percentile(values, p):
    ordered = sorted(values)
    rank = math.ceil(p / 100 * len(ordered)) - 1
    return ordered[max(rank, 0)]

def spread_percent(values):
    return (max(values) - min(values)) / min(values) * 100

def info_int(log, key):
    value = log["info"].get(key, "")
    return int(value) if value.isdigit() else None

def core_of(log):
    start, end = info_int(log, "cpu_start"), info_int(log, "cpu_end")
    if start is None:
        return None
    return str(start) if start == end else f"{start}->{end}"

def stayed_on_core(log):
    start = info_int(log, "cpu_start")
    return start if start is not None and start == info_int(log, "cpu_end") else None

def parse_interrupts(lines):
    n_cpus = len(lines[0].split())
    table = {}
    for line in lines[1:]:
        parts = line.split()
        counts = [int(x) for x in parts[1:1 + n_cpus] if x.isdigit()]
        table[parts[0].rstrip(":")] = (counts, " ".join(parts[1 + len(counts):]))
    return table

def interrupt_deltas(log):
    if not log["interrupts_before"]:
        return []
    before = parse_interrupts(log["interrupts_before"])
    after = parse_interrupts(log["interrupts_after"])
    deltas = []
    for name, (counts, description) in after.items():
        old = before.get(name, ([0] * len(counts), ""))[0]
        delta = [a - b for a, b in zip(counts, old)]
        if any(delta):
            deltas.append((name, delta, description))
    return deltas

def interrupts_on_core(log):
    core = stayed_on_core(log)
    if core is None:
        return None
    return sum(delta[core] for _, delta, _ in interrupt_deltas(log) if core < len(delta))

def page_faults(log):
    minor, major = info_int(log, "page_faults_minor"), info_int(log, "page_faults_major")
    return None if minor is None else minor + major

def ctx_switches(log):
    voluntary, forced = info_int(log, "ctx_switch_voluntary"), info_int(log, "ctx_switch_forced")
    return None if forced is None else voluntary + forced

def row_of(log):
    hz = timer_hz(log)
    cpu_hz = info_int(log, "cpu_hz")
    return {
        "log_time":           log_time(log["name"]),
        "platform":       log["platform"],
        "cpu_mhz":        cpu_hz / 1e6 if cpu_hz else None,
        "timer_overhead": info_int(log, "timer_overhead_ticks"),
        "batch_median_ms":       to_ms(statistics.median(log["batch"]), hz),
        "batch_spread":   spread_percent(log["batch"]),
        "query_median_us":       to_us(statistics.median(log["query"]), hz),
        "query_p99_us":         to_us(percentile(log["query"], 99), hz),
        "query_max_us":         to_us(max(log["query"]), hz),
        "ctx_switches":   ctx_switches(log),
        "interrupts":     interrupts_on_core(log),
        "page_faults":    page_faults(log),
        "cpu_core":           core_of(log),
    }

COLUMNS = [
    ("platform", 18, "{}"),        ("log_time", 14, "{}"),
    ("batch_median_ms", 9, "{:.3f}"),     ("batch_spread", 13, "{:.2f}%"),
    ("query_median_us", 9, "{:.1f}"),     ("query_p99_us", 9, "{:.1f}"),
    ("query_max_us", 9, "{:.1f}"),       ("ctx_switches", 13, "{}"),
    ("interrupts", 11, "{}"),      ("page_faults", 12, "{}"),
    ("cpu_core", 5, "{}"),
]

def has_pmu(log):
    return "pmu_cycles" in log["info"]

def ratio(a, b):
    return None if a is None or not b else a / b

def millions(value):
    return None if value is None else value / 1e6

def cycles_check(log, cycles):
    cpu_hz = info_int(log, "cpu_hz")
    if cycles is None or not cpu_hz:
        return None
    expected = sum(log["batch"]) * cpu_hz / timer_hz(log)
    return (cycles - expected) / expected * 100

def overflow_text(log):
    flags = info_int(log, "pmu_overflow")
    return None if not flags else hex(flags)

def pmu_row_of(log):
    cycles = info_int(log, "pmu_cycles")
    inst = info_int(log, "pmu_inst_retired")
    l1d_miss = ratio(info_int(log, "pmu_l1d_cache_refill"), info_int(log, "pmu_l1d_cache"))
    return {
        "log_time":         log_time(log["name"]),
        "platform":     log["platform"],
        "inst_per_cycle":          ratio(inst, cycles),
        "inst_M":       millions(inst),
        "except_taken":    info_int(log, "pmu_exc_taken"),
        "l1d_tlb_refills":   info_int(log, "pmu_l1d_tlb_refill"),
        "l1d_miss_rate":     None if l1d_miss is None else l1d_miss * 100,
        "l2_refills_M":  millions(info_int(log, "pmu_l2d_cache_refill")),
        "cycles_vs_timer": cycles_check(log, cycles),
        "ctr_overflow":     overflow_text(log),
    }

PMU_COLUMNS = [
    ("platform", 18, "{}"),        ("log_time", 14, "{}"),
    ("inst_per_cycle", 6, "{:.3f}"),          ("inst_M", 10, "{:.1f}"),
    ("except_taken", 10, "{}"),       ("l1d_tlb_refills", 11, "{}"),
    ("l1d_miss_rate", 9, "{:.2f}%"),    ("l2_refills_M", 12, "{:.2f}"),
    ("cycles_vs_timer", 13, "{:+.3f}%"), ("ctr_overflow", 11, "{}"),
]

EXPECTED_CPU_MHZ = 1500
CPU_TOLERANCE = 0.001

def cell(value, width, fmt):
    text = "-" if value is None else fmt.format(value)
    return text.rjust(width)

def print_table(rows, columns):
    columns = [(name, max(width, len(name)), fmt) for name, width, fmt in columns]
    print(" ".join(name.rjust(width) for name, width, _ in columns))
    for row in rows:
        print(" ".join(cell(row[name], width, fmt) for name, width, fmt in columns))

def print_pmu_table(logs):
    rows = sorted((pmu_row_of(log) for log in logs if has_pmu(log)), key=lambda r: (r["platform"], r["log_time"]))
    if rows:
        print("\nhardware counters (batch window)")
        print_table(rows, PMU_COLUMNS)

def group_by_platform(rows):
    groups = {}
    for row in rows:
        groups.setdefault(row["platform"], []).append(row)
    return groups

def timer_overhead_text(group):
    values = {r["timer_overhead"] for r in group}
    if None in values:
        return "timer overhead unknown"
    if len(values) == 1:
        return f"timer overhead {values.pop()} tick(s)"
    return f"timer overhead varies {min(values)}-{max(values)} ticks"

def cpu_ok(mhz):
    return mhz is not None and abs(mhz - EXPECTED_CPU_MHZ) <= EXPECTED_CPU_MHZ * CPU_TOLERANCE

def cpu_text(group):
    bad = [r for r in group if not cpu_ok(r["cpu_mhz"])]
    if not bad:
        return f"cpu {EXPECTED_CPU_MHZ} MHz ok"
    return "cpu CHECK: " + ", ".join(f"{r['log_time']}={mhz_text(r['cpu_mhz'])}" for r in bad)

def mhz_text(mhz):
    return "missing" if mhz is None else f"{mhz:.1f} MHz"

def print_summaries(rows):
    print()
    for platform, group in group_by_platform(rows).items():
        medians = [r["batch_median_ms"] for r in group]
        print(f"{platform}: {len(group)} boot(s), batch {statistics.median(medians):.3f} ms, "
              f"boot-to-boot spread {spread_percent(medians):.2f}%, "
              f"{timer_overhead_text(group)}, {cpu_text(group)}")

def cpu_header(n_cpus, core):
    names = [f"CPU{c}" + ("*" if c == core else "") for c in range(n_cpus)]
    return "  " + "source".rjust(6) + " " + " ".join(n.rjust(7) for n in names)

def print_interrupt_table(log):
    deltas = interrupt_deltas(log)
    n_cpus = len(log["interrupts_after"][0].split())
    print(cpu_header(n_cpus, stayed_on_core(log)))
    for name, delta, description in deltas:
        print(f"  {name:>6} " + " ".join(f"{d:>7}" for d in delta) + f"  {description}")

def print_interrupts(logs):
    for log in sorted(logs, key=lambda l: log_time(l["name"])):
        if log["interrupts_before"]:
            print(f"\n{log['platform']} {log_time(log['name'])}: interrupts during the run (* = benchmark's core)")
            print_interrupt_table(log)

def main():
    args = [a for a in sys.argv[1:] if a != "--interrupts"]
    logs = [parse_log(p) for p in collect_logs(args)]
    logs = [log for log in logs if has_results(log)]
    rows = sorted((row_of(log) for log in logs), key=lambda r: (r["platform"], r["log_time"]))
    print_table(rows, COLUMNS)
    print_summaries(rows)
    print_pmu_table(logs)
    if "--interrupts" in sys.argv:
        print_interrupts(logs)

if __name__ == "__main__":
    main()