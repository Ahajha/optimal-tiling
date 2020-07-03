algorithm:

ERs are going to be represented by a list of numbers, such as:
001213

(indexes start at 0, going up to n-1)

Where anything with the same number is equivalent, and anything with different numbers
are not. The numbers have the property that, reading from left to right, if a number
appears in a given position, then all non-negative numbers below it have appeared before.
We could arrange this by giving each group a "group ID", where the order of the smallest
elements in two groups respects the same order as the group IDs.

std::vector<ER> enumerate(unsigned n)
{
	if (n == 1)
	{
		// Only one of size 1
		return { ER(1); }
	}
	else
	{
		vector<ER> smallerERs = enumerate(n-1);

		vector<ER> result = {};
		for (ER R : smallerERs)
		{
			// For each group in R, there will be a new ER
			// with the last member merged with that group.
			unsigned currentGroup = -1;
			
			for (int i = 0; i < n - 1; i++)
			{
				if (currentGroup < groupID(i))
				{
					currentGroup++;
					
					ER Rprime = copy(R);
		
					// Add an element to back of the ER, make it
					// equivalent to i.
					Rprime.addElement();
					Rprime.merge(i,n - 1);
	
					results.add(Rprime);
				}
			}
			ER Rprime = copy(R);

			Rprime.addElement();

			results.add(Rprime);
		}

		return results;
	}
}
