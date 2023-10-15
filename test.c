int x;
int X[10];

int helper(int val){
	x = x + 1;
	if(val > 0){
		val = val - 1;
		return helper(val);
		//return 0;
	}
	if(val == 0){
		return x;
	}else{
		val = val + 1;
		return -1;
	}
	


}


void main(void) {
	int i;
	int total;
	i = 0;
	total = 0;
	while(i < 10){
		if (i == 7){
			X[i] = 1234;
			i = i + 1;
			continue;
		}
		if(i == 9){
			X[i] = helper(-12);
			break;
		}
		x = 0;
		X[i] = helper(i);
		i = i + 1;
	}

	i = 0;
	write "Printing Array: \n";
	while(i < 10){
		write X[i];
		write "\n";
		i = i + 1;
	}
}

