# M25PE40 Block Table Design

## Goal

Replace the TH25Q80HB-derived memory layout in `Block_M25PE40VMPxxx[]` with the M25PE40 physical sector layout while preserving the existing table format, indentation, and comment style.

## Layout

M25PE40 contains 512 KiB from `00000h` through `7FFFFh`, organized as eight 64 KiB sectors. Define `Noun_Block0` through `Noun_Block7` as Sector 0 through Sector 7, with contiguous RAM and device ranges. Do not retain TH25Q security-register entries because M25PE40 does not provide them.

Keep the existing special Status Register and driver-operation-control records. Move the Status Register noun to `Noun_Block8` and update the three status-block comparisons in the C driver from `Noun_Block4` to `Noun_Block8` so the table and algorithm agree.

Do not enable `MainSectorEraseOn` because the current `EraseFun()` implements bulk erase rather than addressed D8h sector erase.

## Verification

Compile with `iBuild.bat WJx4_M25PE40-VMPxxx`. The build must remain warning-free and update the BIN/SPC outputs successfully.
