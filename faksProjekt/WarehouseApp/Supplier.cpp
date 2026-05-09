#include "Supplier.h"

Supplier::Supplier() {
    id_ = 0;
    companyName_ = "";
    contactPerson_ = "";
    email_ = "";
    phone_ = "";
    address_ = "";
}

Supplier::Supplier(int id, std::string companyName, std::string contactPerson,
    std::string email, std::string phone, std::string address) {
    id_ = id;
    companyName_ = companyName;
    contactPerson_ = contactPerson;
    email_ = email;
    phone_ = phone;
    address_ = address;
}

int Supplier::getId() const {
    return id_;
}

std::string Supplier::getCompanyName() const {
    return companyName_;
}

std::string Supplier::getContactPerson() const {
    return contactPerson_;
}

std::string Supplier::getEmail() const {
    return email_;
}

std::string Supplier::getPhone() const {
    return phone_;
}

std::string Supplier::getAddress() const {
    return address_;
}

void Supplier::setCompanyName(const std::string& companyName) {
    companyName_ = companyName;
}

void Supplier::setContactPerson(const std::string& contactPerson) {
    contactPerson_ = contactPerson;
}

void Supplier::setEmail(const std::string& email) {
    email_ = email;
}

void Supplier::setPhone(const std::string& phone) {
    phone_ = phone;
}

void Supplier::setAddress(const std::string& address) {
    address_ = address;
}

std::string Supplier::toString() const {
    return "[" + std::to_string(id_) + "] " + companyName_ +
        " (Contact: " + contactPerson_ + ", " + email_ + ", " + phone_ + ")";
}