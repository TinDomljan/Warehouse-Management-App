#ifndef TRANSLATIONMANAGER_H
#define TRANSLATIONMANAGER_H

#include <string>
#include <map>

class TranslationManager {
public:
    //singleton, kreiramo sa instance jedan
    static TranslationManager& instance();

    void setLanguage(const std::string& lang);
    std::string getLanguage() const;
    std::string tr(const std::string& key) const;

private:
    TranslationManager();
    std::string currentLanguage_;
    //nested dictionary
    std::map<std::string, std::map<std::string, std::string>> translations_;

    void loadTranslations();
};

#endif // TRANSLATIONMANAGER_H