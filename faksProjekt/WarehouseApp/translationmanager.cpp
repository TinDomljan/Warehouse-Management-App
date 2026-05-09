#include "translationmanager.h"

TranslationManager& TranslationManager::instance() {
    static TranslationManager inst;
    return inst;
}

TranslationManager::TranslationManager() {
    currentLanguage_ = "ENG";
    loadTranslations();
}

void TranslationManager::setLanguage(const std::string& lang) {
    currentLanguage_ = lang;
}

std::string TranslationManager::getLanguage() const {
    return currentLanguage_;
}

std::string TranslationManager::tr(const std::string& key) const {
    auto it = translations_.find(key);
    if (it != translations_.end()) {
        auto langIt = it->second.find(currentLanguage_);
        if (langIt != it->second.end()) {
            return langIt->second;
        }
    }
    return key;  // fallback: return the key itself if no translation found
}

void TranslationManager::loadTranslations() {
    // ===== LOGIN DIALOG =====
    translations_["login_title"]["ENG"] = "Warehouse App - Login";
    translations_["login_title"]["HRV"] = "Skladišna Aplikacija - Prijava";
    translations_["login_header"]["ENG"] = "Warehouse Management System";
    translations_["login_header"]["HRV"] = "Sustav Upravljanja Skladištem";
    translations_["login_username"]["ENG"] = "Username:";
    translations_["login_username"]["HRV"] = "Korisničko ime:";
    translations_["login_password"]["ENG"] = "Password:";
    translations_["login_password"]["HRV"] = "Lozinka:";
    translations_["login_btn"]["ENG"] = "Login";
    translations_["login_btn"]["HRV"] = "Prijava";
    translations_["login_ph_user"]["ENG"] = "Enter username...";
    translations_["login_ph_user"]["HRV"] = "Unesite korisničko ime...";
    translations_["login_ph_pass"]["ENG"] = "Enter password...";
    translations_["login_ph_pass"]["HRV"] = "Unesite lozinku...";
    translations_["login_err_empty"]["ENG"] = "Please enter both username and password.";
    translations_["login_err_empty"]["HRV"] = "Molimo unesite korisničko ime i lozinku.";
    translations_["login_err_invalid"]["ENG"] = "Invalid username or password.";
    translations_["login_err_invalid"]["HRV"] = "Neispravno korisničko ime ili lozinka.";
    translations_["login_err_title"]["ENG"] = "Login Error";
    translations_["login_err_title"]["HRV"] = "Greška prijave";
    translations_["login_failed_title"]["ENG"] = "Login Failed";
    translations_["login_failed_title"]["HRV"] = "Prijava neuspješna";

    // ===== MAIN WINDOW =====
    translations_["main_title"]["ENG"] = "Warehouse Management System";
    translations_["main_title"]["HRV"] = "Sustav Upravljanja Skladištem";
    translations_["main_logged_in"]["ENG"] = "Logged in: ";
    translations_["main_logged_in"]["HRV"] = "Prijavljen: ";
    translations_["main_col_id"]["ENG"] = "ID";
    translations_["main_col_id"]["HRV"] = "ID";
    translations_["main_col_name"]["ENG"] = "Name";
    translations_["main_col_name"]["HRV"] = "Naziv";
    translations_["main_col_price"]["ENG"] = "Price";
    translations_["main_col_price"]["HRV"] = "Cijena";
    translations_["main_col_qty"]["ENG"] = "Quantity";
    translations_["main_col_qty"]["HRV"] = "Količina";
    translations_["main_col_value"]["ENG"] = "Value";
    translations_["main_col_value"]["HRV"] = "Vrijednost";
    translations_["main_col_category"]["ENG"] = "Category";
    translations_["main_col_category"]["HRV"] = "Kategorija";
    translations_["main_col_supplier"]["ENG"] = "Supplier";
    translations_["main_col_supplier"]["HRV"] = "Dobavljač";
    translations_["main_btn_add"]["ENG"] = "Add Product";
    translations_["main_btn_add"]["HRV"] = "Dodaj Proizvod";
    translations_["main_btn_remove"]["ENG"] = "Remove Product";
    translations_["main_btn_remove"]["HRV"] = "Ukloni Proizvod";
    translations_["main_btn_settings"]["ENG"] = "Settings";
    translations_["main_btn_settings"]["HRV"] = "Postavke";
    translations_["main_btn_about"]["ENG"] = "About";
    translations_["main_btn_about"]["HRV"] = "O aplikaciji";
    translations_["main_btn_logout"]["ENG"] = "Logout";
    translations_["main_btn_logout"]["HRV"] = "Odjava";
    translations_["main_menu_file"]["ENG"] = "&File";
    translations_["main_menu_file"]["HRV"] = "&Datoteka";
    translations_["main_menu_tools"]["ENG"] = "&Tools";
    translations_["main_menu_tools"]["HRV"] = "&Alati";
    translations_["main_menu_help"]["ENG"] = "&Help";
    translations_["main_menu_help"]["HRV"] = "&Pomoć";
    translations_["main_remove_select"]["ENG"] = "Please select a product to remove.";
    translations_["main_remove_select"]["HRV"] = "Odaberite proizvod za uklanjanje.";
    translations_["main_remove_confirm"]["ENG"] = "Are you sure you want to remove ";
    translations_["main_remove_confirm"]["HRV"] = "Jeste li sigurni da želite ukloniti ";
    translations_["main_confirm_delete"]["ENG"] = "Confirm Delete";
    translations_["main_confirm_delete"]["HRV"] = "Potvrda brisanja";
    translations_["main_product_removed"]["ENG"] = "Product removed: ";
    translations_["main_product_removed"]["HRV"] = "Proizvod uklonjen: ";
    translations_["main_logout_confirm"]["ENG"] = "Are you sure you want to logout?";
    translations_["main_logout_confirm"]["HRV"] = "Jeste li sigurni da se želite odjaviti?";
    translations_["main_settings_applied"]["ENG"] = "Settings applied";
    translations_["main_settings_applied"]["HRV"] = "Postavke primijenjene";
    translations_["main_welcome"]["ENG"] = "Welcome, ";
    translations_["main_welcome"]["HRV"] = "Dobrodošli, ";
    translations_["main_product_added"]["ENG"] = "Product added: ";
    translations_["main_product_added"]["HRV"] = "Proizvod dodan: ";

    // ===== PRODUCT DIALOG =====
    translations_["product_title"]["ENG"] = "Add New Product";
    translations_["product_title"]["HRV"] = "Dodaj Novi Proizvod";
    translations_["product_name"]["ENG"] = "Name:";
    translations_["product_name"]["HRV"] = "Naziv:";
    translations_["product_price"]["ENG"] = "Price:";
    translations_["product_price"]["HRV"] = "Cijena:";
    translations_["product_qty"]["ENG"] = "Quantity:";
    translations_["product_qty"]["HRV"] = "Količina:";
    translations_["product_category"]["ENG"] = "Category:";
    translations_["product_category"]["HRV"] = "Kategorija:";
    translations_["product_supplier"]["ENG"] = "Supplier:";
    translations_["product_supplier"]["HRV"] = "Dobavljač:";
    translations_["product_btn_save"]["ENG"] = "Save";
    translations_["product_btn_save"]["HRV"] = "Spremi";
    translations_["product_btn_cancel"]["ENG"] = "Cancel";
    translations_["product_btn_cancel"]["HRV"] = "Odustani";
    translations_["product_err_name"]["ENG"] = "Please enter a product name.";
    translations_["product_err_name"]["HRV"] = "Molimo unesite naziv proizvoda.";
    translations_["product_err_title"]["ENG"] = "Validation Error";
    translations_["product_err_title"]["HRV"] = "Greška validacije";
    translations_["product_ph_name"]["ENG"] = "Enter product name...";
    translations_["product_ph_name"]["HRV"] = "Unesite naziv proizvoda...";

    // ===== SETTINGS DIALOG =====
    translations_["settings_title"]["ENG"] = "Settings";
    translations_["settings_title"]["HRV"] = "Postavke";
    translations_["settings_font"]["ENG"] = "Font Size:";
    translations_["settings_font"]["HRV"] = "Veličina fonta:";
    translations_["settings_color"]["ENG"] = "Text Color:";
    translations_["settings_color"]["HRV"] = "Boja teksta:";
    translations_["settings_bg"]["ENG"] = "Background:";
    translations_["settings_bg"]["HRV"] = "Pozadina:";
    translations_["settings_lang"]["ENG"] = "Language:";
    translations_["settings_lang"]["HRV"] = "Jezik:";
    translations_["settings_preview"]["ENG"] = "Preview: This is how your text will look";
    translations_["settings_preview"]["HRV"] = "Pregled: Ovako će izgledati vaš tekst";
    translations_["settings_btn_apply"]["ENG"] = "Apply";
    translations_["settings_btn_apply"]["HRV"] = "Primijeni";
    translations_["settings_btn_cancel"]["ENG"] = "Cancel";
    translations_["settings_btn_cancel"]["HRV"] = "Odustani";
    translations_["settings_choose_color"]["ENG"] = "Choose Color...";
    translations_["settings_choose_color"]["HRV"] = "Odaberi boju...";
    translations_["settings_color_title"]["ENG"] = "Choose Text Color";
    translations_["settings_color_title"]["HRV"] = "Odaberite boju teksta";
    translations_["settings_bg_title"]["ENG"] = "Choose Background Color";
    translations_["settings_bg_title"]["HRV"] = "Odaberite boju pozadine";

    // ===== ABOUT DIALOG =====
    translations_["about_title"]["ENG"] = "About Warehouse App";
    translations_["about_title"]["HRV"] = "O aplikaciji";
    translations_["about_text"]["ENG"] = "Warehouse Management System\nVersion 1.0\n\nDeveloped for Advanced Programming Techniques\nZagreb University of Applied Sciences\n\nAuthor: Tindo";
    translations_["about_text"]["HRV"] = "Sustav Upravljanja Skladištem\nVerzija 1.0\n\nRazvijeno za kolegij Napredne Tehnike Programiranja\nTehničko Veleučilište u Zagrebu\n\nAutor: Tindo";
}