#ifndef DS2_PROJ
#define DS2_PROJ

#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <set>
#include <optional>

using namespace std;


struct RGA_Node {
    std::string id; // Unique identifier for the character
    string value;     // The character value
    bool is_deleted; // Tombstone to mark deletion
    std::string prev_id; // Stores previous character
    std::map<char, int> version_vector;

    RGA_Node(std::string id, std::string value, std::map<char, int> version_vector, string prev_id = ""): id(id), value(value), is_deleted(false), version_vector(version_vector), prev_id(prev_id) {}
};

class RGA {
private:
    std::vector<RGA_Node> nodes; // The main array storing the text
    std::map<std::string, size_t> id_to_index; // Map to quickly find nodes by ID
    std::map<char, int> version_vector;

public:
    //
    bool is_empty(){
        return nodes.empty();
    }

    int getNodeIndex(const RGA_Node& r1){
        return id_to_index[r1.id];
    }

    std::vector<RGA_Node> getNodes(){
        return nodes;
    }

    std::map<std::string, size_t> get_id_to_index() {
        return id_to_index;
    }

    std::map<char,int> getVersionVecotr() {
        return version_vector;
    }

    std::map<char,int> getNodeVersionVector(string id){
        for(const auto& node: nodes){
            if (node.id == id) {
                return node.version_vector;
            }
        }
    }

    bool isDominate(const std::map<char, int>& a, const std::map<char, int>& b){
        bool atleast_one_great = false;
        for (const auto& [client, seq] : b) {
            if (a.count(client) == 0 || a.at(client) < seq){
                return false;
            }
            if (a.count(client) != 0 && a.at(client) > seq){
                atleast_one_great = true;
            }
        }
        for (const auto& [client, seq] : a){
            if (b.count(client) == 0 && seq > 0){
                atleast_one_great = true;
            }
        }
        return atleast_one_great;
    }

    bool is_concurrent(const RGA_Node& a, const RGA_Node& b){
        return !isDominate(a.version_vector,b.version_vector) && !isDominate(b.version_vector,a.version_vector); 
    }

    // Insert a character at a specific position
    void insert(const std::string& id, const string& value, const string& prev_id = "") {
        version_vector[id[0]]++;
        size_t index = 0;
        if (prev_id != "" && id_to_index.find(prev_id) != id_to_index.end()) {
            index = id_to_index[prev_id] + 1;
        }
        else {
            for (const auto& node: nodes){
                if (node.prev_id == id) {
                    index = id_to_index[node.id];
                }
            }
        }
        
        RGA_Node new_node(id, value, version_vector, prev_id);
        nodes.insert(nodes.begin() + index, new_node);
        id_to_index[id] = index;

        // Update indices for subsequent nodes
        for (size_t i = index + 1; i < nodes.size(); ++i) {
            if (nodes[i].prev_id == prev_id){
                nodes[i].prev_id = new_node.id;
            }
            id_to_index[nodes[i].id] = i;
        }
    }

    // Delete a character by marking it as deleted (tombstone)
    void remove(const std::string& id) {
        version_vector[id[0]]--;
        if (id_to_index.find(id) != id_to_index.end()) {
            size_t index = id_to_index[id];
            for (int j = 0; j < nodes.size(); j++){
                if(nodes[j].prev_id == nodes[index].id){
                    RGA_Node tempNode = nodes[id_to_index[nodes[index].prev_id]];
                    while (tempNode.is_deleted) {
                        if (tempNode.prev_id == ""){
                            break;
                        }
                        tempNode = nodes[id_to_index[tempNode.prev_id]];
                    }
                    if(!tempNode.is_deleted){
                        nodes[j].prev_id = tempNode.id;
                    }
                    else{
                        nodes[j].prev_id = "";
                    }
                    break;
                }
            }
            nodes[index].is_deleted = true;
        }
    }

    // Search for a character by its ID
    std::optional<std::string> search(const std::string& id) const {
        if (id_to_index.find(id) != id_to_index.end()) {
            size_t index = id_to_index.at(id);
            if (!nodes[index].is_deleted) {
                return nodes[index].value;  // Now returns the string value
            }
        }
        return std::nullopt;
    }

    void merge(RGA_Node& other_node){
        if (search(other_node.id) == nullopt) {
            // Resolve concurrent inserts at the same position
            bool conflict = false;
            for (const auto& local_node : nodes) {
                if (local_node.prev_id == other_node.prev_id) {
                    if (is_concurrent(local_node, other_node)){
                        conflict = true;
                        // Tie-breaker: lexicographical node ID comparison
                        if (other_node.id < local_node.id) {
                            int index = id_to_index[local_node.id];
                            nodes.insert(nodes.begin() + index, other_node);
                        }
                        else {
                            int index = id_to_index[local_node.id] + 1;
                            nodes.insert(nodes.begin() + index, other_node);
                        }
                        

                    }
                    else if(isDominate(local_node.version_vector,other_node.version_vector)){
                        conflict = true;
                        int index = id_to_index[local_node.id];
                        nodes.insert(nodes.begin() + index, other_node);
                    }
                    else {
                        conflict = true;
                        int index = id_to_index[local_node.id] + 1;
                        nodes.insert(nodes.begin() + index, other_node);
                    }
                    break;
                }
            }
            if (!conflict) {
                insert(other_node.id, other_node.value, other_node.prev_id);
            }
        }
    }

    // // Merge another RGA into this one (used for synchronization)
    // void merge(RGA& other) {
    //     std::set<std::string> existing_ids;
    //     for (const auto& node : nodes) {
    //         existing_ids.insert(node.id);
    //     }

    //     for (const auto& other_node : other.nodes) {
    //         if (!existing_ids.count(other_node.id)) {
    //             // Resolve concurrent inserts at the same position
    //             bool conflict = false;
    //             for (const auto& local_node : nodes) {
    //                 if (local_node.prev_id == other_node.prev_id) {
    //                     if (is_concurrent(local_node, other_node)){
    //                         conflict = true;
    //                         // Tie-breaker: lexicographical node ID comparison
    //                         if (other_node.id < local_node.id) {
    //                             int index = id_to_index[local_node.id];
    //                             nodes.insert(nodes.begin() + index, other_node);
    //                         }
    //                         else {
    //                             int index = id_to_index[local_node.id] + 1;
    //                             nodes.insert(nodes.begin() + index, other_node);
    //                         }
                            

    //                     }
    //                     else if(isDominate(local_node.version_vector,other_node.version_vector)){
    //                         conflict = true;
    //                         int index = id_to_index[local_node.id];
    //                         nodes.insert(nodes.begin() + index, other_node);
    //                     }
    //                     else {
    //                         conflict = true;
    //                         int index = id_to_index[local_node.id] + 1;
    //                         nodes.insert(nodes.begin() + index, other_node);
    //                     }
    //                     break;
    //                 }
    //             }
    //             if (!conflict) {
    //                 insert(other_node.id, other_node.value, other_node.prev_id);
    //             }
    //         }
    //     }
    //     other = *this;
    // }

    // Print the current state of the document (ignoring deleted characters)
    string print_document() const {
        string s1;
        for (const auto& node : nodes) {
            if(!node.is_deleted){
                s1 += node.value;
            }

        }
        return s1;
    }
};


#endif
