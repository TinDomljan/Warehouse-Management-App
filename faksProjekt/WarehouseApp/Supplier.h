#pragma once
#include <string>

class Supplier
{
public:
    Supplier();
    Supplier(int id, std::string companyName, std::string contactPerson,
        std::string email, std::string phone, std::string address);

    // Getters
    int getId() const;
    std::string getCompanyName() const;
    std::string getContactPerson() const;
    std::string getEmail() const;
    std::string getPhone() const;
    std::string getAddress() const;

    // Setters
    void setCompanyName(const std::string& companyName);
    void setContactPerson(const std::string& contactPerson);
    void setEmail(const std::string& email);
    void setPhone(const std::string& phone);
    void setAddress(const std::string& address);

    // Utility
    std::string toString() const;



private:
    int id_;
    std::string companyName_;
    std::string contactPerson_;
    std::string email_;
    std::string phone_;
    std::string address_;
};

