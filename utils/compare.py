import re
import sys
import numpy as np
from pathlib import Path

DATA_FOLDER = Path(__file__).parent.parent / "data"

BEGIN_RE = re.compile(r'^BEGIN\s+(?P<algo>[A-Za-z0-9_-]+)(?P<params>(?:\s+\w+=\S+)*)\s*$')
PARAM_RE = re.compile(r'(\w+)=(\S+)')
RESULT_RE = re.compile(r'^R\s+(?P<q>\d+)(?P<ids>(?:\s+\d+)+)\s*$')
END_RE = re.compile(r'^END\s*$')

# --------------------------------

def compare_brute(ground_truth, results, params):
    mismatches = 0
    K = int(params["K"])
    assert len(results) == int(params["NQ"]), f"Num Queries mismatch --> got {len(results)} results, expected {int(params["NQ"])}"
    for q, result_ids in results.items():
        assert len(result_ids) == K, f"K mismatch --> got {len(result_ids)} results, expected {K}"
        expected = ground_truth[q][:K].tolist()
        if result_ids != expected:
            mismatches += 1
            print(f"Mismatch for query: {q} --> Ground Truth {expected} | Results {result_ids}")
    print(f"Total Mismatches: {mismatches}")

def load_ivecs(path):
    raw = np.fromfile(path, dtype=np.int32)
    k = raw[0]
    raw = raw.reshape(-1, 1 + k)
    assert np.all(raw[:, 0] == k)
    return raw[:, 1:]

# --------------------------------

# Im gonna try to build this modularly, so that datasets can be added, loaded and parsed acordingly!
DATASETS = {"siftsmall": DATA_FOLDER / "siftsmall/siftsmall_groundtruth.ivecs"}
ALGORITHMS = {"brute": compare_brute}

def main():
    LOG_FOLDER = Path(__file__).parent.parent / "logs"
    log_file = LOG_FOLDER / sys.argv[1]

    algo = None
    params = {}
    results = {}

    with open(log_file) as f:
        for line in f:
            if m := BEGIN_RE.match(line):
                algo = m.group("algo")
                params = dict(PARAM_RE.findall(m.group("params")))
            elif m := RESULT_RE.match(line):
                q = int(m.group("q"))
                results[q] = [int(x) for x in m.group("ids").split()]
            elif END_RE.match(line):
                break

    if algo not in ALGORITHMS: 
        raise ValueError(f"Algorithm {algo!r} not supported") 
    
    ground_truth = load_ivecs(DATASETS[params["dataset"]])
    ALGORITHMS[algo](ground_truth, results, params)

if __name__ == "__main__":
    main()