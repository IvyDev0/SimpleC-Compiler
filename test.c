int main()
{
	int n = read();
	
	if (n < 0)
	{
		if (n > 5) write(1);
		else write(4);
	}
	else write(3);

	return 0;
}
