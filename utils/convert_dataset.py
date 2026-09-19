# idea --> convert a dataset into a .bin (and output it's checsum so we can check the transfer worked without loss)
# This .bin can then be like baked into the image. 

import numpy as np
from pathlib import Path

data_folder = Path(__file__).parent.parent / "data"
VECS = 0x53434556 #literally spells VECS, so it's easy to recognize
FNV_OFFSET_BASIS_32 = 2166136261
FNV_PRIME_32 = 16777619

def checksum_fnv1a_32(data):
    hash = FNV_OFFSET_BASIS_32
    for byte in data:
        hash ^= byte
        hash = (hash * FNV_PRIME_32) & 0xFFFFFFFF
    return hash

# Bin output format: (little endian)
# ["VECS"][n][dim][0] then the data (n*dim uint8 values)
def fvecs_to_bin(fvecs_path, bin_path):
    fv = np.fromfile(fvecs_path, dtype=np.float32)
    if fv.size == 0:
        raise ValueError(f"error importing from {fvecs_path}")
    dim = fv.view(np.int32)[0]
    fv = fv.reshape(-1, 1 + dim)
    assert np.all(fv[:, 0].view(np.int32) == dim)

    vectors = fv[:, 1:]
    n = vectors.shape[0]

    assert np.all(vectors == np.round(vectors))
    assert vectors.min() >= 0 and vectors.max() <= 255

    data = vectors.astype(np.uint8) # right now I dont have support for floats, so the numbers need to be ints
    header = np.array([VECS, n, dim, 0], dtype='<u4')
    with open(bin_path, "wb") as f:
        f.write(header.tobytes())
        f.write(data.tobytes())

    print(f"Converted {vectors.shape[0]} vectors of dim {vectors.shape[1]} to {bin_path}")
    print(f"CHECKSUM: 0x{checksum_fnv1a_32(data.tobytes()):08x}")


fvecs_to_bin(data_folder/"siftsmall/siftsmall_base.fvecs", data_folder/"data.bin")