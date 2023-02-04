
#include <cstdio>   // std::puts
#include <utility>   // std::make_index_sequence , std::index_sequence
#include <cstddef>  // std::size_t

template <std::size_t N>
struct string_literal
{
	using index_seq = std::make_index_sequence<N>;
	
	char value[N];
	
	constexpr string_literal(const char(&str)[N]) noexcept 
	 : string_literal(str, index_seq{})
	{
		
	}
	
	template <std::size_t ... index>
	constexpr string_literal(const char(&str)[N], std::index_sequence<index ... > ) noexcept
	 : value{ str[index] ... }
	 {}
};


int main(){
	string_literal literal = "hello world";
	std::puts(literal.value);
		
}
