#!/usr/bin/env python3
"""Patch a raw GBA ROM header: Nintendo logo, title, game code, checksum, padding."""
import sys

LOGO = bytes.fromhex(
    "24FFAE51699AA2213D84820A84E409AD11248B98C0817F21A352BE199309CE2010464A4AF82731EC58C7E83382E3CEBF85F4DF94CE4B09C194568AC01372A7FC9F844D73A3CA9A615897A327FC039876231DC7610304AE56BF38840040A70EFDFF52FE036F9530F197FBC08560D68025A963BE03014E38E2F9A234FFBB3E0344780090CB88113A9465C07C6387F03CAFD625E48B380AAC7221D4F807")
assert len(LOGO) == 156

def crc16(data):
    crc = 0xFFFF
    for b in data:
        crc ^= b
        for _ in range(8):
            crc = (crc >> 1) ^ 0xA001 if crc & 1 else crc >> 1
    return crc

assert crc16(LOGO) == 0xCF56, hex(crc16(LOGO))

def main(path, title="MONSTERMONS", code="AMME", maker="01"):
    rom = bytearray(open(path, "rb").read())
    rom[0x04:0xA0] = LOGO
    rom[0xA0:0xAC] = title.encode()[:12].ljust(12, b"\0")
    rom[0xAC:0xB0] = code.encode()
    rom[0xB0:0xB2] = maker.encode()
    rom[0xB2] = 0x96
    rom[0xB3] = 0
    rom[0xB4:0xBD] = bytes(9)
    chk = 0
    for b in rom[0xA0:0xBD]:
        chk = (chk - b) & 0xFF
    rom[0xBD] = (chk - 0x19) & 0xFF
    # pad to a power-of-two-friendly size boundary (multiple of 256 bytes)
    while len(rom) % 256:
        rom.append(0xFF)
    open(path, "wb").write(rom)

if __name__ == "__main__":
    main(*sys.argv[1:])
