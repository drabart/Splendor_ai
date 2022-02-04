//
// Created by drabart on 04.02.2022.
//

#include <bits/stdc++.h>
#include "GeneticAlgorithm.cpp"
#include "Agent.h"
#include "SplendorGame.h"

bool training = false;

int main()
{
    setbuf(stdout, nullptr);

    if(training)
        train();

    int player_count = 4;
    int ai_player_count = 3;
    vector<Agent> ai_players;
    vector<int> sitting = {1, 0, 0, 0};

    SplendorGame(player_count, );
}
