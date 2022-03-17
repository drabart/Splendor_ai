//
// Created by drabart on 03/01/2022.
//

#include <bits/stdc++.h>
#include "Agent.h"
#include "Board.h"

using namespace std;

#ifndef SPLENDOR_NEAT_SPLENDORGAME_H
#define SPLENDOR_NEAT_SPLENDORGAME_H

class SplendorGame
{
public:
    int player_count{};
    vector<Agent> players;
    vector<pair<int, int> > player_scores;
    Board board;
    bool debug{};
    [[maybe_unused]] bool init;

    SplendorGame()
    {
        init = false;
    }

    [[maybe_unused]] SplendorGame(int player_count, vector<Agent> players)
    {
        this->player_count = player_count;
        this->players = move(players);
        for(int i = 0; i < player_count; ++i)
        {
            player_scores.emplace_back(0, i);
        }

        board = Board(player_count, 7, 5, 5, true);

        // board.print();

        init = true;
    }

    SplendorGame(int player_count, vector<Agent> players, const Board& dummy_board)
    {
        this->player_count = player_count;
        this->players = move(players);
        for(int i = 0; i < player_count; ++i)
        {
            player_scores.emplace_back(0, i);
        }

        // should be faster because it doesn't require loading from file
        board = Board(player_count, 7, 5, 5, false);
        board.nobles_raw = dummy_board.nobles_raw;
        board.tier1 = dummy_board.tier1;
        board.tier2 = dummy_board.tier2;
        board.tier3 = dummy_board.tier3;
        board.load_resources();

        // board.print();

        init = true;
    }

    // return true when buying is possible
    bool buy_card(int player, int tier, int pos)
    {
        PlayerInventory inventory = board.players[player];
        Card card;

        // check if card exists
        if(tier == 1)
        {
            if(board.left_tier1 <= pos)
            {
                return false;
            }
            else
            {
                card = board.tier1[pos];
            }
        }

        if(tier == 2)
        {
            if(board.left_tier2 <= pos)
            {
                return false;
            }
            else
            {
                card = board.tier2[pos];
            }
        }

        if(tier == 3)
        {
            if(board.left_tier3 <= pos)
            {
                return false;
            }
            else
            {
                card = board.tier3[pos];
            }
        }

        // check if player have enough tokens (prioritize normal tokens over gold)
        int tokens_missing = 0;

        for(int i = 0; i < 5; ++i)
        {
            tokens_missing += max(card.cost[i] - inventory.tokens[i] - inventory.resources[i], 0);
        }

        if(inventory.tokens[5] < tokens_missing)
        {
            return false;
        }

        return true;
    }

    bool reserve_card(int player, int tier, int pos)
    {
        PlayerInventory inventory = board.players[player];

        // check if card exists
        if(tier == 1)
        {
            if(board.left_tier1 <= pos)
            {
                return false;
            }
        }

        if(tier == 2)
        {
            if(board.left_tier2 <= pos)
            {
                return false;
            }
        }

        if(tier == 3)
        {
            if(board.left_tier3 <= pos)
            {
                return false;
            }
        }

        int reservation_count = 0;

        for(auto & reservation : inventory.reservations)
        {
            if(reservation.init)
                reservation_count++;
        }

        if(reservation_count >= 3)
            return false;

        return true;
    }

    bool take_111(const int type[5])
    {
        for(int i = 0; i < 5; ++i)
        {
            if(board.tokens[i] < type[i])
                return false;
        }

        /*
        *** do this when doing move (same with take 2) ***

        if(total_tokens > 7)
            inventory.overflow = true;
        */

        return true;
    }

    bool take_2(const int type[5])
    {
        for(int i = 0; i < 5; ++i)
        {
            if(board.tokens[i] < type[i])
                return false;

            if(type[i] && board.tokens[i] < 4)
                return false;
        }

        return true;
    }

    void drop(int player_pos, int overflow, vector<double> out, int it)
    {
        vector<pair<double, int> > to_throw;

        for(int i = 0; i < overflow; ++i)
        {
            to_throw.clear();
            for(int j = 0; j < 5; ++j)
            {
                to_throw.emplace_back(out[it + i * 5 + j], j);
            }
            sort(to_throw.begin(), to_throw.end());
            reverse(to_throw.begin(), to_throw.end());

            for(int j = 0; j < 5; ++j)
            {
                if(board.players[player_pos].tokens[to_throw[j].second] > 0)
                {
                    board.drop(player_pos, to_throw[j].second);
                    break;
                }
            }
        }
    }

    void drop_man(int player_pos, int overflow)
    {

        for(int i = 0; i < overflow; ++i)
        {
            int token;
            while(true)
            {
                printf("Input token to drop (1-5): ");
                scanf("%d", &token);
                if(token < 1 || token > 5)
                    continue;
                if(board.players[player_pos].tokens[token-1] < 1)
                    continue;
                // will only exit if correct token is dropped
                break;
            }

            board.drop(player_pos, token-1);
        }
    }

    bool buy_reserved(int player, int pos)
    {
        PlayerInventory inventory = board.players[player];
        Card card;

        if(!inventory.reservations[pos].init)
            return false;

        card = inventory.reservations[pos];

        // check if player have enough tokens (prioritize normal tokens over gold)
        int tokens_missing = 0;

        for(int i = 0; i < 5; ++i)
        {
            tokens_missing += max(card.cost[i] - inventory.tokens[i] - inventory.resources[i], 0);
        }

        if(inventory.tokens[5] < tokens_missing)
        {
            return false;
        }

        return true;
    }

    // return true if move could have been done
    // return false if move has be picked again
    bool single_move(int player_pos, int it, bool manual = true, vector<double> output = {})
    {
        int overflow;

        /*
        3 * 4 (12)              - buying card
        3 * 5 (15)              - reserving
        3                       - buy reserved
        5 choose 3 (10)         - take 111
        5 choose 1 (5)          - take 2
        5 * 3 (15)          - drop

        60 total
        */

        if(it < 12) // buying cards
        {
            int tier = it / 4 + 1;
            int pos = it % 4;

            if(buy_card(player_pos, tier, pos))
            {
                board.buy_card(player_pos, tier, pos);

                if(!manual)
                    printf("Player %d, BOUGHT card: tier %d, position %d\n", player_pos+1, tier, pos+1);

                return true;
            }

            return false;
        }
        if(it < 12 + 15) // reserving cards
        {
            int j = it - 12;

            int tier = j / 5 + 1;
            int pos = j % 5;

            // printf("%d %d  ", tier, pos);

            // printf("1");
            if(reserve_card(player_pos, tier, pos))
            {
                // printf("inside");
                board.reserve_card(player_pos, tier, pos);

                if(!manual)
                    printf("Player %d, RESERVED card: tier %d, position %d\n", player_pos+1, tier, pos+1);

                return true;
            }

            return false;
        }
        if(it < 12 + 15 + 3) // buying a reserved card
        {
            int j = it - 12 - 15;

            if(buy_reserved(player_pos, j))
            {
                board.buy_reserved(player_pos, j);

                if(!manual)
                    printf("Player %d, BOUGHT RESERVED card: position %d\n", player_pos+1, j+1);

                return true;
            }

            return false;
        }
        if(it < 12 + 15 + 3 + 10) // take 111 tokens
        {
            int j = it - 12 - 15 - 3;

            int tokens[5]{};

            for(auto & token : tokens)
                token = 1;

            if(j < 4)
            {
                tokens[0] = 0;
                tokens[j - 0 + 1] = 0;
            }
            else if(j < 4 + 3)
            {
                tokens[1] = 0;
                tokens[j - 4 + 2] = 0;
            }
            else if(j < 4 + 3 + 2)
            {
                tokens[2] = 0;
                tokens[j - 4 - 3 + 3] = 0;
            }
            else
            {
                tokens[3] = 0;
                tokens[4] = 0;
            }

            if(take_111(tokens))
            {
                overflow = board.take(player_pos, tokens);
                // Board::print_tokens(board.players[player_pos].tokens);
                if(overflow)
                {
                    if(manual)
                        drop_man(player_pos, overflow);
                    else
                        drop(player_pos, overflow, output, 12 + 15 + 3 + 10 + 5);
                }
                if(!manual)
                    printf("Player %d, TOOK tokens: %d%d%d%d%d\n", player_pos+1,
                           tokens[0], tokens[1], tokens[2], tokens[3], tokens[4]);
                // Board::print_tokens(board.players[player_pos].tokens);
                return true;
            }

            return false;
        }
        if(it < 12 + 15 + 3 + 10 + 5) // take 2 tokens
        {
            int j = it - 12 - 15 - 3 - 10;

            int tokens[5]{};
            tokens[j] = 2;

            if(take_2(tokens))
            {
                overflow = board.take(player_pos, tokens);
                // Board::print_tokens(board.players[player_pos].tokens);
                if(overflow)
                {
                    if(manual)
                        drop_man(player_pos, overflow);
                    else
                        drop(player_pos, overflow, output, 12 + 15 + 3 + 10 + 5);
                }
                if(!manual)
                    printf("Player %d, TOOK tokens: %d%d%d%d%d\n", player_pos+1,
                           tokens[0], tokens[1], tokens[2], tokens[3], tokens[4]);
                // Board::print_tokens(board.players[player_pos].tokens);
                return true;
            }

            return false;
        }
        return false;
    }

    void make_move(Agent player, int player_pos, bool print_move = false)
    {
        vector<double> input = board.vectify(player_pos);
        vector<double> output = player.nn.propagate(input);

        // sort for moves remember the index (doing this because it's possible that many 'best moves' will be unavailable)
        vector<pair<double, int> > sorted;
        int out_size = 12 + 15 + 3 + 10 + 5 + 15;
        sorted.reserve(out_size);
        for(int i = 0; i < out_size; ++i)
        {
            sorted.emplace_back(output[i], i);
        }

        // sorting ascending and reversing just because I can
        sort(sorted.begin(), sorted.end());
        reverse(sorted.begin(), sorted.end());

        int overflow;

        for(auto & i : sorted)
        {
            int it = i.second;
            // printf("%d ", it);

            if(single_move(player_pos, it, false, output))
                break;
        }
        // printf("\n");
    }

    vector<int> play_game(int max_move)
    {
        bool win = false;
        int move = 1;

        while(!win)
        {
            for(int i = 0; i < player_count; ++i)
            {
                make_move(players[i], i);
            }

            for(int i = 0; i < player_count; ++i)
            {
                player_scores[i].first = board.players[i].score;
                if(player_scores[i].first >= 15)
                    win = true;
            }

            move++;
            if(move >= max_move)
                break;
        }

        // calculating fitness

        vector<int> results(player_count);
        // bigger score the better (should not be bigger than 2M)
        for(int i = 0; i < player_count; ++i)
        {
            // bigger score the better
            player_scores[i].first = board.players[i].score;
            player_scores[i].first *= 100;
            // fewer cards more flex
            for(int resource : board.players[i].resources)
            {
                player_scores[i].first -= resource;
            }
            player_scores[i].first *= 1000;
            // check not from official guide book but if you have more tokens at the end to flex, then win if yours
            for(int j = 0; j < 5; ++j)
            {
                player_scores[i].first += board.players[i].tokens[j];
            }
        }
        // if now there are ties then F for the player
        sort(player_scores.begin(), player_scores.end());

        if(debug)
        {
            printf("moves: %d; ", move);
            printf("player: %d won\n", player_scores[player_count - 1].second + 1);
        }

        // if game didn't end then that's not a great strategy
        if(move >= max_move)
        {
            for(int i = 0; i < player_count; ++i)
            {
                results[i] += 100000;
            }
        }

        // the worst players are first
        for(int i = 0; i < player_count; ++i)
        {
            // currently, linear progression but changing could be beneficial
            results[player_scores[i].second] += (player_count - i - 1) * 10000;
        }

        // bonus points for better win
        for(int i = 0; i < player_count; ++i)
        {
            results[player_scores[i].second] += 1000 - (player_scores[i].first / 2000);
        }
        return results;
    }
};

#endif //SPLENDOR_NEAT_SPLENDORGAME_H
