#include <iostream>
#include <string>
#include <algorithm>
#include <functional>
#include <vector>
#include <map>
#include <set>
#include <bitset>
#include <queue>

#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <cmath>
using namespace std;

static const int64_t ONE = 0x1;
static const int MAXN = 37;

struct Combination
{
    Combination(int N, int R):
        completed(N < 1 || R > N),
        generated(0),
        n(N), r(R)
    {
        for(int c=1;c<=R;c++)
            curr[c-1] = c;
    }

    void next()
    {
        completed = true;
        for(int i=r-1;i>=0;i--)
        {
            if( curr[i] < n - r + i + 1 )
            {
                int j = curr[i] + 1;
                while( i <= r )
                    curr[i++] = j++;
                completed = false;
                generated++;
                break;
            }
        }
    }

    int curr[MAXN];
    bool completed;
    int generated;
    int n, r;
};

struct graph_t
{	
	graph_t(int n):nv(n)
	{
		// initialize
		target = 0x0;
		for(int i=1;i<=n;i++)
		{
			target |= (ONE << i);
		}
	}
	
	void evaluate(int support[], int k)
	{
		covered = 0x0;
		for(int i=0;i<k;i++)
		{
			covered |= coverage[support[i]];
		}
	}
	
	void makeCoverage()
	{
		for(int i=1;i<=nv;i++)
		{
			coverage[i] = 0x0;
			// cover itself
			coverage[i] |= (ONE << i);
			// cover its neighbors
			for(int j=0;j<E[i].size();j++)
				coverage[i] |= (ONE << E[i][j]);
		}
	}
	
	int nv;		// number of nodes
	vector<vector<int> > E;
	
	int64_t target;
    int64_t covered;
	int64_t coverage[MAXN];
	
};

int solve(graph_t& G)
{
	G.makeCoverage();
	
	set<int> n1n;
	set<int> n1;
	
	// find out all 1 degree nodes
	for(int i=1;i<=G.nv;i++)
	{
		if( G.E[i].size() == 1 && n1.find(i) == n1.end() ){
			int v1 = i, v2 = G.E[i][0];
			
			n1n.insert(v1);
			n1n.insert(v2);
			
			n1.insert(v2);

			// plus all neighbor of G.E[i][0]
			for(int j=0;j<G.E[v2].size();j++)
				n1n.insert(G.E[v2][j]);
		}
	}
	
	
	// all nodes in n1n are in the min cover set, so no need to consider them
	// remove these nodes from the graph
	// this can be done by modifying the target
	for(set<int>::iterator it=n1.begin();
		it!=n1.end();
		it++)
	{
		G.target &= (~(G.coverage[*it]));
	}

    // it is possible the graph is already covered
    if( G.target == 0 ) return n1.size();
	
	bool found = false;
	for(int i=1;i<=G.nv-n1.size();i++)
	{
		Combination comb(G.nv, i);
	    while( !comb.completed )
	    {
			
			G.evaluate(comb.curr, i);
			comb.next();
			
            // the covered set should be a super set of the target
			if( (G.covered & G.target) == G.target )
			{
				found = true;
				break;
			}
	    }
		
		if( found )
			return i + n1.size();
	}
}

int solve_wholegraph(const graph_t& G)
{	
	// split the graph into a few subgraphs, and solve them individually
	vector<int> vertices;
	for(int i=1;i<=G.nv;i++)
		vertices.push_back(i);
	
	// nodes already processed
	set<int> vdone;
	
	int res = 0;
	while( !vertices.empty() )
	{
		// nodes for subgraph
		set<int> sg;
		int vs[MAXN];
		int ns = 0;
		
		// pick the first vertex
		int v = vertices[0];
		vs[1] = v;
		ns = 1;
		sg.insert(v);
		vdone.insert(v);
		
		queue<int> Q;
		Q.push(v);
		while( !Q.empty() )
		{
			int cur = Q.front();
			Q.pop();
			
			// find all its neighbors
			for(int i=0;i<G.E[cur].size();i++)
			{
				int ci = G.E[cur][i];
				if( vdone.find( ci ) == vdone.end() )
				{
					Q.push(ci);
					vs[++ns] = ci;
					sg.insert(ci);
					vdone.insert(ci);
				}
			}
		}
		
		// renumber the nodes
		int index[MAXN];
		int nid = 1;
		for(set<int>::iterator sit=sg.begin();sit!=sg.end();sit++)
		{
			index[(*sit)] = nid++;
		}
		
		// remove the vertices in the subgraph from the original graph
		for(set<int>::iterator sit=sg.begin();sit!=sg.end();sit++)
		{
			vertices.erase( std::find(vertices.begin(), vertices.end(), (*sit)) );
		}
		
		// create the subgraph and solve it
		graph_t subgraph(ns);
		subgraph.E.resize(ns+1);
		for(set<int>::iterator sit=sg.begin();sit!=sg.end();sit++)
		{
			int v1 = (*sit);
			for(int i=0;i<G.E[v1].size();i++)
			{
				int v2 = G.E[v1][i];
				if( sg.find( v2 ) != sg.end() )
				{
					// add this edge to the subgraph
					subgraph.E[ index[v1] ].push_back( index[v2] );
				}
			}
		}
		subgraph.makeCoverage();
		
		res += solve(subgraph);
	}	
	
	return res;
}

int main()
{	
    int n, m;

    while( true )
    {
		scanf("%d %d", &n, &m);
		if( n == 0 && m == 0 ) break;
		
		graph_t G(n);
		G.E.resize(n+1);
		
        for(int i=0;i<m;i++)
        {
			int a, b;
			scanf("%d %d", &a, &b);
			if( a == b ) continue;
			G.E[a].push_back(b);
			G.E[b].push_back(a);
        }

        printf("%d\n", solve_wholegraph(G));
    }

    return 0;
}
