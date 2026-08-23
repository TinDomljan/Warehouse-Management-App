#include "Category.h"


Category::Category() {
	id_ = 0;
	name_ = "";
	description_ = "";

}


Category::Category(int id, std::string name, std::string description) {
	id_ = id;
	name_ = name;
	description_ = description;
}

int Category::getId() const {
	return id_;
}

std::string Category::getName() const {
	return name_;
}

std::string Category::getDescription() const {
	return description_;
}


void Category::setName(const std::string& name) {
	name_ = name;
}

void Category::setDescription(const std::string& description) {
	description_ = description;
}

std::string Category::toString() const {
	return "[" + std::to_string(id_) + "] " + name_ + " - " + description_;
}