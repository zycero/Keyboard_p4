# 自动等待芯片进入下载模式 -> 读回 Flash 0x2000~0x11000 -> 与本地文件比对
import subprocess, sys, os

PY = sys.executable
BD = r'E:\@job2025\36Keyboard\program\esp32-p4\.pio\build\keyboard36_p4'
BOOT_APP0 = r'C:\Users\zycero\.platformio\packages\framework-arduinoespressif32\tools\partitions\boot_app0.bin'
DUMP = r'E:\@job2025\36Keyboard\read_0x2000_0x11000.bin'

print('=' * 60)
print('READY! Do this within 60 seconds:')
print('  1. HOLD power key P_SW (keep board powered)')
print('  2. HOLD BOOT (H2-5 -> GND)')
print('  3. TAP RESET (H2-4 -> GND) then release')
print('  4. Release BOOT, keep holding P_SW')
print('=' * 60)

cmd = [PY, '-m', 'esptool', '--chip', 'esp32p4', '--port', 'COM9',
       '--baud', '115200', '--before', 'no_reset', '--after', 'no_reset',
       '--connect-attempts', '120', 'read-flash', '0x2000', '0xF000', DUMP]
try:
    r = subprocess.run(cmd, capture_output=True, text=True, timeout=300)
    out = (r.stdout or '') + (r.stderr or '')
    print(out[-1200:])
except subprocess.TimeoutExpired:
    print('超时'); sys.exit(1)

if not os.path.exists(DUMP):
    print('!! 读取失败(未进下载模式或连接失败)'); sys.exit(1)

def rd(p):
    with open(p, 'rb') as f:
        return f.read()

dump = rd(DUMP)
print('读回 %d 字节,开始比对...' % len(dump))
print('-' * 60)

def check(name, off, ref):
    seg = dump[off:off + len(ref)]
    if seg == ref:
        print('[OK]    %s 与本地文件完全一致' % name)
    elif seg[:64] == b'\xff' * min(64, len(seg)):
        print('[FAIL]  %s 是空白 FF(根本没写入!)' % name)
    else:
        print('[DIFF]  %s 数据存在但不一致! 前16字节: %s' % (name, seg[:16].hex()))
    print('        Flash实际前16: %s' % seg[:16].hex())
    print('        本地文件前16: %s' % ref[:16].hex())

check('bootloader @0x2000      ', 0x0000, rd(os.path.join(BD, 'bootloader.bin')))
check('partitions @0x8000      ', 0x6000, rd(os.path.join(BD, 'partitions.bin')))
check('boot_app0  @0xE000      ', 0xC000, rd(BOOT_APP0))
check('firmware   @0x10000(4KB)', 0xE000, rd(os.path.join(BD, 'firmware.bin'))[:4096])
print('-' * 60)
print('比对完成')
