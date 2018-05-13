#cython: boundscheck=False, wraparound=False, nonecheck=False, cdivision=True, language_level=3

"""
Adjusted- and Nonadjusted Rand Score, as well as
Adjusted- and Nonadjusted Fowlkes-Mallows Score
(for vectors of `small' ints)

See Hubert L., Arabie P., Comparing Partitions,
    Journal of Classification 2(1), 1985, 193-218

Copyright (C) 2018 Marek.Gagolewski.com


This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
"""

cimport numpy as np
import numpy as np
cimport cython
from libc.math cimport fabs, sqrt


ctypedef fused intT:
    np.int64_t
    np.int32_t
    np.int_t

cdef struct RandResult:
    np.float64_t ar
    np.float64_t r
    np.float64_t fm
    np.float64_t afm


cpdef np.ndarray[np.int_t, ndim=2] normalize_confusion_matrix(
    np.ndarray[np.int_t, ndim=2] C):
    """
    Applies pivoting to a given confusion matrix.
    """
    C = C.copy()
    cpdef unsigned int xc = C.shape[0], yc = C.shape[1], i, j, w

    for i in range(xc-1):
        w = i
        for j in range(i+1, yc): # find w = argmax C[i,w], w=i,i+1,...yc-1
            if C[i,w] < C[i,j]: w = j
        for j in range(xc): # swap columns i and w
            C[j,i], C[j,w] = C[j,w], C[j,i]

    return C


cpdef np.ndarray[np.int_t, ndim=2] confusion_matrix(
    np.ndarray[intT] x, np.ndarray[intT] y):
    """
    Computes the confusion matrix (as a dense matrix)


    Parameters:
    ----------

    x, y : ndarray, shape (n,)
        two small-int vectors of the same lengths, representing
        two k-partitions of the same set


    Returns:
    -------

    C : ndarray, shape (kx, ky)
        a confusion matrix
    """
    cpdef unsigned int n = x.shape[0], i, j, w
    if n != y.shape[0]: raise Exception("incompatible lengths")

    cpdef intT xmin = x[0], ymin = y[0], xmax = x[0], ymax = y[0]
    for i in range(1, n):
        if   x[i] < xmin: xmin = x[i]
        elif x[i] > xmax: xmax = x[i]

        if   y[i] < ymin: ymin = y[i]
        elif y[i] > ymax: ymax = y[i]

    cpdef unsigned int xc = <unsigned int>(xmax-xmin+1)
    cpdef unsigned int yc = <unsigned int>(ymax-ymin+1)

    # if xc == yc == 1 or xc == yc == 0 or xc == yc == n: return 1.0

    if xc*yc > <unsigned int>(10000):
        raise Exception("max_size of the confusion matrix exceeded")

    cdef np.ndarray[np.int_t, ndim=2] C = np.zeros((xc, yc), dtype=np.int_)
    for i in range(n):
        C[x[i]-xmin, y[i]-ymin] += 1

    return C


cpdef RandResult compare_partitions(np.ndarray[intT, ndim=2] C):
    """
    Computes the adjusted and nonadjusted Rand- and FM scores
    based on a given confusion matrix.

    See Hubert L., Arabie P., Comparing Partitions,
       Journal of Classification 2(1), 1985, 193-218, esp. Eqs. (2) and (4)


    Parameters:
    ----------

    C : ndarray, shape (kx,ky)
        a confusion matrix

    Returns:
    -------

    scores : dict
        a dictionary with keys 'ar', 'r', 'afm', 'fm', giving
        the adjusted Rand, Rand, adjusted Fowlkes-Mallows, and
        Fowlkes-Mallows scores, respectively.
    """
    cpdef unsigned int n = C.sum(), xc = C.shape[0], yc = C.shape[1], i, j

    cpdef np.double_t sum_comb_x = 0.0, sum_comb = 0.0, sum_comb_y = 0.0
    cpdef np.double_t t, prod_comb, mean_comb, e_fm
    for i in range(xc):
        t = 0.0
        for j in range(yc):
            t += C[i, j]
            sum_comb += C[i, j]*(C[i, j]-1.0)*0.5
        sum_comb_x += t*(t-1.0)*0.5 # comb2(t)

    for j in range(yc):
        t = 0.0
        for i in range(xc):
            t += C[i, j]
        sum_comb_y += t*(t-1.0)*0.5 # comb2(t)


    prod_comb = (sum_comb_x*sum_comb_y)/n/(n-1.0)*2.0 # expected sum_comb,
                                        # see Eq.(2) in (Hubert, Arabie, 1985)
    mean_comb = (sum_comb_x+sum_comb_y)*0.5
    e_fm = prod_comb/sqrt(sum_comb_x*sum_comb_y) # expected FM (variant)

    cdef RandResult res
    res.ar  = (sum_comb-prod_comb)/(mean_comb-prod_comb)
    res.r   = 1.0 + (2.0*sum_comb - (sum_comb_x+sum_comb_y))/n/(n-1.0)*2.0
    res.fm  = sum_comb/sqrt(sum_comb_x*sum_comb_y)
    res.afm = (res.fm - e_fm)/(1.0 - e_fm) # Eq.(4) in (Hubert, Arabie, 1985)

    return res


cpdef np.float64_t adjusted_rand_score(np.ndarray[intT] x, np.ndarray[intT] y):
    """
    The Rand index adjusted for chance.

    See Hubert L., Arabie P., Comparing Partitions,
    Journal of Classification 2(1), 1985, 193-218


    Parameters:
    ----------

    x, y : ndarray, shape (n,)
        two small-int vectors of the same lengths, representing
        two k-partitions of the same set


    Returns:

    score : float
        partition similarity measure
    """

    return compare_partitions(confusion_matrix(x, y)).ar


cpdef np.float64_t rand_score(np.ndarray[intT] x, np.ndarray[intT] y):
    """
    The original Rand index (not adjusted for chance),
    yielding the `probability' of agreement between the two partitions

    See Hubert L., Arabie P., Comparing Partitions,
    Journal of Classification 2(1), 1985, 193-218


    Parameters:
    ----------

    x, y : ndarray, shape (n,)
        two small-int vectors of the same lengths, representing
        two k-partitions of the same set


    Returns:

    score : float
        partition similarity measure
    """

    return compare_partitions(confusion_matrix(x, y)).r


cpdef np.float64_t adjusted_fm_score(np.ndarray[intT] x, np.ndarray[intT] y):
    """
    The Fowlkes-Mallows index adjusted for chance,

    See Eqs. (2) and (4)  in Hubert L., Arabie P., Comparing Partitions,
    Journal of Classification 2(1), 1985, 193-218

    Parameters:
    ----------

    x, y : ndarray, shape (n,)
        two small-int vectors of the same lengths, representing
        two k-partitions of the same set


    Returns:

    score : float
        partition similarity measure
    """

    return compare_partitions(confusion_matrix(x, y)).afm


cpdef np.float64_t fm_score(np.ndarray[intT] x, np.ndarray[intT] y):
    """
    The original Fowlkes-Mallows index (not adjusted for chance)

    See Hubert L., Arabie P., Comparing Partitions,
    Journal of Classification 2(1), 1985, 193-218

    Parameters:
    ----------

    x, y : ndarray, shape (n,)
        two small-int vectors of the same lengths, representing
        two k-partitions of the same set


    Returns:

    score : float
        partition similarity measure
    """

    return compare_partitions(confusion_matrix(x, y)).fm