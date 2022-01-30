//
// Created by drabart on 14/01/2022.
//

#include <bits/stdc++.h>
using namespace std;

#ifndef SPLENDOR_NEAT_RNGGENERATOR_H
#define SPLENDOR_NEAT_RNGGENERATOR_H

class RngGenerator
{
public:
    int seed{};
    mt19937 rng;

    explicit RngGenerator()
    {
        // quality time for the seeds not to overlap
        seed =  int(chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now().time_since_epoch()).count());
        rng.seed(seed);
    }

    explicit RngGenerator(int seed)
    {
        this->seed = seed;
        rng.seed(this->seed);
    }

    int nextInt(int from, int to)
    {
        if(from > to)
            return 0;
        unsigned int rand = rng();
        // scale to len
        rand %= (to - from);
        // move
        return int(rand) + from;
    }

    double nextDouble(double from, double to)
    {
        if(from == to)
            return from;
        if(from > to)
            return 0;
        auto rand = double((unsigned int)rng());
        double len = to - from;
        // scale to len
        rand /= (4123659995.0 / len);
        // move
        rand += from;
        return rand;
    }

    int nextNormalInt(int from, int to)
    {
        normal_distribution<double> normalDistribution(0.0, 2.0);
        // from -5 to 5 on x; from 0 to 0.4 on y

        double res = normalDistribution(rng);
        res = abs(res);

        res *= double(to - from) / 5.0;
        return min(int(res) + from, to);
    }
};

#endif //SPLENDOR_NEAT_RNGGENERATOR_H
