#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <map>
#include <stdexcept>
#include <iomanip>
#include <limits>
#include <algorithm>

// Custom Exception Class for Library Domain Errors
class LibraryException : public std::runtime_error {
public:
    explicit LibraryException(const std::string& message)
        : std::runtime_error(message) {}
};

// Book Class (OOP Encapsulation)
class Book {
private:
    int id;
    std::string title;
    std::string author;
    bool isAvailable;

public:
    Book() : id(0), title(""), author(""), isAvailable(true) {}

    Book(int id, const std::string& title, const std::string& author, bool isAvailable = true)
        : id(id), title(title), author(author), isAvailable(isAvailable) {}

    // Getters
    int getId() const { return id; }
    std::string getTitle() const { return title; }
    std::string getAuthor() const { return author; }
    bool getAvailability() const { return isAvailable; }

    // State Modifiers
    void issueBook() {
        if (!isAvailable) {
            throw LibraryException("Book ID " + std::to_string(id) + " is already issued to someone else.");
        }
        isAvailable = false;
    }

    void returnBook() {
        if (isAvailable) {
            throw LibraryException("Book ID " + std::to_string(id) + " is already available in the library.");
        }
        isAvailable = true;
    }

    // Serialize to CSV row
    std::string toCSV() const {
        return std::to_string(id) + "," + title + "," + author + "," + (isAvailable ? "1" : "0");
    }

    // Deserialize from CSV row
    static Book fromCSV(const std::string& line) {
        std::stringstream ss(line);
        std::string idStr, title, author, availStr;

        if (!std::getline(ss, idStr, ',') ||
            !std::getline(ss, title, ',') ||
            !std::getline(ss, author, ',') ||
            !std::getline(ss, availStr, ',')) {
            throw LibraryException("Corrupted record in CSV file: " + line);
        }

        int parsedId = std::stoi(idStr);
        bool parsedAvail = (availStr == "1");
        return Book(parsedId, title, author, parsedAvail);
    }

    void display() const {
        std::cout << std::left
                  << std::setw(8) << id
                  << std::setw(30) << title
                  << std::setw(22) << author
                  << std::setw(14) << (isAvailable ? "Available" : "Issued")
                  << "\n";
    }
};

// Library Manager Class (STL Container + File I/O + Business Logic)
class Library {
private:
    std::map<int, Book> books; // STL map keeps books sorted by ID and provides safe lookup
    std::string filename;

    // Helper to strip commas from user text so CSV structure stays intact
    std::string sanitize(std::string input) const {
        std::replace(input.begin(), input.end(), ',', ';');
        return input;
    }

public:
    explicit Library(const std::string& file) : filename(file) {
        loadFromFile();
    }

    void loadFromFile() {
        std::ifstream inFile(filename);
        if (!inFile.is_open()) {
            // File doesn't exist yet on first run; will be created on save
            return;
        }

        books.clear();
        std::string line;
        // Skip header line
        std::getline(inFile, line);

        while (std::getline(inFile, line)) {
            if (line.empty()) continue;
            try {
                Book b = Book::fromCSV(line);
                books[b.getId()] = b;
            } catch (const std::exception& e) {
                std::cerr << "[Warning] Skipping invalid line: " << e.what() << "\n";
            }
        }
        inFile.close();
    }

    void saveToFile() const {
        std::ofstream outFile(filename);
        if (!outFile.is_open()) {
            throw LibraryException("Unable to open file '" + filename + "' for writing.");
        }

        outFile << "ID,Title,Author,Available\n";
        for (const auto& pair : books) {
            outFile << pair.second.toCSV() << "\n";
        }
        outFile.close();
    }

    void addBook(int id, const std::string& title, const std::string& author) {
        if (id <= 0) {
            throw LibraryException("Book ID must be a positive integer.");
        }
        if (title.empty() || author.empty()) {
            throw LibraryException("Book title and author cannot be empty.");
        }
        if (books.find(id) != books.end()) {
            throw LibraryException("A book with ID " + std::to_string(id) + " already exists.");
        }

        books[id] = Book(id, sanitize(title), sanitize(author), true);
        saveToFile();
        std::cout << "Book added and saved successfully.\n";
    }

    void listAllBooks() const {
        if (books.empty()) {
            std::cout << "The library currently has no books.\n";
            return;
        }

        std::cout << "\n" << std::string(74, '-') << "\n";
        std::cout << std::left
                  << std::setw(8) << "ID"
                  << std::setw(30) << "Title"
                  << std::setw(22) << "Author"
                  << std::setw(14) << "Status" << "\n";
        std::cout << std::string(74, '-') << "\n";

        for (const auto& pair : books) {
            pair.second.display();
        }
        std::cout << std::string(74, '-') << "\n";
    }

    void searchById(int id) const {
        auto it = books.find(id);
        if (it == books.end()) {
            throw LibraryException("No book found with ID " + std::to_string(id) + ".");
        }

        std::cout << "\nBook Found:\n";
        std::cout << std::string(74, '-') << "\n";
        it->second.display();
        std::cout << std::string(74, '-') << "\n";
    }

    void searchByTitle(const std::string& keyword) const {
        bool found = false;
        std::string lowerKey = keyword;
        std::transform(lowerKey.begin(), lowerKey.end(), lowerKey.begin(), ::tolower);

        std::cout << "\nSearch Results:\n";
        std::cout << std::string(74, '-') << "\n";
        for (const auto& pair : books) {
            std::string lowerTitle = pair.second.getTitle();
            std::transform(lowerTitle.begin(), lowerTitle.end(), lowerTitle.begin(), ::tolower);

            if (lowerTitle.find(lowerKey) != std::string::npos) {
                pair.second.display();
                found = true;
            }
        }
        std::cout << std::string(74, '-') << "\n";

        if (!found) {
            throw LibraryException("No books matched the title keyword: '" + keyword + "'.");
        }
    }

    void issueBook(int id) {
        auto it = books.find(id);
        if (it == books.end()) {
            throw LibraryException("Cannot issue: Unknown Book ID " + std::to_string(id) + ".");
        }

        it->second.issueBook();
        saveToFile();
        std::cout << "Book '" << it->second.getTitle() << "' has been issued.\n";
    }

    void returnBook(int id) {
        auto it = books.find(id);
        if (it == books.end()) {
            throw LibraryException("Cannot return: Unknown Book ID " + std::to_string(id) + ".");
        }

        it->second.returnBook();
        saveToFile();
        std::cout << "Book '" << it->second.getTitle() << "' has been returned.\n";
    }
};

// Safe integer input helper
int readInt(const std::string& prompt) {
    int value;
    std::cout << prompt;
    if (!(std::cin >> value)) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        throw LibraryException("Invalid numeric input. Please enter a valid number.");
    }
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    return value;
}

int main() {
    Library lib("library_data.csv");
    int choice = 0;

    while (true) {
        std::cout << "\n===== LIBRARY MANAGEMENT SYSTEM =====\n"
                  << "1. Add Book\n"
                  << "2. List All Books\n"
                  << "3. Search Book by ID\n"
                  << "4. Search Book by Title\n"
                  << "5. Issue Book\n"
                  << "6. Return Book\n"
                  << "7. Exit\n";

        try {
            choice = readInt("Enter your choice (1-7): ");

            if (choice == 7) {
                std::cout << "Exiting Library Management System. Goodbye!\n";
                break;
            }

            switch (choice) {
                case 1: {
                    int id = readInt("Enter Book ID: ");
                    std::string title, author;
                    std::cout << "Enter Title: ";
                    std::getline(std::cin, title);
                    std::cout << "Enter Author: ";
                    std::getline(std::cin, author);
                    lib.addBook(id, title, author);
                    break;
                }
                case 2:
                    lib.listAllBooks();
                    break;
                case 3: {
                    int id = readInt("Enter Book ID to search: ");
                    lib.searchById(id);
                    break;
                }
                case 4: {
                    std::string keyword;
                    std::cout << "Enter Title keyword: ";
                    std::getline(std::cin, keyword);
                    lib.searchByTitle(keyword);
                    break;
                }
                case 5: {
                    int id = readInt("Enter Book ID to issue: ");
                    lib.issueBook(id);
                    break;
                }
                case 6: {
                    int id = readInt("Enter Book ID to return: ");
                    lib.returnBook(id);
                    break;
                }
                default:
                    throw LibraryException("Invalid menu option. Please select between 1 and 7.");
            }
        } catch (const LibraryException& e) {
            std::cerr << "[Error] " << e.what() << "\n";
        } catch (const std::exception& e) {
            std::cerr << "[System Error] " << e.what() << "\n";
        }
    }

    return 0;
}