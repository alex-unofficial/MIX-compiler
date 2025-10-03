int fact(int n)
{
  int f = 1;
  while (n > 1)
  {
    f = f * n;
    n = n - 1;
  }

  return f;
}

int main()
{
  return fact(5);
}
