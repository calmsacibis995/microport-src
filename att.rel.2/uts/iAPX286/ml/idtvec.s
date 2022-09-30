	.file	"idt.s"
/
/ @(#)idtvec.s	1.10
/

#include	"../sys/mmu.h"

/
/	Interrupt vector jump table
/
	.text

i0:	push	$0
	ljmp	trp
i1:	push	$1
	ljmp	trp
i2:	push	$2
	ljmp	trp
i3:	push	$3
	ljmp	trp
i4:	push	$4
	ljmp	trp
i5:	push	$5
	ljmp	trp
i6:	push	$6
	ljmp	trp
i7:	push	$7
	ljmp	trp
i8:	push	$8
	ljmp	fault
i9:	push	$9
	ljmp	trp
i10:	push	$10
	ljmp	fault
i11:	push	$11
	ljmp	fault
i12:	push	$12
	ljmp	fault
i13:	push	$13
	ljmp	fault
i14:	push	$14
	ljmp	trp
i15:	push	$15
	ljmp	trp
i16:	push	$16
	ljmp	trp
i17:	push	$17
	ljmp	trp
i18:	push	$18
	ljmp	trp
i19:	push	$19
	ljmp	trp
i20:	push	$20
	ljmp	trp
i21:	push	$21
	ljmp	trp
i22:	push	$22
	ljmp	trp
i23:	push	$23
	ljmp	trp
i24:	push	$24
	ljmp	trp
i25:	push	$25
	ljmp	trp
i26:	push	$26
	ljmp	trp
i27:	push	$27
	ljmp	trp
i28:	push	$28
	ljmp	trp
i29:	push	$29
	ljmp	trp
i30:	push	$30
	ljmp	trp
i31:	push	$31
	ljmp	trp
i32:	push	$32
	ljmp	intr
i33:	push	$33
	ljmp	trp
i34:	push	$34
	ljmp	intr
i35:	push	$35
	ljmp	intr
i36:	push	$36
	ljmp	intr
i37:	push	$37
	ljmp	intr
i38:	push	$38
	ljmp	intr
i39:	push	$39
	ljmp	intr
i40:	push	$40
	ljmp	intr
i41:	push	$41
	ljmp	intr
i42:	push	$42
	ljmp	intr
i43:	push	$43
	ljmp	intr
i44:	push	$44
	ljmp	intr
i45:	push	$45
	ljmp	intr
i46:	push	$46
	ljmp	intr
i47:	push	$47
	ljmp	intr
i48:	push	$48
	ljmp	intr
i49:	push	$49
	ljmp	intr
i50:	push	$50
	ljmp	intr
i51:	push	$51
	ljmp	intr
i52:	push	$52
	ljmp	intr
i53:	push	$53
	ljmp	intr
i54:	push	$54
	ljmp	intr
i55:	push	$55
	ljmp	intr
i56:	push	$56
	ljmp	intr
i57:	push	$57
	ljmp	intr
i58:	push	$58
	ljmp	intr
i59:	push	$59
	ljmp	intr
i60:	push	$60
	ljmp	intr
i61:	push	$61
	ljmp	intr
i62:	push	$62
	ljmp	intr
i63:	push	$63
	ljmp	intr
i64:	push	$64
	ljmp	intr
i65:	push	$65
	ljmp	intr
i66:	push	$66
	ljmp	intr
i67:	push	$67
	ljmp	intr
i68:	push	$68
	ljmp	intr
i69:	push	$69
	ljmp	intr
i70:	push	$70
	ljmp	intr
i71:	push	$71
	ljmp	intr
i72:	push	$72
	ljmp	intr
i73:	push	$73
	ljmp	intr
i74:	push	$74
	ljmp	intr
i75:	push	$75
	ljmp	intr
i76:	push	$76
	ljmp	intr
i77:	push	$77
	ljmp	intr
i78:	push	$78
	ljmp	intr
i79:	push	$79
	ljmp	intr
i80:	push	$80
	ljmp	intr
i81:	push	$81
	ljmp	intr
i82:	push	$82
	ljmp	intr
i83:	push	$83
	ljmp	intr
i84:	push	$84
	ljmp	intr
i85:	push	$85
	ljmp	intr
i86:	push	$86
	ljmp	intr
i87:	push	$87
	ljmp	intr
i88:	push	$88
	ljmp	intr
i89:	push	$89
	ljmp	intr
i90:	push	$90
	ljmp	intr
i91:	push	$91
	ljmp	intr
i92:	push	$92
	ljmp	intr
i93:	push	$93
	ljmp	intr
i94:	push	$94
	ljmp	intr
i95:	push	$95
	ljmp	intr
i96:	push	$96
	ljmp	intr
i97:	push	$97
	ljmp	intr
i98:	push	$98
	ljmp	intr
i99:	push	$99
	ljmp	intr
i100:	push	$100
	ljmp	intr
i101:	push	$101
	ljmp	intr
i102:	push	$102
	ljmp	intr
i103:	push	$103
	ljmp	intr
i104:	push	$104
	ljmp	intr
i105:	push	$105
	ljmp	intr
i106:	push	$106
	ljmp	intr
i107:	push	$107
	ljmp	intr
i108:	push	$108
	ljmp	intr
i109:	push	$109
	ljmp	intr
i110:	push	$110
	ljmp	intr
i111:	push	$111
	ljmp	intr
i112:	push	$112
	ljmp	intr
i113:	push	$113
	ljmp	intr
i114:	push	$114
	ljmp	intr
i115:	push	$115
	ljmp	intr
i116:	push	$116
	ljmp	intr
i117:	push	$117
	ljmp	intr
i118:	push	$118
	ljmp	intr
i119:	push	$119
	ljmp	intr
i120:	push	$120
	ljmp	intr
i121:	push	$121
	ljmp	intr
i122:	push	$122
	ljmp	intr
i123:	push	$123
	ljmp	intr
i124:	push	$124
	ljmp	intr
i125:	push	$125
	ljmp	intr
i126:	push	$126
	ljmp	intr
i127:	push	$127
	ljmp	intr
i128:	push	$128
	ljmp	intr
i129:	push	$129
	ljmp	intr
i130:	push	$130
	ljmp	intr
i131:	push	$131
	ljmp	intr
i132:	push	$132
	ljmp	intr
i133:	push	$133
	ljmp	intr
i134:	push	$134
	ljmp	intr
i135:	push	$135
	ljmp	intr
i136:	push	$136
	ljmp	intr
i137:	push	$137
	ljmp	intr
i138:	push	$138
	ljmp	intr
i139:	push	$139
	ljmp	intr
i140:	push	$140
	ljmp	intr
i141:	push	$141
	ljmp	intr
i142:	push	$142
	ljmp	intr
i143:	push	$143
	ljmp	intr
i144:	push	$144
	ljmp	intr
i145:	push	$145
	ljmp	intr
i146:	push	$146
	ljmp	intr
i147:	push	$147
	ljmp	intr
i148:	push	$148
	ljmp	intr
i149:	push	$149
	ljmp	intr
i150:	push	$150
	ljmp	intr
i151:	push	$151
	ljmp	intr
i152:	push	$152
	ljmp	intr
i153:	push	$153
	ljmp	intr
i154:	push	$154
	ljmp	intr
i155:	push	$155
	ljmp	intr
i156:	push	$156
	ljmp	intr
i157:	push	$157
	ljmp	intr
i158:	push	$158
	ljmp	intr
i159:	push	$159
	ljmp	intr
i160:	push	$160
	ljmp	intr
i161:	push	$161
	ljmp	intr
i162:	push	$162
	ljmp	intr
i163:	push	$163
	ljmp	intr
i164:	push	$164
	ljmp	intr
i165:	push	$165
	ljmp	intr
i166:	push	$166
	ljmp	intr
i167:	push	$167
	ljmp	intr
i168:	push	$168
	ljmp	intr
i169:	push	$169
	ljmp	intr
i170:	push	$170
	ljmp	intr
i171:	push	$171
	ljmp	intr
i172:	push	$172
	ljmp	intr
i173:	push	$173
	ljmp	intr
i174:	push	$174
	ljmp	intr
i175:	push	$175
	ljmp	intr
i176:	push	$176
	ljmp	intr
i177:	push	$177
	ljmp	intr
i178:	push	$178
	ljmp	intr
i179:	push	$179
	ljmp	intr
i180:	push	$180
	ljmp	intr
i181:	push	$181
	ljmp	intr
i182:	push	$182
	ljmp	intr
i183:	push	$183
	ljmp	intr
i184:	push	$184
	ljmp	intr
i185:	push	$185
	ljmp	intr
i186:	push	$186
	ljmp	intr
i187:	push	$187
	ljmp	intr
i188:	push	$188
	ljmp	intr
i189:	push	$189
	ljmp	intr
i190:	push	$190
	ljmp	intr
i191:	push	$191
	ljmp	intr
i192:	push	$192
	ljmp	intr
i193:	push	$193
	ljmp	intr
i194:	push	$194
	ljmp	intr
i195:	push	$195
	ljmp	intr
i196:	push	$196
	ljmp	intr
i197:	push	$197
	ljmp	intr
i198:	push	$198
	ljmp	intr
i199:	push	$199
	ljmp	intr
i200:	push	$200
	ljmp	intr
i201:	push	$201
	ljmp	intr
i202:	push	$202
	ljmp	intr
i203:	push	$203
	ljmp	intr
i204:	push	$204
	ljmp	intr
i205:	push	$205
	ljmp	intr
i206:	push	$206
	ljmp	intr
i207:	push	$207
	ljmp	intr
i208:	push	$208
	ljmp	intr
i209:	push	$209
	ljmp	intr
i210:	push	$210
	ljmp	intr
i211:	push	$211
	ljmp	intr
i212:	push	$212
	ljmp	intr
i213:	push	$213
	ljmp	intr
i214:	push	$214
	ljmp	intr
i215:	push	$215
	ljmp	intr
i216:	push	$216
	ljmp	intr
i217:	push	$217
	ljmp	intr
i218:	push	$218
	ljmp	intr
i219:	push	$219
	ljmp	intr
i220:	push	$220
	ljmp	intr
i221:	push	$221
	ljmp	intr
i222:	push	$222
	ljmp	intr
i223:	push	$223
	ljmp	intr
i224:	push	$224
	ljmp	intr
i225:	push	$225
	ljmp	intr
i226:	push	$226
	ljmp	intr
i227:	push	$227
	ljmp	intr
i228:	push	$228
	ljmp	intr
i229:	push	$229
	ljmp	intr
i230:	push	$230
	ljmp	intr
i231:	push	$231
	ljmp	intr
i232:	push	$232
	ljmp	intr
i233:	push	$233
	ljmp	intr
i234:	push	$234
	ljmp	intr
i235:	push	$235
	ljmp	intr
i236:	push	$236
	ljmp	intr
i237:	push	$237
	ljmp	intr
i238:	push	$238
	ljmp	intr
i239:	push	$239
	ljmp	intr
i240:	push	$240
	ljmp	intr
i241:	push	$241
	ljmp	intr
i242:	push	$242
	ljmp	intr
i243:	push	$243
	ljmp	intr
i244:	push	$244
	ljmp	intr
i245:	push	$245
	ljmp	intr
i246:	push	$246
	ljmp	intr
i247:	push	$247
	ljmp	intr
i248:	push	$248
	ljmp	intr
i249:	push	$249
	ljmp	intr
i250:	push	$250
	ljmp	intr
i251:	push	$251
	ljmp	intr
i252:	push	$252
	ljmp	intr
i253:	push	$253
	ljmp	intr
i254:	push	$254
	ljmp	intr
i255:	push	$255
	ljmp	intr


/
/	The Interrupt Descriptor Table. All interrupt gates are defined here
/
/	format:
/		.value	destoff		( destination offset )
/		.value	destsel		( destination selector )
/		.byte	wc		( word count - not used for int gate )
/		.byte	access
/		.value	reserved
/
	.data

	.globl	idt_base
idt_base:
	.value	i0			/ Interrupt # 0 - Divide exception
	.value	<s>i0
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i1			/ Interrupt # 1 - Single step
	.value	<s>i1
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i2			/ Interrupt # 2 - NMI
	.value	<s>i2
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i3			/ Interrupt # 3 - Breakpoint
	.value	<s>i3
	.byte	0
	.byte	DSC_PRESENT | G_INT | USR_DPL
	.value	0

	.value	i4			/ Interrupt # 4 - INTO
	.value	<s>i4
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i5			/ Interrupt # 5 - BOUND
	.value	<s>i5
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i6			/ Interrupt # 6 - Invalid opcode
	.value	<s>i6
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i7			/ Interrupt # 7 - Extension not avail
	.value	<s>i7
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	0			/ Interrupt # 8 - Double exception
	.value	DFLTSEL << 3
	.byte	0
	.byte	DSC_PRESENT | G_TASK
	.value	0

	.value	i9			/ Interrupt # 9 - Extension seg overrun
	.value	<s>i9
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	0			/ Interrupt # 10 - Invalid TSS
	.value	TFLTSEL << 3
	.byte	0
	.byte	DSC_PRESENT | G_TASK
	.value	0

	.value	i11			/ Interrupt # 11 - Segment not present
	.value	<s>i11
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i12			/ Interrupt # 12 - Stack fault
	.value	<s>i12
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i13			/ Interrupt # 13 - General Protection
	.value	<s>i13
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i14			/ Interrupt # 14 - Reserved
	.value	<s>i14
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i15			/ Interrupt # 15 - Reserved
	.value	<s>i15
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i16			/ Interrupt # 16 - Reserved
	.value	<s>i16
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i17			/ Interrupt # 17 - Reserved
	.value	<s>i17
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i18			/ Interrupt # 18 - Reserved
	.value	<s>i18
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i19			/ Interrupt # 19 - Reserved
	.value	<s>i19
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i20			/ Interrupt # 20 - Reserved
	.value	<s>i20
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i21			/ Interrupt # 21 - Reserved
	.value	<s>i21
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i22			/ Interrupt # 22 - Reserved
	.value	<s>i22
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i23			/ Interrupt # 23 - Reserved
	.value	<s>i23
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i24			/ Interrupt # 24 - Reserved
	.value	<s>i24
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i25			/ Interrupt # 25 - Reserved
	.value	<s>i25
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i26			/ Interrupt # 26 - Reserved
	.value	<s>i26
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i27			/ Interrupt # 27 - Reserved
	.value	<s>i27
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i28			/ Interrupt # 28 - Reserved
	.value	<s>i28
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i29			/ Interrupt # 29 - Reserved
	.value	<s>i29
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i30			/ Interrupt # 30 - Reserved
	.value	<s>i30
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i31			/ Interrupt # 31 - Reserved
	.value	<s>i31
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i32			/ Interrupt # 32
	.value	<s>i32
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i33			/ Interrupt # 33 - Front panel button
	.value	WNDSEL << 3
	.byte	0
	.byte	DSC_PRESENT | G_TASK
	.value	0

	.value	i34			/ Interrupt # 34
	.value	<s>i34
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i35			/ Interrupt # 35
	.value	<s>i35
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i36			/ Interrupt # 36
	.value	<s>i36
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i37			/ Interrupt # 37
	.value	<s>i37
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i38			/ Interrupt # 38
	.value	<s>i38
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i39			/ Interrupt # 39
	.value	<s>i39
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i40			/ Interrupt # 40
	.value	<s>i40
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i41			/ Interrupt # 41
	.value	<s>i41
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i42			/ Interrupt # 42
	.value	<s>i42
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i43			/ Interrupt # 43
	.value	<s>i43
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i44			/ Interrupt # 44
	.value	<s>i44
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i45			/ Interrupt # 45
	.value	<s>i45
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i46			/ Interrupt # 46
	.value	<s>i46
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i47			/ Interrupt # 47
	.value	<s>i47
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i48			/ Interrupt # 48
	.value	<s>i48
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i49			/ Interrupt # 49
	.value	<s>i49
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i50			/ Interrupt # 50
	.value	<s>i50
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i51			/ Interrupt # 51
	.value	<s>i51
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i52			/ Interrupt # 52
	.value	<s>i52
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i53			/ Interrupt # 53
	.value	<s>i53
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i54			/ Interrupt # 54
	.value	<s>i54
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i55			/ Interrupt # 55
	.value	<s>i55
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i56			/ Interrupt # 56
	.value	<s>i56
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i57			/ Interrupt # 57
	.value	<s>i57
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i58			/ Interrupt # 58
	.value	<s>i58
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i59			/ Interrupt # 59
	.value	<s>i59
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i60			/ Interrupt # 60
	.value	<s>i60
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i61			/ Interrupt # 61
	.value	<s>i61
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i62			/ Interrupt # 62
	.value	<s>i62
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i63			/ Interrupt # 63
	.value	<s>i63
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i64			/ Interrupt # 64
	.value	<s>i64
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i65			/ Interrupt # 65
	.value	<s>i65
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i66			/ Interrupt # 66
	.value	<s>i66
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i67			/ Interrupt # 67
	.value	<s>i67
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i68			/ Interrupt # 68
	.value	<s>i68
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i69			/ Interrupt # 69
	.value	<s>i69
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i70			/ Interrupt # 70
	.value	<s>i70
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i71			/ Interrupt # 71
	.value	<s>i71
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i72			/ Interrupt # 72
	.value	<s>i72
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i73			/ Interrupt # 73
	.value	<s>i73
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i74			/ Interrupt # 74
	.value	<s>i74
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i75			/ Interrupt # 75
	.value	<s>i75
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i76			/ Interrupt # 76
	.value	<s>i76
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i77			/ Interrupt # 77
	.value	<s>i77
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i78			/ Interrupt # 78
	.value	<s>i78
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i79			/ Interrupt # 79
	.value	<s>i79
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i80			/ Interrupt # 80
	.value	<s>i80
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i81			/ Interrupt # 81
	.value	<s>i81
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i82			/ Interrupt # 82
	.value	<s>i82
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i83			/ Interrupt # 83
	.value	<s>i83
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i84			/ Interrupt # 84
	.value	<s>i84
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i85			/ Interrupt # 85
	.value	<s>i85
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i86			/ Interrupt # 86
	.value	<s>i86
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i87			/ Interrupt # 87
	.value	<s>i87
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i88			/ Interrupt # 88
	.value	<s>i88
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i89			/ Interrupt # 89
	.value	<s>i89
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i90			/ Interrupt # 90
	.value	<s>i90
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i91			/ Interrupt # 91
	.value	<s>i91
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i92			/ Interrupt # 92
	.value	<s>i92
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i93			/ Interrupt # 93
	.value	<s>i93
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i94			/ Interrupt # 94
	.value	<s>i94
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i95			/ Interrupt # 95
	.value	<s>i95
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i96			/ Interrupt # 96
	.value	<s>i96
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i97			/ Interrupt # 97
	.value	<s>i97
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i98			/ Interrupt # 98
	.value	<s>i98
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i99			/ Interrupt # 99
	.value	<s>i99
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i100			/ Interrupt # 100
	.value	<s>i100
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i101			/ Interrupt # 101
	.value	<s>i101
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i102			/ Interrupt # 102
	.value	<s>i102
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i103			/ Interrupt # 103
	.value	<s>i103
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i104			/ Interrupt # 104
	.value	<s>i104
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i105			/ Interrupt # 105
	.value	<s>i105
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i106			/ Interrupt # 106
	.value	<s>i106
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i107			/ Interrupt # 107
	.value	<s>i107
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i108			/ Interrupt # 108
	.value	<s>i108
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i109			/ Interrupt # 109
	.value	<s>i109
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i110			/ Interrupt # 110
	.value	<s>i110
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i111			/ Interrupt # 111
	.value	<s>i111
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i112			/ Interrupt # 112
	.value	<s>i112
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i113			/ Interrupt # 113
	.value	<s>i113
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i114			/ Interrupt # 114
	.value	<s>i114
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i115			/ Interrupt # 115
	.value	<s>i115
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i116			/ Interrupt # 116
	.value	<s>i116
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i117			/ Interrupt # 117
	.value	<s>i117
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i118			/ Interrupt # 118
	.value	<s>i118
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i119			/ Interrupt # 119
	.value	<s>i119
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i120			/ Interrupt # 120
	.value	<s>i120
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i121			/ Interrupt # 121
	.value	<s>i121
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i122			/ Interrupt # 122
	.value	<s>i122
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i123			/ Interrupt # 123
	.value	<s>i123
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i124			/ Interrupt # 124
	.value	<s>i124
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i125			/ Interrupt # 125
	.value	<s>i125
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i126			/ Interrupt # 126
	.value	<s>i126
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i127			/ Interrupt # 127
	.value	<s>i127
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i128			/ Interrupt # 128
	.value	<s>i128
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i129			/ Interrupt # 129
	.value	<s>i129
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i130			/ Interrupt # 130
	.value	<s>i130
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i131			/ Interrupt # 131
	.value	<s>i131
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i132			/ Interrupt # 132
	.value	<s>i132
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i133			/ Interrupt # 133
	.value	<s>i133
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i134			/ Interrupt # 134
	.value	<s>i134
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i135			/ Interrupt # 135
	.value	<s>i135
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i136			/ Interrupt # 136
	.value	<s>i136
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i137			/ Interrupt # 137
	.value	<s>i137
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i138			/ Interrupt # 138
	.value	<s>i138
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i139			/ Interrupt # 139
	.value	<s>i139
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i140			/ Interrupt # 140
	.value	<s>i140
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i141			/ Interrupt # 141
	.value	<s>i141
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i142			/ Interrupt # 142
	.value	<s>i142
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i143			/ Interrupt # 143
	.value	<s>i143
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i144			/ Interrupt # 144
	.value	<s>i144
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i145			/ Interrupt # 145
	.value	<s>i145
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i146			/ Interrupt # 146
	.value	<s>i146
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i147			/ Interrupt # 147
	.value	<s>i147
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i148			/ Interrupt # 148
	.value	<s>i148
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i149			/ Interrupt # 149
	.value	<s>i149
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i150			/ Interrupt # 150
	.value	<s>i150
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i151			/ Interrupt # 151
	.value	<s>i151
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i152			/ Interrupt # 152
	.value	<s>i152
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i153			/ Interrupt # 153
	.value	<s>i153
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i154			/ Interrupt # 154
	.value	<s>i154
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i155			/ Interrupt # 155
	.value	<s>i155
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i156			/ Interrupt # 156
	.value	<s>i156
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i157			/ Interrupt # 157
	.value	<s>i157
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i158			/ Interrupt # 158
	.value	<s>i158
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i159			/ Interrupt # 159
	.value	<s>i159
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i160			/ Interrupt # 160
	.value	<s>i160
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i161			/ Interrupt # 161
	.value	<s>i161
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i162			/ Interrupt # 162
	.value	<s>i162
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i163			/ Interrupt # 163
	.value	<s>i163
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i164			/ Interrupt # 164
	.value	<s>i164
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i165			/ Interrupt # 165
	.value	<s>i165
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i166			/ Interrupt # 166
	.value	<s>i166
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i167			/ Interrupt # 167
	.value	<s>i167
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i168			/ Interrupt # 168
	.value	<s>i168
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i169			/ Interrupt # 169
	.value	<s>i169
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i170			/ Interrupt # 170
	.value	<s>i170
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i171			/ Interrupt # 171
	.value	<s>i171
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i172			/ Interrupt # 172
	.value	<s>i172
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i173			/ Interrupt # 173
	.value	<s>i173
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i174			/ Interrupt # 174
	.value	<s>i174
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i175			/ Interrupt # 175
	.value	<s>i175
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i176			/ Interrupt # 176
	.value	<s>i176
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i177			/ Interrupt # 177
	.value	<s>i177
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i178			/ Interrupt # 178
	.value	<s>i178
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i179			/ Interrupt # 179
	.value	<s>i179
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i180			/ Interrupt # 180
	.value	<s>i180
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i181			/ Interrupt # 181
	.value	<s>i181
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i182			/ Interrupt # 182
	.value	<s>i182
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i183			/ Interrupt # 183
	.value	<s>i183
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i184			/ Interrupt # 184
	.value	<s>i184
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i185			/ Interrupt # 185
	.value	<s>i185
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i186			/ Interrupt # 186
	.value	<s>i186
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i187			/ Interrupt # 187
	.value	<s>i187
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i188			/ Interrupt # 188
	.value	<s>i188
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i189			/ Interrupt # 189
	.value	<s>i189
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i190			/ Interrupt # 190
	.value	<s>i190
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i191			/ Interrupt # 191
	.value	<s>i191
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i192			/ Interrupt # 192
	.value	<s>i192
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i193			/ Interrupt # 193
	.value	<s>i193
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i194			/ Interrupt # 194
	.value	<s>i194
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i195			/ Interrupt # 195
	.value	<s>i195
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i196			/ Interrupt # 196
	.value	<s>i196
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i197			/ Interrupt # 197
	.value	<s>i197
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i198			/ Interrupt # 198
	.value	<s>i198
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i199			/ Interrupt # 199
	.value	<s>i199
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i200			/ Interrupt # 200
	.value	<s>i200
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i201			/ Interrupt # 201
	.value	<s>i201
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i202			/ Interrupt # 202
	.value	<s>i202
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i203			/ Interrupt # 203
	.value	<s>i203
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i204			/ Interrupt # 204
	.value	<s>i204
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i205			/ Interrupt # 205
	.value	<s>i205
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i206			/ Interrupt # 206
	.value	<s>i206
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i207			/ Interrupt # 207
	.value	<s>i207
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i208			/ Interrupt # 208
	.value	<s>i208
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i209			/ Interrupt # 209
	.value	<s>i209
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i210			/ Interrupt # 210
	.value	<s>i210
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i211			/ Interrupt # 211
	.value	<s>i211
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i212			/ Interrupt # 212
	.value	<s>i212
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i213			/ Interrupt # 213
	.value	<s>i213
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i214			/ Interrupt # 214
	.value	<s>i214
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i215			/ Interrupt # 215
	.value	<s>i215
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i216			/ Interrupt # 216
	.value	<s>i216
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i217			/ Interrupt # 217
	.value	<s>i217
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i218			/ Interrupt # 218
	.value	<s>i218
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i219			/ Interrupt # 219
	.value	<s>i219
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i220			/ Interrupt # 220
	.value	<s>i220
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i221			/ Interrupt # 221
	.value	<s>i221
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i222			/ Interrupt # 222
	.value	<s>i222
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i223			/ Interrupt # 223
	.value	<s>i223
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i224			/ Interrupt # 224
	.value	<s>i224
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i225			/ Interrupt # 225
	.value	<s>i225
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i226			/ Interrupt # 226
	.value	<s>i226
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i227			/ Interrupt # 227
	.value	<s>i227
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i228			/ Interrupt # 228
	.value	<s>i228
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i229			/ Interrupt # 229
	.value	<s>i229
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i230			/ Interrupt # 230
	.value	<s>i230
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i231			/ Interrupt # 231
	.value	<s>i231
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i232			/ Interrupt # 232
	.value	<s>i232
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i233			/ Interrupt # 233
	.value	<s>i233
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i234			/ Interrupt # 234
	.value	<s>i234
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i235			/ Interrupt # 235
	.value	<s>i235
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i236			/ Interrupt # 236
	.value	<s>i236
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i237			/ Interrupt # 237
	.value	<s>i237
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i238			/ Interrupt # 238
	.value	<s>i238
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i239			/ Interrupt # 239
	.value	<s>i239
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i240			/ Interrupt # 240
	.value	<s>i240
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i241			/ Interrupt # 241
	.value	<s>i241
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i242			/ Interrupt # 242
	.value	<s>i242
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i243			/ Interrupt # 243
	.value	<s>i243
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i244			/ Interrupt # 244
	.value	<s>i244
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i245			/ Interrupt # 245
	.value	<s>i245
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i246			/ Interrupt # 246
	.value	<s>i246
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i247			/ Interrupt # 247
	.value	<s>i247
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i248			/ Interrupt # 248
	.value	<s>i248
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i249			/ Interrupt # 249
	.value	<s>i249
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i250			/ Interrupt # 250
	.value	<s>i250
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i251			/ Interrupt # 251
	.value	<s>i251
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i252			/ Interrupt # 252
	.value	<s>i252
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i253			/ Interrupt # 253
	.value	<s>i253
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i254			/ Interrupt # 254
	.value	<s>i254
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

	.value	i255			/ Interrupt # 255
	.value	<s>i255
	.byte	0
	.byte	DSC_PRESENT | G_INT
	.value	0

