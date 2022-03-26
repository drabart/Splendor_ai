//
// Created by drabart on 24/03/2022.
//

#include <bits/stdc++.h>
#include <thread>
#include <utility>
#include "NeuralNetwork.h"
#include "SplendorGame.h"
#include "Agent.h"
#include "Board.h"

class GeneticAlgorithm
{
public:
    // data on objects
    Board test_board;
    vector<LayerBase> layers;

    // shared data for threads
    int unfinished{};

    // population data
    int population_size;
    int rounds;
    int players;
    int generation;
    int max_generation{};
    vector<Agent> population;

    // handling comparison between generations
    int past_gen_trainees;
    int saved_agents;
    int test_rounds;
    [[maybe_unused]] string saved_networks;
    vector<vector<Agent> > past_generations;

    // mutations
    bool mutation_based;
    double mutation_rate;
    double mutation_range;

    // output
    bool debug;
    bool print_fitness;

    // rng
    RngGenerator rng;

    GeneticAlgorithm(int players, int population_size, int rounds,
                     double mutation_rate, double mutation_range,
                     vector<LayerBase> layers_l,
                     bool rand_start, bool print_fitness, bool debug,
                     int past_gen_help, int population_champions, int testRounds)
    {
        generation = 0;
        this->debug = debug;
        this->print_fitness = print_fitness;

        mutation_based = !rand_start;
        this->mutation_rate = mutation_rate;
        this->mutation_range = mutation_range;

        saved_networks = "saved_networks.txt";
        saved_agents = population_champions;
        past_gen_trainees = past_gen_help;
        test_rounds = testRounds;

        this->players = players;
        // making population divisible by players number;
        if(population_size % players)
            population_size += (players - (population_size % players));
        this->population_size = population_size;
        this->rounds = rounds;

        layers = std::move(layers_l);

        test_board = Board(players, 7, 5, 5, true);
        layers[0].nodes = int(test_board.vectify(0).size());

        for(int i = 0; i < this->population_size; ++i)
        {
            population.emplace_back(layers, mutation_based);
            // population.emplace_back(saved_networks, -1, 937707587);
        }
    }

    double static float_mod(double x, int mod)
    {
        return x - int(x / mod) * mod;
    }

    void proceed_one_generation(int threshold, int max_move, const string& crossover_type)
    {
        // print separator and current generation number
        printf("----------------------------------------------------------------------------\n");
        printf("Generation: %d\n", generation+1);

        // start measuring total generation time
        chrono::time_point<chrono::system_clock> t_start, t_end;
        t_start = chrono::system_clock::now();

        // start measuring time for playing phase
        chrono::time_point<chrono::system_clock> start, end;
        start = chrono::system_clock::now();

        // play games across population to determine best players
        play_game(max_move);

        // finish measuring time for playing phase
        end = chrono::system_clock::now();
        chrono::duration<double> elapsed_seconds = end - start;

        // print elapsed time
        printf("Playing done, took: %lf s\n", elapsed_seconds.count());

        // sort by fitness
        sort(population.begin(), population.end(),
             [](Agent const &a1, Agent const &a2) { return a1.fitness < a2.fitness; });

        // save best network to file
        // population[0].save_to_file(saved_networks);

        // pick 'saved_agents' best Agents for use in future generations
        vector<Agent> champions(saved_agents);
        for(int i = 0; i < saved_agents; ++i)
        {
            // sets the gen variable to differentiate during comparison
            population[i].gen = generation;
            // append to buffer
            champions[i] = population[i];
        }
        // add buffer to the main object holding past Agents
        past_generations.push_back(champions);

        // if fitness printing enabled print top 10 best performers
        if(print_fitness)
        {
            // print only top ten best
            for (int i = 0; i < min(population_size, 10); ++i) {
                printf("uid: %d, fitness: %d\n", population[i].uid, population[i].fitness);
            }
        }

        // sum all fitnesses
        int sum = 0;
        for (int i = 0; i < population_size; ++i) {
            sum += population[i].fitness;
        }

        // print some data about current generation
        printf("\nUnfinished games: %d\n", unfinished);
        printf("Average fitness across population: %lf\n", double(sum) / double(population_size));

        // start measuring for crossover and mutation phase
        start = chrono::system_clock::now();

        // pick crossover from argument
        // every crossover leaves of 'threshold' best performing Agents to the next generation
        // every crossover picks randomly 'past_gen_trainee' Agents from past generations
        // 'old Agents' used later refers to the Agents not mentioned above
        // this crossover creates new random Agents in place of old Agents
        if(crossover_type == "cut_fill")
        {
            crossover_cut_fill(threshold);
        }
        // replaces old Agents with new ones created with prefix of one parent genes and suffix of second parent
        else if(crossover_type == "divide_parents")
        {
            crossover_divide_parents(threshold);
        }
        /*
        else if(crossover_type == "random_from_parent")
        {

        }
        else if(crossover_type == "average_from_parent")
        {

        }
        */
        // mutate population
        mutate();

        // end measuring for crossover and mutation phase
        end = chrono::system_clock::now();
        elapsed_seconds = end - start;

        // print elapsed time
        printf("Crossover and mutation took: %lf s\n", elapsed_seconds.count());

        // finish counting time
        t_end = chrono::system_clock::now();
        elapsed_seconds = t_end - t_start;
        double expected_seconds = elapsed_seconds.count() * (max_generation - generation - 1);
        printf("Generation took in total: %lf s, Expected time to finish: %d min %.2lf s\n", elapsed_seconds.count(),
               int(expected_seconds / 60), float_mod(expected_seconds, 60));

        // increment generation
        generation++;
    }

    static void single_game(int player_number, vector<Agent> current_players, int max_move,
                            vector<int>& batch_results, int& finished, const Board& test_board, const bool& debug)
    {
        SplendorGame game = SplendorGame(player_number, std::move(current_players), test_board);
        game.debug = debug;
        game.train = true;

        batch_results = game.play_game(max_move);

        finished = 1;
        if(batch_results[0] > 100000)
        {
            finished = 0;
        }
    }

    // calculates and sets fitness values, and returns number of unfinished games
    void play_games(vector<Agent> &temp_population, int game_rounds, int max_move, bool print_round_completion = false)
    {
        unfinished = 0;

        for(auto & agent : temp_population)
            agent.fitness = 0;

        int games_in_round = int(temp_population.size()) / players;
        vector<thread> threads(games_in_round);

        for(int i = 0; i < game_rounds; ++i)
        {
            shuffle(temp_population.begin(), temp_population.end(), rng.rng);
            vector<Agent> players_batch;
            vector<vector<int> > player_batches(games_in_round, vector<int>(players));
            vector<int> finished(games_in_round);
            for(int j = 0; j < games_in_round; ++j)
            {
                players_batch.clear();
                for(int k = 0; k < players; ++k)
                {
                    players_batch.push_back(temp_population[j * players + k]);
                }

                threads[j] = thread(single_game, players, players_batch, max_move, ref(player_batches[j]),
                                    ref(finished[j]), ref(test_board), ref(debug));
            }

            for(int j = 0; j < games_in_round; ++j)
            {
                // all threads need to arrive, and I believe it's not worth optimizing
                // this further as the latter loops take little time
                threads[j].join();

                unfinished += (1 - finished[j]);
                for(int k = 0; k < players; ++k)
                    temp_population[j * players + k].fitness += player_batches[j][k];
            }

            if(print_round_completion)
                printf("Round %d completed\n", i+1);
        }
    }

    void play_game(int max_move) // result may change depending on the shuffle of the deck
    {
        play_games(population, rounds, max_move);
    }

    [[maybe_unused]] void compare_generations(int gen1, int gen2, int max_move)
    {
        gen1--;
        gen2--;

        // start measuring time for playing phase
        chrono::time_point<chrono::system_clock> start, end;
        start = chrono::system_clock::now();

        vector<Agent>temporary_population(2 * saved_agents);

        for(int i = 0; i < saved_agents; ++i)
        {
            temporary_population[i*2] = past_generations[gen1][i];
            temporary_population[i*2+1] = past_generations[gen2][i];
            temporary_population[i*2].fitness = 0;
            temporary_population[i*2+1].fitness = 0;
        }

        // printf("%d %d", saved_agents, temporary_population.size());
        play_games(temporary_population, test_rounds, max_move);

        // finish measuring time for playing phase
        end = chrono::system_clock::now();
        chrono::duration<double> elapsed_seconds = end - start;

        // print elapsed time
        printf("\nComparing gen %d and gen %d done, took: %lf s\n", gen1+1, gen2+1, elapsed_seconds.count());

        // sort by fitness
        sort(temporary_population.begin(), temporary_population.end(),
             [](Agent const &a1, Agent const &a2) { return a1.fitness < a2.fitness; });

        printf("Unfinished games: %d\n", unfinished);

        // printing best performers
        if(print_fitness)
        {
            // print only top ten best
            for (int i = 0; i < min(temporary_population.size(), 10ul); ++i) {
                printf("gen: %d, uid: %d, fitness: %d\n", temporary_population[i].gen + 1,
                       temporary_population[i].uid, temporary_population[i].fitness);
            }
        }

        int sum_over_gen1 = 0;
        int sum_over_gen2 = 0;

        for(auto & i : temporary_population)
        {
            if(i.gen == gen1)
                sum_over_gen1 += i.fitness;
            else
                sum_over_gen2 += i.fitness;
        }

        printf("gen %d fitness avg: %lf\ngen %d fitness avg: %lf\n", gen1+1,
               double(sum_over_gen1) / saved_agents, gen2+1, double(sum_over_gen2) / saved_agents);

        if(sum_over_gen1 > sum_over_gen2)
            printf("gen %d lost to gen %d\n\n", gen1+1, gen2+1);
        else
            printf("gen %d lost to gen %d\n\n", gen2+1, gen1+1);
    }

    // save = 0 don't save
    // save = 1 append to file
    // save = 2 truncate file and save
    [[maybe_unused]] void compare_all(int from_each, int max_move, int save)
    {
        printf("\n\nComparing past generations\n");

        vector<Agent> temp_population;

        for(int i = 0; i < generation; ++i)
        {
            for(int j = 0; j < from_each; ++j)
            {
                temp_population.push_back(past_generations[i][j]);
            }
        }

        play_games(temp_population, test_rounds, max_move, true);

        // sort by fitness
        sort(temp_population.begin(), temp_population.end(),
             [](Agent const &a1, Agent const &a2) { return a1.fitness < a2.fitness; });

        printf("Unfinished games: %d\n", unfinished);

        // if so truncate file
        if(save == 2)
        {
            fstream save_file("saved_networks.txt", fstream::out | fstream::trunc);
            save_file.close();
        }

        // printing best performers
        if(print_fitness)
        {
            // print only top ten best
            for (int i = 0; i < min(temp_population.size(), 10ul); ++i) {
                printf("gen: %d, uid: %d, fitness: %d\n", temp_population[i].gen + 1,
                       temp_population[i].uid, temp_population[i].fitness);

                if(save)
                    temp_population[i].save_to_file(saved_networks);
            }
        }

        vector<pair<int, int> > sums(generation);

        for(int i = 0; i < generation; ++i)
            sums[i].second = i;

        for(auto & agent : temp_population)
            sums[agent.gen].first += agent.fitness;

        sort(sums.begin(), sums.end());

        printf("Best generations:\n");
        for(int i = 0; i < min(generation, 10); ++i)
            printf("generation: %d, total fitness: %d\n", sums[i].second + 1, sums[i].first);
    }

    // replace worst performing agents with new random ones
    // leave (threshold) best ones
    void crossover_cut_fill(int threshold)
    {
        sort(population.begin(), population.end(),
             [](Agent const &a1, Agent const &a2) { return a1.fitness < a2.fitness; });

        for(int i = threshold; i < population_size; ++i)
        {
            population[i].new_network();
        }
    }

    // how many agents pass without selection
    void crossover_divide_parents(int threshold)
    {
        // population is already sorted by fitness from best to worst
        vector<Agent> new_population = population;

        // leaves off threshold of the best performers
        // if there have been
        for(int i = threshold; i < population_size - (generation > 0 ? past_gen_trainees : 0); i++)
        {
            // pick 2 parents with gaussian distribution
            int parent_id1 = rng.nextNormalInt(0, population_size - 1);
            int parent_id2 = rng.nextNormalInt(0, population_size - 1);
            // copy parents for ease of use later
            Agent parent1 = population[parent_id1];
            Agent parent2 = population[parent_id2];
            // getting numerical values from parents
            vector<double> genes1 = parent1.nn.vectify();
            vector<double> genes2 = parent2.nn.vectify();

            // counting number of weights and biases
            int weights_count = int(genes1[genes1.size()-2]);
            int biases_count = int(genes1[genes1.size()-1]);

            // vectors for new weights and biases
            vector<double> new_weights(weights_count);
            vector<double> new_biases(biases_count);

            // choosing place to split parents with uniform distribution
            int cutoff_w = rng.nextInt(1, weights_count-2);
            int cutoff_b = rng.nextInt(1, biases_count-2);
            // int cutoff_b = 0;

            for(int j = 0; j < cutoff_w; ++j)
                new_weights[j] = genes1[j];

            for(int j = cutoff_w; j < weights_count; ++j)
                new_weights[j] = genes2[j];

            for(int j = 0; j < cutoff_b; ++j)
                new_weights[j] = genes1[j + weights_count];

            for(int j = cutoff_b; j < biases_count; ++j)
                new_weights[j] = genes2[j + weights_count];

            new_population[i].new_network(new_weights, new_biases);
        }

        if(generation == 0)
        {
            population = new_population;
            return;
        }

        for(int i = 0; i < past_gen_trainees; ++i)
        {
            // pick generation from past (uniform distribution)
            int picked_gen = rng.nextInt(0, generation);
            // pick individual from there (gaussian distribution)
            int picked_individual = rng.nextNormalInt(0, int(past_generations[picked_gen].size()) - 1);
            // set the new Agent to one from past gen
            new_population[population_size -  past_gen_trainees + i] = past_generations[picked_gen][picked_individual];
        }

        population = new_population;
    }

    void mutate()
    {
        for(int i = 0; i < population_size; ++i)
        {
            vector<double> to_mutate = population[i].nn.vectify();
            for(double & gene : to_mutate)
            {
                if(rng.nextDouble(0.0, 1.0) < mutation_rate)
                {
                    gene += gene * rng.nextDouble(-mutation_range, mutation_range);
                }
                if(rng.nextDouble(0.0, 1.0) < mutation_rate / 20)
                {
                    gene *= -1;
                }
                if(rng.nextDouble(0.0, 1.0) < mutation_rate / 10)
                {
                    gene += rng.nextDouble(-mutation_range, mutation_range) * 3;
                }
            }
            to_mutate.pop_back();
            to_mutate.pop_back();
            population[i].nn.load_from_vector(to_mutate);
        }
    }
};
