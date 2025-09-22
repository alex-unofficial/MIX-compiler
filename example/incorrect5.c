int method1(int a)
{
  int b;
  b = a + 10;
  if (b) {
    break;
  }
  else;

  return b;
}

int method2(int c, int d)
{
  int e;
  e = method1(c);
  e = e + d;
  return e;
}
