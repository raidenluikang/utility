#include <string>
#include <string_view>
#include <cstdint>
#include <iostream>
#include <type_traits>

template <typename T>
std::string_view type_to_string_view() noexcept
{
	static const long value = 42;
	static const std::uintptr_t pvalue = ( std::uintptr_t )( &value );
	
	if constexpr (sizeof(std::uintptr_t) == 8) {
		static const char value[17] = 
		{ 
			"0123456789abcdef"[ pvalue >> 60 & 0x0F ],
			"0123456789abcdef"[ pvalue >> 56 & 0x0F ],
			"0123456789abcdef"[ pvalue >> 52 & 0x0F ],
			"0123456789abcdef"[ pvalue >> 48 & 0x0F ],
			"0123456789abcdef"[ pvalue >> 44 & 0x0F ],
			"0123456789abcdef"[ pvalue >> 40 & 0x0F ],
			"0123456789abcdef"[ pvalue >> 36 & 0x0F ],
			"0123456789abcdef"[ pvalue >> 32 & 0x0F ],
			"0123456789abcdef"[ pvalue >> 28 & 0x0F ],
			"0123456789abcdef"[ pvalue >> 24 & 0x0F ],
			"0123456789abcdef"[ pvalue >> 20 & 0x0F ],
			"0123456789abcdef"[ pvalue >> 16 & 0x0F ],
			"0123456789abcdef"[ pvalue >> 12 & 0x0F ],
			"0123456789abcdef"[ pvalue >> 8  & 0x0F ],
			"0123456789abcdef"[ pvalue >> 4  & 0x0F ],
			"0123456789abcdef"[ pvalue >> 0  & 0x0F ],

			'\0'
		};
		return std::string_view(value, 16);
	} else if constexpr (sizeof(std::uintptr_t) == 4) {
		static const char value[9] = 
		{ 
			"0123456789abcdef"[ pvalue >> 28 & 0x0F ],
			"0123456789abcdef"[ pvalue >> 24 & 0x0F ],
			"0123456789abcdef"[ pvalue >> 20 & 0x0F ],
			"0123456789abcdef"[ pvalue >> 16 & 0x0F ],
			"0123456789abcdef"[ pvalue >> 12 & 0x0F ],
			"0123456789abcdef"[ pvalue >> 8  & 0x0F ],
			"0123456789abcdef"[ pvalue >> 4  & 0x0F ],
			"0123456789abcdef"[ pvalue >> 0  & 0x0F ],

			'\0'
		};
		return std::string_view(value, 8);
	}
	else 
	{
		static_assert(sizeof(T)> 0, "sizeof(std::uintptr) = 8 or 4 is supported, only!");
	}
}


int main(){
	
		std::cout << "type_to_string_view(int)     - "  << type_to_string_view<int>() << std::endl;
		std::cout << "type_to_string_view(long)    - "  << type_to_string_view<long>() << std::endl;
		std::cout << "type_to_string_view(double)  - "  << type_to_string_view<double>() << std::endl;
		std::cout << "type_to_string_view(string)  - "  << type_to_string_view<std::string>() << std::endl;
		std::cout << "type_to_string_view(wstring) - "  << type_to_string_view<std::wstring>() << std::endl;
		
}
