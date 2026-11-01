// ============================================================
// DRIVEON CAR RENTAL - FRONTEND
// ============================================================

let allCars = [];

let selectedCar = null;


// ============================================================
// DEFAULT IMAGE MAP
// ============================================================

const carImages = {

    "Toyota Fortuner":
        "images/fortuner.jpg",

    "Hyundai Creta":
        "images/creta.jpg",

    "Mahindra Thar":
        "images/thar.jpg",

    "Kia Seltos":
        "images/seltos.jpg",

    "Tata Nexon":
        "images/nexon.jpg",

    "Honda City":
        "images/city.jpg",

    "Audi":
        "images/audi.jpg",

    "BMW":
        "images/bmw.jpg",

    "Mercedes":
        "images/mercedes.jpg",

    "Hyundai Verna":
        "images/verna.jpg",

    "Mahindra XUV700":
        "images/xuv700.jpg"
};


// ============================================================
// GET CAR IMAGE
// ONLY IMAGE CHANGE
// ============================================================

function getCarImage(car) {

    const fullName =
        `${car.brand || ""} ${car.model || ""}`.trim();

    const name =
        fullName.toLowerCase().replace(/\s+/g, " ");


    if (name.includes("fortuner"))
        return "images/fortuner.jpg";


    if (name.includes("creta"))
        return "images/creta.jpg";


    if (name.includes("thar"))
        return "images/thar.jpg";


    if (name.includes("seltos"))
        return "images/seltos.jpg";


    if (name.includes("nexon"))
        return "images/nexon.jpg";


    if (name.includes("city"))
        return "images/city.jpg";


    if (name.includes("audi"))
        return "images/audi.jpg";


    if (name.includes("bmw"))
        return "images/bmw.jpg";


    if (name.includes("mercedes"))
        return "images/mercedes.jpg";


    if (name.includes("verna"))
        return "images/verna.jpg";


    if (
        name.includes("xuv700") ||
        name.includes("xuv 700") ||
        name.includes("xuv")
    )
        return "images/xuv700.jpg";


    // Database image
    if (
        car.image &&
        car.image.trim() !== ""
    )
        return car.image;


    return "";
}


// ============================================================
// PAGE LOAD
// ============================================================

document.addEventListener(
    "DOMContentLoaded",
    function () {

        loadCars();

        loadDashboard();

        loadRentals();

        setMinimumDate();

        // Real-time update
        setInterval(
            function () {

                loadCars();

                loadDashboard();

                loadRentals();

            },
            5000
        );

    }
);


// ============================================================
// LOAD CARS
// ============================================================

async function loadCars() {

    try {

        const response =
            await fetch("/api/cars");

        if (!response.ok)
            throw new Error("Cars API failed");

        allCars =
            await response.json();

        displayCars(allCars);

    }
    catch (error) {

        console.error(error);

    }

}


// ============================================================
// DISPLAY CARS
// ============================================================

function displayCars(cars) {

    const container =
        document.getElementById(
            "carsContainer"
        );

    container.innerHTML = "";

    if (cars.length === 0) {

        container.innerHTML = `
            <div style="
                grid-column:1/-1;
                text-align:center;
                padding:60px;
                color:#777;
            ">
                <h3>No cars available.</h3>
                <p>Add a new car using the + Add Car button.</p>
            </div>
        `;

        return;
    }


    cars.forEach(
        function (car) {

            const fullName =
                `${car.brand} ${car.model}`;


            // =================================================
            // IMAGE FIX
            // =================================================

            const image =
                getCarImage(car);


            const isAvailable =
                car.status === "Available";


            const statusClass =
                isAvailable
                    ? "available"
                    : "rented";


            const statusText =
                isAvailable
                    ? "Available"
                    : "Rented";


            const card =
                document.createElement("div");

            card.className =
                "car-card";


            card.innerHTML = `

                <img
                    class="car-image"
                    src="${image || "images/bmw.jpg"}"
                    alt="${escapeHtml(fullName)}"
                    onerror="this.onerror=null; this.src='images/bmw.jpg';">

                <div class="car-body">

                    <div class="car-brand">
                        ${escapeHtml(car.brand)}
                    </div>

                    <div class="car-model">
                        ${escapeHtml(car.model)}
                    </div>

                    <span
                        class="status ${statusClass}">
                        ${statusText}
                    </span>

                    <div class="car-details">

                        <div class="detail">
                            Fuel: ${escapeHtml(car.fuel)}
                        </div>

                        <div class="detail">
                            Seats: ${escapeHtml(car.seats)}
                        </div>

                        <div class="detail">
                            Year: ${escapeHtml(car.year)}
                        </div>

                        <div class="detail">
                            ID: ${car.id}
                        </div>

                    </div>

                    <div class="price">
                        ₹${escapeHtml(car.rateDay)}/day
                    </div>

                    <div class="card-buttons">

                        <button
                            class="rent-btn"
                            ${isAvailable ? "" : "disabled"}
                            onclick="openRentModal(${car.id})">

                            ${isAvailable
                                ? "Rent Car"
                                : "Rented"}

                        </button>

                        <button
                            class="delete-btn"
                            onclick="deleteCar(${car.id})">

                            Delete

                        </button>

                    </div>

                </div>
            `;


            container.appendChild(card);

        }
    );
}


// ============================================================
// SEARCH
// ============================================================

function searchCars() {

    const value =
        document
            .getElementById("searchInput")
            .value
            .toLowerCase()
            .trim();


    const filtered =
        allCars.filter(
            function (car) {

                const text =
                    `${car.brand} ${car.model} ${car.fuel}`
                        .toLowerCase();

                return text.includes(value);

            }
        );


    displayCars(filtered);
}


// ============================================================
// OPEN RENT MODAL
// ============================================================

function openRentModal(id) {

    selectedCar =
        allCars.find(
            car => car.id === id
        );


    if (!selectedCar)
        return;


    if (selectedCar.status !== "Available") {

        alert(
            "This car is already rented."
        );

        return;
    }


    document
        .getElementById("rentCarId")
        .value = selectedCar.id;


    document
        .getElementById("selectedCarText")
        .textContent =
        `${selectedCar.brand} ${selectedCar.model} - ₹${selectedCar.rateDay}/day`;


    document
        .getElementById("rentModal")
        .classList
        .add("show");


    setMinimumDate();

    calculateAmount();

}


// ============================================================
// CLOSE RENT MODAL
// ============================================================

function closeRentModal() {

    document
        .getElementById("rentModal")
        .classList
        .remove("show");

}


// ============================================================
// SET MINIMUM DATE
// ============================================================

function setMinimumDate() {

    const now =
        new Date();

    now.setMinutes(
        now.getMinutes() -
        now.getTimezoneOffset()
    );

    const value =
        now
            .toISOString()
            .slice(0, 16);


    const pickup =
        document.getElementById(
            "pickupDate"
        );

    const returnDate =
        document.getElementById(
            "returnDate"
        );


    if (pickup) {

        pickup.min = value;

        if (!pickup.value)
            pickup.value = value;

    }


    if (returnDate) {

        returnDate.min = value;

    }

}


// ============================================================
// CALCULATE AMOUNT
// ============================================================

function calculateAmount() {

    if (!selectedCar)
        return;


    const pickup =
        document
            .getElementById("pickupDate")
            .value;


    const returnDate =
        document
            .getElementById("returnDate")
            .value;


    if (!pickup || !returnDate) {

        document
            .getElementById("estimatedAmount")
            .textContent = "₹0";

        return;
    }


    const start =
        new Date(pickup);

    const end =
        new Date(returnDate);


    if (end <= start) {

        document
            .getElementById("estimatedAmount")
            .textContent = "₹0";

        return;
    }


    const diff =
        end.getTime() -
        start.getTime();


    const hours =
        Math.ceil(
            diff /
            (1000 * 60 * 60)
        );


    const days =
        Math.ceil(
            hours / 24
        );


    let amount = 0;


    const dayRate =
        Number(
            selectedCar.rateDay
        ) || 0;


    const hourRate =
        Number(
            selectedCar.rateHour
        ) || 0;


    if (hours >= 24) {

        amount =
            days * dayRate;

    }
    else {

        amount =
            hours * hourRate;

    }


    document
        .getElementById("estimatedAmount")
        .textContent =
        `₹${amount}`;
}


// ============================================================
// SUBMIT RENTAL
// ============================================================

async function submitRental() {

    const name =
        document
            .getElementById("customerName")
            .value
            .trim();


    const phone =
        document
            .getElementById("phone")
            .value
            .trim();


    const pickup =
        document
            .getElementById("pickupDate")
            .value;


    const returnDate =
        document
            .getElementById("returnDate")
            .value;


    if (!name ||
        !phone ||
        !pickup ||
        !returnDate) {

        alert(
            "Please fill all rental details."
        );

        return;
    }


    const start =
        new Date(pickup);

    const end =
        new Date(returnDate);


    if (end <= start) {

        alert(
            "Return time must be after pickup time."
        );

        return;
    }


    const diff =
        end.getTime() -
        start.getTime();


    const hours =
        Math.ceil(
            diff /
            (1000 * 60 * 60)
        );


    const days =
        Math.ceil(
            hours / 24
        );


    const dayRate =
        Number(
            selectedCar.rateDay
        ) || 0;


    const hourRate =
        Number(
            selectedCar.rateHour
        ) || 0;


    let amount = 0;


    if (hours >= 24) {

        amount =
            days * dayRate;

    }
    else {

        amount =
            hours * hourRate;

    }


    const data = {

        name: name,

        phone: phone,

        carId: selectedCar.id,

        pickup: pickup,

        returnDate: returnDate,

        hours: hours,

        days: days,

        amount: amount

    };


    try {

        const response =
            await fetch(
                "/api/rentals",
                {
                    method: "POST",

                    headers: {
                        "Content-Type":
                            "application/json"
                    },

                    body:
                        JSON.stringify(data)
                }
            );


        const result =
            await response.json();


        if (!response.ok ||
            !result.success) {

            alert(
                result.message ||
                "Rental failed."
            );

            return;
        }


        closeRentModal();


        showSuccess(
            "Car rented successfully!"
        );


        clearRentForm();


        await loadCars();

        await loadDashboard();

        await loadRentals();

    }
    catch (error) {

        console.error(error);

        alert(
            "Server connection error."
        );

    }

}


// ============================================================
// CLEAR RENT FORM
// ============================================================

function clearRentForm() {

    document
        .getElementById("customerName")
        .value = "";

    document
        .getElementById("phone")
        .value = "";

    document
        .getElementById("returnDate")
        .value = "";

    document
        .getElementById("estimatedAmount")
        .textContent = "₹0";

}


// ============================================================
// LOAD DASHBOARD
// ============================================================

async function loadDashboard() {

    try {

        const response =
            await fetch(
                "/api/dashboard"
            );


        const data =
            await response.json();


        document
            .getElementById("totalCars")
            .textContent =
            data.totalCars;


        document
            .getElementById("availableCars")
            .textContent =
            data.availableCars;


        document
            .getElementById("activeRentals")
            .textContent =
            data.activeRentals;


        document
            .getElementById("totalRentals")
            .textContent =
            data.totalRentals;


        document
            .getElementById("totalRevenue")
            .textContent =
            "₹" +
            Number(
                data.totalRevenue || 0
            ).toLocaleString("en-IN");

    }
    catch (error) {

        console.error(
            "Dashboard error:",
            error
        );

    }

}


// ============================================================
// LOAD RENTAL HISTORY
// ============================================================

async function loadRentals() {

    try {

        const response =
            await fetch(
                "/api/rentals"
            );


        const rentals =
            await response.json();


        const table =
            document.getElementById(
                "rentalTable"
            );


        table.innerHTML = "";


        if (rentals.length === 0) {

            table.innerHTML = `
                <tr>
                    <td colspan="8"
                        style="
                            text-align:center;
                            padding:40px;
                            color:#777;
                        ">
                        No rental history yet.
                    </td>
                </tr>
            `;

            return;
        }


        rentals
            .slice()
            .reverse()
            .forEach(
                function (rental) {

                    const tr =
                        document.createElement("tr");


                    const statusClass =
                        rental.status === "RETURNED"
                            ? "history-returned"
                            : "history-active";


                    let action = "";


                    if (rental.status === "ACTIVE") {

                        action = `
                            <button
                                class="return-btn"
                                onclick="manualReturn(${rental.carId})">
                                Return
                            </button>
                        `;

                    }
                    else {

                        action = `
                            <span style="color:#777;">
                                Completed
                            </span>
                        `;

                    }


                    tr.innerHTML = `

                        <td>
                            ${escapeHtml(rental.name)}
                        </td>

                        <td>
                            ${escapeHtml(rental.phone)}
                        </td>

                        <td>
                            ${escapeHtml(rental.car)}
                        </td>

                        <td>
                            ${formatDate(rental.pickup)}
                        </td>

                        <td>
                            ${formatDate(rental.returnDate)}
                        </td>

                        <td>
                            ₹${escapeHtml(rental.amount)}
                        </td>

                        <td>
                            <span
                                class="${statusClass}">
                                ${escapeHtml(rental.status)}
                            </span>
                        </td>

                        <td>
                            ${action}
                        </td>

                    `;


                    table.appendChild(tr);

                }
            );

    }
    catch (error) {

        console.error(
            "Rental history error:",
            error
        );

    }

}


// ============================================================
// MANUAL RETURN
// ============================================================

async function manualReturn(carId) {

    const confirmReturn =
        confirm(
            "Are you sure you want to return this car?"
        );


    if (!confirmReturn)
        return;


    try {

        const response =
            await fetch(
                `/api/rentals/return/${carId}`,
                {
                    method: "POST"
                }
            );


        const result =
            await response.json();


        if (!result.success) {

            alert(
                result.message ||
                "Return failed."
            );

            return;
        }


        showSuccess(
            "Car returned successfully!"
        );


        await loadCars();

        await loadDashboard();

        await loadRentals();

    }
    catch (error) {

        console.error(error);

        alert(
            "Server connection error."
        );

    }

}


// ============================================================
// DELETE CAR
// ============================================================

async function deleteCar(id) {

    const car =
        allCars.find(
            c => c.id === id
        );


    if (!car)
        return;


    if (car.status === "Rented") {

        alert(
            "Rented car cannot be deleted. Return it first."
        );

        return;
    }


    const confirmed =
        confirm(
            `Delete ${car.brand} ${car.model}?`
        );


    if (!confirmed)
        return;


    try {

        const response =
            await fetch(
                `/api/cars/${id}`,
                {
                    method: "DELETE"
                }
            );


        const result =
            await response.json();


        if (!result.success) {

            alert(
                result.message ||
                "Delete failed."
            );

            return;
        }


        showSuccess(
            "Car deleted successfully!"
        );


        await loadCars();

        await loadDashboard();

    }
    catch (error) {

        console.error(error);

        alert(
            "Server connection error."
        );

    }

}


// ============================================================
// OPEN ADD CAR
// ============================================================

function openAddCarModal() {

    document
        .getElementById("addCarModal")
        .classList
        .add("show");

}


// ============================================================
// CLOSE ADD CAR
// ============================================================

function closeAddCarModal() {

    document
        .getElementById("addCarModal")
        .classList
        .remove("show");

}


// ============================================================
// ADD CAR
// ============================================================

async function addCar() {

    const brand =
        document
            .getElementById("carBrand")
            .value
            .trim();


    const model =
        document
            .getElementById("carModel")
            .value
            .trim();


    const rateDay =
        document
            .getElementById("carRateDay")
            .value
            .trim();


    const rateHour =
        document
            .getElementById("carRateHour")
            .value
            .trim();


    const fuel =
        document
            .getElementById("carFuel")
            .value;


    const seats =
        document
            .getElementById("carSeats")
            .value;


    const year =
        document
            .getElementById("carYear")
            .value;


    const image =
        document
            .getElementById("carImage")
            .value
            .trim();


    if (!brand ||
        !model ||
        !rateDay ||
        !rateHour) {

        alert(
            "Please fill Brand, Model and Rates."
        );

        return;
    }


    const data = {

        brand: brand,

        model: model,

        rateDay: rateDay,

        rateHour: rateHour,

        fuel: fuel,

        seats: seats,

        year: year,

        image: image

    };


    try {

        const response =
            await fetch(
                "/api/cars",
                {
                    method: "POST",

                    headers: {
                        "Content-Type":
                            "application/json"
                    },

                    body:
                        JSON.stringify(data)
                }
            );


        const result =
            await response.json();


        if (!result.success) {

            alert(
                result.message ||
                "Could not add car."
            );

            return;
        }


        closeAddCarModal();


        clearAddCarForm();


        showSuccess(
            "New car added successfully!"
        );


        await loadCars();

        await loadDashboard();

    }
    catch (error) {

        console.error(error);

        alert(
            "Server connection error."
        );

    }

}


// ============================================================
// CLEAR ADD CAR FORM
// ============================================================

function clearAddCarForm() {

    document
        .getElementById("carBrand")
        .value = "";

    document
        .getElementById("carModel")
        .value = "";

    document
        .getElementById("carRateDay")
        .value = "";

    document
        .getElementById("carRateHour")
        .value = "";

    document
        .getElementById("carSeats")
        .value = "5";

    document
        .getElementById("carYear")
        .value = "2026";

    document
        .getElementById("carImage")
        .value = "";

}


// ============================================================
// RESET DASHBOARD
// ============================================================

async function resetDashboard() {

    const confirmed =
        confirm(
            "Reset dashboard?\n\nThis will remove rental history and make all cars Available."
        );


    if (!confirmed)
        return;


    try {

        const response =
            await fetch(
                "/api/reset",
                {
                    method: "POST"
                }
            );


        const result =
            await response.json();


        if (!result.success) {

            alert(
                result.message ||
                "Reset failed."
            );

            return;
        }


        showSuccess(
            "Dashboard and rental data reset successfully!"
        );


        await loadCars();

        await loadDashboard();

        await loadRentals();

    }
    catch (error) {

        console.error(error);

        alert(
            "Server connection error."
        );

    }

}


// ============================================================
// SUCCESS MODAL
// ============================================================

function showSuccess(message) {

    document
        .getElementById("successMessage")
        .textContent =
        message;


    document
        .getElementById("successModal")
        .classList
        .add("show");

}


function closeSuccessModal() {

    document
        .getElementById("successModal")
        .classList
        .remove("show");

}


// ============================================================
// NAVIGATION
// ============================================================

function goToCars() {

    document
        .getElementById("cars")
        .scrollIntoView({
            behavior: "smooth"
        });

}


// ============================================================
// FORMAT DATE
// ============================================================

function formatDate(value) {

    if (!value)
        return "-";


    const date =
        new Date(value);


    if (isNaN(date.getTime()))
        return value;


    return date.toLocaleString(
        "en-IN",
        {
            dateStyle: "short",
            timeStyle: "short"
        }
    );

}


// ============================================================
// ESCAPE HTML
// ============================================================

function escapeHtml(value) {

    if (value === null ||
        value === undefined)
        return "";


    return String(value)
        .replace(
            /&/g,
            "&amp;"
        )
        .replace(
            /</g,
            "&lt;"
        )
        .replace(
            />/g,
            "&gt;"
        )
        .replace(
            /"/g,
            "&quot;"
        )
        .replace(
            /'/g,
            "&#039;"
        );

}