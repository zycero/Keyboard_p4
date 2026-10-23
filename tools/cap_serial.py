import serial, time
s = serial.Serial('COM9', 115200, timeout=0.2)
print('listening 18s ... do BOOT+RESET now')
total = b''
for i in range(18):
    d = s.read(4096)
    total += d
    print('sec %02d: %4d bytes %s' % (i + 1, len(d), repr(d[:60])))
    time.sleep(0.8)
s.close()
