
#include <tuple>
#include <functional>
#include <assert.h>
#include <variant>
#include <type_traits>

namespace msm
{

enum class HandledEnum
{
    No            = 0,
    Yes           = 1,
    Guard_reject  = 2,
    deferred      = 4,
};

inline constexpr HandledEnum operator | (HandledEnum lhs, HandledEnum rhs) noexcept {
	return static_cast<HandledEnum>( static_cast<int>(lhs) | static_cast<int>(rhs) );
}
inline constexpr HandledEnum operator & (HandledEnum lhs, HandledEnum rhs) noexcept {
	return static_cast<HandledEnum>( static_cast<int>(lhs) & static_cast<int>(rhs) );
}

inline constexpr bool to_bool(HandledEnum e) noexcept { return static_cast<int>(e) != 0 ; }


struct state_base
{
	template <typename Event, typename FSM>
	void on_entry(const Event & , FSM& ){}
	
	template <typename Event, typename FSM>
	void on_exit(const Event & , FSM& ) {}
};


template <class D>
class state_machine_def
{
public:
	
	template <class Event,class FSM>
	void on_entry(Event const& ,FSM&){}
	
	template <class Event,class FSM>
	void on_exit(Event const&,FSM& ){}
		
		
	template <typename T1, typename Event, typename T2, void(D::*action)(Event const&), bool (D::*guard)(Event const&) >
	struct row
	{
		using Source = T1;
		using Target = T2;
		using Evt = Event;

		template <class FSM>
		static constexpr HandledEnum action_call(FSM& fsm, Event const& evt)
		{
			if constexpr(action != nullptr)
				(fsm.*action)(evt);
			return HandledEnum::Yes;
		}
		
		template <class FSM>
		static constexpr bool guard_call(FSM& fsm, Event const& evt)
		{
			// in this front-end, we don't need to know source and target states
			if constexpr (guard != nullptr){
				return (fsm.*guard)(evt);
			} else {
				return true;
			}
		}
		
	};
	
	template <typename T1, typename Event, typename T2, void(D::*action)(Event const&)>
	struct a_row: public row<T1, Event, T2, action, nullptr>{};
	
	template <typename T1, typename Event, typename T2, bool(D::*guard)(Event const&)>
	struct g_row: public row<T1, Event, T2, nullptr, guard > {};
	
	template <typename T1, typename Event, typename T2>
	struct _row: public row<T1, Event, T2, nullptr, nullptr>{};
	
	
protected:
	
	// Default no-transition handler. Can be replaced in the Derived SM class.
	template <class FSM,class Event, class State>
	void no_transition(Event const& ,FSM&, State const& )
	{
		assert(false);
	}
	
};
	
template <typename Tuple> struct to_variant_state;

template <typename ... Ts>
struct to_variant_state< std::tuple < Ts ... > > { using type = std::variant<typename std::add_pointer<Ts>::type  ... >; };


template <typename TransactionTable> struct transaction_table_to_state_tuple;

template <typename ... Rows>
struct transaction_table_to_state_tuple< std::tuple< Rows ... > >
{
	using type_source = std::tuple< typename Rows::Source ... > ;
	using type_target = std::tuple< typename Rows::Target ... > ;
	
	using type = decltype(std::tuple_cat(std::declval<type_source>(), std::declval<type_target>()) );
};

template <typename Tuple> struct make_type_unique;


template<typename T> struct type_identity{ using type = T; };

template <typename T, typename... Ts>
struct unique_helper : type_identity<T> {};

template <typename... Ts, typename U, typename... Us>
struct unique_helper< std::tuple<Ts...>, U, Us...>
    : std::conditional_t<(std::is_same_v<U, Ts> || ...)
                       , unique_helper<std::tuple<Ts...>, Us...>
                       , unique_helper<std::tuple<Ts..., U>, Us...>> {};


template <typename ...Ts>
struct make_type_unique< std::tuple<Ts ... > > {
	using type = typename unique_helper<std::tuple<>, Ts...>::type;
};


struct none_event{};
	
	

template <class Def>
class state_machine : public Def
{
public:
	
	using transition_table = typename Def::transition_table;
	
	using library_sm = state_machine<Def>;
	
	using state_tuple = typename make_type_unique< 
							typename transaction_table_to_state_tuple< 
									transition_table 
							>::type 
				>::type;
	
	using state_variant = typename to_variant_state< state_tuple >::type;
	
	using initial_state = typename Def::initial_state;
	
	using initial_event = none_event;
	
	
	template <typename Event>
	struct process_event_visitor
	{
			Event const& evt;
			library_sm& fsm;
			
			explicit  
			process_event_visitor(Event const& evt, library_sm& fsm) noexcept
				: evt(evt), fsm(fsm)
			{}
			
			template <typename State>
			HandledEnum operator()(State * state) const
			{
					//for each row in m_states
					// check if curr row ::Source == State and row::Evt == Event
					//       if ( row::guard_call(fsm, evt) ){ row::action_call(fsm, evt) }
					
					return do_it(state, std::make_index_sequence< std::tuple_size<transition_table>::value > {} );
					
			}
			
			
			// if transaction_table is empty.
			template <typename State>
			HandledEnum do_it(State* state, std::index_sequence<> ) const { return HandledEnum::No; }

			template <typename State, size_t ... index>
			HandledEnum do_it(State * state, std::index_sequence<index ... > ) const
			{
				return (do_each<State, typename std::tuple_element<index, transition_table>::type >( state ) | ... );
			}
			
			template <typename State, typename Row>
			HandledEnum do_each([[maybe_unused]]State * state) const
			{
				if constexpr (std::is_same<State, typename Row::Source>::value && std::is_same<Event, typename Row::Evt >::value )
				{
					if ( !Row::guard_call(fsm, evt) )
						return HandledEnum::Guard_reject;
						
					auto result = Row::action_call(fsm, evt);
					
					state->on_exit(evt, fsm);
					
					using NextState = typename Row::Target;
					NextState* nextState = &std::get<NextState>(fsm.m_states);
					nextState->on_entry(evt, fsm);
					fsm.m_curr_state = nextState;

					return result;
				} else {
					return HandledEnum::No;
				}	
			}
	};
	
	template<class Event>
	void process_event(const Event& evt)
	{
		printf("state_machine_def::process_event() - BEGIN\n");
		using visitor = struct process_event_visitor<Event>;
		
		HandledEnum result = std::visit(visitor{evt, *this}, m_curr_state); 
		
		if (result == HandledEnum::No)
		{
			std::visit( [this, &evt](auto state){
				this->no_transition(evt, *this, *state);
			},
			m_curr_state);
		}
		
		printf("state_machine_def::process_event() - END\n");
	}
	
	
		
		
	state_machine()
		: m_states{}, m_curr_state{ &std::get<initial_state>(m_states) } 
	{
		
		
	}
	
	void start()
	{
		printf("state_machine_def::start() - BEGIN\n");
		std::visit([this](auto state_ptr){
			state_ptr->on_entry(initial_event{}, *this);
		}, 
		m_curr_state);
		
		printf("state_machine_def::start() - END\n");
	}
	
	void stop()
	{
		printf("state_machine_def::stop() - BEGIN\n");
		std::visit([this](auto state_ptr){
			state_ptr->on_exit(initial_event{}, *this);
		}, 
		
		m_curr_state);
		printf("state_machine_def::stop() - END\n");
	}
	
private:
	state_tuple 	m_states;
	state_variant   m_curr_state;
};
	
} // namespace msm


#include <stdio.h>

struct my_event
{
	int data;
};


struct initial_state: public msm::state_base
{
		template <typename Event, typename FSM>
		void on_entry(const Event & , FSM& ){
			puts("initial_state::on_entry()");
		}
		
		template <typename Event, typename FSM>
		void on_exit(const Event & , FSM& ) {
				puts("initial_state::on_exit()");
		}
};

struct next_state: public msm::state_base{
	template <typename Event, typename FSM>
	void on_entry(const Event & , FSM& ){
		puts("next_state::on_entry()");
	}
	
	template <typename Event, typename FSM>
	void on_exit(const Event & , FSM& ) {
		puts("next_state::on_exit()");
	}
};

struct my_state_machine_def : public msm::state_machine_def<my_state_machine_def>
{
	using initial_state = struct initial_state;
	
	using base = msm::state_machine_def<my_state_machine_def>;
	using p = my_state_machine_def;

	void action(const my_event& evt)
	{
		printf("state_machine::action() - my_event data = %d\n", evt.data);
	}

	bool guard(const my_event& evt)
	{ 
		return evt.data > 0; 
	}

	using transition_table = std::tuple<
			/*******************             START         |    EVENT  |  Next     |Action |  Guard      */
			/* --------------------------+-----------------+-----------+-----------+-------+--------+--- */
			typename base:: template row<  initial_state   ,  my_event , next_state, &p::action, &p::guard >,
			typename base:: template row<  next_state      ,  my_event , initial_state, nullptr, &p::guard >
	>;
};

using my_state_msm = msm::state_machine< my_state_machine_def>;
void test_my_state_msms()
{
	my_state_msm fsm;
		fsm.start();
		fsm.process_event(my_event{ 1 } );
		fsm.process_event(my_event{ -1 } );
		fsm.stop();
		
}

int main(int argc, char* argv[])
{
    test_my_state_msms(); 
}
