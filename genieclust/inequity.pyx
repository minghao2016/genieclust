# distutils: language=c++
# cython: boundscheck=False
# cython: cdivision=True
# cython: nonecheck=False
# cython: wraparound=False
# cython: language_level=3


"""
Inequity (Inequality) Measures

Copyright (C) 2018-2020 Marek Gagolewski (https://www.gagolewski.com)
All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
this list of conditions and the following disclaimer in the documentation
and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its contributors
may be used to endorse or promote products derived from this software without
specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
"""

cimport cython
cimport numpy as np
import numpy as np
from . cimport c_inequity

ctypedef fused T:
    int
    long
    long long
    ssize_t
    float
    double

cdef T square(T x):
    return x*x



cpdef double gini_index(T[:] x, bint is_sorted=False):
    """
    The Normalised Gini index:

    $$
        G(x_1,\dots,x_n) = \frac{
        \sum_{i=1}^{n-1} \sum_{j=i+1}^n |x_i-x_j|
        }{
        (n-1) \sum_{i=1}^n x_i
        }.
    $$

    Time complexity: $O(n)$ for sorted data; it holds:
    $$
        G(x_1,\dots,x_n) = \frac{
        \sum_{i=1}^{n} (n-2i+1) x_{\sigma(n-i+1)}
        }{
        (n-1) \sum_{i=1}^n x_i
        },
    $$
    where $\sigma$ is an ordering permutation of $(x_1,\dots,x_n)$.

    Time complexity: $O(n)$ for sorted data.


    Both the Gini and Bonferroni indices can be used to quantify the "inequity"
    of a numeric sample. They can be perceived as measures of data dispersion.
    For constant vectors (perfect equity), the indices yield values of 0.
    Vectors with all elements but one equal to 0 (perfect inequity),
    are assigned scores of 1.
    Both indices follow the Pigou-Dalton principle (are Schur-convex):
    setting $x_i = x_i - h$ and $x_j = x_j + h$ with $h > 0$
    and $x_i - h \geq  x_j + h$ (taking from the "rich" and giving to the "poor")
    decreases the inequity.

    These indices have applications in economics, amongst others.
    The Gini clustering algorithm uses the Gini index as a measure
    of the inequality of cluster sizes.


    References:
    -----------

    Gagolewski M., Bartoszuk M., Cena A., Genie: A new, fast, and
    outlier-resistant hierarchical clustering algorithm,
    Information Sciences 363, 2016, pp. 8-23. doi:10.1016/j.ins.2016.05.003

    Gini C., Variabilita e Mutabilita,
    Tipografia di Paolo Cuppini, Bologna, 1912.



    See also:
    ---------

    bonferroni_index


    Parameters:
    ----------

    x : ndarray, shape (n,)
        Input vector >= 0.

    is_sorted : bool
        Indicates if x is sorted increasingly.


    Returns:
    -------

    index : double
        The value of the inequity index, a number in [0,1].
    """

    if not is_sorted: x = np.sort(x)
    return c_inequity.Cgini_sorted(&x[0], x.shape[0])



cpdef double bonferroni_index(T[:] x, bint is_sorted=False):
    """
    The Normalised Bonferroni index:
    $$
        B(x_1,\dots,x_n) = \frac{
        \sum_{i=1}^{n}  \left( n-\sum_{j=1}^i \frac{n}{n-j+1} \right) x_{\sigma(n-i+1)}
        }{
        (n-1) \sum_{i=1}^n x_i
        },
    $$
    where $\sigma$ is an ordering permutation of $(x_1,\dots,x_n)$.

    Time complexity: $O(n)$ for sorted data.


    References:
    -----------

    Bonferroni C., Elementi di Statistica Generale, Libreria Seber,
    Firenze, 1930.


    See also:
    ---------

    gini_index


    Parameters:
    ----------

    x : ndarray, shape (n,)
        Input vector >= 0.

    is_sorted : bool
        Indicates if x is sorted increasingly.


    Returns:
    -------

    index : double
        The value of the inequity index, a number in [0,1].
    """

    if not is_sorted: x = np.sort(x)
    return c_inequity.Cbonferroni_sorted(&x[0], x.shape[0])
