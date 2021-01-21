#ifndef MESHES_H
#define MESHES_H

//Consider: Make this MeshManager singleton or something that lets you getQuad() and getLine()?
//To Do: get this code review by DX to ensure if I have to refactor this to 'proper C++ OOP' or whatever

class Meshes
{
    public:
    Meshes();
    
    static Meshes *instance; //access from anywhere
    unsigned int quadVAO;
    unsigned int quadTextVAO;
    unsigned int quadTextVBO;
    //int lineVBO;
};

#endif