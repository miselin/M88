bits 16
cpu 8086
segment isrs public align=4 use16 class=code
extern isr
global isr0
isr0:
  push ax
  mov al, 0
  call isr
  pop ax
  iret

global isr1
isr1:
  push ax
  mov al, 1
  call isr
  pop ax
  iret

global isr2
isr2:
  push ax
  mov al, 2
  call isr
  pop ax
  iret

global isr3
isr3:
  push ax
  mov al, 3
  call isr
  pop ax
  iret

global isr4
isr4:
  push ax
  mov al, 4
  call isr
  pop ax
  iret

global isr5
isr5:
  push ax
  mov al, 5
  call isr
  pop ax
  iret

global isr6
isr6:
  push ax
  mov al, 6
  call isr
  pop ax
  iret

global isr7
isr7:
  push ax
  mov al, 7
  call isr
  pop ax
  iret

global isr8
isr8:
  push ax
  mov al, 8
  call isr
  pop ax
  iret

global isr9
isr9:
  push ax
  mov al, 9
  call isr
  pop ax
  iret

global isr10
isr10:
  push ax
  mov al, 10
  call isr
  pop ax
  iret

global isr11
isr11:
  push ax
  mov al, 11
  call isr
  pop ax
  iret

global isr12
isr12:
  push ax
  mov al, 12
  call isr
  pop ax
  iret

global isr13
isr13:
  push ax
  mov al, 13
  call isr
  pop ax
  iret

global isr14
isr14:
  push ax
  mov al, 14
  call isr
  pop ax
  iret

global isr15
isr15:
  push ax
  mov al, 15
  call isr
  pop ax
  iret

global isr16
isr16:
  push ax
  mov al, 16
  call isr
  pop ax
  iret

global isr17
isr17:
  push ax
  mov al, 17
  call isr
  pop ax
  iret

global isr18
isr18:
  push ax
  mov al, 18
  call isr
  pop ax
  iret

global isr19
isr19:
  push ax
  mov al, 19
  call isr
  pop ax
  iret

global isr20
isr20:
  push ax
  mov al, 20
  call isr
  pop ax
  iret

global isr21
isr21:
  push ax
  mov al, 21
  call isr
  pop ax
  iret

global isr22
isr22:
  push ax
  mov al, 22
  call isr
  pop ax
  iret

global isr23
isr23:
  push ax
  mov al, 23
  call isr
  pop ax
  iret

global isr24
isr24:
  push ax
  mov al, 24
  call isr
  pop ax
  iret

global isr25
isr25:
  push ax
  mov al, 25
  call isr
  pop ax
  iret

global isr26
isr26:
  push ax
  mov al, 26
  call isr
  pop ax
  iret

global isr27
isr27:
  push ax
  mov al, 27
  call isr
  pop ax
  iret

global isr28
isr28:
  push ax
  mov al, 28
  call isr
  pop ax
  iret

global isr29
isr29:
  push ax
  mov al, 29
  call isr
  pop ax
  iret

global isr30
isr30:
  push ax
  mov al, 30
  call isr
  pop ax
  iret

global isr31
isr31:
  push ax
  mov al, 31
  call isr
  pop ax
  iret

global isr32
isr32:
  push ax
  mov al, 32
  call isr
  pop ax
  iret

global isr33
isr33:
  push ax
  mov al, 33
  call isr
  pop ax
  iret

global isr34
isr34:
  push ax
  mov al, 34
  call isr
  pop ax
  iret

global isr35
isr35:
  push ax
  mov al, 35
  call isr
  pop ax
  iret

global isr36
isr36:
  push ax
  mov al, 36
  call isr
  pop ax
  iret

global isr37
isr37:
  push ax
  mov al, 37
  call isr
  pop ax
  iret

global isr38
isr38:
  push ax
  mov al, 38
  call isr
  pop ax
  iret

global isr39
isr39:
  push ax
  mov al, 39
  call isr
  pop ax
  iret

global isr40
isr40:
  push ax
  mov al, 40
  call isr
  pop ax
  iret

global isr41
isr41:
  push ax
  mov al, 41
  call isr
  pop ax
  iret

global isr42
isr42:
  push ax
  mov al, 42
  call isr
  pop ax
  iret

global isr43
isr43:
  push ax
  mov al, 43
  call isr
  pop ax
  iret

global isr44
isr44:
  push ax
  mov al, 44
  call isr
  pop ax
  iret

global isr45
isr45:
  push ax
  mov al, 45
  call isr
  pop ax
  iret

global isr46
isr46:
  push ax
  mov al, 46
  call isr
  pop ax
  iret

global isr47
isr47:
  push ax
  mov al, 47
  call isr
  pop ax
  iret

global isr48
isr48:
  push ax
  mov al, 48
  call isr
  pop ax
  iret

global isr49
isr49:
  push ax
  mov al, 49
  call isr
  pop ax
  iret

global isr50
isr50:
  push ax
  mov al, 50
  call isr
  pop ax
  iret

global isr51
isr51:
  push ax
  mov al, 51
  call isr
  pop ax
  iret

global isr52
isr52:
  push ax
  mov al, 52
  call isr
  pop ax
  iret

global isr53
isr53:
  push ax
  mov al, 53
  call isr
  pop ax
  iret

global isr54
isr54:
  push ax
  mov al, 54
  call isr
  pop ax
  iret

global isr55
isr55:
  push ax
  mov al, 55
  call isr
  pop ax
  iret

global isr56
isr56:
  push ax
  mov al, 56
  call isr
  pop ax
  iret

global isr57
isr57:
  push ax
  mov al, 57
  call isr
  pop ax
  iret

global isr58
isr58:
  push ax
  mov al, 58
  call isr
  pop ax
  iret

global isr59
isr59:
  push ax
  mov al, 59
  call isr
  pop ax
  iret

global isr60
isr60:
  push ax
  mov al, 60
  call isr
  pop ax
  iret

global isr61
isr61:
  push ax
  mov al, 61
  call isr
  pop ax
  iret

global isr62
isr62:
  push ax
  mov al, 62
  call isr
  pop ax
  iret

global isr63
isr63:
  push ax
  mov al, 63
  call isr
  pop ax
  iret

global isr64
isr64:
  push ax
  mov al, 64
  call isr
  pop ax
  iret

global isr65
isr65:
  push ax
  mov al, 65
  call isr
  pop ax
  iret

global isr66
isr66:
  push ax
  mov al, 66
  call isr
  pop ax
  iret

global isr67
isr67:
  push ax
  mov al, 67
  call isr
  pop ax
  iret

global isr68
isr68:
  push ax
  mov al, 68
  call isr
  pop ax
  iret

global isr69
isr69:
  push ax
  mov al, 69
  call isr
  pop ax
  iret

global isr70
isr70:
  push ax
  mov al, 70
  call isr
  pop ax
  iret

global isr71
isr71:
  push ax
  mov al, 71
  call isr
  pop ax
  iret

global isr72
isr72:
  push ax
  mov al, 72
  call isr
  pop ax
  iret

global isr73
isr73:
  push ax
  mov al, 73
  call isr
  pop ax
  iret

global isr74
isr74:
  push ax
  mov al, 74
  call isr
  pop ax
  iret

global isr75
isr75:
  push ax
  mov al, 75
  call isr
  pop ax
  iret

global isr76
isr76:
  push ax
  mov al, 76
  call isr
  pop ax
  iret

global isr77
isr77:
  push ax
  mov al, 77
  call isr
  pop ax
  iret

global isr78
isr78:
  push ax
  mov al, 78
  call isr
  pop ax
  iret

global isr79
isr79:
  push ax
  mov al, 79
  call isr
  pop ax
  iret

global isr80
isr80:
  push ax
  mov al, 80
  call isr
  pop ax
  iret

global isr81
isr81:
  push ax
  mov al, 81
  call isr
  pop ax
  iret

global isr82
isr82:
  push ax
  mov al, 82
  call isr
  pop ax
  iret

global isr83
isr83:
  push ax
  mov al, 83
  call isr
  pop ax
  iret

global isr84
isr84:
  push ax
  mov al, 84
  call isr
  pop ax
  iret

global isr85
isr85:
  push ax
  mov al, 85
  call isr
  pop ax
  iret

global isr86
isr86:
  push ax
  mov al, 86
  call isr
  pop ax
  iret

global isr87
isr87:
  push ax
  mov al, 87
  call isr
  pop ax
  iret

global isr88
isr88:
  push ax
  mov al, 88
  call isr
  pop ax
  iret

global isr89
isr89:
  push ax
  mov al, 89
  call isr
  pop ax
  iret

global isr90
isr90:
  push ax
  mov al, 90
  call isr
  pop ax
  iret

global isr91
isr91:
  push ax
  mov al, 91
  call isr
  pop ax
  iret

global isr92
isr92:
  push ax
  mov al, 92
  call isr
  pop ax
  iret

global isr93
isr93:
  push ax
  mov al, 93
  call isr
  pop ax
  iret

global isr94
isr94:
  push ax
  mov al, 94
  call isr
  pop ax
  iret

global isr95
isr95:
  push ax
  mov al, 95
  call isr
  pop ax
  iret

global isr96
isr96:
  push ax
  mov al, 96
  call isr
  pop ax
  iret

global isr97
isr97:
  push ax
  mov al, 97
  call isr
  pop ax
  iret

global isr98
isr98:
  push ax
  mov al, 98
  call isr
  pop ax
  iret

global isr99
isr99:
  push ax
  mov al, 99
  call isr
  pop ax
  iret

global isr100
isr100:
  push ax
  mov al, 100
  call isr
  pop ax
  iret

global isr101
isr101:
  push ax
  mov al, 101
  call isr
  pop ax
  iret

global isr102
isr102:
  push ax
  mov al, 102
  call isr
  pop ax
  iret

global isr103
isr103:
  push ax
  mov al, 103
  call isr
  pop ax
  iret

global isr104
isr104:
  push ax
  mov al, 104
  call isr
  pop ax
  iret

global isr105
isr105:
  push ax
  mov al, 105
  call isr
  pop ax
  iret

global isr106
isr106:
  push ax
  mov al, 106
  call isr
  pop ax
  iret

global isr107
isr107:
  push ax
  mov al, 107
  call isr
  pop ax
  iret

global isr108
isr108:
  push ax
  mov al, 108
  call isr
  pop ax
  iret

global isr109
isr109:
  push ax
  mov al, 109
  call isr
  pop ax
  iret

global isr110
isr110:
  push ax
  mov al, 110
  call isr
  pop ax
  iret

global isr111
isr111:
  push ax
  mov al, 111
  call isr
  pop ax
  iret

global isr112
isr112:
  push ax
  mov al, 112
  call isr
  pop ax
  iret

global isr113
isr113:
  push ax
  mov al, 113
  call isr
  pop ax
  iret

global isr114
isr114:
  push ax
  mov al, 114
  call isr
  pop ax
  iret

global isr115
isr115:
  push ax
  mov al, 115
  call isr
  pop ax
  iret

global isr116
isr116:
  push ax
  mov al, 116
  call isr
  pop ax
  iret

global isr117
isr117:
  push ax
  mov al, 117
  call isr
  pop ax
  iret

global isr118
isr118:
  push ax
  mov al, 118
  call isr
  pop ax
  iret

global isr119
isr119:
  push ax
  mov al, 119
  call isr
  pop ax
  iret

global isr120
isr120:
  push ax
  mov al, 120
  call isr
  pop ax
  iret

global isr121
isr121:
  push ax
  mov al, 121
  call isr
  pop ax
  iret

global isr122
isr122:
  push ax
  mov al, 122
  call isr
  pop ax
  iret

global isr123
isr123:
  push ax
  mov al, 123
  call isr
  pop ax
  iret

global isr124
isr124:
  push ax
  mov al, 124
  call isr
  pop ax
  iret

global isr125
isr125:
  push ax
  mov al, 125
  call isr
  pop ax
  iret

global isr126
isr126:
  push ax
  mov al, 126
  call isr
  pop ax
  iret

global isr127
isr127:
  push ax
  mov al, 127
  call isr
  pop ax
  iret

global isr128
isr128:
  push ax
  mov al, 128
  call isr
  pop ax
  iret

global isr129
isr129:
  push ax
  mov al, 129
  call isr
  pop ax
  iret

global isr130
isr130:
  push ax
  mov al, 130
  call isr
  pop ax
  iret

global isr131
isr131:
  push ax
  mov al, 131
  call isr
  pop ax
  iret

global isr132
isr132:
  push ax
  mov al, 132
  call isr
  pop ax
  iret

global isr133
isr133:
  push ax
  mov al, 133
  call isr
  pop ax
  iret

global isr134
isr134:
  push ax
  mov al, 134
  call isr
  pop ax
  iret

global isr135
isr135:
  push ax
  mov al, 135
  call isr
  pop ax
  iret

global isr136
isr136:
  push ax
  mov al, 136
  call isr
  pop ax
  iret

global isr137
isr137:
  push ax
  mov al, 137
  call isr
  pop ax
  iret

global isr138
isr138:
  push ax
  mov al, 138
  call isr
  pop ax
  iret

global isr139
isr139:
  push ax
  mov al, 139
  call isr
  pop ax
  iret

global isr140
isr140:
  push ax
  mov al, 140
  call isr
  pop ax
  iret

global isr141
isr141:
  push ax
  mov al, 141
  call isr
  pop ax
  iret

global isr142
isr142:
  push ax
  mov al, 142
  call isr
  pop ax
  iret

global isr143
isr143:
  push ax
  mov al, 143
  call isr
  pop ax
  iret

global isr144
isr144:
  push ax
  mov al, 144
  call isr
  pop ax
  iret

global isr145
isr145:
  push ax
  mov al, 145
  call isr
  pop ax
  iret

global isr146
isr146:
  push ax
  mov al, 146
  call isr
  pop ax
  iret

global isr147
isr147:
  push ax
  mov al, 147
  call isr
  pop ax
  iret

global isr148
isr148:
  push ax
  mov al, 148
  call isr
  pop ax
  iret

global isr149
isr149:
  push ax
  mov al, 149
  call isr
  pop ax
  iret

global isr150
isr150:
  push ax
  mov al, 150
  call isr
  pop ax
  iret

global isr151
isr151:
  push ax
  mov al, 151
  call isr
  pop ax
  iret

global isr152
isr152:
  push ax
  mov al, 152
  call isr
  pop ax
  iret

global isr153
isr153:
  push ax
  mov al, 153
  call isr
  pop ax
  iret

global isr154
isr154:
  push ax
  mov al, 154
  call isr
  pop ax
  iret

global isr155
isr155:
  push ax
  mov al, 155
  call isr
  pop ax
  iret

global isr156
isr156:
  push ax
  mov al, 156
  call isr
  pop ax
  iret

global isr157
isr157:
  push ax
  mov al, 157
  call isr
  pop ax
  iret

global isr158
isr158:
  push ax
  mov al, 158
  call isr
  pop ax
  iret

global isr159
isr159:
  push ax
  mov al, 159
  call isr
  pop ax
  iret

global isr160
isr160:
  push ax
  mov al, 160
  call isr
  pop ax
  iret

global isr161
isr161:
  push ax
  mov al, 161
  call isr
  pop ax
  iret

global isr162
isr162:
  push ax
  mov al, 162
  call isr
  pop ax
  iret

global isr163
isr163:
  push ax
  mov al, 163
  call isr
  pop ax
  iret

global isr164
isr164:
  push ax
  mov al, 164
  call isr
  pop ax
  iret

global isr165
isr165:
  push ax
  mov al, 165
  call isr
  pop ax
  iret

global isr166
isr166:
  push ax
  mov al, 166
  call isr
  pop ax
  iret

global isr167
isr167:
  push ax
  mov al, 167
  call isr
  pop ax
  iret

global isr168
isr168:
  push ax
  mov al, 168
  call isr
  pop ax
  iret

global isr169
isr169:
  push ax
  mov al, 169
  call isr
  pop ax
  iret

global isr170
isr170:
  push ax
  mov al, 170
  call isr
  pop ax
  iret

global isr171
isr171:
  push ax
  mov al, 171
  call isr
  pop ax
  iret

global isr172
isr172:
  push ax
  mov al, 172
  call isr
  pop ax
  iret

global isr173
isr173:
  push ax
  mov al, 173
  call isr
  pop ax
  iret

global isr174
isr174:
  push ax
  mov al, 174
  call isr
  pop ax
  iret

global isr175
isr175:
  push ax
  mov al, 175
  call isr
  pop ax
  iret

global isr176
isr176:
  push ax
  mov al, 176
  call isr
  pop ax
  iret

global isr177
isr177:
  push ax
  mov al, 177
  call isr
  pop ax
  iret

global isr178
isr178:
  push ax
  mov al, 178
  call isr
  pop ax
  iret

global isr179
isr179:
  push ax
  mov al, 179
  call isr
  pop ax
  iret

global isr180
isr180:
  push ax
  mov al, 180
  call isr
  pop ax
  iret

global isr181
isr181:
  push ax
  mov al, 181
  call isr
  pop ax
  iret

global isr182
isr182:
  push ax
  mov al, 182
  call isr
  pop ax
  iret

global isr183
isr183:
  push ax
  mov al, 183
  call isr
  pop ax
  iret

global isr184
isr184:
  push ax
  mov al, 184
  call isr
  pop ax
  iret

global isr185
isr185:
  push ax
  mov al, 185
  call isr
  pop ax
  iret

global isr186
isr186:
  push ax
  mov al, 186
  call isr
  pop ax
  iret

global isr187
isr187:
  push ax
  mov al, 187
  call isr
  pop ax
  iret

global isr188
isr188:
  push ax
  mov al, 188
  call isr
  pop ax
  iret

global isr189
isr189:
  push ax
  mov al, 189
  call isr
  pop ax
  iret

global isr190
isr190:
  push ax
  mov al, 190
  call isr
  pop ax
  iret

global isr191
isr191:
  push ax
  mov al, 191
  call isr
  pop ax
  iret

global isr192
isr192:
  push ax
  mov al, 192
  call isr
  pop ax
  iret

global isr193
isr193:
  push ax
  mov al, 193
  call isr
  pop ax
  iret

global isr194
isr194:
  push ax
  mov al, 194
  call isr
  pop ax
  iret

global isr195
isr195:
  push ax
  mov al, 195
  call isr
  pop ax
  iret

global isr196
isr196:
  push ax
  mov al, 196
  call isr
  pop ax
  iret

global isr197
isr197:
  push ax
  mov al, 197
  call isr
  pop ax
  iret

global isr198
isr198:
  push ax
  mov al, 198
  call isr
  pop ax
  iret

global isr199
isr199:
  push ax
  mov al, 199
  call isr
  pop ax
  iret

global isr200
isr200:
  push ax
  mov al, 200
  call isr
  pop ax
  iret

global isr201
isr201:
  push ax
  mov al, 201
  call isr
  pop ax
  iret

global isr202
isr202:
  push ax
  mov al, 202
  call isr
  pop ax
  iret

global isr203
isr203:
  push ax
  mov al, 203
  call isr
  pop ax
  iret

global isr204
isr204:
  push ax
  mov al, 204
  call isr
  pop ax
  iret

global isr205
isr205:
  push ax
  mov al, 205
  call isr
  pop ax
  iret

global isr206
isr206:
  push ax
  mov al, 206
  call isr
  pop ax
  iret

global isr207
isr207:
  push ax
  mov al, 207
  call isr
  pop ax
  iret

global isr208
isr208:
  push ax
  mov al, 208
  call isr
  pop ax
  iret

global isr209
isr209:
  push ax
  mov al, 209
  call isr
  pop ax
  iret

global isr210
isr210:
  push ax
  mov al, 210
  call isr
  pop ax
  iret

global isr211
isr211:
  push ax
  mov al, 211
  call isr
  pop ax
  iret

global isr212
isr212:
  push ax
  mov al, 212
  call isr
  pop ax
  iret

global isr213
isr213:
  push ax
  mov al, 213
  call isr
  pop ax
  iret

global isr214
isr214:
  push ax
  mov al, 214
  call isr
  pop ax
  iret

global isr215
isr215:
  push ax
  mov al, 215
  call isr
  pop ax
  iret

global isr216
isr216:
  push ax
  mov al, 216
  call isr
  pop ax
  iret

global isr217
isr217:
  push ax
  mov al, 217
  call isr
  pop ax
  iret

global isr218
isr218:
  push ax
  mov al, 218
  call isr
  pop ax
  iret

global isr219
isr219:
  push ax
  mov al, 219
  call isr
  pop ax
  iret

global isr220
isr220:
  push ax
  mov al, 220
  call isr
  pop ax
  iret

global isr221
isr221:
  push ax
  mov al, 221
  call isr
  pop ax
  iret

global isr222
isr222:
  push ax
  mov al, 222
  call isr
  pop ax
  iret

global isr223
isr223:
  push ax
  mov al, 223
  call isr
  pop ax
  iret

global isr224
isr224:
  push ax
  mov al, 224
  call isr
  pop ax
  iret

global isr225
isr225:
  push ax
  mov al, 225
  call isr
  pop ax
  iret

global isr226
isr226:
  push ax
  mov al, 226
  call isr
  pop ax
  iret

global isr227
isr227:
  push ax
  mov al, 227
  call isr
  pop ax
  iret

global isr228
isr228:
  push ax
  mov al, 228
  call isr
  pop ax
  iret

global isr229
isr229:
  push ax
  mov al, 229
  call isr
  pop ax
  iret

global isr230
isr230:
  push ax
  mov al, 230
  call isr
  pop ax
  iret

global isr231
isr231:
  push ax
  mov al, 231
  call isr
  pop ax
  iret

global isr232
isr232:
  push ax
  mov al, 232
  call isr
  pop ax
  iret

global isr233
isr233:
  push ax
  mov al, 233
  call isr
  pop ax
  iret

global isr234
isr234:
  push ax
  mov al, 234
  call isr
  pop ax
  iret

global isr235
isr235:
  push ax
  mov al, 235
  call isr
  pop ax
  iret

global isr236
isr236:
  push ax
  mov al, 236
  call isr
  pop ax
  iret

global isr237
isr237:
  push ax
  mov al, 237
  call isr
  pop ax
  iret

global isr238
isr238:
  push ax
  mov al, 238
  call isr
  pop ax
  iret

global isr239
isr239:
  push ax
  mov al, 239
  call isr
  pop ax
  iret

global isr240
isr240:
  push ax
  mov al, 240
  call isr
  pop ax
  iret

global isr241
isr241:
  push ax
  mov al, 241
  call isr
  pop ax
  iret

global isr242
isr242:
  push ax
  mov al, 242
  call isr
  pop ax
  iret

global isr243
isr243:
  push ax
  mov al, 243
  call isr
  pop ax
  iret

global isr244
isr244:
  push ax
  mov al, 244
  call isr
  pop ax
  iret

global isr245
isr245:
  push ax
  mov al, 245
  call isr
  pop ax
  iret

global isr246
isr246:
  push ax
  mov al, 246
  call isr
  pop ax
  iret

global isr247
isr247:
  push ax
  mov al, 247
  call isr
  pop ax
  iret

global isr248
isr248:
  push ax
  mov al, 248
  call isr
  pop ax
  iret

global isr249
isr249:
  push ax
  mov al, 249
  call isr
  pop ax
  iret

global isr250
isr250:
  push ax
  mov al, 250
  call isr
  pop ax
  iret

global isr251
isr251:
  push ax
  mov al, 251
  call isr
  pop ax
  iret

global isr252
isr252:
  push ax
  mov al, 252
  call isr
  pop ax
  iret

global isr253
isr253:
  push ax
  mov al, 253
  call isr
  pop ax
  iret

global isr254
isr254:
  push ax
  mov al, 254
  call isr
  pop ax
  iret

global isr255
isr255:
  push ax
  mov al, 255
  call isr
  pop ax
  iret

