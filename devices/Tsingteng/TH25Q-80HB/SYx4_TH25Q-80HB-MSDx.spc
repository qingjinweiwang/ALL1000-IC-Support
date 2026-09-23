////////////////////////////////////////////////////////////////////////
// TSINGTENG TH25Q-80HB
////////////////////////////////////////////////////////////////////////

[0]
.special.
GMessage :
G<<PROTECTION OPTIONS>>

S0000L0100 STATUS REGISTER 1 :
    E04,04 BP0 = 1
    E08,08 BP1 = 1
    E10,10 BP2 = 1
    E20,20 BP3 = 1
    E40,40 BP4 = 1

S0001L0100 STATUS REGISTER 2 :
    E40,40 CMP = 1

G<<USER SECURITY REGISTER OPTIONS>>

S0003L0100 USER SECURITY REGISTER REGION OPTION :
    E01,01 Enable Security Register Program
    E02,02 Enable Security Register Verify
    E04,04 Enable Security Register Erase
    E08,08 Enable Security Register Blank Check
    E10,10 Enable Security Register Checksum Calculate
    E80,80 Enable Status Register Verify/Blank Check
