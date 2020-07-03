#include "equivRelation.hpp"
#include <iostream>

void compare(const equivRelation& er1, const equivRelation& er2)
{
	std::cout << "er1 <  er2: " << (er1 <  er2) << std::endl;
	std::cout << "er1 <= er2: " << (er1 <= er2) << std::endl;
	std::cout << "er1 >  er2: " << (er1 >  er2) << std::endl;
	std::cout << "er1 >= er2: " << (er1 >= er2) << std::endl;
	std::cout << "er1 == er2: " << (er1 == er2) << std::endl;
	std::cout << "er1 != er2: " << (er1 != er2) << std::endl;
	
	std::cout << std::endl;
}

int main()
{
	/*
	// Test reverse
	
	// For each size of ER
	for (unsigned i = 1; i < 10; i++)
	{
		// For each ER of that size
		auto ERs = equivRelation::enumerate(i);
		
		for (auto& er : ERs)
		{
			auto rev = er.reverse();
			
			// Ensure the definition of reversal holds
			for (unsigned j = 0; j < i; j++)
			{
				for (unsigned k = 0; k < i; k++)
				{
					if (er.equivalent(j,k) != rev.equivalent(i-j-1,i-k-1))
					{
						std::cout << "uh" << std::endl;
						return 1;
					}
				}
			}
		}
	}
	*/
	
	// Test comparisons
	
	equivRelation er1(4), er2(5);
	
	compare(er1,er2);
	
	equivRelation er3(5), er4(5);
	
	er3.merge(1,2);
	er4.merge(2,1);
	
	compare(er3,er4);
	
	equivRelation er5(5), er6(5);
	
	er5.merge(1,2);
	er5.merge(2,3);
	er5.merge(3,4);
	
	er6.merge(1,2);
	er6.merge(3,4);
	er6.merge(1,4);
	
	compare(er5,er6);
	
	equivRelation er7(5), er8(5);
	
	er7.merge(0,4);
	
	compare(er7,er8);
	
	return 0;
}
