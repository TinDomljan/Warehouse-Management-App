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
    return key;  // ako nema prijevoda vrati ključ
}

void TranslationManager::loadTranslations() {

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


    translations_["main_menu_data"]["ENG"] = "&Data";
    translations_["main_menu_data"]["HRV"] = "&Podaci";
    translations_["main_menu_orders"]["ENG"] = "Orders";
    translations_["main_menu_orders"]["HRV"] = "Narudžbe";

    // ===== USERS TAB / USER DIALOG =====
    translations_["users_tab"]["ENG"] = "Users";
    translations_["users_tab"]["HRV"] = "Korisnici";
    translations_["users_col_id"]["ENG"] = "ID";
    translations_["users_col_id"]["HRV"] = "ID";
    translations_["users_col_username"]["ENG"] = "Username";
    translations_["users_col_username"]["HRV"] = "Korisničko ime";
    translations_["users_col_fullname"]["ENG"] = "Full Name";
    translations_["users_col_fullname"]["HRV"] = "Ime i prezime";
    translations_["users_col_role"]["ENG"] = "Role";
    translations_["users_col_role"]["HRV"] = "Uloga";
    translations_["users_btn_add"]["ENG"] = "Add User";
    translations_["users_btn_add"]["HRV"] = "Dodaj Korisnika";
    translations_["users_btn_edit"]["ENG"] = "Edit User";
    translations_["users_btn_edit"]["HRV"] = "Uredi Korisnika";
    translations_["users_btn_delete"]["ENG"] = "Delete User";
    translations_["users_btn_delete"]["HRV"] = "Obriši Korisnika";
    translations_["user_add_title"]["ENG"] = "Add New User";
    translations_["user_add_title"]["HRV"] = "Dodaj Novog Korisnika";
    translations_["user_edit_title"]["ENG"] = "Edit User";
    translations_["user_edit_title"]["HRV"] = "Uredi Korisnika";
    translations_["user_username"]["ENG"] = "Username:";
    translations_["user_username"]["HRV"] = "Korisničko ime:";
    translations_["user_fullname"]["ENG"] = "Full Name:";
    translations_["user_fullname"]["HRV"] = "Ime i prezime:";
    translations_["user_password"]["ENG"] = "Password:";
    translations_["user_password"]["HRV"] = "Lozinka:";
    translations_["user_role"]["ENG"] = "Role:";
    translations_["user_role"]["HRV"] = "Uloga:";
    translations_["user_ph_username"]["ENG"] = "Enter username...";
    translations_["user_ph_username"]["HRV"] = "Unesite korisničko ime...";
    translations_["user_ph_fullname"]["ENG"] = "Enter full name...";
    translations_["user_ph_fullname"]["HRV"] = "Unesite ime i prezime...";
    translations_["user_ph_password"]["ENG"] = "Enter password...";
    translations_["user_ph_password"]["HRV"] = "Unesite lozinku...";
    translations_["user_ph_password_edit"]["ENG"] = "Leave empty to keep current password";
    translations_["user_ph_password_edit"]["HRV"] = "Ostavite prazno za zadržavanje lozinke";
    translations_["user_btn_save"]["ENG"] = "Save";
    translations_["user_btn_save"]["HRV"] = "Spremi";
    translations_["user_btn_cancel"]["ENG"] = "Cancel";
    translations_["user_btn_cancel"]["HRV"] = "Odustani";
    translations_["user_err_title"]["ENG"] = "Validation Error";
    translations_["user_err_title"]["HRV"] = "Greška validacije";
    translations_["user_err_username"]["ENG"] = "Please enter a username.";
    translations_["user_err_username"]["HRV"] = "Molimo unesite korisničko ime.";
    translations_["user_err_fullname"]["ENG"] = "Please enter a full name.";
    translations_["user_err_fullname"]["HRV"] = "Molimo unesite ime i prezime.";
    translations_["user_err_password"]["ENG"] = "Please enter a password.";
    translations_["user_err_password"]["HRV"] = "Molimo unesite lozinku.";
    translations_["user_select_edit"]["ENG"] = "Please select a user to edit.";
    translations_["user_select_edit"]["HRV"] = "Odaberite korisnika za uređivanje.";
    translations_["user_select_delete"]["ENG"] = "Please select a user to delete.";
    translations_["user_select_delete"]["HRV"] = "Odaberite korisnika za brisanje.";
    translations_["user_delete_title"]["ENG"] = "Delete User";
    translations_["user_delete_title"]["HRV"] = "Brisanje Korisnika";
    translations_["user_delete_confirm"]["ENG"] = "Are you sure you want to delete user ";
    translations_["user_delete_confirm"]["HRV"] = "Jeste li sigurni da želite obrisati korisnika ";
    translations_["user_err_self_delete"]["ENG"] = "You cannot delete your own account.";
    translations_["user_err_self_delete"]["HRV"] = "Ne možete obrisati vlastiti račun.";
    translations_["user_err_last_admin"]["ENG"] = "Cannot delete the last remaining Admin account.";
    translations_["user_err_last_admin"]["HRV"] = "Nije moguće obrisati zadnji Admin račun.";
    translations_["user_err_duplicate"]["ENG"] = "A user with that username already exists.";
    translations_["user_err_duplicate"]["HRV"] = "Korisnik s tim korisničkim imenom već postoji.";
    translations_["user_op_failed"]["ENG"] = "The operation failed. Please try again.";
    translations_["user_op_failed"]["HRV"] = "Operacija nije uspjela. Pokušajte ponovno.";


    translations_["order_title"]["ENG"] = "Orders";
    translations_["order_title"]["HRV"] = "Narudžbe";
    translations_["order_col_id"]["ENG"] = "ID";
    translations_["order_col_id"]["HRV"] = "ID";
    translations_["order_col_customer"]["ENG"] = "Customer";
    translations_["order_col_customer"]["HRV"] = "Kupac";
    translations_["order_col_date"]["ENG"] = "Date";
    translations_["order_col_date"]["HRV"] = "Datum";
    translations_["order_col_status"]["ENG"] = "Status";
    translations_["order_col_status"]["HRV"] = "Status";
    translations_["order_col_items"]["ENG"] = "Items";
    translations_["order_col_items"]["HRV"] = "Stavke";
    translations_["order_btn_add"]["ENG"] = "Add";
    translations_["order_btn_add"]["HRV"] = "Dodaj";
    translations_["order_btn_edit"]["ENG"] = "Edit";
    translations_["order_btn_edit"]["HRV"] = "Uredi";
    translations_["order_btn_delete"]["ENG"] = "Delete";
    translations_["order_btn_delete"]["HRV"] = "Obriši";
    translations_["order_add_title"]["ENG"] = "Add Order";
    translations_["order_add_title"]["HRV"] = "Dodaj Narudžbu";
    translations_["order_edit_title"]["ENG"] = "Edit Order";
    translations_["order_edit_title"]["HRV"] = "Uredi Narudžbu";
    translations_["order_customer"]["ENG"] = "Customer:";
    translations_["order_customer"]["HRV"] = "Kupac:";
    translations_["order_date"]["ENG"] = "Order Date:";
    translations_["order_date"]["HRV"] = "Datum narudžbe:";
    translations_["order_status"]["ENG"] = "Status:";
    translations_["order_status"]["HRV"] = "Status:";
    translations_["order_items"]["ENG"] = "Items:";
    translations_["order_items"]["HRV"] = "Stavke:";
    translations_["order_item_col_pid"]["ENG"] = "Product ID";
    translations_["order_item_col_pid"]["HRV"] = "ID proizvoda";
    translations_["order_item_col_pname"]["ENG"] = "Product Name";
    translations_["order_item_col_pname"]["HRV"] = "Naziv proizvoda";
    translations_["order_item_col_qty"]["ENG"] = "Quantity";
    translations_["order_item_col_qty"]["HRV"] = "Količina";
    translations_["order_item_add"]["ENG"] = "Add Item";
    translations_["order_item_add"]["HRV"] = "Dodaj Stavku";
    translations_["order_item_remove"]["ENG"] = "Remove Item";
    translations_["order_item_remove"]["HRV"] = "Ukloni Stavku";
    translations_["order_btn_save"]["ENG"] = "Save";
    translations_["order_btn_save"]["HRV"] = "Spremi";
    translations_["order_btn_cancel"]["ENG"] = "Cancel";
    translations_["order_btn_cancel"]["HRV"] = "Odustani";
    translations_["order_ph_customer"]["ENG"] = "Enter customer name...";
    translations_["order_ph_customer"]["HRV"] = "Unesite ime kupca...";
    translations_["order_err_title"]["ENG"] = "Validation Error";
    translations_["order_err_title"]["HRV"] = "Greška validacije";
    translations_["order_err_customer"]["ENG"] = "Please enter a customer name.";
    translations_["order_err_customer"]["HRV"] = "Molimo unesite ime kupca.";
    translations_["order_err_items"]["ENG"] = "Each item must have quantity greater than 0.";
    translations_["order_err_items"]["HRV"] = "Svaka stavka mora imati količinu veću od 0.";
    translations_["order_select_edit"]["ENG"] = "Please select an order to edit.";
    translations_["order_select_edit"]["HRV"] = "Odaberite narudžbu za uređivanje.";
    translations_["order_select_delete"]["ENG"] = "Please select an order to delete.";
    translations_["order_select_delete"]["HRV"] = "Odaberite narudžbu za brisanje.";
    translations_["order_confirm_delete"]["ENG"] = "Are you sure you want to delete order for ";
    translations_["order_confirm_delete"]["HRV"] = "Jeste li sigurni da želite obrisati narudžbu za ";
    translations_["order_delete_title"]["ENG"] = "Delete Order";
    translations_["order_delete_title"]["HRV"] = "Brisanje Narudžbe";


    translations_["about_title"]["ENG"] = "About Warehouse App";
    translations_["about_title"]["HRV"] = "O aplikaciji";
    translations_["about_text"]["ENG"] = "Warehouse Management System\nVersion 1.0\n\nDeveloped for Advanced Programming Techniques\nZagreb University of Applied Sciences\n\nAuthor: Tindo";
    translations_["about_text"]["HRV"] = "Sustav Upravljanja Skladištem\nVerzija 1.0\n\nRazvijeno za kolegij Napredne Tehnike Programiranja\nTehničko Veleučilište u Zagrebu\n\nAutor: Tindo";
}