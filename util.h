//
// STARTER CODE: util.h
//
// TODO:  Write your own header
//
#include<fstream>
#include<queue>
#include<vector>
#pragma once
#include "hashmap.h"
#include <map>
#include <unordered_map>
#include "bitstream.h"

typedef hashmap hashmapF;
typedef unordered_map <int, string> hashmapE;

struct HuffmanNode {
    int character;
    int count;
    bool sum;  // to distinguish nodes with char values
                // and nodes that hold the addition between 2 children
    int order;
    HuffmanNode* zero;
    HuffmanNode* one;
};

// recursive helper for freeTree()
void _freeTree(HuffmanNode* node) {
    if (node == nullptr) {
        return;
    }
    _freeTree(node->zero);
    _freeTree(node->one);
    delete node;
}


//
// *This method frees the memory allocated for the Huffman tree.
//
void freeTree(HuffmanNode* node) {
    _freeTree(node);
}

//
// *This function build the frequency map.  If isFile is true, then it reads
// from filename.  If isFile is false, then it reads from a string filename.
//
void buildFrequencyMap(string filename, bool isFile, hashmapF &map) {
    if (isFile == true) {
        // reading from a file
        ifstream infile(filename);
        char curr;
        while (infile.get(curr)) {
            if (map.containsKey(curr)) {
                // get the current value and add one
                int newVal;
                newVal = map.get(curr) + 1;
                map.put(curr, newVal);
            } else {
                // set the initial value to one
                map.put(curr, 1);
            }
        }
    } else {
        // reading from a string
        for (size_t i = 0; i < filename.size(); i++) {
            if (map.containsKey(filename[i])) {
                // get the current value and add one
                int newVal;
                newVal = map.get(filename[i]) + 1;
                map.put(filename[i], newVal);
            } else {
                // set the initial value to one
                map.put(filename[i], 1);
            }
        }
    }
    // add the end of file char
    map.put(PSEUDO_EOF, 1);
}

class prioritize {
    public:
    bool operator() (const HuffmanNode* p1, const HuffmanNode* p2) {
        // if they are equal, we must go by insertion order.
        if (p1->count == p2->count) {
            return p1->order > p2->order;
        }
        return p1->count > p2->count;
    }
};

void copyHuffmanNode(HuffmanNode* node, HuffmanNode other) {
    node->character = other.character;
    node->count = other.count;
    node->zero = other.zero;
    node->one = other.one;
}

HuffmanNode* constructTree(priority_queue<HuffmanNode*, vector<HuffmanNode*>, prioritize> &pQueue) {
    int counter = pQueue.size();
    while (pQueue.size() > 1) {
        // pop two nodes from the queue
        HuffmanNode* temp1;
        HuffmanNode* temp2;
        temp1 = pQueue.top();
        pQueue.pop();
        temp2 = pQueue.top();
        pQueue.pop();
        HuffmanNode* root = new HuffmanNode;
        root->sum = true;
        root->zero = temp1;
        root->one = temp2;
        root->count = temp1->count + temp2->count;
        root->order = counter++;
        root->character = NOT_A_CHAR;
        pQueue.push(root);
    }
    return pQueue.top();
}


//
// *This function builds an encoding tree from the frequency map.
//
HuffmanNode* buildEncodingTree(hashmapF &map) {
    priority_queue<HuffmanNode*, vector<HuffmanNode*>, prioritize> pQueue;
    vector<int> keys = map.keys();

    // build the nodes and put into the priority queue.
    int counter = 0;
    for (int x : keys) {
        HuffmanNode* newNode = new HuffmanNode;
        newNode->character = x;
        newNode->count = map.get(x);
        newNode->zero = nullptr;
        newNode->one = nullptr;
        newNode->sum = false;
        newNode->order = counter;
        counter++;
        pQueue.push(newNode);
    }
    // Creating the tree from the priority queue
    HuffmanNode* root = constructTree(pQueue);
    // freeing the priority queue.
    return root;
}

//
// *Recursive helper function for building the encoding map.
//
void _buildEncodingMap(HuffmanNode* node, hashmapE &encodingMap, string str,
                       HuffmanNode* prev) {
    if (node == nullptr) {
        return;
    }
    // append to the string the current bit.
    // if the node == previous's right node, add 0, else 1
    if (prev != nullptr && node == prev->zero) {
        str.append("0");
    } else if (prev != nullptr) {
        str.append("1");
    }
    prev = node;
    // Travel to the leftmost node
    _buildEncodingMap(node->zero, encodingMap, str, prev);
    // add the current string path into the map along with character.
    if (node->character != NOT_A_CHAR) {
        encodingMap.emplace(node->character, str);
    }
    // traverse to the right tree
    _buildEncodingMap(node->one, encodingMap, str, prev);
}

//
// *This function builds the encoding map from an encoding tree.
//
hashmapE buildEncodingMap(HuffmanNode* tree) {
    hashmapE encodingMap;
    // check to see if the tree is nullptr
    if (tree == nullptr) {
        return encodingMap;
    }
    // do in order traversal
    // Everytime we move to the left or right, add the bit to a string
    // if we encounter a leaf add the char and current string to the map
    string str = "";
    HuffmanNode* prev = nullptr;
    _buildEncodingMap(tree, encodingMap, str, prev);
    return encodingMap;
}


// Writes to the output stream if makefile is true,
// write to the return string for encode()
void _encode(char &bit, ofbitstream& output, bool &makeFile, int &size) {
    if (makeFile) {  // check if we have to write to output stream
        // write the bit to the stream
        if (bit == '1') {
            output.writeBit(1);
        } else {
            output.writeBit(0);
        }
    }
}

//
// *This function encodes the data in the input stream into the output stream
// using the encodingMap.  This function calculates the number of bits
// written to the output stream and sets result to the size parameter, which is
// passed by reference.  This function also returns a string representation of
// the output file, which is particularly useful for testing.
//
string encode(ifstream& input, hashmapE &encodingMap, ofbitstream& output,
              int &size, bool makeFile) {
    // read the file character by character
    // match the character with the map
    // write the binary value to the output stream
    // write the PSEUDO_EOF bit to the end
    size = 0;
    string returnStr = "";
    char character;
    // read the file character by character
    while (input >> noskipws >> character) {
        // convert the character in the map to the value associated
        // and write bit by bit
        for (char bit : encodingMap[character]) {
            _encode(bit, output, makeFile, size);
            size++;
        }
        returnStr.append(encodingMap[character]);
    }
    // Write the EOF values
    for (char bit : encodingMap[PSEUDO_EOF]) {
        _encode(bit, output, makeFile, size);
        size++;
    }
    returnStr.append(encodingMap[PSEUDO_EOF]);
    return returnStr;
}


//
// *This function decodes the input stream and writes the result to the output
// stream using the encodingTree.  This function also returns a string
// representation of the output file, which is particularly useful for testing.
//
string decode(ifbitstream &input, HuffmanNode* encodingTree, ofstream &output) {
    // read the characters one at at time from the input
    //       traverse the tree with each bit until we hit a leaf node
    //       return to the root
    //       repeat until the 011 or EOF is read and stop reading
    HuffmanNode* curr = encodingTree;  // used to traverse the tree
    int bit;
    string tempS;
    string outputStr = "";
    while (input) {
        bit = input.readBit();
        // check if we are at a leaf node, otherwise continue traversing
        if (curr->zero == nullptr && curr->one == nullptr) {
            if (curr->character == PSEUDO_EOF) {
                break;
            }
            tempS = curr->character;
            output << tempS;
            outputStr += curr->character;
            // return the curr back to the root
            curr = encodingTree;
        }
        // traverse the tree
        if (bit == 1) {
            curr = curr->one;
        } else if (bit == 0) {
            curr = curr->zero;
        } else {
            cout << "something went wrong!" << endl;
            cout << "Bit: " << bit << endl;
        }
    }
    return outputStr;
}

//
// *This function completes the entire compression process.  Given a file,
// filename, this function (1) builds a frequency map; (2) builds an encoding
// tree; (3) builds an encoding map; (4) encodes the file (don't forget to
// include the frequency map in the header of the output file).  This function
// should create a compressed file named (filename + ".huf") and should also
// return a string version of the bit pattern.
//
string compress(string filename) {
    hashmapF frequencyMap;
    HuffmanNode* encodingTree = nullptr;
    hashmapE encodingMap;
    bool isFile = true;
    int size;

    // step 1 build frequencyMap
    buildFrequencyMap(filename, isFile, frequencyMap);
    // step 2 encode the tree
    encodingTree = buildEncodingTree(frequencyMap);
    // step 3 build the encoding map
    encodingMap = buildEncodingMap(encodingTree);
    // step4 encode the text
    ofbitstream output(filename + ".huf");
    ifstream input(filename);
    stringstream ss;
    ss << frequencyMap;
    output << frequencyMap;
    size = 0;
    string codeStr = encode(input, encodingMap, output, size, true);
    output.close();
    freeTree(encodingTree);
    return codeStr;  // TO DO: update this return
}

//
// *This function completes the entire decompression process.  Given the file,
// filename (which should end with ".huf"), (1) extract the header and build
// the frequency map; (2) build an encoding tree from the frequency map; (3)
// using the encoding tree to decode the file.  This function should create a
// compressed file using the following convention.
// If filename = "example.txt.huf", then the uncompressed file should be named
// "example_unc.txt".  The function should return a string version of the
// uncompressed file.  Note: this function should reverse what the compress
// function did.
//
string decompress(string filename) {
        size_t pos = filename.find(".huf");
        if ((int)pos >= 0) {
            filename = filename.substr(0, pos);
        }
        pos = filename.find(".");
        string ext = filename.substr(pos, filename.length() - pos);
        filename = filename.substr(0, pos);
        ifbitstream input(filename + ext + ".huf");
        ofstream output(filename + "_unc" + ext);

        hashmapF dump;
        input >> dump;

        // build the map
        HuffmanNode* encodingTree;
        encodingTree = buildEncodingTree(dump);
        string decodeStr  = decode(input, encodingTree, output);
        output.close();  //  must close file so autograder can open for testing
    return decodeStr;
}
