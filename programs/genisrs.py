with open('rom/isrs.asm', 'w') as f:
    f.write('bits 16\ncpu 8086\n')
    f.write('segment isrs public align=4 use16 class=code\n')
    f.write('extern isr\n')
    for i in range(256):
        f.write('global isr{0}\n'.format(i))
        f.write('isr{0}:\n'.format(i))
        f.write('  push ax\n')
        f.write('  mov al, {0}\n'.format(i))
        f.write('  call isr\n')
        f.write('  pop ax\n')
        f.write('  iret\n')
        f.write('\n')
