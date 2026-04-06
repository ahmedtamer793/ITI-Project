# 🚜 Agri-Uber: Agricultural Resource Management System
## 📌 Project Overview
Agri-Uber is a comprehensive C++ console application designed to bridge the gap between **Farmers** and **Agricultural Machine Owners**. It provides a seamless platform for booking machinery, managing schedules, and tracking financial statistics, fully integrated with a **MySQL** database for robust data persistence.

*Developed as a project submission for the Information Technology Institute (ITI).*

## ✨ Key Features
- **Role-Based Authentication:** Secure registration and login for both Farmers and Machine Owners.
- **Machine Management:** Owners can list tractors, harvesters, irrigation pumps, and plows with custom hourly rates.
- **Dynamic Booking System:** Farmers can browse available machines, check date availability, and request bookings.
- **Status Tracking:** Owners can Accept/Reject requests; Farmers can Cancel or Mark as Completed.
- **Financial Dashboard:** Automated calculation of total costs, platform fees (5%), and net earnings for owners.

## 🛠️ Tech Stack & Architecture
- **Language:** C++
- **Database:** MySQL Server
- **Database Driver:** MySQL Connector/C++ (JDBC API)
- **Architecture & Design Patterns:**
  - **Repository Pattern:** Clean separation between business logic and data access (e.g., `MySQLUserRepo`, `MySQLBookingRepo`).
  - **Dependency Injection:** Repositories are injected into Service classes (`AuthService`, `BookingService`).
  - **OOP Principles:** Extensive use of Inheritance, Polymorphism, Encapsulation, and Interfaces.
  - **Input Validation:** Robust Regex-based validation for user inputs (names, phones, dates).

## ⚙️ How to Setup and Run

### 1. Database Configuration (Important)
To ensure a smooth evaluation experience without import errors, the raw database files are provided.
1. Locate the `agri_uber.zip` file in this repository.
2. Extract the `agri_uber` folder from the zip file.
3. Navigate to your XAMPP installation directory, specifically: `C:\xampp\mysql\data\` (or wherever XAMPP is installed).
4. Paste the extracted `agri_uber` folder directly into the `data` directory.
5. Start the **MySQL module** via the XAMPP Control Panel.
    
**Default Connection Details:**
- Host: `127.0.0.1`
- Port: `3307` *(Ensure your MySQL is configured to run on this port)*
- Username: `root`
- Password: *(Leave blank)*

### 2. Running the Application
1. Clone the repository to your local machine.
2. Open the `ITI Project.sln` file using **Visual Studio**.
3. Build and Run the project (`Ctrl + F5`).

*Note: All required dynamic link libraries (e.g., `mysqlcppconn-10-vs14.dll`) and authentication plugins are included in the repository to ensure an immediate, out-of-the-box build without requiring external environment setups.*
