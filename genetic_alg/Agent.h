//
// Created by drabart on 03/01/2022.
//

#include <bits/stdc++.h>

#include <utility>
#include "NeuralNetwork.h"
#include "RngGenerator.h"
using namespace std;

#ifndef SPLENDOR_NEAT_AGENT_H
#define SPLENDOR_NEAT_AGENT_H

class Agent
{
public:
    NeuralNetwork nn;
    int fitness;
    int uid{};
    bool mutation_based;
    vector<LayerBase> layers;
    RngGenerator rng;

    Agent(vector<LayerBase> layers, bool rng)
    {
        this->layers = move(layers);
        mutation_based = rng;
        nn = NeuralNetwork(this->layers, rng);
        fitness = 0;
        change_uid();
    }

    Agent(const string& filepath, int line, int uid)
    {
        layers.clear();
        mutation_based = false;
        fitness = 0;
        if(line != -1)
            load_from_file_line(filepath, line);
        else
            load_from_file_uid(filepath, uid);
    }

    void new_network()
    {
        nn = NeuralNetwork(layers, mutation_based);
        // change uid as nn defines the agent
        change_uid();
    }

    void new_network(vector<double> weights, vector<double> biases)
    {
        nn.load_from_vector(move(weights), move(biases));
        // change uid as nn defines the agent
        change_uid();
    }

    void change_uid()
    {
        uid = rng.nextInt(0, INT32_MAX);
    }

    void save_to_file(const string& filepath)
    {
        fstream save_file;
        save_file.open(filepath, fstream::out | fstream::app);
        if(!save_file.is_open())
        {
            printf("Can't access the file!\n");
            return;
        }

        save_file.precision(4);

        save_file << uid << " " << fitness << " " << mutation_based << " " << layers.size() << " ";
        for(LayerBase & layer : layers)
        {
            save_file << layer.nodes << " " << layer.activation << " ";
        }

        vector<double> weights_and_biases = nn.vectify();
        weights_and_biases.pop_back(); // remove number of total weights and biases
        weights_and_biases.pop_back();
        for(double & variable : weights_and_biases)
        {
            save_file << variable << " ";
        }

        save_file << "\n";
    }

    void load_from_file_line(const string& filepath, int line)
    {
        fstream load_file;
        load_file.open(filepath, fstream::in);
        if(!load_file.is_open())
        {
            printf("Can't access the file!\n");
            return;
        }

        string s; // dump string to load into
        stringstream ss; // stream is more useful and easier to handle here
        for(int i = 0; i < line - 1 && !load_file.eof(); ++i)
        {
            getline(load_file, s);
        }

        if(load_file.eof())
        {
            printf("No such line!\n");
            return;
        }

        getline(load_file, s);
        ss << s;

        int layer_count, node_count;
        string activation_type;
        ss >> uid >> fitness >> mutation_based >> layer_count;
        for(int i = 0; i < layer_count; ++i)
        {
            ss >> node_count >> activation_type;
            layers.emplace_back(node_count, activation_type);
        }
        printf("hi");

        nn = NeuralNetwork(layers, mutation_based);
        vector<double> weights_and_biases(nn.weights_count + nn.biases_count);
        double variable;
        for(int i = 0; i < nn.weights_count + nn.biases_count; ++i)
        {
            ss >> variable;
            weights_and_biases[i] = variable;
        }
        printf("hi");

        nn.load_from_vector(weights_and_biases);
    }

    void load_from_file_uid(const string& filepath, int searched_uid)
    {
        fstream load_file;
        load_file.open(filepath, fstream::in);
        if(!load_file.is_open())
        {
            printf("Can't access the file!\n");
            return;
        }

        uid = 0;

        string s; // dump string to load into
        stringstream ss; // stream is more useful and easier to handle here
        while(!load_file.eof())
        {
            getline(load_file, s);
            ss << s;
            int t_uid;
            ss >> t_uid;
            if(t_uid == searched_uid)
            {
                uid = t_uid;
                break;
            }
        }

        if(!uid)
        {
            printf("No such uid!\n");
            return;
        }

        int layer_count, node_count;
        string activation_type;
        ss >> fitness >> mutation_based >> layer_count;
        for(int i = 0; i < layer_count; ++i)
        {
            ss >> node_count >> activation_type;
            layers.emplace_back(node_count, activation_type);
        }

        nn = NeuralNetwork(layers, mutation_based);

        vector<double> weights_and_biases(nn.weights_count + nn.biases_count);
        double variable;
        for(int i = 0; i < nn.weights_count + nn.biases_count; ++i)
        {
            ss >> variable;
            weights_and_biases[i] = variable;
        }

        nn.load_from_vector(weights_and_biases);
    }
};

#endif //SPLENDOR_NEAT_AGENT_H
