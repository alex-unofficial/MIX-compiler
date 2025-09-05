int foo(int x, int x)
{
  int a = x;

  return a;
}

int foo()
{
  int b = 0;

  return b;
}

int bar(int a, int b)
{
  int x, b;

  return x;
}

int main()
{
  int a = 0;
  int a = foo(a);

  a = bar(a, 0);

  return a;
}
