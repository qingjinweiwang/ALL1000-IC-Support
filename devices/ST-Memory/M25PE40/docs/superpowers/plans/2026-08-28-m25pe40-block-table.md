# M25PE40 Block Table Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Make the driver block table describe M25PE40's eight 64 KiB sectors instead of TH25Q80HB security regions.

**Architecture:** Preserve the existing fixed-width block-record format. Replace only the memory records and synchronize the Status Register noun checks required by the new block numbering.

**Tech Stack:** Embedded C, ARMCC RVCT 2.2

## Global Constraints

- Preserve comment hierarchy, indentation, and 10-word block-record formatting.
- Keep `MainByte8Width`; do not advertise sector erase until D8h erase is implemented.
- Do not change unrelated device algorithms.

---

### Task 1: Correct the M25PE40 block table

**Files:**
- Modify: `WJx4_M25PE40-VMPxxx.h`
- Modify: `WJx4_M25PE40-VMPxxx.c`
- Test: `iBuild.bat WJx4_M25PE40-VMPxxx`

**Interfaces:**
- Consumes: M25PE40 sector addresses and existing ALL-1000 block-table ABI.
- Produces: eight main-memory records plus existing status/control records.

- [x] **Step 1:** Replace the TH25Q-derived table body with Sector 0–7 ranges from `00000h–7FFFFh`.
- [x] **Step 2:** Change the three Status Register comparisons from `Noun_Block4` to `Noun_Block8`.
- [x] **Step 3:** Build and require warning-free compilation plus successful BIN/SPC updates.
- [x] **Step 4:** Inspect the final ranges and confirm temporary build files were cleaned.
