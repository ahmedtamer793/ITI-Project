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
  - **Repository Pattern:** Clean separation between business logic and data access.
  - **Dependency Injection:** Repositories are injected into Service classes.
  - **OOP Principles:** Extensive use of Inheritance, Polymorphism, and Interfaces.

## ⚙️ How to Setup and Run

### 1. Database Configuration
Due to connection export constraints, the raw database files are provided for a guaranteed setup.
1. Locate the `agri_uber.zip` file in this repository.
2. Extract the `agri_uber` folder from the zip file.
3. Paste it into your MySQL data directory: `C:\xampp\mysql\data\`.
4. Start the **MySQL module** via XAMPP on port **3307**.

### 2. Running the Application
1. Open the `ITI Project.sln` file using **Visual Studio**.
2. Build and Run the project (`Ctrl + F5`).
*Note: All required DLLs (e.g., `mysqlcppconn-10-vs14.dll`) are included in the root folder.*

---

## 💡 The Core Idea: "Uber for Farmers" (أوبر الفلاحين)

The project stems from a real-world agricultural challenge: **Access to Technology**. Many small-scale farmers cannot afford to buy expensive machinery like harvesters or high-end tractors, while many machine owners have their equipment sitting idle between seasons.

**Agri-Uber** functions exactly like a ride-sharing app but for the fields:
- **For the Farmer:** It democratizes access to advanced tools. A farmer can "hail" a tractor for a specific date and duration, paying only for what they use, which significantly reduces harvest costs.
- **For the Owner:** It turns idle machinery into a profit center, ensuring their equipment is working and earning throughout the year.
- **The Result:** A shared economy model that boosts agricultural productivity and ensures that no land goes unplowed due to a lack of equipment.


