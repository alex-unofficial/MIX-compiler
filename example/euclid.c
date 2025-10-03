int gcd(int m, int n)
{
  int q = m / n;
  int r = m - q*n;

  if (r == 0)
  {
    return n;
  }
  else
  {
    return gcd(n, r);
  }
}

int main()
{
  return gcd(36, 45);
}
