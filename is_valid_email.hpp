#pragma once

#include <stddef.h> // size_t


/*
(?:(?:[^<>()\[\].,;:\s@"]+(?:\.[^<>()\[\].,;:\s@"]+)*)|".+")@(?:(?:[^<>()\[\].,;:\s@"]+\.)+[^<>()\[\].,;:\s@"]{2,})
[^<>()\[\].,;:\s@"] = T
[^<>()\[\].,;:\s@"]
[^<>()\[\].,;:\s@"]
[^<>()\[\].,;:\s@"]
(?:(?:T+(?:\.T+)*)|".+")@(?:(?:T+\.)+T{2,})
       name @ domain
       
name = (?:(?:T+(?:\.T+)*)|".+")  ==> if avoid group ==>  T+(\.T+)*|".+"   -> T+(\.T+)*  OR  ".+"
domain = (?:(?:T+\.)+T{2,})        ==> if avoid group ==>  (T+\.)+T{2,}   --> s1 . s2 . ... . sn   every si -> is T,  and strlen(sn) >= 2
*/
 

constexpr bool is_valid_email_symbol(const char c) noexcept
{
    switch(c)
    {
	    case '<':
	    case '>':
	    case '(':
	    case ')':
	    case '[':
	    case ']':
	    case '.':
	    case ',':
	    case ';':
	    case ':':
	    
	    //whitespace C local
	    case 0x20: //space ' '
	    case 0x0c: //form feed '\f'
	    case 0x0a: //line feed '\n'
	    case 0x0d: //carriage return (0x0d, '\r')
	    case 0x09: //horizontal tab (0x09, '\t') 
	    case 0x0b: //vertical tab (0x0b, '\v') 

	    //dog sign
	    case '@':
	    
	    //and last quote symbol
	    case '\"':
	    	return false;

	    default:
	    	return true;
	    
    }
}

constexpr bool is_valid_email_name(const char* str, size_t len) noexcept
{
	//An empty string not allowed
	if (len == 0) return false;
	
	//1. fully quoted string
	if (str[0] == '\"')
	{
		if (len <= 2) return false;
		if (str[len-1] != '\"') return false;
		
		// dot not matched to line feed
		for (size_t x = 1; x < len - 1; ++x) 
		{
			if (str[x] == '\n') return false;
		}
		
		return true;
	}
	
	//2. T+(\.T+)*
	size_t hlen = 0;
	for (size_t x = 0; x != len; ++x){
		const char c = str[x];
		if (c == '.')
		{
			if (hlen == 0) return false; // dot dot not allowed
			hlen = 0;
		} 
		else if (is_valid_email_symbol(c))
		{
			++hlen;
		}
		else
		{
			return false;
		}
	}
	
	return hlen > 0; // last symbol must be T symbol.

}

constexpr bool is_valid_email_domain(const char* str, size_t len) noexcept
{
	//(T+\.)+T{2,} 

     size_t hlen = 0;
     bool dot_found = false;
     for (size_t x = 0; x != len; ++x)
     {
     	 const char c = str[x];
         if (c == '.')
         {
               if (hlen == 0) return false; // dot dot  not allowed. Also dot should not be a first character.
               hlen = 0; //start a new group
               dot_found = true;
         } else if (is_valid_email_symbol(c)){
             hlen++;
         } else {
         	return false;
         }
     }
     
     if (!dot_found) return false; // must be at least one dot
     return hlen >= 2; //last group should have  two or more symbols.
}

constexpr bool is_valid_email_me(const char* str, size_t len) noexcept
{
	for (size_t x = len; x > 0 ; --x){
		if ( str[x - 1] == '@' )
		{
		    // B: str[x] ... str[len-1]:  total len -x
		    // A: str[0] ... str[x-2]  :  total x - 1
		    return is_valid_domain(str + x, len - x) && is_valid_name(str, x - 1);
		}
	}
	return false;
}

template <typename StringLike> // std::string, std::string_view or some similar classes.
bool is_valid_email_me(const StringLike& email) { 
	const auto pos = email.rfind('@');
	const auto size = email.size();
	return pos != email.npos  && is_valid_name(email.data(), pos) && is_valid_domain(email.data() + pos + 1, size - (pos + 1));
	//return is_valid_email_me(email.c_str(), email.size()); 
}
