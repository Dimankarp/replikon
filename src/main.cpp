#include "chat_tree.h"
#include "crdts/register.h"
#include <iostream>

int main() {
  replikon::ChatTree<int> tree;
  tree.GetHeader();
  replikon::crdt::Register<std::string> reg;
  reg.Merge("Me!");
  std::cout << "Hello " << reg.GetUpdate({}) << "\n";
}