//
// Created by drabart on 04.02.2022.
//

#include <bits/stdc++.h>
#include "GeneticAlgorithm.cpp"
#include "Agent.h"
#include "SplendorGame.h"

SplendorGame game;
bool training = true;

// TODO fix manual card input
// TODO add graphics interface
// TODO test some more
void ask_for_move(int player_pos)
{
    bool incorrect_move = true;
    while(incorrect_move)
    {
        printf("Player %d, input your move: ", player_pos + 1);

        char t;
        string l;
        int type[5];
        int a, b, it;

        cin >> t;

        if (t == 't')
        {
            cin >> l;
            // cout << l << "\n";
            int sum = 0;
            int xr = 0;
            for (int i = 0; i < 5; ++i)
            {
                if (l[i] == '2')
                    type[i] = 2;
                else if (l[i] == '1')
                    type[i] = 1;
                else
                    type[i] = 0;
                sum += type[i];
                xr ^= type[i];
            }

            // printf("%d %d\n", sum, xr);

            if(sum > 3 || sum < 2 || (sum == 2 && xr == 0) || (sum == 3 && xr == 3))
                continue;

            if(sum == 2)
            {
                for(int i = 0; i < 5; ++i)
                {
                    if(type[i])
                        it = i;
                }
                it += 12 + 15 + 3 + 10;
            }

            if(sum == 3)
            {
                if(type[0] == 0)
                {
                    for(int i = 1; i < 5; ++i)
                    {
                        if(type[i] == 0)
                            it = i - 1;
                    }
                }
                else if(type[1] == 0)
                {
                    for(int i = 2; i < 5; ++i)
                    {
                        if(type[i] == 0)
                            it = i + 2;
                    }
                }
                else if(type[2] == 0)
                {
                    for(int i = 3; i < 5; ++i)
                    {
                        if(type[i] == 0)
                            it = i + 4;
                    }
                }
                else
                {
                    it = 9;
                }

                it += 12 + 15 + 3;
            }

            // printf("it: %d, good: %d\n", it, game.take_111(type));
        }
        if (t == 'b')
        {
            cin >> l;
            if (l == "r") {
                cin >> a;
                it = a - 1 + 12 + 15;
            } else {
                cin >> a >> b;
                it = (a - 1) * 4 + b - 1;
            }
        }
        if (t == 'r')
        {
            cin >> a >> b;
            it = (a - 1) * 5 + b - 1 + 12;
        }

        incorrect_move = !game.single_move(player_pos, it);
    }
}

int main()
{
    setbuf(stdout, nullptr);

    if(training)
    {
        train();
        return 0;
    }

    int player_count = 4;
    vector<int> sitting = {0, 0, 0, 0}; // 0 - AI player; 1 - human player
    int ai_player_count = 0;
    for(auto a:sitting)
        if(a == 0)
            ai_player_count++;
    vector<Agent> ai_players;

    for(int i = 0; i < ai_player_count; ++i)
    {
        ai_players.emplace_back();
        ai_players.back().load_from_file_line("/home/drabart/Splendor_ai/PastRuns/pla4gen100_0.txt", i);
    }

    game = SplendorGame(player_count, ai_players, true);

    printf("Game loaded!\n");
    printf("\033[48;5;239m");

    bool win = false;
    int move = 1;

    while(!win)
    {
        int it = 0;
        for(int i = 0; i < player_count; ++i)
        {
            if(sitting[i] == 0)
            {
                game.make_move(game.players[it], i, true);
            }
            else
            {
                game.board.print();
                ask_for_move(i);
            }

            // printf("\033[2J\033[1;1H");
        }

        for(int i = 0; i < player_count; ++i)
        {
            game.player_scores[i].first = game.board.players[i].score;
            if(game.player_scores[i].first >= 15)
                win = true;
        }

        move++;
    }

    printf("Final board state:\n");

    game.board.print();

    printf("Game over!\nEnded at move: %d\n", move);

    vector<int> results(player_count);
    // bigger score the better (should not be bigger than 2M)
    for(int i = 0; i < player_count; ++i)
    {
        // bigger score the better
        game.player_scores[i].first = game.board.players[i].score;
        game.player_scores[i].first *= 100;
        // fewer cards more flex
        for(int resource : game.board.players[i].resources)
        {
            game.player_scores[i].first -= resource;
        }
        game.player_scores[i].first *= 1000;
        // check not from official guide book but if you have more tokens at the end to flex, then win if yours
        for(int j = 0; j < 5; ++j)
        {
            game.player_scores[i].first += game.board.players[i].tokens[j];
        }
    }
    // if now there are ties then F for the player
    sort(game.player_scores.begin(), game.player_scores.end());
    reverse(game.player_scores.begin(), game.player_scores.end());

    for(int i = 0; i < player_count; ++i)
    {
        printf("player: %d ", game.player_scores[i].second + 1);
        if(i == 0)
            printf("won ");
        printf("with %d victory points (%d score points)\n",
               game.board.players[game.player_scores[i].second].score, game.player_scores[i].first);
    }

    return 0;
}
