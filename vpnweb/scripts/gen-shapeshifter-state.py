#!/usr/bin/env python3
"""
Generates the Curve25519 keypair that is needed by the shapeshifter-dispatcher
server.

Depends on python3-pysodium package.
"""

import base64
import json
import os

import pysodium
import binascii

BRIDGE_PREAMBLE = "Bridge obfs4 <IP ADDRESS>:<PORT> <FINGERPRINT> cert="
BRIDGE_END = " iatMode=0"


def generate(statedir):
    try:
        os.makedirs(statedir)
    except Exception:
        pass
    print("[+] Generating shapeshifter parameters...")

    public, private = pysodium.crypto_box_keypair()

    priv_hex = binascii.b2a_hex(private)
    pub_hex = binascii.b2a_hex(public)
    node_id = os.urandom(20)
    node_id_hex = binascii.b2a_hex(node_id)
    drbg_seed = os.urandom(24)

    def tostr(b):
        return b.decode('utf-8')

    with open(statedir + '/obfs4_state.json', 'w') as state:
        state.write(json.dumps({
            'node-id': tostr(node_id_hex),
            'private-key': tostr(priv_hex),
            'public-key': tostr(pub_hex),
            'drbg-seed': tostr(binascii.b2a_hex(drbg_seed)),
            'iatMode': 0}))

    cert = base64.b64encode(node_id + public)
    print("CERT:", cert)

    with open(statedir + '/obfs4_cert.txt', 'w') as certf:
        certf.write(tostr(cert).rstrip('='))

    with open(statedir + '/obfs4_bridgeline.txt', 'w') as bridgef:
        bridgef.write(BRIDGE_PREAMBLE + tostr(cert) + BRIDGE_END)
    print("[+] done")


if __name__ == "__main__":
    import argparse
    parser = argparse.ArgumentParser()
    parser.add_argument("statedir")
    args = parser.parse_args()
    generate(args.statedir)
