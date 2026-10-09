#include <iostream>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <string>

#include "CantorSet.h"

namespace {

void printMenu() {
    std::cout << "\n=== Cantor Set menu ===\n"
                 " 1. Set A from string\n"
                 " 2. Set B from string\n"
                 " 3. Show A and B\n"
                 " 4. Add atom to A\n"
                 " 5. Add nested set to A\n"
                 " 6. Remove atom from A\n"
                 " 7. Check atom in A\n"
                 " 8. A + B (union)\n"
                 " 9. A * B (intersection)\n"
                 "10. A - B (difference)\n"
                 "11. Power set of A\n"
                 "12. Compare A == B\n"
                 "13. Size of A\n"
                 " 0. Exit\n> ";
}

std::string readLine(const std::string& prompt) {
    std::cout << prompt;
    std::string s;
    std::getline(std::cin, s);
    return s;
}

}  // namespace

int main() {
    CantorSet a, b;
    while (true) {
        printMenu();
        std::string choice;
        if (!std::getline(std::cin, choice)) break;
        try {
            if (choice == "0") {
                break;
            } else if (choice == "1") {
                a = CantorSet::fromString(readLine("Enter A, e.g. {a, b, {c}}: "));
            } else if (choice == "2") {
                b = CantorSet::fromString(readLine("Enter B: "));
            } else if (choice == "3") {
                std::cout << "A = " << a << "\nB = " << b << "\n";
            } else if (choice == "4") {
                std::cout << (a.addAtom(readLine("Atom: ")) ? "Added\n" : "Already present\n");
            } else if (choice == "5") {
                CantorSet s = CantorSet::fromString(readLine("Nested set: "));
                std::cout << (a.addSet(s) ? "Added\n" : "Already present\n");
            } else if (choice == "6") {
                std::cout << (a.removeAtom(readLine("Atom: ")) ? "Removed\n" : "Not found\n");
            } else if (choice == "7") {
                std::cout << (a.containsAtom(readLine("Atom: ")) ? "Yes\n" : "No\n");
            } else if (choice == "8") {
                std::cout << a + b << "\n";
            } else if (choice == "9") {
                std::cout << a * b << "\n";
            } else if (choice == "10") {
                std::cout << a - b << "\n";
            } else if (choice == "11") {
                std::cout << a.powerSet() << "\n";
            } else if (choice == "12") {
                std::cout << (a == b ? "A == B\n" : "A != B\n");
            } else if (choice == "13") {
                std::cout << a.size() << "\n";
            } else {
                std::cout << "Unknown option\n";
            }
        } catch (const std::exception& e) {
            std::cout << "Error: " << e.what() << "\n";
        }
    }
    return 0;
}