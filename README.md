# 🚜 Agri-Uber: Agricultural Resource Management System

## 📌 Project Overview
Agri-Uber is a comprehensive C++ console application designed to bridge the gap between **Farmers** and **Agricultural Machine Owners**. It provides a seamless platform for booking machinery, managing schedules, and tracking financial statistics, fully integrated with a **SQLite** database for robust, lightweight, and zero-configuration data persistence.

*Developed as a project submission for the Information Technology Institute (ITI).*

## ✨ Key Features
- **Role-Based Authentication:** Secure registration and login for both Farmers and Machine Owners.
- **Machine Management:** Owners can list tractors, harvesters, irrigation pumps, and plows with custom hourly rates.
- **Dynamic Booking System:** Farmers can browse available machines, check date availability, and request bookings.
- **Status Tracking:** Owners can Accept/Reject requests; Farmers can Cancel or Mark as Completed.
- **Financial Dashboard:** Automated calculation of total costs, platform fees (5%), and net earnings for owners.

## 🛠️ Tech Stack & Architecture
- **Language:** C++
- **Database:** SQLite (Serverless, self-contained SQL database engine)
- **Database Driver:** SQLite3 C/C++ API
- **Architecture & Design Patterns:**
  - **Repository Pattern:** Clean separation between business logic and data access.
  - **Dependency Injection:** Repositories are injected into Service classes.
  - **OOP Principles:** Extensive use of Inheritance, Polymorphism, and Interfaces.

## ⚙️ How to Setup and Run

### 1. Zero-Configuration Database
Unlike traditional DBMS setups, this project uses **SQLite**. There is no need to install XAMPP, configure ports, or run external database servers. 
- Upon running the application for the first time, a local database file named `agri_uber.db` is **automatically generated** in the root directory.
- All required tables and schemas are initialized automatically by the application code.

### 2. Running the Application
1. Open the `ITI Project.sln` file using **Visual Studio**.
2. Build and Run the project (`Ctrl + F5`).
*Note: The required SQLite source files (`sqlite3.h` and `sqlite3.c`) are already included within the project source code. No external DLLs are needed!*

---

## 💡 The Core Idea: "Uber for Farmers" (أوبر الفلاحين)

The project stems from a real-world agricultural challenge: **Access to Technology**. Many small-scale farmers cannot afford to buy expensive machinery like harvesters or high-end tractors, while many machine owners have their equipment sitting idle between seasons.

**Agri-Uber** functions exactly like a ride-sharing app but for the fields:
- **For the Farmer:** It democratizes access to advanced tools. A farmer can "hail" a tractor for a specific date and duration, paying only for what they use, which significantly reduces harvest costs.
- **For the Owner:** It turns idle machinery into a profit center, ensuring their equipment is working and earning throughout the year.
- **The Result:** A shared economy model that boosts agricultural productivity and ensures that no land goes unplowed due to a lack of equipment.

- ##Schema : https://drive.google.com/file/d/1gMxBt7OPKDrmr_YEH2lUbQMPyRY_0bxW/view?usp=sharing
