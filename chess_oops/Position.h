#pragma once

struct Position {
    int row;
    int col;

    bool operator==(const Position& other) const {
        return row == other.row && col == other.col;
    }

    bool operator!=(const Position& other) const {
        return !(*this == other);
    }

    bool isValid() const {
        return row >= 0 && row < 8 && col >= 0 && col < 8;
    }
};