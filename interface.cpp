//
// Created by drabart on 04.02.2022.
//

#include <bits/stdc++.h>
#include "GeneticAlgorithm.cpp"
#include "Agent.h"
#include "SplendorGame.h"

bool training = false;

// TODO finish this
int ask_for_move(int player_pos)
{
    char s[256];
    int sl;
    printf("Player %d, input your move: ", player_pos+1);
    scanf("%s", s);
    sl = int(strlen(s));

    stringstream ss;
    ss << s;

    char t;
    string l;
    int type[5];
    int a, b, c;

    ss >> t;

    if(t == 't')
    {
        ss >> l;
        for(int i = 0; i < 5; ++i)
        {
            if(l[i] == '1')
                type[i] = 1;
            else
                type[i] = 0;
        }

    }
    if(t == 'b')
    {
        ss >> l;
        if(l == "r")
        {

        }
        else
        {
            ss >> l;
            for(int i = 0; i < 5; ++i)
            {
                if(l[i] == '1')
                    type[i] = 1;
                else
                    type[i] = 0;
            }
        }
    }
    if(t == 'r')
    {
        ss >> a;
    }

    return 0;
}

int main()
{
    setbuf(stdout, nullptr);

    if(training)
        train();

    int player_count = 4;
    int ai_player_count = 3;
    vector<Agent> ai_players;
    ai_players.reserve(ai_player_count);
    vector<int> sitting = {1, 0, 0, 0}; // 0 - AI player; 1 - human player

    for(int i = 0; i < ai_player_count; ++i)
    {
        ai_players.emplace_back();
        ai_players.back().load_from_file_line("saved_networks.txt", i);
    }

    SplendorGame game = SplendorGame(player_count, ai_players);

    bool win = false;
    int move = 1;

    while(!win)
    {
        int it = 0;
        for(int i = 0; i < player_count; ++i)
        {
            if(sitting[i] == 0)
                game.make_move(game.players[it], i);
            else
                game.make_move(i, ask_for_move(i));

            printf("\033[2J\033[1;1H");
        }

        for(int i = 0; i < player_count; ++i)
        {
            game.player_scores[i].first = game.board.players[i].score;
            if(game.player_scores[i].first >= 15)
                win = true;
        }

        move++;
    }
}
