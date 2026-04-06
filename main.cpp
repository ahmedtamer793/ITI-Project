#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <algorithm>
#include <regex>
#include <limits>
#include <exception>
#include <cstdlib>
#include <ctime>
#include <sstream>
#include <iomanip>
#include <mysql_driver.h>
#include <mysql_connection.h>
#include <cppconn/exception.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/resultset.h>

using namespace std;

// ==========================================
// 0. Database Manager
// Responsible for establishing a connection with the MySQL server
// ==========================================
class DBManager {
public:
    static sql::Connection* getConnection() {
        try {
            sql::mysql::MySQL_Driver* driver = sql::mysql::get_mysql_driver_instance();
            sql::ConnectOptionsMap properties;

            // Database configuration
            properties["hostName"] = "tcp://127.0.0.1:3307";
            properties["userName"] = "root";
            properties["password"] = "";
            properties["schema"] = "agri_uber";

            // Required authentication plugin for XAMPP
            properties["OPT_DEFAULT_AUTH"] = "mysql_native_password";

            return driver->connect(properties);
        }
        catch (sql::SQLException& e) {
            cout << "\n[Database Connection Error]: " << e.what() << "\n";
            return nullptr;
        }
    }
};

// ==========================================
// 1. Utilities & Enums
// ==========================================
enum class BookingStatus { Pending, Accepted, Rejected, Completed, Cancelled };

string statusToString(BookingStatus s) {
    switch (s) {
    case BookingStatus::Pending: return "Pending";
    case BookingStatus::Accepted: return "Accepted";
    case BookingStatus::Rejected: return "Rejected";
    case BookingStatus::Completed: return "Completed";
    case BookingStatus::Cancelled: return "Cancelled";
    default: return "Unknown";
    }
}

// Validator class for handling formatting and constraints
class InputValidator {
public:
    static bool isValidName(const string& name) {
        regex nameRegex("^[a-zA-Z\\s]{3,50}$");
        return regex_match(name, nameRegex);
    }

    static bool isValidPhone(const string& phone) {
        regex phoneRegex("^01[0125][0-9]{8}$");
        return regex_match(phone, phoneRegex);
    }

    static bool isValidFutureDate(const string& dateStr) {
        regex dateRegex("^\\d{4}-(0[1-9]|1[0-2])-(0[1-9]|[12]\\d|3[01])$");
        if (!regex_match(dateStr, dateRegex)) return false;

        int year, month, day;
        char dash1, dash2;
        stringstream ss(dateStr);
        ss >> year >> dash1 >> month >> dash2 >> day;

        time_t t = time(0);
        tm* now = localtime(&t);
        int currentYear = now->tm_year + 1900;
        int currentMonth = now->tm_mon + 1;
        int currentDay = now->tm_mday;

        if (year < currentYear) return false;
        if (year == currentYear && month < currentMonth) return false;
        if (year == currentYear && month == currentMonth && day < currentDay) return false;

        return true;
    }
};

// Input handler for robust console reading
class InputHandler {
public:
    static int getIntInput(string prompt, int min, int max) {
        int input;
        while (true) {
            cout << prompt;
            if (cin >> input && input >= min && input <= max) {
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                return input;
            }
            else {
                cout << "Error: Invalid Input! Please enter a number between " << min << " and " << max << ".\n";
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
            }
        }
    }

    static double getDoubleInput(string prompt, double min) {
        double input;
        while (true) {
            cout << prompt;
            if (cin >> input && input >= min) {
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                return input;
            }
            else {
                cout << "Error: Invalid Input! Must be a number >= " << min << ".\n";
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
            }
        }
    }

    static string getStringInput(string prompt, bool (*validator)(const string&)) {
        string input;
        while (true) {
            cout << prompt;
            getline(cin, input);
            if (validator(input)) {
                return input;
            }
            else {
                cout << "Error: Invalid Format! Please try again.\n";
            }
        }
    }

    static string getMachineTypeInput() {
        cout << "\nSelect Machine Type:\n";
        cout << "1. Tractor\n";
        cout << "2. Harvester\n";
        cout << "3. Irrigation Pump\n";
        cout << "4. Plow\n";

        int choice = getIntInput("Choice: ", 1, 4);
        switch (choice) {
        case 1: return "Tractor";
        case 2: return "Harvester";
        case 3: return "Irrigation Pump";
        case 4: return "Plow";
        default: return "Unknown";
        }
    }
};

// ==========================================
// 2. Entities (Models)
// ==========================================
enum class UserRole { Farmer, MachineOwner };

class User {
private:
    int id;
    string name;
    string phone;
    UserRole role;

public:
    User(int id, string name, string phone, UserRole role)
        : id(id), name(name), phone(phone), role(role) {
    }

    virtual ~User() = default;

    int getId() const { return id; }
    string getName() const { return name; }
    string getPhone() const { return phone; }
    UserRole getRole() const { return role; }

    virtual void displayInfo() = 0;
};

// Farmer Entity
class Farmer : public User {
private:
    double landSize;
public:
    Farmer(int id, string name, string phone, double size)
        : User(id, name, phone, UserRole::Farmer), landSize(size) {
    }

    double getLandSize() const { return landSize; }

    void displayInfo() override {
        cout << "\n--- Farmer Profile ---\n";
        cout << "Name : " << getName() << "\n";
        cout << "Phone: " << getPhone() << "\n";
        cout << "Land : " << landSize << " Acres\n";
        cout << "----------------------\n";
    }
};

// Machine Owner Entity
class MachineOwner : public User {
public:
    MachineOwner(int id, string name, string phone)
        : User(id, name, phone, UserRole::MachineOwner) {
    }

    void displayInfo() override {
        cout << "\n--- Owner Profile ---\n";
        cout << "Name : " << getName() << "\n";
        cout << "Phone: " << getPhone() << "\n";
    }
};

class Machine {
private:
    int id;
    int ownerId;
    string type;
    double pricePerHour;
    double rating;

public:
    Machine(int id, int ownerId, string type, double price)
        : id(id), ownerId(ownerId), type(type), pricePerHour(price), rating(0.0) {
    }

    int getId() const { return id; }
    int getOwnerId() const { return ownerId; }
    string getType() const { return type; }
    double getPrice() const { return pricePerHour; }
    double getRating() const { return rating; }
};

class Booking {
private:
    int id;
    int farmerId;
    int machineId;
    string date;
    int hours;
    double totalCost;
    BookingStatus status;

public:
    Booking(int id, int fId, int mId, string d, int h, double cost)
        : id(id), farmerId(fId), machineId(mId), date(d), hours(h), totalCost(cost), status(BookingStatus::Pending) {
    }

    BookingStatus getStatus() const { return status; }
    void setStatus(BookingStatus s) { status = s; }
    int getMachineId() const { return machineId; }
    string getDate() const { return date; }
    int getId() const { return id; }
    int getFarmerId() const { return farmerId; }
    int getHours() const { return hours; }
    double getTotalCost() const { return totalCost; }
};

// ==========================================
// 3. Interfaces (Contracts)
// ==========================================

// DTO for handling joined Booking data for owners (SOLID: Single Responsibility)
struct OwnerBookingDTO {
    int bookingId;
    int machineId;
    string machineType;
    string bookingDate;
    int hours;
    double totalCost;
    BookingStatus status;
    string farmerName;
    string farmerPhone;
};

class IUserRepository {
public:
    virtual ~IUserRepository() = default;
    virtual void saveUser(shared_ptr<User> user) = 0;
    virtual shared_ptr<User> findByPhone(string phone) = 0;
    virtual shared_ptr<User> findById(int id) = 0;
};

class IMachineRepository {
public:
    virtual ~IMachineRepository() = default;
    virtual void saveMachine(shared_ptr<Machine> machine) = 0;
    virtual vector<shared_ptr<Machine>> getAllMachines() = 0;
    virtual shared_ptr<Machine> getById(int id) = 0;
    virtual vector<shared_ptr<Machine>> getMachinesByOwner(int ownerId) = 0;
};

class IBookingRepository {
public:
    virtual ~IBookingRepository() = default;
    virtual void saveBooking(shared_ptr<Booking> booking) = 0;
    virtual void updateBooking(shared_ptr<Booking> booking) = 0;
    virtual bool isMachineAvailable(int machineId, string date) = 0;
    virtual vector<shared_ptr<Booking>> getAllBookings() = 0;
    virtual vector<shared_ptr<Booking>> getBookingsByFarmer(int farmerId) = 0;
    virtual shared_ptr<Booking> getById(int id) = 0;
    virtual void getOwnerFinancialStats(int ownerId, int& completedCount, double& totalEarnings) = 0;
    virtual vector<OwnerBookingDTO> getOwnerBookingsDetails(int ownerId) = 0;
};

// ==========================================
// 4. Repositories (Data Access Layer - MySQL)
// ==========================================

// --- MySQL User Repository ---
class MySQLUserRepo : public IUserRepository {
public:
    void saveUser(shared_ptr<User> user) override {
        try {
            sql::Connection* con = DBManager::getConnection();
            if (!con) return;

            sql::PreparedStatement* pstmt = con->prepareStatement(
                "INSERT INTO users (name, phone, role, land_size) VALUES (?, ?, ?, ?)"
            );
            pstmt->setString(1, user->getName());
            pstmt->setString(2, user->getPhone());

            if (user->getRole() == UserRole::Farmer) {
                pstmt->setString(3, "Farmer");
                shared_ptr<Farmer> f = dynamic_pointer_cast<Farmer>(user);
                pstmt->setDouble(4, f ? f->getLandSize() : 0.0);
            }
            else {
                pstmt->setString(3, "MachineOwner");
                pstmt->setDouble(4, 0.0);
            }

            pstmt->executeUpdate();
            delete pstmt;
            delete con;
        }
        catch (sql::SQLException& e) {
            cout << "\n[DB Error - Save User]: " << e.what() << "\n";
        }
    }

    shared_ptr<User> findByPhone(string phone) override {
        shared_ptr<User> foundUser = nullptr;
        try {
            sql::Connection* con = DBManager::getConnection();
            if (!con) return nullptr;

            sql::PreparedStatement* pstmt = con->prepareStatement(
                "SELECT user_id, name, phone, role, land_size FROM users WHERE phone = ?"
            );
            pstmt->setString(1, phone);
            sql::ResultSet* res = pstmt->executeQuery();

            if (res->next()) {
                int db_id = res->getInt(1);
                string db_name = res->getString(2);
                string db_phone = res->getString(3);
                string db_role = res->getString(4);

                if (db_role == "Farmer") {
                    double db_landSize = res->getDouble(5);
                    foundUser = make_shared<Farmer>(db_id, db_name, db_phone, db_landSize);
                }
                else {
                    foundUser = make_shared<MachineOwner>(db_id, db_name, db_phone);
                }
            }

            delete res;
            delete pstmt;
            delete con;
        }
        catch (sql::SQLException& e) {
            cout << "\n[DB Error - Find User]: " << e.what() << "\n";
        }
        return foundUser;
    }

    shared_ptr<User> findById(int id) override {
        shared_ptr<User> foundUser = nullptr;
        try {
            sql::Connection* con = DBManager::getConnection();
            if (!con) return nullptr;

            sql::PreparedStatement* pstmt = con->prepareStatement(
                "SELECT user_id, name, phone, role, land_size FROM users WHERE user_id = ?"
            );
            pstmt->setInt(1, id);
            sql::ResultSet* res = pstmt->executeQuery();

            if (res->next()) {
                int db_id = res->getInt(1);
                string db_name = res->getString(2);
                string db_phone = res->getString(3);
                string db_role = res->getString(4);

                if (db_role == "Farmer") {
                    double db_landSize = res->getDouble(5);
                    foundUser = make_shared<Farmer>(db_id, db_name, db_phone, db_landSize);
                }
                else {
                    foundUser = make_shared<MachineOwner>(db_id, db_name, db_phone);
                }
            }

            delete res;
            delete pstmt;
            delete con;
        }
        catch (sql::SQLException& e) {
            cout << "\n[DB Error - Find User]: " << e.what() << "\n";
        }
        return foundUser;
    }
};

// --- MySQL Machine Repository ---
class MySQLMachineRepo : public IMachineRepository {
public:
    void saveMachine(shared_ptr<Machine> machine) override {
        try {
            sql::Connection* con = DBManager::getConnection();
            if (!con) return;

            sql::PreparedStatement* pstmt = con->prepareStatement(
                "INSERT INTO machines (owner_id, machine_type, price_per_hour) VALUES (?, ?, ?)"
            );
            pstmt->setInt(1, machine->getOwnerId());
            pstmt->setString(2, machine->getType());
            pstmt->setDouble(3, machine->getPrice());

            pstmt->executeUpdate();
            delete pstmt;
            delete con;
        }
        catch (sql::SQLException& e) {
            cout << "\n[DB Error - Save Machine]: " << e.what() << "\n";
        }
    }

    vector<shared_ptr<Machine>> getAllMachines() override {
        vector<shared_ptr<Machine>> machineList;
        try {
            sql::Connection* con = DBManager::getConnection();
            if (!con) return machineList;

            sql::PreparedStatement* pstmt = con->prepareStatement(
                "SELECT machine_id, owner_id, machine_type, price_per_hour FROM machines"
            );
            sql::ResultSet* res = pstmt->executeQuery();

            while (res->next()) {
                int m_id = res->getInt(1);
                int m_ownerId = res->getInt(2);
                string m_type = res->getString(3);
                double m_price = res->getDouble(4);

                machineList.push_back(make_shared<Machine>(m_id, m_ownerId, m_type, m_price));
            }

            delete res;
            delete pstmt;
            delete con;
        }
        catch (sql::SQLException& e) {
            cout << "\n[DB Error - Get All Machines]: " << e.what() << "\n";
        }
        return machineList;
    }

    shared_ptr<Machine> getById(int id) override {
        shared_ptr<Machine> foundMachine = nullptr;
        try {
            sql::Connection* con = DBManager::getConnection();
            if (!con) return nullptr;

            sql::PreparedStatement* pstmt = con->prepareStatement(
                "SELECT machine_id, owner_id, machine_type, price_per_hour FROM machines WHERE machine_id = ?"
            );
            pstmt->setInt(1, id);
            sql::ResultSet* res = pstmt->executeQuery();

            if (res->next()) {
                int m_id = res->getInt(1);
                int m_ownerId = res->getInt(2);
                string m_type = res->getString(3);
                double m_price = res->getDouble(4);

                foundMachine = make_shared<Machine>(m_id, m_ownerId, m_type, m_price);
            }

            delete res;
            delete pstmt;
            delete con;
        }
        catch (sql::SQLException& e) {
            cout << "\n[DB Error - Get Machine By ID]: " << e.what() << "\n";
        }
        return foundMachine;
    }

    vector<shared_ptr<Machine>> getMachinesByOwner(int ownerId) override {
        vector<shared_ptr<Machine>> ownerMachines;
        try {
            sql::Connection* con = DBManager::getConnection();
            if (!con) return ownerMachines;

            sql::PreparedStatement* pstmt = con->prepareStatement(
                "SELECT machine_id, owner_id, machine_type, price_per_hour FROM machines WHERE owner_id = ?"
            );
            pstmt->setInt(1, ownerId);
            sql::ResultSet* res = pstmt->executeQuery();

            while (res->next()) {
                int m_id = res->getInt(1);
                int m_ownerId = res->getInt(2);
                string m_type = res->getString(3);
                double m_price = res->getDouble(4);

                ownerMachines.push_back(make_shared<Machine>(m_id, m_ownerId, m_type, m_price));
            }

            delete res;
            delete pstmt;
            delete con;
        }
        catch (sql::SQLException& e) {
            cout << "\n[DB Error - Get Machines By Owner]: " << e.what() << "\n";
        }
        return ownerMachines;
    }
};

// --- MySQL Booking Repository ---
class MySQLBookingRepo : public IBookingRepository {
    BookingStatus stringToStatus(string s) {
        if (s == "Accepted") return BookingStatus::Accepted;
        if (s == "Rejected") return BookingStatus::Rejected;
        if (s == "Completed") return BookingStatus::Completed;
        if (s == "Cancelled") return BookingStatus::Cancelled;
        return BookingStatus::Pending;
    }

public:
    void saveBooking(shared_ptr<Booking> b) override {
        try {
            sql::Connection* con = DBManager::getConnection();
            if (!con) return;

            sql::PreparedStatement* pstmt = con->prepareStatement(
                "INSERT INTO bookings (farmer_id, machine_id, booking_date, hours, total_cost, status) VALUES (?, ?, ?, ?, ?, ?)"
            );
            pstmt->setInt(1, b->getFarmerId());
            pstmt->setInt(2, b->getMachineId());
            pstmt->setString(3, b->getDate());
            pstmt->setInt(4, b->getHours());
            pstmt->setDouble(5, b->getTotalCost());
            pstmt->setString(6, statusToString(b->getStatus()));

            pstmt->executeUpdate();
            delete pstmt;
            delete con;
        }
        catch (sql::SQLException& e) {
            cout << "\n[DB Error - Save Booking]: " << e.what() << "\n";
        }
    }

    void updateBooking(shared_ptr<Booking> b) override {
        try {
            sql::Connection* con = DBManager::getConnection();
            if (!con) return;

            sql::PreparedStatement* pstmt = con->prepareStatement(
                "UPDATE bookings SET status = ? WHERE booking_id = ?"
            );
            pstmt->setString(1, statusToString(b->getStatus()));
            pstmt->setInt(2, b->getId());
            pstmt->executeUpdate();

            delete pstmt;
            delete con;
        }
        catch (sql::SQLException& e) {
            cout << "\n[DB Error - Update Booking]: " << e.what() << "\n";
        }
    }

    bool isMachineAvailable(int machineId, string requestedDate) override {
        bool available = true;
        try {
            sql::Connection* con = DBManager::getConnection();
            if (!con) return false;

            sql::PreparedStatement* pstmt = con->prepareStatement(
                "SELECT booking_id FROM bookings WHERE machine_id = ? AND booking_date = ? AND status IN ('Pending', 'Accepted')"
            );
            pstmt->setInt(1, machineId);
            pstmt->setString(2, requestedDate);
            sql::ResultSet* res = pstmt->executeQuery();

            if (res->next()) available = false;

            delete res;
            delete pstmt;
            delete con;
        }
        catch (sql::SQLException& e) {
            cout << "\n[DB Error - Check Availability]: " << e.what() << "\n";
        }
        return available;
    }

    vector<shared_ptr<Booking>> getAllBookings() override {
        vector<shared_ptr<Booking>> list;
        try {
            sql::Connection* con = DBManager::getConnection();
            if (!con) return list;

            sql::PreparedStatement* pstmt = con->prepareStatement(
                "SELECT booking_id, farmer_id, machine_id, booking_date, hours, total_cost, status FROM bookings"
            );
            sql::ResultSet* res = pstmt->executeQuery();

            while (res->next()) {
                auto b = make_shared<Booking>(res->getInt(1), res->getInt(2), res->getInt(3),
                    res->getString(4), res->getInt(5), res->getDouble(6));
                b->setStatus(stringToStatus(res->getString(7)));
                list.push_back(b);
            }
            delete res; delete pstmt; delete con;
        }
        catch (sql::SQLException& e) {
            cout << "\n[DB Error - Get All Bookings]: " << e.what() << "\n";
        }
        return list;
    }

    vector<shared_ptr<Booking>> getBookingsByFarmer(int farmerId) override {
        vector<shared_ptr<Booking>> list;
        try {
            sql::Connection* con = DBManager::getConnection();
            if (!con) return list;

            sql::PreparedStatement* pstmt = con->prepareStatement(
                "SELECT booking_id, farmer_id, machine_id, booking_date, hours, total_cost, status FROM bookings WHERE farmer_id = ?"
            );
            pstmt->setInt(1, farmerId);
            sql::ResultSet* res = pstmt->executeQuery();

            while (res->next()) {
                auto b = make_shared<Booking>(res->getInt(1), res->getInt(2), res->getInt(3),
                    res->getString(4), res->getInt(5), res->getDouble(6));
                b->setStatus(stringToStatus(res->getString(7)));
                list.push_back(b);
            }
            delete res; delete pstmt; delete con;
        }
        catch (sql::SQLException& e) {
            cout << "\n[DB Error - Get Bookings By Farmer]: " << e.what() << "\n";
        }
        return list;
    }

    shared_ptr<Booking> getById(int id) override {
        shared_ptr<Booking> found = nullptr;
        try {
            sql::Connection* con = DBManager::getConnection();
            if (!con) return nullptr;

            sql::PreparedStatement* pstmt = con->prepareStatement(
                "SELECT booking_id, farmer_id, machine_id, booking_date, hours, total_cost, status FROM bookings WHERE booking_id = ?"
            );
            pstmt->setInt(1, id);
            sql::ResultSet* res = pstmt->executeQuery();

            if (res->next()) {
                found = make_shared<Booking>(res->getInt(1), res->getInt(2), res->getInt(3),
                    res->getString(4), res->getInt(5), res->getDouble(6));
                found->setStatus(stringToStatus(res->getString(7)));
            }
            delete res; delete pstmt; delete con;
        }
        catch (sql::SQLException& e) {
            cout << "\n[DB Error - Get Booking By ID]: " << e.what() << "\n";
        }
        return found;
    }

    // --- New Optimized Queries ---
    void getOwnerFinancialStats(int ownerId, int& completedCount, double& totalEarnings) override {
        completedCount = 0;
        totalEarnings = 0.0;
        try {
            sql::Connection* con = DBManager::getConnection();
            if (!con) return;

            sql::PreparedStatement* pstmt = con->prepareStatement(
                "SELECT COUNT(b.booking_id), SUM(b.total_cost * 0.95) "
                "FROM bookings b "
                "JOIN machines m ON b.machine_id = m.machine_id "
                "WHERE m.owner_id = ? AND b.status = 'Completed'"
            );
            pstmt->setInt(1, ownerId);
            sql::ResultSet* res = pstmt->executeQuery();

            if (res->next()) {
                completedCount = res->getInt(1);
                // Handle NULL if there are no completed jobs yet
                totalEarnings = res->isNull(2) ? 0.0 : res->getDouble(2);
            }

            delete res;
            delete pstmt;
            delete con;
        }
        catch (sql::SQLException& e) {
            cout << "\n[DB Error - Get Owner Stats]: " << e.what() << "\n";
        }
    }

    vector<OwnerBookingDTO> getOwnerBookingsDetails(int ownerId) override {
        vector<OwnerBookingDTO> list;
        try {
            sql::Connection* con = DBManager::getConnection();
            if (!con) return list;

            sql::PreparedStatement* pstmt = con->prepareStatement(
                "SELECT b.booking_id, b.machine_id, m.machine_type, b.booking_date, b.hours, b.total_cost, b.status, u.name, u.phone "
                "FROM bookings b "
                "JOIN machines m ON b.machine_id = m.machine_id "
                "JOIN users u ON b.farmer_id = u.user_id "
                "WHERE m.owner_id = ?"
            );
            pstmt->setInt(1, ownerId);
            sql::ResultSet* res = pstmt->executeQuery();

            while (res->next()) {
                OwnerBookingDTO dto;
                dto.bookingId = res->getInt(1);
                dto.machineId = res->getInt(2);
                dto.machineType = res->getString(3);
                dto.bookingDate = res->getString(4);
                dto.hours = res->getInt(5);
                dto.totalCost = res->getDouble(6);
                dto.status = stringToStatus(res->getString(7));
                dto.farmerName = res->getString(8);
                dto.farmerPhone = res->getString(9);
                list.push_back(dto);
            }
            delete res; delete pstmt; delete con;
        }
        catch (sql::SQLException& e) {
            cout << "\n[DB Error - Get Owner Bookings Details]: " << e.what() << "\n";
        }
        return list;
    }
};

// ==========================================
// 5. Services (Business Logic)
// ==========================================
class AuthService {
    IUserRepository& repo;
    shared_ptr<User> currentUser;
public:
    AuthService(IUserRepository& r) : repo(r), currentUser(nullptr) {}

    bool registerUser(shared_ptr<User> u) {
        if (repo.findByPhone(u->getPhone())) {
            cout << "Error: Phone number already registered!\n";
            return false;
        }
        repo.saveUser(u);
        cout << "Registration Successful for: " << u->getName() << "!\n";
        return true;
    }

    bool login(string p) {
        currentUser = repo.findByPhone(p);
        if (currentUser) {
            cout << "\nWelcome Back, " << currentUser->getName() << "!\n";
            return true;
        }
        cout << "Login Failed: User Not Found.\n";
        return false;
    }

    shared_ptr<User> getCurrentUser() { return currentUser; }
    void logout() { currentUser = nullptr; cout << "Logged out successfully.\n"; }
};

class PricingCalculator {
public:
    static double calculateTotal(double price, int hours) { return price * hours; }
};

class BookingService {
    IBookingRepository& bRepo;
    IMachineRepository& mRepo;
    IUserRepository& uRepo;
    int nextBookingId = 1;
public:
    BookingService(IBookingRepository& b, IMachineRepository& m, IUserRepository& u)
        : bRepo(b), mRepo(m), uRepo(u) {
    }

    void showAvailableMachines() {
        auto list = mRepo.getAllMachines();
        if (list.empty()) {
            cout << "No machines available currently.\n";
            return;
        }
        cout << "\n--- Available Machines ---\n";
        for (auto& m : list) {
            cout << "ID: " << m->getId() << " | Type: " << m->getType()
                << " | Price: " << m->getPrice() << " EGP/hr\n";
        }
    }

    bool requestBooking(int fId, int mId, string d, int h) {
        auto m = mRepo.getById(mId);
        if (!m) {
            cout << "Error: Machine NOT Found!\n";
            return false;
        }

        if (!bRepo.isMachineAvailable(mId, d)) {
            cout << "\n[!] ERROR: This machine is already booked on " << d << ". Please choose another date or machine.\n";
            return false;
        }

        double total = PricingCalculator::calculateTotal(m->getPrice(), h);
        bRepo.saveBooking(make_shared<Booking>(nextBookingId++, fId, mId, d, h, total));
        cout << "\n Booking Request Sent! Status is Pending.\n";
        cout << "Total Cost: " << total << " EGP.\n";
        return true;
    }

    void showOwnerStats(shared_ptr<User> owner) {
        owner->displayInfo();

        int completedCount = 0;
        double totalNetEarnings = 0.0;

        bRepo.getOwnerFinancialStats(owner->getId(), completedCount, totalNetEarnings);

        cout << "--- Financial Stats ---\n";
        cout << "Completed Jobs : " << completedCount << "\n";
        cout << fixed << setprecision(2);
        cout << "Total Earnings : " << totalNetEarnings << " EGP\n";
        cout << "-----------------------\n";
    }

    void showFarmerBookingsAndInvoices(int farmerId) {
        auto myBookings = bRepo.getBookingsByFarmer(farmerId);
        if (myBookings.empty()) {
            cout << "You have no bookings yet.\n";
            return;
        }

        cout << "\n========== My Bookings & Invoices ==========\n";
        for (auto& b : myBookings) {
            auto machine = mRepo.getById(b->getMachineId());
            double total = b->getTotalCost();
            double platformFee = total * 0.05;
            double ownerShare = total * 0.95;

            cout << "Booking ID : #" << b->getId() << "\n";
            cout << "Date       : " << b->getDate() << "\n";
            if (machine) {
                cout << "Machine    : " << machine->getType() << " (ID: " << machine->getId() << ")\n";
            }
            cout << "Status     : " << statusToString(b->getStatus()) << "\n";

            if (b->getStatus() != BookingStatus::Cancelled && b->getStatus() != BookingStatus::Rejected) {
                cout << "--- Financial Details ---\n";
                cout << fixed << setprecision(2);
                cout << "Total Paid    : " << total << " EGP\n";
                cout << "Platform Fee  : " << platformFee << " EGP (5%)\n";
                cout << "Machine Owner : " << ownerShare << " EGP\n";
            }
            cout << "--------------------------------------------\n";
        }
    }

    void manageFarmerBookings(int farmerId) {
        showFarmerBookingsAndInvoices(farmerId);

        cout << "\nOptions: 1. Cancel a Booking | 2. Mark as Completed | 3. Back\n";
        int choice = InputHandler::getIntInput("Choice: ", 1, 3);

        if (choice == 3) return;

        int bId = InputHandler::getIntInput("Enter Booking ID: ", 1, 99999);
        auto b = bRepo.getById(bId);

        if (!b || b->getFarmerId() != farmerId) {
            cout << "Error: Booking not found or unauthorized.\n";
            return;
        }

        if (choice == 1) {
            if (b->getStatus() == BookingStatus::Pending || b->getStatus() == BookingStatus::Accepted) {
                b->setStatus(BookingStatus::Cancelled);
                bRepo.updateBooking(b);
                cout << "Booking Cancelled Successfully.\n";
            }
            else {
                cout << "Error: Cannot cancel a booking in current status.\n";
            }
        }
        else if (choice == 2) {
            if (b->getStatus() == BookingStatus::Accepted) {
                b->setStatus(BookingStatus::Completed);
                bRepo.updateBooking(b);
                cout << "Booking marked as Completed. Thank you!\n";
            }
            else {
                cout << "Error: Only 'Accepted' bookings can be marked as completed.\n";
            }
        }
    }

    void manageOwnerBookings(int ownerId) {
        auto bookingsDetails = bRepo.getOwnerBookingsDetails(ownerId);
        bool found = !bookingsDetails.empty();

        cout << "\n========== Incoming Bookings ==========\n";
        for (const auto& detail : bookingsDetails) {
            double total = detail.totalCost;
            double platformFee = total * 0.05;
            double netEarnings = total * 0.95;

            cout << "Booking ID : #" << detail.bookingId << "\n";
            cout << "Machine    : " << detail.machineType << " (ID: " << detail.machineId << ")\n";
            cout << "Date       : " << detail.bookingDate << " for " << detail.hours << " hours\n";
            cout << "Farmer Name: " << detail.farmerName << " | Phone: " << detail.farmerPhone << "\n";
            cout << "Net Earn   : " << netEarnings << " EGP\n";
            cout << "Status     : " << statusToString(detail.status) << "\n";
            cout << "---------------------------------------\n";
        }

        if (!found) {
            cout << "No bookings found for your machines yet.\n";
            return;
        }

        cout << "\nOptions: 1. Accept/Reject a Booking | 2. Back\n";
        int action = InputHandler::getIntInput("Choice: ", 1, 2);
        if (action == 2) return;

        int bId = InputHandler::getIntInput("Enter Booking ID to manage: ", 1, 99999);
        auto b = bRepo.getById(bId);

        if (!b) {
            cout << "Error: Booking not found.\n";
            return;
        }

        bool isMyBooking = false;
        for (const auto& d : bookingsDetails) {
            if (d.bookingId == bId) {
                isMyBooking = true;
                break;
            }
        }

        if (!isMyBooking) {
            cout << "Error: Unauthorized action.\n";
            return;
        }

        if (b->getStatus() != BookingStatus::Pending) {
            cout << "Error: You can only Accept/Reject 'Pending' bookings.\n";
            return;
        }

        cout << "1. Accept | 2. Reject\n";
        int decision = InputHandler::getIntInput("Choice: ", 1, 2);
        if (decision == 1) {
            b->setStatus(BookingStatus::Accepted);
            bRepo.updateBooking(b);
            cout << "Booking Accepted successfully!\n";
        }
        else {
            b->setStatus(BookingStatus::Rejected);
            bRepo.updateBooking(b);
            cout << "Booking Rejected. Dates are freed.\n";
        }
    }
};

// ==========================================
// 6. Presentation Layer (Main Menu)
// ==========================================
void showMainMenu() {
    cout << "\n================================\n";
    cout << "    Agri-Uber: Farmer System    \n";
    cout << "================================\n";
    cout << "1. Register As Farmer\n";
    cout << "2. Register As Machine Owner\n";
    cout << "3. Login\n";
    cout << "4. Exit\n";
}

int main() {
    MySQLUserRepo userRepo;
    MySQLMachineRepo machineRepo;
    MySQLBookingRepo bookingRepo;

    AuthService authService(userRepo);
    BookingService bookingService(bookingRepo, machineRepo, userRepo);

    int nextUserId = 1;
    int nextMachineId = 1;

    bool running = true;

    while (running) {
        try {
            showMainMenu();
            int mainChoice = InputHandler::getIntInput("Choice: ", 1, 4);

            switch (mainChoice) {
            case 1: {
                cout << "\n--- Farmer Registration ---\n";
                string name = InputHandler::getStringInput("Enter Name (Letters only): ", InputValidator::isValidName);
                string phone = InputHandler::getStringInput("Enter Phone (01xxxxxxxxx): ", InputValidator::isValidPhone);
                double landSize = InputHandler::getDoubleInput("Enter Land Size (Acres): ", 0.1);

                authService.registerUser(make_shared<Farmer>(nextUserId++, name, phone, landSize));
                break;
            }
            case 2: {
                cout << "\n--- Machine Owner Registration ---\n";
                string name = InputHandler::getStringInput("Enter Name (Letters only): ", InputValidator::isValidName);
                string phone = InputHandler::getStringInput("Enter Phone (01xxxxxxxxx): ", InputValidator::isValidPhone);

                auto owner = make_shared<MachineOwner>(nextUserId++, name, phone);
                if (authService.registerUser(owner)) {
                    owner = dynamic_pointer_cast<MachineOwner>(userRepo.findByPhone(phone));
                    if (owner) {
                        int numMachines = InputHandler::getIntInput("How many machines do you want to add? (1-20): ", 1, 20);

                        for (int i = 0; i < numMachines; i++) {
                            cout << "\nMachine #" << i + 1 << ":\n";
                            string type = InputHandler::getMachineTypeInput();
                            double price = InputHandler::getDoubleInput("Enter Price Per Hour: ", 1.0);

                            machineRepo.saveMachine(make_shared<Machine>(nextMachineId++, owner->getId(), type, price));
                            cout << "Machine Added Successfully!\n";
                        }
                    }
                }
                break;
            }
            case 3: {
                cout << "\n--- Login ---\n";
                string phone = InputHandler::getStringInput("Enter Phone (01xxxxxxxxx): ", InputValidator::isValidPhone);

                if (authService.login(phone)) {
                    auto user = authService.getCurrentUser();
                    bool loggedIn = true;

                    while (loggedIn) {
                        switch (user->getRole()) {
                        case UserRole::Farmer: {
                            cout << "\n--- Farmer Menu ---\n";
                            cout << "1. Show My Info\n2. View Available Machines\n3. Book Machine\n4. Manage My Bookings\n5. Logout\n";
                            int fChoice = InputHandler::getIntInput("Choice: ", 1, 5);

                            if (fChoice == 1) {
                                user->displayInfo();
                            }
                            else if (fChoice == 2) {
                                bookingService.showAvailableMachines();
                            }
                            else if (fChoice == 3) {
                                cout << "\n--- Available Machines For Booking ---\n";
                                bookingService.showAvailableMachines();

                                cout << "\n--- Please enter booking details ---\n";
                                int mId = InputHandler::getIntInput("Enter Machine ID from the list: ", 1, 9999);
                                string date = InputHandler::getStringInput("Enter Date (YYYY-MM-DD): ", InputValidator::isValidFutureDate);
                                int hrs = InputHandler::getIntInput("Hours: ", 1, 24);

                                bookingService.requestBooking(user->getId(), mId, date, hrs);
                            }
                            else if (fChoice == 4) {
                                bookingService.manageFarmerBookings(user->getId());
                            }
                            else if (fChoice == 5) {
                                authService.logout();
                                loggedIn = false;
                            }
                            break;
                        }
                        case UserRole::MachineOwner: {
                            cout << "\n--- Owner Menu ---\n";
                            cout << "1. Show My Info & Stats\n2. Add New Machine\n3. Manage Incoming Bookings\n4. Logout\n";
                            int oChoice = InputHandler::getIntInput("Choice: ", 1, 4);

                            if (oChoice == 1) {
                                bookingService.showOwnerStats(user);
                            }
                            else if (oChoice == 2) {
                                cout << "\n--- Add New Machine ---\n";
                                string type = InputHandler::getMachineTypeInput();
                                double price = InputHandler::getDoubleInput("Enter Price Per Hour: ", 1.0);

                                machineRepo.saveMachine(make_shared<Machine>(nextMachineId++, user->getId(), type, price));
                                cout << "New Machine Added Successfully!\n";
                            }
                            else if (oChoice == 3) {
                                bookingService.manageOwnerBookings(user->getId());
                            }
                            else if (oChoice == 4) {
                                authService.logout();
                                loggedIn = false;
                            }
                            break;
                        }
                        }
                    }
                }
                break;
            }
            case 4:
                cout << "Exiting Agri-Uber... Good Bye!\n";
                running = false;
                break;
            }
        }
        catch (const exception& e) {
            cerr << "\n[System Error]: " << e.what() << "\nReturning to Main Menu...\n";
        }
    }

    return 0;
}