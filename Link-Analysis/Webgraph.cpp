// Webgraph.cpp
// Author: Jinwei Shen
//
// Implementation of Webgraph class functions

#include <queue>
#include <cmath>

#include "Node.h"
#include "Webgraph.h"

typedef unsigned int uint;

using namespace std;

const vector<Node> &Webgraph::getAllNodes() const {
    return all_nodes;
}

bool Webgraph::hasLink(const string &url_) const {
    for (uint i = 0; i < all_nodes.size(); ++i) {
        if (all_nodes[i].getUrl() == url_) {
            return true;
        }
    }
    return false;
}

// Assume the node with the provided url exists
const Node &Webgraph::getNodeFromLink(const std::string &url_) const {
    uint i = 0;
    for (; i < all_nodes.size(); ++i) {
        if (all_nodes[i].getUrl() == url_) {
            break;
        }
    }
    return all_nodes[i];
}

// Assume the node with the provided url exists
uint Webgraph::getNodeIndexFromLink(const std::string &url_) const {
    uint i = 0;
    for (; i < all_nodes.size(); ++i) {
        if (all_nodes[i].getUrl() == url_) {
            break;
        }
    }
    return i;
}

// Assume the provided node exists
const std::vector<std::string> &Webgraph::getIncomingLinks(const Node &n) const {
    return n.getParents();
}

// Assume the provided node exists
const std::vector<std::string> &Webgraph::getOutgoingLinks(const Node &n) const {
    return n.getChildren();
}

/*
    Get the rank and normalized rank for all the URLs

    Returns:
        std::map<std::string, vector<float> >:  A map contains the URLs with
                                                corresponding ranks.
            Key: std::string, URL
            Value: vector<float>, first element is the actual rank,
                                second element is the normalized rank
*/
const map<string, pair<float, float>> Webgraph::getAllRanks() const {
    map<string, pair<float, float>> all_ranks;
    //get all the acutal rank for each URL, and record the highest rank
    float rank_max = 0.0;
    for (uint i = 0; i < all_nodes.size(); ++i) {
        all_ranks[all_nodes[i].getUrl()] = pair<float, float>();
        all_ranks[all_nodes[i].getUrl()].first = all_nodes[i].getRank();
        if (all_nodes[i].getRank() > rank_max) {
            rank_max = all_nodes[i].getRank();
        }
    }
    //calculate the scale factor
    float scale = 10 / rank_max;
    //store the normalized rank
    for (uint i = 0; i < all_nodes.size(); ++i) {
        all_ranks[all_nodes[i].getUrl()].second = all_nodes[i].getRank() * scale;
    }
    return all_ranks;
}

bool Webgraph::addLink(const string &url_) {
    if (hasLink(url_)) {
        return false;
    }
    Node n(url_);
    all_nodes.push_back(n);
    return true;
}

bool Webgraph::addConnection(const std::string &from_url,
                             const std::string &to_url) {
    if (hasLink(from_url) == false) {
        addLink(from_url);
    }
    if (hasLink(to_url) == false) {
        addLink(to_url);
    }
    // update Node info
    uint index_from = getNodeIndexFromLink(from_url);
    uint index_to = getNodeIndexFromLink(to_url);
    bool addedC = all_nodes[index_from].addChild(all_nodes[index_to].getUrl());
    bool addedP = all_nodes[index_to].addParent(all_nodes[index_from].getUrl());

    return (addedC || addedP);
}

/*
    inital the rank update, assume url exists

    Parameters:
        string url_: The URL for the starting node for update
*/
void Webgraph::updateRank(const string &url_) {
    //inital the queue and start the iterative update
    queue<uint> work_queue;
    uint index_start = getNodeIndexFromLink(url_);
    work_queue.push(index_start);
    updateHelper(work_queue);
    return;
}

/*
    Actual rank update function, iterative update the
    rank of the nodes until the queue is empty

    Parameters:
        queue<uint> work_queue: The queue that store the URL
                                nodes' index that need to be updated
*/
void Webgraph::updateHelper(queue<uint> work_queue) {
    //finish condition
    if (work_queue.empty()) {
        return;
    }
    //update the rank of the node that is on the top
    //of the queue
    uint index_current = work_queue.front();
    work_queue.pop();
    //calculate new rank based on incoming nodes
    vector<string> parents = getIncomingLinks(all_nodes[index_current]);
    float new_rank = 0;
    for (uint i = 0; i < parents.size(); ++i) {
        float from_rank = all_nodes[getNodeIndexFromLink(parents[i])].getRank();
        int outgoing_number = getOutgoingLinks(all_nodes[getNodeIndexFromLink(parents[i])]).size();
        new_rank += from_rank / outgoing_number;
    }
    new_rank = (1 - damping_factor) + damping_factor * new_rank;
    float old_rank = all_nodes[index_current].getRank();
    //update the rank
    all_nodes[index_current].updateRank(new_rank);
    //check converge, threshold current set to 0.01%
    if (float(abs(new_rank - old_rank) / old_rank) < rank_threshold) {
        //converged, current node's children would not add to queue
        updateHelper(work_queue);
    }
    else {
        //not converged, all current node's children to queue
        vector<string> children = getOutgoingLinks(all_nodes[index_current]);

        for (uint i = 0; i < children.size(); ++i) {
            work_queue.push(getNodeIndexFromLink(children[i]));
        }
        updateHelper(work_queue);
    }
    return;
}
