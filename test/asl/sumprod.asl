fn main() i64 {
  i64 a = sigma(10);
  i64 b = pi(10) / pi(9);

  add_10(&b);
  add_10(&b);

  return a + b;
}

fn sigma(i64 n) i64 {
  i64 sum = 0, i;

  for i = 1; i <= n; i = i + 1 {
    sum = sum + i;
  }

  return sum;
}

fn pi(i64 n) i64 {
  i64 prod = 1, i;

  for i = 1; i <= n; i = i + 1 {
    prod = prod * i;
  }

  return prod;
}

fn add_10(i64* x) {
  *x = *x + 10;
}
