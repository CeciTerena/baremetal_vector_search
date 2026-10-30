# idea --> convert a dataset into a .bin (and output it's checsum so we can check the transfer worked without loss)

import numpy as np
from pathlib import Path

data_folder = Path(__file__).parent.parent / "data"
VECS = 0x53434556 #literally spells VECS, so it's easy to recognize
QURY = 0x59525551 # QURY  ()
CENT = 0x544E4543 # CENT (centroids)
OFFS = 0x5346464F # OFFS (where each list starts)
LIDS = 0x5344494C # LIDS (id of each reordered vector). L for list.
IVEC = 0x43455649 # IVEC (vectors reordered by list)

FNV_OFFSET_BASIS_32 = 2166136261
FNV_PRIME_32 = 16777619

NLIST = 100
KMEANS_SEED = 42
KMEANS_MAX_ITER = 25

def checksum_fnv1a_32(data):
    hash = FNV_OFFSET_BASIS_32
    for byte in data:
        hash ^= byte
        hash = (hash * FNV_PRIME_32) & 0xFFFFFFFF
    return hash

def squared_distances(data, point): 
    differences = data - point
    squared = differences ** 2
    return squared.sum(axis=1)

def nearest_centroid(data, centroids):
    distances = np.empty((len(data), len(centroids)), dtype=np.float32)
    for index, centroid in enumerate(centroids):
        distances[:, index] = squared_distances(data, centroid)
    return distances.argmin(axis=1)

def pick_weighted(rng, weights):
    weights = weights.astype(np.float64)
    probabilities = weights / weights.sum()
    return rng.choice(len(weights), p = probabilities)

def kmeans_plus_plus(data, k, rng):
    first = rng.integers(len(data))
    centroids = [data[first]]
    min_dist = squared_distances(data, data[first])
    for _ in range(1, k):
        chosen = pick_weighted(rng, min_dist)
        centroids.append(data[chosen])
        dist_to_new = squared_distances(data, data[chosen])
        min_dist = np.minimum(min_dist, dist_to_new)
    return np.array(centroids)

def recompute(data, assignment, centroids):
    new_centroids = centroids.copy()
    for c in range(len(centroids)):
        members = data[assignment == c]
        if len(members) > 0:
            new_centroids[c] = members.mean(axis=0)
    return new_centroids

def train_kmeans(data, k):
    rng = np.random.default_rng(KMEANS_SEED)
    centroids = kmeans_plus_plus(data, k, rng)
    for _ in range(KMEANS_MAX_ITER):
        assignment = nearest_centroid(data, centroids)
        new_centroids = recompute(data, assignment, centroids)
        if np.array_equal(new_centroids, centroids):
            break
        centroids = new_centroids
    return centroids

# -------------------------------------

def read_fvecs(path):
    fv = np.fromfile(path, dtype=np.float32)
    if fv.size == 0:
        raise ValueError(f"error importing from {path}")
    dim = fv.view(np.int32)[0]
    rows = fv.reshape(-1, 1 + dim)
    assert np.all(rows[:, 0].view(np.int32) == dim)
    return rows[:, 1:]

def make_block(magic, array):
    rows = array.reshape(len(array), -1)
    n, dim = rows.shape
    header = np.array([magic, n, dim, 0], dtype = '<u4')
    data = rows.tobytes()
    block = header.tobytes() + data
    padded_length = ((len(block) + 15) // 16) * 16
    padded_block = block.ljust(padded_length, b'\x00')
    return padded_block, checksum_fnv1a_32(data)

def ivf_lists(base, centroids):
    assignments = nearest_centroid(base, centroids)
    order = np.argsort(assignments, kind="stable")
    list_sizes = np.bincount(assignments, minlength=len(centroids))
    offsets = np.zeros(len(centroids) + 1, dtype = '<u4')
    offsets[1:] = np.cumsum(list_sizes)
    ids = order.astype('<u4')
    reordered = base[order]
    return offsets, ids, reordered

def magic_name(magic):
    return magic.to_bytes(4, "little").decode()

def to_bin(base_path, query_path, bin_path, learn_path):
    base = read_fvecs(base_path)
    queries = read_fvecs(query_path)
    learn = read_fvecs(learn_path)
    if base.shape[1] != queries.shape[1]:
        raise ValueError(f"vector dimensions: {base.shape[1]} and query dimensions: {queries.shape[1]} mismatch")

    centroids = train_kmeans(learn, NLIST)
    offsets, ids, reordered = ivf_lists(base, centroids)
    blocks = [(VECS, base), (QURY, queries), (CENT, centroids), (OFFS, offsets), (LIDS, ids), (IVEC, reordered)]
    
    data = b""
    for magic, array in blocks: 
        block, checksum = make_block(magic, array)
        data += block
        print(f"{magic_name(magic)} CHECKSUM: 0x{checksum:08x}")

    with open(bin_path, "wb") as f:
        f.write(data)

to_bin(data_folder/"siftsmall/siftsmall_base.fvecs", data_folder/"siftsmall/siftsmall_query.fvecs", data_folder/"data.bin", data_folder/"siftsmall/siftsmall_learn.fvecs")