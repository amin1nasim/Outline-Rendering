#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>
#include <stdexcept>
#include <iostream>

struct VFB
{
    unsigned int V;
    bool A = 0;
    bool F = 0;
    bool B = 0;
    bool Fa = 0; //Not required in A#2
    bool Ba = 0; //Not required in A#2
};

// This structure keeps the information of a plane/face of the object
struct Face
{
    int vi[3];
    glm::vec3 n;
    glm::vec3 center;
};

class EdgeBuffer
{
public:
    //Constructor
    EdgeBuffer(int numVertices)
    {
        //Here we initialize the matrix to have numVertices number of rows
        //There are no columns yet, We add them using the Edge-Buffer construction algorithm!
        for (int i = 0; i < numVertices; i++)
        {
            eb.push_back(std::vector<VFB>());
        }
    }
    
    void clearBuffer() {
        for (int i = 0; i < eb.size(); i++) {
            for (int j = 0; j < eb[i].size(); j++) {
                eb[i][j].F = 0;
                eb[i][j].B = 0;
                eb[i][j].Fa = 0;
                eb[i][j].Ba = 0;
            }
        }
    }

    // Toggles (XOR) the front bit(F)
    void toggleFront(int v1, int v2) {
        v2 = indexFinder(v1, v2); // IndexFinder Searches the row associated with v1 to find the index of v2.
        eb[v1][v2].F ^= 1;
    }

    // Toggles (XOR) the back bit(B)
    void toggleBack(int v1, int v2) {
        v2 = indexFinder(v1, v2);
        eb[v1][v2].B ^= 1;
    }
    
    // Sets the front absolute bit to one(Fa)
    void oneFrontA(int v1, int v2) {
        v2 = indexFinder(v1, v2);
        eb[v1][v2].Fa |= 1;
    }

    // Sets the back absolute bit to one(Ba)
    void oneBackA(int v1, int v2) {
        v2 = indexFinder(v1, v2);
        eb[v1][v2].Ba |= 1;
    }
    
    // Sets the value of artist bit(A = state)
    void setArtist(int v1, int v2, bool state) {
        v2 = indexFinder(v1, v2);
        eb[v1][v2].A = state;
    }
    
    // IndexFinder Searches the row associated with v1 to find the index of v2.
    int indexFinder(int v1, int v2) {
        for (int i = 0; i < eb[v1].size(); i++) { // finding v2 in the row associated to v1
            if (eb[v1][i].V == v2)
                return i;
        }
        throw std::invalid_argument("Could not find the vertex in the EdgeBuffer"); // If v2 was not found in row of v1, throw an exception
    }
    
    // Print the edge buffer
    void print()
    {
        printf("Printing EdgeBuffer: \n");
        for (int i = 0; i < eb.size(); i++)
        {
            printf("%d\t", i);
            for (int j = 0; j < eb[i].size(); j++)
            {
                printf("%d | %d | %d | %d | %d |||", eb[i][j].V, eb[i][j].F, eb[i][j].B, eb[i][j].Fa, eb[i][j].Ba);
            }
            printf("\n");
        }
        printf("\n\n");
    }

    //The 2D vector of VFB's
    std::vector<std::vector<VFB>> eb;
};
