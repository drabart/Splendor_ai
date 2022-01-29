//
// Created by drabart on 03/01/2022.
//

#include <bits/stdc++.h>

#include <utility>
#include "RngGenerator.h"
using namespace std;

#ifndef SPLENDOR_NEAT_NEURALNETWORK_H
#define SPLENDOR_NEAT_NEURALNETWORK_H

class LayerBase
{
public:
    int nodes{};
    string activation; // sigmoid, relu, linear

    LayerBase() = default;

    LayerBase(int nodes, string activation)
    {
        this->nodes = nodes;
        this->activation = move(activation);
    }
};

class Layer : public LayerBase
{
public:
    bool is_output;
    int next_layer_nodes;
    vector<vector<double> > weights;
    vector<double> biases;
    RngGenerator rng;

    Layer(const LayerBase& base, const LayerBase& next_layer, bool rand = true)
    {
        nodes = base.nodes;
        activation = base.activation;
        is_output = false;
        if(next_layer.nodes)
            next_layer_nodes = next_layer.nodes;
        else
        {
            is_output = true;
            next_layer_nodes = 0;
        }
        if(rand)
        {
            for(int i = 0; i < nodes; ++i)
            {
                biases.push_back(rng.nextDouble(-1.0, 1.0));
            }
            if(!is_output)
            {
                vector<double> v;
                for(int i = 0; i < nodes; ++i)
                {
                    weights.push_back(v);
                    for(int j = 0; j < next_layer_nodes; ++j)
                    {
                        weights[i].push_back(rng.nextDouble(-1.0, 1.0));
                    }
                }
            }
        }
        else
        {
            for(int i = 0; i < nodes; ++i)
            {
                biases.push_back(0.0);
            }
            if(!is_output)
            {
                vector<double> v;
                for(int i = 0; i < nodes; ++i)
                {
                    weights.push_back(v);
                    for(int j = 0; j < next_layer_nodes; ++j)
                    {
                        weights[i].push_back(0.0);
                    }
                }
            }
        }
    }

    double act(double input)
    {
        if(activation == "sigmoid")
        {
            return 1.0 / (1.0 + exp(-input));
        }
        if(activation == "relu")
        {
            return max(0.0, input);
        }
        // linear is default
        return input;
    }

    vector<double> move_to_next(vector<double> input)
    {
        int is = nodes;
        int os = next_layer_nodes;

        for(int i = 0; i < is; ++i)
        {
            input[i] += biases[i];
            input[i] = act(input[i]);
        }

        if(is_output)
        {
            return input;
        }

        vector<double> output;
        output.reserve(os);
        for(int i = 0; i < os; ++i)
        {
            output.push_back(0.0);
        }

        for(int i = 0; i < is; ++i)
        {
            for(int j = 0; j < os; ++j)
            {
                output[j] += weights[i][j] * input[i];
            }
        }

        return output;
    }
};

class NeuralNetwork
{
public:
    int layer_count{};
    vector<Layer> layers;
    int weights_count{};
    int biases_count{};

    NeuralNetwork() = default;

    NeuralNetwork(vector<LayerBase> layers, bool mutation_based)
    {
        layer_count = int(layers.size());
        for(int i = 0; i < layer_count - 1; ++i)
        {
            weights_count += layers[i].nodes * layers[i+1].nodes;
            biases_count += layers[i].nodes;
        }
        biases_count += layers[layer_count - 1].nodes;

        for(int i = 0; i < layer_count - 1; ++i)
        {
            this->layers.emplace_back(layers[i], layers[i+1], !mutation_based);
        }
        LayerBase output(0, "");
        this->layers.emplace_back(layers[layer_count - 1], output, !mutation_based);
    }

    vector<double> propagate(vector<double> input)
    {
        for(int i=0; i^layer_count; ++i)
        {
            input = layers[i].move_to_next(input);
        }

        return input;
    }

    vector<double> vectify()
    {
        vector<double> result;

        // weights
        for(auto& layer : layers)
        {
            if(layer.is_output)
                continue;
            for(auto & from_weights : layer.weights) // for every node in current layer
            {
                for(double & weight : from_weights)
                {
                    result.push_back(weight);
                }
            }
        }

        // biases
        for(auto& layer : layers)
        {
            for(auto & bias : layer.biases) // for every node in current layer
            {
                result.push_back(bias);
            }
        }

        result.push_back(weights_count);
        result.push_back(biases_count);

        return result;
    }

#pragma clang diagnostic push
#pragma ide diagnostic ignored "LocalValueEscapesScope"
    // be careful as this rewrites whole nn. returns false if vector is of incorrect length
    bool load_from_vector(vector<double> weights, vector<double> biases)
    {
        if(weights.size() != weights_count || biases.size() != biases_count)
            return false;

        int it = 0;

        // weights
        for(int i = 0; i < layer_count - 1; ++i)
        {
            for(auto & weight : layers[i].weights) // for every node in current layer
            {
                for(double & k : weight)
                {
                    k = weights[it++];
                }
            }
        }

        it = 0;
        // biases
        for(int i = 0; i < layer_count; ++i)
        {
            for(double & bias : layers[i].biases) // for every node in current layer
            {
                bias = biases[it++];
            }
        }
        return true;
    }

    // be careful as this rewrites whole nn. returns false if vector is of incorrect length
    bool load_from_vector(vector<double> new_nn)
    {
        if(new_nn.size() != weights_count + biases_count)
            return false;

        int it = 0;

        // weights
        for(int i = 0; i < layer_count - 1; ++i)
        {
            for(auto & weight : layers[i].weights) // for every node in current layer
            {
                for(double & k : weight)
                {
                    k = new_nn[it++];
                }
            }
        }

        // biases
        for(int i = 0; i < layer_count; ++i)
        {
            for(double & bias : layers[i].biases) // for every node in current layer
            {
                bias = new_nn[it++];
            }
        }
        return true;
    }
#pragma clang diagnostic pop

    [[maybe_unused]] void print()
    {
        for(int i = 0; i < layer_count; ++i)
        {
            printf("Layer: %d - %d Nodes - with %s activation (%d %d)\n", i, layers[i].nodes,
                   layers[i].activation.c_str(), layers[i].nodes, layers[i].next_layer_nodes);
        }
    }
};

#endif //SPLENDOR_NEAT_NEURALNETWORK_H
