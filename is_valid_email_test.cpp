
 
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <utility>

#include <ctre.hpp>

#include <string_view>
#include <regex>
#include <chrono>
#include <string>
#include <vector>

#include <boost/regex.hpp>

#if 1
bool is_valid_email_ctre (const std::string& email)
{
    return (bool)ctre::match<R"rx((?:(?:[^<>()\[\].,;:\s@"]+(?:\.[^<>()\[\].,;:\s@"]+)*)|".+")@(?:(?:[^<>()\[\].,;:\s@"]+\.)+[^<>()\[\].,;:\s@"]{2,}))rx">(email);
}
#endif

bool is_valid_email_boost(const std::string& email) 
{
    static const boost::regex rx{R"rx((?:(?:[^<>()\[\].,;:\s@"]+(?:\.[^<>()\[\].,;:\s@"]+)*)|".+")@(?:(?:[^<>()\[\].,;:\s@"]+\.)+[^<>()\[\].,;:\s@"]{2,}))rx"};
    return boost::regex_match(email, rx);
}
bool is_valid_email_std(const std::string& email) 
{
    static const std::regex rx{R"rx((?:(?:[^<>()\[\].,;:\s@"]+(?:\.[^<>()\[\].,;:\s@"]+)*)|".+")@(?:(?:[^<>()\[\].,;:\s@"]+\.)+[^<>()\[\].,;:\s@"]{2,}))rx"};
    return std::regex_match(email, rx);
}


/*
(?:(?:[^<>()\[\].,;:\s@"]+(?:\.[^<>()\[\].,;:\s@"]+)*)|".+")@(?:(?:[^<>()\[\].,;:\s@"]+\.)+[^<>()\[\].,;:\s@"]{2,})

[^<>()\[\].,;:\s@"] = T
[^<>()\[\].,;:\s@"]
[^<>()\[\].,;:\s@"]
[^<>()\[\].,;:\s@"]


(?:(?:T+(?:\.T+)*)|".+")@(?:(?:T+\.)+T{2,})

       A @ B
       
A = (?:(?:T+(?:\.T+)*)|".+")  ==> if avoid group ==>  T+(\.T+)*|".+"   -> T+(\.T+)*  OR  ".+"
B = (?:(?:T+\.)+T{2,})        ==> if avoid group ==>  (T+\.)+T{2,}   --> s1 . s2 . ... . sn   every si -> is T,  and strlen(sn) >= 2

*/


constexpr bool is_valid_T(const char c) noexcept
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

constexpr bool is_valid_A(const char* str, size_t len) noexcept
{
	//An empty string not allowed
	if (len == 0) return false;
	
	//1. fully quoted string
	if (str[0] == '\"')
	{
		if (len <= 2) return false;
		if (str[len-1] != '\"') return false;
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
		else if (is_valid_T(c))
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

constexpr bool is_valid_B(const char* str, size_t len) noexcept
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
         } else if (is_valid_T(c)){
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
		    return is_valid_B(str + x, len - x) && is_valid_A(str, x - 1);
		}
	}
	return false;
}

bool is_valid_email_me(const std::string& email) { 
	const auto pos = email.rfind('@');
	const auto size = email.size();
	return pos != email.npos  && is_valid_A(email.data(), pos) && is_valid_B(email.data() + pos + 1, size - (pos + 1));
	//return is_valid_email_me(email.c_str(), email.size()); 
}


int main()
{
    {
        std::string less = "abraca@gmail.com";
        std::string upp = "AbRaCa@GmAiL.CoM";
        bool less_valid = is_valid_email_std(less);
        bool upp_valid = is_valid_email_std(upp);
        printf("STD: less_valid = %d  upp_valid = %d\n", (int)less_valid, (int)upp_valid);

        less_valid = is_valid_email_boost(less);
        upp_valid = is_valid_email_boost(upp);
        printf("BOOST: less_valid = %d  upp_valid = %d\n", (int)less_valid, (int)upp_valid);
#if 1
        less_valid = is_valid_email_ctre(less);
        upp_valid = is_valid_email_ctre(upp);
        printf("CTRE: less_valid = %d  upp_valid = %d\n", (int)less_valid, (int)upp_valid);
#endif

    }
    const int N = 1000000;
    srand((unsigned)time(0));
    std::vector<std::string> emails;
    emails.reserve(N);
    for (int i = 0; i < N; ++i)
    {
        std::string email;
        email.resize(rand() % 256 + 16);
        bool first = false;
        bool dot = false;
        int index = 0;
        if (rand()%2 == 1){
        for (char& c : email)
          {
                c = rand() % 26 + 64;
                ++index;
                if (!first && rand() % 2 == 0 && index > 2)
                {
                    first = true;
                    c = '@';
                }
                else
                if (first && !dot && rand() % 2== 0 && index > 4){
					dot = true;
					c  = '.';
				}
          }
	  } else {
		  email[0] = '\"';
		  email[email.size() - 1] = '\"';
		  for (size_t index = 1; index < email.size() - 1; ++index)
		     email[index] = rand() % 117 + 9;
	  }

          emails.push_back(email);
    }
    printf("====================================================\n");
    //test std::regex
    {
		auto start = std::chrono::steady_clock::now();
		int valid_count = 0;
		for (int i = 0; i < N; ++i){
			if (is_valid_email_std(emails[i]))
				++valid_count;
		}
		auto finish = std::chrono::steady_clock::now();
		
		printf("STD: valid_count = %d\n", valid_count);
		printf("STD: elapsed time = %.9f\n", std::chrono::duration<double>(finish - start).count());
        long int per = std::chrono::duration_cast<std::chrono::nanoseconds>(finish - start).count() / N;
        printf("STD: is_valid_email took %ld nanoseconds\n", per);
	}
	printf("=====================================================\n");
     //test boost::regex
    {
		auto start = std::chrono::steady_clock::now();
		int valid_count = 0;
		for (int i = 0; i < N; ++i){
			if (is_valid_email_boost(emails[i]))
				++valid_count;
		}
		auto finish = std::chrono::steady_clock::now();
		
		printf("BOOST: valid_count = %d\n", valid_count);
		printf("BOOST: elapsed time = %.9f\n", std::chrono::duration<double>(finish - start).count());
        long int per = std::chrono::duration_cast<std::chrono::nanoseconds>(finish - start).count() / N;
        printf("BOOST: is_valid_email took %ld nanoseconds\n", per);
	}
	printf("=====================================================\n");
	//test ctre
	#if 1
	{
		auto start = std::chrono::steady_clock::now();
		int valid_count = 0;
		for (int i = 0; i < N; ++i){
			if (is_valid_email_ctre(emails[i]))
				++valid_count;
		}
		auto finish = std::chrono::steady_clock::now();
		
		printf("CTRE: valid_count = %d\n", valid_count);
		printf("CTRE: elapsed time = %.9f\n", std::chrono::duration<double>(finish - start).count());
		
        long int per = std::chrono::duration_cast<std::chrono::nanoseconds>(finish - start).count() / N;
        printf("CTRE: is_valid_email took %ld nanoseconds\n", per);
	}
	#endif
	
	printf("====================================================\n");
	//TEST my implementation
	{
		
		auto start = std::chrono::steady_clock::now();
		int valid_count = 0;
		for (int i = 0; i < N; ++i){
			if (is_valid_email_me(emails[i]))
				++valid_count;
		}
		auto finish = std::chrono::steady_clock::now();
		
		printf("ME: valid_count = %d\n", valid_count);
		printf("ME: elapsed time = %.9f\n", std::chrono::duration<double>(finish - start).count());
		
        long int per = std::chrono::duration_cast<std::chrono::nanoseconds>(finish - start).count() / N;
        printf("ME: is_valid_email took %ld nanoseconds\n", per);
		
	}
	
	int mismatched = 0;
	for (int i = 0; i < N; ++i){
		bool me_value = is_valid_email_me(emails[i]);
		bool std_value =  is_valid_email_std(emails[i]);
		if (me_value != std_value)
		{
			printf("mismatch email: %s\n", emails[i].c_str());
			printf("me_value = %d, std_value = %d\n", (int)me_value, (int)std_value);
			++mismatched;
			if (mismatched > 10)break;
		}
	}
    return 0;
}
