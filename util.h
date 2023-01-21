//
// util.h
//
// Program file compression project that takes a file and uses Huffman encoding technique to compress files to take up less memory. This technique uses calculates the frequency of characters in a file and using that frequency, the Huffman encoding is decided. Using a priority queue, we sort each character based on frequency and then use that to create our encoding data stuctures. Then based on the encoding we create our Huffman encoding based on each character and create our compress file. Decompression is also allowed here based on giving the compression file our frequency map to be able to decompress the compressed file.
//
// Course: CS 251, Fall 2022, Wed 10am Lab
// Author: Ali Abuzir
// System: macOS
//

#pragma once

//
// *Necessary libraries included here.
//
#include "hashmap.h"
#include "bitstream.h"
#include <unordered_map>
#include <fstream>
#include <queue>

typedef hashmap hashmapF; // defines a synonym for hashmap
typedef unordered_map <int, string> hashmapE; // defines a synonym for unordered_map<int, string>

//
// *Struct to hold each character and its frequency and children in encoded tree.
//
struct HuffmanNode {
  int character;
  int count;
  HuffmanNode* zero;
  HuffmanNode* one;
};

//
// *Compare struct that acts as a compare function (functor) uses in priority queue to sort each character based on frequency.
//
struct compare
{
  bool operator()(const HuffmanNode *lhs,
    const HuffmanNode *rhs)
  {
    return lhs->count > rhs->count;
  }
};

//
// *This method frees the memory allocated for the Huffman tree.
//
void freeTree(HuffmanNode* node) {
    
  if (node == nullptr) {
    return;
  }
  else {
    freeTree(node->zero);
    freeTree(node->one);
    delete node;
    node = nullptr;
  } 
}

//
// *This function build the frequency map.  If isFile is true, then it reads
// from filename.  If isFile is false, then it reads from a string filename.
// Frequency map is used to count number of occurences of each character in frequency map.
//
void buildFrequencyMap(string filename, bool isFile, hashmapF &map) {
    
  if (isFile) { // if filename represents a file
    ifstream infile(filename); // opens file

    if (infile.is_open()) {
      char grab;
      while (infile.get(grab)) { // gets each character in file
        int asciiGrab = static_cast<int>(grab); // converts char to int
        if (map.containsKey(asciiGrab)) { // chekcs if character already exists in hashmap
          int currValue = map.get(asciiGrab); // gets character's value in hashmap if it already exists
          map.put(asciiGrab, currValue + 1); // increments character frequency value in hashmap
        }
        else { // if character is not in frequency map
          map.put(asciiGrab, 1);
        }
      }
      map.put(PSEUDO_EOF, 1); // adds the eof character to frequency map
    }
  }
    
  else { // if filename is a string
    for (char currChar : filename) { // loops through each character in string
      int asciiGrab = static_cast<int>(currChar); // converts char to int
      if (map.containsKey(asciiGrab)) { // checks to see if character exists in hashmap
        int currValue = map.get(asciiGrab); // grabs the character's value in hashmap if it already exists
        map.put(asciiGrab, currValue + 1); // increments character frequency value in hashmap
      }
      else { // if character is not in frequency map
        map.put(asciiGrab, 1);
      }
    }
    map.put(PSEUDO_EOF, 1); // adds the eof character to frequency map
  }
}

//
// *This function builds an encoding tree from the frequency map.
//
HuffmanNode* buildEncodingTree(hashmapF &map) {
    
  priority_queue<HuffmanNode*, vector<HuffmanNode*>, compare> pqueue; // creates a priority queue instance that stores pushed HuffamNode* elements in a vector and compares them with the compare functor to sort them based on count (frequency)

  for (auto &eachChar : map.keys()) { // loops through each character in the hashmap
    HuffmanNode *newHuffman = new HuffmanNode; // allocates a new HuffmanNode in memory to hold character and frequency with points to 
    newHuffman->character = eachChar;
    newHuffman->count = map.get(eachChar); // frequency
    newHuffman->zero = nullptr;
    newHuffman->one = nullptr;

    pqueue.push(newHuffman); // pushes node into priority queue
  }

  while (pqueue.size() > 1) {
    HuffmanNode *first = pqueue.top();
    pqueue.pop();
    HuffmanNode *second = pqueue.top();
    pqueue.pop();

    HuffmanNode *newHuffman = new HuffmanNode; // allocates a new HuffmanNode in memory to hold NOT_A_CHAR in order to get Huffman encoding 
    newHuffman->character = NOT_A_CHAR;
    newHuffman->count = first->count + second->count; // sum of first two popped node frequencies from priority queue
    newHuffman->zero = first; // points to first popped node
    newHuffman->one = second; // points to second popped node

    pqueue.push(newHuffman); // pushes new node to priority queue
  }

  return pqueue.top(); // returns root of encoding tree
}

//
// *Recursive helper function for building the encoding map using in order recursion.
//
void _buildEncodingMap(HuffmanNode* node, hashmapE &encodingMap, string str,
                       HuffmanNode* prev) {
  
  if (node == nullptr) {
    return;
  }
  else {
    _buildEncodingMap(node->zero, encodingMap, str+'0', node); // recursively calls left (zero) and adds 0 to str to get huffman encoding for each character (prev is called using node since node is going to be the next prev after the call is executed)
    if (prev->character == NOT_A_CHAR && node->character != NOT_A_CHAR) {
      encodingMap.emplace(node->character, str);
    }
    _buildEncodingMap(node->one, encodingMap, str+'1', node); // recursively calls right (one) and adds 1 to str to get huffman encoding for each character (prev is called using node since node is going to be the next prev after the call is executed)
  }
}

//
// *This function builds the encoding map from an encoding tree using recursive helper function above.
//
hashmapE buildEncodingMap(HuffmanNode* tree) {
   
  hashmapE encodingMap;

  string eachValue;
  _buildEncodingMap(tree, encodingMap, eachValue, tree); // builds encoding map
    
  return encodingMap; // returns unordered map of each character and its huffman encoding
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
  
  string bitString;
  
  if (makeFile) {
    char getChar;
    int charToInt;
    while (input.get(getChar)) { // gets each character from input file
      charToInt = static_cast<int>(getChar); // converts from char to int to check in map as each character is its ASCII value in map
      if (encodingMap.count(getChar)) { // checks if character exists in map
        bitString += encodingMap.at(charToInt); // adds huffman encoding to bitString
      }
    }

    if (encodingMap.count(PSEUDO_EOF)) { // checks if eof character exists in encoding map
      bitString += encodingMap.at(PSEUDO_EOF); // adds huffman encoding for eof to bitString
    }

    for (auto &eachChar : bitString) { // loops through bitString and writes bit to out
      charToInt = static_cast<int>(eachChar);
      if (charToInt == 48) { // since '0' is 48 as an int but we want to write 0 as the int itself
        output.writeBit(0);
      }
      else { // we could either have 0 or 1, so we write 1 as an int as '1' as an int is 49
        output.writeBit(1);
      }
      ++size; // increments size
    }
  }
  return bitString; // returns bitSting to help with testing
}


//
// *This function decodes the input stream and writes the result to the output
// stream using the encodingTree.  This function also returns a string
// representation of the output file, which is particularly useful for testing.
//
string decode(ifbitstream &input, HuffmanNode* encodingTree, ofstream &output) {

  HuffmanNode *curr = encodingTree; // creates an iterate pointer curr to go through nodes in our encoding tree without losing the access to the root of the tree which has access to the whole tree. Losing the root will cause memory leaks
  string decodedString;
  
  while (!input.eof()) { // iterates through input bit stream until we get to the end of the stream
    int bit = input.readBit(); // gets each bit from input bit stream

    if (bit == 0) {
      curr = curr->zero; // goes to the left (zero) child in encoding tree to find character associated with bits
    }
    else {
      curr = curr->one; // goes to the right (one) child in encoding tree to find character associated with bits
    }

    if (curr != nullptr) {
      if (curr->character != NOT_A_CHAR && curr->character != PSEUDO_EOF) { // as long as character in tree is not the NOT_A_CHAR character then we put that character into our output stream;
        output.put(curr->character);
        decodedString += curr->character; // adds character to test string decodedString
        curr = encodingTree; // restarts at the root
      }
    }
    else { // if curr ever does equal nullptr, we break as we got all the characters and the input bit stream has not come to an end yet and/or it can mean we didn't perform our encoding properly
      break;
    }
  }
  
  return decodedString; // returns the test decoded string for testing
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
  bool isFile = true;
  
  HuffmanNode *encodingTree = nullptr;
  hashmapE encodingMap;
  
  ifstream input(filename);
  ofbitstream output(filename+".huf"); // filename + ".huf" creates a file with filename with .huf appened to it with our encoding file contents
  int size = 0;
  bool makeFile = true;
  string bitString;
  
  buildFrequencyMap(filename, isFile, frequencyMap); // builds our frequency map

  encodingTree = buildEncodingTree(frequencyMap); // builds our encoding tree and assigns it to our encodingTree node

  encodingMap = buildEncodingMap(encodingTree); // builds our encoding map and assigns it to out encodingMap map

  if (input.is_open()) {
    output << frequencyMap; // adds frequency map to compressed file in order to get frequencies of characters to decode if desired
    bitString = encode(input, encodingMap, output, size, makeFile); // encodes our file (compresses) and returns a string of the file encoded with huffman encoding
  }

  freeTree(encodingTree); // frees memory associated with encoding tree
  
  return bitString; // returns the huffman encoding string of the encoded file
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
    
  ifbitstream input(filename); // input bitstream to receive compressed file
  stringstream parseFilename(filename); // used to parse filename to include _unc
  string decompFile;

  getline(parseFilename, decompFile, '.'); // parses the string before the first dot so in example.txt.huf it will get example and place it int decompFile
  decompFile += "_unc.txt"; // adds _unc.txt to example to get example_unc.txt which is what we want the decompressed file name to be
  
  ofstream output(decompFile); // creates an output file stream and titles it the decompFile name
  hashmapF frequencyMap;

  input >> frequencyMap; // grabs the frequency map from the file as we put it there and uses it to decode our compressed file

  HuffmanNode *encodingTree = nullptr;
  encodingTree = buildEncodingTree(frequencyMap); // builds the encoding tree out of the frequency file to understand each bit and what character it leads to

  string decodedString;
  decodedString = decode(input, encodingTree, output); // decodes the compressed file using the encoding tree and assigns it to decodedString as well as the output file stream

  freeTree(encodingTree); // frees memory associated with encoding tree

  return decodedString; // returns a string of your decompressed file
}