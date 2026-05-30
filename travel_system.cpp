/*
=============================================================
  TRAVEL SYSTEM — консольна програма на C++
  Демонструє:
    - Поліморфізм     : віртуальні методи display() і validate()
    - Композицію      : User містить Credentials (не існує окремо)
    - Агрегацію       : TourManager зберігає вектор Tour*
                        (тури існують незалежно від менеджера)
=============================================================

  ІЄРАРХІЯ КЛАСІВ:

  Displayable  (абстрактний, поліморфізм)
  ├── Tour  (абстрактний)
  │   ├── BeachTour       — пляжний
  │   ├── ExcursionTour   — екскурсійний
  │   └── MountainTour    — гірський
  └── User
      └── [композиція] Credentials

  Validator  (абстрактний інтерфейс, поліморфізм)
  ├── EmailValidator
  └── PasswordValidator

  TourManager   — агрегація Tour*
  AuthManager   — управління користувачами
  UI            — меню і введення

=============================================================
*/

#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <algorithm>
#include <sstream>
#include <limits>
#include <memory>

using namespace std;

// ─────────────────────────────────────────────────────────
//  КОЛЬОРИ ДЛЯ КОНСОЛІ
// ─────────────────────────────────────────────────────────
namespace Color {
    const string RESET  = "\033[0m";
    const string RED    = "\033[31m";
    const string GREEN  = "\033[32m";
    const string YELLOW = "\033[33m";
    const string BLUE   = "\033[34m";
    const string CYAN   = "\033[36m";
    const string BOLD   = "\033[1m";
}

// ─────────────────────────────────────────────────────────
//  АБСТРАКТНИЙ КЛАС: Displayable  (поліморфізм)
//  Будь-що що можна вивести на екран
// ─────────────────────────────────────────────────────────
class Displayable {
public:
    virtual void display() const = 0;       // чисто віртуальний
    virtual string getSummary() const = 0;  // чисто віртуальний
    virtual ~Displayable() {}
};

// ─────────────────────────────────────────────────────────
//  АБСТРАКТНИЙ КЛАС: Validator  (поліморфізм)
//  Інтерфейс для будь-якої валідації
// ─────────────────────────────────────────────────────────
class Validator {
public:
    virtual bool validate(const string& value) const = 0;
    virtual string getErrorMessage() const = 0;
    virtual ~Validator() {}
};

// ─────────────────────────────────────────────────────────
//  EmailValidator  — конкретна реалізація Validator
// ─────────────────────────────────────────────────────────
class EmailValidator : public Validator {
public:
    bool validate(const string& email) const override {
        // перевіряємо наявність @ і крапки після @
        size_t atPos = email.find('@');
        if (atPos == string::npos || atPos == 0) return false;

        size_t dotPos = email.find('.', atPos);
        if (dotPos == string::npos) return false;
        if (dotPos == atPos + 1)   return false;
        if (dotPos == email.size() - 1) return false;

        // не повинно бути пробілів
        if (email.find(' ') != string::npos) return false;

        return true;
    }

    string getErrorMessage() const override {
        return "Невірний формат email (приклад: user@gmail.com)";
    }
};

// ─────────────────────────────────────────────────────────
//  PasswordValidator  — конкретна реалізація Validator
// ─────────────────────────────────────────────────────────
class PasswordValidator : public Validator {
private:
    int minLength;
public:
    PasswordValidator(int minLen = 6) : minLength(minLen) {}

    bool validate(const string& password) const override {
        return (int)password.length() >= minLength;
    }

    string getErrorMessage() const override {
        return "Пароль мінімум " + to_string(minLength) + " символів";
    }
};

// ─────────────────────────────────────────────────────────
//  КЛАС: Credentials  (частина User — КОМПОЗИЦІЯ)
//  Не має сенсу без User, створюється і знищується разом з ним
// ─────────────────────────────────────────────────────────
class Credentials {
private:
    string email;
    string passwordHash; // спрощено — зберігаємо як є

    // проста "хеш"-функція для демонстрації
    string simpleHash(const string& pwd) const {
        int hash = 0;
        for (char c : pwd) hash = hash * 31 + c;
        return to_string(abs(hash));
    }

public:
    Credentials(const string& email, const string& password)
        : email(email), passwordHash(simpleHash(password)) {}

    bool checkPassword(const string& password) const {
        return passwordHash == simpleHash(password);
    }

    string getEmail() const { return email; }
};

// ─────────────────────────────────────────────────────────
//  КЛАС: User  (містить Credentials — КОМПОЗИЦІЯ)
//  Наслідує Displayable — поліморфізм
// ─────────────────────────────────────────────────────────
class User : public Displayable {
private:
    Credentials credentials; // КОМПОЗИЦІЯ — Credentials живе всередині User
    bool adminFlag;

public:
    User(const string& email, const string& password, bool isAdmin = false)
        : credentials(email, password), adminFlag(isAdmin) {}

    bool checkPassword(const string& pwd) const {
        return credentials.checkPassword(pwd);
    }

    string getEmail() const { return credentials.getEmail(); }
    bool   isAdmin()  const { return adminFlag; }

    // реалізація Displayable — поліморфізм
    void display() const override {
        cout << Color::CYAN << "👤 Користувач: " << Color::RESET
             << getEmail();
        if (adminFlag)
            cout << Color::YELLOW << "  [👑 Адміністратор]" << Color::RESET;
        cout << endl;
    }

    string getSummary() const override {
        return (adminFlag ? "👑 " : "👤 ") + getEmail();
    }
};

// ─────────────────────────────────────────────────────────
//  АБСТРАКТНИЙ КЛАС: Tour  (поліморфізм)
//  Наслідує Displayable
// ─────────────────────────────────────────────────────────
class Tour : public Displayable {
protected:
    int    id;
    string name;
    string country;
    double price;
    int    days;
    string date;
    string desc;

public:
    Tour(int id, const string& name, const string& country,
         double price, int days, const string& date, const string& desc)
        : id(id), name(name), country(country),
          price(price), days(days), date(date), desc(desc) {}

    virtual ~Tour() {}

    // геттери
    int    getId()      const { return id; }
    string getName()    const { return name; }
    string getCountry() const { return country; }
    double getPrice()   const { return price; }
    int    getDays()    const { return days; }
    string getDate()    const { return date; }
    string getDesc()    const { return desc; }

    // чисто віртуальний — кожен тип туру повертає свій тип
    virtual string getType() const = 0;

    // чисто віртуальний — кожен тип туру виводить свої особливості
    virtual void displayExtras() const = 0;

    // реалізація display() — спільна рамка для всіх турів
    // displayExtras() викликається поліморфно
    void display() const override {
        cout << Color::BOLD;
        cout << "┌──────────────────────────────────────────┐" << endl;
        cout << "│ [" << setw(2) << id << "] "
             << Color::CYAN << name << Color::RESET << Color::BOLD << endl;
        cout << "├──────────────────────────────────────────┤" << endl;
        cout << Color::RESET;
        cout << "│ 📍 Країна  : " << country << endl;
        cout << "│ 🏷  Тип     : " << Color::GREEN << getType()
             << Color::RESET << endl;
        cout << "│ 🗓  Днів    : " << days << endl;
        cout << "│ 📅 Дата    : " << date << endl;
        cout << "│ 💰 Ціна    : " << Color::YELLOW << fixed
             << setprecision(0) << price << "€" << Color::RESET << endl;
        cout << "│ 📝 Опис    : " << desc << endl;
        displayExtras();  // поліморфний виклик
        cout << "└──────────────────────────────────────────┘" << endl;
    }

    string getSummary() const override {
        ostringstream oss;
        oss << "[" << id << "] " << name << " | "
            << country << " | " << getType()
            << " | " << fixed << setprecision(0) << price << "€"
            << " | " << days << " днів";
        return oss.str();
    }
};

// ─────────────────────────────────────────────────────────
//  BeachTour — пляжний тур  (конкретна реалізація Tour)
// ─────────────────────────────────────────────────────────
class BeachTour : public Tour {
private:
    string seaType;    // Середземне, Червоне, Егейське...
    bool   allInclusive;

public:
    BeachTour(int id, const string& name, const string& country,
              double price, int days, const string& date,
              const string& desc, const string& sea, bool allInc)
        : Tour(id, name, country, price, days, date, desc),
          seaType(sea), allInclusive(allInc) {}

    string getType() const override { return "Пляжний"; }

    void displayExtras() const override {
        cout << "│ 🌊 Море    : " << seaType << endl;
        cout << "│ 🍹 All Inc : "
             << (allInclusive ? "Так" : "Ні") << endl;
    }
};

// ─────────────────────────────────────────────────────────
//  ExcursionTour — екскурсійний тур
// ─────────────────────────────────────────────────────────
class ExcursionTour : public Tour {
private:
    int    sitesCount;   // кількість пам'яток
    string language;     // мова екскурсії

public:
    ExcursionTour(int id, const string& name, const string& country,
                  double price, int days, const string& date,
                  const string& desc, int sites, const string& lang)
        : Tour(id, name, country, price, days, date, desc),
          sitesCount(sites), language(lang) {}

    string getType() const override { return "Екскурсійний"; }

    void displayExtras() const override {
        cout << "│ 🏛  Пам'яток: " << sitesCount << endl;
        cout << "│ 🗣  Мова    : " << language << endl;
    }
};

// ─────────────────────────────────────────────────────────
//  MountainTour — гірський тур
// ─────────────────────────────────────────────────────────
class MountainTour : public Tour {
private:
    int    maxAltitude;  // висота в метрах
    string difficulty;   // легкий / середній / важкий

public:
    MountainTour(int id, const string& name, const string& country,
                 double price, int days, const string& date,
                 const string& desc, int altitude, const string& diff)
        : Tour(id, name, country, price, days, date, desc),
          maxAltitude(altitude), difficulty(diff) {}

    string getType() const override { return "Гірський"; }

    void displayExtras() const override {
        cout << "│ ⛰  Висота  : " << maxAltitude << " м" << endl;
        cout << "│ 💪 Склад.  : " << difficulty << endl;
    }
};

// ─────────────────────────────────────────────────────────
//  КЛАС: TourManager  (АГРЕГАЦІЯ Tour*)
//  Зберігає вказівники на тури, але не "володіє" ними повністю —
//  тури можуть існувати поза менеджером (агрегація, не композиція)
// ─────────────────────────────────────────────────────────
class TourManager {
private:
    vector<Tour*> tours; // АГРЕГАЦІЯ — вектор вказівників

    // допоміжна: нижній регістр
    static string toLower(string s) {
        transform(s.begin(), s.end(), s.begin(), ::tolower);
        return s;
    }

public:
    // додати тур (ззовні — агрегація)
    void addTour(Tour* tour) {
        tours.push_back(tour);
    }

    // кількість турів
    int count() const { return (int)tours.size(); }

    // вивести всі
    void displayAll() const {
        if (tours.empty()) {
            cout << Color::YELLOW << "Турів не знайдено." << Color::RESET << endl;
            return;
        }
        cout << Color::BOLD << "\n=== Всі тури (" << tours.size() << ") ===" 
             << Color::RESET << endl;
        for (const auto* t : tours)
            t->display(); // поліморфний виклик!
    }

    // пошук за назвою
    void searchByName(const string& query) const {
        string q = toLower(query);
        bool found = false;
        cout << Color::BOLD << "\n=== Результати пошуку: \"" << query << "\" ===" 
             << Color::RESET << endl;
        for (const auto* t : tours) {
            if (toLower(t->getName()).find(q) != string::npos ||
                toLower(t->getCountry()).find(q) != string::npos ||
                toLower(t->getDesc()).find(q) != string::npos) {
                t->display();
                found = true;
            }
        }
        if (!found)
            cout << Color::RED << "Нічого не знайдено." << Color::RESET << endl;
    }

    // фільтр за типом туру (поліморфізм getType())
    void filterByType(const string& type) const {
        cout << Color::BOLD << "\n=== Тур типу: " << type << " ===" 
             << Color::RESET << endl;
        bool found = false;
        for (const auto* t : tours) {
            if (t->getType() == type) { // поліморфний виклик getType()!
                t->display();
                found = true;
            }
        }
        if (!found)
            cout << Color::RED << "Турів такого типу не знайдено." 
                 << Color::RESET << endl;
    }

    // фільтр за ціною
    void filterByPrice(double minP, double maxP) const {
        cout << Color::BOLD << "\n=== Тури від " << minP << "€ до "
             << maxP << "€ ===" << Color::RESET << endl;
        bool found = false;
        for (const auto* t : tours) {
            if (t->getPrice() >= minP && t->getPrice() <= maxP) {
                t->display();
                found = true;
            }
        }
        if (!found)
            cout << Color::RED << "Турів в такому діапазоні не знайдено."
                 << Color::RESET << endl;
    }

    // фільтр за кількістю днів
    void filterByDays(int minD, int maxD) const {
        cout << Color::BOLD << "\n=== Тури від " << minD << " до "
             << maxD << " днів ===" << Color::RESET << endl;
        bool found = false;
        for (const auto* t : tours) {
            if (t->getDays() >= minD && t->getDays() <= maxD) {
                t->display();
                found = true;
            }
        }
        if (!found)
            cout << Color::RED << "Турів не знайдено." << Color::RESET << endl;
    }

    // сортування за ціною
    void sortByPrice(bool ascending = true) {
        sort(tours.begin(), tours.end(), [ascending](Tour* a, Tour* b) {
            return ascending ? a->getPrice() < b->getPrice()
                             : a->getPrice() > b->getPrice();
        });
        cout << Color::GREEN << "✅ Відсортовано за ціною "
             << (ascending ? "↑" : "↓") << Color::RESET << endl;
    }

    // сортування за кількістю днів
    void sortByDays() {
        sort(tours.begin(), tours.end(), [](Tour* a, Tour* b) {
            return a->getDays() < b->getDays();
        });
        cout << Color::GREEN << "✅ Відсортовано за тривалістю ↑" 
             << Color::RESET << endl;
    }

    // знайти за id
    Tour* findById(int id) const {
        for (auto* t : tours)
            if (t->getId() == id) return t;
        return nullptr;
    }

    // вивести короткий список (getSummary — поліморфізм)
    void displayShortList() const {
        cout << Color::BOLD << "\n--- Список турів ---" << Color::RESET << endl;
        for (const auto* t : tours)
            cout << "  " << t->getSummary() << endl; // поліморфний виклик!
    }

    // адмін: додати новий тур через консоль
    void adminAddTour() {
        cout << Color::BOLD << "\n=== Додати новий тур ===" << Color::RESET << endl;
        
        int type;
        cout << "Тип туру:\n  1. Пляжний\n  2. Екскурсійний\n  3. Гірський\nВибір: ";
        cin >> type;
        cin.ignore();

        int    id = (int)tours.size() + 1;
        string name, country, date, desc;
        double price;
        int    days;

        cout << "Назва: ";      getline(cin, name);
        cout << "Країна: ";     getline(cin, country);
        cout << "Опис: ";       getline(cin, desc);
        cout << "Ціна (€): ";   cin >> price;
        cout << "Днів: ";       cin >> days;
        cin.ignore();
        cout << "Дата (дд.мм.рррр): "; getline(cin, date);

        Tour* newTour = nullptr;

        if (type == 1) {
            string sea;
            char allInc;
            cout << "Назва моря: "; getline(cin, sea);
            cout << "All Inclusive? (y/n): "; cin >> allInc;
            cin.ignore();
            newTour = new BeachTour(id, name, country, price, days,
                                    date, desc, sea, allInc == 'y');
        } else if (type == 2) {
            int    sites;
            string lang;
            cout << "Кількість пам'яток: "; cin >> sites;
            cin.ignore();
            cout << "Мова екскурсії: "; getline(cin, lang);
            newTour = new ExcursionTour(id, name, country, price, days,
                                        date, desc, sites, lang);
        } else if (type == 3) {
            int    altitude;
            string diff;
            cout << "Максимальна висота (м): "; cin >> altitude;
            cin.ignore();
            cout << "Складність (легкий/середній/важкий): "; getline(cin, diff);
            newTour = new MountainTour(id, name, country, price, days,
                                       date, desc, altitude, diff);
        } else {
            cout << Color::RED << "Невірний тип." << Color::RESET << endl;
            return;
        }

        if (newTour) {
            tours.push_back(newTour);
            cout << Color::GREEN << "✅ Тур \"" << name 
                 << "\" успішно додано!" << Color::RESET << endl;
        }
    }

    // адмін: видалити тур за id
    void adminDeleteTour(int id) {
        auto it = find_if(tours.begin(), tours.end(),
                          [id](Tour* t) { return t->getId() == id; });
        if (it == tours.end()) {
            cout << Color::RED << "❌ Тур з ID " << id 
                 << " не знайдено." << Color::RESET << endl;
            return;
        }
        string name = (*it)->getName();
        delete *it;       // звільняємо пам'ять
        tours.erase(it);
        cout << Color::GREEN << "✅ Тур \"" << name 
             << "\" видалено." << Color::RESET << endl;
    }

    // статистика
    void displayStats() const {
        if (tours.empty()) return;

        double totalPrice = 0, minP = tours[0]->getPrice(), maxP = tours[0]->getPrice();
        int totalDays = 0;

        for (const auto* t : tours) {
            totalPrice += t->getPrice();
            totalDays  += t->getDays();
            if (t->getPrice() < minP) minP = t->getPrice();
            if (t->getPrice() > maxP) maxP = t->getPrice();
        }

        cout << Color::BOLD << "\n=== 📊 Статистика ===" << Color::RESET << endl;
        cout << "  Кількість турів  : " << tours.size() << endl;
        cout << "  Середня ціна     : " << fixed << setprecision(0)
             << totalPrice / tours.size() << "€" << endl;
        cout << "  Мінімальна ціна  : " << minP << "€" << endl;
        cout << "  Максимальна ціна : " << maxP << "€" << endl;
        cout << "  Середня тривал.  : "
             << totalDays / (int)tours.size() << " днів" << endl;
    }

    // деструктор — звільняємо пам'ять
    ~TourManager() {
        for (auto* t : tours)
            delete t;
    }
};

// ─────────────────────────────────────────────────────────
//  КЛАС: AuthManager — управління користувачами
// ─────────────────────────────────────────────────────────
class AuthManager {
private:
    vector<User*> users;
    User* currentUser = nullptr;

    // валідатори (поліморфізм через Validator*)
    EmailValidator    emailValidator;
    PasswordValidator passwordValidator;

public:
    AuthManager() {
        // Додаємо адміністраторів за замовчуванням
        users.push_back(new User("admin@travel.com",   "admin123",   true));
        users.push_back(new User("manager@travel.com", "manager456", true));
    }

    ~AuthManager() {
        for (auto* u : users) delete u;
    }

    // вхід
    bool login(const string& email, const string& password) {
        // спочатку валідація через поліморфний Validator
        if (!emailValidator.validate(email)) {
            cout << Color::RED << "❌ " << emailValidator.getErrorMessage()
                 << Color::RESET << endl;
            return false;
        }

        for (auto* u : users) {
            if (u->getEmail() == email && u->checkPassword(password)) {
                currentUser = u;
                cout << Color::GREEN << "✅ Вхід успішний! Привіт, "
                     << u->getSummary() << Color::RESET << endl;
                return true;
            }
        }
        cout << Color::RED << "❌ Невірний email або пароль."
             << Color::RESET << endl;
        return false;
    }

    // реєстрація
    bool registerUser(const string& email, const string& password) {
        // валідація email (поліморфізм)
        if (!emailValidator.validate(email)) {
            cout << Color::RED << "❌ " << emailValidator.getErrorMessage()
                 << Color::RESET << endl;
            return false;
        }
        // валідація пароля (поліморфізм)
        if (!passwordValidator.validate(password)) {
            cout << Color::RED << "❌ " << passwordValidator.getErrorMessage()
                 << Color::RESET << endl;
            return false;
        }
        // чи вже існує
        for (const auto* u : users) {
            if (u->getEmail() == email) {
                cout << Color::RED << "❌ Користувач вже існує."
                     << Color::RESET << endl;
                return false;
            }
        }

        users.push_back(new User(email, password, false));
        cout << Color::GREEN << "✅ Реєстрація успішна! Тепер увійдіть."
             << Color::RESET << endl;
        return true;
    }

    // вихід
    void logout() {
        if (currentUser) {
            cout << Color::CYAN << "👋 До побачення, "
                 << currentUser->getEmail() << "!" << Color::RESET << endl;
            currentUser = nullptr;
        }
    }

    bool isLoggedIn() const { return currentUser != nullptr; }
    bool isAdmin()    const { return currentUser && currentUser->isAdmin(); }

    string getCurrentEmail() const {
        return currentUser ? currentUser->getEmail() : "";
    }

    // вивести всіх користувачів (тільки для адміна)
    void displayAllUsers() const {
        cout << Color::BOLD << "\n=== Користувачі (" << users.size()
             << ") ===" << Color::RESET << endl;
        for (const auto* u : users)
            u->display(); // поліморфний виклик!
    }
};

// ─────────────────────────────────────────────────────────
//  КЛАС: UI — головне меню і введення
// ─────────────────────────────────────────────────────────
class UI {
private:
    TourManager& tm;  // агрегація — UI використовує менеджер
    AuthManager& auth;

    // очистити буфер вводу
    static void clearInput() {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }

    // зчитати ціле число з перевіркою
    static int readInt(const string& prompt) {
        int val;
        while (true) {
            cout << prompt;
            if (cin >> val) { clearInput(); return val; }
            cout << Color::RED << "❌ Введіть ціле число." 
                 << Color::RESET << endl;
            clearInput();
        }
    }

    // зчитати дробове число
    static double readDouble(const string& prompt) {
        double val;
        while (true) {
            cout << prompt;
            if (cin >> val) { clearInput(); return val; }
            cout << Color::RED << "❌ Введіть число." 
                 << Color::RESET << endl;
            clearInput();
        }
    }

    // головна шапка
    static void printHeader() {
        cout << Color::BOLD << Color::BLUE;
        cout << "\n╔══════════════════════════════════════════╗" << endl;
        cout << "║         🌍  TRAVEL SYSTEM  🌍            ║" << endl;
        cout << "╚══════════════════════════════════════════╝" << endl;
        cout << Color::RESET;
    }

    // меню авторизації
    void authMenu() {
        while (!auth.isLoggedIn()) {
            printHeader();
            cout << "  1. Увійти" << endl;
            cout << "  2. Зареєструватись" << endl;
            cout << "  0. Вихід з програми" << endl;

            int choice = readInt("Ваш вибір: ");

            if (choice == 0) {
                cout << Color::CYAN << "До побачення!" 
                     << Color::RESET << endl;
                exit(0);
            }

            string email, password;
            cout << "Email   : "; getline(cin, email);
            cout << "Пароль  : "; getline(cin, password);

            if (choice == 1) auth.login(email, password);
            else if (choice == 2) auth.registerUser(email, password);
            else cout << Color::RED << "Невірний вибір." << Color::RESET << endl;
        }
    }

    // меню перегляду турів
    void toursMenu() {
        while (true) {
            cout << Color::BOLD << "\n--- 🗺  Тури ---" << Color::RESET << endl;
            cout << "  1. Переглянути всі тури" << endl;
            cout << "  2. Короткий список" << endl;
            cout << "  3. Пошук" << endl;
            cout << "  4. Фільтр за типом" << endl;
            cout << "  5. Фільтр за ціною" << endl;
            cout << "  6. Фільтр за тривалістю" << endl;
            cout << "  7. Сортувати за ціною ↑" << endl;
            cout << "  8. Сортувати за ціною ↓" << endl;
            cout << "  9. Сортувати за тривалістю" << endl;
            cout << " 10. Детально про тур (за ID)" << endl;
            cout << " 11. Статистика" << endl;
            cout << "  0. Назад" << endl;

            int choice = readInt("Ваш вибір: ");

            if (choice == 0) break;

            switch (choice) {
                case 1: tm.displayAll(); break;
                case 2: tm.displayShortList(); break;
                case 3: {
                    string q;
                    cout << "Пошуковий запит: "; getline(cin, q);
                    tm.searchByName(q);
                    break;
                }
                case 4: {
                    cout << "Тип (Пляжний / Екскурсійний / Гірський): ";
                    string t; getline(cin, t);
                    tm.filterByType(t);
                    break;
                }
                case 5: {
                    double mn = readDouble("Мін ціна (€): ");
                    double mx = readDouble("Макс ціна (€): ");
                    tm.filterByPrice(mn, mx);
                    break;
                }
                case 6: {
                    int mn = readInt("Мін днів: ");
                    int mx = readInt("Макс днів: ");
                    tm.filterByDays(mn, mx);
                    break;
                }
                case 7:  tm.sortByPrice(true);  tm.displayShortList(); break;
                case 8:  tm.sortByPrice(false); tm.displayShortList(); break;
                case 9:  tm.sortByDays();        tm.displayShortList(); break;
                case 10: {
                    int id = readInt("ID туру: ");
                    Tour* t = tm.findById(id);
                    if (t) t->display(); // поліморфний виклик!
                    else cout << Color::RED << "❌ Тур не знайдено." 
                              << Color::RESET << endl;
                    break;
                }
                case 11: tm.displayStats(); break;
                default: cout << Color::RED << "Невірний вибір." 
                              << Color::RESET << endl;
            }
        }
    }

    // адмін-панель
    void adminMenu() {
        while (true) {
            cout << Color::BOLD << Color::YELLOW
                 << "\n--- ⚙️  Адмін-панель ---" << Color::RESET << endl;
            cout << "  1. Додати тур" << endl;
            cout << "  2. Видалити тур" << endl;
            cout << "  3. Всі користувачі" << endl;
            cout << "  0. Назад" << endl;

            int choice = readInt("Ваш вибір: ");

            if (choice == 0) break;

            switch (choice) {
                case 1: tm.adminAddTour(); break;
                case 2: {
                    tm.displayShortList();
                    int id = readInt("ID туру для видалення: ");
                    tm.adminDeleteTour(id);
                    break;
                }
                case 3: auth.displayAllUsers(); break;
                default: cout << Color::RED << "Невірний вибір." 
                              << Color::RESET << endl;
            }
        }
    }

public:
    UI(TourManager& tm, AuthManager& auth) : tm(tm), auth(auth) {}

    // головний цикл
    void run() {
        authMenu(); // спочатку вхід

        while (true) {
            printHeader();
            cout << Color::CYAN << "  Увійшли як: " << auth.getCurrentEmail()
                 << (auth.isAdmin() ? " 👑" : "") << Color::RESET << endl;
            cout << endl;
            cout << "  1. Тури" << endl;
            if (auth.isAdmin())
                cout << "  2. Адмін-панель" << endl;
            cout << "  3. Вийти з акаунту" << endl;
            cout << "  0. Закрити програму" << endl;

            int choice = readInt("Ваш вибір: ");

            switch (choice) {
                case 0:
                    cout << Color::CYAN << "До побачення!" 
                         << Color::RESET << endl;
                    return;
                case 1: toursMenu(); break;
                case 2:
                    if (auth.isAdmin()) adminMenu();
                    else cout << Color::RED << "❌ Доступ заборонено." 
                              << Color::RESET << endl;
                    break;
                case 3:
                    auth.logout();
                    authMenu(); // знову вхід
                    break;
                default:
                    cout << Color::RED << "Невірний вибір." 
                         << Color::RESET << endl;
            }
        }
    }
};

// ─────────────────────────────────────────────────────────
//  ФУНКЦІЯ: завантаження тестових даних
// ─────────────────────────────────────────────────────────
void loadSampleData(TourManager& tm) {
    // BeachTour(id, name, country, price, days, date, desc, sea, allInclusive)
    tm.addTour(new BeachTour(1, "Санторіні", "Греція", 850, 8,
        "01.06.2026",
        "Казкові білі будиночки над блакитним морем",
        "Егейське море", false));

    tm.addTour(new BeachTour(2, "Анталія", "Туреччина", 400, 7,
        "12.06.2026",
        "Найпопулярніший курорт з системою все включено",
        "Середземне море", true));

    tm.addTour(new BeachTour(3, "Шарм-ель-Шейх", "Єгипет", 550, 10,
        "05.07.2026",
        "Кришталева вода та корали Червоного моря",
        "Червоне море", true));

    // ExcursionTour(id, name, country, price, days, date, desc, sites, language)
    tm.addTour(new ExcursionTour(4, "Рим", "Італія", 650, 7,
        "10.05.2026",
        "Вічне місто з Колізеєм та Ватиканом",
        12, "Українська"));

    tm.addTour(new ExcursionTour(5, "Париж", "Франція", 500, 5,
        "15.05.2026",
        "Романтична столиця з Ейфелевою вежею",
        8, "Українська"));

    tm.addTour(new ExcursionTour(6, "Стамбул", "Туреччина", 450, 5,
        "25.05.2026",
        "На межі двох континентів з багатою культурою",
        10, "Англійська"));

    tm.addTour(new ExcursionTour(7, "Афіни", "Греція", 560, 5,
        "18.05.2026",
        "Акрополь та жива культура стародавньої Греції",
        9, "Українська"));

    // MountainTour(id, name, country, price, days, date, desc, altitude, difficulty)
    tm.addTour(new MountainTour(8, "Альпи", "Франція", 900, 6,
        "15.07.2026",
        "Захоплюючі гірські краєвиди та активний відпочинок",
        3800, "Середній"));

    tm.addTour(new MountainTour(9, "Каппадокія", "Туреччина", 750, 6,
        "10.06.2026",
        "Польоти на повітряній кулі над казковим ландшафтом",
        1200, "Легкий"));

    tm.addTour(new MountainTour(10, "Доломіти", "Італія", 980, 7,
        "20.07.2026",
        "Неймовірні скелі та альпійські луки",
        3200, "Важкий"));
}

// ─────────────────────────────────────────────────────────
//  MAIN
// ─────────────────────────────────────────────────────────
int main() {
    // Для Windows — підтримка UTF-8 в консолі
    // system("chcp 65001 > nul");

    TourManager tourManager;
    AuthManager authManager;

    loadSampleData(tourManager);

    UI ui(tourManager, authManager);
    ui.run();

    return 0;
}

/*
=============================================================
  КОМПІЛЯЦІЯ ТА ЗАПУСК:

  Linux / Mac:
    g++ -std=c++17 -o travel travel_system.cpp && ./travel

  Windows (MinGW):
    g++ -std=c++17 -o travel.exe travel_system.cpp && travel.exe

=============================================================
  ДЕМОНСТРАЦІЯ КОНЦЕПЦІЙ:

  ПОЛІМОРФІЗМ:
    - Tour::display() викликає displayExtras() — у кожного типу своя
    - Tour::getType() — повертає різні рядки залежно від класу
    - Displayable* може вказувати на Tour або User
    - Validator* може бути EmailValidator або PasswordValidator
    - TourManager::displayAll() — один цикл, різна поведінка

  КОМПОЗИЦІЯ:
    - User містить Credentials як поле (не вказівник)
    - Credentials не існує без User
    - Знищення User → автоматичне знищення Credentials

  АГРЕГАЦІЯ:
    - TourManager зберігає vector<Tour*>
    - Tour* створюються поза TourManager (в loadSampleData)
    - UI отримує TourManager& і AuthManager& ззовні
    - Тури теоретично можуть існувати незалежно від менеджера

=============================================================
*/
