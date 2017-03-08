#include "var.hpp"

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
	
	static const std::string UNDEFINED_VAR_VALUE = "";
	static const std::string IDENTIFIER_CHARS = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz_0123456789";

	static void check_name(const std::string &name);
	static bool find_next_variable(const std::string &text, size_t *pos, std::string *var_text);
	static size_t find_unescaped(const std::string &search, char needle, size_t pos);
	static void remove_escapes(std::string &text);
	static void expand_variables(Expander *ex, std::string &text);

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
		if (is_registered(ex, var))
		{
			throw std::logic_error("variable already exists: " + var);
		}
		ex->substitutions[var].external = false;
		ex->substitutions[var].value = new std::string();
	}
	
	extern void unregister_internal(Expander *ex, const std::string &var)
	{
		if (!is_registered(ex, var))
		{
			return;
		}
		if (ex->substitutions[var].external)
		{
			throw std::logic_error("cannot unregister external variable: " + var);
		}
		delete ex->substitutions[var].value;
		ex->substitutions.erase(var);
	}
	
	extern bool is_registered(const Expander *ex, const std::string &var)
	{
		return ex->substitutions.find(var) != ex->substitutions.end();
	}
	
	extern void set_value(Expander *ex, const std::string &var, const std::string &value)
	{
		if (!is_registered(ex, var))
		{
			throw std::logic_error("variable does not exist: " + var);
		}
		if (ex->substitutions[var].external)
		{
			throw std::logic_error("cannot set external variable: " + var);
		}
		*ex->substitutions[var].value = value;
	}
	
	extern const std::string &get_value(const Expander *ex, const std::string &var)
	{
		if (!is_registered(ex, var))
		{
			return UNDEFINED_VAR_VALUE;
		}
		return *ex->substitutions.at(var).value;
	}
	
	extern void register_external(Expander *ex, const std::string &var, std::string *value_ptr)
	{
		check_name(var);
		if (is_registered(ex, var))
		{
			throw std::logic_error("variable already exists: " + var);
		}
		ex->substitutions[var].external = true;
		ex->substitutions[var].value = value_ptr;
	}
	
	extern void unregister_external(Expander *ex, const std::string &var)
	{
		if (!is_registered(ex, var))
		{
			return;
		}
		if (!ex->substitutions[var].external)
		{
			throw std::logic_error("variable is not external: " + var);
		}
		ex->substitutions.erase(var);
	}
	
	extern void expand(Expander *ex, std::string &text)
	{
		expand_variables(ex, text);
		remove_escapes(text);
	}

	static void remove_escapes(std::string &text)
	{
		bool escaped = false;
		std::string::iterator iter = text.begin();
		while (iter != text.end())
		{
			if (!escaped)
			{
				if (*iter == '\\')
				{
					escaped = true;
					iter = text.erase(iter);
				}
				else
				{
					iter++;
				}
			}
			else
			{
				switch (*iter)
				{
					case '\\':
					case '$':
						escaped = false;
						break;

					default:
						throw std::logic_error("bad escape sequence: \\" + *iter);
						break;
				}
				iter++;
			}
		}
	}
	
	static void expand_variables(Expander *ex, std::string &text)
	{
		size_t pos = 0;
		std::string var_text;
		std::string var;
		while (find_next_variable(text, &pos, &var_text))
		{
			var = var_text.substr(1);
			const std::string val = get_value(ex, var);
			text.replace(pos, var_text.size(), val);
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
			*pos = find_unescaped(text, '$', *pos);
			if (*pos >= text.size() - 1)
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
				else
				{
					(*pos)++;
				}
			}
		}
		return found;
	}

	static size_t find_unescaped(const std::string &search, char needle, size_t pos)
	{
		bool escaped = false;
		for (size_t i = pos; i < search.size(); i++)
		{
			if (search[i] == '\\' && !escaped)
			{
				escaped = true;
			}
			else if (search[i] == needle && !escaped)
			{
				return i;
			}
			else if (escaped)
			{
				escaped = false;
			}
		}
		return std::string::npos;
	}

	static void check_name(const std::string &name)
	{
		if (!is_valid_identifier(name))
		{
			throw std::logic_error("not a valid identifier: " + name);
		}
	}

} }

