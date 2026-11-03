# Parking Management System (C++ / OOP)

A terminal-based Parking Management System developed in C++ using core Object-Oriented Programming concepts: Classes & Objects, Encapsulation, Abstraction, Inheritance, Polymorphism, Exception Handling and Operator Overloading.

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

The parking lot has **24 slots**: 10 for Two-Wheelers (`B1–B10`), 10 for Cars (`C1–C10`) and 4 for Heavy Vehicles (`H1–H4`). The project is divided into 4 modules:

### 1. Register Vehicle Entry
* Accepts vehicle type, vehicle number and owner name, then auto-allocates the first free slot of that type.
* Rejects duplicate vehicles (`DuplicateVehicleException`) and full parking (`ParkingFullException`).
* Prints an entry ticket with slot number, entry time and hourly rate.

### 2. Display Available Parking Slots
* Slot map showing every slot as free `[ B1 ]` or occupied `[XB1X]`, grouped by vehicle type.
* Free / occupied / total summary and a table of currently parked vehicles with time parked so far.
* **Find a Parked Vehicle** by number, with the fee accumulated so far.

### 3. Record Vehicle Exit and Duration
* Finds the vehicle by number, frees its slot and calculates the parking duration from entry and exit time.
* Exit time can be the current system time, or a manual duration in minutes (useful for demo/testing).
* Every exit is stored in history with a unique receipt number. Unknown vehicles raise `VehicleNotFoundException`.

### 4. Calculate Parking Fees and Generate Receipts
* Billed per hour, a partial hour counts as a full hour, minimum charge is 1 hour.

| Vehicle Type | Rate | Minimum Charge |
|--------------|------|----------------|
| Two-Wheeler | Rs 10 / hour | 1 hour |
| Car | Rs 30 / hour | 1 hour |
| Heavy Vehicle | Rs 60 / hour | **2 hours** |

* Prints a detailed receipt (vehicle, slot, entry/exit time, duration, billed hours, billing rule, total fee). Receipts can be generated later or reprinted using the receipt number.
* **Collection Report:** all receipts sorted by fee (highest first) with total collection.

---

## 🧱 Class Design

```text
Vehicle (abstract)
 ├── TwoWheeler
 ├── Car
 └── HeavyVehicle        (overrides billedHours: minimum 2 hours)

Slot          : one parking slot, holds a Vehicle (or empty)
Receipt       : stores one completed stay + fee, prints with operator<<
ParkingLot    : owns all Slots and Receipts, core business logic
ParkingApp    : menu and user interface
Input         : safe input reading helper

ParkingException (extends std::runtime_error)
 ├── ParkingFullException
 ├── DuplicateVehicleException
 ├── VehicleNotFoundException
 ├── ReceiptNotFoundException
 └── InvalidInputException
```

---

## 🛠️ OOP Concepts Implemented

* **Classes & Objects:** `Vehicle`, `Slot`, `Receipt`, `ParkingLot`, `ParkingApp`, `Input`.
* **Encapsulation:** private data members with public getters and methods (e.g. `Slot`, `Receipt`, `ParkingLot`).
* **Abstraction:** `Vehicle` is an abstract class with pure virtual functions (`getType`, `getTypeName`, `getRatePerHour`).
* **Inheritance:** `TwoWheeler`, `Car` and `HeavyVehicle` derive from `Vehicle`; custom exceptions derive from `ParkingException`.
* **Polymorphism:** virtual `billedHours()` and `billingRule()`; `HeavyVehicle` overrides them. Slots store `shared_ptr<Vehicle>`, so the correct version runs at runtime.
* **Constructors & Virtual Destructor:** proper object initialization and safe cleanup through base pointers.
* **Static Member:** `Receipt::counter` generates unique receipt numbers.
* **Operator Overloading:** `operator<<` (friend) prints a `Receipt` directly with `cout << receipt`.
* **Exception Handling:** custom exception hierarchy with `try / catch` in the main menu loop.
* **Factory Function:** `createVehicle()` builds the right derived object for the chosen type.
* **STL & Smart Pointers:** `vector`, `sort` with lambda, `shared_ptr`.

---

## 🚀 How to Compile and Run

### 1. Prerequisites
* A C++ compiler supporting C++11 or later (`g++` from MinGW / MSYS2 / Linux GCC / Clang).

### 2. Compilation
Open your terminal in the project directory and run:
```bash
g++ -std=c++11 main.cpp -o parking_system
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

> 💡 **Tip:** Choose option `9` (Load Demo Data) to instantly park 6 sample vehicles. In option `4` (Record Exit), choose "enter duration manually" to test fees for any duration.

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
   3. Find a Parked Vehicle

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
Receipt No    : 1002
Vehicle No    : RJ27IJ7890
Owner         : Mukesh Yadav
Vehicle Type  : Heavy Vehicle
Slot          : H1
----------------------------------------------
Entry Time    : 03-Nov-2026 03:22
Exit Time     : 03-Nov-2026 03:52
Duration      : 0h 30m (30 min)
----------------------------------------------
Rate          : Rs 60 per hour
Billed Hours  : 2
Billing Rule  : partial hour = full hour, min 2 hours
TOTAL FEE     : Rs 120
==============================================
       Thank you! Drive safely.
```
