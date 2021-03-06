//===- LoopParallelization.cpp - Example code from "Writing an LLVM Pass" ---------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements two versions of the LLVM "LoopParallelization World" pass described
// in docs/WritingAnLLVMPass.html
//
//===----------------------------------------------------------------------===//

#include "llvm/ADT/PostOrderIterator.h"
#include "llvm/ADT/SetOperations.h"
#include "llvm/ADT/StringMap.h"
#include "llvm/ADT/DenseSet.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/CFG.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Analysis/CFG.h"

using namespace llvm;

#define DEBUG_TYPE "cs255"
#define KEY_LEN 256
#define SMALL_SIZE 32

StringMap<DenseSet<StringRef> *> dominators = StringMap<DenseSet<StringRef> *>();
DenseSet<std::pair<StringRef, StringRef>> back_edges = DenseSet<std::pair<StringRef, StringRef>>();
StringMap<BasicBlock *> bb_by_name = StringMap<BasicBlock *>();
auto natural_loops = DenseMap<std::pair<StringRef, StringRef>, DenseSet<StringRef> *>();

void dump_dominance( ) {

	/* Loop through all the basic blocks (all should be in the dominators relation struct) */
	errs() << "Dominance Relation:\n";
	for ( StringMap<DenseSet<StringRef> *>::iterator bb_dom = dominators.begin(),
			  bb_end = dominators.end(); bb_dom != bb_end; ++bb_dom ) {
		DenseSet<StringRef> *dom_set = bb_dom->getValue();
		for ( DenseSet<StringRef>::iterator curr_dominator = dom_set->begin(),
				  dom_end = dom_set->end();
			  curr_dominator != dom_end; ++curr_dominator ) {
			/* For each block that dominates the current block */
			/* Output Dominance Relationship */
			errs() << curr_dominator->str() << " -> " << bb_dom->getKey() << ", ";
		}
		errs() << "\n";
	}
	return;
}

void dump_back_edges( ) {
	
	/* Loop through all the set of back edges */
	for ( DenseSet<std::pair<StringRef, StringRef>>::iterator back_edge = back_edges.begin(),
			  be_end = back_edges.end(); back_edge != be_end; ++back_edge ) {
		/* Print the Edge */
		errs() << "Back Edge: "
			   << back_edge->first << " -> " << back_edge->second << "\n";
	}
}

void dump_natural_loops( ) {

	/* Loop through all the back edges with their corresponding natural loops */
	for ( DenseMap<std::pair<StringRef, StringRef>, DenseSet<StringRef> *>::iterator natural_loop = natural_loops.begin(), nl_end = natural_loops.end(); natural_loop != nl_end; ++natural_loop ) {
		/* Print the Edge */
		std::pair<StringRef, StringRef> back_edge = natural_loop->getFirst();
		DenseSet<StringRef> *loop_set = natural_loop->getSecond();
		errs() << "Back Edge: "
			   << back_edge.first << " -> " << back_edge.second << "\n";
		/* Print the Natural Loop Elements */
		errs() << "Corresponding Blocks in the loop: ";
		for ( auto name = loop_set->begin(), ne = loop_set->end(); name != ne; ++name ) {
			errs() << *name << ", ";
		}
		errs() << "\n";
	}
}

void dump_loop_parallelization_analysis( ) {
	errs() << "Number of Loops: "
		   << std::to_string( natural_loops.size() )
		   << "\n";
	dump_dominance();
	dump_natural_loops();

}
DenseSet<StringRef> *get_full_bb_set( Function &F ) {

	/* Get a set containing all the BBs */
	DenseSet<StringRef> *all_bb = new DenseSet<StringRef>();
	int bb_counter = 0;
	for ( Function::iterator BB = F.begin(), FE = F.end(); BB != FE; ++BB ) {
		BB->setName( "BB" + std::to_string(bb_counter++) );
		StringRef bb_name = BB->getName();
		/* Insert new name into all BBs set */
		all_bb->insert( bb_name );
		StringMapEntry<BasicBlock *> *name_to_bb;
		name_to_bb = StringMapEntry<BasicBlock *>::Create( bb_name, &(*BB) );
		bb_by_name.insert( name_to_bb );
	}

	return all_bb;
}

void init_dom( Function &F, DenseSet<StringRef> *all_bb ) {
	
	/* Initialize Dominator Sets */
	/* Special init for entry block */
	BasicBlock &entry = F.getEntryBlock();
	StringRef entry_name = entry.getName();
	StringMapEntry<DenseSet<StringRef> *> *entry_dom;
	entry_dom = StringMapEntry<DenseSet<StringRef> *>::Create( entry_name, new DenseSet<StringRef>() );
	DenseSet<StringRef> *entry_dom_set = entry_dom->getValue();
	entry_dom_set->insert( entry_name );

	/* Initialize Remaining BBs DOM sets to sets of all nodes */
	for ( Function::iterator BB = F.begin(), FE = F.end(); BB != FE; ++BB ) {
		/* For each basic block */
		/* Get it's name */
		StringRef *bb_name = new StringRef( BB->getName().str() );

		/* Set the DOM values to the full set */
		StringMapEntry<DenseSet<StringRef> *> *dom;
		dom = StringMapEntry<DenseSet<StringRef> *>::Create( *bb_name, new DenseSet<StringRef>(*all_bb) );
		dominators.insert( dom );
	}
}

int set_compare( DenseSet<StringRef> set1,
				 DenseSet<StringRef> set2 ) {

	for ( DenseSet<StringRef>::iterator I = set1.begin(), E = set1.end();
		  I != E; ++I ) {
		if ( set2.count(*I) == 0 ) { return 1; } /* If something is in set1, and not set 2 */
	}
	for ( DenseSet<StringRef>::iterator I = set2.begin(), E = set2.end();
		  I != E; ++I ) {
		if ( set1.count(*I) == 0 ) { return 1; } /* If something is in set2, and not set1 */
	}
	/* If both match, return 0 */
	return 0;
}

DenseSet<StringRef> *get_dominating_set( BasicBlock &BB ) {

	DenseSet<StringRef> *dom_cap = new DenseSet<StringRef>();
	DenseSet<StringRef> *self_set = new DenseSet<StringRef>();

	/* Get the Super Set of all involved predecessors' dominators */
	for (BasicBlock *pred : predecessors( &BB ) ) {
		StringRef bb_name = pred->getName();
		set_union( *dom_cap, *dominators.lookup( bb_name ) );
	}
	/* Compute the Intersection of all the predecessors' dominators */
	for (BasicBlock *pred : predecessors( &BB ) ) {
		StringRef bb_name = pred->getName();
		set_intersect( *dom_cap, *dominators.lookup( bb_name ) );
	}
	/* Add self to DOM set */
	self_set->insert( BB.getName() );
	set_union( *dom_cap, *self_set );

	return dom_cap;
}

void iterative_dominance_solver(Function &F) {
	errs() << "Function: " << F.getName() << "\n";

	/* Get set of basic blocks */
	DenseSet<StringRef> *all_bb = get_full_bb_set( F );
	/* Set the DOM set for all blocks to full set */
	init_dom( F, all_bb );
	
	/* Begin iterative algorithm */
	bool changed = true;
	while ( changed ) {
		changed = false;

		/* Loop through each of the blocks */
		for ( auto &BB : F ) {
			//for ( Function::iterator BB = F.begin(), FE = F.end(); BB != FE; ++BB ) {
			/* Calculate the next iteration of the dominating set */
			StringRef bb_name = BB.getName();
			DenseSet<StringRef> *tmp = get_dominating_set( BB );
			/* Compare the new iteration set to the old */
			/* If they match, a fixed point has been found, break */
			if ( set_compare( *tmp, *(dominators.lookup( bb_name )) ) == 0 ) { /* Match == 0 */
				break;
			} else { /* Otherwise, update with the new DOM set */
				dominators.erase( bb_name );
				dominators.insert( std::make_pair( bb_name, tmp ) );
				changed = true;
			}
		}
	}
}

void calculate_back_edges( Function &F ) {

	/* A back edge is an arc whose head dominates its tail */
	/* Loop through all the dominance relationships */
	for ( StringMap<DenseSet<StringRef> *>::iterator bb_dom = dominators.begin(),
			  bb_end = dominators.end(); bb_dom != bb_end; ++bb_dom ) {
		/* For each BB that dominates the current */
		DenseSet<StringRef> *dom_set = bb_dom->getValue();
		for ( DenseSet<StringRef>::iterator curr_dominator = dom_set->begin(),
				  dom_end = dom_set->end();
			  curr_dominator != dom_end; ++curr_dominator ) {
			/* Check if the dominating BB has a (predecessor) edge coming from the dominatee */
			BasicBlock *dominatee = bb_by_name[bb_dom->getKey()];
			BasicBlock *dominator = bb_by_name[*curr_dominator];
			/* In other words, check if the the dominatee is a predecessor of the dominator */
			for ( BasicBlock *pred : predecessors( dominator ) ) {
				if ( pred == dominatee ) {
					/* Found a back edge */
					auto back_edge = std::make_pair( dominatee->getName(),
													  dominator->getName() );
					back_edges.insert( back_edge );
				}
			}
		}			
	}
}

int dfs_rec( DenseSet<BasicBlock *> *visited, BasicBlock *start, BasicBlock *target, BasicBlock *ignore=nullptr ) {
	if ( start == target ) { return 1; }
	if ( target == ignore ) { return 0; }
	for ( BasicBlock *BB : successors( start ) ) {
		if ( BB == ignore ) { continue; }
		if ( BB == target ) { return true; }
		if ( visited->count( BB ) != 0 /* BB has been visited */ ) { continue; }
		else { visited->insert( BB ); }; /* Add BB to set of visited blocks */
		if ( dfs_rec( visited, BB, target, ignore ) ) { return 1; }
	}
	return 0;
}
int dfs( BasicBlock *start, BasicBlock *target, BasicBlock *ignore=nullptr ) {
	if ( (start == nullptr) || (target == nullptr) ) { return 0; }
	DenseSet<BasicBlock *> *visited = new DenseSet<BasicBlock *>();
	int result = dfs_rec( visited, start, target, ignore );
	delete visited;
	return result;
}

void find_natural_loops( Function &F ) {

	/* Check all back edges for corresponding natural loops */
	for ( DenseSet<std::pair<StringRef, StringRef>>::iterator back_edge = back_edges.begin(),
			  be_end = back_edges.end(); back_edge != be_end; ++back_edge ) {
		BasicBlock *h = bb_by_name[back_edge->second];
		BasicBlock *t = bb_by_name[back_edge->first];
		DenseSet<StringRef> *natural_loop_set = new DenseSet<StringRef>();
		/* For each BB in the CFG, check if it can reach 't' */
		for ( Function::iterator BB = F.begin(), FE = F.end(); BB != FE; ++BB ) {
			if ( dfs( &(*BB), t, h ) ) {
				natural_loop_set->insert( BB->getName() );
			}
			// if ( (&(*BB) == h) || (&(*BB) == t) ) { continue; }
			// /* If it is reachable, */
			// if ( isPotentiallyReachable( &(*BB), t ) ) {
			// 	/* Add to set of nodes in natural loop */
			// 	natural_loop_set->insert( BB->getName() );
			// }
			// errs() << "PO Iterating: ";
			// ReversePostOrderTraversal<Function *> RPOT( &F );
			// for ( auto I = RPOT.begin(); I != RPOT.end(); ++I ) {
			// 	/* If we reach 't', add this block to the set */
			// 	errs() << (*I)->getName() << ", ";
			// 	if ( *I == t ) {
			// 		natural_loop_set->insert( (*I)->getName() );
			// 	}
			// }
			// errs() << "\n";
		}
		/* Add 'h' and 't' to the set of BBs in the natural loop */
		natural_loop_set->insert( back_edge->second );
		natural_loop_set->insert( back_edge->first );

		natural_loops.insert( std::make_pair( *back_edge, natural_loop_set ) );
	}
}

/* The following code counts and reports the number of static 
 * statements in every basic block. */
void  count_static_stmts(Function &F){
	errs() <<  "Function: " << F.getName() << "\n";

	int bb_counter = 0;
	for (Function::iterator BB = F.begin(), FE = F.end(); BB != FE; ++BB) {	
		int stmt_counter = 0;
		for (BasicBlock::iterator II = BB->begin(), BBE = BB->end(); II != BBE; ++II) {	
			if(!isa<AllocaInst>(II) && !isa<PHINode>(II))
				stmt_counter++;
		}
		errs() << "\tNo. " << bb_counter << "bb: " << stmt_counter << " statements.\n";
		bb_counter++;
	}
}

/* The following code transforms the program to count
 * the number of dynamic basic block executions.  
 * TODO You need to transform the program to
 * count the number of dynamic statement executions.
 * You can base you work on the following code. */

void count_dynamic_bbs(Function &F){

	Module *M = F.getParent();
	for (Function::iterator BB = F.begin(), E = F.end(); BB != E; ++BB) {	
		BasicBlock::iterator InsertPos = BB->begin();
		while (InsertPos!=BB->end() && (isa<AllocaInst>(InsertPos) || isa<PHINode>(InsertPos)))
			InsertPos++;
		LLVMContext &Context = F.getContext();
		Type *VoidTy = Type::getVoidTy(Context);
		Constant *IncFn = M->getOrInsertFunction("increase", VoidTy, (Type *)0);
		if(InsertPos==BB->end())
			CallInst::Create(IncFn,  "", &*BB);
		else
			CallInst::Create(IncFn,  "", &*InsertPos);
	}


}

/* You need to transform the main function to Call init()
 * at the entry of the program. */

void add_init_call(Function &F){
	if(F.getName().equals("main")){
		/* TODO */
	}


}


namespace {
	// LoopParallelization - The first implementation, without getAnalysisUsage.
	struct LoopParallelization : public FunctionPass {
		static char ID; // Pass identification, replacement for typeid
		LoopParallelization() : FunctionPass(ID) {}

		bool runOnFunction(Function &F) override {
			//count_static_stmts(F);
			//count_dynamic_bbs(F);
			iterative_dominance_solver(F);
			calculate_back_edges(F);
			find_natural_loops(F);
			dump_loop_parallelization_analysis();
			add_init_call(F);
			return true;
		}
	};
}

char LoopParallelization::ID = 0;
static RegisterPass<LoopParallelization> X("loop-par", "Loop Parallelization Pass");
