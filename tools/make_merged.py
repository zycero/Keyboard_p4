# Keyboard36 v0.1 (ESP32-P4) - merged firmware packer
# Layout (P4): 0x0 FF-pad | 0x2000 bootloader | 0x8000 partitions
#              | 0xE000 boot_app0(ota data) | 0x10000 firmware
import os

BD = r'E:\@job2025\36Keyboard\program\esp32-p4\.pio\build\keyboard36_p4'
BOOT_APP0 = os.path.expandvars(
    r'%USERPROFILE%\.platformio\packages\framework-arduinoespressif32\tools\partitions\boot_app0.bin')
OUT = r'E:\@job2025\36Keyboard\program\esp32-p4\firmware_merged.bin'

def rd(p):
    with open(p, 'rb') as f:
        return f.read()

parts = [
    (0x2000, rd(os.path.join(BD, 'bootloader.bin'))),
    (0x8000, rd(os.path.join(BD, 'partitions.bin'))),
    (0xE000, rd(BOOT_APP0)),
    (0x10000, rd(os.path.join(BD, 'firmware.bin'))),
]
size = max(o + len(d) for o, d in parts)
buf = bytearray(b'\xff' * size)
for off, data in parts:
    buf[off:off + len(data)] = data
with open(OUT, 'wb') as f:
    f.write(buf)

print('written %s (%d bytes)' % (OUT, len(buf)))
for addr in (0x0, 0x2000, 0x8000, 0xE000, 0x10000):
    print('  0x%05X: 0x%02X' % (addr, buf[addr]))
assert buf[0x2000] == 0xE9, 'bootloader magic missing!'
assert buf[0x10000] == 0xE9, 'firmware magic missing!'
print('OK: bootloader@0x2000 & firmware@0x10000 magic verified')
