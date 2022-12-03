i64 x, y;

fn main() i64 {
  i64 a, b;

  x = g:get_10();
  y = -5;

  a = g:min(y, 2);
  a = g:min(a, 0);
  b = g:max(x, 2);
  b = g:max(b, 15);
  x = g:min(g:max(x, 1), 1);

  return a + b + x + y;
}

scope g {
  fn max(i64 x, i64 y) i64 {
    if x > y {
      return x;
    }
    return y;
  }

  fn min(i64 x, i64 y) i64 {
    if x < y {
      return x;
    }
    return y;
  }

  fn get_10() i64 {
    return 2 * 2 + 6;
  }
}
