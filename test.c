int fact(int n)
{
	if(n==1)
		return n;
	else
		return fact(n-1);
}
int main()
{
	int n;
	n = read();
	if(n>0) write(1);
	else if(n<0) write(-1);
	else write(0);
	return 0;	
}
