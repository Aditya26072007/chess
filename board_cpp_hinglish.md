# Board.cpp — Line-by-Line Hinglish Explanation

---

## Line 1: `#include "Board.h"`
> **Board.h** file ko include kar rahe hain — isme Board class ki **declaration** hai (grid, methods wagairah). Bina iske compiler ko pata nahi chalega ki Board kya hai.

---

## Lines 3–12: `makeBackRankPiece()` — Helper Function

```cpp
static std::unique_ptr<Piece> makeBackRankPiece(int col, Color c) {
```
> Ye ek **static helper function** hai. `static` matlab ye sirf **is file ke andar** use ho sakta hai, bahar se koi call nahi kar sakta.
>
> Kaam: Column number (0–7) ke basis pe **back rank ka piece** banata hai (Rook, Knight, Bishop, Queen, King).
>
> Return type `unique_ptr<Piece>` hai — matlab ye **smart pointer** return karta hai jo automatically memory manage karega.

```cpp
    switch (col) {
        case 0: case 7: return std::make_unique<Rook>(c);
```
> Column 0 (a-file) aur Column 7 (h-file) pe **Rook** (haathi) baithta hai.

```cpp
        case 1: case 6: return std::make_unique<Knight>(c);
```
> Column 1 (b-file) aur Column 6 (g-file) pe **Knight** (ghoda) baithta hai.

```cpp
        case 2: case 5: return std::make_unique<Bishop>(c);
```
> Column 2 (c-file) aur Column 5 (f-file) pe **Bishop** (oont) baithta hai.

```cpp
        case 3:         return std::make_unique<Queen>(c);
```
> Column 3 (d-file) pe **Queen** (wazir/rani) baithti hai.

```cpp
        default:        return std::make_unique<King>(c);   // col 4
```
> Baaki bachta hai Column 4 (e-file) — yahan **King** (raja/badshah) baithta hai.

> [!TIP]
> Chess ka standard setup yaad karo: **R N B Q K B N R** — yahi pattern ye function follow karta hai.

---

## Lines 14–27: `setupBoard()` — Board Ko Set Karo

```cpp
void Board::setupBoard() {
```
> Ye method poore board pe **32 pieces** place karta hai — initial chess position set karta hai.

```cpp
    for (auto& row : grid)
        for (auto& square : row)
            square.reset();
```
> Pehle **saare squares khali kar do** — `reset()` call karne se `unique_ptr` apna piece delete kar deta hai aur `nullptr` ho jaata hai.
> Ye zaroori hai agar koi pehle se game chal rahi thi aur dobara setup karna ho.

```cpp
    for (int col = 0; col < 8; col++) {
```
> Ab 0 se 7 tak har column pe pieces place karenge.

```cpp
        grid[0][col] = makeBackRankPiece(col, Color::BLACK);
```
> **Row 0 = Rank 8** (board ka sabse upar) — yahan **Black ke back-rank pieces** (Rook, Knight, etc.) baith rahe hain.

```cpp
        grid[1][col] = std::make_unique<Pawn>(Color::BLACK);
```
> **Row 1 = Rank 7** — yahan **Black ke 8 Pawns** (pyaade/sipahi) baith rahe hain.

```cpp
        grid[6][col] = std::make_unique<Pawn>(Color::WHITE);
```
> **Row 6 = Rank 2** — yahan **White ke 8 Pawns** baith rahe hain.

```cpp
        grid[7][col] = makeBackRankPiece(col, Color::WHITE);
```
> **Row 7 = Rank 1** (board ka sabse neeche) — yahan **White ke back-rank pieces** baith rahe hain.

> [!NOTE]
> **Row 2 se Row 5 tak (Rank 6 to Rank 3)** — koi piece nahi, sab khali hain (already `reset()` se `nullptr` hai).

**Board ka structure ab aisa dikhta hai:**
```
Row 0:  r n b q k b n r   ← Black back rank
Row 1:  p p p p p p p p   ← Black pawns
Row 2:  . . . . . . . .   ← khali
Row 3:  . . . . . . . .   ← khali
Row 4:  . . . . . . . .   ← khali
Row 5:  . . . . . . . .   ← khali
Row 6:  P P P P P P P P   ← White pawns
Row 7:  R N B Q K B N R   ← White back rank
```

---

## Lines 29–32: `getPieceAt()` — Kisi Square Ka Piece Lo

```cpp
Piece* Board::getPieceAt(Position pos) const {
    if (!pos.isValid()) return nullptr;
    return grid[pos.row][pos.col].get();
}
```
> **Kaam:** Diye gaye position pe jo piece hai wo return karo.
>
> 1. Pehle check karo — position **valid** hai? (row aur col dono 0–7 ke beech hain?) Agar nahi, toh `nullptr` return karo (koi piece nahi).
> 2. Agar valid hai, toh `grid[row][col]` se **raw pointer** (`.get()`) return karo.
>
> `const` matlab ye method board ko **modify nahi karega**, sirf read karega.
>
> Agar square khali hai, toh `unique_ptr` mein kuch nahi hoga, toh `.get()` waise bhi `nullptr` return karega.

---

## Lines 34–36: `isSquareOccupied()` — Kya Square Pe Koi Hai?

```cpp
bool Board::isSquareOccupied(Position pos) const {
    return getPieceAt(pos) != nullptr;
}
```
> Bahut simple — `getPieceAt()` call karo. Agar `nullptr` nahi aaya, matlab **koi piece hai** wahan → `true` return karo.
>
> Ye Pawn ke move mein use hota hai — pawn sirf aage tabhi ja sakta hai jab **saamne wala square khali** ho.

---

## Lines 38–41: `isEnemyPiece()` — Kya Wo Dushman Ka Piece Hai?

```cpp
bool Board::isEnemyPiece(Position pos, Color myColor) const {
    Piece* p = getPieceAt(pos);
    return p != nullptr && p->getColor() != myColor;
}
```
> **Kaam:** Check karo ki diye gaye square pe **dushman ka piece** hai ya nahi.
>
> 1. Pehle wahan ka piece lo.
> 2. Agar piece **exist karta hai** (`!= nullptr`) **AUR** uska color **mera color nahi hai** → toh haan, ye dushman ka piece hai.
>
> Ye Pawn ke diagonal capture mein use hota hai — pawn sirf **tirchha (diagonal)** tabhi ja sakta hai jab wahan **enemy piece** ho.

---

## Lines 43–47: `movePiece()` — Piece Ko Move Karo

```cpp
std::unique_ptr<Piece> Board::movePiece(Position from, Position to) {
    std::unique_ptr<Piece> captured = std::move(grid[to.row][to.col]);
    grid[to.row][to.col] = std::move(grid[from.row][from.col]);
    return captured;
}
```
> Ye sabse **important** method hai. Step by step:
>
> 1. **`captured = std::move(grid[to][to])`** — Destination square pe agar koi piece tha (enemy piece), usse **capture** karke side mein rakh lo. `std::move` se ownership transfer hoti hai.
>    - Agar square khali tha, toh `captured` mein `nullptr` aayega.
>
> 2. **`grid[to] = std::move(grid[from])`** — Ab **source piece ko destination pe move** kar do. Source square ab **khali** ho jaata hai automatically.
>
> 3. **`return captured`** — Jo piece capture hua tha, usse return karo. Ye `undoMove()` mein kaam aayega agar move undo karna ho.

> [!IMPORTANT]
> `std::move` koi piece physically move nahi karta — ye sirf **ownership transfer** karta hai. Jab `unique_ptr` move hota hai, toh purana pointer `nullptr` ban jaata hai.

---

## Lines 49–52: `undoMove()` — Move Wapas Lo (Undo)

```cpp
void Board::undoMove(Position from, Position to, std::unique_ptr<Piece> captured) {
    grid[from.row][from.col] = std::move(grid[to.row][to.col]);
    grid[to.row][to.col] = std::move(captured);
}
```
> Ye `movePiece()` ka **exact ulta** hai:
>
> 1. **`grid[from] = move(grid[to])`** — Piece ko **wapas original position** pe rakh do (destination se source pe).
> 2. **`grid[to] = move(captured)`** — Agar koi piece capture hua tha, usse **wapas destination pe rakh do**.
>
> **Ye kab use hota hai?** Jab `Game` class check karti hai — "agar ye move karu toh kya mera **raja (King) check mein aa jaayega?**" Toh wo:
> 1. Move karta hai (`movePiece`)
> 2. Check karta hai (`isInCheck`)
> 3. Move **undo** karta hai (`undoMove`) — taaki board pehle jaisa ho jaaye
>
> Iss pattern ko **Simulate → Test → Undo** kehte hain.

---

## Lines 54–62: `findKing()` — Raja Kahan Hai?

```cpp
Position Board::findKing(Color color) const {
    for (int r = 0; r < 8; r++)
        for (int c = 0; c < 8; c++) {
            Piece* p = grid[r][c].get();
            if (p && p->getColor() == color && dynamic_cast<King*>(p))
                return Position{r, c};
        }
    return Position{-1, -1};
}
```
> **Kaam:** Board pe given color ka **King (raja)** dhoondo aur uski position return karo.
>
> 1. **Saare 64 squares scan karo** (8×8 loop).
> 2. Har square pe check karo:
>    - `p` — koi piece hai? (null nahi hai?)
>    - `p->getColor() == color` — sahi color ka hai?
>    - `dynamic_cast<King*>(p)` — ye **actually King hai?** `dynamic_cast` RTTI (Runtime Type Information) use karta hai — agar `p` sach mein `King` type ka hai toh pointer milega, warna `nullptr`.
> 3. Agar mila, toh uski **Position return** kar do.
> 4. Agar poora board scan kar liya aur nahi mila → `{-1, -1}` return karo (invalid position, matlab King nahi mili — ye normally nahi hona chahiye).
>
> **Ye kab use hota hai?** `isInCheck()` mein — pehle King ka position pata karo, phir check karo ki koi enemy piece us tak pahunch sakta hai ya nahi.

---

## Lines 64–75: `printBoard()` — Board Print Karo

```cpp
void Board::printBoard(std::ostream& os) const {
    os << std::endl << "    a b c d e f g h" << std::endl;
```
> Sabse pehle **column labels** print karo — `a` se `h` tak (chess ki files).

```cpp
    for (int r = 0; r < 8; r++) {
        os << " " << 8 - r << "  ";
```
> Har row ke shuru mein **rank number** print karo. Row 0 ka rank = 8, Row 7 ka rank = 1, isliye `8 - r`.

```cpp
        for (int c = 0; c < 8; c++) {
            Piece* p = grid[r][c].get();
            os << (p ? p->getSymbol() : '.') << ' ';
        }
```
> Har square ke liye:
> - Agar piece hai → uska **symbol** print karo (e.g., `K`, `p`, `R`)
> - Agar khali hai → **dot (`.`)** print karo
>
> `p ? p->getSymbol() : '.'` — ye **ternary operator** hai (if-else ka shortcut).
>
> **Uppercase = White** (K, Q, R, B, N, P), **Lowercase = Black** (k, q, r, b, n, p).

```cpp
        os << " " << 8 - r << std::endl;
    }
    os << "    a b c d e f g h" << std::endl;
}
```
> Har row ke end mein bhi **rank number** print karo (readability ke liye), aur last mein dobara **column labels**.

**Output aisa dikhta hai:**
```
    a b c d e f g h
 8  r n b q k b n r  8
 7  p p p p p p p p  7
 6  . . . . . . . .  6
 5  . . . . . . . .  5
 4  . . . . . . . .  4
 3  . . . . . . . .  3
 2  P P P P P P P P  2
 1  R N B Q K B N R  1
    a b c d e f g h
```

---

## Lines 77–80: `operator<<` Overload

```cpp
std::ostream& operator<<(std::ostream& os, const Board& board) {
    board.printBoard(os);
    return os;
}
```
> Ye **operator overloading** hai. Iska matlab ab hum Board ko **directly `cout` se print** kar sakte hain:
> ```cpp
> std::cout << board;   // ye kaam karega ab!
> ```
> Bina iske hume likhna padta: `board.printBoard(std::cout);`
>
> `return os;` isliye karte hain taaki **chaining** ho sake: `cout << board << "kuch aur";`

---

## Summary Table

| Method | Ek Line Mein Kya Karta Hai |
|---|---|
| `makeBackRankPiece()` | Column number se sahi piece banata hai (R/N/B/Q/K) |
| `setupBoard()` | 32 pieces ko starting position pe rakhta hai |
| `getPieceAt()` | Kisi square ka piece return karta hai (ya nullptr) |
| `isSquareOccupied()` | Square khali hai ya nahi — true/false |
| `isEnemyPiece()` | Dushman ka piece hai kya — true/false |
| `movePiece()` | Piece move karo, captured piece return karo |
| `undoMove()` | Move undo karo (Simulate-Test-Undo pattern ke liye) |
| `findKing()` | Board pe King dhoondo, position return karo |
| `printBoard()` | Board ko ASCII art mein print karo |
| `operator<<` | `cout << board` kaam kare isliye overload |
