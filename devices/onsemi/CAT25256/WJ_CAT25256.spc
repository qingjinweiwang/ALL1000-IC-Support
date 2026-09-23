
////////////////////////////////////////////////////////////////////////
// ON Semiconductor CAT25256VI-GT3(SOP8)
////////////////////////////////////////////////////////////////////////
[0]
.special.
GStatus Register Input (effective mask 0x8C):
G Value  BP1:BP0  WPEN  Effect
G  00       00      0   No array protection; WP pin function disabled
G  04       01      0   Protect 0x6000-0x7FFF; WP pin function disabled
G  08       10      0   Protect 0x4000-0x7FFF; WP pin function disabled
G  0C       11      0   Protect 0x0000-0x7FFF; WP pin function disabled
G  80       00      1   No array protection; WP Low locks Status Register
G  84       01      1   Protect 0x6000-0x7FFF; WP Low locks Status Register
G  88       10      1   Protect 0x4000-0x7FFF; WP Low locks Status Register
G  8C       11      1   Protect 0x0000-0x7FFF; WP Low locks Status Register
G When WPEN=1 and WP=High, Status Register remains writable after WREN.
G Only bits 7, 3 and 2 are effective; other input bits are masked out.
t0000L01 Status Register Value (Default 0x00)
