/*	Jake Shoffner
	worddice.cpp
	04/17/22
*/
#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <map>
#include <sstream>
#include <list>
using namespace std;

typedef enum {SOURCE, SINK, DICE, WORD} Node_Type;

class Edge {
  public:
	class Node *to;
	class Node *from;
	Edge *reverse;
	int original, residual;
	string name;
};
class Node {
  public:
	int id, spellingID, visited;
	Node_Type type;
	Edge *backedge;
	vector <bool> letters;
	vector <Edge *> adj;
	string name;

	Node();
};
class Graph {
  public:
	vector <Node *> nodes, dice, words;
	vector <Edge *> edges, Path;
	map <int, Node *> N_Map;
	map <string, Edge *> E_Map;
	Node *src, *snk;

	int BFS();
	int Find_Augmenting_Path(string word);
	Node *Get_Node(int &id, vector <string> &dname);
	Node *Get_NodeW(int &id, char &letter);
	Edge *Get_Edge(Node *fr, Node *tt);
};

int main(int argc, char **argv) {
	ifstream dfin, wfin;
	vector <string> d, empty;
	string rtemp;
	int id = 0;
	Node *source, *sink;
	Graph *g;
	
	/* Reading in the files */
	if (argc != 3) {
		cerr << "Invalid number of arguments: worddice dice-file word-list\n";
		return -1;
	}
	dfin.open(argv[1]);
	wfin.open(argv[2]);

	/* Storing the dice in a vector for later */
	while (dfin >> rtemp) d.push_back(rtemp);

	/* Reading in words */
	while (wfin >> rtemp) {
		/* Creating a graph each time */
		g = new Graph;

		/* Creating the source node and graph accordingly */
		source = new Node();
		source->id = 0;
		source->type = SOURCE;
		source->backedge = NULL;
		g->nodes.push_back(source);
		g->src = source;

		/* Creting nodes for each dice and pushing them back temporarily into my dice vector */
		for (id = 0; id < (int)d.size(); ++id) {
			g->Get_Node(id, d)->backedge = g->Get_Edge(source, g->Get_Node(id, d));
			g->dice.push_back(g->Get_Node(id, d));
		}

		/* Creating nodes for each letter and pushing them back temporarily into my words vector */
		for (unsigned int i = 0; i < rtemp.size(); ++i) {
			g->words.push_back(g->Get_NodeW(id, rtemp[i]));
			++id;
		}
		
		/* Creating the sink node and setting it up accordingly */
		sink = new Node();
		sink->id = id + 1; 
		sink->type = SINK;
		g->nodes.push_back(sink);
		g->snk = sink;
		
		/* Going through each dice's letters and seeing if it has a match with a letter of the word */
		for (unsigned int i = 0; i < g->dice.size(); ++i) {
			for (unsigned int j = 0; j < rtemp.size(); ++j) {
				for (int x = 0; x < (int)g->dice[i]->letters.size(); ++x) {
					if (g->dice[i]->letters[x] == 1 && x == rtemp[j] - 'A')  g->words[j]->backedge = g->Get_Edge(g->dice[i], g->words[j]);
				}
			}
		}
	
		/* Creating the edges between the letters and the sink node, as well as setting the backedge of the sink */
		for (unsigned int i = 0; i < g->words.size(); ++i) sink->backedge = g->Get_Edge(g->words[i], sink);

		/* Print testing */	
		g->Find_Augmenting_Path(rtemp);

		/* Deleting the graph and freeing up allocations */
		for (unsigned int i = 0; i < g->nodes.size(); ++i) delete g->nodes[i];
		for (unsigned int i = 0; i < g->edges.size(); ++i) delete g->edges[i];
		delete g;
	}
	return 0;
}

/* Initially setting up the visited field and letters vector */
Node::Node() {
	visited = 0;
	letters.resize(26,0);
}
Node *Graph::Get_Node(int &lol, vector <string> &dname) {
	Node *n;
	int ind = 0;

	/* If the Node id already exists, we just return the node */
	if (N_Map.find(lol) != N_Map.end()) return N_Map[lol];

	/* Creating the node if it does not exist */
	n = new Node();
	n->id = lol + 1;
	n->type = DICE;
	
	/* Updating vector of letters to represent the letters we have on the dice */
	for (unsigned int i = 0; i < dname[lol].size(); ++i) {
		ind = dname[lol][i] - 'A';
		n->letters[ind] = 1;
	}
	
	N_Map[lol] = n;
	nodes.push_back(n);
	return n;
}
Edge *Graph::Get_Edge(Node *fr, Node *tt) {
	string t1, t2, en, ren;
	stringstream ss;
	Edge *e, *re;
	
	/* Converting IDs to strings for concatenation */
	ss << fr->id;
	ss >> t1;
	ss.clear();
	ss << tt->id;
	ss >> t2;
	ss.clear();
	en = t1 + "->";
	en += t2;
	ren = t2 + "->";
	ren += t1;

	/* Checks to make sure we aren't creating duplicate edges */
	if (E_Map.find(en) != E_Map.end()) return E_Map[en];

	/* Creating the edge if it does not exist */
	e = new Edge;
	e->name = en;
	E_Map[en] = e;
	edges.push_back(e);
	e->original = 1;
	e->residual = 0;
	e->from = fr;
	e->to = tt;
	
	/* Creating reverse edge */
	re = new Edge;
	re->original = 0;
	re->residual = 1;
	re->from = tt;
	re->to = fr;
	re->reverse = e;
	e->reverse = re;
	
	/* Pushing back edges to proper adjacency lists */
	fr->adj.push_back(e);
	tt->adj.push_back(re);
	edges.push_back(re);
	return e;
}
/* This is the same as the above Get_Node(), except for WORDs instead of DICE */
Node *Graph::Get_NodeW(int &lol, char &letter) {
	Node *n;
	int ind = 0;

	if (N_Map.find(lol) != N_Map.end()) return N_Map[lol];

	n = new Node();
	n->id = lol + 1;
	n->type = WORD;
	ind = letter - 'A';
	n->letters[ind] = 1;

	N_Map[lol] = n;
	nodes.push_back(n);
	return n;
}
int Graph::BFS() {
	Edge *e;
	Node *n;
	list <Node *> queue;
	
	/* Setting each node's visited field back to 0 */
	for (unsigned int i = 0; i < nodes.size(); ++i) nodes[i]->visited = 0;
	src->visited = 1;
	queue.push_back(src);
	
	/* Goes until queue is completely processed */
	while (!queue.empty()) {
		n = queue.front();
		queue.pop_front();
	
		/* If BFS() has a valid path, process path and return 1 */
		if (n == snk) {
			while (n != src) {
				Path.push_back(n->backedge);
				n = n->backedge->from;
			}
			return 1;
		}
	
		/* Running through the adjacency list and pushing back if it's a valid edge path */
		for (unsigned int i = 0; i < n->adj.size(); ++i) {
			e = n->adj[i];
			if (e->to->visited == 0 && e->original == 1) {
				queue.push_back(e->to);
				e->to->visited = 1;
				e->to->backedge = e;
			}
		}
	}	
	return 0;
}
int Graph::Find_Augmenting_Path(string word) {
	unsigned int counter = 0;
	
	/* Calling BFS() to get every PATH possible until there are no more PATHs left */
	while(BFS()) {
		/* Going through the Path and updating the residuals and original of the edges and reverse edges */
		for (unsigned int i = 0; i < Path.size(); ++i) {
			Path[i]->original = 0;
			Path[i]->residual = 1;
			Path[i]->reverse->original = 1;
			Path[i]->reverse->residual = 0;
			
			/* Getting the spelling ID of the path for printing */
			Path[i]->to->spellingID = Path[i]->from->id;
		}

		/* Clearing the Path and updating counter */
		Path.clear();
		++counter;
	}

	/* If the counter does not equal the size of the word, there is no possible way we can spell that specified word */
	if (counter != word.size()) cout << "Cannot spell " << word << '\n';

	/* If we can't 'Cannot' spell the word... then we CAN spell the word! So we print the word and the spellingIDs */
	else {
		for (unsigned int i = 0; i < words.size(); ++i) {
			if (i == words.size() - 1) cout << words[i]->spellingID - 1 << ": ";
			else cout << words[i]->spellingID - 1 << ",";
		}
		cout << word << '\n';
	}
	return 0;
}
