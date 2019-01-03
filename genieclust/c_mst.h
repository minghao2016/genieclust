/*  Minimum Spanning Tree Algorithms:
 *  a. Prim-Jarník's for Complete Undirected Graphs,
 *  b. Kruskal's for k-NN graphs.
 *
 *  Copyright (C) 2018-2019 Marek.Gagolewski.com
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *
 *  1. Redistributions of source code must retain the above copyright notice,
 *  this list of conditions and the following disclaimer.
 *
 *  2. Redistributions in binary form must reproduce the above copyright notice,
 *  this list of conditions and the following disclaimer in the documentation
 *  and/or other materials provided with the distribution.
 *
 *  3. Neither the name of the copyright holder nor the names of its
 *  contributors may be used to endorse or promote products derived from this
 *  software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 *  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 *  PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 *  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 *  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 *  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 *  OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 *  WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 *  OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 *  ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */


#ifndef __c_mst_h
#define __c_mst_h

#include <vector>
#include <algorithm>
#include <stdexcept>
#include <limits>
#include "c_argfuns.h"
#include "c_disjoint_sets.h"

#define DOUBLE_INFTY (std::numeric_limits<double>::infinity())


/*! Computes a minimum spanning tree of a k-Nearest Neighbor Graph
 *  using Kruskal's algorithm, and orders its edges w.r.t. increasing weights.
 *
 *  Note that in general, an MST of the M-Nearest Neighbor Graph
 *  might not be the MST of the complete Pairwise Distances Graph.
 *
 * @param dist   a c_contiguous array, shape (n,k),
 *        dist[i,j] gives the weight of the (undirected) edge {i, ind[i,j]}
 * @param ind    a c_contiguous array, shape (n,k),
 *        (undirected) edge definition, interpreted as {i, ind[i,j]}
 * @param n number of nodes
 * @param k minimal degree of all the nodes
 * @param mst_d [out] vector of length n-1, gives weights of the
 *        resulting MST edges in nondecreasing order
 * @param mst_i [out] vector of length 2*(n-1), representing
 *        a c_contiguous array of shape (n-1,2), defining the edges
 *        corresponding to mst_d, with mst_i[j,0]<mst_i[j,1] for all j
 *
 * @return number of edges in the minimal spanning forest
 */
ssize_t Cmst_nn(const double* dist, const ssize_t* ind, ssize_t n, ssize_t k,
             double* mst_d, ssize_t* mst_i)
{
    if (n <= 0)   throw std::domain_error("n <= 0");
    if (k <= 0)   throw std::domain_error("k <= 0");
    ssize_t nk = n*k;

    // determine the ordering permutation of dist
    std::vector<ssize_t> arg_dist(nk);
    Cargsort(arg_dist.data(), dist, nk, false);

    ssize_t arg_dist_cur = 0;
    ssize_t mst_edge_cur = 0;
    std::vector<ssize_t> nn_used(n, 0);
    CDisjointSets ds(n);
    while (mst_edge_cur < n-1) {
        if (arg_dist_cur >= nk) {
            ssize_t ret = mst_edge_cur;
            while (mst_edge_cur < n-1) {
                mst_i[2*mst_edge_cur+0] = -1;
                mst_i[2*mst_edge_cur+1] = -1;
                mst_d[mst_edge_cur]   = DOUBLE_INFTY;
                mst_edge_cur++;
            }
            return ret;
        }

        ssize_t u = arg_dist[arg_dist_cur]/k; // u is the asg_dist_cur-th edge
        if (nn_used[u] >= k || u < 0 || u >= n)
            throw std::logic_error("ASSERT FAIL in Cmst_nn");
        ssize_t v = ind[k*u+nn_used[u]];      // v is its nn_used[u]-th NN
        double d = dist[k*u+nn_used[u]];
        nn_used[u]++;                         // mark u's NN as used
        arg_dist_cur++;

        if (ds.find(u) == ds.find(v))
            continue;

        if (u > v) std::swap(u, v);
        mst_i[2*mst_edge_cur+0] = u;
        mst_i[2*mst_edge_cur+1] = v;
        mst_d[mst_edge_cur]   = d;

        ds.merge(u, v);
        mst_edge_cur++;
    }

    return mst_edge_cur;
}


/*! Comparer used to sort MST edges w.r.t. increasing weights
 */
struct CMstTriple {
    CMstTriple() {}

    CMstTriple(ssize_t i1, ssize_t i2, double d) {
        this->d = d;
        if (i1 < i2) {
            this->i1 = i1;
            this->i2 = i2;
        }
        else {
            this->i1 = i2;
            this->i2 = i1;
        }
    }

    bool operator<(const CMstTriple& other) const {
        if (d == other.d) {
            if (i1 == other.i1)
                return i2 < other.i2;
            else
                return i1 < other.i1;
        }
        else
            return d < other.d;
    }

    ssize_t i1;
    ssize_t i2;
    double d;
};


/*! A Jarník (Prim/Dijkstra)-like algorithm for determining
 *  a(*) minimum spanning tree (MST) of a complete undirected graph
 *  with weights given by a symmetric n*n matrix.
 *
 *  (*) Note that there might be multiple minimum trees spanning a given graph.
 *
 *
 *  References:
 *  ----------
 *
 *  M. Gagolewski, M. Bartoszuk, A. Cena,
 *  Genie: A new, fast, and outlier-resistant hierarchical clustering algorithm,
 *  Information Sciences 363 (2016) 8–23.
 *
 *  V. Jarník, O jistém problému minimálním,
 *  Práce Moravské Přírodovědecké Společnosti 6 (1930) 57–63.
 *
 *  C.F. Olson, Parallel algorithms for hierarchical clustering,
 *  Parallel Comput. 21 (1995) 1313–1325.
 *
 *  R. Prim, Shortest connection networks and some generalizations,
 *  Bell Syst. Tech. J. 36 (1957) 1389–1401.
 *
 *
 * @param dist a c_contiguous (symmetric-what a waste!) array, shape (n,n),
 *        dist[i,j] gives the weight of the (undirected) edge {i, j}
 * @param n number of nodes
 * @param mst_d [out] vector of length n-1, gives weights of the
 *        resulting MST edges in nondecreasing order
 * @param mst_i [out] vector of length 2*(n-1), representing
 *        a c_contiguous array of shape (n-1,2), defining the edges
 *        corresponding to mst_d, with mst_i[j,0]<mst_i[j,1] for all j
 */
void Cmst_complete(const double* dist, ssize_t n, double* mst_d, ssize_t* mst_i)
{
    std::vector<double>  Dnn(n, DOUBLE_INFTY);
    std::vector<ssize_t> Fnn(n);
    std::vector<ssize_t> M(n);
    std::vector<CMstTriple> res(n-1);
    for (ssize_t i=0; i<n; ++i) M[i] = i;

    ssize_t lastj = 0, bestj, bestjpos;
    for (ssize_t i=0; i<n-1; ++i) {
        // M[1], ... M[n-i-1] - points not yet in the MST
        bestjpos = bestj = 0;
        for (ssize_t j=1; j<n-i; ++j) {
            if (dist[n*lastj+M[j]] < Dnn[M[j]]) {
                Dnn[M[j]] = dist[n*lastj+M[j]];
                Fnn[M[j]] = lastj;
            }
            if (Dnn[M[j]] < Dnn[bestj]) {        // D[0] == INFTY
                bestj = M[j];
                bestjpos = j;
            }
        }

        M[bestjpos] = M[n-i-1]; // never ever visit bestj again
        lastj = bestj;          // next time, start from bestj

        // and an edge to MST: (smaller index first)
        res[i] = CMstTriple(Fnn[bestj], bestj, Dnn[bestj]);
    }

    std::sort(res.begin(), res.end());

    for (ssize_t i=0; i<n-1; ++i) {
        mst_d[i] = res[i].d;
        mst_i[2*i+0] = res[i].i1; // i1 < i2
        mst_i[2*i+1] = res[i].i2;
    }
}

#endif