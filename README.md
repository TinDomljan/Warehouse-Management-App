# WarehouseApp

Desktop application for managing inventory in a small or medium-sized business. Built with C++17 and Qt 6, it covers products, suppliers, orders, and inventory analysis, with networking, cryptography, and reporting capabilities baked in.

Developed as a university project for the Advanced Programming Techniques course at TVZ Zagreb.

## What It Does

WarehouseApp lets you track stock levels, manage suppliers and orders, run parallel inventory analysis, generate PDF reports, and communicate over TCP/UDP/HTTP. It also includes AES and RSA encryption, SHA-256 password hashing with salt and pepper, and digital signature support.

There are three user defined roles, mostly based on how much visibility and functionality they have over the app itself:

**Administrator** has full access to everything, including network services, cryptographic operations, and clearing the activity log

**Manager** can manage products, suppliers, and orders, run analyses, and generate reports, but has no access to networking or crypto features

**Clerk** has read-only access to products, suppliers, orders, and the activity log

## Project Structure

The project builds three executables and four libraries from a single CMake configuration:

```
WarehouseApp/
├── WarehouseApp          # Main GUI application
├── WarehouseServer       # TCP/UDP server (console app, port 23117)
├── WarehouseValidator    # Binary file validator (console app, no Qt dependency)
├── WarehouseUtils        # Static library (currency formatting, barcode validation, stock calculations)
├── WarehouseExport       # Shared library (CSV and HTML export)
├── WarehouseDlgs         # Shared library (About dialog, margin calculator dialog)
└── WarehouseResources    # Shared library (PNG icons generated at build time)
```

## Features

### Data Management

- **SQLite database** with four tables (products, categories, suppliers, users), full CRUD on each
- **XML** (DOM) for orders with nested items, supporting read/write/edit/delete
- **JSON** for activity logging with timestamped user actions
- **Custom binary format** (`.bin`) for archiving inventory analysis snapshots, with a `WHSE` magic header, version field, and record count
- **Sorting** by name, price, quantity, or total value via SQL `ORDER BY`
- **Filtering** by product name, category, or supplier via `WHERE ... LIKE`
- **Calculated field**: total value computed as `price * quantity` in the SQL query
- **Lookup fields**: category name and supplier name pulled in through `LEFT JOIN`

### User Interface

- **Six dialogs**: LoginDialog, ProductDialog, SettingsDialog, SupplierDialog, OrderDialog, and MainWindow
- **Localization** in Croatian and English across all five main dialogs, using a singleton `TranslationManager` with nested `std::map` lookups (language -> key -> translation)
- **Settings** stored in two places: visual preferences (font size, text color, background color, language) go to `settings.ini`, while window state and last username go to the Windows Registry. Both managed through a `SettingsManager` singleton using `QSettings`

### Reports

- PDF report generation via `QPrinter`, showing all products grouped by category (master-detail), with per-product details including supplier, quantity, unit price, and total value

### Multithreading

- **Parallel inventory analysis** splits the product list across 3 `InventoryAnalyzer` threads using `QThreadPool` and `QRunnable`
- Each thread counts low-stock items (threshold < 10), calculates total segment value, and finds the most expensive product in its range
- **Performance comparison**: the same analysis runs sequentially first, then in parallel, with `QElapsedTimer` measuring both to compute a speedup factor
- **CPU-bound work**: uses a volatile `sqrt` loop (not `sleep_for`) for genuine parallelism demonstration
- **Safe GUI updates**: worker signals cross thread boundaries via `Qt::QueuedConnection`, so the progress bar and status labels only update on the main thread
- **QMutex** protects a shared `processedCount` counter (polled every 100ms by a `QTimer` to drive the `QProgressBar`)
- **std::mutex** with `std::lock_guard` protects concurrent writes to `analysis_result.txt`

### Inter-Process Communication

- **WarehouseValidator** is a standalone console app (no Qt dependency) that validates `stock_snapshot.bin` files by checking magic bytes, version, and record count. Launched from WarehouseApp via `QProcess`, it returns exit code 0 for valid files, 1 for errors.

### Networking

- **TCP**: client in WarehouseApp sends a barcode or product name; WarehouseServer queries the database and returns product data as JSON. Also supports binary file transfer (sending `stock_snapshot.bin` as a stream)
- **UDP**: client sends a `STATUS` datagram to port 23118; server responds with JSON containing total products, low-stock count, and total inventory value. Also supports sending activity log entries as binary `LogPacket` datagrams
- **HTTP**: downloads a large CSV product catalog from a remote URL with progress tracking. Speed throttling is implemented via a `QTimer` that reads limited chunks every 200ms, with selectable limits (Unlimited, 100 KB/s, 50 KB/s). Uses `Accept-Encoding: identity` to prevent compressed responses from breaking byte-level throttling
- **REST**: connects to the Exchange Rates API (`open.er-api.com/v6/latest/EUR`) and displays current exchange rates in a formatted table

### Cryptography

All crypto is implemented through OpenSSL:

- **AES-256-CBC** encrypts the binary snapshot file. The key is derived from a user password via SHA-256. A random 16-byte IV is generated with `RAND_bytes()` and stored at the start of the encrypted file
- **RSA-2048** encrypts/decrypts user data exports (usernames and roles serialized to JSON). Key pair (`private.pem`, `public.pem`) is generated on first launch
- **SHA-256 password hashing** with a deterministic salt derived from `SHA-256(username.toLower() + "WHSE_SALT_RULE_v1")` (first 16 bytes, never stored in the database). Pepper is checked by iterating through 10 candidate values until a match is found
- **Digital signatures**: the orders file (`orders.xml`) is signed with the RSA private key using `EVP_DigestSign` with SHA-256. Verification uses `EVP_DigestVerify` with the public key

### Libraries

- **WarehouseUtils** (static): `formatCurrency()` for EUR formatting, `validateBarcode()` for EAN-8/EAN-13 validation, and `StockCalculator` class with `calculateTurnoverRate()` and `calculateReorderPoint()`
- **WarehouseExport** (shared): `exportToCSV()`, `exportToHTML()`, and a `DataFormatter` class with `formatReport()` for product data export
- **WarehouseDlgs** (shared): two DLL dialogs loaded at runtime. `ShowAboutDialog()` displays app info with a warehouse icon hand-drawn using `QPainter`. `ShowQuickCalcDialog()` is a margin calculator with live-updating results via lambda-connected `valueChanged` signals
- **WarehouseResources** (shared): three PNG icons generated at build time by a small `IconGenerator` executable, embedded via Qt resources. `loadWarehouseIcon()` loads them for use in tabs and buttons

## Prerequisites

- **Qt 6** (tested with 6.11.0)
- **CMake 3.16+**
- **MinGW 64-bit** (or another C++17-capable compiler)
- **OpenSSL** (Win64 build, for AES/RSA/SHA-256)
- **SQLite** (bundled with Qt's QSQLITE driver)

## Building

```bash
mkdir build && cd build
cmake .. -G "MinGW Makefiles"
cmake --build .
```

Or open the project in Qt Creator and build from there.

The build produces: `WarehouseApp.exe`, `WarehouseServer.exe`, `WarehouseValidator.exe`, and the three DLLs (`WarehouseExport.dll`, `WarehouseDlgs.dll`, `WarehouseResources.dll`).

## Running

### Main Application

Launch `WarehouseApp.exe`. On first run, the database is created automatically. Default login credentials are set up during initialization.

### Server

WarehouseServer needs to run from a separate terminal (not from Qt Creator, since switching build targets would kill the process):

```bash
cd path\to\build\output
set PATH=C:\Qt\6.11.0\mingw_64\bin;%PATH%
WarehouseServer.exe
```

The server listens on TCP port 23117 and UDP port 23118.

### Validator

Used internally by WarehouseApp via the "Validate Backup" button, but can also be run manually:

```bash
WarehouseValidator.exe path\to\stock_snapshot.bin
```

## Tech Stack

| Component | Technology |
|-----------|------------|
| Language | C++17 |
| Framework | Qt 6 (Widgets, Xml, Sql, Network, PrintSupport) |
| Build System | CMake |
| Database | SQLite |
| Crypto | OpenSSL (libcrypto) |
| IDE | Qt Creator, Visual Studio Code |

## License

University project. Not intended for production use.
