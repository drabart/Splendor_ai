//
// Created by drabart on 03/01/2022.
//

#include <bits/stdc++.h>
#include <thread>
#include <utility>
#include "NeuralNetwork.h"
#include "MemoryTool.h"
#include "GeneticAlgorithm.h"
using namespace std;

int train()
{
    int thread_count = 16;
    int player_count = 4;
    int max_move = 35;

    vector<LayerBase> layers;
    layers.emplace_back(0, "sigmoid"); // first layer will automatically initialise
    layers.emplace_back(300, "linear");
    layers.emplace_back(200, "relu");
    layers.emplace_back(100, "sigmoid");
    layers.emplace_back(60, "relu");

    // training time is mostly dependent on population_size * rounds
    int generations = 40;
    int population = 10080 / generations; // size of population
    int agents_from_before = population / 10; // number of agents added from previous generations
    int population_champions = 4; // number of saved Agents to use in following generations
    int training_rounds = 8; // number of rounds used for comparing Agents
    int test_rounds = 10; // number of rounds used for final assesment
    int mutation_rate = 0.005; // frequency of mutations
    int mutation_range = 0.05; // intensity of mutations
    bool random_start = true; // decides if starting neural networks will be 0-es or random
    bool print_generation_finesses = false; // prints fitness of each generation top 10 performers
    bool debug_info = false; // prints some additional info

    // TODO represent unfinished games as percentage
    GeneticAlgorithm handler = GeneticAlgorithm(player_count, population, training_rounds,
                                                mutation_rate, mutation_range,
                                                layers, random_start, print_generation_finesses, debug_info,
                                                agents_from_before, population_champions, test_rounds);

    long double nn_vectified_size = handler.population[0].nn.vectify().size();

    printf("Approximate memory usage (neural network): %.3LfMB, (boards): %.3LfMB, (saved networks per generation): %.3LfMB\n",
           nn_vectified_size * 8.0 * handler.population_size / 1048576.0,
           ((long double)handler.test_board.vectify(0).size() * 8.0 / 1048576.0 +
           nn_vectified_size * 8.0 * player_count / 1048576.0) * thread_count,
           nn_vectified_size * 8.0 * population_champions / 1048576.0);

    handler.max_generation = generations;

    for(int i = 0; i < handler.max_generation; ++i)
    {
        // TODO more crossovers
        // TODO play around with hyperparameters
        handler.proceed_one_generation(10, max_move, "divide_parents");
    }

    handler.print_fitness = true;
    handler.compare_all(population_champions, max_move, 2);

    return 0;
}
