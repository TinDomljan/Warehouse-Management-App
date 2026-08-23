#include "User.h"
#include "cryptomanager.h"
#include <QString>

User::User() {
    id_ = 0;
    username_ = "";
    fullName_ = "";
    password_ = "";
    role_ = UserRole::Clerk;
    loggedIn_ = false;
}

User::User(int id, std::string username, std::string fullName,
    std::string password, UserRole role) {
    id_ = id;
    username_ = username;
    fullName_ = fullName;
    password_ = password; //ovdje nije spremljena lozinka nego SHA-256 hash sa soli i paprom
    role_ = role;
    loggedIn_ = false;
}

// Getters
int User::getId() const {
    return id_;
}

std::string User::getUsername() const {
    return username_;
}

std::string User::getFullName() const {
    return fullName_;
}

UserRole User::getRole() const {
    return role_;
}

bool User::isLoggedIn() const {
    return loggedIn_;
}

// Setters
void User::setFullName(const std::string& fullName) {
    fullName_ = fullName;
}

void User::setPassword(const std::string& password) {
    password_ = password;
}

void User::setRole(UserRole role) {
    role_ = role;
}

// Autentifikacija
bool User::checkPassword(const std::string& password) const {


    const QString derivedSalt = CryptoManager::deriveSalt(QString::fromStdString(username_));
    return CryptoManager::verifyPasswordBruteForcePepper(
        QString::fromStdString(password),
        derivedSalt,
        QString::fromStdString(password_));
}

bool User::login(const std::string& password) {
    if (checkPassword(password)) {
        loggedIn_ = true;
        return true;
    }
    return false;
}

void User::logout() {
    loggedIn_ = false;
}


std::string User::getRoleAsString() const {
    switch (role_) {
    case UserRole::Admin:   return "Admin";
    case UserRole::Manager: return "Manager";
    case UserRole::Clerk:   return "Clerk";
    default:                return "Unknown";
    }
}

std::string User::toString() const {
    return "[" + std::to_string(id_) + "] " + username_ +
        " (" + fullName_ + ") | Role: " + getRoleAsString() +
        " | Status: " + (loggedIn_ ? "Online" : "Offline");
}