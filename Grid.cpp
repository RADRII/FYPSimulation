#include "Grid.h"
#include "Resource.h" // needed for linking of res entry locations
#include <iostream>
#include <queue>

LocNode::LocNode(int x1, int y1, NodeKind k) {
    x = x1;
    y = y1;
    type = k;
    occupancy = 0;
}

string LocNode::tostring() {
    string s;
    if(type == EMPTY) { s = "e"; }
    else if(type == HAB_ZONE) { s = "h"; }
    else if(type == OBSTACLE) { s = "o"; }
    else if(type == RESOURCE) { s = "r"; }
    else { s = "u"; }
    s += "[";
    // s += id;
    // s += " ";
    s += f_to_s(x);
    s += ",";
    s += f_to_s(y);
    s += "]";
    return s;
}

LocGrid::LocGrid(int s) {
    size = s;

    //Create empty grid of size s
    nodes.resize(size, std::vector<LocNode>(size, LocNode(0, 0, EMPTY)));
    for (int x = 0; x < size; x++) {
        for (int y = 0; y < size; y++) {
            nodes[x][y].x = x;
            nodes[x][y].y = y;
        }
    }
}

LocNode* LocGrid::getNode(int x, int y) {
    if (x >= 0 && x < size && y >= 0 && y < size)
        return &nodes[x][y];
    else
        return nullptr;
}

std::vector<LocNode*> LocGrid::findPath(LocNode* startNode, LocNode* endNode) {
    std::vector<LocNode*> path;
    if (!startNode || !endNode)
        return path;

    queue<LocNode*> frontier;
    frontier.push(startNode);

    while (!frontier.empty()) {
        LocNode* current = frontier.front();
        frontier.pop();

        if (current == endNode) {
            // Reconstruct path
            LocNode* temp = current;
            while (temp != nullptr) {
                path.push_back(temp);
                temp = temp->parent;
            }
            std::reverse(path.begin(), path.end());
            break;
        }

        // Traverse neighbors
        static const int dx[] = { -1, 1, 0, 0 }; // Left, Right, Up, Down
        static const int dy[] = { 0, 0, -1, 1 };

        for (int i = 0; i < 4; ++i) {
            int newX = current->x + dx[i];
            int newY = current->y + dy[i];

            LocNode* neighbor = getNode(newX, newY);
            if (!neighbor || neighbor->type != OBSTACLE || neighbor->parent)
                continue;

            neighbor->parent = current;
            frontier.push(neighbor);
        }
    }

    return path;
}