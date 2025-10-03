int gcd(int m, int n)
{
  int q = m / n;
  int r = m - q*n;

  while (r > 0)
  {
    m = n;
    n = r;

    q = m / n;
    r = m - q*n;
  }

  return n;
}

int main()
{
  return gcd(36, 45);
}
