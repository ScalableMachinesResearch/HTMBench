#include "avl.h"
#include <vector>
#include <cstdlib>
#include <cstring>
#include <omp.h>
#include <stdlib.h>
#include <random>
#include <iostream>



#define MAX_VALUE 2147483647
#define MIN_VALUE -2147483648


int cmp_func(AVLTreeKey k1, AVLTreeKey k2)
{
    return k1 - k2;
}

AVLTree *g_tree = avl_tree_new(cmp_func);
int g_find_num;


typedef struct Instruction {
	char action;
	int value;
} Instruction_t;

std::default_random_engine generator;
std::uniform_int_distribution<int> distribution_value;
std::uniform_int_distribution<int> distribution_action(1, 10);
#ifdef OMP_LOCK
omp_lock_t global_lock;
#endif

int get_value(){
	 return distribution_value(generator);
}
Instruction_t get_instruction(){
	Instruction_t ret_ins;
	ret_ins.value = get_value();
	int num = distribution_action(generator);
	if (num <= g_find_num ) {
		ret_ins.action = 'F';
	}
	else if (num <= (5+g_find_num/2)) {
		ret_ins.action = 'I';
	}
	else {
		ret_ins.action = 'R';
	}
	return ret_ins;
}

AVLTreeNode * my_avl_insert(int value) {
  AVLTreeNode * node_tmp;
  MY_TM_BEGIN(0);
  node_tmp = avl_tree_insert(g_tree, value, 0);
  MY_TM_END(0);
  return node_tmp;
}
AVLTreeNode * my_avl_search(int value) {
  AVLTreeNode * node_tmp;
  MY_TM_BEGIN(0);
  node_tmp = avl_tree_lookup_node(g_tree, value);
  MY_TM_END(0);
  return node_tmp;
}
void my_avl_remove(int value){
  MY_TM_BEGIN(0);
  avl_tree_remove(g_tree, value);
  MY_TM_END(0);
}

int main( int argc, char **argv ) {
	if (argc < 6) {
		std::cout << "Invalid Input" << std::endl;
		std::cout <<"Usage: " << argv[0] <<" [num_of_threads] [Key_range_size] [Tree_initial_ratio] [Find_operation_ratio] [Num_of_instructions_per_thread]" << std::endl;
		return 0;
	}

	int num_threads = atoi(argv[1]);
  int key_range_size = atoi(argv[2]);
  distribution_value= std::uniform_int_distribution<int>((-1/2)*key_range_size, (1/2)*key_range_size-1);
	int initial_tree_size = atof(argv[3]) * key_range_size;
  g_find_num = atof(argv[4]) * 10;
	int num_ins_per_thread = atoi(argv[5]);
  std::cout <<"initial_tree_size = " << initial_tree_size <<std::endl;
  std::cout <<"g_find_num = " << g_find_num << std::endl;
#ifdef RTM
	TM_STARTUP(num_threads);
#endif
#ifdef OMP_LOCK
        omp_init_lock(&global_lock);
#endif
	//construct the tree

	for(int i = 0; i < initial_tree_size; i++){
		my_avl_insert(get_value());
	}
	std::cout << "AVL Tree Initialized" << std::endl;
  std::cout << "we are going to loop " << num_ins_per_thread <<std::endl;
	//launch the work
	omp_set_num_threads(num_threads);
#pragma omp parallel
{
	int id = omp_get_thread_num();
	std::vector<int> counter(3);
	for(int i=0; i < num_ins_per_thread; i++){
		Instruction_t &&ins_tmp = get_instruction();
		//if (id == 0) std::cout << ins_tmp.action <<":" << ins_tmp.value << std::endl;
		bool find_result;
		if (ins_tmp.action == 'F') {
  		my_avl_search(ins_tmp.value);
  		counter[0]++;
		}
		else if (ins_tmp.action == 'I') {
      my_avl_insert(ins_tmp.value);
      counter[1]++;
		}
    else if (ins_tmp.action == 'R') {
      my_avl_remove(ins_tmp.value);
      counter[2]++;
		}
		//if there is something else, do nothing..
		//if (id == 0) avl_dump(g_tree);
	}
	std::cout << "Thread " << id <<" finished: " << counter[0] << " " << counter[1] << " " << counter[2] << std::endl;
}

	//destroy the tree
	avl_tree_free(g_tree);
#ifdef RTM
  TM_SHUTDOWN();
#endif
#ifdef OMP_LOCK
        omp_destroy_lock(&global_lock);
#endif
	return 0;
}
