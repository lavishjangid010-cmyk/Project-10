/*
 * ============================================================
 *  PARKING MANAGEMENT SYSTEM (C++ / OOP)
 *  OOP Concepts: Classes & Objects, Encapsulation, Abstraction,
 *                Inheritance, Polymorphism, Exception Handling,
 *                Operator Overloading, Static Members
 * ============================================================
 *  Team Members:
 *    25EGJIT014 - Krishna Sharma
 *    25EGJIT015 - Kuldeep Prjapat
 *    25EGJIT017 - Lavish Jangid
 *    25EGJIT018 - Laxmi Kumavat
 * ============================================================
 */

#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <memory>
#include <algorithm>
#include <stdexcept>
#include <ctime>
#include <cctype>
#include <cstdlib>

using namespace std;

/* ============================================================
 *  Utility helpers
 * ============================================================ */
namespace util {

string line(char ch, int n) { return string(n, ch); }

string toUpper(string s) {
    for (size_t i = 0; i < s.size(); i++) s[i] = (char)toupper((unsigned char)s[i]);
    return s;
}

string trim(const string& s) {
    size_t a = 0, b = s.size();
    while (a < b && isspace((unsigned char)s[a])) a++;
    while (b > a && isspace((unsigned char)s[b - 1])) b--;
    return s.substr(a, b - a);
}

string formatTime(time_t t) {
    char buf[32];
    struct tm* tmv = localtime(&t);
    strftime(buf, sizeof(buf), "%d-%b-%Y %H:%M", tmv);
    return string(buf);
}

string formatDuration(int minutes) {
    char buf[32];
    snprintf(buf, sizeof(buf), "%dh %02dm", minutes / 60, minutes % 60);
    return string(buf);
}

} // namespace util

/* ============================================================
 *  Custom exception hierarchy (Exception Handling)
 * ============================================================ */
class ParkingException : public runtime_error {
public:
    explicit ParkingException(const string& msg) : runtime_error(msg) {}
};

class ParkingFullException : public ParkingException {
public:
    explicit ParkingFullException(const string& type)
        : ParkingException("Parking is FULL for " + type + ". No free slot available.") {}
};

class DuplicateVehicleException : public ParkingException {
public:
    explicit DuplicateVehicleException(const string& plate)
        : ParkingException("Vehicle " + plate + " is already parked. Entry rejected.") {}
};

class VehicleNotFoundException : public ParkingException {
public:
    explicit VehicleNotFoundException(const string& plate)
        : ParkingException("Vehicle " + plate + " is not currently parked here.") {}
};

class ReceiptNotFoundException : public ParkingException {
public:
    explicit ReceiptNotFoundException(int no)
        : ParkingException("Receipt number " + to_string(no) + " not found.") {}
};

class InvalidInputException : public ParkingException {
public:
    explicit InvalidInputException(const string& msg) : ParkingException(msg) {}
};

/* ============================================================
 *  Vehicle hierarchy (Abstraction, Inheritance, Polymorphism)
 * ============================================================ */
enum VehicleType { BIKE = 0, CAR = 1, HEAVY = 2 };
const int NUM_TYPES = 3;

class Vehicle {                          /* abstract base class */
protected:
    string plate;
    string owner;

public:
    Vehicle(const string& plateNo, const string& ownerName)
        : plate(plateNo), owner(ownerName) {}
    virtual ~Vehicle() {}                /* virtual destructor */

    const string& getPlate() const { return plate; }
    const string& getOwner() const { return owner; }

    /* pure virtual functions: every vehicle type MUST define these */
    virtual VehicleType getType() const = 0;
    virtual string getTypeName() const = 0;
    virtual int getRatePerHour() const = 0;

    /* virtual: derived classes may change the billing rule */
    virtual int billedHours(int minutes) const {
        int h = (minutes + 59) / 60;     /* partial hour = full hour */
        return h < 1 ? 1 : h;            /* minimum 1 hour */
    }
    virtual string billingRule() const {
        return "partial hour = full hour, min 1 hour";
    }

    int calculateFee(int minutes) const {
        return billedHours(minutes) * getRatePerHour();
    }
};

class TwoWheeler : public Vehicle {
public:
    TwoWheeler(const string& p, const string& o) : Vehicle(p, o) {}
    VehicleType getType() const override { return BIKE; }
    string getTypeName() const override { return "Two-Wheeler"; }
    int getRatePerHour() const override { return 10; }
};

class Car : public Vehicle {
public:
    Car(const string& p, const string& o) : Vehicle(p, o) {}
    VehicleType getType() const override { return CAR; }
    string getTypeName() const override { return "Car"; }
    int getRatePerHour() const override { return 30; }
};

class HeavyVehicle : public Vehicle {
public:
    HeavyVehicle(const string& p, const string& o) : Vehicle(p, o) {}
    VehicleType getType() const override { return HEAVY; }
    string getTypeName() const override { return "Heavy Vehicle"; }
    int getRatePerHour() const override { return 60; }

    /* Polymorphism: heavy vehicles have a 2 hour minimum charge */
    int billedHours(int minutes) const override {
        return max(2, Vehicle::billedHours(minutes));
    }
    string billingRule() const override {
        return "partial hour = full hour, min 2 hours";
    }
};

/* Factory: creates the right object for the chosen type */
shared_ptr<Vehicle> createVehicle(VehicleType type, const string& plate, const string& owner) {
    if (plate.size() < 4)
        throw InvalidInputException("Vehicle number must be at least 4 characters.");
    switch (type) {
        case BIKE:  return make_shared<TwoWheeler>(plate, owner);
        case CAR:   return make_shared<Car>(plate, owner);
        default:    return make_shared<HeavyVehicle>(plate, owner);
    }
}

/* ============================================================
 *  Slot class (Encapsulation)
 * ============================================================ */
class Slot {
private:
    string label;
    VehicleType type;
    shared_ptr<Vehicle> vehicle;         /* nullptr when free */
    time_t entryTime;

public:
    Slot(const string& lbl, VehicleType t) : label(lbl), type(t), entryTime(0) {}

    bool isFree() const { return vehicle == nullptr; }
    const string& getLabel() const { return label; }
    VehicleType getType() const { return type; }
    shared_ptr<Vehicle> getVehicle() const { return vehicle; }
    time_t getEntryTime() const { return entryTime; }

    void park(shared_ptr<Vehicle> v, time_t entry) {
        vehicle = v;
        entryTime = entry;
    }

    shared_ptr<Vehicle> release() {
        shared_ptr<Vehicle> v = vehicle;
        vehicle.reset();
        return v;
    }

    int parkedMinutes() const {
        double secs = difftime(time(NULL), entryTime);
        return secs < 0 ? 0 : (int)(secs / 60.0);
    }
};

/* ============================================================
 *  Receipt class (static member, operator overloading)
 * ============================================================ */
class Receipt {
private:
    static int counter;                  /* shared by all Receipt objects */

    int receiptNo;
    shared_ptr<Vehicle> vehicle;
    string slotLabel;
    time_t entryTime;
    time_t exitTime;
    int durationMin;
    int hours;
    int fee;
    bool generated;

public:
    Receipt(shared_ptr<Vehicle> v, const string& slot, time_t in, time_t out)
        : receiptNo(counter++), vehicle(v), slotLabel(slot), entryTime(in),
          exitTime(out), hours(0), fee(0), generated(false) {
        double mins = difftime(out, in) / 60.0;
        durationMin = mins < 0 ? 0 : (int)mins;
    }

    int getReceiptNo() const { return receiptNo; }
    const Vehicle& getVehicle() const { return *vehicle; }
    int getDurationMin() const { return durationMin; }
    int getHours() const { return hours; }
    int getFee() const { return fee; }
    bool isGenerated() const { return generated; }

    /* Module 4: fee calculation (calls the vehicle's polymorphic rule) */
    void generate() {
        if (!generated) {
            hours = vehicle->billedHours(durationMin);
            fee = vehicle->calculateFee(durationMin);
            generated = true;
        }
    }

    friend ostream& operator<<(ostream& os, const Receipt& r);
};

int Receipt::counter = 1001;

ostream& operator<<(ostream& os, const Receipt& r) {
    os << "\n" << util::line('=', 46) << "\n";
    os << "              PARKING RECEIPT\n";
    os << util::line('=', 46) << "\n";
    os << "Receipt No    : " << r.receiptNo << "\n";
    os << "Vehicle No    : " << r.vehicle->getPlate() << "\n";
    os << "Owner         : " << r.vehicle->getOwner() << "\n";
    os << "Vehicle Type  : " << r.vehicle->getTypeName() << "\n";
    os << "Slot          : " << r.slotLabel << "\n";
    os << util::line('-', 46) << "\n";
    os << "Entry Time    : " << util::formatTime(r.entryTime) << "\n";
    os << "Exit Time     : " << util::formatTime(r.exitTime) << "\n";
    os << "Duration      : " << util::formatDuration(r.durationMin)
       << " (" << r.durationMin << " min)\n";
    os << util::line('-', 46) << "\n";
    os << "Rate          : Rs " << r.vehicle->getRatePerHour() << " per hour\n";
    os << "Billed Hours  : " << r.hours << "\n";
    os << "Billing Rule  : " << r.vehicle->billingRule() << "\n";
    os << "TOTAL FEE     : Rs " << r.fee << "\n";
    os << util::line('=', 46) << "\n";
    os << "       Thank you! Drive safely.\n";
    return os;
}

/* ============================================================
 *  ParkingLot class: owns slots and history (core logic)
 * ============================================================ */
class ParkingLot {
private:
    vector<Slot> slots;
    vector<Receipt> history;

    static string typeName(VehicleType t) {
        return t == BIKE ? "Two-Wheeler" : (t == CAR ? "Car" : "Heavy Vehicle");
    }

public:
    ParkingLot(int bikeSlots = 10, int carSlots = 10, int heavySlots = 4) {
        const int counts[NUM_TYPES] = { bikeSlots, carSlots, heavySlots };
        const char prefix[NUM_TYPES] = { 'B', 'C', 'H' };
        for (int t = 0; t < NUM_TYPES; t++)
            for (int i = 1; i <= counts[t]; i++)
                slots.push_back(Slot(string(1, prefix[t]) + to_string(i), (VehicleType)t));
    }

    const vector<Slot>& getSlots() const { return slots; }
    const vector<Receipt>& getHistory() const { return history; }

    int totalCount(VehicleType t) const {
        int c = 0;
        for (size_t i = 0; i < slots.size(); i++) if (slots[i].getType() == t) c++;
        return c;
    }
    int freeCount(VehicleType t) const {
        int c = 0;
        for (size_t i = 0; i < slots.size(); i++)
            if (slots[i].getType() == t && slots[i].isFree()) c++;
        return c;
    }

    Slot* findSlotByPlate(const string& plate) {
        for (size_t i = 0; i < slots.size(); i++)
            if (!slots[i].isFree() && slots[i].getVehicle()->getPlate() == plate)
                return &slots[i];
        return nullptr;
    }

    Receipt* findReceipt(int no) {
        for (size_t i = 0; i < history.size(); i++)
            if (history[i].getReceiptNo() == no) return &history[i];
        return nullptr;
    }

    /* Module 1 */
    const Slot& registerEntry(shared_ptr<Vehicle> v, time_t entry) {
        if (findSlotByPlate(v->getPlate()) != nullptr)
            throw DuplicateVehicleException(v->getPlate());

        for (size_t i = 0; i < slots.size(); i++) {
            if (slots[i].getType() == v->getType() && slots[i].isFree()) {
                slots[i].park(v, entry);
                return slots[i];
            }
        }
        throw ParkingFullException(typeName(v->getType()));
    }

    /* Module 3 */
    Receipt& recordExit(const string& plate, time_t exitTime) {
        Slot* s = findSlotByPlate(plate);
        if (s == nullptr) throw VehicleNotFoundException(plate);

        time_t entry = s->getEntryTime();
        string label = s->getLabel();
        shared_ptr<Vehicle> v = s->release();      /* frees the slot */
        history.push_back(Receipt(v, label, entry, exitTime));
        return history.back();
    }
};

/* ============================================================
 *  Input handling class
 * ============================================================ */
class Input {
public:
    static string readLine() {
        string s;
        if (!getline(cin, s)) {                    /* end of input: quit cleanly */
            cout << "\n";
            exit(0);
        }
        return s;
    }

    static int readInt(const string& prompt, int minVal, int maxVal) {
        while (true) {
            cout << prompt;
            string s = util::trim(readLine());
            try {
                size_t pos = 0;
                int val = stoi(s, &pos);
                if (pos == s.size() && val >= minVal && val <= maxVal) return val;
            } catch (const exception&) { }
            cout << "  [!] Invalid input. Enter a number between "
                 << minVal << " and " << maxVal << ".\n";
        }
    }

    static void pressEnter() {
        cout << "\nPress Enter to continue...";
        readLine();
    }
};

/* ============================================================
 *  ParkingApp class: menu and user interface
 * ============================================================ */
class ParkingApp {
private:
    ParkingLot lot;

    static const char* typeLabel(int t) {
        return t == 0 ? "Two-Wheeler" : (t == 1 ? "Car" : "Heavy Vehicle");
    }
    static int typeRate(int t) { return t == 0 ? 10 : (t == 1 ? 30 : 60); }

    void printMenu() const {
        cout << "\n==================================================\n";
        cout << "            PARKING MANAGEMENT SYSTEM\n";
        cout << "==================================================\n";
        cout << " [Module 1: Register Vehicle Entry]\n";
        cout << "   1. Register Vehicle Entry\n\n";
        cout << " [Module 2: Display Available Slots]\n";
        cout << "   2. Display Parking Slots Status\n";
        cout << "   3. Find a Parked Vehicle\n\n";
        cout << " [Module 3: Vehicle Exit & Duration]\n";
        cout << "   4. Record Vehicle Exit\n\n";
        cout << " [Module 4: Fees & Receipts]\n";
        cout << "   5. Calculate Fee & Generate Receipt\n";
        cout << "   6. Collection Report (Sorted by Fee)\n\n";
        cout << "   9. Load Demo Data\n";
        cout << "   0. Exit System\n";
        cout << "==================================================\n";
    }

    /* ---------- Module 1 ---------- */
    void registerEntry() {
        cout << "\n===== REGISTER VEHICLE ENTRY =====\n";
        cout << "Vehicle Types:\n";
        for (int t = 0; t < NUM_TYPES; t++)
            cout << "  " << (t + 1) << ". " << left << setw(14) << typeLabel(t)
                 << " (Rs " << typeRate(t) << "/hour, "
                 << lot.freeCount((VehicleType)t) << " free slot(s))\n";

        VehicleType type = (VehicleType)(Input::readInt("Select vehicle type (1-3): ", 1, NUM_TYPES) - 1);
        if (lot.freeCount(type) == 0) throw ParkingFullException(typeLabel(type));

        cout << "Enter Vehicle Number (e.g. RJ14AB1234): ";
        string plate = util::toUpper(util::trim(Input::readLine()));
        cout << "Enter Driver/Owner Name: ";
        string owner = util::trim(Input::readLine());
        if (owner.empty()) owner = "N/A";

        shared_ptr<Vehicle> v = createVehicle(type, plate, owner);
        const Slot& s = lot.registerEntry(v, time(NULL));

        cout << "\n" << util::line('=', 44) << "\n";
        cout << "            PARKING ENTRY TICKET\n";
        cout << util::line('=', 44) << "\n";
        cout << "Vehicle No   : " << v->getPlate() << "\n";
        cout << "Owner        : " << v->getOwner() << "\n";
        cout << "Vehicle Type : " << v->getTypeName() << "\n";
        cout << "Slot Alloted : " << s.getLabel() << "\n";
        cout << "Entry Time   : " << util::formatTime(s.getEntryTime()) << "\n";
        cout << "Rate         : Rs " << v->getRatePerHour() << " per hour\n";
        cout << util::line('=', 44) << "\n";
    }

    /* ---------- Module 2 ---------- */
    void displaySlots() const {
        const vector<Slot>& slots = lot.getSlots();
        cout << "\n===== PARKING SLOT STATUS =====\n";
        cout << "Legend:  [ B1 ] = Free     [XB1X] = Occupied\n\n";

        int occupied = 0;
        for (int t = 0; t < NUM_TYPES; t++) {
            VehicleType vt = (VehicleType)t;
            cout << left << setw(14) << typeLabel(t) << " (Rs " << setw(2) << typeRate(t)
                 << "/hr) - Free: " << lot.freeCount(vt) << " / " << lot.totalCount(vt) << "\n  ";
            for (size_t i = 0; i < slots.size(); i++) {
                if (slots[i].getType() != vt) continue;
                if (slots[i].isFree()) cout << "[ " << slots[i].getLabel() << " ] ";
                else { cout << "[X" << slots[i].getLabel() << "X] "; occupied++; }
            }
            cout << "\n\n";
        }
        cout << "Total Slots: " << slots.size() << " | Occupied: " << occupied
             << " | Available: " << (slots.size() - occupied) << "\n\n";

        if (occupied > 0) {
            cout << "Currently Parked Vehicles:\n" << util::line('-', 82) << "\n";
            cout << left << setw(6) << "Slot" << setw(16) << "Vehicle No" << setw(18) << "Owner"
                 << setw(14) << "Type" << setw(18) << "Entry Time" << "Parked\n";
            cout << util::line('-', 82) << "\n";
            for (size_t i = 0; i < slots.size(); i++) {
                if (slots[i].isFree()) continue;
                shared_ptr<Vehicle> v = slots[i].getVehicle();
                cout << left << setw(6) << slots[i].getLabel() << setw(16) << v->getPlate()
                     << setw(18) << v->getOwner() << setw(14) << v->getTypeName()
                     << setw(18) << util::formatTime(slots[i].getEntryTime())
                     << util::formatDuration(slots[i].parkedMinutes()) << "\n";
            }
            cout << util::line('-', 82) << "\n";
        }
    }

    void findVehicle() {
        cout << "\n===== FIND PARKED VEHICLE =====\n";
        cout << "Enter Vehicle Number: ";
        string plate = util::toUpper(util::trim(Input::readLine()));

        Slot* s = lot.findSlotByPlate(plate);
        if (s == nullptr) throw VehicleNotFoundException(plate);

        shared_ptr<Vehicle> v = s->getVehicle();
        int mins = s->parkedMinutes();
        cout << "\nVehicle No   : " << v->getPlate() << "\n";
        cout << "Owner        : " << v->getOwner() << "\n";
        cout << "Type         : " << v->getTypeName() << "\n";
        cout << "Slot         : " << s->getLabel() << "\n";
        cout << "Entry Time   : " << util::formatTime(s->getEntryTime()) << "\n";
        cout << "Parked For   : " << util::formatDuration(mins) << "\n";
        cout << "Fee So Far   : Rs " << v->calculateFee(mins) << " (if it exits now)\n";
    }

    /* ---------- Module 3 ---------- */
    void vehicleExit() {
        cout << "\n===== RECORD VEHICLE EXIT =====\n";
        cout << "Enter Vehicle Number: ";
        string plate = util::toUpper(util::trim(Input::readLine()));

        Slot* s = lot.findSlotByPlate(plate);
        if (s == nullptr) throw VehicleNotFoundException(plate);

        cout << "\n  1. Use current system time as exit time\n";
        cout << "  2. Enter parked duration manually in minutes (for demo/testing)\n";
        int mode = Input::readInt("Choose (1-2): ", 1, 2);

        time_t exitTime = time(NULL);
        if (mode == 2) {
            int mins = Input::readInt("Enter duration in minutes (1-14400): ", 1, 14400);
            exitTime = s->getEntryTime() + (time_t)mins * 60;
        }

        string label = s->getLabel();
        Receipt& r = lot.recordExit(plate, exitTime);

        cout << "\n[+] Exit recorded. Slot " << label << " is now FREE.\n";
        cout << "    Vehicle   : " << r.getVehicle().getPlate() << "\n";
        cout << "    Exit Time : " << util::formatTime(exitTime) << "\n";
        cout << "    Duration  : " << util::formatDuration(r.getDurationMin())
             << " (" << r.getDurationMin() << " min)\n";

        if (Input::readInt("\nCalculate fee and generate receipt now? (1 = Yes, 0 = Later): ", 0, 1) == 1) {
            r.generate();
            cout << r;
        } else {
            cout << "Receipt No " << r.getReceiptNo() << " is pending. Use option 5 later.\n";
        }
    }

    /* ---------- Module 4 ---------- */
    void feeAndReceipt() {
        cout << "\n===== CALCULATE FEE & GENERATE RECEIPT =====\n";
        const vector<Receipt>& h = lot.getHistory();
        if (h.empty()) {
            cout << "[!] No vehicle has exited yet.\n";
            return;
        }

        int pending = 0;
        cout << "Exited vehicles:\n" << util::line('-', 58) << "\n";
        cout << left << setw(11) << "Receipt No" << setw(17) << "Vehicle No"
             << setw(15) << "Duration" << "Fee Status\n" << util::line('-', 58) << "\n";
        for (size_t i = 0; i < h.size(); i++) {
            cout << left << setw(11) << h[i].getReceiptNo() << setw(17) << h[i].getVehicle().getPlate()
                 << setw(15) << util::formatDuration(h[i].getDurationMin())
                 << (h[i].isGenerated() ? "Generated" : "PENDING") << "\n";
            if (!h[i].isGenerated()) pending++;
        }
        cout << util::line('-', 58) << "\nPending receipts: " << pending << "\n\n";

        int no = Input::readInt("Enter Receipt No (0 to cancel): ", 0, 999999);
        if (no == 0) return;

        Receipt* r = lot.findReceipt(no);
        if (r == nullptr) throw ReceiptNotFoundException(no);
        if (r->isGenerated()) cout << "\n(Receipt already generated, showing again)\n";
        r->generate();
        cout << *r;
    }

    void collectionReport() const {
        cout << "\n===== COLLECTION REPORT (Sorted by Fee) =====\n";
        const vector<Receipt>& h = lot.getHistory();
        if (h.empty()) {
            cout << "[!] No completed parking records yet.\n";
            return;
        }

        vector<const Receipt*> sorted;
        int total = 0, done = 0;
        for (size_t i = 0; i < h.size(); i++) {
            sorted.push_back(&h[i]);
            if (h[i].isGenerated()) { total += h[i].getFee(); done++; }
        }
        sort(sorted.begin(), sorted.end(), [](const Receipt* a, const Receipt* b) {
            int fa = a->isGenerated() ? a->getFee() : -1;
            int fb = b->isGenerated() ? b->getFee() : -1;
            return fa > fb;                          /* highest fee first, pending last */
        });

        cout << util::line('-', 76) << "\n";
        cout << left << setw(9) << "Receipt" << setw(16) << "Vehicle No" << setw(15) << "Type"
             << setw(11) << "Duration" << setw(8) << "Hours" << "Fee (Rs)\n";
        cout << util::line('-', 76) << "\n";
        for (size_t i = 0; i < sorted.size(); i++) {
            const Receipt* r = sorted[i];
            cout << left << setw(9) << r->getReceiptNo() << setw(16) << r->getVehicle().getPlate()
                 << setw(15) << r->getVehicle().getTypeName()
                 << setw(11) << util::formatDuration(r->getDurationMin());
            if (r->isGenerated()) cout << setw(8) << r->getHours() << r->getFee() << "\n";
            else                  cout << setw(8) << "-" << "PENDING\n";
        }
        cout << util::line('-', 76) << "\n";
        cout << "Receipts generated : " << done << " of " << h.size() << "\n";
        cout << "TOTAL COLLECTION   : Rs " << total << "\n";
    }

    /* ---------- Demo data ---------- */
    void loadDemoData() {
        struct Demo { VehicleType type; const char* plate; const char* owner; int minsAgo; };
        const Demo demo[] = {
            { BIKE,  "RJ14AB1234", "Ramesh Kumar",  45  },
            { CAR,   "RJ14CD5678", "Suresh Meena",  135 },
            { CAR,   "RJ20EF9012", "Mahesh Jain",   20  },
            { BIKE,  "RJ14GH3456", "Dinesh Saini",  300 },
            { HEAVY, "RJ27IJ7890", "Mukesh Yadav",  190 },
            { CAR,   "RJ14KL1122", "Naresh Sharma", 75  }
        };
        int added = 0;
        time_t now = time(NULL);
        for (size_t i = 0; i < sizeof(demo) / sizeof(demo[0]); i++) {
            try {
                lot.registerEntry(createVehicle(demo[i].type, demo[i].plate, demo[i].owner),
                                  now - (time_t)demo[i].minsAgo * 60);
                added++;
            } catch (const ParkingException&) { /* skip duplicates / full */ }
        }
        cout << "\n[+] " << added << " demo vehicle(s) parked.\n";
    }

public:
    void run() {
        int choice;
        do {
            printMenu();
            choice = Input::readInt("Enter your choice: ", 0, 9);
            try {
                switch (choice) {
                    case 1: registerEntry();    break;
                    case 2: displaySlots();     break;
                    case 3: findVehicle();      break;
                    case 4: vehicleExit();      break;
                    case 5: feeAndReceipt();    break;
                    case 6: collectionReport(); break;
                    case 9: loadDemoData();     break;
                    case 0: cout << "\nThank you for using the Parking Management System. Goodbye!\n"; break;
                    default: cout << "\n[!] Invalid option. Choose 0-6 or 9.\n"; break;
                }
            } catch (const ParkingException& e) {
                cout << "\n[!] " << e.what() << "\n";
            }
            if (choice != 0) Input::pressEnter();
        } while (choice != 0);
    }
};

int main() {
    ParkingApp app;
    app.run();
    return 0;
}
