/* %W% */

struct doskeys kb_std_dos [] = {
    { 2, K_SHIFTTAB, DOSKEY | 3 },	/* 02 2 @ */
    { 15, K_SHIFTTAB, DOSKEY | 15 },	/* 15 TAB */
    { 26, K_SHIFTTAB, DOSKEY | 16 },	/* 16 ALT Q */
    { 27, K_SHIFTTAB, DOSKEY | 17 },	/* 17 ALT W */
    { 28, K_SHIFTTAB, DOSKEY | 18 },	/* 18 ALT E */
    { 29, K_SHIFTTAB, DOSKEY | 19 },	/* 19 ALT R */
    { 20, K_ALTTAB, DOSKEY | 20 },	/* 20 ALT T */
    { 21, K_ALTTAB, DOSKEY | 21 },	/* 21 ALT Y */
    { 22, K_ALTTAB, DOSKEY | 22 },	/* 22 ALT U */
    { 23, K_ALTTAB, DOSKEY | 23 },	/* 23 ALT I */
    { 24, K_ALTTAB, DOSKEY | 24 },	/* 24 ALT O */
    { 25, K_ALTTAB, DOSKEY | 25 },	/* 25 ALT P */

    { 30, K_ALTTAB, DOSKEY | 30 },	/* 30 ALT A */
    { 31, K_ALTTAB, DOSKEY | 31 },	/* 31 ALT S */
    { 32, K_ALTTAB, DOSKEY | 32 },	/* 32 ALT D */
    { 33, K_ALTTAB, DOSKEY | 33 },	/* 33 ALT F */
    { 34, K_ALTTAB, DOSKEY | 34 },	/* 34 ALT G */
    { 35, K_ALTTAB, DOSKEY | 35 },	/* 35 ALT H */
    { 36, K_ALTTAB, DOSKEY | 36 },	/* 36 ALT J */
    { 37, K_ALTTAB, DOSKEY | 37 },	/* 37 ALT K */
    { 38, K_ALTTAB, DOSKEY | 38 },	/* 38 ALT L */
    { 44, K_ALTTAB, DOSKEY | 44 },	/* 44 ALT Z */
    { 45, K_ALTTAB, DOSKEY | 45 },	/* 45 ALT X */
    { 46, K_ALTTAB, DOSKEY | 46 },	/* 46 ALT C */
    { 47, K_ALTTAB, DOSKEY | 47 },	/* 47 ALT V */
    { 48, K_ALTTAB, DOSKEY | 48 },	/* 48 ALT B */
    { 49, K_ALTTAB, DOSKEY | 49 },	/* 49 ALT N */
    { 50, K_ALTTAB, DOSKEY | 50 },	/* 50 ALT M */

    { 59, K_NORMTAB, CTLKEY | DOSKEY | 59 },	/* 59 F1 */
    { 59, K_CTRLTAB,		       94 },	/*  ^ F21 */
    { 60, K_NORMTAB, CTLKEY | DOSKEY | 60 },	/* 60 F2 */
    { 60, K_CTRLTAB,		       95 },	/*  ^ F22 */
    { 61, K_NORMTAB, CTLKEY | DOSKEY | 61 },	/* 61 F3 */
    { 61, K_CTRLTAB,		       96 },	/*  ^ F23 */
    { 62, K_NORMTAB, CTLKEY | DOSKEY | 62 },	/* 62 F4 */
    { 62, K_CTRLTAB,		       97 },	/*  ^ F24 */
    { 63, K_NORMTAB, CTLKEY | DOSKEY | 63 },	/* 63 F5 */
    { 63, K_CTRLTAB,		       98 },	/*  ^ F25 */
    { 64, K_NORMTAB, CTLKEY | DOSKEY | 64 },	/* 64 F6 */
    { 64, K_CTRLTAB,		       99 },	/*  ^ F26 */
    { 65, K_NORMTAB, CTLKEY | DOSKEY | 65 },	/* 65 F7 */
    { 65, K_CTRLTAB,		      100 },	/*  ^ F27 */
    { 66, K_NORMTAB, CTLKEY | DOSKEY | 66 },	/* 66 F8 */
    { 66, K_CTRLTAB,		      101 },	/*  ^ F28 */
    { 67, K_NORMTAB, CTLKEY | DOSKEY | 67 },	/* 67 F9 */
    { 67, K_CTRLTAB,		      102 },	/*  ^ F29 */
    { 68, K_NORMTAB, CTLKEY | DOSKEY | 68 },	/* 68 F10 */
    { 68, K_CTRLTAB,		      103 },	/*  ^ F30 */

    { 55, K_NORMTAB, NUMLCK | CTLKEY | DOSKEY | '*'},	/* 55 PRT SC * */
    { 55, K_CTRLTAB,			       114 },	/* ^  PRT SC */
    { 71, K_NORMTAB, NUMLCK | CTLKEY | DOSKEY | 71 },	/* 71 HOME */
    { 71, K_CTRLTAB,			       119 },	/* ^  HOME */
    { 72, K_NORMTAB, NUMLCK |          DOSKEY | 72 },	/* 72 UP */
    { 73, K_NORMTAB, NUMLCK | CTLKEY | DOSKEY | 73 },	/* 73 PG UP */
    { 73, K_CTRLTAB,			       132 },	/* ^  PG UP */
    { 75, K_NORMTAB, NUMLCK | CTLKEY | DOSKEY | 75 },	/* 75 LEFT */
    { 75, K_CTRLTAB,			       115 },	/* ^  LEFT */
    { 77, K_NORMTAB, NUMLCK | CTLKEY | DOSKEY | 77 },	/* 77 RIGHT */
    { 77, K_CTRLTAB,			       116 },	/* ^  RIGHT */
    { 79, K_NORMTAB, NUMLCK | CTLKEY | DOSKEY | 79 },	/* 79 END */
    { 79, K_CTRLTAB,			       117 },	/* ^  END */
    { 80, K_NORMTAB, NUMLCK |          DOSKEY | 80 },	/* 80 DOWN */
    { 81, K_NORMTAB, NUMLCK | CTLKEY | DOSKEY | 81 },	/* 81 PG DN */
    { 81, K_CTRLTAB,			       118 },	/* ^  PG DN */
    { 82, K_NORMTAB, NUMLCK |          DOSKEY | 82 },	/* 82 INS */
    { 83, K_NORMTAB, NUMLCK |          DOSKEY | 83 },	/* 83 DEL */

    { 59, K_SHIFTTAB, DOSKEY | 84 },		/* 84 (SHIFT) F11 */
    { 60, K_SHIFTTAB, DOSKEY | 85 },		/* 85 (SHIFT) F12 */
    { 61, K_SHIFTTAB, DOSKEY | 86 },		/* 86 (SHIFT) F13 */
    { 62, K_SHIFTTAB, DOSKEY | 87 },		/* 87 (SHIFT) F14 */
    { 63, K_SHIFTTAB, DOSKEY | 88 },		/* 88 (SHIFT) F15 */
    { 64, K_SHIFTTAB, DOSKEY | 89 },		/* 89 (SHIFT) F16 */
    { 65, K_SHIFTTAB, DOSKEY | 90 },		/* 90 (SHIFT) F17 */
    { 66, K_SHIFTTAB, DOSKEY | 91 },		/* 91 (SHIFT) F18 */
    { 67, K_SHIFTTAB, DOSKEY | 92 },		/* 92 (SHIFT) F19 */
    { 68, K_SHIFTTAB, DOSKEY | 93 },		/* 93 (SHIFT) F20 */

    { 59, K_ALTTAB, DOSKEY | 104 },	/* 104 (ALT) F31 */
    { 60, K_ALTTAB, DOSKEY | 105 },	/* 105 (ALT) F32 */
    { 61, K_ALTTAB, DOSKEY | 106 },	/* 106 (ALT) F33 */
    { 62, K_ALTTAB, DOSKEY | 107 },	/* 107 (ALT) F34 */
    { 63, K_ALTTAB, DOSKEY | 108 },	/* 108 (ALT) F35 */
    { 64, K_ALTTAB, DOSKEY | 109 },	/* 109 (ALT) F36 */
    { 65, K_ALTTAB, DOSKEY | 110 },	/* 110 (ALT) F37 */
    { 66, K_ALTTAB, DOSKEY | 111 },	/* 111 (ALT) F38 */
    { 67, K_ALTTAB, DOSKEY | 112 },	/* 112 (ALT) F39 */
    { 68, K_ALTTAB, DOSKEY | 113 },	/* 113 (ALT) F40 */

    { 2, K_ALTTAB, DOSKEY | 120 },	/* 2 ALT 1 */
    { 3, K_ALTTAB, DOSKEY | 121 },	/* 3 ALT 2 */
    { 4, K_ALTTAB, DOSKEY | 122 },	/* 4 ALT 3 */
    { 5, K_ALTTAB, DOSKEY | 123 },	/* 5 ALT 4 */
    { 6, K_ALTTAB, DOSKEY | 124 },	/* 6 ALT 5 */
    { 7, K_ALTTAB, DOSKEY | 125 },	/* 7 ALT 6 */
    { 8, K_ALTTAB, DOSKEY | 126 },	/* 8 ALT 7 */
    { 9, K_ALTTAB, DOSKEY | 127 },	/* 9 ALT 8 */
    { 10, K_ALTTAB, DOSKEY | 128 },	/* 10 ALT 9 */
    { 11, K_ALTTAB, DOSKEY | 129 },	/* 11 ALT 0 */
    { 12, K_ALTTAB, DOSKEY | 130 },	/* 12 ALT - */
    { 13, K_ALTTAB, DOSKEY | 131 },	/* 13 ALT = */
    { 0, 0, 0 },
};

