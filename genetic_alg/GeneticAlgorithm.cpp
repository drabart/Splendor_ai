//
// Created by drabart on 03/01/2022.
//

/*
PBs:
    2 player: win in 28
    3 player:
    4 player: win in 29
*/


#include <bits/stdc++.h>
#include "NeuralNetwork.h"
#include "SplendorGame.h"
#include "Agent.h"
#include "Board.h"
using namespace std;

class GeneticAlgorithm
{
public:
    int population_size;
    int rounds;
    int players;
    int generation;
    bool mutation_based;
    double mutation_rate;
    double mutation_range;
    bool debug;
    bool print_fitness;
    string saved_networks;
    vector<Agent> population;
    vector<LayerBase> layers;
    RngGenerator rng;
    Board test_board;

    GeneticAlgorithm(int players, int population_size, int rounds, double mutation_rate, double mutation_range,
                     vector<LayerBase> layers_l, bool rand_start, bool print_fitness, bool debug)
    {
        generation = 1;
        this->debug = debug;
        this->print_fitness = print_fitness;

        mutation_based = !rand_start;
        this->mutation_rate = mutation_rate;
        this->mutation_range = mutation_range;

        saved_networks = "saved_networks.txt";
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

    void proceed_one_generation(int threshold, int max_move, const string& crossover_type)
    {
        // print separator and current generation number
        printf("----------------------------------------------------------------------------\n");
        printf("Generation: %d\n", generation);

        // start measuring time for playing phase
        chrono::time_point<chrono::system_clock> start, end;
        start = chrono::system_clock::now();

        // play games across population to determine best players
        int unfinished_games = play_game(max_move);

        // finish measuring time for playing phase
        end = chrono::system_clock::now();
        chrono::duration<double> elapsed_seconds = end - start;

        // print elapsed time
        printf("Playing done, took: %lf s\n", elapsed_seconds.count());

        // sort by fitness
        sort(population.begin(), population.end(),
             [](Agent const &a1, Agent const &a2) { return a1.fitness < a2.fitness; });

        population[0].save_to_file(saved_networks);

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
        printf("\nUnfinished games: %d\n", unfinished_games);
        printf("Average fitness across population: %lf\n", double(sum) / double(population_size));

        // start measuring for crossover and mutation phase
        start = chrono::system_clock::now();

        // pick crossover from argument
        if(crossover_type == "cut_fill")
        {
            crossover_cut_fill(threshold);
        }
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

        // increment generation
        generation++;
    }

    int play_game(int max_move) // result changes depending on the shuffle of the deck
    {
        int unfinished = 0;

        for(int i = 0; i < population_size; ++i)
            population[i].fitness = 0;
        for(int i = 0; i < rounds; ++i)
        {
            shuffle(population.begin(), population.end(), rng.rng);
            vector<Agent> players_batch;
            for(int j = 0; j < (population_size / players); ++j)
            {
                players_batch.clear();
                for(int k = 0; k < players; ++k)
                {
                    players_batch.push_back(population[j * players + k]);
                }
                SplendorGame game = SplendorGame(players, players_batch, test_board);
                game.debug = debug;

                vector<int> fitness_results = game.play_game(max_move);

                if(fitness_results[0] > 100000)
                {
                    unfinished++;
                }

                for(int k = 0; k < players; ++k)
                {
                    population[j * players + k].fitness += fitness_results[k];
                }
            }
        }

        return unfinished;
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
        vector<Agent> new_population = population;

        for(int i = threshold; i < population_size; i+=2)
        {
            int parent_id1 = rng.nextNormalInt(0, population_size - 1);
            int parent_id2 = rng.nextNormalInt(0, population_size - 1);
            // printf("%d %d\n", parent_id1, parent_id2);
            Agent parent1 = population[parent_id1];
            Agent parent2 = population[parent_id2];
            vector<double> genes1 = parent1.nn.vectify();
            vector<double> genes2 = parent2.nn.vectify();

            int weights_count = int(genes1[genes1.size()-2]);
            int biases_count = int(genes1[genes1.size()-1]);

            vector<double> new_weights1(weights_count);
            vector<double> new_weights2(weights_count);

            vector<double> new_biases1(biases_count);
            vector<double> new_biases2(biases_count);

            int cutoff_w = rng.nextInt(1, weights_count-2);
            int cutoff_b = rng.nextInt(1, biases_count-2);
            // int cutoff_b = 0;

            for(int j = 0; j < cutoff_w; ++j)
            {
                new_weights1[j] = genes1[j];
                new_weights2[j] = genes2[j];
            }

            for(int j = cutoff_w; j < weights_count; ++j)
            {
                new_weights1[j] = genes2[j];
                new_weights2[j] = genes1[j];
            }

            for(int j = 0; j < cutoff_b; ++j)
            {
                new_weights1[j] = genes1[j + weights_count];
                new_weights2[j] = genes2[j + weights_count];
            }

            for(int j = cutoff_b; j < biases_count; ++j)
            {
                new_weights1[j] = genes2[j + weights_count];
                new_weights2[j] = genes1[j + weights_count];
            }

            new_population[i].new_network(new_weights1, new_biases1);
            if(i+1 < population_size)
                new_population[i+1].new_network(new_weights2, new_biases2);
        }

        population = new_population;
        new_population.clear();
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
                if(rng.nextDouble(0.0, 1.0) < mutation_rate / 3)
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

vector<LayerBase> layers;

int main()
{
    setbuf(stdout, nullptr);
    layers.emplace_back(0, "sigmoid"); // first layer will automatically initialise
    layers.emplace_back(300, "linear");
    layers.emplace_back(200, "relu");
    layers.emplace_back(100, "sigmoid");
    layers.emplace_back(60, "relu");
    // training time is mostly dependent on population_size * rounds
    GeneticAlgorithm handler = GeneticAlgorithm(2, 6, 4, 0.01, 0.05,
                                                layers, false, true, false);

    for(int i = 0; i < 100; ++i)
    {
        // TODO some multithreading wouldn't hurt
        // TODO fitness is bad distinction across generations
        handler.proceed_one_generation(5, 100, "divide_parents");
    }
}
