/*
 * ============================================================
 *  PARKING MANAGEMENT SYSTEM (C Language)
 *  DSA Concepts: Array of Structures, Linear Search, Bubble Sort
 * ============================================================
 *  Team Members:
 *    25EGJIT014 - Krishna Sharma
 *    25EGJIT015 - Kuldeep Prjapat
 *    25EGJIT017 - Lavish Jangid
 *    25EGJIT018 - Laxmi Kumavat
 * ============================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

/* ---------------- Parking lot configuration ---------------- */
#define NUM_TYPES    3
#define BIKE_SLOTS   10
#define CAR_SLOTS    10
#define HEAVY_SLOTS  4
#define TOTAL_SLOTS  (BIKE_SLOTS + CAR_SLOTS + HEAVY_SLOTS)
#define MAX_RECORDS  200
#define PLATE_LEN    16
#define NAME_LEN     40

static const char *TYPE_NAMES[NUM_TYPES]  = { "Two-Wheeler", "Car", "Heavy Vehicle" };
static const char  TYPE_PREFIX[NUM_TYPES] = { 'B', 'C', 'H' };
static const int   TYPE_RATE[NUM_TYPES]   = { 10, 30, 60 };                  /* Rs per hour */
static const int   TYPE_START[NUM_TYPES]  = { 0, BIKE_SLOTS, BIKE_SLOTS + CAR_SLOTS };
static const int   TYPE_COUNT[NUM_TYPES]  = { BIKE_SLOTS, CAR_SLOTS, HEAVY_SLOTS };

/* ---------------- Data structures ---------------- */
typedef struct {
    int    occupied;
    int    type;
    char   plate[PLATE_LEN];
    char   owner[NAME_LEN];
    time_t entryTime;
} Slot;

typedef struct {
    int    receiptNo;
    char   plate[PLATE_LEN];
    char   owner[NAME_LEN];
    int    type;
    int    slotIndex;
    time_t entryTime;
    time_t exitTime;
    int    durationMin;
    int    billedHours;
    int    fee;
    int    feeCalculated;       /* 0 = pending, 1 = receipt generated */
} Record;

Slot   slots[TOTAL_SLOTS];      /* array of structures: the parking lot */
Record history[MAX_RECORDS];    /* array of structures: completed stays */
int    recordCount   = 0;
int    nextReceiptNo = 1001;

/* ---------------- Safe input helpers ---------------- */
void readLine(char *buf, int size) {
    if (fgets(buf, size, stdin) == NULL) {      /* end of input: quit cleanly */
        printf("\n");
        exit(0);
    }
    size_t len = strlen(buf);
    if (len > 0 && buf[len - 1] == '\n') {
        buf[len - 1] = '\0';
    } else {
        int c;
        while ((c = getchar()) != '\n' && c != EOF) { }  /* discard extra input */
    }
}

int readInt(const char *prompt, int min, int max) {
    char buf[64];
    char *end;
    long val;
    while (1) {
        printf("%s", prompt);
        readLine(buf, sizeof(buf));
        val = strtol(buf, &end, 10);
        if (end != buf && *end == '\0' && val >= min && val <= max)
            return (int)val;
        printf("  [!] Invalid input. Enter a number between %d and %d.\n", min, max);
    }
}

void pressEnter(void) {
    char buf[8];
    printf("\nPress Enter to continue...");
    readLine(buf, sizeof(buf));
}

void printLine(char ch, int n) {
    int i;
    for (i = 0; i < n; i++) putchar(ch);
    putchar('\n');
}

void toUpperStr(char *s) {
    int i;
    for (i = 0; s[i] != '\0'; i++) s[i] = (char)toupper((unsigned char)s[i]);
}

void trimStr(char *s) {
    int start = 0, end = (int)strlen(s) - 1;
    while (s[start] && isspace((unsigned char)s[start])) start++;
    while (end >= start && isspace((unsigned char)s[end])) end--;
    memmove(s, s + start, (size_t)(end - start + 1));
    s[end - start + 1] = '\0';
}

void formatTime(time_t t, char *buf, size_t size) {
    struct tm *tmv = localtime(&t);
    strftime(buf, size, "%d-%b-%Y %H:%M", tmv);
}

void slotLabel(int idx, char *buf) {
    int t;
    for (t = NUM_TYPES - 1; t >= 0; t--) {
        if (idx >= TYPE_START[t]) {
            sprintf(buf, "%c%d", TYPE_PREFIX[t], idx - TYPE_START[t] + 1);
            return;
        }
    }
    strcpy(buf, "?");
}

void formatDuration(int minutes, char *buf) {
    sprintf(buf, "%dh %02dm", minutes / 60, minutes % 60);
}

/* ---------------- Linear Search helpers ---------------- */
int findFreeSlot(int type) {
    int i;
    for (i = TYPE_START[type]; i < TYPE_START[type] + TYPE_COUNT[type]; i++)
        if (!slots[i].occupied) return i;
    return -1;
}

int findSlotByPlate(const char *plate) {
    int i;
    for (i = 0; i < TOTAL_SLOTS; i++)
        if (slots[i].occupied && strcmp(slots[i].plate, plate) == 0) return i;
    return -1;
}

int findRecordByReceipt(int receiptNo) {
    int i;
    for (i = 0; i < recordCount; i++)
        if (history[i].receiptNo == receiptNo) return i;
    return -1;
}

int countFree(int type) {
    int i, c = 0;
    for (i = TYPE_START[type]; i < TYPE_START[type] + TYPE_COUNT[type]; i++)
        if (!slots[i].occupied) c++;
    return c;
}

int currentMinutes(const Slot *s) {
    double secs = difftime(time(NULL), s->entryTime);
    return secs < 0 ? 0 : (int)(secs / 60.0);
}

/* ============================================================
 *  MODULE 4 (core): FEE CALCULATION
 *  Rule: billed per hour, partial hour counts as full hour,
 *        minimum charge = 1 hour.
 * ============================================================ */
int calculateBilledHours(int minutes) {
    int hours = (minutes + 59) / 60;
    return hours < 1 ? 1 : hours;
}

/* ============================================================
 *  MODULE 1: REGISTER VEHICLE ENTRY
 * ============================================================ */
int addVehicle(int type, const char *plate, const char *owner, time_t entry) {
    int idx = findFreeSlot(type);
    if (idx == -1) return -1;
    slots[idx].occupied  = 1;
    slots[idx].type      = type;
    slots[idx].entryTime = entry;
    strncpy(slots[idx].plate, plate, PLATE_LEN - 1);
    slots[idx].plate[PLATE_LEN - 1] = '\0';
    strncpy(slots[idx].owner, owner, NAME_LEN - 1);
    slots[idx].owner[NAME_LEN - 1] = '\0';
    return idx;
}

void registerEntry(void) {
    int type, idx;
    char plate[PLATE_LEN], owner[NAME_LEN], label[8], timeStr[32];
    int t;

    printf("\n===== REGISTER VEHICLE ENTRY =====\n");
    printf("Vehicle Types:\n");
    for (t = 0; t < NUM_TYPES; t++)
        printf("  %d. %-14s (Rs %d/hour, %d free slot(s))\n",
               t + 1, TYPE_NAMES[t], TYPE_RATE[t], countFree(t));

    type = readInt("Select vehicle type (1-3): ", 1, NUM_TYPES) - 1;
    if (countFree(type) == 0) {
        printf("\n[!] Sorry, no free slot available for %s. Parking is FULL for this type.\n",
               TYPE_NAMES[type]);
        return;
    }

    do {
        printf("Enter Vehicle Number (e.g. RJ14AB1234): ");
        readLine(plate, PLATE_LEN);
        trimStr(plate);
        toUpperStr(plate);
        if (strlen(plate) < 4) printf("  [!] Vehicle number must be at least 4 characters.\n");
    } while (strlen(plate) < 4);

    if (findSlotByPlate(plate) != -1) {
        printf("\n[!] Vehicle %s is already parked. Entry rejected.\n", plate);
        return;
    }

    printf("Enter Driver/Owner Name: ");
    readLine(owner, NAME_LEN);
    trimStr(owner);
    if (strlen(owner) == 0) strcpy(owner, "N/A");

    idx = addVehicle(type, plate, owner, time(NULL));
    slotLabel(idx, label);
    formatTime(slots[idx].entryTime, timeStr, sizeof(timeStr));

    printf("\n");
    printLine('=', 44);
    printf("            PARKING ENTRY TICKET\n");
    printLine('=', 44);
    printf("Vehicle No   : %s\n", plate);
    printf("Owner        : %s\n", owner);
    printf("Vehicle Type : %s\n", TYPE_NAMES[type]);
    printf("Slot Alloted : %s\n", label);
    printf("Entry Time   : %s\n", timeStr);
    printf("Rate         : Rs %d per hour\n", TYPE_RATE[type]);
    printLine('=', 44);
}

/* ============================================================
 *  MODULE 2: DISPLAY AVAILABLE PARKING SLOTS
 * ============================================================ */
void displaySlots(void) {
    int t, i, occupiedCount = 0, found = 0;
    char label[8], timeStr[32], dur[16];

    printf("\n===== PARKING SLOT STATUS =====\n");
    printf("Legend:  [ B1 ] = Free     [XB1X] = Occupied\n\n");

    for (t = 0; t < NUM_TYPES; t++) {
        printf("%-14s (Rs %2d/hr) - Free: %d / %d\n",
               TYPE_NAMES[t], TYPE_RATE[t], countFree(t), TYPE_COUNT[t]);
        printf("  ");
        for (i = TYPE_START[t]; i < TYPE_START[t] + TYPE_COUNT[t]; i++) {
            slotLabel(i, label);
            if (slots[i].occupied) printf("[X%sX] ", label);
            else                   printf("[ %s ] ", label);
        }
        printf("\n\n");
    }

    for (i = 0; i < TOTAL_SLOTS; i++) if (slots[i].occupied) occupiedCount++;
    printf("Total Slots: %d | Occupied: %d | Available: %d\n\n",
           TOTAL_SLOTS, occupiedCount, TOTAL_SLOTS - occupiedCount);

    for (i = 0; i < TOTAL_SLOTS; i++) {
        if (!slots[i].occupied) continue;
        if (!found) {
            printf("Currently Parked Vehicles:\n");
            printLine('-', 82);
            printf("%-6s %-16s %-18s %-14s %-18s %-8s\n",
                   "Slot", "Vehicle No", "Owner", "Type", "Entry Time", "Parked");
            printLine('-', 82);
            found = 1;
        }
        slotLabel(i, label);
        formatTime(slots[i].entryTime, timeStr, sizeof(timeStr));
        formatDuration(currentMinutes(&slots[i]), dur);
        printf("%-6s %-16s %-18s %-14s %-18s %-8s\n", label, slots[i].plate,
               slots[i].owner, TYPE_NAMES[slots[i].type], timeStr, dur);
    }
    if (found) printLine('-', 82);
}

void searchVehicle(void) {
    char plate[PLATE_LEN], label[8], timeStr[32], dur[16];
    int idx, mins;

    printf("\n===== FIND PARKED VEHICLE (Linear Search) =====\n");
    printf("Enter Vehicle Number: ");
    readLine(plate, PLATE_LEN);
    trimStr(plate);
    toUpperStr(plate);

    idx = findSlotByPlate(plate);
    if (idx == -1) {
        printf("\n[!] Vehicle %s is not currently parked here.\n", plate);
        return;
    }
    slotLabel(idx, label);
    formatTime(slots[idx].entryTime, timeStr, sizeof(timeStr));
    mins = currentMinutes(&slots[idx]);
    formatDuration(mins, dur);

    printf("\nVehicle No   : %s\n", slots[idx].plate);
    printf("Owner        : %s\n", slots[idx].owner);
    printf("Type         : %s\n", TYPE_NAMES[slots[idx].type]);
    printf("Slot         : %s\n", label);
    printf("Entry Time   : %s\n", timeStr);
    printf("Parked For   : %s\n", dur);
    printf("Fee So Far   : Rs %d (if it exits now)\n",
           calculateBilledHours(mins) * TYPE_RATE[slots[idx].type]);
}

/* ============================================================
 *  MODULE 4: CALCULATE PARKING FEES & GENERATE RECEIPT
 * ============================================================ */
void printReceipt(const Record *r) {
    char label[8], inStr[32], outStr[32], dur[16];
    slotLabel(r->slotIndex, label);
    formatTime(r->entryTime, inStr, sizeof(inStr));
    formatTime(r->exitTime, outStr, sizeof(outStr));
    formatDuration(r->durationMin, dur);

    printf("\n");
    printLine('=', 46);
    printf("              PARKING RECEIPT\n");
    printLine('=', 46);
    printf("Receipt No    : %d\n", r->receiptNo);
    printf("Vehicle No    : %s\n", r->plate);
    printf("Owner         : %s\n", r->owner);
    printf("Vehicle Type  : %s\n", TYPE_NAMES[r->type]);
    printf("Slot          : %s\n", label);
    printLine('-', 46);
    printf("Entry Time    : %s\n", inStr);
    printf("Exit Time     : %s\n", outStr);
    printf("Duration      : %s (%d min)\n", dur, r->durationMin);
    printLine('-', 46);
    printf("Rate          : Rs %d per hour\n", TYPE_RATE[r->type]);
    printf("Billed Hours  : %d (partial hour = full hour)\n", r->billedHours);
    printf("TOTAL FEE     : Rs %d\n", r->fee);
    printLine('=', 46);
    printf("       Thank you! Drive safely.\n");
}

void generateReceipt(int recIdx) {
    Record *r = &history[recIdx];
    if (!r->feeCalculated) {
        r->billedHours   = calculateBilledHours(r->durationMin);
        r->fee           = r->billedHours * TYPE_RATE[r->type];
        r->feeCalculated = 1;
    }
    printReceipt(r);
}

void feeAndReceiptMenu(void) {
    int i, pending = 0, no, idx;

    printf("\n===== CALCULATE FEE & GENERATE RECEIPT =====\n");
    if (recordCount == 0) {
        printf("[!] No vehicle has exited yet.\n");
        return;
    }

    printf("Exited vehicles:\n");
    printLine('-', 58);
    printf("%-10s %-16s %-14s %-12s\n", "Receipt No", "Vehicle No", "Duration", "Fee Status");
    printLine('-', 58);
    for (i = 0; i < recordCount; i++) {
        char dur[16];
        formatDuration(history[i].durationMin, dur);
        printf("%-10d %-16s %-14s %-12s\n", history[i].receiptNo, history[i].plate, dur,
               history[i].feeCalculated ? "Generated" : "PENDING");
        if (!history[i].feeCalculated) pending++;
    }
    printLine('-', 58);
    printf("Pending receipts: %d\n\n", pending);

    no = readInt("Enter Receipt No (0 to cancel): ", 0, 999999);
    if (no == 0) return;
    idx = findRecordByReceipt(no);
    if (idx == -1) {
        printf("\n[!] Receipt number %d not found.\n", no);
        return;
    }
    if (history[idx].feeCalculated) printf("\n(Receipt already generated, showing again)\n");
    generateReceipt(idx);
}

/* ============================================================
 *  MODULE 3: RECORD VEHICLE EXIT AND DURATION
 * ============================================================ */
int recordExitOf(int slotIdx, time_t exitTime) {
    Record *r;
    if (recordCount >= MAX_RECORDS) return -1;

    r = &history[recordCount];
    memset(r, 0, sizeof(*r));
    r->receiptNo   = nextReceiptNo++;
    strcpy(r->plate, slots[slotIdx].plate);
    strcpy(r->owner, slots[slotIdx].owner);
    r->type        = slots[slotIdx].type;
    r->slotIndex   = slotIdx;
    r->entryTime   = slots[slotIdx].entryTime;
    r->exitTime    = exitTime;
    r->durationMin = (int)(difftime(exitTime, r->entryTime) / 60.0);
    if (r->durationMin < 0) r->durationMin = 0;

    slots[slotIdx].occupied = 0;       /* free the slot */
    return recordCount++;
}

void vehicleExit(void) {
    char plate[PLATE_LEN], label[8], dur[16], outStr[32];
    int idx, mode, recIdx, mins;
    time_t exitTime;

    printf("\n===== RECORD VEHICLE EXIT =====\n");
    printf("Enter Vehicle Number: ");
    readLine(plate, PLATE_LEN);
    trimStr(plate);
    toUpperStr(plate);

    idx = findSlotByPlate(plate);
    if (idx == -1) {
        printf("\n[!] Vehicle %s not found in parking. Check the number and try again.\n", plate);
        return;
    }

    printf("\n  1. Use current system time as exit time\n");
    printf("  2. Enter parked duration manually in minutes (for demo/testing)\n");
    mode = readInt("Choose (1-2): ", 1, 2);

    if (mode == 1) {
        exitTime = time(NULL);
    } else {
        mins = readInt("Enter duration in minutes (1-14400): ", 1, 14400);
        exitTime = slots[idx].entryTime + (time_t)mins * 60;
    }

    slotLabel(idx, label);
    recIdx = recordExitOf(idx, exitTime);
    if (recIdx == -1) {
        printf("\n[!] History storage full. Cannot record exit.\n");
        return;
    }

    formatTime(exitTime, outStr, sizeof(outStr));
    formatDuration(history[recIdx].durationMin, dur);
    printf("\n[+] Exit recorded. Slot %s is now FREE.\n", label);
    printf("    Vehicle   : %s\n", history[recIdx].plate);
    printf("    Exit Time : %s\n", outStr);
    printf("    Duration  : %s (%d min)\n", dur, history[recIdx].durationMin);

    if (readInt("\nCalculate fee and generate receipt now? (1 = Yes, 0 = Later): ", 0, 1))
        generateReceipt(recIdx);
    else
        printf("Receipt No %d is pending. Use option 5 later.\n", history[recIdx].receiptNo);
}

/* ============================================================
 *  MODULE 4 (report): COLLECTION REPORT (Bubble Sort by fee)
 * ============================================================ */
void collectionReport(void) {
    int order[MAX_RECORDS];
    int i, j, tmp, totalCollection = 0, done = 0;
    char dur[16];

    printf("\n===== COLLECTION REPORT (Sorted by Fee - Bubble Sort) =====\n");
    if (recordCount == 0) {
        printf("[!] No completed parking records yet.\n");
        return;
    }

    for (i = 0; i < recordCount; i++) {
        order[i] = i;
        if (history[i].feeCalculated) {
            totalCollection += history[i].fee;
            done++;
        }
    }

    /* Bubble Sort on index array: highest fee first, pending at the end */
    for (i = 0; i < recordCount - 1; i++) {
        int swapped = 0;
        for (j = 0; j < recordCount - i - 1; j++) {
            int a = history[order[j]].feeCalculated ? history[order[j]].fee : -1;
            int b = history[order[j + 1]].feeCalculated ? history[order[j + 1]].fee : -1;
            if (a < b) {
                tmp = order[j]; order[j] = order[j + 1]; order[j + 1] = tmp;
                swapped = 1;
            }
        }
        if (!swapped) break;
    }

    printLine('-', 76);
    printf("%-9s %-16s %-14s %-10s %-7s %-8s\n",
           "Receipt", "Vehicle No", "Type", "Duration", "Hours", "Fee (Rs)");
    printLine('-', 76);
    for (i = 0; i < recordCount; i++) {
        Record *r = &history[order[i]];
        formatDuration(r->durationMin, dur);
        if (r->feeCalculated)
            printf("%-9d %-16s %-14s %-10s %-7d %-8d\n", r->receiptNo, r->plate,
                   TYPE_NAMES[r->type], dur, r->billedHours, r->fee);
        else
            printf("%-9d %-16s %-14s %-10s %-7s %-8s\n", r->receiptNo, r->plate,
                   TYPE_NAMES[r->type], dur, "-", "PENDING");
    }
    printLine('-', 76);
    printf("Receipts generated : %d of %d\n", done, recordCount);
    printf("TOTAL COLLECTION   : Rs %d\n", totalCollection);
}

/* ---------------- Demo data ---------------- */
void loadDemoData(void) {
    static const struct { int type; const char *plate; const char *owner; int minsAgo; } demo[] = {
        { 0, "RJ14AB1234", "Ramesh Kumar",  45  },
        { 1, "RJ14CD5678", "Suresh Meena",  135 },
        { 1, "RJ20EF9012", "Mahesh Jain",   20  },
        { 0, "RJ14GH3456", "Dinesh Saini",  300 },
        { 2, "RJ27IJ7890", "Mukesh Yadav",  190 },
        { 1, "RJ14KL1122", "Naresh Sharma", 75  }
    };
    int i, added = 0, n = (int)(sizeof(demo) / sizeof(demo[0]));
    time_t now = time(NULL);

    for (i = 0; i < n; i++) {
        if (findSlotByPlate(demo[i].plate) != -1) continue;
        if (addVehicle(demo[i].type, demo[i].plate, demo[i].owner,
                       now - (time_t)demo[i].minsAgo * 60) != -1)
            added++;
    }
    printf("\n[+] %d demo vehicle(s) parked.\n", added);
}

/* ---------------- Menu ---------------- */
void printMenu(void) {
    printf("\n==================================================\n");
    printf("            PARKING MANAGEMENT SYSTEM\n");
    printf("==================================================\n");
    printf(" [Module 1: Register Vehicle Entry]\n");
    printf("   1. Register Vehicle Entry\n\n");
    printf(" [Module 2: Display Available Slots]\n");
    printf("   2. Display Parking Slots Status\n");
    printf("   3. Find a Parked Vehicle (Linear Search)\n\n");
    printf(" [Module 3: Vehicle Exit & Duration]\n");
    printf("   4. Record Vehicle Exit\n\n");
    printf(" [Module 4: Fees & Receipts]\n");
    printf("   5. Calculate Fee & Generate Receipt\n");
    printf("   6. Collection Report (Sorted by Fee)\n\n");
    printf("   9. Load Demo Data\n");
    printf("   0. Exit System\n");
    printf("==================================================\n");
}

int main(void) {
    int choice;
    do {
        printMenu();
        choice = readInt("Enter your choice: ", 0, 9);
        switch (choice) {
            case 1: registerEntry();     break;
            case 2: displaySlots();      break;
            case 3: searchVehicle();     break;
            case 4: vehicleExit();       break;
            case 5: feeAndReceiptMenu(); break;
            case 6: collectionReport();  break;
            case 9: loadDemoData();      break;
            case 0: printf("\nThank you for using the Parking Management System. Goodbye!\n"); break;
            default: printf("\n[!] Invalid option. Choose 0-6 or 9.\n"); break;
        }
        if (choice != 0) pressEnter();
    } while (choice != 0);
    return 0;
}
