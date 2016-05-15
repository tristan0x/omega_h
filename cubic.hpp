// poly.cpp : solution of cubic equation
// (c) Khashin S.I. http://math.ivanovo.ac.ru/dalgebra/Khashin/index.html
// khash2 (at) gmail.com
//
// Dan Ibanez: took initial code, fixed the less-than-3-roots case,
// added geometric multiplicities output, focus only on real roots

// solve cubic equation x^3 + a * x^2 + b * x + c = 0
INLINE UInt solve_cubic(
    Real a, Real b, Real c,
    Few<Real, 3>& roots, Few<UInt, 3>& mults) {
// http://mathworld.wolfram.com/CubicFormula.html
// https://en.wikipedia.org/wiki/Cubic_function#Reduction_to_a_depressed_cubic
  // this is b^2 from wikipedia
  Real a2 = square(a);
  // this is (-p/3) from wikipedia, Q from wolfram mathworld
  Real q  = (a2 - 3 * b) / 9;
  // this is (q/2) from wikipedia, -R from wolfram mathworld
  Real r  = (a * (2. * a2 - 9. * b) + 27. * c) / 54;
  // this is (q^2/4) from wikipedia
  Real r2 = square(r);
  // this is (-p^3/27) from wikipedia
  Real q3 = cube(q);
//this is the (opposite of the) Cardano assumption ((q^2/4) + (p^3/27)) > 0
  if(r2 < q3) {
// https://en.wikipedia.org/wiki/Cubic_function#Three_real_roots
    Real t = r / sqrt(q3);
    t = max2(-1., t);
    t = min2( 1., t);
    t = acos(t);
    a /= 3.;
    q = -2. * sqrt(q);
    roots[0] = q * cos(t / 3.) - a;
    roots[1] = q * cos((t + 2. * PI) / 3.) - a;
    roots[2] = q * cos((t - 2. * PI) / 3.) - a;
    mults[0] = 1;
    mults[1] = 1;
    mults[2] = 1;
    return 3;
  } else {
// https://en.wikipedia.org/wiki/Cubic_function#Cardano.27s_method
    Real u3 = -r - sqrt(r2 - q3);
  //std::pow will not accept a negative base (it can't tell
  //that (1./3.) is exactly the reciprocal of an odd number),
  //so we could strip out the sign on input and put it back
  //on output:
  //Real u = sign(u3)*pow(fabs(u3), 1./3.);
  //even better, C++11 provides std::cbrt which solves this
    Real u = cbrt(u3);
    Real v = (u == 0.0) ? 0.0 : (q / u);
    Real t1 = u + v;
    // recall x = t - (b/(3a)), in our case x = t - (a/3)
    roots[0] = t1 - (a / 3.);
    Real t_real = -0.5 * (u + v);
    Real t_imag = 0.5 * sqrt(3.) * (u - v);
    roots[1] = (t_real) - (a / 3.);
    if (fabs(t_imag) < EPSILON) {
      if (are_close(roots[0], roots[1])) {
        mults[0] = 3;
        return 1;
      }
      mults[0] = 1;
      mults[1] = 2;
      return 2;
    }
    mults[0] = 1;
    return 1;
  }
}

/* http://mathworld.wolfram.com/CharacteristicPolynomial.html */
INLINE void characteristic_cubic(Matrix<3,3> A,
    Real& a, Real& b, Real& c)
{
  Real tA = trace(A);
  Real c2 = (1. / 2.) * ((tA * tA) - trace(A * A));
  a = -tA;
  b = c2;
  c = -determinant(A);
}

/* the null space of the matrix (s = m - lI)
   is the space spanned by the eigenvectors.
   the multiplicity of this root is the dimensionality
   of that space, i.e. the number of eigenvectors. */

/* in the case that the null space is 1D, take the
   largest cross product of any pair of columns */
INLINE void single_eigenvector(Matrix<3,3> m, Real l,
    Vector<3>& v) {
  Matrix<3,3> s = (m - (l * identity_matrix<3,3>()));
  v = cross(s[0], s[1]);
  Real v_norm = norm(v);
  Vector<3> c = cross(s[1], s[2]);
  Real c_norm = norm(c);
  if (c_norm > v_norm) {
    v = c;
    v_norm = c_norm;
  }
  c = cross(s[0], s[2]);
  c_norm = norm(c);
  if (c_norm > v_norm) {
    v = c;
    v_norm = c_norm;
  }
  CHECK(v_norm > EPSILON);
  v = v / v_norm;
}

/* in the case that the null space is 2D, find the
   largest-norm column and get a couple vectors
   orthogonal to that */
INLINE void double_eigenvector(Matrix<3,3> m, Real l,
    Vector<3>& u, Vector<3>& v) {
  Matrix<3,3> s = (m - (l * identity_matrix<3,3>()));
  Vector<3> n = s[0];
  Real n_norm = norm(v);
  Vector<3> c = s[1];
  Real c_norm = norm(c);
  if (c_norm > n_norm) {
    n = c;
    n_norm = c_norm;
  }
  c = s[2];
  c_norm = norm(c);
  if (c_norm > n_norm) {
    n = c;
    n_norm = c_norm;
  }
  CHECK(n_norm > EPSILON);
  n = n / n_norm;
  Matrix<3,3> b = form_ortho_basis(n);
  u = b[1]; v = b[2];
}

/* ensure people are checking the return code
   indicating success */
INLINE bool decompose_eigen_cubic(
    Matrix<3,3> m,
    Matrix<3,3>& q,
    Few<Real, 3>& l) __attribute__((warn_unused_result));

INLINE bool decompose_eigen_cubic(
    Matrix<3,3> m,
    Matrix<3,3>& q,
    Vector<3>& l) {
  Real a,b,c;
  characteristic_cubic(m, a, b, c);
  Few<Real, 3> roots;
  Few<UInt, 3> mults;
  UInt nroots = solve_cubic(a, b, c, roots, mults);
  /* there are only a few output cases, see solve_cubic() */
  if (nroots == 3) {
    for (UInt i = 0; i < 3; ++i) {
      single_eigenvector(m, roots[i], q[i]);
      l[i] = roots[i];
    }
  }
  if (nroots == 2 && mults[1] == 2) {
    single_eigenvector(m, roots[0], q[0]);
    l[0] = roots[0];
    double_eigenvector(m, roots[1], q[1], q[2]);
    l[1] = l[2] = roots[1];
    return true;
  }
  if (nroots == 1 && mults[0] == 3) {
    l[0] = l[1] = l[2] = roots[0];
    q = identity_matrix<3,3>();
    return true;
  }
  return false;
}
