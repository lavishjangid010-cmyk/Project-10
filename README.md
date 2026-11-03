# Parking Management System (C Language)

A terminal-based Parking Management System developed in C using fundamental Data Structures and Algorithms (DSA) concepts: Array of Structures, Linear Search, and Bubble Sort.

---

## 👥 Team Members

| Roll Number | Student Name |
|-------------|--------------|
| `25EGJIT014` | Krishna Sharma |
| `25EGJIT015` | Kuldeep Prjapat |
| `25EGJIT017` | Lavish Jangid |
| `25EGJIT018` | Laxmi Kumavat |

---

## 📌 Project Overview & Module Breakdown

This project is divided into 4 distinct modules as per the DSA project curriculum. The parking lot has **24 slots**: 10 for Two-Wheelers (`B1–B10`), 10 for Cars (`C1–C10`) and 4 for Heavy Vehicles (`H1–H4`).

### 1. Register Vehicle Entry
* **Add Vehicle:** Accepts vehicle type, vehicle number and owner name, then auto-allocates the first free slot of that type.
* **Duplicate Validation:** Prevents registering a vehicle that is already parked.
* **Full-Parking Check:** Shows a message when no slot is free for the selected vehicle type.
* **Entry Ticket:** Prints a ticket with slot number, entry time and hourly rate.

### 2. Display Available Parking Slots
* **Slot Map:** Shows every slot as free `[ B1 ]` or occupied `[XB1X]`, grouped by vehicle type.
* **Availability Summary:** Free / occupied / total slots per type and overall.
* **Parked Vehicles Table:** Lists currently parked vehicles with owner, entry time and time parked so far.
* **Find a Parked Vehicle:** Uses **Linear Search** by vehicle number and shows the fee accumulated so far.

### 3. Record Vehicle Exit and Duration
* **Exit by Vehicle Number:** Finds the vehicle using Linear Search and frees its slot.
* **Duration Calculation:** Calculates parking duration from entry and exit time (system time, or manual minutes for demo/testing).
* **History Storage:** Every exit is saved in a history array with a unique receipt number.
* **Error Handling:** Gracefully reports vehicle-not-found scenarios.

### 4. Calculate Parking Fees and Generate Receipts
* **Fee Rule:** Billed per hour, a partial hour counts as a full hour, minimum charge is 1 hour.

| Vehicle Type | Rate |
|--------------|------|
| Two-Wheeler | Rs 10 / hour |
| Car | Rs 30 / hour |
| Heavy Vehicle | Rs 60 / hour |

* **Receipt Generation:** Prints a detailed receipt (vehicle, slot, entry/exit time, duration, billed hours, total fee). Receipts can be generated later or reprinted using the receipt number.
* **Collection Report:** Lists all receipts **sorted by fee (highest first) using Bubble Sort** and shows the total collection.

---

## 🛠️ DSA Concepts Implemented

* **Structures (`struct`):** `Slot` and `Record` for encapsulating heterogeneous parking data.
* **Array of Structures:** `slots[24]` for the parking lot and `history[200]` for completed parking records.
* **Linear Search:** O(n) search for free slots, vehicle by number, and receipt by number.
* **Bubble Sort:** O(n²) sorting (with early-exit optimization) on an index array for the collection report.
* **Time Handling:** `time.h` (`time_t`, `difftime`, `strftime`) for entry/exit time and duration.
* **Input Stream Handling:** Safe reading with `fgets` + `strtol` preventing infinite input loops or buffer overflow.

---

## 🚀 How to Compile and Run

### 1. Prerequisites
* A C compiler like `gcc` (MinGW / MSYS2 / Clang / Linux GCC).

### 2. Compilation
Open your terminal in the project directory and run:
```bash
gcc main.c -o parking_system
```

### 3. Execution
* **On Windows:**
  ```bash
  parking_system.exe
  ```
* **On Linux / macOS:**
  ```bash
  ./parking_system
  ```

> 💡 **Tip:** Choose option `9` (Load Demo Data) to instantly park 6 sample vehicles. In option `4` (Record Exit), choose "enter duration manually" to test fee calculation for any duration.

---

## 💻 Sample Program Menu

```text
==================================================
            PARKING MANAGEMENT SYSTEM
==================================================
 [Module 1: Register Vehicle Entry]
   1. Register Vehicle Entry

 [Module 2: Display Available Slots]
   2. Display Parking Slots Status
   3. Find a Parked Vehicle (Linear Search)

 [Module 3: Vehicle Exit & Duration]
   4. Record Vehicle Exit

 [Module 4: Fees & Receipts]
   5. Calculate Fee & Generate Receipt
   6. Collection Report (Sorted by Fee)

   9. Load Demo Data
   0. Exit System
==================================================
```

---

## 🧾 Sample Output (Receipt)

```text
==============================================
              PARKING RECEIPT
==============================================
Receipt No    : 1001
Vehicle No    : RJ14CD5678
Owner         : Suresh Meena
Vehicle Type  : Car
Slot          : C1
----------------------------------------------
Entry Time    : 03-Nov-2026 04:03
Exit Time     : 03-Nov-2026 06:18
Duration      : 2h 15m (135 min)
----------------------------------------------
Rate          : Rs 30 per hour
Billed Hours  : 3 (partial hour = full hour)
TOTAL FEE     : Rs 90
==============================================
       Thank you! Drive safely.
```
