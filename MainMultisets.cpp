#include <iostream>
#include <stdexcept>
#include <string>

#include "CantorMultiset.h"

namespace {

void printMenu() {
    std::cout << "\n=== Cantor Multiset menu ===\n"
                 " 1. Multiset A from string\n"
                 " 2. Multiset B from string\n"
                 " 3. Show A and B\n"
                 " 4. Add atom to A (with count)\n"
                 " 5. Add nested multiset to A\n"
                 " 6. Remove atom from A (with count)\n"
                 " 7. Count of atom in A\n"
                 " 8. Union of A and B (max counts)\n"
                 " 9. A + B (sum of counts)\n"
                 "10. A * B (intersection)\n"
                 "11. A - B (difference)\n"
                 "12. Power set of A\n"
                 "13. Compare A == B\n"
                 "14. Size of A (total / distinct)\n"
                 " 0. Exit\n> ";
}

std::string readLine(const std::string& prompt) {
    std::cout << prompt;
    std::string s;
    std::getline(std::cin, s);
    return s;
}

std::size_t readCount(const std::string& prompt) {
    int n = std::stoi(readLine(prompt));
    if (n < 1) throw std::invalid_argument("count must be at least 1");
    return static_cast<std::size_t>(n);
}

}  // namespace

int main() {
    CantorMultiset a, b;
    while (true) {
        printMenu();
        std::string choice;
        if (!std::getline(std::cin, choice)) break;
        try {
            if (choice == "0") {
                break;
            } else if (choice == "1") {
                a = CantorMultiset::fromString(readLine("Enter A, e.g. {a, a, {b, b}}: "));
            } else if (choice == "2") {
                b = CantorMultiset::fromString(readLine("Enter B: "));
            } else if (choice == "3") {
                std::cout << "A = " << a << "\nB = " << b << "\n";
            } else if (choice == "4") {
                std::string atom = readLine("Atom: ");
                a.addAtom(atom, readCount("Count: "));
                std::cout << "Added\n";
            } else if (choice == "5") {
                a.addSet(CantorMultiset::fromString(readLine("Nested multiset: ")));
                std::cout << "Added\n";
            } else if (choice == "6") {
                std::string atom = readLine("Atom: ");
                std::cout << "Removed " << a.removeAtom(atom, readCount("Count: ")) << "\n";
            } else if (choice == "7") {
                std::cout << a.countAtom(readLine("Atom: ")) << "\n";
            } else if (choice == "8") {
                std::cout << a.unionWith(b) << "\n";
            } else if (choice == "9") {
                std::cout << a + b << "\n";
            } else if (choice == "10") {
                std::cout << a * b << "\n";
            } else if (choice == "11") {
                std::cout << a - b << "\n";
            } else if (choice == "12") {
                std::cout << a.powerSet() << "\n";
            } else if (choice == "13") {
                std::cout << (a == b ? "A == B\n" : "A != B\n");
            } else if (choice == "14") {
                std::cout << a.size() << " total, " << a.distinctSize() << " distinct\n";
            } else {
                std::cout << "Unknown option\n";
            }
        } catch (const std::exception& e) {
            std::cout << "Error: " << e.what() << "\n";
        }
    }
    return 0;
}