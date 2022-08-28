#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define white 0
#define black 1

const struct pieces_t {
    int PAWN[2];
    int ROOK[2];
    int KNIGHT[2];
    int BISHOP[2];
    int QUEEN[2];
    int KING[2];

    char FROM_NUM[7][2][3];
} PIECES = {{1, 11}, {2, 12}, {3, 13}, {4, 14}, {5, 15}, {6, 16}, {{"0 \0", "0 \0"}, {"WP\0", "BP\0"}, {"WR\0", "BR\0"}, {"WK\0", "BK\0"}, {"WB\0", "BB\0"}, {"WQ\0", "BQ\0"}, {"WG\0", "BG\0"}}};

struct castling_t {
    int isLeftRookMoved;
    int isRightRookMoved;
    int isKingMoved;
} castling[2] = {{0, 0, 0}, {0, 0, 0}};

struct enpassant_t {
    int isActive;
    char from[2];
    char to[2];
    char target[2];
} enpassant;

int field[8][8] = {0};

int player = 0;
int t = 0;

void redrawField(char* state){
    printf("---------------------------------------------------\n\n");
    printf("Chess Board (t = %d):\t\t%s\n\n", t++ / 2, state);

    for(int y = 7; y >= 0; y--){
        printf("%d  ", y + 1);

        for(int x = 0; x < 8; x++){
            int piece = field[x][y];

            printf("%5s", PIECES.FROM_NUM[piece % 10][piece > 10]);
        }

        printf("\n\n");
    }

    printf("      A    B    C    D    E    F    G    H\n\n\n");
    printf("Player %d (%c):", player + 1, player ? 'B' : 'W');
}

void arrangeFigures(){
    for(int y = 0; y < 8; y++){
        if((y > 1) && (y < 6))
            continue;

        int color = y > 1;

        if(!(y % 7)){
            field[0][y] = PIECES.ROOK[color];
            field[1][y] = PIECES.KNIGHT[color];
            field[2][y] = PIECES.BISHOP[color];
            field[3][y] = PIECES.QUEEN[color];
            field[4][y] = PIECES.KING[color];
            field[5][y] = PIECES.BISHOP[color];
            field[6][y] = PIECES.KNIGHT[color];
            field[7][y] = PIECES.ROOK[color];
        }else
            for(int x = 0; x < 8; x++){
                field[x][y] = PIECES.PAWN[color];
            }
    }
}

void play(char initial_pos[2], int final_pos[2]){
    field[final_pos[0] - 1][final_pos[1] - 1] = getPieceType(initial_pos);
    field[initial_pos[0] - 'A'][initial_pos[1] - '1'] = 0;

    if((initial_pos[0] == enpassant.from[0]) && (initial_pos[1] == enpassant.from[1]) &&
       ((final_pos[0] + 'A' - 1) == enpassant.to[0]) && ((final_pos[1] + '1' - 1) == enpassant.to[1]))
        field[enpassant.target[0] - 'A'][enpassant.target[1] - '1'] = 0;

    player = (player + 1) % 2;
}

int getPieceType(char pos[2]){
    return field[pos[0] - 'A'][pos[1] - '1'];
}

int isAvailable(int pos[2], int color, int special){
    if((pos[0] < 0) || (pos[0] > 7) || (pos[1] < 0) || (pos[1] > 7))
        return 0;

    return special == 0 ? (field[pos[0]][pos[1]] == 0) || ((field[pos[0]][pos[1]] > 10) ^ color) : special == 1 ? field[pos[0]][pos[1]] == 0 : (field[pos[0]][pos[1]] != 0) && ((field[pos[0]][pos[1]] > 10) ^ color);
}

void getMoves(char pos[2], int color, int vector[3], int depth, int moves[][2], int* start, int special){
    if(depth == 0)
        return;

    int x = pos[0] - 'A';
    int y = pos[1] - '1';

    int unit_x = vector[0];
    int unit_y = vector[1];
    int mirror = vector[2];

    int new_vector[3] = {unit_x, unit_y, 0};

    if(mirror){
        int temp_vector[3] = {unit_x, unit_y, 0};

        if(unit_y != 0){
            temp_vector[1] *= -1;
            getMoves(pos, color, temp_vector, depth, moves, start, special);
        }

        if(unit_x != 0){
            temp_vector[0] *= -1;
            getMoves(pos, color, temp_vector, depth, moves, start, special);

            if(unit_y != 0){
                temp_vector[1] *= -1;
                getMoves(pos, color, temp_vector, depth, moves, start, special);
            }
        }
    }

    int new_pos[2] = {x + unit_x, y + unit_y};

    if(isAvailable(new_pos, color, special)){
        moves[*start][0] = new_pos[0] + 1;
        moves[(*start)++][1] = new_pos[1] + 1;

        char temp_pos[2] = {'A' + new_pos[0], '1' + new_pos[1]};

        if(isAvailable(new_pos, color, 1))
            getMoves(temp_pos, color, new_vector, depth - 1, moves, start, special);
    }
}

int getPawnMoves(char pos[2], int color, int moves[][2]){
    int y = pos[1] - '1';

    int start = 0;

    if(!color){
        int vector[3] = {0, 1, 0};

        if(y == 1)
            getMoves(pos, color, vector, 2, moves, &start, 1);
        else
            getMoves(pos, color, vector, 1, moves, &start, 1);

        vector[0] = -1;
        getMoves(pos, color, vector, 1, moves, &start, 2);

        vector[0] = 1;
        getMoves(pos, color, vector, 1, moves, &start, 2);
    }else{
        int vector[3] = {0, -1, 0};

        if(y == 6)
            getMoves(pos, color, vector, 2, moves, &start, 1);
        else
            getMoves(pos, color, vector, 1, moves, &start, 1);

        vector[0] = -1;
        getMoves(pos, color, vector, 1, moves, &start, 2);

        vector[0] = 1;
        getMoves(pos, color, vector, 1, moves, &start, 2);
    }

    if((pos[0] == enpassant.from[0]) && (pos[1] == enpassant.from[1])){
        moves[start][0] = enpassant.to[0] - 'A' + 1;
        moves[start++][1] = enpassant.to[1] - '1' + 1;
    }

    return start;
}

int getRookMoves(char pos[2], int color, int moves[][2]){
    int start = 0;

    int vector[3] = {0, 1, 1};
    getMoves(pos, color, vector, 8, moves, &start, 0);

    vector[0] = 1;
    vector[1] = 0;
    getMoves(pos, color, vector, 8, moves, &start, 0);

    return start;
}

int getKnightMoves(char pos[2], int color, int moves[][2]){
    int start = 0;

    int vector[3] = {1, 2, 1};
    getMoves(pos, color, vector, 1, moves, &start, 0);

    vector[0] = 2;
    vector[1] = 1;
    getMoves(pos, color, vector, 1, moves, &start, 0);

    return start;
}

int getBishopMoves(char pos[2], int color, int moves[][2]){
    int start = 0;

    int vector[3] = {1, 1, 1};
    getMoves(pos, color, vector, 8, moves, &start, 0);

    return start;
}

int getQueenMoves(char pos[2], int color, int moves[][2]){
    int start = 0;

    int vector[3] = {1, 1, 1};
    getMoves(pos, color, vector, 8, moves, &start, 0);

    vector[0] = 0;
    getMoves(pos, color, vector, 8, moves, &start, 0);

    vector[0] = 1;
    vector[1] = 0;
    getMoves(pos, color, vector, 8, moves, &start, 0);

    return start;
}

int getKingMoves(char pos[2], int color, int moves[][2]){
    int start = 0;

    int vector[3] = {1, 1, 1};
    getMoves(pos, color, vector, 1, moves, &start, 0);

    vector[0] = 0;
    getMoves(pos, color, vector, 1, moves, &start, 0);

    vector[0] = 1;
    vector[1] = 0;
    getMoves(pos, color, vector, 1, moves, &start, 0);

    if(!castling[color].isKingMoved && !castling[color].isLeftRookMoved){
        int temp_moves[2][2];
        int temp_vector[3] = {1, 0, 0};
        int temp = 0;

        getMoves(pos, color, temp_vector, 2, temp_moves, &temp, 0);
        moves[start][0] = temp_moves[1][0];
        moves[start++][1] = temp_moves[1][1];
    }

    if(!castling[color].isKingMoved && !castling[color].isRightRookMoved){
        int temp_moves[2][2];
        int temp_vector[3] = {-1, 0, 0};
        int temp = 0;

        getMoves(pos, color, temp_vector, 2, temp_moves, &temp, 0);
        moves[start][0] = temp_moves[1][0];
        moves[start++][1] = temp_moves[1][1];
    }

    return start;
}

int getPossibleMoves(char pos[2], int moves[][2]){
    int piece = getPieceType(pos);
    int color = piece > 10;

    if(color != player)
        return 0;

    switch(piece % 10){
        case 1:
            getPawnMoves(pos, color, moves);
            break;

        case 2:
            getRookMoves(pos, color, moves);
            break;

        case 3:
            getKnightMoves(pos, color, moves);
            break;

        case 4:
            getBishopMoves(pos, color, moves);
            break;

        case 5:
            getQueenMoves(pos, color, moves);
            break;

        case 6:
            getKingMoves(pos, color, moves);
            break;
    }

    return removeDangerous(pos, color, moves);
}

int removeDangerous(char pos[2], int color, int moves[][2]){
    int new_moves[64][2] = {0};
    int j = 0;

    for(int i = 0; (moves[i][0] != 0) && (moves[i][1] != 0); i++){
        int initial_pos[2] = {pos[0] - 'A', pos[1] - '1'};
        int final_pos[2] = {moves[i][0] - 1, moves[i][1] - 1};

        int origs[2] = {field[initial_pos[0]][initial_pos[1]], field[final_pos[0]][final_pos[1]]};

        play(pos, moves[i]);
        player = (player + 1) % 2;

        if(!isChecked(color)){
            new_moves[j][0] = moves[i][0];
            new_moves[j++][1] = moves[i][1];
        }

        field[initial_pos[0]][initial_pos[1]] = origs[0];
        field[final_pos[0]][final_pos[1]] = origs[1];

        if(enpassant.isActive && (initial_pos[0] == (enpassant.from[0] - 'A')) && (initial_pos[1] == (enpassant.from[1] - '1')) &&
           (final_pos[0] == (enpassant.to[0] - 'A')) && (final_pos[1] == (enpassant.to[1] - '1')))
            field[enpassant.target[0] - 'A'][enpassant.target[1] - '1'] = PIECES.PAWN[!color];
    }

    for(int i = 0; i < 64; i++){
        moves[i][0] = new_moves[i][0];
        moves[i][1] = new_moves[i][1];
    }

    return j;
}

int isTargeted(int moves[][2], int target){
    for(int i = 0; (moves[i][0] != 0) && (moves[i][1] != 0); i++){
        char pos[2] = {moves[i][0] + 'A' - 1, moves[i][1] + '1' - 1};

        if(getPieceType(pos) == target)
            return 1;
    }

    return 0;
}

int isChecked(int color){
    char pos[2];

    int breaked = 0;
    for(int y = 0; (y < 8) && !breaked; y++)
        for(int x = 0; x < 8; x++)
            if(field[x][y] == PIECES.KING[color]){
                pos[0] = x + 'A';
                pos[1] = y + '1';

                breaked = 1;
                break;
            }

    int moves[64][2] = {0};
    int checked = 0;

    getPawnMoves(pos, color, moves);
    if(isTargeted(moves, PIECES.PAWN[!color]))
        checked = 1;
    else{
        memset(moves, 0, sizeof(moves));

        getRookMoves(pos, color, moves);
        if(isTargeted(moves, PIECES.ROOK[!color]))
            checked = 1;
        else{
            memset(moves, 0, sizeof(moves));

            getKnightMoves(pos, color, moves);
            if(isTargeted(moves, PIECES.KNIGHT[!color]))
                checked = 1;
            else{
                memset(moves, 0, sizeof(moves));

                getBishopMoves(pos, color, moves);
                if(isTargeted(moves, PIECES.BISHOP[!color]))
                    checked = 1;
                else{
                    memset(moves, 0, sizeof(moves));

                    getQueenMoves(pos, color, moves);
                    if(isTargeted(moves, PIECES.QUEEN[!color]))
                        checked = 1;
                    else{
                        memset(moves, 0, sizeof(moves));

                        getKingMoves(pos, color, moves);
                        if(isTargeted(moves, PIECES.KING[!color]))
                            checked = 1;
                    }
                }
            }
        }
    }

    return checked;
}

int isStalemate(int color){
    int temp_moves[64][2] = {0};

    for(int y = 0; y < 8; y++)
        for(int x = 0; x < 8; x++)
            if((field[x][y] > 10) == color){
                char pos[2] = {x + 'A', y + '1'};

                if(getPossibleMoves(pos, temp_moves) > 0)
                    return 0;
            }

    return 1;
}

void enpassantReset(){
    memset(enpassant.from, 0, sizeof(enpassant.from));
    memset(enpassant.to, 0, sizeof(enpassant.to));
    memset(enpassant.target, 0, sizeof(enpassant.target));

    enpassant.isActive = 0;
}

void enpassantCheck(char pos[2], int pos2[2], int color){
    if(enpassant.isActive)
        enpassantReset();

    char to_pos[2] = {pos2[0] + 'A' - 1, pos2[1] + '1' - 1};

    if((getPieceType(to_pos) % 10 == PIECES.PAWN[0]) && (abs(pos[1] - to_pos[1]) == 2)){
        char temp_pos1[2] = {to_pos[0] - 1, to_pos[1]};
        char temp_pos2[2] = {to_pos[0] + 1, to_pos[1]};

        int piece1 = getPieceType(temp_pos1);
        int piece2 = getPieceType(temp_pos2);

        if((piece1 == 0) && (piece2 == 0))
            return;

        if((temp_pos1[0] >= 'A') && (piece1 != 0) && ((piece1 > 10) == !color)){
            enpassant.from[0] = temp_pos1[0];
            enpassant.from[1] = temp_pos1[1];
        }

        if((temp_pos1[0] <= 'H') && (piece2 != 0) && ((piece2 > 10) == !color)){
            enpassant.from[0] = temp_pos2[0];
            enpassant.from[1] = temp_pos2[1];
        }

        enpassant.to[0] = to_pos[0];
        enpassant.to[1] = to_pos[1] - 1 + 2 * color;
        enpassant.target[0] = to_pos[0];
        enpassant.target[1] = to_pos[1];

        enpassant.isActive = 1;
    }
}

void castlingCheck(char pos[2], int pos2[2]){
    char to_pos[2] = {pos2[0] + 'A' - 1, pos2[1] + '1' - 1};

    if(((pos[0] == 'A') || (pos[0] == 'E') || (pos[0] == 'H')) && ((pos[1] == '1') || (pos[1] == '8'))){
            int piece = getPieceType(pos);
            int pieceType = piece % 10;
            int color = piece > 10;

            if((pos[0] == 'A') && ((pos[1] == '1') || (pos[1] == '8')))
                castling[color].isLeftRookMoved = 1;

            if((pos[0] == 'E') && ((pos[1] == '1') || (pos[1] == '8'))){
                if((to_pos[0] == 'G') && ((to_pos[1] == '1') || (to_pos[1] == '8'))){
                    char temp_pos1[2] = {'H', to_pos[1]};
                    int temp_pos2[2] = {'F' - 'A' + 1, to_pos[1] - '1' + 1};

                    play(temp_pos1, temp_pos2);
                    player = (player + 1) % 2;
                }

                if((to_pos[0] == 'C') && ((to_pos[1] == '1') || (to_pos[1] == '8'))){
                    char temp_pos1[2] = {'A', to_pos[1]};
                    int temp_pos2[2] = {'D' - 'A' + 1, to_pos[1] - '1' + 1};

                    play(temp_pos1, temp_pos2);
                    player = (player + 1) % 2;
                }

                castling[color].isKingMoved = 1;

            }

            if((pos[0] == 'H') && ((pos[1] == '1') || (pos[1] == '8')))
                castling[color].isRightRookMoved = 1;
       }
}

void promote(char pos[2], int color, int pieceType){
    field[pos[0] - 'A'][pos[1] - '1'] = pieceType + color * 10;
}

int main()
{
    arrangeFigures();

    while(1){
        system("cls");

        int stalemate = isStalemate(player);
        if(isChecked(player))
            if(stalemate){
                redrawField("Checkmate!");
                printf("\n---------------------------------------------------\n");
                break;
            }else
                redrawField("Check!");
        else
            if(stalemate){
                redrawField("Stalemate!");
                printf("\n---------------------------------------------------\n");
                break;
            }else
                redrawField("");

        int moves[64][2] = {0};

        int n = 0;
        char pos[2];

        printf("\n\nSelect a piece:\n\n");

        while(n == 0){
            scanf("%s", pos);

            n = getPossibleMoves(pos, moves);

            if(n == 0)
                printf("\nIncorrect selection or no possible moves, select another piece:\n\n");
        }

        printf("\nPress\t");

        int i;

        label2:

        for(i = 0; (moves[i][0] != 0) && (moves[i][1] != 0); i++)
            printf("%d: %c%c\t", i + 1, moves[i][0] + 'A' - 1, moves[i][1] + '1' - 1);

        printf("\n\n");

        int move;
        scanf("%d", &move);

        if((move > i) || (move < 1)){
            printf("\nIncorrect selection, press:\t");
            goto label2;
        }

        int elliminated = field[moves[move - 1][0] - 1][moves[move - 1][1] - 1];
        int elliminatedByEnpassant = field[enpassant.target[0] - 'A'][enpassant.target[1] - '1'];
        if((elliminated != 0) || (enpassant.isActive && (elliminatedByEnpassant != 0))){
            char* piece1;
            char* piece2;

            if(enpassant.isActive && (elliminatedByEnpassant != 0)){
                piece1 = &PIECES.FROM_NUM[getPieceType(enpassant.from) % 10][player];
                piece2 = &PIECES.FROM_NUM[getPieceType(enpassant.target) % 10][!player];

                printf("\n\nEllimination by enpassant: ");
            }else{
                piece1 = &PIECES.FROM_NUM[getPieceType(pos) % 10][player];
                piece2 = &PIECES.FROM_NUM[elliminated % 10][!player];

                printf("\n\nEllimination: ");
            }

            printf("%s %s => %c%c %s\n", pos, piece1, moves[move - 1][0] + 'A' - 1, moves[move - 1][1] + '1' - 1 - enpassant.isActive * (elliminatedByEnpassant != 0) * (1 + 2 * player), piece2);
        }

        play(pos, moves[move - 1]);

        castlingCheck(pos, moves[move - 1]);
        enpassantCheck(pos, moves[move - 1], !player);

        char temp_pos[2] = {moves[move - 1][0] + 'A' - 1, moves[move - 1][1] + '1' - 1};
        if((getPieceType(temp_pos) % 10 == 1) && ((temp_pos[1] == '1') || (temp_pos[1] == '8'))){
            printf("\nTo promote pawn press:\t1:%s\t2:%s\t3:%s\t4:%s\n\n", PIECES.FROM_NUM[2][!player], PIECES.FROM_NUM[3][!player], PIECES.FROM_NUM[4][!player], PIECES.FROM_NUM[5]);

            int selected;
            scanf("%d", &selected);

            promote(temp_pos, !player, selected + 1);
        }

        printf("\n---------------------------------------------------\n");

        if(getch() == 'e')
            break;
    }

    return 1;
}
