#define _WIN32_WINNT 0x0601

#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <algorithm>
#include <ctime>
#include <cstdlib>
#include <cctype>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

// ============================================================
// CAR STRUCTURE
// ============================================================

struct Car
{
    int id;
    string brand;
    string model;
    string rateDay;
    string rateHour;
    string fuel;
    string seats;
    string year;
    string status;
    string image;
};

vector<Car> cars;

// ============================================================
// PATHS
// ============================================================

string BASE_DIR;
string PUBLIC_DIR;
string DATABASE_DIR;

string CARS_FILE;
string RENTALS_FILE;
string BOOKINGS_FILE;

// ============================================================
// BASIC HELPERS
// ============================================================

string trim(const string &s)
{
    size_t start = 0;

    while (start < s.size() &&
           isspace((unsigned char)s[start]))
    {
        start++;
    }

    size_t end = s.size();

    while (end > start &&
           isspace((unsigned char)s[end - 1]))
    {
        end--;
    }

    return s.substr(start, end - start);
}

vector<string> split(const string &text, char delimiter)
{
    vector<string> result;
    string item;
    stringstream ss(text);

    while (getline(ss, item, delimiter))
    {
        result.push_back(item);
    }

    return result;
}

string jsonEscape(const string &s)
{
    string out;

    for (char c : s)
    {
        if (c == '\\')
            out += "\\\\";
        else if (c == '"')
            out += "\\\"";
        else if (c == '\n')
            out += "\\n";
        else if (c == '\r')
            out += "\\r";
        else
            out += c;
    }

    return out;
}

string getJsonValue(const string &json, const string &key)
{
    string search = "\"" + key + "\"";

    size_t pos = json.find(search);

    if (pos == string::npos)
        return "";

    pos = json.find(':', pos);

    if (pos == string::npos)
        return "";

    pos++;

    while (pos < json.size() &&
           isspace((unsigned char)json[pos]))
    {
        pos++;
    }

    if (pos >= json.size())
        return "";

    if (json[pos] == '"')
    {
        pos++;

        string result;

        while (pos < json.size())
        {
            if (json[pos] == '"' &&
                (pos == 0 || json[pos - 1] != '\\'))
            {
                break;
            }

            if (json[pos] == '\\' &&
                pos + 1 < json.size())
            {
                pos++;

                if (json[pos] == '"')
                    result += '"';
                else if (json[pos] == '\\')
                    result += '\\';
                else if (json[pos] == 'n')
                    result += '\n';
                else
                    result += json[pos];
            }
            else
            {
                result += json[pos];
            }

            pos++;
        }

        return result;
    }

    size_t end = pos;

    while (end < json.size() &&
           json[end] != ',' &&
           json[end] != '}')
    {
        end++;
    }

    return trim(json.substr(pos, end - pos));
}

int toInt(const string &s)
{
    try
    {
        return stoi(s);
    }
    catch (...)
    {
        return 0;
    }
}

// ============================================================
// GET EXE DIRECTORY
// ============================================================

string getExeDirectory()
{
    char buffer[MAX_PATH];

    DWORD length =
        GetModuleFileNameA(
            NULL,
            buffer,
            MAX_PATH
        );

    if (length == 0)
        return ".";

    string path(buffer, length);

    size_t pos = path.find_last_of("\\/");

    if (pos == string::npos)
        return ".";

    return path.substr(0, pos);
}

// ============================================================
// LOAD CARS
// ============================================================

void loadCars()
{
    cars.clear();

    ifstream file(CARS_FILE);

    if (!file.is_open())
        return;

    string line;

    while (getline(file, line))
    {
        if (line.empty())
            continue;

        vector<string> p = split(line, '|');

        if (p.size() < 9)
            continue;

        Car c;

        c.id = toInt(p[0]);
        c.brand = p[1];
        c.model = p[2];
        c.rateDay = p[3];
        c.rateHour = p[4];
        c.fuel = p[5];
        c.seats = p[6];
        c.year = p[7];
        c.status = p[8];

        if (p.size() >= 10)
            c.image = p[9];
        else
            c.image = "";

        cars.push_back(c);
    }

    file.close();
}

// ============================================================
// SAVE CARS
// ============================================================

void saveCars()
{
    ofstream file(CARS_FILE);

    if (!file.is_open())
        return;

    for (const Car &c : cars)
    {
        file
            << c.id << "|"
            << c.brand << "|"
            << c.model << "|"
            << c.rateDay << "|"
            << c.rateHour << "|"
            << c.fuel << "|"
            << c.seats << "|"
            << c.year << "|"
            << c.status << "|"
            << c.image
            << "\n";
    }

    file.close();
}

// ============================================================
// FIND CAR
// ============================================================

Car* findCar(int id)
{
    for (auto &c : cars)
    {
        if (c.id == id)
            return &c;
    }

    return nullptr;
}

// ============================================================
// AUTOMATIC RETURN
// ============================================================

bool parseDateTime(const string &dateTime, tm &result)
{
    if (dateTime.size() < 16)
        return false;

    try
    {
        result = {};

        result.tm_year =
            stoi(dateTime.substr(0, 4)) - 1900;

        result.tm_mon =
            stoi(dateTime.substr(5, 2)) - 1;

        result.tm_mday =
            stoi(dateTime.substr(8, 2));

        result.tm_hour =
            stoi(dateTime.substr(11, 2));

        result.tm_min =
            stoi(dateTime.substr(14, 2));

        result.tm_sec = 0;

        result.tm_isdst = -1;

        return true;
    }
    catch (...)
    {
        return false;
    }
}

void autoReturnExpiredRentals()
{
    ifstream file(RENTALS_FILE);

    if (!file.is_open())
        return;

    vector<string> updatedRentals;

    string line;

    bool changed = false;

    time_t now = time(nullptr);

    while (getline(file, line))
    {
        if (line.empty())
            continue;

        vector<string> p = split(line, '|');

        if (p.size() >= 9)
        {
            string status = trim(p[8]);

            if (status == "ACTIVE")
            {
                string returnDate = trim(p[4]);

                tm returnTime{};

                if (parseDateTime(returnDate, returnTime))
                {
                    time_t endTime =
                        mktime(&returnTime);

                    if (endTime <= now)
                    {
                        // Rental becomes returned
                        p[8] = "RETURNED";

                        // Car becomes available
                        int carId = toInt(p[2]);

                        Car *car = findCar(carId);

                        if (car != nullptr)
                        {
                            car->status = "Available";
                        }

                        changed = true;
                    }
                }
            }
        }

        string rebuilt;

        for (size_t i = 0; i < p.size(); i++)
        {
            if (i > 0)
                rebuilt += "|";

            rebuilt += p[i];
        }

        updatedRentals.push_back(rebuilt);
    }

    file.close();

    if (changed)
    {
        ofstream out(RENTALS_FILE);

        for (const string &r : updatedRentals)
        {
            out << r << "\n";
        }

        out.close();

        saveCars();
    }
}

// ============================================================
// CARS JSON
// ============================================================

string carsJson()
{
    stringstream json;

    json << "[";

    for (size_t i = 0; i < cars.size(); i++)
    {
        const Car &c = cars[i];

        if (i > 0)
            json << ",";

        json << "{";

        json << "\"id\":" << c.id << ",";

        json << "\"brand\":\""
             << jsonEscape(c.brand)
             << "\",";

        json << "\"model\":\""
             << jsonEscape(c.model)
             << "\",";

        json << "\"rateDay\":\""
             << jsonEscape(c.rateDay)
             << "\",";

        json << "\"rateHour\":\""
             << jsonEscape(c.rateHour)
             << "\",";

        json << "\"fuel\":\""
             << jsonEscape(c.fuel)
             << "\",";

        json << "\"seats\":\""
             << jsonEscape(c.seats)
             << "\",";

        json << "\"year\":\""
             << jsonEscape(c.year)
             << "\",";

        json << "\"status\":\""
             << jsonEscape(c.status)
             << "\",";

        json << "\"image\":\""
             << jsonEscape(c.image)
             << "\"";

        json << "}";
    }

    json << "]";

    return json.str();
}

// ============================================================
// RENTALS JSON
// ============================================================

string rentalsJson()
{
    ifstream file(RENTALS_FILE);

    stringstream json;

    json << "[";

    bool first = true;

    string line;

    while (getline(file, line))
    {
        if (line.empty())
            continue;

        vector<string> p = split(line, '|');

        if (p.size() < 9)
            continue;

        int carId = toInt(p[2]);

        string carName = "Unknown Car";

        Car *car = findCar(carId);

        if (car != nullptr)
        {
            carName =
                car->brand + " " + car->model;
        }

        if (!first)
            json << ",";

        first = false;

        json << "{";

        json << "\"name\":\""
             << jsonEscape(p[0])
             << "\",";

        json << "\"phone\":\""
             << jsonEscape(p[1])
             << "\",";

        json << "\"carId\":"
             << carId
             << ",";

        json << "\"car\":\""
             << jsonEscape(carName)
             << "\",";

        json << "\"pickup\":\""
             << jsonEscape(p[3])
             << "\",";

        json << "\"returnDate\":\""
             << jsonEscape(p[4])
             << "\",";

        json << "\"hours\":\""
             << jsonEscape(p[5])
             << "\",";

        json << "\"days\":\""
             << jsonEscape(p[6])
             << "\",";

        json << "\"amount\":\""
             << jsonEscape(p[7])
             << "\",";

        json << "\"status\":\""
             << jsonEscape(p[8])
             << "\"";

        json << "}";
    }

    json << "]";

    return json.str();
}

// ============================================================
// DASHBOARD JSON
// ============================================================

string dashboardJson()
{
    int totalCars = (int)cars.size();

    int availableCars = 0;

    for (const Car &c : cars)
    {
        if (c.status == "Available")
            availableCars++;
    }

    int totalRentals = 0;

    int activeRentals = 0;

    double totalRevenue = 0;

    ifstream file(RENTALS_FILE);

    string line;

    while (getline(file, line))
    {
        if (line.empty())
            continue;

        vector<string> p = split(line, '|');

        if (p.size() < 9)
            continue;

        totalRentals++;

        if (p[8] == "ACTIVE")
            activeRentals++;

        try
        {
            totalRevenue += stod(p[7]);
        }
        catch (...)
        {
        }
    }

    file.close();

    stringstream json;

    json << "{";

    json << "\"totalCars\":"
         << totalCars
         << ",";

    json << "\"availableCars\":"
         << availableCars
         << ",";

    json << "\"activeRentals\":"
         << activeRentals
         << ",";

    json << "\"totalRentals\":"
         << totalRentals
         << ",";

    json << "\"totalRevenue\":"
         << totalRevenue;

    json << "}";

    return json.str();
}

// ============================================================
// MIME TYPE
// ============================================================

string getMimeType(const string &path)
{
    size_t dot = path.find_last_of('.');

    if (dot == string::npos)
        return "text/plain";

    string ext = path.substr(dot);

    if (ext == ".html")
        return "text/html";

    if (ext == ".css")
        return "text/css";

    if (ext == ".js")
        return "application/javascript";

    if (ext == ".jpg" ||
        ext == ".jpeg")
        return "image/jpeg";

    if (ext == ".png")
        return "image/png";

    if (ext == ".webp")
        return "image/webp";

    if (ext == ".gif")
        return "image/gif";

    if (ext == ".svg")
        return "image/svg+xml";

    return "application/octet-stream";
}

// ============================================================
// SEND RESPONSE
// ============================================================

void sendResponse(
    SOCKET client,
    const string &body,
    const string &contentType = "text/html",
    int statusCode = 200,
    const string &statusText = "OK"
)
{
    string response;

    response =
        "HTTP/1.1 " +
        to_string(statusCode) +
        " " +
        statusText +
        "\r\n";

    response +=
        "Content-Type: " +
        contentType +
        "; charset=UTF-8\r\n";

    response +=
        "Content-Length: " +
        to_string(body.size()) +
        "\r\n";

    response +=
        "Access-Control-Allow-Origin: *\r\n";

    response +=
        "Access-Control-Allow-Methods: GET, POST, DELETE, OPTIONS\r\n";

    response +=
        "Access-Control-Allow-Headers: Content-Type\r\n";

    response +=
        "Connection: close\r\n\r\n";

    response += body;

    send(
        client,
        response.c_str(),
        (int)response.size(),
        0
    );
}

// ============================================================
// READ STATIC FILE
// ============================================================

void serveFile(
    SOCKET client,
    string path
)
{
    if (path == "/")
        path = "/index.html";

    if (path.find("..") != string::npos)
    {
        sendResponse(
            client,
            "Forbidden",
            "text/plain",
            403,
            "Forbidden"
        );

        return;
    }

    string fullPath =
        PUBLIC_DIR + path;

    ifstream file(
        fullPath,
        ios::binary
    );

    if (!file.is_open())
    {
        sendResponse(
            client,
            "File Not Found",
            "text/plain",
            404,
            "Not Found"
        );

        return;
    }

    stringstream buffer;

    buffer << file.rdbuf();

    string content = buffer.str();

    file.close();

    sendResponse(
        client,
        content,
        getMimeType(fullPath)
    );
}

// ============================================================
// HANDLE CLIENT
// ============================================================

void handleClient(SOCKET client)
{
    char buffer[65536];

    int received =
        recv(
            client,
            buffer,
            sizeof(buffer) - 1,
            0
        );

    if (received <= 0)
    {
        closesocket(client);
        return;
    }

    buffer[received] = '\0';

    string request(buffer, received);

    size_t firstSpace =
        request.find(' ');

    size_t secondSpace =
        request.find(
            ' ',
            firstSpace + 1
        );

    if (firstSpace == string::npos ||
        secondSpace == string::npos)
    {
        closesocket(client);
        return;
    }

    string method =
        request.substr(
            0,
            firstSpace
        );

    string path =
        request.substr(
            firstSpace + 1,
            secondSpace - firstSpace - 1
        );

    // Remove query string
    size_t question =
        path.find('?');

    if (question != string::npos)
        path = path.substr(0, question);

    // Request body
    string body;

    size_t bodyPos =
        request.find("\r\n\r\n");

    if (bodyPos != string::npos)
    {
        body =
            request.substr(
                bodyPos + 4
            );
    }

    // OPTIONS
    if (method == "OPTIONS")
    {
        sendResponse(
            client,
            "",
            "text/plain"
        );

        closesocket(client);
        return;
    }

    // Always check expired rentals
    autoReturnExpiredRentals();

    // ========================================================
    // GET CARS
    // ========================================================

    if (method == "GET" &&
        path == "/api/cars")
    {
        sendResponse(
            client,
            carsJson(),
            "application/json"
        );

        closesocket(client);
        return;
    }

    // ========================================================
    // GET DASHBOARD
    // ========================================================

    if (method == "GET" &&
        path == "/api/dashboard")
    {
        sendResponse(
            client,
            dashboardJson(),
            "application/json"
        );

        closesocket(client);
        return;
    }

    // ========================================================
    // GET RENTALS
    // ========================================================

    if (method == "GET" &&
        path == "/api/rentals")
    {
        sendResponse(
            client,
            rentalsJson(),
            "application/json"
        );

        closesocket(client);
        return;
    }

    // ========================================================
    // ADD CAR
    // POST /api/cars
    // ========================================================

    if (method == "POST" &&
        path == "/api/cars")
    {
        string brand =
            getJsonValue(body, "brand");

        string model =
            getJsonValue(body, "model");

        string rateDay =
            getJsonValue(body, "rateDay");

        string rateHour =
            getJsonValue(body, "rateHour");

        string fuel =
            getJsonValue(body, "fuel");

        string seats =
            getJsonValue(body, "seats");

        string year =
            getJsonValue(body, "year");

        string image =
            getJsonValue(body, "image");

        if (brand.empty() ||
            model.empty())
        {
            sendResponse(
                client,
                "{\"success\":false,\"message\":\"Brand and model are required\"}",
                "application/json",
                400,
                "Bad Request"
            );

            closesocket(client);
            return;
        }

        int newId = 100;

        for (const Car &c : cars)
        {
            if (c.id >= newId)
                newId = c.id + 1;
        }

        Car c;

        c.id = newId;
        c.brand = brand;
        c.model = model;
        c.rateDay = rateDay.empty() ? "0" : rateDay;
        c.rateHour = rateHour.empty() ? "0" : rateHour;
        c.fuel = fuel.empty() ? "Petrol" : fuel;
        c.seats = seats.empty() ? "5" : seats;
        c.year = year.empty() ? "2026" : year;
        c.status = "Available";
        c.image = image;

        cars.push_back(c);

        saveCars();

        sendResponse(
            client,
            "{\"success\":true,\"message\":\"Car added successfully\"}",
            "application/json"
        );

        closesocket(client);
        return;
    }

    // ========================================================
    // RENT CAR
    // POST /api/rentals
    // ========================================================

    if (method == "POST" &&
        path == "/api/rentals")
    {
        string name =
            getJsonValue(body, "name");

        if (name.empty())
            name = getJsonValue(body, "customerName");

        string phone =
            getJsonValue(body, "phone");

        string carIdText =
            getJsonValue(body, "carId");

        string pickup =
            getJsonValue(body, "pickup");

        string returnDate =
            getJsonValue(body, "returnDate");

        string hours =
            getJsonValue(body, "hours");

        string days =
            getJsonValue(body, "days");

        string amount =
            getJsonValue(body, "amount");

        int carId =
            toInt(carIdText);

        Car *car =
            findCar(carId);

        if (car == nullptr)
        {
            sendResponse(
                client,
                "{\"success\":false,\"message\":\"Car not found\"}",
                "application/json",
                404,
                "Not Found"
            );

            closesocket(client);
            return;
        }

        if (car->status != "Available")
        {
            sendResponse(
                client,
                "{\"success\":false,\"message\":\"Car is already rented\"}",
                "application/json",
                400,
                "Bad Request"
            );

            closesocket(client);
            return;
        }

        if (name.empty() ||
            phone.empty() ||
            pickup.empty() ||
            returnDate.empty())
        {
            sendResponse(
                client,
                "{\"success\":false,\"message\":\"Please fill all rental details\"}",
                "application/json",
                400,
                "Bad Request"
            );

            closesocket(client);
            return;
        }

        ofstream file(
            RENTALS_FILE,
            ios::app
        );

        file
            << name << "|"
            << phone << "|"
            << carId << "|"
            << pickup << "|"
            << returnDate << "|"
            << hours << "|"
            << days << "|"
            << amount << "|"
            << "ACTIVE"
            << "\n";

        file.close();

        car->status = "Rented";

        saveCars();

        sendResponse(
            client,
            "{\"success\":true,\"message\":\"Car rented successfully\"}",
            "application/json"
        );

        closesocket(client);
        return;
    }

    // ========================================================
    // MANUAL RETURN
    // POST /api/rentals/return/{id}
    // ========================================================

    string returnPrefix =
        "/api/rentals/return/";

    if (method == "POST" &&
        path.find(returnPrefix) == 0)
    {
        string idText =
            path.substr(
                returnPrefix.size()
            );

        int carId =
            toInt(idText);

        Car *car =
            findCar(carId);

        if (car == nullptr)
        {
            sendResponse(
                client,
                "{\"success\":false,\"message\":\"Car not found\"}",
                "application/json",
                404,
                "Not Found"
            );

            closesocket(client);
            return;
        }

        ifstream file(
            RENTALS_FILE
        );

        vector<string> lines;

        string line;

        bool returned = false;

        while (getline(file, line))
        {
            vector<string> p =
                split(line, '|');

            if (p.size() >= 9 &&
                toInt(p[2]) == carId &&
                p[8] == "ACTIVE" &&
                !returned)
            {
                p[8] = "RETURNED";

                string updated;

                for (size_t i = 0;
                     i < p.size();
                     i++)
                {
                    if (i > 0)
                        updated += "|";

                    updated += p[i];
                }

                lines.push_back(updated);

                returned = true;
            }
            else
            {
                lines.push_back(line);
            }
        }

        file.close();

        if (returned)
        {
            ofstream out(
                RENTALS_FILE
            );

            for (const string &l : lines)
                out << l << "\n";

            out.close();

            car->status = "Available";

            saveCars();

            sendResponse(
                client,
                "{\"success\":true,\"message\":\"Car returned successfully\"}",
                "application/json"
            );
        }
        else
        {
            sendResponse(
                client,
                "{\"success\":false,\"message\":\"No active rental found\"}",
                "application/json",
                400,
                "Bad Request"
            );
        }

        closesocket(client);
        return;
    }

    // ========================================================
    // DELETE CAR
    // DELETE /api/cars/{id}
    // ========================================================

    string deletePrefix =
        "/api/cars/";

    if (method == "DELETE" &&
        path.find(deletePrefix) == 0)
    {
        string idText =
            path.substr(
                deletePrefix.size()
            );

        int carId =
            toInt(idText);

        Car *car =
            findCar(carId);

        if (car == nullptr)
        {
            sendResponse(
                client,
                "{\"success\":false,\"message\":\"Car not found\"}",
                "application/json",
                404,
                "Not Found"
            );

            closesocket(client);
            return;
        }

        if (car->status == "Rented")
        {
            sendResponse(
                client,
                "{\"success\":false,\"message\":\"Rented car cannot be deleted. Return it first.\"}",
                "application/json",
                400,
                "Bad Request"
            );

            closesocket(client);
            return;
        }

        cars.erase(
            remove_if(
                cars.begin(),
                cars.end(),
                [carId](const Car &c)
                {
                    return c.id == carId;
                }
            ),
            cars.end()
        );

        saveCars();

        sendResponse(
            client,
            "{\"success\":true,\"message\":\"Car deleted successfully\"}",
            "application/json"
        );

        closesocket(client);
        return;
    }

    // ========================================================
    // RESET DATA
    // POST /api/reset
    // ========================================================

    if (method == "POST" &&
        path == "/api/reset")
    {
        for (auto &c : cars)
        {
            c.status = "Available";
        }

        saveCars();

        ofstream rentals(
            RENTALS_FILE,
            ios::trunc
        );

        rentals.close();

        ofstream bookings(
            BOOKINGS_FILE,
            ios::trunc
        );

        bookings.close();

        sendResponse(
            client,
            "{\"success\":true,\"message\":\"Dashboard and rental data reset successfully\"}",
            "application/json"
        );

        closesocket(client);
        return;
    }

    // ========================================================
    // STATIC FILE
    // ========================================================

    serveFile(
        client,
        path
    );

    closesocket(client);
}

// ============================================================
// MAIN
// ============================================================

int main()
{
    BASE_DIR =
        getExeDirectory();

    PUBLIC_DIR =
        BASE_DIR + "\\public";

    DATABASE_DIR =
        BASE_DIR + "\\database";

    CARS_FILE =
        DATABASE_DIR + "\\cars.txt";

    RENTALS_FILE =
        DATABASE_DIR + "\\rentals.txt";

    BOOKINGS_FILE =
        DATABASE_DIR + "\\bookings.txt";

    CreateDirectoryA(
        DATABASE_DIR.c_str(),
        NULL
    );

    CreateDirectoryA(
        PUBLIC_DIR.c_str(),
        NULL
    );

    loadCars();

    // ========================================================
    // WINSOCK START
    // ========================================================

    WSADATA wsaData;

    if (WSAStartup(
            MAKEWORD(2, 2),
            &wsaData) != 0)
    {
        cout << "WSAStartup failed." << endl;
        return 1;
    }

    SOCKET serverSocket =
        socket(
            AF_INET,
            SOCK_STREAM,
            IPPROTO_TCP
        );

    if (serverSocket == INVALID_SOCKET)
    {
        cout << "Socket creation failed." << endl;

        WSACleanup();

        return 1;
    }

    sockaddr_in serverAddress{};

    serverAddress.sin_family =
        AF_INET;

    serverAddress.sin_addr.s_addr =
        inet_addr("127.0.0.1");

    serverAddress.sin_port =
        htons(8080);

    if (bind(
            serverSocket,
            (sockaddr *)&serverAddress,
            sizeof(serverAddress)) == SOCKET_ERROR)
    {
        cout << "Bind failed. Port 8080 may already be in use."
             << endl;

        closesocket(serverSocket);

        WSACleanup();

        return 1;
    }

    if (listen(
            serverSocket,
            SOMAXCONN) == SOCKET_ERROR)
    {
        cout << "Listen failed." << endl;

        closesocket(serverSocket);

        WSACleanup();

        return 1;
    }

    cout << "======================================" << endl;
    cout << "       DRIVEON CAR RENTAL" << endl;
    cout << "======================================" << endl;
    cout << "Server running at:" << endl;
    cout << "http://127.0.0.1:8080/" << endl;
    cout << "======================================" << endl;

    while (true)
    {
        SOCKET client =
            accept(
                serverSocket,
                NULL,
                NULL
            );

        if (client == INVALID_SOCKET)
            continue;

        handleClient(client);
    }

    closesocket(serverSocket);

    WSACleanup();

    return 0;
}