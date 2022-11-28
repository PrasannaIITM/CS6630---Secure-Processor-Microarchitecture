import time
def XTIME(a,n=1):
    for i in range(n):
        if a & 0x80:
            a = a << 1
            a ^= 0x1B
        else:
            a = a << 1
    return a & 0xFF
    
def GETBYTE(w,n):
    return ((w >> (24-(8*n))) & 0xff)

def bytes_to_word(a):
    return (a[0] << 24 | a[1] << 16 | a[2] << 8 | a[3])

invsbox = [
  0x52,0x09,0x6a,0xd5,0x30,0x36,0xa5,0x38,0xbf,0x40,0xa3,0x9e,0x81,0xf3,0xd7,0xfb,
  0x7c,0xe3,0x39,0x82,0x9b,0x2f,0xff,0x87,0x34,0x8e,0x43,0x44,0xc4,0xde,0xe9,0xcb,
  0x54,0x7b,0x94,0x32,0xa6,0xc2,0x23,0x3d,0xee,0x4c,0x95,0x0b,0x42,0xfa,0xc3,0x4e,
  0x08,0x2e,0xa1,0x66,0x28,0xd9,0x24,0xb2,0x76,0x5b,0xa2,0x49,0x6d,0x8b,0xd1,0x25,
  0x72,0xf8,0xf6,0x64,0x86,0x68,0x98,0x16,0xd4,0xa4,0x5c,0xcc,0x5d,0x65,0xb6,0x92,
  0x6c,0x70,0x48,0x50,0xfd,0xed,0xb9,0xda,0x5e,0x15,0x46,0x57,0xa7,0x8d,0x9d,0x84,
  0x90,0xd8,0xab,0x00,0x8c,0xbc,0xd3,0x0a,0xf7,0xe4,0x58,0x05,0xb8,0xb3,0x45,0x06,
  0xd0,0x2c,0x1e,0x8f,0xca,0x3f,0x0f,0x02,0xc1,0xaf,0xbd,0x03,0x01,0x13,0x8a,0x6b,
  0x3a,0x91,0x11,0x41,0x4f,0x67,0xdc,0xea,0x97,0xf2,0xcf,0xce,0xf0,0xb4,0xe6,0x73,
  0x96,0xac,0x74,0x22,0xe7,0xad,0x35,0x85,0xe2,0xf9,0x37,0xe8,0x1c,0x75,0xdf,0x6e,
  0x47,0xf1,0x1a,0x71,0x1d,0x29,0xc5,0x89,0x6f,0xb7,0x62,0x0e,0xaa,0x18,0xbe,0x1b,
  0xfc,0x56,0x3e,0x4b,0xc6,0xd2,0x79,0x20,0x9a,0xdb,0xc0,0xfe,0x78,0xcd,0x5a,0xf4,
  0x1f,0xdd,0xa8,0x33,0x88,0x07,0xc7,0x31,0xb1,0x12,0x10,0x59,0x27,0x80,0xec,0x5f,
  0x60,0x51,0x7f,0xa9,0x19,0xb5,0x4a,0x0d,0x2d,0xe5,0x7a,0x9f,0x93,0xc9,0x9c,0xef,
  0xa0,0xe0,0x3b,0x4d,0xae,0x2a,0xf5,0xb0,0xc8,0xeb,0xbb,0x3c,0x83,0x53,0x99,0x61,
  0x17,0x2b,0x04,0x7e,0xba,0x77,0xd6,0x26,0xe1,0x69,0x14,0x63,0x55,0x21,0x0c,0x7d
]

rcon = [1, 2, 4, 8, 16, 32, 64, 128, 27, 54]

POSITIONS = [[0, 13, 10, 7],[4, 1, 14, 11],[8, 5, 2, 15],[12, 9, 6, 3]]

def mix_column(a):
    t = a[0] ^ a[1] ^ a[2] ^ a[3]
    u = a[0]
    
    v = a[0] ^ a[1]
    v = XTIME(v)
    a[0] = a[0] ^ v ^ t
    
    v = a[1] ^ a[2]
    v = XTIME(v)
    a[1] = a[1] ^ v ^ t
    
    v = a[2] ^ a[3]
    v = XTIME(v)
    a[2] = a[2] ^ v ^ t
    
    v = a[3] ^ u
    v = XTIME(v)
    a[3] = a[3] ^ v ^ t

    return a
    
def get_diff_after_MC():
    fault_list = [i for i in range(1, 256)]
    list_diff = []
    fault_len = len(fault_list)

    for pos in range(4):
        for i in range(fault_len):
            col = [0 for _ in range(4)]
            col[pos] = fault_list[i]
            col = mix_column(col)
            list_diff.append(bytes_to_word(col))
    return list_diff


def k10_cand_from_diffMC(ct, fct, col, diffMC_list):
    candidates = []
    good = []
    faulty = []

    for i in range(4):
        good.append(ct[POSITIONS[col][i]])
        faulty.append(fct[POSITIONS[col][i]])

    for i in range(len(diffMC_list)):
        for k0 in range(256):
            diff = int(invsbox[good[0] ^ k0] ^ invsbox[faulty[0] ^ k0])
            if diff != GETBYTE(diffMC_list[i], 0):
                continue
            for k1 in range(256):
                diff = int(invsbox[good[1] ^ k1] ^ invsbox[faulty[1] ^ k1])
                if diff != GETBYTE(diffMC_list[i], 1):
                    continue
                for k2 in range(256):
                    diff = int(invsbox[good[2] ^ k2] ^ invsbox[faulty[2] ^ k2])
                    if diff != GETBYTE(diffMC_list[i], 2):
                        continue
                    for k3 in range(256):
                        diff = int(invsbox[good[3] ^ k3] ^ invsbox[faulty[3] ^ k3])
                        if diff != GETBYTE(diffMC_list[i], 3):
                            continue
                        candidates.append((k0 << 24) | (k1 << 16) | (k2 << 8 )| k3)

    return candidates
    
def find_candidates(ct, fct):
    cand_len = []
    candidates = []

    for col9 in range(4):
        diffMC_list = get_diff_after_MC()
        curr_col_candidates = k10_cand_from_diffMC(ct, fct, col9, diffMC_list)
        cand_len.append(len(curr_col_candidates))
        candidates.append(curr_col_candidates)
    return candidates

def intersection(l1, l2):
    ans = []
    for i in range(len(l1)):
        for j in range(len(l2)):
            if l1[i] == l2[j]:
                ans.append(l2[j])
    return ans


def key_recovery(ct_list, fct_list):
    all_candidates = [None for i in range(4)]
    curr_cand_len = [None for i in range(4)]
    for i in range(len(ct_list)):
        candidates = find_candidates(ct_list[i], fct_list[i])
        curr_cand_count = 1
        for col in range(4):
            if not all_candidates[col]:
                all_candidates[col] = candidates[col].copy()
            else:
                all_candidates[col] = intersection(all_candidates[col], candidates[col])
            curr_cand_count = curr_cand_count * len(all_candidates[col])
            curr_cand_len[col]=len(all_candidates[col])
        print(f"Number of candidates for positions \n0, 13, 10, 7: {curr_cand_len[0]} \n4, 1, 14, 11: {curr_cand_len[1]}\n8, 5, 2, 15: {curr_cand_len[2]}\n12, 9, 6, 3: {curr_cand_len[3]}\n")
        print(f"Using {i + 1} ct-fct pairs, total number of candidates for 10th RK are: {curr_cand_count}\n\n")

    return all_candidates

f = open("EE19B106.txt")
l = f.readlines()[1:]
ct = []
fct = []
for i in range(len(l)):
    [cti, fcti] = l[i].strip().split()

    cct = []
    assert len(cti) == 32
    for j in range(0, 32, 2):
        cct.append(int(cti[j:j+2], 16))

    cfct = []
    assert len(fcti) == 32
    for j in range(0, 32, 2):
        cfct.append(int(fcti[j:j+2], 16))
    ct.append(cct)
    fct.append(cfct)

toc = time.time()
recovered_key = key_recovery(ct, fct)
tic = time.time()

subkey10 = [None for i in range(16)]
for i in range(4):
    for j in range(4):
        subkey10[POSITIONS[i][j]] = GETBYTE(recovered_key[i][0], j)

print("10th Round Key: ", [hex(ele) for ele in subkey10])
print(f"Time required to recover 10th round key: {tic - toc}")
# print(subkey10)


# subkeys = reverse_key_expansion(subkey10)
# for i in range(11):
#     print(subkeys[i*16:(i+1)*16])
#     print(" ".join([str(hex(ele))[2:] for ele in subkeys[i*16 : (i+1)*16]]))