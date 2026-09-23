////////////////////////////////////////////////////////////////////////
// MICRON N25Q256Ax3ESFxxx
////////////////////////////////////////////////////////////////////////

[0]
.special.

GMessage :
G<<PROTECTION OPTIONS>>

S0000L0100 STATUS REGISTER :
E04,04 BP0 = 1
E08,08 BP1 = 1
E10,10 BP2 = 1
E40,40 BP3 = 1
E20,20 TB = Bottom


G<<OTP OPERATION OPTIONS>>

S0003L0100 OTP OPERATION OPTIONS :
E01,01 Enable OTP Program
E02,02 Enable OTP Verify
E08,08 Enable OTP Blank Check
E10,10 Enable OTP Checksum Calculate


G<<IMPORTANT NOTICE>>

G BP3:BP0 PROTECTION SIZE:
G 0000 = None
G 0001 = 64KB
G 0010 = 128KB
G 0011 = 256KB
G 0100 = 512KB
G 0101 = 1MB
G 0110 = 2MB
G 0111 = 4MB
G 1000 = 8MB
G 1001 = 16MB
G 1010-1111 = Full 32MB

G TB=0 protects from Top.
G TB=1 protects from Bottom.

G OTP DATA VIRTUAL ADDRESS: 02000000h-0200003Fh
G OTP CONTROL ADDRESS:      02000040h
G FFh = Keep OTP Unlocked
G FEh = Permanently Lock OTP
G OTP cannot be erased after programming.
G OTP lock is irreversible.
