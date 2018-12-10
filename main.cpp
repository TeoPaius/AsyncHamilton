#include <iostream>
#include <vector>
#include <fstream>
#include <future>
#include <unistd.h>
#include <chrono>
#include <thread>

using namespace std;
using namespace std::chrono_literals;





int nrEdges;
int nrVertices;
mutex printingMtx;
mutex seedMtx;
int idSeed = 0;
class Node
{
public:
    int id;
    vector<Node*> children;

    Node()
    {
        id = -1;
        children = vector<Node*>();
    }
    Node(int _id):Node()
    {
        id = _id;
    }

};

class Link
{
public:

    future<void> th;
    vector<Node*> way;
    int visited[1000];
    bool finished = false;
    int lId;


    Link()
    {
        seedMtx.lock();
        this->lId = idSeed;
        idSeed++;
        seedMtx.unlock();
        for(int i = 0; i < nrVertices; ++i)
            visited[i] = false;
        way = vector<Node*>();
    }

    Link(const Link* other):Link()
    {
        for(int i = 0; i < nrVertices; ++i)
            visited[i] = other->visited[i];

        for(Node* n : other->way)
        {
            way.push_back(n);
        }
    }

    void printWay(bool isCycle)
    {
        printingMtx.lock();

        cout << "ID: " << lId << '|';
        for(Node* node : way)
        {
            cout << node->id << ' ';
        }
        if(isCycle)
            cout << way[0]->id;
        cout << '\n';
        printingMtx.unlock();
    }


};

bool visited[1000];
vector<Link*> links;


void dfsBrute(Node* root, Link* currentLink)
{

    //currentLink->printWay(false);
    currentLink->visited[root->id] = true;
    //cout << root->id << ' ';
    currentLink->way.push_back(root);
    vector<future<void>> threads;


    if(root->children.empty())
        return;

    bool wasFirst = false;
    int keepId = -1;
    for(int i = 0; i < root->children.size(); ++i)
    {
        if(currentLink->visited[root->children[i]->id])
        {
            if(currentLink->way.size() == nrVertices && currentLink->way[0]->id == root->children[i]->id)
                currentLink->printWay(true);
        }

        if(!currentLink->visited[root->children[i]->id])
        {
            if(keepId == -1)
            {
                keepId = i;
            }
            else
            {
                Link* newLink = new Link(currentLink);
                Node* child = root->children[i];
                threads.emplace_back(async(std::launch::async,[i, child, newLink]{
                    //this_thread::sleep_for(1s);
                    //cout << "start th\n";

                    dfsBrute(child, newLink);}));
            }
        }
    }
    if(keepId != -1)
        dfsBrute(root->children[keepId], currentLink);


    for (auto &thread : threads) {
        thread.get();
    }
    currentLink->visited[root->id] = false;
    currentLink->way.pop_back();
    currentLink->finished = true;


}



vector<Node*> nodes;



int main() {

    ifstream is("../input.txt");
    is >> nrEdges;
    is >> nrVertices;

    for(int i = 0; i < nrVertices; ++i)
    {
        nodes.push_back(new Node(i));
        visited[i] = false;
    }

    int from, to;
    for(int i = 0; i < nrEdges; ++i)
    {

        is >> from >> to;
        nodes[from]->children.push_back(nodes[to]);
    }

    dfsBrute(nodes[0], new Link());


    return 0;
}