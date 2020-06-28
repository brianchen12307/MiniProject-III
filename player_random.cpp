#include <iostream>
#include <fstream>
#include <array>
#include <vector>
#include <cstdlib>
#include <limits.h>
#include <ctime>
#define AIdepth 4
using namespace std;
struct Point {
    int x, y, state_value;
    Point() : Point(0, 0, 0) {}
    Point(int x, int y, int val = 0) : x(x), y(y), state_value(val) {}
    Point(const Point& rhs) {
        *this = rhs;
    }
    Point& operator=(Point const& rhs) {
        this->x = rhs.x; 
        this->y = rhs.y;
        this->state_value = rhs.state_value;
        return *this;
    }
    Point operator+(const Point& rhs) const {
        return Point(x + rhs.x, y + rhs.y);
    }
};

int player;
const int SIZE = 8;
std::array<std::array<int, SIZE>, SIZE> board;
std::vector<Point> next_valid_spots;

int map[8][8] = {
 {30, 20, 8, 8, 8, 8, 20, 30},
 {20, 4, 4, 3, 3, 4, 4, 20},
 {8, 3, 2, 2, 2, 2, 4, 8},
 {8, 3, 2, 0, 0, 2, 3, 8},
 {8, 3, 2, 0, 0, 2, 3, 8},
 {8, 3, 2, 2, 2, 2, 4, 8},
 {20, 4, 3, 3, 3, 3, 4, 20},
 {30, 20, 8, 8, 8, 8, 20, 30}
};

class State {
public:
    enum SPOT_STATE {
        EMPTY = 0,
        BLACK = 1,
        WHITE = 2
    };
    static const int SIZE = 8;
    const std::array<Point, 8> directions{ {
        Point(-1, -1), Point(-1, 0), Point(-1, 1),
        Point(0, -1), /*{0, 0}, */Point(0, 1),
        Point(1, -1), Point(1, 0), Point(1, 1)
    } };
    std::array<std::array<int, SIZE>, SIZE> board;
    std::vector<Point> next_valid_spots;
    std::array<int, 3> disc_count;
    int cur_player, eat_discs, place_advantage;
private:
    // Turn control to the other player.
    int get_next_player(int player) const {
        return 3 - player;
    }
    bool is_spot_on_board(Point p) const {
        return 0 <= p.x && p.x < SIZE && 0 <= p.y && p.y < SIZE;
    }
    //get disc color
    int get_disc(Point p) const {
        return board[p.x][p.y];
    }
    void set_disc(Point p, int disc) {
        board[p.x][p.y] = disc;
    }
    //whether the disc at that spot is the specific color or not.
    bool is_disc_at(Point p, int color) const {
        if (!is_spot_on_board(p))
            return false;
        if (get_disc(p) != color)
            return false;
        return true;
    }
    bool is_spot_valid(Point center) const {
        if (get_disc(center) != EMPTY)
            return false;
        for (Point dir : directions) {
            // Move along the direction while testing.
            Point p = center + dir;
            if (!is_disc_at(p, get_next_player(cur_player)))
                continue;
            p = p + dir;
            while (is_spot_on_board(p) && get_disc(p) != EMPTY) {
                if (is_disc_at(p, cur_player))
                    return true;
                p = p + dir;
            }
        }
        return false;
    }
    void flip_discs(Point center) {
        for (Point dir : directions) {
            // Move along the direction while testing.
            Point p = center + dir;
            if (!is_disc_at(p, get_next_player(cur_player)))
                continue;
            std::vector<Point> discs({ p });
            p = p + dir;
            while (is_spot_on_board(p) && get_disc(p) != EMPTY) {
                if (is_disc_at(p, cur_player)) {
                    for (Point s : discs) {
                        set_disc(s, cur_player);
                    }
                    disc_count[cur_player] += discs.size();
                    disc_count[get_next_player(cur_player)] -= discs.size();
                    eat_discs = discs.size();
                    break;
                }
                discs.push_back(p);
                p = p + dir;
            }
        }
    }
public:
    State(int player, std::array<std::array<int, SIZE>, SIZE> board, std::vector<Point> next_valid_spots) {
        cur_player = player;
        for (int i = 0; i < SIZE; i++) {
            for (int j = 0; j < SIZE; j++) {
                this->board[i][j] = board[i][j];
                if (board[i][j] == EMPTY) disc_count[EMPTY]++;
                else if (board[i][j] == BLACK) disc_count[BLACK]++;
                else if (board[i][j] == WHITE) disc_count[WHITE]++;
            }
        }

        for (Point it :next_valid_spots) {
            this->next_valid_spots.push_back(it);
        }
    }

    State(const State& rhs) {
        *this = rhs;
    }

    State& operator=(const State&rhs) {
        this->cur_player = rhs.cur_player;
        disc_count[EMPTY] = rhs.disc_count[EMPTY];
        disc_count[BLACK] = rhs.disc_count[BLACK];
        disc_count[WHITE] = rhs.disc_count[WHITE];

        this->next_valid_spots.clear();
        for (Point it : rhs.next_valid_spots)
            this->next_valid_spots.push_back(it);

        for (int i = 0; i < SIZE; i++) {
            for (int j = 0; j < SIZE; j++) {
                this->board[i][j] = rhs.board[i][j];
            }
        }
        return *this;
    }

    std::vector<Point> get_valid_spots() const {
        std::vector<Point> valid_spots;
        for (int i = 0; i < SIZE; i++) {
            for (int j = 0; j < SIZE; j++) {
                Point p = Point(i, j);
                if (board[i][j] != EMPTY)
                    continue;
                if (is_spot_valid(p))
                    valid_spots.push_back(p);
            }
        }
        return valid_spots;
    }

    void put_disc(Point p) {
        set_disc(p, cur_player);
        disc_count[cur_player]++;
        disc_count[EMPTY]--;
        flip_discs(p);
        place_advantage = map[p.x][p.y];
        
        cur_player = get_next_player(cur_player);
        next_valid_spots = get_valid_spots();
    }

    int curValue() {
        float value;
        float rate = disc_count[cur_player] / disc_count[3 - cur_player] * 3;

        value = rate + (float)next_valid_spots.size() * 2+ eat_discs * 2 + place_advantage * 5;
        return value;
    }
};

Point minmax(State board, int depth) {
    if (depth == AIdepth || board.next_valid_spots.empty()) {
        Point tmp;
        tmp.state_value = board.curValue();
        return tmp;
    }
    else {
        if (board.cur_player == player) {
            Point maxPoint;
            maxPoint.state_value = INT_MIN;
            for (Point it : board.next_valid_spots) {
                State nextBoard = board;
                nextBoard.put_disc(it);
                it.state_value = minmax(nextBoard, depth + 1).state_value;
                maxPoint = (it.state_value > maxPoint.state_value) ? it : maxPoint;
            }
            return maxPoint;
        }
        else {
            Point minPoint;
            minPoint.state_value = INT_MAX;
            for (Point it : board.next_valid_spots) {
                State nextBoard = board;
                nextBoard.put_disc(it);
                it.state_value = minmax(nextBoard, depth + 1).state_value;
                minPoint = (it.state_value < minPoint.state_value) ? it : minPoint;
            }
            return minPoint;
        }
    }
}

Point alphabeta(State state, Point a, Point b, int depth) {
    if (depth == AIdepth || state.next_valid_spots.empty()) {
        Point tmp(0, 0, state.curValue());
        return tmp;
    }
    else {
        if (state.cur_player == player) {
            Point maxPoint;
            for (Point it : state.next_valid_spots) {
                maxPoint = it;
                State nextstate = state;
                nextstate.put_disc(it);

                a.state_value = max(a.state_value, alphabeta(nextstate, a, b, depth + 1).state_value);
                maxPoint.state_value = a.state_value;

                //pruning //b is from min, which is from (cur_depth - 1),
                          //if the first state_value sampled from (cur_depth + 1) is larger then b,
                          //it can promise that in (cur_depth - 1), the sampled state_value below the
                          //(cur_depth + 1) will be ignored / pruned.
                if (b.state_value <= a.state_value) break;
            }
            return maxPoint;
        }
        else {
            Point minPoint;
            for (Point it : state.next_valid_spots) {
                minPoint = it;
                State nextstate = state;
                nextstate.put_disc(it);

                b.state_value = min(b.state_value, alphabeta(nextstate, a, b, depth + 1).state_value);
                minPoint.state_value = b.state_value;

                if (b.state_value <= a.state_value) break;
            }
            return minPoint;
        }
    }
}

void read_board(std::ifstream& fin) {
    fin >> player;
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            fin >> board[i][j];
        }
    }
}

void read_valid_spots(std::ifstream& fin) {
    int n_valid_spots;
    fin >> n_valid_spots;
    int x, y;
    for (int i = 0; i < n_valid_spots; i++) {
        fin >> x >> y;
        next_valid_spots.push_back({ x, y });
    }
}

void write_valid_spot(std::ofstream& fout) {
    // Remember to flush the output to ensure the last action is written to file.
    Point p;

    //alphabeta
    State AI(player, board, next_valid_spots);
    Point a(0, 0, INT_MIN), b(0, 0, INT_MAX);
    p = alphabeta(AI, a, b, 1);
    
    //minimax
    //p = minmax(AI, 1);

    
    fout << p.x << " " << p.y << std::endl;
    fout.flush();
}


int main(int, char** argv) {
    std::ifstream fin(argv[1]);
    std::ofstream fout(argv[2]);
    read_board(fin);
    read_valid_spots(fin);
    write_valid_spot(fout);
    cout << amount << endl;
    system("pause");
    fin.close();
    fout.close();
    return 0;
}
