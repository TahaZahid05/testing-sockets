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

    std::map<char,int> getVersionVector() {
        return version_vector;
    }

    std::map<char,int> getNodeVersionVector(const string id){
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
        if(!atleast_one_great) {
            for (const auto& [client, seq] : a){
                if (b.count(client) == 0 && seq > 0){
                    atleast_one_great = true;
                    break;
                }
            }
        }
        return atleast_one_great;
    }

    bool is_concurrent(const RGA_Node& a, const RGA_Node& b){
        return !isDominate(a.version_vector,b.version_vector) && !isDominate(b.version_vector,a.version_vector); 
    }

    // void insert(const RGA_Node& insert_node){
    //     version_vector[insert_node.id[0]]++;
    //     size_t index = 0;
    //     // cout << id << " " << prev_id << endl;
    //     if (insert_node.prev_id != "" && id_to_index.find(insert_node.prev_id) != id_to_index.end()) {
    //         // cout << id_to_index[prev_id];
    //         index = id_to_index[insert_node.prev_id] + 1;
    //     }
    //     else {
    //         for (const auto& node: nodes){
    //             if (node.prev_id == insert_node.id) {
    //                 index = id_to_index[node.id];
    //             }
    //         }
    //     }
    //     nodes.insert(nodes.begin() + index, insert_node);
    //     id_to_index[insert_node.id] = index;

    //     // Update indices for subsequent nodes
    //     for (auto node : nodes) {
    //         if (id_to_index[node.id] >= index && !node.is_deleted){
    //             id_to_index[node.id] += 1;
    //         }
    //         if(node.prev_id == insert_node.prev_id && !node.is_deleted){
    //             node.prev_id = insert_node.id;
    //         }
    //     }
    // }

    // Insert a character at a specific position
    void insert(const std::string& id, const string& value, const std::map<char, int>& version_vector_pass, const string& prev_id = "") {
        RGA_Node new_node(id, value, version_vector_pass, prev_id);
        version_vector[id[0]]++;
        size_t index = 0;
        if (prev_id != "" && id_to_index.find(prev_id) != id_to_index.end()) {
            //TO DO: Have to add code here to consider shifting prev_id forward after comparing version vector
            for (const auto& node: nodes){
                if (node.prev_id == new_node.prev_id && !node.is_deleted && node.id != new_node.id) {
                    // cout << node.id << " " << new_node.id << endl;
                    for (const auto& pair : node.version_vector) {
                        // std::cout << pair.first << ": " << pair.second << std::endl;
                    }
                    for (const auto& pair : new_node.version_vector) {
                        // std::cout << pair.first << ": " << pair.second << std::endl;
                    }
                    if (is_concurrent(node, new_node)) {
                        if (node.id < new_node.id) {
                            new_node.prev_id = node.id;
                            insert(new_node.id, new_node.value, new_node.version_vector, new_node.prev_id);
                            return;
                        }
                        else {
                            break;
                        }
                    }
                    else if (isDominate(node.version_vector,new_node.version_vector)) {
                        new_node.prev_id = node.id;
                        insert(new_node.id, new_node.value, new_node.version_vector, new_node.prev_id);
                        return;
                    }
                    else {
                        break;
                    }

                }
            }
            // cout << "Finally" << endl;
            // cout << new_node.id << new_node.prev_id << endl;
            index = id_to_index[prev_id] + 1;
            // cout << index << endl;
        }
        else {
            for (const auto& node: nodes){
                if (node.prev_id == id) {
                    index = id_to_index[node.id];
                }
            }
        }
        

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
        // version_vector[id[0]]--;
        if (id_to_index.find(id) != id_to_index.end()) {
            size_t index = id_to_index[id];
            RGA_Node* deletedNode = searchNode(id);
            for (auto& node: nodes) {
                if (node.id != id && !node.is_deleted) {
                    if(node.prev_id == deletedNode->id) {
                        if(deletedNode->prev_id == "") {
                            node.prev_id = "";
                        }
                        else {
                            RGA_Node* tempNode = searchNode(deletedNode->prev_id);
                            while (tempNode->is_deleted) {
                                if (tempNode->prev_id == "") {
                                    break;
                                }
                                tempNode = searchNode(tempNode->prev_id);
                            }
                            if (!tempNode->is_deleted) {
                                node.prev_id = tempNode->id;
                            }
                            else {
                                node.prev_id = "";
                            }
                        }
                    }
                    if (id_to_index[node.id] >= index) {
                        id_to_index[node.id] -= 1;

                    }
                }
            }
            deletedNode->is_deleted = true;
            // for (int j = 0; j < nodes.size(); j++){
            //     if(nodes[j].prev_id == nodes[index].id){
            //         RGA_Node tempNode = nodes[id_to_index[nodes[index].prev_id]];
            //         while (tempNode.is_deleted) {
            //             if (tempNode.prev_id == ""){
            //                 break;
            //             }
            //             tempNode = nodes[id_to_index[tempNode.prev_id]];
            //         }
            //         if(!tempNode.is_deleted){
            //             nodes[j].prev_id = tempNode.id;
            //         }
            //         else{
            //             nodes[j].prev_id = "";
            //         }
            //         break;
            //     }
            // }
        }

    }

    RGA_Node* searchNode(const string& idPass) {
        for(auto& node : nodes) {
            if (node.id == idPass) {
                return &node;
            }
        }
        throw std::runtime_error("Node not found: " + idPass);
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
                // cout << other_node.id << " " << local_node.id << endl;
                // cout << other_node.value << " " << local_node.value << endl;
                if (local_node.prev_id == other_node.prev_id && !local_node.is_deleted) {
                    if (is_concurrent(local_node, other_node)){
                        conflict = true;
                        // Tie-breaker: lexicographical node ID comparison
                        if (other_node.id < local_node.id) {
                            // int index = id_to_index[local_node.id];
                            // id_to_index[other_node.id] = index;
                            // nodes.insert(nodes.begin() + index, other_node);
                            // insert(other_node);
                            insert(other_node.id,other_node.value,other_node.version_vector,other_node.prev_id);
                        }
                        else {
                            // other_node.prev_id = local_node.id;
                            // insert(other_node);
                            // int index = id_to_index[local_node.id] + 1;
                            // id_to_index[other_node.id] = index;
                            // nodes.insert(nodes.begin() + index, other_node);
                            other_node.prev_id = local_node.id;
                            insert(other_node.id,other_node.value,other_node.version_vector,other_node.prev_id);
                        }
                        

                    }
                    else if(isDominate(local_node.version_vector,other_node.version_vector)){
                        conflict = true;
                        // other_node.prev_id = local_node.id;
                        // insert(other_node);
                        // int index = id_to_index[local_node.id] + 1;
                        // id_to_index[other_node.id] = index;
                        // nodes.insert(nodes.begin() + index, other_node);
                        other_node.prev_id = local_node.id;
                        insert(other_node.id,other_node.value,other_node.version_vector,other_node.prev_id);
                    }
                    else {
                        // cout << local_node.version_vector.at('A') << endl;
                        // cout << other_node.version_vector.at('A') << " " << other_node.version_vector.at('B') << endl;
                        // conflict = true;
                        // insert(other_node);
                        conflict = true;
                        // int index = id_to_index[local_node.id];
                        // id_to_index[other_node.id] = index;
                        // nodes.insert(nodes.begin() + index, other_node);
                        insert(other_node.id,other_node.value,other_node.version_vector,other_node.prev_id);
                    }
                    break;
                }
            }
            if (!conflict) {
                // insert(other_node);
                insert(other_node.id, other_node.value, other_node.version_vector, other_node.prev_id);
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
    string print_document() {
        string s1;
        for (const auto& node : nodes) {
            if(!node.is_deleted){
                // cout << node.id << " " << node.is_deleted << " " << node.prev_id << " " << node.value << " " << id_to_index[node.id] <<  endl;
                s1 += node.value;
            }

        }
        return s1;
    }

    void initializeFromContent(const std::string& content, char clientId) {
        nodes.clear();
        id_to_index.clear();
        version_vector.clear();
        version_vector[clientId] = content.empty() ? 0 : 1;

        string prev_id = "";
        for (size_t i = 0; i < content.size(); i++) {
            string id = string(1, clientId) + to_string(i+1);
            string val(1, content[i]);
            map<char, int> node_vv = version_vector;
            RGA_Node newNode(id, val, node_vv, prev_id);
            nodes.push_back(newNode);
            id_to_index[id] = i;
            prev_id = id;
            version_vector[clientId]++;
        }
    }

    std::string serializeState() const {
        std::string state;

        // Serialize version vector
        for (const auto& [client, seq] : version_vector) {
            state += string(1, client) + ":" + to_string(seq) + ";";
        }
        state += "|";

        // Serialize nodes
        for (const auto& node : nodes) {
            if (!node.is_deleted) {
                state += node.id + "," + node.value + "," + (node.is_deleted ? "1" : "0") + "," + node.prev_id + ",";

                // Serialize node's version vector
                for (const auto& [client, seq] : node.version_vector) {
                    state += string(1, client) + ":" + to_string(seq) + ",";
                }
                state += ";";
            }
        }
        return state;
    }

    void deserializeState(const std::string& state) {
        nodes.clear();
        id_to_index.clear();
        version_vector.clear();

        size_t vv_end = state.find('|');
        string vv_str = state.substr(0, vv_end);
        string nodes_str = state.substr(vv_end + 1);

        // Deserialize version vector
        size_t vv_pos = 0;
        while (vv_pos < vv_str.length()) {
            size_t colon = vv_str.find(':', vv_pos);
            if (colon == string::npos) break;

            char client = vv_str[vv_pos];
            size_t semicolon = vv_str.find(';', colon);
            int seq = stoi(vv_str.substr(colon + 1, semicolon - colon - 1));

            version_vector[client] = seq;
            vv_pos = semicolon + 1;
        }

        // Deserialize nodes
        size_t node_pos = 0;
        while (node_pos < nodes_str.length()) {
            size_t node_end = nodes_str.find(';', node_pos);
            if (node_end == string::npos) break;

            string node_str = nodes_str.substr(node_pos, node_end - node_pos);
            vector<string> parts;
            size_t part_start = 0;

            while (true) {
                size_t comma = node_str.find(',', part_start);
                if (comma == string::npos) {
                    parts.push_back(node_str.substr(part_start));
                    break;
                }
                parts.push_back(node_str.substr(part_start, comma - part_start));
                part_start = comma + 1;
            }

            if (parts.size() >= 4) {
                string id = parts[0];
                string value = parts[1];
                bool is_deleted = parts[2] == "1";
                string prev_id = parts[3];

                map<char, int> node_vv;
                for (size_t i = 4; i < parts.size(); i++) {
                    string vv_part = parts[i];
                    size_t colon = vv_part.find(':');
                    if (colon != string::npos) {
                        char client = vv_part[0];
                        int seq = stoi(vv_part.substr(colon + 1));
                        node_vv[client] = seq;
                    }
                }

                RGA_Node node(id, value, node_vv, prev_id);
                node.is_deleted = is_deleted;
                nodes.push_back(node);
                id_to_index[id] = nodes.size() - 1;
            }

            node_pos = node_end + 1;
        }
    }
};



#endif
