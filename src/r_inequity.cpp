/*  Economic Inequity (Inequality) Measures.
 *
 *  Copyright (C) 2018-2020 Marek Gagolewski (https://www.gagolewski.com)
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

#include "c_inequity.h"
#include <Rcpp.h>
#include <algorithm>



//' @title Inequity (Inequality) Measures
//'
//' @description
//' \code{gini_index()} gives the normalised Gini index
//' and \code{bonferroni_index()} implements the Bonferroni index.
//'
//' @details
//' Both indices can be used to quantify the "inequity" of a numeric sample.
//' They can be perceived as measures of data dispersion.
//' For constant vectors (perfect equity), the indices yield values of 0.
//' Vectors with all elements but one equal to 0 (perfect inequity),
//' are assigned scores of 1.
//' Both indices follow the Pigou-Dalton principle (are Schur-convex):
//' setting \eqn{x_i = x_i - h} and \eqn{x_j = x_j + h} with \eqn{h > 0}
//' and {x_i - h >=  x_j + h} (taking from the "rich" and
//' giving to the "poor") decreases the inequity.
//'
//' These indices have applications in economics, amongst others.
//' The Gini clustering algorithm uses the Gini index as a measure
//' of the inequality of cluster sizes.
//'
//'
//' The normalised  Gini index is given by:
//' \deqn{
//'     G(x_1,\dots,x_n) = \frac{
//'     \sum_{i=1}^{n-1} \sum_{j=i+1}^n |x_i-x_j|
//'     }{
//'     (n-1) \sum_{i=1}^n x_i
//'     }.
//' }
//'
//' The normalised Bonferroni index is given by:
//' \deqn{
//'     B(x_1,\dots,x_n) = \frac{
//'     \sum_{i=1}^{n}  (n-\sum_{j=1}^i \frac{n}{n-j+1})
//'          x_{\sigma(n-i+1)}
//'     }{
//'     (n-1) \sum_{i=1}^n x_i
//' }   },
//'
//'
//' Time complexity: \eqn{O(n)} for sorted (increasingly) data.
//' Otherwise, the vector will be sorted.
//'
//' In particular, for ordered inputs, it holds:
//' \deqn{
//'     G(x_1,\dots,x_n) = \frac{
//'     \sum_{i=1}^{n} (n-2i+1) x_{\sigma(n-i+1)}
//'     }{
//'     (n-1) \sum_{i=1}^n x_i
//'     },
//' }
//' where \eqn{\sigma} is an ordering permutation of \eqn{(x_1,\dots,x_n)}.
//'
//'
//' @references
//' Bonferroni C., Elementi di Statistica Generale, Libreria Seber,
//' Firenze, 1930.
//'
//' Gagolewski M., Bartoszuk M., Cena A., Genie: A new, fast, and
//' outlier-resistant hierarchical clustering algorithm,
//' Information Sciences 363, 2016, pp. 8-23. doi:10.1016/j.ins.2016.05.003
//'
//' Gini C., Variabilita e Mutabilita, Tipografia di Paolo Cuppini, Bologna, 1912.
//'
//'
//' @param x numeric vector of non-negative values
//'
//' @return The value of the inequity index, a number in [0,1].
//'
//' @examples
//' gini_index(c(2, 2, 2, 2, 2))  # no inequality
//' gini_index(c(0, 0, 10, 0, 0)) # one has it all
//' gini_index(c(7, 0, 3, 0, 0))  # give to the poor, take away from the rich
//' gini_index(c(6, 0, 3, 1, 0))  # (a.k.a. Pigou-Dalton principle)
//' bonferroni_index(c(2, 2, 2, 2, 2))
//' bonferroni_index(c(0, 0, 10, 0, 0))
//' bonferroni_index(c(7, 0, 3, 0, 0))
//' bonferroni_index(c(6, 0, 3, 1, 0))
//'
//' @rdname inequity
//' @export
// [[Rcpp::export]]
double gini_index(Rcpp::NumericVector x)
{
    ssize_t n = x.size();

    // check if sorted; if not, sort.
    for (ssize_t i=1; i<n; ++i) {
        if (x[i-1] > x[i]) {
            x = Rcpp::clone(x);
            std::sort(x.begin(), x.end());
            break;
        }
    }

    return Cgini_sorted(REAL(SEXP(x)), n);
}





//' @rdname inequity
//' @export
// [[Rcpp::export]]
double bonferroni_index(Rcpp::NumericVector x)
{
    ssize_t n = x.size();

    // check if sorted; if not, sort.
    for (ssize_t i=1; i<n; ++i) {
        if (x[i-1] > x[i]) {
            x = Rcpp::clone(x);
            std::sort(x.begin(), x.end());
            break;
        }
    }

    return Cbonferroni_sorted(REAL(SEXP(x)), n);
}

