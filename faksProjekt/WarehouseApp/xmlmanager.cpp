#include "xmlmanager.h"

// Qt XML classes — our equivalents of the professor's TXMLDocument and IXMLNode
#include <QDomDocument>   // represents the entire XML document (the tree)
#include <QDomElement>    // represents a single XML element (<supplier>, <companyName>, etc.)
#include <QFile>          // for reading/writing the file on disk
#include <QTextStream>    // for writing formatted text to the file

XmlManager::XmlManager(const QString& filePath)
    : filePath_(filePath) {
    ensureFileExists();
}

// ============================================================
// HELPER: Create the XML file if it doesn't exist yet
// On first run, there's no suppliers.xml — we create an empty one
// ============================================================
void XmlManager::ensureFileExists() {
    QFile file(filePath_);
    if (!file.exists()) {
        // Build a minimal XML document with just the root element
        // Professor equivalent: xml->AddChild("suppliers")
        QDomDocument doc;

        // Processing instruction = the <?xml version="1.0"?> line at the top
        QDomProcessingInstruction header = doc.createProcessingInstruction(
            "xml", "version=\"1.0\" encoding=\"UTF-8\"");
        doc.appendChild(header);

        // Root element = <suppliers></suppliers>
        QDomElement root = doc.createElement("suppliers");
        doc.appendChild(root);

        // Write to disk
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream stream(&file);
            doc.save(stream, 4);  // 4 = indent with 4 spaces (human readable)
            file.close();
        }
    }
}

// ============================================================
// READ: Load all suppliers from the XML file
// This is the most important method — it demonstrates DOM tree navigation
//
// Professor's approach (Udžbenik str. 53):
//   IXMLNode* root = xml->DocumentElement;
//   for (int i = 0; i < root->ChildNodes->Count; i++) {
//       IXMLNode* node = root->ChildNodes->Nodes[i];
//       String ime = node->ChildNodes["ime"]->Text;
//   }
//
// Our approach: same logic, different class names
// ============================================================
std::vector<Supplier> XmlManager::loadSuppliers() {
    std::vector<Supplier> suppliers;

    // Step 1: Open the file and parse it into a DOM tree
    QFile file(filePath_);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return suppliers;  // file doesn't exist or can't be read
    }

    QDomDocument doc;
    if (!doc.setContent(&file)) {
        file.close();
        return suppliers;  // file exists but isn't valid XML
    }
    file.close();

    // Step 2: Get the root element (<suppliers>)
    // Professor equivalent: IXMLNode* root = xml->DocumentElement;
    QDomElement root = doc.documentElement();

    // Step 3: Iterate through all <supplier> child elements
    // Professor equivalent: for (i = 0; i < root->ChildNodes->Count; i++)
    QDomElement supplierElem = root.firstChildElement("supplier");
    while (!supplierElem.isNull()) {

        // Step 4: Read the "id" attribute from <supplier id="1">
        // Professor equivalent: node->GetAttribute("id")
        int id = supplierElem.attribute("id").toInt();

        // Step 5: Read text content of each child element
        // Professor equivalent: node->ChildNodes["companyName"]->Text
        std::string companyName = supplierElem.firstChildElement("companyName")
                                      .text().toStdString();
        std::string contactPerson = supplierElem.firstChildElement("contactPerson")
                                        .text().toStdString();
        std::string email = supplierElem.firstChildElement("email")
                                .text().toStdString();
        std::string phone = supplierElem.firstChildElement("phone")
                                .text().toStdString();
        std::string address = supplierElem.firstChildElement("address")
                                  .text().toStdString();

        // Step 6: Create a Supplier object from the extracted data
        suppliers.push_back(Supplier(id, companyName, contactPerson,
                                     email, phone, address));

        // Step 7: Move to the next <supplier> sibling element
        // Professor equivalent: incrementing the loop counter i++
        supplierElem = supplierElem.nextSiblingElement("supplier");
    }

    return suppliers;
}

// ============================================================
// CREATE: Add a new supplier to the XML file
//
// Flow: Load existing tree → create new element → add children → save
// Professor equivalent (Udžbenik str. 55):
//   IXMLNode* newNode = root->AddChild("kontakt");
//   newNode->AddChild("ime")->Text = "Pero";
// ============================================================
void XmlManager::addSupplier(const Supplier& supplier) {
    // Load the existing XML tree
    QFile file(filePath_);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;
    QDomDocument doc;
    doc.setContent(&file);
    file.close();

    QDomElement root = doc.documentElement();

    // Create a new <supplier> element with an id attribute
    // Professor equivalent: IXMLNode* newNode = root->AddChild("supplier");
    QDomElement newSupplier = doc.createElement("supplier");
    newSupplier.setAttribute("id", supplier.getId());

    // Create child elements for each field
    // Professor equivalent: newNode->AddChild("companyName")->Text = "Acme";
    // In Qt, we create the element, create a text node, append text to element,
    // then append element to parent. More verbose but same result.

    // Helper pattern: create element → create text → append text → append element
    auto addTextElement = [&](const QString& tagName, const QString& value) {
        QDomElement elem = doc.createElement(tagName);
        QDomText textNode = doc.createTextNode(value);
        elem.appendChild(textNode);
        newSupplier.appendChild(elem);
    };

    addTextElement("companyName", QString::fromStdString(supplier.getCompanyName()));
    addTextElement("contactPerson", QString::fromStdString(supplier.getContactPerson()));
    addTextElement("email", QString::fromStdString(supplier.getEmail()));
    addTextElement("phone", QString::fromStdString(supplier.getPhone()));
    addTextElement("address", QString::fromStdString(supplier.getAddress()));

    // Append the new <supplier> to the root <suppliers>
    root.appendChild(newSupplier);

    // Save the entire tree back to the file
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate))
        return;
    QTextStream stream(&file);
    doc.save(stream, 4);
    file.close();
}

// ============================================================
// UPDATE: Modify an existing supplier by finding it by ID
//
// Flow: Load tree → find element with matching ID → update children → save
// Professor doesn't show update explicitly, but the concept is:
// navigate to the node, change its Text property
// ============================================================
void XmlManager::updateSupplier(const Supplier& supplier) {
    QFile file(filePath_);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;
    QDomDocument doc;
    doc.setContent(&file);
    file.close();

    QDomElement root = doc.documentElement();

    // Find the <supplier> element with the matching id attribute
    QDomElement supplierElem = root.firstChildElement("supplier");
    while (!supplierElem.isNull()) {
        if (supplierElem.attribute("id").toInt() == supplier.getId()) {
            // Found it — update each child element's text content
            // We replace the old text node with a new one

            // Helper: find child element by tag name and replace its text
            auto updateField = [&](const QString& tagName, const QString& newValue) {
                QDomElement field = supplierElem.firstChildElement(tagName);
                if (!field.isNull()) {
                    // Remove old text node, add new one
                    while (field.hasChildNodes()) {
                        field.removeChild(field.firstChild());
                    }
                    field.appendChild(doc.createTextNode(newValue));
                }
            };

            updateField("companyName", QString::fromStdString(supplier.getCompanyName()));
            updateField("contactPerson", QString::fromStdString(supplier.getContactPerson()));
            updateField("email", QString::fromStdString(supplier.getEmail()));
            updateField("phone", QString::fromStdString(supplier.getPhone()));
            updateField("address", QString::fromStdString(supplier.getAddress()));

            break;  // found and updated, no need to continue
        }
        supplierElem = supplierElem.nextSiblingElement("supplier");
    }

    // Save
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate))
        return;
    QTextStream stream(&file);
    doc.save(stream, 4);
    file.close();
}

// ============================================================
// DELETE: Remove a supplier by ID
//
// Flow: Load tree → find element → remove from parent → save
// Professor equivalent: root->ChildNodes->Delete(index)
// ============================================================
void XmlManager::deleteSupplier(int id) {
    QFile file(filePath_);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;
    QDomDocument doc;
    doc.setContent(&file);
    file.close();

    QDomElement root = doc.documentElement();

    // Find the <supplier> element with the matching id
    QDomElement supplierElem = root.firstChildElement("supplier");
    while (!supplierElem.isNull()) {
        if (supplierElem.attribute("id").toInt() == id) {
            // removeChild removes the element from the tree
            // The element and all its children are destroyed
            root.removeChild(supplierElem);
            break;
        }
        supplierElem = supplierElem.nextSiblingElement("supplier");
    }

    // Save without the deleted element
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate))
        return;
    QTextStream stream(&file);
    doc.save(stream, 4);
    file.close();
}

// ============================================================
// HELPER: Find the next available ID
// Scans all existing suppliers and returns max(id) + 1
// ============================================================
int XmlManager::getNextId() {
    std::vector<Supplier> suppliers = loadSuppliers();
    int maxId = 0;
    for (int i = 0; i < suppliers.size(); i++) {
        if (suppliers[i].getId() > maxId) {
            maxId = suppliers[i].getId();
        }
    }
    return maxId + 1;
}