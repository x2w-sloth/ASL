fn main() i64 {

  if is_odd(21)  != true { return 1; }
  if is_even(20) != true { return 1; }
  if is_leap_year(2022) != false { return 1; }
  if is_leap_year(2024) != true { return 1; }
  if is_leap_year(2000) != true { return 1; }

  return 0;
}

fn is_even(i32 x) bool {
  return x % 2 == 0;
}

fn is_odd(i32 x) bool {
  return is_even(x) == false;
}

fn is_leap_year(i32 year) bool {
  if year % 400 == 0 {
    return true;
  }
  if year % 100 == 0 {
    return false;
  }
  if year % 4 == 0 {
    return true;
  }
  return false;
}
