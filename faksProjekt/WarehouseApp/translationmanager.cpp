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
    auto it = translations_.find(key); //find() a ne [] da se ne ubaci prazan element ako ne postoji
    if (it != translations_.end()) { //da li postoji kljuc
        auto langIt = it->second.find(currentLanguage_); //jezik
        if (langIt != it->second.end()) {
            return langIt->second; //vracamo tekst
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

    // ===== BOTTOM BAR: DATA MANAGEMENT BUTTONS =====
    translations_["main_btn_manage_orders"]["ENG"] = "Manage Orders";
    translations_["main_btn_manage_orders"]["HRV"] = "Upravljanje Narudžbama";
    translations_["main_btn_manage_logs"]["ENG"] = "Manage Log";
    translations_["main_btn_manage_logs"]["HRV"] = "Upravljanje Zapisnikom";

    // ===== LOG DIALOG (JSON CRUD) =====
    translations_["log_title"]["ENG"] = "Activity Log";
    translations_["log_title"]["HRV"] = "Zapisnik Aktivnosti";
    translations_["log_col_id"]["ENG"] = "ID";
    translations_["log_col_id"]["HRV"] = "ID";
    translations_["log_col_time"]["ENG"] = "Time";
    translations_["log_col_time"]["HRV"] = "Vrijeme";
    translations_["log_col_user"]["ENG"] = "User";
    translations_["log_col_user"]["HRV"] = "Korisnik";
    translations_["log_col_action"]["ENG"] = "Action";
    translations_["log_col_action"]["HRV"] = "Akcija";
    translations_["log_col_target"]["ENG"] = "Target";
    translations_["log_col_target"]["HRV"] = "Meta";
    translations_["log_btn_add"]["ENG"] = "Add";
    translations_["log_btn_add"]["HRV"] = "Dodaj";
    translations_["log_btn_edit"]["ENG"] = "Edit";
    translations_["log_btn_edit"]["HRV"] = "Uredi";
    translations_["log_btn_delete"]["ENG"] = "Delete";
    translations_["log_btn_delete"]["HRV"] = "Obriši";
    translations_["log_btn_clear"]["ENG"] = "Clear All";
    translations_["log_btn_clear"]["HRV"] = "Očisti Sve";
    translations_["log_btn_close"]["ENG"] = "Close";
    translations_["log_btn_close"]["HRV"] = "Zatvori";
    translations_["log_add_title"]["ENG"] = "Add Log Entry";
    translations_["log_add_title"]["HRV"] = "Dodaj Zapis";
    translations_["log_edit_title"]["ENG"] = "Edit Log Entry";
    translations_["log_edit_title"]["HRV"] = "Uredi Zapis";
    translations_["log_user"]["ENG"] = "User:";
    translations_["log_user"]["HRV"] = "Korisnik:";
    translations_["log_action"]["ENG"] = "Action:";
    translations_["log_action"]["HRV"] = "Akcija:";
    translations_["log_target"]["ENG"] = "Target:";
    translations_["log_target"]["HRV"] = "Meta:";
    translations_["log_time"]["ENG"] = "Time:";
    translations_["log_time"]["HRV"] = "Vrijeme:";
    translations_["log_ph_user"]["ENG"] = "Enter username...";
    translations_["log_ph_user"]["HRV"] = "Unesite korisničko ime...";
    translations_["log_ph_action"]["ENG"] = "Enter action...";
    translations_["log_ph_action"]["HRV"] = "Unesite akciju...";
    translations_["log_ph_target"]["ENG"] = "Enter target...";
    translations_["log_ph_target"]["HRV"] = "Unesite metu...";
    translations_["log_btn_save"]["ENG"] = "Save";
    translations_["log_btn_save"]["HRV"] = "Spremi";
    translations_["log_btn_cancel"]["ENG"] = "Cancel";
    translations_["log_btn_cancel"]["HRV"] = "Odustani";
    translations_["log_err_title"]["ENG"] = "Validation Error";
    translations_["log_err_title"]["HRV"] = "Greška validacije";
    translations_["log_err_user"]["ENG"] = "Please enter a username.";
    translations_["log_err_user"]["HRV"] = "Molimo unesite korisničko ime.";
    translations_["log_select_edit"]["ENG"] = "Please select an entry to edit.";
    translations_["log_select_edit"]["HRV"] = "Odaberite zapis za uređivanje.";
    translations_["log_select_delete"]["ENG"] = "Please select an entry to delete.";
    translations_["log_select_delete"]["HRV"] = "Odaberite zapis za brisanje.";
    translations_["log_delete_title"]["ENG"] = "Delete Log Entry";
    translations_["log_delete_title"]["HRV"] = "Brisanje Zapisa";
    translations_["log_delete_confirm"]["ENG"] = "Are you sure you want to delete the entry for ";
    translations_["log_delete_confirm"]["HRV"] = "Jeste li sigurni da želite obrisati zapis za ";
    translations_["log_clear_title"]["ENG"] = "Clear Log";
    translations_["log_clear_title"]["HRV"] = "Očisti Zapisnik";
    translations_["log_clear_confirm"]["ENG"] = "Are you sure you want to clear the entire activity log?";
    translations_["log_clear_confirm"]["HRV"] = "Jeste li sigurni da želite očistiti cijeli zapisnik aktivnosti?";


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

    // ===== TAB TITLES =====
    translations_["tab_products"]["ENG"] = "Products";
    translations_["tab_products"]["HRV"] = "Proizvodi";
    translations_["tab_snapshots"]["ENG"] = "Snapshots";
    translations_["tab_snapshots"]["HRV"] = "Snimke";
    translations_["tab_suppliers"]["ENG"] = "Suppliers";
    translations_["tab_suppliers"]["HRV"] = "Dobavljači";
    translations_["tab_network"]["ENG"] = "Network";
    translations_["tab_network"]["HRV"] = "Mreža";
    translations_["tab_crypto"]["ENG"] = "Crypto";
    translations_["tab_crypto"]["HRV"] = "Kriptografija";

    // ===== PRODUCTS TAB =====
    translations_["p_search"]["ENG"] = "Search:";
    translations_["p_search"]["HRV"] = "Pretraga:";
    translations_["p_sort"]["ENG"] = "Sort:";
    translations_["p_sort"]["HRV"] = "Sortiranje:";
    translations_["p_btn_manage"]["ENG"] = "Manage Product";
    translations_["p_btn_manage"]["HRV"] = "Upravljaj Proizvodom";
    translations_["p_btn_analyze"]["ENG"] = "Analyze Inventory";
    translations_["p_btn_analyze"]["HRV"] = "Analiziraj Zalihe";
    translations_["p_btn_csv"]["ENG"] = "Export CSV";
    translations_["p_btn_csv"]["HRV"] = "Izvoz CSV";
    translations_["p_btn_html"]["ENG"] = "Export HTML";
    translations_["p_btn_html"]["HRV"] = "Izvoz HTML";
    translations_["p_leadtime"]["ENG"] = "Lead time (days):";
    translations_["p_leadtime"]["HRV"] = "Vrijeme isporuke (dani):";
    translations_["p_btn_reorder"]["ENG"] = "Reorder Report";
    translations_["p_btn_reorder"]["HRV"] = "Izvještaj Narudžbi";
    translations_["sort_default"]["ENG"] = "Default (ID)";
    translations_["sort_default"]["HRV"] = "Zadano (ID)";
    translations_["sort_name_az"]["ENG"] = "Name A → Z";
    translations_["sort_name_az"]["HRV"] = "Naziv A → Z";
    translations_["sort_name_za"]["ENG"] = "Name Z → A";
    translations_["sort_name_za"]["HRV"] = "Naziv Z → A";
    translations_["sort_price_asc"]["ENG"] = "Price ↑";
    translations_["sort_price_asc"]["HRV"] = "Cijena ↑";
    translations_["sort_price_desc"]["ENG"] = "Price ↓";
    translations_["sort_price_desc"]["HRV"] = "Cijena ↓";
    translations_["sort_qty_asc"]["ENG"] = "Quantity ↑";
    translations_["sort_qty_asc"]["HRV"] = "Količina ↑";
    translations_["sort_qty_desc"]["ENG"] = "Quantity ↓";
    translations_["sort_qty_desc"]["HRV"] = "Količina ↓";
    translations_["sort_value_asc"]["ENG"] = "Value ↑";
    translations_["sort_value_asc"]["HRV"] = "Vrijednost ↑";
    translations_["sort_value_desc"]["ENG"] = "Value ↓";
    translations_["sort_value_desc"]["HRV"] = "Vrijednost ↓";
    translations_["sort_category"]["ENG"] = "Category";
    translations_["sort_category"]["HRV"] = "Kategorija";
    translations_["sort_supplier"]["ENG"] = "Supplier";
    translations_["sort_supplier"]["HRV"] = "Dobavljač";

    // ===== BOTTOM BAR =====
    translations_["b_quickcalc"]["ENG"] = "Quick Calculator";
    translations_["b_quickcalc"]["HRV"] = "Brzi Kalkulator";
    translations_["b_report"]["ENG"] = "Generate Report";
    translations_["b_report"]["HRV"] = "Generiraj Izvještaj";

    // ===== SNAPSHOTS TAB =====
    translations_["snap_info"]["ENG"] = "Save or load a binary snapshot of inventory-analysis results.\nRun 'Analyze Inventory' (Products tab) first, then save the per-thread\nanalysis segments to a custom binary file.";
    translations_["snap_info"]["HRV"] = "Spremi ili učitaj binarnu snimku rezultata analize zaliha.\nPrvo pokreni 'Analiziraj Zalihe' (tab Proizvodi), zatim spremi segmente\nanalize po dretvi u vlastitu binarnu datoteku.";
    translations_["snap_btn_save"]["ENG"] = "Save Snapshot";
    translations_["snap_btn_save"]["HRV"] = "Spremi Snimku";
    translations_["snap_btn_load"]["ENG"] = "Load Snapshot";
    translations_["snap_btn_load"]["HRV"] = "Učitaj Snimku";
    translations_["snap_btn_validate"]["ENG"] = "Validate Database";
    translations_["snap_btn_validate"]["HRV"] = "Validiraj bazu";

    // ===== SUPPLIERS TAB =====
    translations_["sup_btn_add"]["ENG"] = "Add Supplier";
    translations_["sup_btn_add"]["HRV"] = "Dodaj Dobavljača";
    translations_["sup_btn_edit"]["ENG"] = "Edit Supplier";
    translations_["sup_btn_edit"]["HRV"] = "Uredi Dobavljača";
    translations_["sup_btn_delete"]["ENG"] = "Delete Supplier";
    translations_["sup_btn_delete"]["HRV"] = "Obriši Dobavljača";
    translations_["sup_col_company"]["ENG"] = "Company";
    translations_["sup_col_company"]["HRV"] = "Tvrtka";
    translations_["sup_col_contact"]["ENG"] = "Contact";
    translations_["sup_col_contact"]["HRV"] = "Kontakt";
    translations_["sup_col_email"]["ENG"] = "Email";
    translations_["sup_col_email"]["HRV"] = "Email";
    translations_["sup_col_phone"]["ENG"] = "Phone";
    translations_["sup_col_phone"]["HRV"] = "Telefon";
    translations_["sup_col_address"]["ENG"] = "Address";
    translations_["sup_col_address"]["HRV"] = "Adresa";

    // ===== NETWORK TAB =====
    translations_["net_barcode"]["ENG"] = "Barcode / product name:";
    translations_["net_barcode"]["HRV"] = "Barkod / naziv proizvoda:";
    translations_["net_btn_query"]["ENG"] = "Query Product";
    translations_["net_btn_query"]["HRV"] = "Upit Proizvoda";
    translations_["net_btn_rates"]["ENG"] = "Fetch Exchange Rates  (REST · EUR base)";
    translations_["net_btn_rates"]["HRV"] = "Dohvati Tečajeve  (REST · EUR baza)";
    translations_["net_btn_sendbackup"]["ENG"] = "Send Backup  (stock_snapshot.bin)  [TCP]";
    translations_["net_btn_sendbackup"]["HRV"] = "Pošalji Kopiju  (stock_snapshot.bin)  [TCP]";
    translations_["net_btn_status"]["ENG"] = "Request Status (UDP)";
    translations_["net_btn_status"]["HRV"] = "Zatraži Status (UDP)";
    translations_["net_btn_logsummary"]["ENG"] = "Send Log Summary (UDP)";
    translations_["net_btn_logsummary"]["HRV"] = "Pošalji Sažetak Zapisa (UDP)";
    translations_["net_http"]["ENG"] = "HTTP Download";
    translations_["net_http"]["HRV"] = "HTTP Preuzimanje";
    translations_["net_btn_download"]["ENG"] = "Download";
    translations_["net_btn_download"]["HRV"] = "Preuzmi";
    translations_["net_btn_cancel"]["ENG"] = "Cancel";
    translations_["net_btn_cancel"]["HRV"] = "Odustani";
    translations_["speed_unlimited"]["ENG"] = "Unlimited";
    translations_["speed_unlimited"]["HRV"] = "Neograničeno";

    // ===== CRYPTO TAB =====
    translations_["c_password"]["ENG"] = "Password:";
    translations_["c_password"]["HRV"] = "Lozinka:";
    translations_["c_btn_enc"]["ENG"] = "Encrypt File";
    translations_["c_btn_enc"]["HRV"] = "Šifriraj Datoteku";
    translations_["c_btn_dec"]["ENG"] = "Decrypt File";
    translations_["c_btn_dec"]["HRV"] = "Dešifriraj Datoteku";
    translations_["c_ready"]["ENG"] = "Ready.";
    translations_["c_ready"]["HRV"] = "Spremno.";
    translations_["c_btn_genkeys"]["ENG"] = "Generate Keys";
    translations_["c_btn_genkeys"]["HRV"] = "Generiraj Ključeve";
    translations_["c_btn_exportusers"]["ENG"] = "Export Users (RSA Encrypted)";
    translations_["c_btn_exportusers"]["HRV"] = "Izvoz Korisnika (RSA Šifrirano)";
    translations_["c_btn_decryptusers"]["ENG"] = "Decrypt Users Export";
    translations_["c_btn_decryptusers"]["HRV"] = "Dešifriraj Izvoz Korisnika";
    translations_["c_btn_sign"]["ENG"] = "Sign Orders";
    translations_["c_btn_sign"]["HRV"] = "Potpiši Narudžbe";
    translations_["c_btn_verify"]["ENG"] = "Verify Orders";
    translations_["c_btn_verify"]["HRV"] = "Provjeri Narudžbe";

    // ===== INLINE DIALOGS =====
    translations_["dlg_close"]["ENG"] = "Close";
    translations_["dlg_close"]["HRV"] = "Zatvori";
    translations_["dlg_decrypted_users"]["ENG"] = "Decrypted Users Export";
    translations_["dlg_decrypted_users"]["HRV"] = "Dešifrirani Izvoz Korisnika";
    translations_["dlg_analysis_results"]["ENG"] = "Inventory Analysis Results";
    translations_["dlg_analysis_results"]["HRV"] = "Rezultati Analize Zaliha";
    translations_["dlg_reorder"]["ENG"] = "Reorder Report";
    translations_["dlg_reorder"]["HRV"] = "Izvještaj Narudžbi";

    // ===== PRODUCT DIALOG (EDIT MODE) =====
    translations_["pd_manage_title"]["ENG"] = "Manage Product";
    translations_["pd_manage_title"]["HRV"] = "Upravljaj Proizvodom";
    translations_["pd_stock_change"]["ENG"] = "Stock change:";
    translations_["pd_stock_change"]["HRV"] = "Promjena zalihe:";
    translations_["pd_stock_in"]["ENG"] = "Stock In";
    translations_["pd_stock_in"]["HRV"] = "Zaprimi";
    translations_["pd_stock_out"]["ENG"] = "Stock Out";
    translations_["pd_stock_out"]["HRV"] = "Izdaj";
    translations_["pd_delete"]["ENG"] = "Delete";
    translations_["pd_delete"]["HRV"] = "Obriši";
}