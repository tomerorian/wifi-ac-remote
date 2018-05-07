def split_len(seq, length):
    return [seq[i:i+length] for i in range(0, len(seq), length)]

def split_codes():
    codes = '''
    '''

    for a in codes.split():
      a = a[1:-1].replace(' ', '')
      a = split_len(a, 4)

      print "7 " + " ".join(a) + " 6"

def num(s):
    return int(s, 2)

def xbin(s):
    return hex(num(s))

def calc_crc(code):
    b0 = code[0:8][::-1]
    b1 = code[8:16][::-1]
    b2 = code[16:24][::-1]
    b3 = code[24:32][::-1]
    b4 = code[32:40][::-1]
    b5 = code[40:48][::-1]
    b6 = code[48:56][::-1]
    b7 = code[56:64][::-1]
    bs = [b0, b1, b2, b3, b4, b5, b6]

    crc = 0
    for b in bs:
        crc += (num(b) & 0xF) + (num(b) >> 4 & 0xF)
        crc &= 0xFF
        print crc

    print xbin(b7) == hex(crc),
    print "expected:", xbin(b7),
    print "result:", hex(crc), " ".join(split_len(code, 4))

def calc_crcs():
    codes = '''
    710000000100011000111110000000000000000000000110000000000100110006
    '''

    codes = [code.replace(' ', '')[1:-1] for code in codes.split('\n') if code.strip() != '']

    for code in codes:
        calc_crc(code)


calc_crcs()
