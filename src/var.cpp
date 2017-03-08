#include "var.hpp"
#include "string.hpp"

#include <map>
#include <stdexcept>

namespace msa { namespace var {

	typedef struct expander_entry_type
	{
		bool external;
		std::string *value;
	} ExpanderItem;

	struct expander_type
	{
		std::map<std::string, ExpanderItem> substitutions;
	};
	
	static const std::string IDENTIFIER_CHARS = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz_0123456789";

	static void check_name(const std::string &name);
	static bool find_next_variable(const std::string &text, size_t *pos, std::string *var_text);

	extern void create_expander(Expander **expander)
	{
		Expander *ex = new Expander;
		*expander = ex;
	}
	
	extern void dispose_expander(Expander *expander)
	{
		std::map<std::string, ExpanderItem>::iterator iter = expander->substitutions.begin();
		while (iter != expander->substitutions.end())
		{
			if (!iter->second.external)
			{
				delete iter->second.value;
			}
			iter = expander->substitutions.erase(iter);
		}
		delete expander;
	}
	
	extern void register_internal(Expander *ex, const std::string &var)
	{
		check_name(var);
		std::string name = std::string(var);
		msa::string::to_upper(name);
		if (is_registered(ex, name))
		{
			throw std::logic_error("variable already exists: " + name);
		}
		ex->substitutions[name].external = false;
		ex->substitutions[name].value = new std::string();
	}
	
	extern void unregister_internal(Expander *ex, const std::string &var)
	{
		std::string name = std::string(var);
		msa::string::to_upper(name);
		if (!is_registered(ex, name))
		{
			return;
		}
		if (ex->substitutions[name].external)
		{
			throw std::logic_error("cannot unregister external variable: " + name);
		}
		delete ex->substitutions[name].value;
		ex->substitutions.erase(name);
	}
	
	extern bool is_registered(const Expander *ex, const std::string &var)
	{
		std::string name = std::string(var);
		msa::string::to_upper(name);
		return ex->substitutions.find(name) != ex->substitutions.end();
	}
	
	extern void set_value(Expander *ex, const std::string &var, const std::string &value)
	{
		std::string name = std::string(var);
		msa::string::to_upper(name);
		if (!is_registered(ex, name))
		{
			throw std::logic_error("variable does not exist: " + name);
		}
		if (ex->substitutions[name].external)
		{
			throw std::logic_error("cannot set external variable: " + name);
		}
		*ex->substitutions[name].value = value;
	}
	
	extern const std::string &get_value(const Expander *ex, const std::string &var)
	{
		std::string name = std::string(var);
		msa::string::to_upper(name);
		if (!is_registered(ex, name))
		{
			throw std::logic_error("variable does not exist: " + name);
		}
		return *ex->substitutions.at(name).value;
	}
	
	extern void register_external(Expander *ex, const std::string &var, std::string *value_ptr)
	{
		check_name(var);
		std::string name = std::string(var);
		msa::string::to_upper(name);
		if (is_registered(ex, name))
		{
			throw std::logic_error("variable already exists: " + name);
		}
		ex->substitutions[name].external = true;
		ex->substitutions[name].value = value_ptr;
	}
	
	extern void unregister_external(Expander *ex, const std::string &var)
	{
		std::string name = std::string(var);
		msa::string::to_upper(name);
		if (!is_registered(ex, name))
		{
			return;
		}
		if (!ex->substitutions[name].external)
		{
			throw std::logic_error("variable is not external: " + name);
		}
		ex->substitutions.erase(name);
	}
	
	extern void expand(Expander *ex, std::string &text)
	{
		size_t pos = 0;
		std::string var_text;
		std::string var;
		while (find_next_variable(text, &pos, &var_text))
		{
			var = var_text.substr(1);
			if (is_registered(ex, var))
			{
				text.replace(pos, var_text.size(), get_value(ex, var));
			}
			else
			{
				pos++;
			}
		}
	}
	
	extern bool is_valid_identifier(const std::string &str)
	{
		if (str[0] >= '0' && str[0] <= '9')
		{
			return false;
		}
		size_t bad_char_pos = str.find_first_not_of(IDENTIFIER_CHARS, 0);
		return bad_char_pos == std::string::npos;
	}
	
	static bool find_next_variable(const std::string &text, size_t *pos, std::string *var_text)
	{
		bool found = false;
		while (!found && *pos != std::string::npos)
		{
			*pos = text.find('$', *pos);
			if (*pos == text.size() - 1)
			{
				*pos = std::string::npos;
			}
			else if (text[*pos + 1] < '0' || text[*pos + 1] > '9')
			{
				size_t non_ident = text.find_first_not_of(IDENTIFIER_CHARS, *pos + 1);
				if (non_ident > *pos + 1)
				{
					size_t end_pos = (non_ident != std::string::npos) ? non_ident - 1 : text.size() - 1;
					*var_text = text.substr(*pos, (end_pos - *pos) + 1);
					found = true;
				}
				(*pos)++;
			}
		}
		return found;
	}

	static void check_name(const std::string &name)
	{
		if (!is_valid_identifier(name))
		{
			throw std::logic_error("not a valid identifier: " + name);
		}
	}

} }

