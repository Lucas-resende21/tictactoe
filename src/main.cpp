#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <array>
#include <string>
#include <random>
#include <chrono>

// Classe TicTacToe
class TicTacToe {
private:
    std::array<std::array<char, 3>, 3> board; // Tabuleiro do jogo
    std::mutex board_mutex; // Mutex para controle de acesso ao tabuleiro
    std::condition_variable turn_cv; // Variável de condição para alternância de turnos
    char current_player; // Jogador atual ('X' ou 'O')
    bool game_over; // Estado do jogo
    char winner; // Vencedor do jogo

public:
    TicTacToe() : current_player('X'), game_over(false), winner(' ') {
        // Inicializar o tabuleiro com espaços vazios
        for (auto& row : board) {
            row.fill(' ');
        }
    }

    void display_board() {
        std::lock_guard<std::mutex> lock(board_mutex);
        std::cout << "Current board:\n";
        for (const auto& row : board) {
            for (char cell : row) {
                std::cout << (cell == ' ' ? '.' : cell) << " ";
            }
            std::cout << "\n";
        }
        std::cout << std::endl;
    }

    bool make_move(char player, int row, int col) {
        std::unique_lock<std::mutex> lock(board_mutex);

        // Verificar se o jogo já terminou ou a jogada é inválida
        if (game_over || board[row][col] != ' ' || player != current_player) {
            return false;
        }

        // Realizar a jogada
        board[row][col] = player;
        if (check_win(player)) {
            game_over = true;
            winner = player;
        } else if (check_draw()) {
            game_over = true;
            winner = 'D'; // D para empate
        } else {
            // Alternar o jogador
            current_player = (current_player == 'X') ? 'O' : 'X';
        }

        // Notificar a alternância de turnos
        turn_cv.notify_all();
        return true;
    }

    bool check_win(char player) {
        // Verificar linhas, colunas e diagonais
        for (int i = 0; i < 3; ++i) {
            if ((board[i][0] == player && board[i][1] == player && board[i][2] == player) || // Linha
                (board[0][i] == player && board[1][i] == player && board[2][i] == player)) { // Coluna
                return true;
            }
        }
        // Verificar diagonais
        if ((board[0][0] == player && board[1][1] == player && board[2][2] == player) ||
            (board[0][2] == player && board[1][1] == player && board[2][0] == player)) {
            return true;
        }
        return false;
    }


    bool check_draw() {
        // Verificar se todas as posições estão preenchidas
        for (const auto& row : board) {
            for (char cell : row) {
                if (cell == ' ') return false;
            }
        }
        return true;
    }

    bool is_game_over() {
        std::lock_guard<std::mutex> lock(board_mutex);
        return game_over;
    }

    char get_winner() {
        std::lock_guard<std::mutex> lock(board_mutex);
        return winner;
    }
};

// Classe Player
class Player {
private:
    TicTacToe& game; // Referência para o jogo
    char symbol; // Símbolo do jogador ('X' ou 'O')
    std::string strategy; // Estratégia do jogador

public:
    Player(TicTacToe& g, char s, std::string strat) 
        : game(g), symbol(s), strategy(strat) {}

    void play() {
        if (strategy == "sequential") {
            play_sequential();
        } else if (strategy == "random") {
            play_random();
        }
    }

private:
    void play_sequential() {
        for (int row = 0; row < 3; ++row) {
            for (int col = 0; col < 3; ++col) {
                if (game.make_move(symbol, row, col)) {
                    std::cout << "Player " << symbol << " (Sequential) played at (" 
                              << row << ", " << col << ")\n";
                    return;
                }
            }
        }
    }

    void play_random() {
        std::mt19937 rng(std::chrono::steady_clock::now().time_since_epoch().count());
        std::uniform_int_distribution<int> dist(0, 2);

        while (true) {
            int row = dist(rng);
            int col = dist(rng);
            if (game.make_move(symbol, row, col)) {
                std::cout << "Player " << symbol << " (Random) played at (" 
                          << row << ", " << col << ")\n";
                return;
            }
        }
    }
};
void player_turn(Player& player, TicTacToe& game) {
    while (!game.is_game_over()) {
        player.play();
        std::this_thread::sleep_for(std::chrono::milliseconds(500)); // Pequena pausa para alternância
    }
}
// Função principal
int main() {
    // Inicializar o jogo e os jogadores
    TicTacToe game;
    Player playerX(game, 'X', "sequential");
    Player playerO(game, 'O', "random");

    // Criar as threads para os jogadores
    std::thread t1(player_turn, std::ref(playerX), std::ref(game));
    std::thread t2(player_turn, std::ref(playerO), std::ref(game));

    // Aguardar o término das threads
    t1.join();
    t2.join();

    // Exibir o resultado final do jogo
    game.display_board();
    if (game.get_winner() == 'D') {
        std::cout << "It's a draw!" << std::endl;
    } else {
        std::cout << "The winner is " << game.get_winner() << "!" << std::endl;
    }

    return 0;
}