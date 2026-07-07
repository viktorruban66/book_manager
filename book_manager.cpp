// book_manager.cpp
#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iomanip>
#include <ctime>
#include <map>
#include <variant>
#include <regex>
#include <cctype>

using namespace std;

struct Book {
    int id;
    string title;
    string author;
    int year;
    string genre;
    bool read;
    int rating;
    string addedDate;

    Book(int id, const string& title, const string& author, int year, const string& genre,
         bool read, int rating, const string& addedDate = "")
        : id(id), title(title), author(author), year(year), genre(genre), read(read), rating(rating), addedDate(addedDate) {
        if (addedDate.empty()) {
            time_t now = time(nullptr);
            tm* tm_now = localtime(&now);
            char buf[11];
            strftime(buf, sizeof(buf), "%Y-%m-%d", tm_now);
            this->addedDate = string(buf);
        }
    }
};

class BookManager {
private:
    vector<Book> books;
    int nextId = 1;

    string getCurrentDate() {
        time_t now = time(nullptr);
        tm* tm_now = localtime(&now);
        char buf[11];
        strftime(buf, sizeof(buf), "%Y-%m-%d", tm_now);
        return string(buf);
    }

public:
    Book addBook(const string& title, const string& author, int year, const string& genre, bool read, int rating) {
        if (rating < 1 || rating > 10) throw invalid_argument("Оценка должна быть от 1 до 10");
        int currentYear;
        time_t now = time(nullptr);
        tm* tm_now = localtime(&now);
        currentYear = tm_now->tm_year + 1900;
        if (year < 0 || year > currentYear) throw invalid_argument("Год должен быть от 0 до " + to_string(currentYear));
        Book book(nextId, title, author, year, genre, read, rating);
        books.push_back(book);
        nextId++;
        return book;
    }

    Book* findBook(int id) {
        auto it = find_if(books.begin(), books.end(), [id](const Book& b) { return b.id == id; });
        return it != books.end() ? &(*it) : nullptr;
    }

    bool editBook(int id, const map<string, string>& updates) {
        Book* book = findBook(id);
        if (!book) return false;
        for (const auto& [key, value] : updates) {
            if (key == "title") book->title = value;
            else if (key == "author") book->author = value;
            else if (key == "year") book->year = stoi(value);
            else if (key == "genre") book->genre = value;
            else if (key == "read") book->read = (value == "1");
            else if (key == "rating") book->rating = stoi(value);
        }
        return true;
    }

    bool deleteBook(int id) {
        auto it = find_if(books.begin(), books.end(), [id](const Book& b) { return b.id == id; });
        if (it == books.end()) return false;
        books.erase(it);
        return true;
    }

    vector<Book> searchBooks(const string& query) {
        string q = query;
        transform(q.begin(), q.end(), q.begin(), ::tolower);
        vector<Book> result;
        for (const auto& b : books) {
            string titleLower = b.title;
            string authorLower = b.author;
            transform(titleLower.begin(), titleLower.end(), titleLower.begin(), ::tolower);
            transform(authorLower.begin(), authorLower.end(), authorLower.begin(), ::tolower);
            if (titleLower.find(q) != string::npos || authorLower.find(q) != string::npos) {
                result.push_back(b);
            }
        }
        return result;
    }

    vector<Book> filterByRead(bool read) const {
        vector<Book> result;
        for (const auto& b : books) {
            if (b.read == read) result.push_back(b);
        }
        return result;
    }

    vector<Book> filterByGenre(const string& genre) const {
        vector<Book> result;
        for (const auto& b : books) {
            if (b.genre == genre) result.push_back(b);
        }
        return result;
    }

    map<string, variant<int, double, map<string, int>>> getStats() const {
        int total = books.size();
        int readCount = filterByRead(true).size();
        int unread = total - readCount;
        int sumRating = 0;
        int ratingCount = 0;
        for (const auto& b : books) {
            if (b.read) { sumRating += b.rating; ratingCount++; }
        }
        double avgRating = ratingCount > 0 ? static_cast<double>(sumRating) / ratingCount : 0.0;
        map<string, int> genres;
        for (const auto& b : books) {
            genres[b.genre]++;
        }
        map<string, variant<int, double, map<string, int>>> stats;
        stats["total"] = total;
        stats["read"] = readCount;
        stats["unread"] = unread;
        stats["avg_rating"] = avgRating;
        stats["genres"] = genres;
        return stats;
    }

    void saveToFile(const string& filename = "books_data.txt") {
        ofstream out(filename);
        if (!out) return;
        for (const auto& b : books) {
            out << b.id << '|'
                << b.title << '|'
                << b.author << '|'
                << b.year << '|'
                << b.genre << '|'
                << b.read << '|'
                << b.rating << '|'
                << b.addedDate << '\n';
        }
    }

    void loadFromFile(const string& filename = "books_data.txt") {
        ifstream in(filename);
        if (!in) return;
        books.clear();
        string line;
        while (getline(in, line)) {
            stringstream ss(line);
            string idStr, title, author, yearStr, genre, readStr, ratingStr, addedDate;
            getline(ss, idStr, '|');
            getline(ss, title, '|');
            getline(ss, author, '|');
            getline(ss, yearStr, '|');
            getline(ss, genre, '|');
            getline(ss, readStr, '|');
            getline(ss, ratingStr, '|');
            getline(ss, addedDate, '|');
            int id = stoi(idStr);
            int year = stoi(yearStr);
            bool read = (readStr == "1");
            int rating = stoi(ratingStr);
            books.emplace_back(id, title, author, year, genre, read, rating, addedDate);
            if (id >= nextId) nextId = id + 1;
        }
    }

    const vector<Book>& getBooks() const { return books; }
};

string readString(const string& prompt) {
    cout << prompt;
    string input;
    getline(cin, input);
    return input;
}

int readInt(const string& prompt) {
    while (true) {
        cout << prompt;
        string input;
        getline(cin, input);
        try {
            return stoi(input);
        } catch (...) {
            cout << "Введите число.\n";
        }
    }
}

bool readBool(const string& prompt) {
    while (true) {
        string input = readString(prompt);
        if (input == "1") return true;
        if (input == "0") return false;
        cout << "Введите 1 или 0.\n";
    }
}

void printBook(const Book& book) {
    string status = book.read ? "✅ Прочитана" : "📖 Не прочитана";
    cout << "#" << book.id << " - " << book.title << " (" << book.year << ")\n";
    cout << "   Автор: " << book.author << ", Жанр: " << book.genre << "\n";
    cout << "   " << status << ", Оценка: " << book.rating << "/10\n";
    cout << "   Добавлена: " << book.addedDate << "\n";
}

int main() {
    BookManager manager;
    manager.loadFromFile();

    while (true) {
        cout << "\n===== МЕНЕДЖЕР КНИГ (C++) =====" << endl;
        cout << "1. Добавить книгу\n";
        cout << "2. Показать все книги\n";
        cout << "3. Показать прочитанные книги\n";
        cout << "4. Показать непрочитанные книги\n";
        cout << "5. Найти книги по автору или названию\n";
        cout << "6. Редактировать книгу\n";
        cout << "7. Удалить книгу\n";
        cout << "8. Показать статистику\n";
        cout << "9. Сохранить в файл\n";
        cout << "10. Загрузить из файла\n";
        cout << "0. Выход\n";
        string choice = readString("Выберите действие: ");

        if (choice == "0") break;

        if (choice == "1") {
            string title = readString("Название: ");
            if (title.empty()) { cout << "Название не может быть пустым.\n"; continue; }
            string author = readString("Автор: ");
            if (author.empty()) { cout << "Автор не может быть пустым.\n"; continue; }
            int year = readInt("Год издания: ");
            string genre = readString("Жанр: ");
            if (genre.empty()) genre = "Неизвестно";
            bool read = readBool("Статус (1-прочитана, 0-не прочитана): ");
            int rating = readInt("Оценка (1-10): ");
            try {
                Book book = manager.addBook(title, author, year, genre, read, rating);
                cout << "Книга добавлена с ID " << book.id << "\n";
            } catch (const exception& e) {
                cout << "Ошибка: " << e.what() << "\n";
            }
        } else if (choice == "2") {
            if (manager.getBooks().empty()) {
                cout << "Нет книг.\n";
            } else {
                for (const auto& b : manager.getBooks()) printBook(b);
            }
        } else if (choice == "3") {
            auto books = manager.filterByRead(true);
            if (books.empty()) cout << "Нет прочитанных книг.\n";
            else for (const auto& b : books) printBook(b);
        } else if (choice == "4") {
            auto books = manager.filterByRead(false);
            if (books.empty()) cout << "Нет непрочитанных книг.\n";
            else for (const auto& b : books) printBook(b);
        } else if (choice == "5") {
            string query = readString("Введите часть названия или автора: ");
            auto results = manager.searchBooks(query);
            if (results.empty()) cout << "Книги не найдены.\n";
            else for (const auto& b : results) printBook(b);
        } else if (choice == "6") {
            int id = readInt("Введите ID книги для редактирования: ");
            Book* book = manager.findBook(id);
            if (!book) { cout << "Книга не найдена.\n"; continue; }
            cout << "Оставьте поле пустым, чтобы не менять.\n";
            string newTitle = readString("Название (" + book->title + "): ");
            string newAuthor = readString("Автор (" + book->author + "): ");
            string newYear = readString("Год (" + to_string(book->year) + "): ");
            string newGenre = readString("Жанр (" + book->genre + "): ");
            string newRead = readString("Статус (1-прочитана, 0-не прочитана) сейчас: " + string(book->read ? "1" : "0") + ": ");
            string newRating = readString("Оценка (" + to_string(book->rating) + "): ");
            map<string, string> updates;
            if (!newTitle.empty()) updates["title"] = newTitle;
            if (!newAuthor.empty()) updates["author"] = newAuthor;
            if (!newYear.empty()) updates["year"] = newYear;
            if (!newGenre.empty()) updates["genre"] = newGenre;
            if (!newRead.empty()) updates["read"] = newRead;
            if (!newRating.empty()) updates["rating"] = newRating;
            if (manager.editBook(id, updates)) {
                cout << "Книга обновлена.\n";
            } else {
                cout << "Ошибка обновления.\n";
            }
        } else if (choice == "7") {
            int id = readInt("Введите ID книги для удаления: ");
            if (manager.deleteBook(id)) {
                cout << "Книга удалена.\n";
            } else {
                cout << "Книга не найдена.\n";
            }
        } else if (choice == "8") {
            auto stats = manager.getStats();
            cout << "\n=== СТАТИСТИКА ===\n";
            cout << "Всего книг: " << get<int>(stats["total"]) << "\n";
            cout << "Прочитано: " << get<int>(stats["read"]) << "\n";
            cout << "Не прочитано: " << get<int>(stats["unread"]) << "\n";
            cout << "Средняя оценка: " << fixed << setprecision(2) << get<double>(stats["avg_rating"]) << "\n";
            cout << "По жанрам:\n";
            auto genres = get<map<string, int>>(stats["genres"]);
            for (const auto& [g, c] : genres) {
                cout << "  " << g << ": " << c << "\n";
            }
        } else if (choice == "9") {
            manager.saveToFile();
            cout << "Сохранено.\n";
        } else if (choice == "10") {
            manager.loadFromFile();
            cout << "Загружено.\n";
        } else {
            cout << "Неизвестная команда.\n";
        }
    }
    return 0;
}
