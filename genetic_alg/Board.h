//
// Created by drabart on 14/01/2022.
//

#include <bits/stdc++.h>
#include "RngGenerator.h"
using namespace std;

#ifndef SPLENDOR_NEAT_BOARD_H
#define SPLENDOR_NEAT_BOARD_H

class Card
{
public:
    int cost[5]{};
    int type[5]{};
    int type_f;
    int score;
    bool init;

    Card()
    {
        type_f = -1;
        for(int i = 0; i < 5; ++i)
        {
            cost[i] = 0;
            type[i] = 0;
        }
        score = 0;
        init = false;
    }

    Card(const int cost[5], int type, int score)
    {
        type_f = type - 1;

        for(int i = 0; i < 5; ++i)
        {
            this->cost[i] = cost[i];
            this->type[i] = 0;
            if(type == i + 1)
            {
                this->type[i] = 1;
            }
        }
        this->score = score;
        init = true;
    }

    // vectified it has size 11
    vector<double> vectify()
    {
        vector<double> result;
        for(int i : cost)
        {
            result.push_back(double(i) / 5.0);
        }
        for(int i : type)
        {
            result.push_back(double(i));
        }
        result.push_back(double(score) / 5.0);
        return result;
    }
};

class PlayerInventory
{
public:
    int tokens[6]{};
    int resources[5]{};
    int score{};
    int overflow{};
    Card reservations[3];
    [[maybe_unused]] bool init;

    void reset()
    {
        for(int & token : tokens)
        {
            token = 0;
        }
        for(int & resource : resources)
        {
            resource = 0;
        }
        score = 0;
        for(auto & reservation : reservations)
        {
            reservation = Card();
        }
    }

    PlayerInventory()
    {
        reset();
        init = true;
    }

    vector<double> vectify()
    {
        vector<double> result;
        for(int & token : tokens)
        {
            result.push_back(double(token) / 7.0);
        }
        for(int & resource : resources)
        {
            result.push_back(double(resource) / 18.0);
        }
        result.push_back(double(score) / 30.0);
        for(auto & reservation : reservations)
        {
            vector<double> v = reservation.vectify();
            for(auto a : v)
            {
                result.push_back(a);
            }
        }
        return result;
    }
};

class Noble
{
public:
    int cost[5]{};
    int score;
    bool init;

    Noble()
    {
        for(int & i : cost)
        {
            i = 0;
        }
        score = 0;
        init = false;
    }

    explicit Noble(const int cost[5])
    {
        for(int i = 0; i < 5; ++i)
        {
            this->cost[i] = cost[i];
        }
        score = 3;
        init = true;
    }

    vector<double> vectify()
    {
        vector<double> result;
        for(int & i : cost)
        {
            result.push_back(double(i) / 4.0);
        }
        result.push_back(double(score) / 3.0);
        return result;
    }
};

class Board
{
public:
    int player_count{};
    int default_tokens{};
    int gold_tokens{};
    int nobles_number{};
    vector<PlayerInventory> players;
    int tokens[6]{};
    vector<Card> tier1;
    vector<Card> tier2;
    vector<Card> tier3;
    int left_tier1{};
    int left_tier2{};
    int left_tier3{};
    int colors[6] = {2, 15, 4, 52, 9, 3};
    vector<Noble> nobles_raw;
    vector<Noble> nobles;
    string print_lines[6];
    RngGenerator rng;
    bool manual_mode;
    [[maybe_unused]] bool init;

    Board()
    {
        init = false;
    }

    Board(int player_count, int default_tokens, int gold_tokens, int nobles_number, bool auto_load, bool manual_cards = false)
    {
        this->player_count = player_count;
        this->default_tokens = default_tokens;
        this->gold_tokens = gold_tokens;
        this->nobles_number = nobles_number;
        manual_mode = manual_cards;

        for(int i = 0; i < 5; ++i)
        {
            tokens[i] = default_tokens;
        }
        tokens[5] = gold_tokens;

        for(int i = 0; i < player_count; ++i)
        {
            players.emplace_back();
        }

        init = false;

        if(auto_load)
        {
            load_resources();
        }
    }

    void load_resources()
    {
        if(!manual_mode)
        {
            if(nobles_raw.empty())
            {
                load_nobles();
            }
            if(tier1.empty())
            {
                load_cards();
            }

            pick_nobles();

            shuffle_cards();
            left_tier1 = int(tier1.size());
            left_tier2 = int(tier2.size());
            left_tier3 = int(tier3.size());
        }
        else
        {
            pick_nobles();

            for(int i = 0; i < 4; ++i)
                tier1.push_back(ask_for_card());
            for(int i = 0; i < 4; ++i)
                tier2.push_back(ask_for_card());
            for(int i = 0; i < 4; ++i)
                tier3.push_back(ask_for_card());
            left_tier1 = 40;
            left_tier2 = 30;
            left_tier3 = 20;
        }

        init = true;
    }

    Card ask_for_card()
    {
        int cost[5];
        int score;
        int resource;
        char sep;
        cout << "Input card data (score, resource, cost): ";
        cin >> score >> sep >> resource >> sep >> cost[0] >> sep >> cost[1] >> sep >> cost[2] >> sep >> cost[3] >> sep >> cost[4];
        Card card(cost, resource, score);
        return card;
    }

    void shuffle_cards()
    {
        shuffle(tier1.begin(), tier1.end(), rng.rng);
        shuffle(tier2.begin(), tier2.end(), rng.rng);
        shuffle(tier3.begin(), tier3.end(), rng.rng);
    }

    void pick_nobles()
    {
        if(!manual_mode)
        {
            shuffle(nobles_raw.begin(), nobles_raw.end(), rng.rng);

            nobles.clear();
            for(int i = 0; i < nobles_number; ++i)
            {
                nobles.push_back(nobles_raw[i]);
            }
        }
        else
        {
            int cost[5];
            nobles.clear();
            for(int i = 0; i < nobles_number; ++i)
            {
                cout << "Input " << i+1 << " noble: ";
                for(int j = 0; j < 5; ++j)
                    cin >> cost[j];

                Noble noble(cost);
                nobles.push_back(noble);
            }
        }
    }

    void load_cards()
    {
        tier1.clear();
        tier2.clear();
        tier3.clear();

        fstream cards_file;
        cards_file.open("cards.csv", fstream::in);
        string line;
        int tier;
        int cost[5];
        int type;
        int score;

        if(cards_file.is_open())
        {
            while(getline(cards_file, line))
            {
                if(line[0] < 48 || line[0] > 57)
                {
                    continue;
                }

                int it = 0;
                for(char c : line)
                {
                    if(c == ',')
                        continue;

                    if(it == 0)
                        tier = c - 48;

                    if(it == 1)
                        score = c - 48;

                    if(it == 2)
                        type = c - 48;

                    if(it >= 3)
                        cost[it-3] = c - 48;
                    it++;
                }

                Card card(cost, type, score);
                if(tier == 1)
                    tier1.push_back(card);
                else if(tier == 2)
                    tier2.push_back(card);
                else
                    tier3.push_back(card);
            }
            cards_file.close();
        }
        else printf("No cards.csv file");
    }

    void load_nobles()
    {
        nobles_raw.clear();

        fstream nobles_file;
        nobles_file.open("nobles.csv", fstream::in);
        string line;
        int cost[5];

        if(nobles_file.is_open())
        {
            while(getline(nobles_file, line))
            {
                if(line[0] < 48 || line[0] > 57)
                {
                    continue;
                }

                int it = 0;
                for(char c : line)
                {
                    if(c == ',')
                        continue;

                    cost[it] = c - 48;
                    it++;
                }

                Noble noble(cost);
                nobles_raw.push_back(noble);
            }
            nobles_file.close();
        }
        else printf("No nobles.csv file");
    }

    vector<double> vectify(int player_pos)
    {
        vector<double> result;

        // max 6 players
        result.push_back(double(player_count) / 6.0);

        result.push_back(double(default_tokens) / 10.0);

        result.push_back(double(gold_tokens) / 10.0);

        for(int token : tokens)
        {
            result.push_back(double(token) / 10.0);
        }

        // dividing players so that they are in the same order regardless of position
        for(int i = player_pos; i < player_count; ++i)
        {
            vector<double> v = players[i].vectify();
            for(auto a : v)
            {
                result.push_back(a);
            }
        }

        for(int i = 0; i < player_pos; ++i)
        {
            vector<double> v = players[i].vectify();
            for(auto a : v)
            {
                result.push_back(a);
            }
        }

        for(int i = 0; i < 4; ++i)
        {
            vector<double> v;
            if(i < left_tier1)
                v = tier1[i].vectify();
            else
                v.resize(11, 0);
            for(auto a : v)
                result.push_back(a);
        }

        for(int i = 0; i < 4; ++i)
        {
            vector<double> v;
            if(i < left_tier2)
                v = tier2[i].vectify();
            else
                v.resize(11, 0);
            for(auto a : v)
                result.push_back(a);
        }

        for(int i = 0; i < 4; ++i)
        {
            vector<double> v;
            if(i < left_tier3)
                v = tier3[i].vectify();
            else
                v.resize(11, 0);
            for(auto a : v)
                result.push_back(a);
        }

        for(auto & noble : nobles)
        {
            vector<double> v = noble.vectify();
            for(auto a : v)
            {
                result.push_back(a);
            }
        }

        return result;
    }

    void noble_check(int player_pos)
    {
        for(int i = 0; i < nobles_number; ++i)
        {
            if(!nobles[i].init)
                continue;
            bool have_req = true;
            for(int j = 0; j < 5; ++j)
            {
                if(nobles[i].cost[j] > players[player_pos].resources[j])
                {
                    have_req = false;
                    break;
                }
            }
            if(have_req)
            {
                players[player_pos].score += nobles[i].score;
                nobles[i] = Noble();
            }
        }
    }

    void remove_card(int tier, int pos)
    {
        Card card;

        if(tier == 1)
        {
            card = tier1[pos];

            if(manual_mode)
            {
                tier1.push_back(card);
                tier1[pos] = ask_for_card();
            }
            else
            {
                tier1.erase(tier1.begin() + pos);
                tier1.push_back(card);
            }

            left_tier1--;
        }

        if(tier == 2)
        {
            card = tier2[pos];

            if(manual_mode)
            {
                tier2.push_back(card);
                tier2[pos] = ask_for_card();
            }
            else
            {
                tier2.erase(tier2.begin() + pos);
                tier2.push_back(card);
            }

            left_tier2--;
        }

        if(tier == 3)
        {
            card = tier3[pos];

            if(manual_mode)
            {
                tier3.push_back(card);
                tier3[pos] = ask_for_card();
            }
            else
            {
                tier3.erase(tier3.begin() + pos);
                tier3.push_back(card);
            }

            left_tier3--;
        }
    }

    void buy_card(int player_pos, int tier, int pos)
    {
        // players[player_pos] - inventory
        Card card;

        if(tier == 1)
        {
            card = tier1[pos];
        }

        if(tier == 2)
        {
            card = tier2[pos];
        }

        if(tier == 3)
        {
            card = tier3[pos];
        }

        int tokens_missing = 0;

        for(int i = 0; i < 5; ++i)
        {
            tokens_missing += max(card.cost[i] - players[player_pos].tokens[i] - players[player_pos].resources[i], 0);
            tokens[i] += max(min(card.cost[i] - players[player_pos].resources[i], players[player_pos].tokens[i]), 0);
            players[player_pos].tokens[i] -= max(min(card.cost[i] - players[player_pos].resources[i], players[player_pos].tokens[i]), 0);
        }

        players[player_pos].tokens[5] -= tokens_missing;
        tokens[5] += tokens_missing;

        players[player_pos].score += card.score;

        players[player_pos].resources[card.type_f]++;

        noble_check(player_pos);

        remove_card(tier, pos);
    }

    void reserve_card(int player_pos, int tier, int pos)
    {
        // players[player_pos] - inventory
        Card card;

        if(pos == 4 && manual_mode)
        {
            card = ask_for_card();
            if(tier == 1)
            {
                tier1.push_back(card);
                left_tier1--;
            }
            if(tier == 2)
            {
                tier2.push_back(card);
                left_tier2--;
            }
            if(tier == 3)
            {
                tier3.push_back(card);
                left_tier3--;
            }
        }
        else
        {
            if(tier == 1)
            {
                card = tier1[pos];
            }

            if(tier == 2)
            {
                card = tier2[pos];
            }

            if(tier == 3)
            {
                card = tier3[pos];
            }
        }

        if(tokens[5])
        {
            players[player_pos].tokens[5]++;
            tokens[5]--;
        }

        for(auto & reservation : players[player_pos].reservations)
        {
            if(!reservation.init)
            {
                reservation = card;
                break;
            }
        }

        if(!(pos == 4 && manual_mode))
            remove_card(tier, pos);
    }

    void buy_reserved(int player_pos, int pos)
    {
        // players[player_pos] - inventory
        Card card;
        card = players[player_pos].reservations[pos];

        int tokens_missing = 0;

        for(int i = 0; i < 5; ++i)
        {
            tokens_missing += max(card.cost[i] - players[player_pos].tokens[i] - players[player_pos].resources[i], 0);
            tokens[i] += max(min(card.cost[i] - players[player_pos].resources[i], players[player_pos].tokens[i]), 0);
            players[player_pos].tokens[i] -= max(min(card.cost[i] - players[player_pos].resources[i], players[player_pos].tokens[i]), 0);
        }

        players[player_pos].tokens[5] -= tokens_missing;
        tokens[5] += tokens_missing;

        players[player_pos].score += card.score;

        players[player_pos].resources[card.type_f]++;
        players[player_pos].reservations[pos] = Card();

        noble_check(player_pos);
    }

    // returns true if player need to get rid of excess tokens
    int take(int player_pos, const int taken_tokens[5])
    {
        int total = 0;

        for(int i = 0; i < 5; ++i)
        {
            players[player_pos].tokens[i] += taken_tokens[i];
            tokens[i] -= taken_tokens[i];
            total += players[player_pos].tokens[i];
        }

        players[player_pos].overflow = 0;

        if(total > 10)
            players[player_pos].overflow = total - 10;

        return players[player_pos].overflow;
    }

    void drop(int player_pos, int token_type)
    {
        players[player_pos].tokens[token_type]--;
        tokens[token_type]++;
    }

    static void print_tokens(int printed_tokens[5])
    {
        for(int i = 0; i < printed_tokens[0]; ++i)
        {
            printf("\033[38;5;2m0");
        }
        if(printed_tokens[0])
            printf("\n");
        for(int i = 0; i < printed_tokens[1]; ++i)
        {
            printf("\033[38;5;15m0");
        }
        if(printed_tokens[1])
            printf("\n");
        for(int i = 0; i < printed_tokens[2]; ++i)
        {
            printf("\033[38;5;4m0");
        }
        if(printed_tokens[2])
            printf("\n");
        for(int i = 0; i < printed_tokens[3]; ++i)
        {
            printf("\033[38;5;52m0");
        }
        if(printed_tokens[3])
            printf("\n");
        for(int i = 0; i < printed_tokens[4]; ++i)
        {
            printf("\033[38;5;9m0");
        }
        if(printed_tokens[4])
            printf("\n");
        printf("\n");
        printf("\033[38;5;7m");
    }

    static void print_tokens_g(int printed_tokens[6])
    {
        //printf("Green: %d; White: %d; Blue: %d; Black: %d; Red: %d; Gold %d\n", printed_tokens[0],
        //       printed_tokens[1], printed_tokens[2], printed_tokens[3], printed_tokens[4], printed_tokens[5]);

        for(int i = 0; i < printed_tokens[0]; ++i)
        {
            printf("\033[38;5;2m0");
        }
        if(printed_tokens[0])
            printf("\n");
        for(int i = 0; i < printed_tokens[1]; ++i)
        {
            printf("\033[38;5;15m0");
        }
        if(printed_tokens[1])
            printf("\n");
        for(int i = 0; i < printed_tokens[2]; ++i)
        {
            printf("\033[38;5;4m0");
        }
        if(printed_tokens[2])
            printf("\n");
        for(int i = 0; i < printed_tokens[3]; ++i)
        {
            printf("\033[38;5;52m0");
        }
        if(printed_tokens[3])
            printf("\n");
        for(int i = 0; i < printed_tokens[4]; ++i)
        {
            printf("\033[38;5;9m0");
        }
        if(printed_tokens[4])
            printf("\n");
        for(int i = 0; i < printed_tokens[5]; ++i)
        {
            printf("\033[38;5;3m0");
        }
        if(printed_tokens[5])
            printf("\n");
        printf("\n");
        printf("\033[38;5;7m");
    }

    void add_to_print(int printed_tokens[5], int height, int width)
    {
        int line = 1;
        for(int j = 0; j < 5; ++j)
        {
            if(printed_tokens[j])
            {
                print_lines[line] += "\033[38;5;" + to_string(colors[j]) + "m";
                for(int k = 0; k < printed_tokens[j]; ++k)
                    print_lines[line] += "0";
                for(int k = printed_tokens[j]; k < width; ++k)
                    print_lines[line] += " ";
                print_lines[line] += "\033[38;5;7m";
                line++;
            }
        }
        for(int j = line; j < height; ++j)
        {
            for(int k = 0; k < width; ++k)
                print_lines[j] += " ";
        }
    }

    [[maybe_unused]] void print()
    {
        /*
         * /033[38;5;2m - green
         * /033[38;5;15m - white
         * /033[38;5;4m - blue
         * /033[38;5;52m - brown
         * /033[38;5;9m - red
         * /033[38;5;3m - gold
         * /033[0m - reset
         */
        
        printf("Tokens:\n");
        print_tokens_g(tokens);

        printf("Cards tier 1 (%d left):\n", left_tier1);
        for(int i = 0; i < min(4, left_tier1); ++i)
        {
            print_lines[0] += "Score: " + to_string(tier1[i].score) + ", Type: " + to_string(tier1[i].type_f);
            while(print_lines[0].length() % 25)
                print_lines[0] += " ";
            add_to_print(tier1[i].cost, 5, 25);
        }
        for(int i = 0; i < 5; ++i)
        {
            cout << print_lines[i] << "\n";
            print_lines[i] = "";
        }
        cout << "\n";

        printf("Cards tier 2 (%d left):\n", left_tier2);
        for(int i = 0; i < min(4, left_tier2); ++i)
        {
            print_lines[0] += "Score: " + to_string(tier2[i].score) + ", Type: " + to_string(tier2[i].type_f);
            while(print_lines[0].length() % 25)
                print_lines[0] += " ";
            add_to_print(tier2[i].cost, 4, 25);
        }
        for(int i = 0; i < 4; ++i)
        {
            cout << print_lines[i] << "\n";
            print_lines[i] = "";
        }
        cout << "\n";

        printf("Cards tier 3 (%d left):\n", left_tier3);
        for(int i = 0; i < min(4, left_tier3); ++i)
        {
            print_lines[0] += "Score: " + to_string(tier3[i].score) + ", Type: " + to_string(tier3[i].type_f);
            while(print_lines[0].length() % 25)
                print_lines[0] += " ";
            add_to_print(tier3[i].cost, 5, 25);
        }
        for(int i = 0; i < 5; ++i)
        {
            cout << print_lines[i] << "\n";
            print_lines[i] = "";
        }
        cout << "\n";

        printf("Nobles:\n");
        for(auto & noble : nobles)
        {
            add_to_print(noble.cost, 4, 10);
        }
        for(int i = 1; i < 4; ++i)
        {
            cout << print_lines[i] << "\n";
            print_lines[i] = "";
        }
        printf("\n");

        int card_tokens[5];
        for(int i = 0; i < player_count; ++i)
        {
            printf("Player %d:\n", i+1);
            printf("Score: %d\n", players[i].score);
            printf("Tokens:\n");
            print_tokens_g(players[i].tokens);
            printf("Resources:\n");
            for(int j = 0; j < 5; ++j)
            {
                card_tokens[j] = players[i].resources[j];
            }
            print_tokens(card_tokens);
            printf("Reservations:\n");
            for(auto & reservation : players[i].reservations)
            {
                if(!reservation.init) continue;
                
                print_lines[0] += "Score: " + to_string(reservation.score) + ", Type: " + to_string(reservation.type_f);
                while(print_lines[0].length() % 25)
                    print_lines[0] += " ";
                add_to_print(reservation.cost, 5, 25);
            }
            for(int i = 0; i < 5; ++i)
            {
                cout << print_lines[i] << "\n";
                print_lines[i] = "";
            }
            cout << "\n";
        }
        printf("\n--------------------------------------------------------------------------------------\n\n");
    }
};

#endif //SPLENDOR_NEAT_BOARD_H
