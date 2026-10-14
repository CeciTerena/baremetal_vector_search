# idea --> convert a dataset into a .bin (and output it's checsum so we can check the transfer worked without loss)

import numpy as np
from pathlib import Path

data_folder = Path(__file__).parent.parent / "data"
VECS = 0x53434556 #literally spells VECS, so it's easy to recognize
QURY = 0x59525551 #spells QURY
FNV_OFFSET_BASIS_32 = 2166136261
FNV_PRIME_32 = 16777619

def checksum_fnv1a_32(data):
    hash = FNV_OFFSET_BASIS_32
    for byte in data:
        hash ^= byte
        hash = (hash * FNV_PRIME_32) & 0xFFFFFFFF
    return hash

# by block i mean the vectors block, the query block, the ground truth block....
def block_convert(path, magic):
    fv = np.fromfile(path, dtype=np.float32)
    if fv.size == 0:
        raise ValueError(f"error importing from {path}")
    dim = fv.view(np.int32)[0]
    fv = fv.reshape(-1, 1 + dim)
    assert np.all(fv[:, 0].view(np.int32) == dim)

    vectors = fv[:, 1:]
    n = vectors.shape[0]

    data = vectors.astype('<f4')
    header = np.array([magic, n, dim, 0], dtype='<u4')

    block = header.tobytes() + data.tobytes()
    # pad (align) to multiple of 16
    padded_length = ((len(block) + 15) // 16) * 16 
    padded_data = block.ljust(padded_length, b'\x00')

    checksum = checksum_fnv1a_32(data.tobytes())

    return padded_data, dim, checksum

# Bin output format (little endian)
def fvecs_to_bin(fvecs_path, query_path, bin_path):
    vector_block, vec_dims, vec_checksum = block_convert(fvecs_path, VECS)
    query_block, query_dims, query_checksum = block_convert(query_path, QURY)
    if vec_dims != query_dims: 
        raise ValueError(f"vector dimensions: {vec_dims} and query dimentions: {query_dims} mismatch")
    
    data = vector_block + query_block
    with open(bin_path, "wb") as f:
        f.write(data)

    print(f"VEC CHECKSUM: 0x{vec_checksum:08x}")
    print(f"QUERY CHECKSUM: 0x{query_checksum:08x}")



fvecs_to_bin(data_folder/"siftsmall/siftsmall_base.fvecs", data_folder/"siftsmall/siftsmall_query.fvecs", data_folder/"data.bin")