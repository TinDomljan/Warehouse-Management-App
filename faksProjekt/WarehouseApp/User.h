#pragma once
#include <string>

enum class UserRole {
    Admin,
    Manager,
    Clerk
};

class User {
public:
    User();
    User(int id, std::string username, std::string fullName,
        std::string password, UserRole role);

    // Getters
    int getId() const;
    std::string getUsername() const;
    std::string getFullName() const;
    UserRole getRole() const;
    bool isLoggedIn() const;

    // Setters
    void setFullName(const std::string& fullName);
    void setPassword(const std::string& password);
    void setRole(UserRole role);

    // Authentication
    bool login(const std::string& password);
    void logout();
    bool checkPassword(const std::string& password) const;

    // Utility
    std::string getRoleAsString() const;
    std::string toString() const;

private:
    int id_;
    std::string username_;
    std::string fullName_;
    std::string password_;
    UserRole role_;
    bool loggedIn_;
};