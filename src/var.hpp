#ifndef VAR_HPP
#define VAR_HPP

#include <string>

namespace msa { namespace var {

	typedef struct expander_type Expander;

	extern void create_expander(Expander **ex);
	extern void dispose_expander(Expander *ex);
	extern void register_internal(Expander *ex, const std::string &var);
	extern void unregister_internal(Expander *ex, const std::string &var);
	extern bool is_registered(const Expander *ex, const std::string &var);
	extern void set_value(Expander *ex, const std::string &var, const std::string &value);
	extern const std::string &get_value(const Expander *ex, const std::string &var);
	extern void register_external(Expander *ex, const std::string &var, std::string *value_ptr);
	extern void unregister_external(Expander *ex, const std::string &var);
	extern void expand(Expander *ex, std::string &text);
	extern bool is_valid_identifier(const std::string &str);
	
} }

#endif
