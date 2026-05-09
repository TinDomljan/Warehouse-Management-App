#pragma once
#include <string>

class Category {
public:
    
    Category();
    Category(int id, std::string name, std::string description);

    int getId() const;
    std::string getName() const;
    std::string getDescription() const;

   
    void setName(const std::string& name);
    void setDescription(const std::string& description);


    std::string toString() const;

private:
    int id_;
    std::string name_;
    std::string description_;
};