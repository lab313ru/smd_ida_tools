# 1) Export ASM from IDA
# 2) Export MAP from IDA
# 3) asm_fixer.py rom.asm rom.map
# 4) outputs: rom_new.asm, equals.inc, externs.inc, rams.inc, structs.inc

'''
Valid tags for IDA:
1) CODE_START - CODE_END
2) ORG
3) DEL_START - DEL_END
4) BIN_START - BIN_END
5) INC_START - INC_END
(Insert opening tag by pressing Ins button at object's start)
(Insert closing tag by pressing Shift+Ins button at object's end)

# IF you want to replace some code with the new one, use the following            #
# Code between brackets will replace code after } bracket and before CODE_END tag #
CODE_START
{
    some_new_code_line1
    some_new_code_line2
    some_new_code_line3
}
    some_original_code_line1
    some_original_code_line2
    some_original_code_line3
CODE_END


# If you want to use AS's org directive, use the following #
# It will insert org directive into fixed ASM listing      #
ORG $AABBCC


# If you want to exclude some unused code (function or data array), #
# place opening tag DEL_START at the beggining of unneeded code,    #
# and closing tag DEL_END - at the end of unneded code              #

DEL_START
useless_array: dc.b $AB, $AB
DEL_END


# If you want to move some code (data array, or just part of code) #
# into separate file use the following #
# BIN_START - BIN_END - for arrays to store them in binary form. Will create binclude directive #
# INC_START - INC_END - for parts of listing to store them as-is. Will create include directive #

BIN_START "some_dir/some_bin.bin"
data_bytes: dc.b $BC, $BC
    dc.b $CD, $CD
BIN_END

INC_START "some_dir/some_inc.inc"
palette_bytes: dc.w $EBC, $ABC, $EEE, $EEE
INC_END

'''

import errno
import struct
import sys
import re
import os


def collect_structs(text):
    structs = dict()

    r = re.compile(r'^(\w+)[ \t]+struc.*\n(?:^[ \t]+.*\n)?((?:\w+:(?:[ \t]+)?(?:dc\.[bwl])|(?:\w+).*\n)+)\1[ \t]+ends', re.MULTILINE)

    mm = r.findall(text)

    for m in mm:
        structs[m[0]] = m[1].splitlines()

    return structs


def collect_equs(text):
    equs = dict()

    r = re.compile(r'^(\w+):(?:[ \t]+)?equ[ \t]+((?:\$)?[0-9A-F]+)', re.MULTILINE)

    mm = r.findall(text)

    for m in mm:
        equs[m[0]] = m[1]

    return equs


def get_rom_start(text):
    p = text.find('\n; segment "ROM"')

    if p == -1:
        return text

    return text[p+1:]


def get_rom_end(text):
    p = text.find('\n; end of \'ROM\'')

    if p == -1:
        return text

    return text[:p+1]


def fix_dcb(text):
    r = re.compile(r'(dcb\.b[ \t]+((?:\$)?[0-9A-F]+),((?:\$)?[0-9A-F]+))')

    mm = r.findall(text)

    for m in mm:
        count = int(m[1][1:], 16) if m[1][0] == '$' else int(m[1])
        val = int(m[2][1:], 16) if m[2][0] == '$' else int(m[2])

        if val == 0:
            text = text.replace(m[0], 'rorg $%X' % count, 1)
        else:
            text = text.replace(m[0], 'dc.b [%s]%s' % (m[1], m[2]), 1)

    # text, n = r.subn('dc.\g<1> [\g<2>]\g<3>', text)

    return text


def create_externs():
    externs = dict()

    # externs['Z80_RAM'] = 0xA00000
    # externs['Z80_YM2612'] = 0xA04000
    externs['IO_PCBVER'] = 0xA10000
    externs['IO_CT1_DATA'] = 0xA10002
    externs['IO_CT2_DATA'] = 0xA10004
    externs['IO_EXT_DATA'] = 0xA10006
    externs['IO_CT1_CTRL'] = 0xA10008
    externs['IO_CT2_CTRL'] = 0xA1000A
    externs['IO_EXT_CTRL'] = 0xA1000C
    externs['IO_CT1_RX'] = 0xA1000E
    externs['IO_CT1_TX'] = 0xA10010
    externs['IO_CT1_SMODE'] = 0xA10012
    externs['IO_CT2_RX'] = 0xA10014
    externs['IO_CT2_TX'] = 0xA10016
    externs['IO_CT2_SMODE'] = 0xA10018
    externs['IO_EXT_RX'] = 0xA1001A
    externs['IO_EXT_TX'] = 0xA1001C
    externs['IO_EXT_SMODE'] = 0xA1001E
    externs['IO_RAMMODE'] = 0xA11000
    externs['IO_Z80BUS'] = 0xA11100
    externs['IO_Z80RES'] = 0xA11200
    externs['IO_FDC'] = 0xA12000
    externs['IO_TIME'] = 0xA13000
    externs['IO_TMSS'] = 0xA14000
    externs['VDP_DATA'] = 0xC00000
    externs['VDP__DATA'] = 0xC00002
    externs['VDP_CTRL'] = 0xC00004
    externs['VDP__CTRL'] = 0xC00006
    externs['VDP_CNTR'] = 0xC00008
    externs['VDP__CNTR'] = 0xC0000A
    externs['VDP___CNTR'] = 0xC0000C
    externs['VDP____CNTR'] = 0xC0000E
    externs['VDP_PSG'] = 0xC00011

    return externs


def collect_rams(text):
    rams = dict()

    r = re.compile(r'^ [0-9A-F]{4}:((?:00FF|00A0)[0-9A-F]{4}) {7}(\w+)', re.MULTILINE)

    p = text.find(':00A00000')

    if p == -1:
        return rams

    text = text[p-5:]

    mm = r.findall(text)

    for m in mm:
        rams[m[0]] = m[1]

    return rams


def exact_zero_off(text):
    r = re.compile(r'([ \t,]+0)\(', re.MULTILINE)
    text, n = r.subn('\g<1>.w(', text)

    return text


def fix_struct_start_end(text, structs):
    for struc in structs:
        pat = '(%s)[ \\t]+<(.*)>' % struc
        r = re.compile(pat, re.MULTILINE)
        text, n = r.subn('\g<1> \g<2>', text)

    return text


def remove_many_empty(text):
    r = re.compile(r'(\n){3}', re.MULTILINE)
    text, n = r.subn('\g<1>', text)

    return text


def remove_ida_comments(text):
    r = re.compile(r'^[ \t]+;[ \t]+(?:DATA|CODE)[ \t]+XREF.*\n', re.MULTILINE)
    text, n = r.subn('', text)

    r = re.compile(r'[ \t]+;[ \t]+(?:DATA|CODE)[ \t]+XREF.*', re.MULTILINE)
    text, n = r.subn('', text)

    r = re.compile(r'^[ \t]+;[ \t]+.*\n', re.MULTILINE)
    text, n = r.subn('', text)

    r = re.compile(r'^;[ \t]+[-=]+\n', re.MULTILINE)
    text, n = r.subn('', text)

    r = re.compile(r'\n;[ \t]+=+[ \t]+S[ \t]+U[ \t]+B[ \t]+R.*\n', re.MULTILINE)
    text, n = r.subn('', text)

    r = re.compile(r'^;[ \t]+End[ \t]+of[ \t]+function.*\n', re.MULTILINE)
    text, n = r.subn('', text)

    r = re.compile(r'(\$[0-9A-F]{2}.*)[ \t]+;.*', re.MULTILINE)
    text, n = r.subn('\g<1>', text)

    r = re.compile(r'^;[ \t]+Attributes:[ \t]+.*', re.MULTILINE)
    text, n = r.subn('', text)

    text = remove_many_empty(text)

    return text


def fix_quotates(text):
    r = re.compile(r"([^#'])'(.+?)'", re.MULTILINE)

    text, n = r.subn('\g<1>"\g<2>"', text)

    return text


def fix_at(text):
    r = re.compile(r'^((?:\w*@+\w*)+):', re.MULTILINE)

    mm = r.findall(text)

    for m in mm:
        text = text.replace(m, m.replace('@', '_'))

    return text


def apply_del(text):
    r = re.compile(r'^(DEL_START.+?DEL_END)$', re.MULTILINE | re.DOTALL)

    text, n = r.subn('', text)

    return text


def mkdir_p(path):
    try:
        os.makedirs(path)
    except OSError as exc:
        if exc.errno == errno.EEXIST and os.path.isdir(path):
            pass
        else:
            raise


def parse_array(text, size):
    r = re.compile(r'((?:\$)?[0-9A-F]+)', re.MULTILINE)

    mm = r.findall(text)

    data = ''

    for m in mm:
        val = int(m[1:], 16) if m[0] == '$' else int(m)
        if size == 'b':
            data += struct.pack('>B', val & 0xFF)
        elif size == 'w':
            data += struct.pack('>H', val & 0xFFFF)
        else:
            data += struct.pack('>I', val & 0xFFFFFFFF)

    return data


def apply_bin(text):
    r = re.compile(
        r'^BIN_START[ \t]+"(.*)"\n(\w+):((?:(?:[ \t]+)?dc\.([bwl])[ \t]+(?:(?:\$)?[0-9A-F]+(?:,(?:[ \t]+)?)?)+\n)+?)BIN_END$',
        re.MULTILINE)

    while True:
        mm = r.findall(text)

        if len(mm) == 0:
            break

        for m in mm:
            dd = os.path.dirname(m[0])
            mkdir_p(dd)

            with open(m[0], 'wb') as w:
                data = parse_array(m[2], m[3])
                w.write(data)

            text, n = r.subn('\g<2>:\n    binclude "\g<1>"', text)

    return text


def apply_inc(text):
    r = re.compile(
        r'^INC_START[ \t]+"(.*)"\n(\w+):((?:(?:[ \t]+)?dc\.[bwl][ \t]+(?:(?:\$)?[0-9A-F]+(?:,(?:[ \t]+)?)?)+\n)+?)INC_END$',
        re.MULTILINE)

    while True:
        mm = r.findall(text)

        if len(mm) == 0:
            break

        for m in mm:
            dd = os.path.dirname(m[0])
            mkdir_p(dd)

            with open(m[0], 'wb') as w:
                w.write(m[2])

            text, n = r.subn('\g<2>:\n    include "\g<1>"', text)

    return text


def apply_code(text):
    r = re.compile(r'^CODE_START\n{\n((?:.*\n)+?)}\n+(\w+):((?:.*\n)+?)CODE_END$', re.MULTILINE)
    text, n = r.subn('\n\g<2>:\n\g<1>', text)

    r = re.compile(r'^CODE_START\n{\n((?:.*\n)+?)}\n+((?:.*\n)+?)CODE_END$', re.MULTILINE)
    text, n = r.subn('\n\g<1>', text)

    return text


def apply_org(text):
    r = re.compile(r'^ORG[ \t]+(\$[0-9A-F]+)$', re.MULTILINE)

    text, n = r.subn('    org \g<1>', text)

    return text


def main1(path):
    with open(path, 'rb') as f:
        dr = os.path.dirname(path)

        text = f.read()
        text = text.replace('\r\n', '\n')
        structs = collect_structs(text)
        equs = collect_equs(text)
        externs = create_externs()

        text = get_rom_start(text)
        text = get_rom_end(text)

        r = re.compile(r'^\w+:(?:[ \t]+)?(.*)[ \t]+(?:(?:(\d+)[ \t]+dup\(\?\))|(?:\?.*))')

        with open(os.path.join(dr, 'structs.inc'), 'wb') as w:
            for struct, keys in structs.iteritems():
                pp = ','.join('p'+str(i) for i, _ in enumerate(keys))
                w.write('%s macro %s\n' % (struct, pp))

                pat = '%s[ \\t]+<0>' % struct
                r3 = re.compile(pat, re.MULTILINE)

                text, rr3 = r3.subn(('%s <' % struct) + ', '.join(['0'] * len(keys)) + '>', text)

                for i, key in enumerate(keys):
                    m = r.findall(key)

                    if len(m) == 1:
                        m = m[0]
                        if m[1] == '':
                            w.write('    %s %s\n' % (m[0], 'p%d' % i))
                        else:
                            w.write('    %s [%s]%s\n' % (m[0], m[1], 'p%d' % i))

                w.write('%s endm\n\n' % struct)

        with open(os.path.join(dr, 'equals.inc'), 'wb') as w:
            for equ, val in equs.iteritems():
                w.write('%s: equ %s\n' % (equ, val))

        with open(os.path.join(dr, 'externs.inc'), 'wb') as w:
            for x, val in externs.iteritems():
                w.write('%s: equ $%06X\n' % (x, val))

        pre, ext = os.path.splitext(path)
        with open(pre + '_new' + ext, 'wb') as w:
            w.write('    cpu 68000\n')
            w.write('    supmode on\n')
            w.write('    padding off\n')
            if len(structs) > 0:
                w.write('    include "%s"\n' % 'structs.inc')
            if len(equs) > 0:
                w.write('    include "%s"\n' % 'equals.inc')
            if len(sys.argv) > 2:
                w.write('    include "%s"\n' % 'rams.inc')
            if len(externs) > 0:
                w.write('    include "%s"\n' % 'externs.inc')
            w.write('    include "%s"\n\n' % 'funcs.inc')

            text = exact_zero_off(text)
            text = fix_dcb(text)
            text = fix_struct_start_end(text, structs)
            text = remove_ida_comments(text)
            text = apply_del(text)
            text = apply_bin(text)
            text = apply_inc(text)
            text = apply_code(text)
            text = apply_org(text)
            text = remove_many_empty(text)
            text = fix_quotates(text)
            text = fix_at(text)

            w.write(text)


def main2(path):
    with open(path, 'rb') as f:
        dr = os.path.dirname(path)
        text = f.read()

        rams = collect_rams(text)

        with open(os.path.join(dr, 'rams.inc'), 'wb') as w:
            for addr, val in rams.iteritems():
                w.write('%s: equ $%s\n' % (val, addr[2:]))


def main3(path):
    dr = os.path.dirname(path)
    funcs_inc_path = os.path.join(dr, 'funcs.inc')

    if not os.path.exists(funcs_inc_path):
        with open(funcs_inc_path, 'wb') as w:
            w.write('\n')


if __name__ == '__main__':
    main1(sys.argv[1])
    if len(sys.argv) > 2:
        main2(sys.argv[2])
    main3(sys.argv[1])
