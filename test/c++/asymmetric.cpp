/*******************************************************************************
 *
 * This file is part of ezARPACK, an easy-to-use C++ wrapper for
 * the ARPACK-NG FORTRAN library.
 *
 * Copyright (C) 2016-2018 Igor Krivenko <igor.s.krivenko@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 ******************************************************************************/

#include "common.hpp"

//////////////////////////////////////////////
// Eigenproblems with general real matrices //
//////////////////////////////////////////////

using params_t = arpack_worker<Asymmetric>::params_t;

const int N = 100;
const double diag_coeff = 0.75;
const int offdiag_offset = 3;
const double offdiag_coeff = 1.0;
const int nev = 10;

// Symmetric matrix A
auto A = make_sparse_matrix<Asymmetric>(N, diag_coeff, offdiag_offset, offdiag_coeff);
// Inner product matrix
auto M = make_inner_prod_matrix<Asymmetric>(N);

auto spectrum_parts = {params_t::LargestMagnitude,
                       params_t::SmallestMagnitude,
                       params_t::LargestReal, params_t::SmallestReal,
                       params_t::LargestImag, params_t::SmallestImag };

TEST(arpack_worker_symmetric, InnerProduct) {
 ASSERT_GT(eigenvalues(M)(0),.0);
}

// Standard eigenproblem
TEST(arpack_worker_asymmetric, Standard) {
 auto Aop = [](vector_const_view<double> from, int, vector_view<double> to, int) {
  to = A*from;
 };

 arpack_worker<Asymmetric> ar(first_dim(A));

 for(auto e : spectrum_parts) {
  params_t params(nev, e, params_t::Ritz);
  ar(Aop, params);
  check_eigenvectors(ar,A);
 }
}

// Generalized eigenproblem: invert mode
TEST(arpack_worker_asymmetric, Invert) {
 decltype(A) invMA = inverse(M) * A;

 auto op = [&invMA](vector_const_view<double> from, int, vector_view<double> to, int, bool) {
  to = invMA * from;
 };
 auto Bop = [](vector_const_view<double> from, int, vector_view<double> to, int) {
  to = M * from;
 };

 arpack_worker<Asymmetric> ar(first_dim(A));

 for(auto e : spectrum_parts) {
  params_t params(nev, e, params_t::Ritz);
  params.ncv = 30;
  ar(op, Bop, arpack_worker<Asymmetric>::Invert, params);
  check_eigenvectors(ar,A,M);
 }
}

MAKE_MAIN;
