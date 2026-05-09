#include <iostream>
#include "Category.h"
#include "Supplier.h"
#include "Product.h"
#include "User.h"
#include "ActivityLog.h"

int main() {
    // --- Keep all previous tests ---
    Category electronics(1, "Electronics", "Battery-powered items");
    Supplier acme(1, "Acme Electronics", "Ivan Horvat",
        "ivan@acme.hr", "+385-1-234-5678", "Ilica 42, Zagreb");
    Product laptop(1, "Laptop HP ProBook", 899.99, 25, electronics, acme);
	User admin(1, "admin", "Administrator", "admin123", UserRole::Admin);
   
    ActivityLog log;

    log.addEntry(admin.getUsername(), "Created product", laptop.getName());
	log.addEntry(admin.getUsername(), "CREATE", "Product: " + laptop.getName());


	std::cout << "Product details:\n" << log.toString() << std::endl;

	std::vector<LogEntry> adminLog = log.getEntriesByUser(admin.getUsername());
    for (int i= 0; i < adminLog.size(); i++) {
        std::cout << log.formatEntry(adminLog[i]) << std::endl;
	}

	std::cout << "Total log entries: " << log.getEntryCount() << std::endl;
	log.deleteEntry(1);
	std::cout << log.toString() << std::endl;

    return 0;
}