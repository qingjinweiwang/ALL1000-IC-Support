# M25PE40 template cleanup

User scope: retain shared code and overall AGD25Q40C driver architecture; remove unsupported features from the active M25PE40 .c/.h files.

Keep firmware ABI tables, pin mapping, power sequencing, SPI transport, main-array read/program/verify/blank-check/checksum, bulk erase, JEDEC ID comparison and BP2:BP0 protection. Do not replace the driver with the backup implementation.

Using the local Micron M25PE40 Rev. D (January 2018) datasheet, page 20 command table and pages 26–27 status layout, remove SR2/SR3, template OTP/extended-memory paths, four-byte addressing, WPSEL, 2Bh/2Fh security-register operations and 98h global unlock. The device's E8h/E5h sector lock registers are distinct and are not implemented by this cleanup.

Status operations use only SRAM byte 0 and mask 1Ch. Preserve the existing .spc control at SRAM byte 3 bit 7 for status verification/blank checking; status reads must not overwrite this control. Keep main-array loops and firmware entry ordering. Do not change the shared WEL helper or interrupt timeout behavior in this deletion-focused change.

Validation: review the patch against original source, scan for removed symbols and commands, compile with ARMCC and link with the existing ALL1000Code objects. Store build output locally without running the build script's external deployment step. Hardware validation remains required.
