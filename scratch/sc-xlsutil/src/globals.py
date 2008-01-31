
import sys, struct, math

class ByteConvertError(Exception): pass


class Params(object):
    def __init__ (self):
        self.Debug = False


def output (msg):
    sys.stdout.write(msg)

def error (msg):
    sys.stderr.write(msg)


def decodeName (name):
    """decode name that contains unprintable characters."""

    if len(name) == 0:
        return name

    if ord(name[0]) <= 5:
        name = "<%2.2Xh>"%ord(name[0]) + name[1:]

    return name


def getRichText (bytes, textLen=None):
    """parse a string of the rich-text format that Excel uses."""

    flags = bytes[0]
    if type(flags) == type('c'):
        flags = ord(flags)
    is16Bit   = (flags & 0x01)
    isFarEast = (flags & 0x04)
    isRich    = (flags & 0x08)

    i = 1
    formatRuns = 0
    if isRich:
        formatRuns = getSignedInt(bytes[i:i+2])
        i += 2

    extInfo = 0
    if isFarEast:
        extInfo = getSignedInt(bytes[i:i+4])
        i += 4

    extraBytes = 0
    if textLen == None:
        extraBytes = formatRuns*4 + extInfo
        textLen = len(bytes) - extraBytes - i

    totalByteLen = i + textLen + extraBytes
    if is16Bit:
        return ("<16-bit strings not supported yet>", totalByteLen)

    text = toTextBytes(bytes[i:i+textLen])
    return (text, totalByteLen)


def dumpBytes (chars, subDivide=None):
    line = 0
    subDivideLine = None
    if subDivide != None:
        subDivideLine = subDivide/16

    charLen = len(chars)
    if charLen == 0:
        # no bytes to dump.
        return

    labelWidth = int(math.ceil(math.log(charLen, 10)))
    flushBytes = False
    for i in xrange(0, charLen):
        if (i+1)%16 == 1:
            # print line header with seek position
            fmt = "%%%d.%dd: "%(labelWidth, labelWidth)
            output(fmt%i)

        byte = ord(chars[i])
        output("%2.2X "%byte)
        flushBytes = True

        if (i+1)%4 == 0:
            # put extra space at every 4 bytes.
            output(" ")

        if (i+1)%16 == 0:
            output("\n")
            flushBytes = False
            if subDivideLine != None and (line+1)%subDivideLine == 0:
                output("\n")
            line += 1

    if flushBytes:
        output("\n")


def getSectorPos (secID, secSize):
    return 512 + secID*secSize


def getRawBytes (bytes, spaced=True, reverse=True):
    text = ''
    for b in bytes:
        if type(b) == type(''):
            b = ord(b)
        if len(text) == 0:
            text = "%2.2X"%b
        elif spaced:
            if reverse:
                text = "%2.2X "%b + text
            else:
                text += " %2.2X"%b
        else:
            if reverse:
                text = "%2.2X"%b + text
            else:
                text += "%2.2X"%b
    return text


def getTextBytes (bytes):
    return toTextBytes(bytes)


def toTextBytes (bytes):
    n = len(bytes)
    text = ''
    for i in xrange(0, n):
        b = bytes[i]
        if type(b) == type(0x00):
            b = struct.pack('B', b)
        text += b
    return text


def getSignedInt (bytes):
    # little endian
    n = len(bytes)
    if n == 0:
        return 0

    text = toTextBytes(bytes)
    if n == 1:
        # byte - 1 byte
        return struct.unpack('b', text)[0]
    elif n == 2:
        # short - 2 bytes
        return struct.unpack('<h', text)[0]
    elif n == 4:
        # int, long - 4 bytes
        return struct.unpack('<l', text)[0]

    raise ByteConvertError


def getFloat (bytes):
    n = len(bytes)
    if n == 0:
        return 0.0

    text = toTextBytes(bytes)
    return struct.unpack('<f', text)[0]


def getDouble (bytes):
    n = len(bytes)
    if n == 0:
        return 0.0

    text = toTextBytes(bytes)
    return struct.unpack('<d', text)[0]


def getUTF8FromUTF16 (bytes):
    # little endian utf-16 strings
    byteCount = len(bytes)
    loopCount = int(byteCount/2)
    text = ''
    for i in xrange(0, loopCount):
        code = ''
        if bytes[i*2+1] != '\x00':
            code += bytes[i*2+1]
        if bytes[i*2] != '\x00':
            code += bytes[i*2]
        text += unicode(code, 'utf-8')
    return text

