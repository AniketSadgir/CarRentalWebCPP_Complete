# DriveHub Car Rental Management System

HTML + CSS + JavaScript frontend, native C++ Winsock backend, and local TXT database.

## Run with existing MinGW
Open a terminal in this folder and run:

g++ -std=c++11 main.cpp -o CarRentalServer.exe -lws2_32

Then:

CarRentalServer.exe

Open http://127.0.0.1:8080/

## Features
- Available cars and rental rates
- Hour/day rental calculation
- Rental booking
- Active/returned rental status
- Today's revenue and total revenue
- Rental history
- Add/delete cars
- Local TXT database; no MySQL required
