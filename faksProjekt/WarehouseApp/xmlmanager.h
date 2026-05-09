#ifndef XMLMANAGER_H
#define XMLMANAGER_H

#include <QString>
#include <QList>
#include "Supplier.h"

// XmlManager handles all XML file operations for supplier data
// Uses Qt's DOM (Document Object Model) approach:
// 1. Load entire XML file into memory as a tree
// 2. Navigate the tree to read/modify data
// 3. Save the tree back to file
//
// Professor's equivalent: TXMLDocument + IXMLNode (Udžbenik str. 52-58)
// Our equivalent: QDomDocument + QDomElement (Qt XML module)

class XmlManager {
public:
    XmlManager(const QString& filePath);

    // CRUD operations — the four operations the rubric requires
    // Create: add a new supplier to the XML file
    void addSupplier(const Supplier& supplier);

    // Read: load all suppliers from the XML file
    std::vector<Supplier> loadSuppliers();

    // Update: modify an existing supplier (matched by ID)
    void updateSupplier(const Supplier& supplier);

    // Delete: remove a supplier by ID
    void deleteSupplier(int id);

private:
    QString filePath_;

    // Helper: find the next available ID (max existing ID + 1)
    int getNextId();

    // Helper: create the XML file with an empty root element if it doesn't exist
    void ensureFileExists();
};

#endif // XMLMANAGER_H