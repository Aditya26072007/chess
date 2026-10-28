#include <iostream>
#include <string>
#include <cctype>
#include "Game.h"

bool parseSquare(const std::string& s, Position& out) {
    if (s.size() != 2) return false;
    char file = std::tolower(static_cast<unsigned char>(s[0]));
    char rank = s[1];
    if (file < 'a' || file > 'h' || rank < '1' || rank > '8') return false;
    out.col = file - 'a';
    out.row = 8 - (rank - '0');
    return true;
}

int main() {
    Game game;
    std::string a, b;

    while (game.getState() != GameState::CHECKMATE) {
        std::cout << game.getBoard();
        if (game.getState() == GameState::CHECK) std::cout << "Check!" << std::endl;
        std::cout << (game.getCurrentTurn() == Color::WHITE ? "White" : "Black")
                  << " to move (e.g. e2 e4, or 'quit'): ";

        if (!(std::cin >> a)) break;
        if (a == "quit") break;
        if (!(std::cin >> b)) break;

        Position from, to;
        if (!parseSquare(a, from) || !parseSquare(b, to)) {
            std::cout << "Invalid input. Use squares like e2 e4." << std::endl;
            continue;
        }
        if (!game.makeMove(from, to))
            std::cout << "Illegal move, try again." << std::endl;
    }

    if (game.getState() == GameState::CHECKMATE) {
        std::cout << game.getBoard();
        std::cout << "Checkmate - "
                  << (game.getCurrentTurn() == Color::WHITE ? "Black" : "White")
                  << " wins" << std::endl;
    }
}